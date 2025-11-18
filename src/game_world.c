#include <stdio.h>
#include <unistd.h>
#include "game_process.h"
#include "game_tools.h"
#include "game_utils.h"
#include "game_ui.h"

game_process_t game_process;
TreeCacheEntry tree_cache[18] = {0};
int tree_cache_count = 0;
ent_t* player = {0};
static world_t world;

ent_t* WorldPlayer(void){
  return player;
}

bool TogglePause(ui_menu_t* m){
  if(game_process.state[SCREEN_GAMEPLAY] == GAME_READY)
    GameSetState(GAME_PAUSE);
  else if (game_process.state[SCREEN_GAMEPLAY] == GAME_PAUSE)
    GameSetState(GAME_READY);

  return true;
}

void GameSetState(GameState state){
  if(game_process.screen != SCREEN_GAMEPLAY)
    return;

  game_process.state[SCREEN_GAMEPLAY] = state;
}

void GameReady(void *context){
  WorldInitOnce();
  game_process.state[SCREEN_GAMEPLAY] = GAME_READY;
}


void AddFloatingText(render_text_t *rt){
  for (int i = 0; i < MAX_EVENTS; i++){
    if(world.floatytext_used[i])
      continue;

    world.floatytext_used[i] = true;
    world.texts[i] = rt;
    return;
  }
}
map_grid_t* WorldGetMap(void){
  return world.map;
}

ent_t* WorldGetEntAtTile(Cell tile){
  return MapGetTile(world.map,tile)->occupant;
}

Cell GetWorldCoordsFromIntGrid(Cell pos, float len){
  int grid_x = (int)pos.x/CELL_WIDTH;
  int grid_y = (int)pos.y/CELL_HEIGHT;

  int grid_step = (int)len/CELL_WIDTH;

  int start_x = CLAMP(grid_x - grid_step,1,GRID_WIDTH);
  int start_y = CLAMP(grid_y - grid_step,1,GRID_HEIGHT);

  int end_x = CLAMP(grid_x + grid_step,1,GRID_WIDTH);
  int end_y = CLAMP(grid_y + grid_step,1,GRID_HEIGHT);


 Cell candidates[GRID_WIDTH * GRID_HEIGHT];
/*
 int count = 0;
  for (int x = start_x; x < end_x; x++)
    for(int y = start_y; y < end_y; y++){
      if(world.intgrid[x][y])
        continue;
      if(distance(grid_x,grid_y,x,y) > len)
        continue;

      candidates[count++] = (Cell){x,y};
    }

  if (count == 0)
    return CELL_EMPTY;

  int r = rand() % count;
*/
  //return candidates[r];
}


int WorldGetEnts(ent_t** results,EntFilterFn fn, void* params){
  int num_results = 0;
  for(int i = 0; i < world.num_ent; i++){
    if(!fn(world.ents[i],params))
      continue;

    results[num_results] = world.ents[i];
    num_results++;
  }

  return num_results;
}

bool RegisterBehaviorTree(BehaviorData data){
  TreeCacheEntry entry = {0};
  entry.id = data.id;
  entry.root = BuildTreeNode(data.id,NULL);
  tree_cache[tree_cache_count++] = entry;

  return entry.root!=NULL;
}

bool WorldGetTurnState(void){
  return !CheckEvent(game_process.events,EVENT_TURN);
}

ent_t* WorldGetEnt(const char* name){
  return NULL;
}

ent_t* WorldGetEntById(unsigned int uid){
  for(int i = 0; i < world.num_ent; i++){
    if(world.ents[i]->uid == uid)
      return world.ents[i];
  }

  return NULL;
}

int RemoveSprite(int index){
  int last_pos = world.num_spr -1;
  
  if(!FreeSprite(world.sprs[index]))
    return 0;

  world.num_spr--;
  if(index!=last_pos){
    world.sprs[index] = world.sprs[last_pos];
    return 1;
  }
 
  return 0;
  
}

int RemoveEnt(int index){
  int last_pos = world.num_ent -1;

  if(!FreeEnt(world.ents[index]))
    return 0;

  world.num_ent--;
  if(index!=last_pos){
    world.ents[index] = world.ents[last_pos];
    return 1;
  }

  return 0;

}

int AddEnt(ent_t *e) {
  if (world.num_ent < MAX_ENTS) {
    int index = world.num_ent;
    world.ents[index] = e;
    world.num_ent++;

    return CURRENT_ENT_IDENTIFIER++;
  }
  return -1;
} 

int AddSprite(sprite_t *s){
  if(world.num_spr < MAX_ENTS){
    int index = world.num_spr;
    world.sprs[world.num_spr++] = s;

    return index;
  }

  return -1;
}

bool RegisterEnt( ent_t *e){
  e->uid = AddEnt(e);

  TileStatus status = MapSetOccupant(world.map,e,e->pos);
  if(status > TILE_ISSUES)
    TraceLog(LOG_WARNING,"Issue %i at tile %i,%i ",status,e->pos.x,e->pos.y);

  if(e->type == ENT_PERSON)
    player = e;

  if(e->sprite)
    RegisterSprite(e->sprite);

  return e->uid > -1;
}

bool RegisterSprite(sprite_t *s){
  s->suid = AddSprite(s);

  return s->suid > -1;
}

void WorldInitOnce(){
  InteractionStep();
  for(int i = 0; i< world.num_ent; i++)
    EntInitOnce(world.ents[i]);
}

void WorldPreUpdate(){
  InteractionStep();
  
  for(int i = 0; i < world.num_spr; i++){
    SpriteSync(world.sprs[i]);
  }
}

void WorldFixedUpdate(){
  if(ActionInput()){
    ResetEvent(game_process.events,EVENT_TURN);
  }
  for(int i = 0; i < world.num_ent; i++){
    switch(world.ents[i]->state){
      case STATE_END:
        i-=RemoveEnt(i);
        break;
      case STATE_DIE:
        EntDestroy(world.ents[i]);
        break;
      default:
        EntSync(world.ents[i]);
        break;
    }
  }
}

void WorldPostUpdate(){
  for(int i = 0; i < MAX_EVENTS; i++){
    if(!world.floatytext_used[i])
      continue;

    if(world.texts[i]->duration <=0){
      world.floatytext_used[i] =false;
      continue;
    }

    world.texts[i]->duration--;
  }
}

void WorldTurnUpdate(void* context){
  for(int i = 0; i < world.num_ent; i++)
    ActionSync(world.ents[i]);
}

void InitWorld(world_data_t data){
  world = (world_t){0};
  
  world.map = InitMapGrid();
  
  for (int i = 1; i < ENT_DONE;i++){
    if (room_instances[i].id == ENT_DONE)
      break;

    RegisterEnt(InitEnt(room_instances[i]));
  }
}

void FreeWorld(){
  for (int i = 0; i < world.num_spr; i++){
    RemoveSprite(i);
  }
  world.num_spr = 0;

  for (int i = 0; i < world.num_ent; i++){
    RemoveEnt(i);
  }
  world.num_ent = 0;
}

void WorldRender(){
  for(int i = 0; i < world.num_spr;i++)
    if(world.sprs[i]->owner !=NULL)
      DrawSprite(world.sprs[i]);
    else
      i-=RemoveSprite(i);

  for(int i = 0; i < MAX_EVENTS; i++){
    if(!world.floatytext_used[i])
      continue;
    render_text_t rt = *world.texts[i];
    DrawTextEx(ui.font,rt.text, rt.pos,rt.size,1,rt.color);
  }

}

void InitGameProcess(){
  for(int i = 0; i < BEHAVIOR_COUNT; i++){
    if(room_behaviors[i].id == BEHAVIOR_COUNT)
      break;

    if(room_behaviors[i].is_root)
      RegisterBehaviorTree(room_behaviors[i]);
  }


  for(int s = 0; s<SCREEN_DONE; s++){
    game_process.album_id[s] = -1;
    for(int u = 0; u<UPDATE_DONE;u++){
      game_process.update_steps[s][u] = DO_NOTHING;
    
    }
    game_process.children[s].process= PROCESS_NONE;
    for(int p = 0; p < PROCESS_DONE; p++)
      game_process.children[s].state[p]=GAME_NONE;
  }

  game_process.next[SCREEN_TITLE] = SCREEN_OPTIONS;
  game_process.init[SCREEN_TITLE] = InitTitleScreen;
  game_process.finish[SCREEN_TITLE] = UnloadTitleScreen;
  game_process.update_steps[SCREEN_TITLE][UPDATE_DRAW] = DrawTitleScreen;
  game_process.update_steps[SCREEN_TITLE][UPDATE_FRAME] = UpdateTitleScreen;

  game_process.next[SCREEN_OPTIONS] = SCREEN_GAMEPLAY;
  game_process.init[SCREEN_OPTIONS] = InitOptionsScreen;
  game_process.finish[SCREEN_OPTIONS] = UnloadOptionsScreen;
  game_process.finish[SCREEN_OPTIONS] = UnloadOptionsScreen;
  game_process.update_steps[SCREEN_OPTIONS][UPDATE_DRAW] = DrawOptionsScreen;
  game_process.update_steps[SCREEN_OPTIONS][UPDATE_FRAME] = UpdateOptionsScreen;

 
  game_process.next[SCREEN_GAMEPLAY] = SCREEN_ENDING;
  game_process.init[SCREEN_GAMEPLAY] = InitGameplayScreen;
  game_process.finish[SCREEN_GAMEPLAY] = UnloadGameplayScreen;
  game_process.update_steps[SCREEN_GAMEPLAY][UPDATE_FIXED] = FixedUpdate;
  game_process.update_steps[SCREEN_GAMEPLAY][UPDATE_PRE] = PreUpdate;
  game_process.update_steps[SCREEN_GAMEPLAY][UPDATE_DRAW] = DrawGameplayScreen;
  game_process.update_steps[SCREEN_GAMEPLAY][UPDATE_FRAME] = UpdateGameplayScreen;
  game_process.update_steps[SCREEN_GAMEPLAY][UPDATE_POST] = PostUpdate;
   
  game_process.next[SCREEN_ENDING] = SCREEN_TITLE;
  game_process.init[SCREEN_ENDING] = InitEndScreen;
  game_process.finish[SCREEN_ENDING] = UnloadEndScreen;
  game_process.update_steps[SCREEN_ENDING][UPDATE_DRAW] = DrawEndScreen;
  game_process.update_steps[SCREEN_ENDING][UPDATE_FRAME] = UpdateEndScreen;

  game_process.screen = SCREEN_TITLE;
  game_process.state[SCREEN_GAMEPLAY] = GAME_LOADING;
  game_process.events = InitEvents();
  game_process.init[SCREEN_TITLE]();

}

void InitGameEvents(){
  world_data_t wdata = {0};

  cooldown_t* loadEvent = InitCooldown(6,EVENT_GAME_PROCESS,GameReady,NULL);
  AddEvent(game_process.events,loadEvent);
  InitWorld(wdata);
  game_process.children[SCREEN_GAMEPLAY].process = PROCESS_LEVEL;
  game_process.game_frames = 0;

 cooldown_t* turnEvent = InitCooldown(20,EVENT_TURN,WorldTurnUpdate,NULL);
 turnEvent->is_recycled = true;
 AddEvent(game_process.events,turnEvent);
}

bool GameTransitionScreen(){
  GameScreen current = game_process.screen;
  GameScreen prepare = game_process.next[current];
  if(game_process.state[current] >= GAME_FINISHED)
    return false;
  game_process.init[prepare]();
  game_process.state[current] = GAME_FINISHED;
  game_process.finish[current]();
  game_process.screen = prepare;
  game_process.state[prepare] = GAME_LOADING;
  return true;
}

void GameProcessStep(){
  if(game_process.screen != SCREEN_GAMEPLAY)
    return;

  game_process.game_frames++;
  if(game_process.events)
    StepEvents(game_process.events);
}

void GameProcessSync(bool wait){
  if(game_process.state[game_process.screen] > GAME_READY){

    game_process.update_steps[SCREEN_GAMEPLAY][UPDATE_DRAW]();
  
    return;
  }
  
  for(int i = 0; i <UPDATE_DONE;i++){
    if(i > UPDATE_DRAW && wait)
      return;
    game_process.update_steps[game_process.screen][i]();
  }

  for(int i = 0; i < PROCESS_DONE;i++){
    if(game_process.children[game_process.screen].process==PROCESS_NONE)
      continue;
    child_process_t* kids = &game_process.children[game_process.screen];
    for(int j = 0; j < UPDATE_DONE; j++)
      if(kids->update_steps[i][j]!=NULL)
        kids->update_steps[i][j]();
  }
}

void GameProcessEnd(){
  UnloadEvents(game_process.events);
  FreeWorld();
  FreeInteractions();
}

const char* GetGameTime(){
  return TextFormat("%09i",(int)(game_process.game_frames/fixedFPS));
}
