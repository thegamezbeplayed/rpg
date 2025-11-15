#include <stdio.h>
#include <unistd.h>
#include "game_process.h"
#include "game_tools.h"
#include "game_helpers.h"
#include "game_utils.h"
#include "game_ui.h"

game_process_t game_process;
TreeCacheEntry tree_cache[18] = {0};
int tree_cache_count = 0;

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

static world_t world;

void AddFloatingText(render_text_t *rt){
  for (int i = 0; i < MAX_EVENTS; i++){
    if(world.floatytext_used[i])
      continue;

    world.floatytext_used[i] = true;
    world.texts[i] = rt;
    return;
  }
}

void WorldTurnAddMatch(ent_t* e, bool color_matches){
  Cell epos = e->intgrid_pos;
  int cell = 0;
  if(world.grid.matches[0][epos.x][epos.y])
    cell = 1;

    world.grid.matches[cell][epos.x][epos.y] = e;
    if(color_matches)
      world.grid.color_matches[cell][epos.x][epos.y] = true;

}

float WorldGetGridCombo(Cell intgrid){
  grid_combo_t *grid = world.grid.combos[intgrid.x][intgrid.y];

  return grid->color_mul->current + grid->type_mul->current;
}


bool WorldGetShapeMoves(int y, int x){
    ent_t *shape = world.grid.combos[x][y]->tile->child;

    return shape->control->moves > 0;
}

int WorldCalcMatchCombos(void){

  int matched = 0;
  bool calculated[GRID_WIDTH][GRID_HEIGHT]={0};
  for (int i = 0; i < 2; i++)
  for (int x = 0; x < GRID_WIDTH; x++)
    for (int y = 0; y < GRID_HEIGHT; y++){
      if(!world.grid.matches[i][x][y]){
       continue;
      }
      if(!calculated[x][y])
        matched++;
      
      calculated[x][y] = true;

      StatIncrementValue(world.grid.combos[x][y]->type_mul,true);
      if(world.grid.color_matches[i][x][y])
        StatIncrementValue(world.grid.combos[x][y]->color_mul,true);
    }

  for (int x = 0; x < GRID_WIDTH; x++)
    for (int y = 0; y < GRID_HEIGHT; y++)
      if(calculated[x][y]){
        SetState(world.grid.combos[x][y]->tile->child,STATE_DIE,EntAddPoints);
      }
      else{
        if(world.grid.combos[x][y]->tile->child->control->moved){
          StatEmpty(world.grid.combos[x][y]->type_mul);
          StatEmpty(world.grid.combos[x][y]->color_mul);
        }
        
        SetState(world.grid.combos[x][y]->tile->child,STATE_IDLE,NULL);
        SetState(world.grid.combos[x][y]->tile,STATE_IDLE,NULL);
      }
  int groups = matched /3;
  bool remainder = (matched %3)>0;

  if(remainder)
    groups++;
  
  return groups;  
}

Cell WorldGetMaxShapes(){
  return (Cell){(int)world.max_shape->current,(int)world.max_color->current};
}

int WorldGetShapeSums(int* out){
  for (int x = 0; x < GRID_WIDTH; x++){
    for (int y = 0; y < GRID_HEIGHT; y++){
      ent_t* t = world.grid.combos[x][y]->tile;
      if(!t->child)
        continue;
      ShapeFlags s = SHAPE_TYPE(t->child->shape);
      out[s]++;
    }
  }

  return (int)world.max_shape->current;
}

void WorldClearMatches(void){

  for (int i = 0; i < 2; i++){
    for(int x = 0; x < GRID_WIDTH;x++){
      for (int y = 0; y < GRID_HEIGHT; y++){
        world.grid.matches[i][x][y]=NULL;
        world.grid.color_matches[i][x][y]=false;
        if(world.grid.combos[x][y]->tile->child)
          world.grid.combos[x][y]->tile->child->control->moved = false;
        
        world.grid.combos[x][y]->type_mul->increment=0.125f*world.max_shape->current;
        world.grid.combos[x][y]->color_mul->increment=0.25f*(((int)world.max_color->current)>>4);

      }
    }
  }
}

int WorldGetMatches(void){

  int matches = 0;
  for (int i = 0; i < 2; i++){
    for(int x = 0; x < GRID_WIDTH;x++){
      for (int y = 0; y < GRID_HEIGHT; y++){
        if(world.grid.matches[i][x][y]==NULL)
          continue;

        matches++;
      }
    }
  }
      
  return matches;
}

bool TurnSetState(TurnState state){
  if(!TurnCanChangeState(state))
    return false;

  world.grid.state = state;

  TraceLog(LOG_INFO,"Turn State now - %s",turn_name[state].name);
  TurnOnChangeState(state);
  return true;
}

bool TurnCanChangeState(TurnState state){
  if(state == world.grid.state)
    return false;

  state_change_requirement_t *req = &turn_reqs[state];
  return req->can(world.grid.state, req->required);
}

bool WorldCheckNewGrid(void){
  ShapeID grid[GRID_HEIGHT][GRID_WIDTH];
  int moves[GRID_HEIGHT][GRID_WIDTH];

  for(int x = 0; x < GRID_WIDTH; x++)
    for (int y = 0 ; y < GRID_HEIGHT; y++){
      grid[x][y] = world.grid.combos[x][y]->tile->child->shape;
      moves[x][y] = world.grid.combos[x][y]->tile->child->control->moves;
    }
  return CanBeSolvedInMoves(grid,moves,4);
}

void TurnOnChangeState(TurnState state){
  switch(state){
    case TURN_START:
      if(WorldCheckNewGrid())
        TurnSetState(TURN_INPUT);
      else
        MenuSetState(&ui.menus[MENU_EXIT],MENU_ACTIVE);
      break;
    case TURN_SCORE:
      int turn_matches = WorldCalcMatchCombos();
      if(turn_matches > 0)
        world.combo_streak+=turn_matches;
      else
        world.combo_streak =0;
      TurnSetState(TURN_END);
      break;
    case TURN_END:
      world.grid.turn++;
      if(world.grid.turn%13==0)
        StatIncrementValue(world.max_color,true);
      if (world.grid.turn%27==0)
        StatIncrementValue(world.max_shape,true);
      TurnSetState(TURN_STANDBY);
      break;
    case TURN_STANDBY:
      WorldClearMatches();
      break;
    default:
      break;
  }
}

TurnState TurnGetState(void){
  return world.grid.state;
}

bool CheckWorldGridAdjacent(ent_t* e, ent_t* other){

  return is_adjacent(e->intgrid_pos,other->intgrid_pos);
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

ObjectInstance GetObjectInstanceByShapeID(ShapeID id){
  for (int i = SHAPE_NONE; i<SHAPE_COUNT;i++){
    if(room_instances[i].id != id)
      continue;

    return room_instances[i];
  }
  
  return room_instances[0];
}

bool RegisterEnt( ent_t *e){
  e->uid = AddEnt(e);

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

  TurnSetState(TURN_INPUT);
}

void WorldPreUpdate(){
  if(TurnGetState()==TURN_STANDBY){
    ent_t* tile_pool[GRID_WIDTH*GRID_HEIGHT];
    int num_empty = WorldGetEnts(tile_pool,FilterEmptyTile,NULL);
    if (num_empty == 0)
      TurnSetState(TURN_START);
  }

  InteractionStep();
  
  for(int i = 0; i < world.num_spr; i++){
    SpriteSync(world.sprs[i]);
  }
}

void WorldFixedUpdate(){
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

void InitWorld(world_data_t data){
  world = (world_t){0};
  world.combo_streak = 0;
  world.combo_mul=InitStatOnMin(STAT_COMBO_MUL,1.0f,16.0F);
  world.combo_mul->increment = 0.25f;
  world.max_shape = InitStatOnMin(STAT_MAX_TYPE,SHAPE_TYPE_STUD,SHAPE_TYPE_MAX);
  world.max_color = InitStatOnMin(STAT_MAX_COLOR,SHAPE_COLOR_GRAY,SHAPE_COLOR_MAX);

  world.max_color->increment = 16;

  for(int y = 0; y < GRID_HEIGHT; y++)
    for(int x = 0; x < GRID_WIDTH; x++){
      //Vector2 pos = {x*cs + gridStart.x,y*cs+gridStart.y};
      ent_t* tile = InitEntStatic(BASE_TILE,VECTOR2_ZERO);
      tile->intgrid_pos = (Cell){x,y};
      ElementAddGameElement(tile);
      RegisterEnt(tile);
      world.grid.combos[x][y] = malloc(sizeof(grid_combo_t));
      world.grid.combos[x][y]->tile = tile;
      world.grid.combos[x][y]->type_mul = InitStatOnMin(STAT_TYPE_MUL,1.0f,10.0f);
      world.grid.combos[x][y]->color_mul = InitStatOnMin(STAT_COLOR_MUL,1.0f,10.0f);
      world.grid.combos[x][y]->type_mul->increment=0.25f;
      world.grid.combos[x][y]->color_mul->increment=0.5f;
    }

  for (int i = 0; i < data.num_ents; i++)
    RegisterEnt(InitEnt(data.ents[i]));
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

  game_process.next[SCREEN_TITLE] = SCREEN_GAMEPLAY;
  game_process.init[SCREEN_TITLE] = InitTitleScreen;
  game_process.finish[SCREEN_TITLE] = UnloadTitleScreen;
  game_process.update_steps[SCREEN_TITLE][UPDATE_DRAW] = DrawTitleScreen;
  game_process.update_steps[SCREEN_TITLE][UPDATE_FRAME] = UpdateTitleScreen;

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
  MenuSetState(&ui.menus[MENU_PAUSE],MENU_READY);
  MenuSetState(&ui.menus[MENU_PLAY_AREA],MENU_READY);
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

void AddPoints(float mul,float points, Vector2 pos){
  //TraceLog(LOG_INFO,"===Add %0.2f Points===",points*mul);
  mul+=world.combo_mul->current;
  world.points+=mul*points;

  render_text_t *rt = malloc(sizeof(render_text_t));
  *rt = (render_text_t){
    .text = strdup(TextFormat("+%d",(int)(points*mul))),
    .pos = pos,
    .size = 54,
    .color =YELLOW,
    .duration = (int)(45+mul*9)
  };
  AddFloatingText(rt);
  UploadScore();
}

ShapeFlags WorldGetPossibleShape(){
  return world.max_shape->current;
}

const char* GetGameTime(){
  return TextFormat("%09i",(int)(game_process.game_frames/fixedFPS));
}

const char* GetPoints(){
  return TextFormat("%08i",GetPointsInt());
}

const char* GetTurn(){
  return turn_name[world.grid.state].name;
}

const char* GetComboStreak(){
  return TextFormat("%02i",GetComboInt());
}

int GetComboInt(){
  return world.combo_streak;

}
int GetPointsInt(){
  return (int)world.points;
}
