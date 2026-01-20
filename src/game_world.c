#include <stdio.h>
#include <unistd.h>
#include "game_process.h"
#include "game_tools.h"
#include "game_utils.h"
#include "game_ui.h"
#include "game_info.h"

game_process_t game_process;
TreeCacheEntry tree_cache[18] = {0};
int tree_cache_count = 0;
ent_t* player = {0};
static world_t world;

void WorldEvent(EventType type, void* data, uint64_t uid){
  event_t event = {
    .type = type,
    .data = data,
    .iuid = uid
  };

  if(world.bus->count)
    EventEmit(world.bus, &event);
}

void WorldTargetSubscribe(EventType event, EventCallback cb, void* data, uint64_t iid){

  event_sub_t* sub = EventSubscribe(world.bus, event, cb, data);
  sub->uid = iid;

}
void WorldSubscribe(EventType event, EventCallback cb, void* data){

  EventSubscribe(world.bus, event, cb, data);
}

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

void GameReady(void){
  WorldInitOnce();
  game_process.state[SCREEN_GAMEPLAY] = GAME_READY;
}

void WorldTestPrint(){
  stat_sheet_t* sb = calloc(1,sizeof(stat_sheet_t));
  element_value_t* header[2];
  int title_len = EntGetNamePretty(header, player);
  
  sb->ln[sb->lines++] = InfoInitLineItem(header,title_len, "%s %s");
  
  element_value_t* base[2];
  int items = EntGetStatPretty(base, player->stats[STAT_HEALTH]);
  sb->ln[sb->lines++] = InfoInitLineItem(base,items, "%s: %s");

  items = EntGetStatPretty(base, player->stats[STAT_ARMOR]);

  sb->ln[sb->lines++] = InfoInitLineItem(base,items, "%s: %s");

  for(int i = 0; i < sb->lines; i++){
    PrintSyncLine(sb->ln[i],FETCH_ONCE);
    TraceLog(LOG_INFO,"%s\n",TextFormatLineItem(sb->ln[i]));
  }
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

item_def_t* GetItemDefByID(GearID id){
  for (int i = 0; i < world.items->size; i++){
    if(world.items->pool[i]->id == id)
      return world.items->pool[i];
  }

  return NULL;
}

map_grid_t* WorldGetMap(void){
  return world.map;
}

int WorldGetEntSprites(sprite_t** pool){
  int count = 0;
  for(int i = 0; i < world.num_spr; i++){
    if(world.sprs[i] == NULL)
      continue;

    if(world.sprs[i]->owner)
      pool[count++] = world.sprs[i];
  }

  return count;
}

ent_t* WorldGetEntAtTile(Cell tile){
  map_cell_t* t = MapGetTile(world.map,tile);
  if(t)
    return t->occupant;
  
  return NULL;
}

map_cell_t* WorldGetTile(Cell pos){
  return MapGetTile(world.map,pos);
}

Cell GetWorldCoordsFromIntGrid(Cell pos, float len){

  int start_x = CLAMP(pos.x - len,1,GRID_WIDTH);
  int start_y = CLAMP(pos.y - len,1,GRID_HEIGHT);

  int end_x = CLAMP(pos.x + len,1,GRID_WIDTH);
  int end_y = CLAMP(pos.y + len,1,GRID_HEIGHT);


 Cell candidates[GRID_WIDTH * GRID_HEIGHT];
 int count = 0;
  for (int x = start_x; x < end_x; x++)
    for(int y = start_y; y < end_y; y++){
      if(world.map->tiles[x][y].status >= TILE_ISSUES)
        continue;
      if(distance(pos.x,pos.y,x,y) > len)
        continue;

      candidates[count++] = (Cell){x,y};
    }

  if (count == 0)
    return CELL_UNSET;

  int r = rand() % count;
  return candidates[r];
}


int WorldGetEnts(ent_t** results,EntFilterFn fn, param_t* ctx){
  int num_results = 0;
  for(int i = 0; i < world.num_ent; i++){
    if(!world.ents[i])
      continue;

    if(!fn(world.ents[i], ctx))
      continue;

    results[num_results++] = world.ents[i];
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

env_t* WorldGetEnvById(unsigned int uid){
  for(int i = 0; i < world.num_env; i++){
    if(world.envs[i]->uid == uid)
      return world.envs[i];
  }

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

int AddEnv(env_t *e){
  if(world.num_env < MAX_ENVS){
    int index = world.num_env;
    world.envs[index] = e;
    world.num_env++;

    return index;
  }
  return -1;
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

bool RegisterEnv( env_t *e){
 int index = AddEnv(e);
 e->uid = e->type;
 TileStatus status = MapSetTile(world.map,e,e->pos);
  if(status > TILE_ISSUES)
    TraceLog(LOG_WARNING,"Issue %i at tile %i,%i ",status,e->pos.x,e->pos.y);

  if(e->sprite)
    e->sprite->is_visible = true;

 e->status = ENV_STATUS_NORMAL; 
 if(e->uid < 0)
  return false;

 game_object_uid_i guid = GameObjectMakeUID("OBJ_ENVIRO", index, world.ctx->time->current);

 assert(guid != UID_INVALID); 

 e->gouid = guid;

 param_t p = ParamMake(DATA_ENV, 0, e);
 p.gouid = e->gouid;
 Cell c = CELL_NEW(world.map->width/2, world.map->height/2);
 MakeLocalContext(world.ctx->tables[OBJ_ENV], &p, c);
 local_ctx_t* owner = LocalGetEntry(world.ctx->tables[OBJ_MAP_CELL], e->map_cell->gouid);

 param_t m = ParamMake(DATA_MAP_CELL, 0, e->map_cell);
 m.gouid = e->map_cell->gouid;
 local_ctx_t* res = MakeLocalContext(world.ctx->tables[OBJ_MAP_CELL], &m, c);

 if(res)
   e->map_cell->in_ctx = true;

 world.map->updates = true;
 return true;
}

bool RegisterEnt( ent_t *e){
  e->uid = AddEnt(e);

  TileStatus status = MapSetOccupant(world.map,e,e->pos);
  if(status > TILE_ISSUES)
    TraceLog(LOG_WARNING,"Issue %i at tile %i,%i ",status,e->pos.x,e->pos.y);

  if(e->type == ENT_PERSON){
    player = e;
    SkillCapOff(player->skills[SKILL_LVL]);
    WorldTestPrint();
    InitInput(player);
  }

  if(e->sprite)
    RegisterSprite(e->sprite);

  if(e->uid < 0)
  return false;

  game_object_uid_i guid = GameObjectMakeUID("OBJ_ENTITY", e->uid, world.ctx->time->current);

  assert(guid != UID_INVALID);

  e->gouid = guid;

  param_t p = ParamMake(DATA_ENTITY, 0, e);
  p.gouid = e->gouid;
  Cell c = CELL_NEW(world.map->width/2, world.map->height/2);
  MakeLocalContext(world.ctx->tables[OBJ_ENT], &p, c);
  return true;

}

map_cell_t* RegisterMapCell(int x, int y){
  map_cell_t* out = calloc(1,sizeof(map_cell_t));
  int index = IntGridIndex(x, y);
  out->index = index;
  out->props = calloc(1,sizeof(site_properties_t));

  const char *cat = "OBJ_MAP_CELL";


  game_object_uid_i guid = GameObjectMakeUID(cat, index, world.ctx->time->current);

  assert(guid != UID_INVALID);

  out->gouid = guid;
  out->coords = CELL_NEW(x,y);
  return out;

}

bool RegisterSprite(sprite_t *s){
  s->suid = AddSprite(s);

  return s->suid > -1;
}

bool RegisterItem(ItemInstance g){
  item_def_t* item = DefineItem(g);

  world.items->pool[world.items->size++] = item;
}

void OnWorldCtx(EventType event, void* data, void* user){
  local_table_t* table = user;
  local_ctx_t* ctx = data;
  switch(event){
    case EVENT_ADD_LOCAL_CTX:
      AddLocalFromCtx(table, ctx);
      break;
    case EVENT_ENT_DEATH:
    case EVENT_ENV_DEATH:
      LocalPruneCtx(table, ctx->gouid);
      break;
  }

}

void OnWorldByGOUID(EventType event, void* data, void* user){
  local_table_t* table = user;
  game_object_uid_i* gouid = data;

  LocalPruneCtx(table, *gouid);

}
local_ctx_t* WorldGetContext(DataType type, game_object_uid_i gouid){
  ObjectCategory cat = OBJ_NONE;
  switch(type){
    case DATA_ENTITY:
      cat = OBJ_ENT;
      break;
    case DATA_ENV:
      cat = OBJ_ENV;
      break;
    case DATA_MAP_CELL:
      cat = OBJ_MAP_CELL;
      break;
  }

  if(cat == OBJ_NONE)
    return NULL;

  return LocalGetEntry(world.ctx->tables[cat], gouid);
}
void WorldValidateContext(){
world_context_t* wctx = world.ctx;

  if(wctx==NULL)
    return;

  for (int i = 0; i < OBJ_ALL; i++){
    wctx->tables[i]->valid = true;
  }
}

//void WorldContext
void WorldApplyLocalContext(ent_t* e){
  world_context_t* wctx = world.ctx;

  if(wctx==NULL)
    return;

  for (int i = 0; i < OBJ_ALL; i++){
    local_table_t* t = wctx->tables[i];
    if(!t || t->count == 0)
      continue;

    for (int j = 0; j < t->count; j++)
      AddLocalFromCtx(e->local, &t->entries[j]);
  }
}

void WorldContextInitOnce(void){
  for(int i = 0; i < OBJ_ALL; i++)
    LocalSortByDist(world.ctx->tables[i]);
}

void WorldInitOnce(){
  WorldMapLoaded(world.map);
  //MapRoomSpawn(world.map, ENT_DEER, 5);
  //MapRoomSpawn(world.map, ENT_BEAR, 3);
  //MapRoomSpawn(world.map, ENT_RAT, 3);

  WorldContextInitOnce();
  
  InteractionStep();
  
  for(int i = 0; i< world.num_ent; i++)
    EntInitOnce(world.ents[i]);

  WorldValidateContext();
}

void WorldPreUpdate(){
  InteractionStep();
 
  StepEvents(world.events[STEP_FIXED]);

  for(int i = 0; i < world.num_spr; i++){
    SpriteSync(world.sprs[i]);
  }

  TurnPhase p = ActionManagerPreSync();
    

  for(int i = 0; i < world.num_ent; i++){
    EntControlStep(world.ents[i], world.ctx->turn, p);
  }
  
}

void WorldFixedUpdate(){
  for(int i = 0; i < world.num_ent; i++){
    switch(world.ents[i]->state){
      case STATE_END:
        i-=RemoveEnt(i);
        break;
      case STATE_DIE:
        //EntDestroy(world.ents[i]);
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

void WorldEndTurn(void){
  world.ctx->turn++;
  ActionManagerSync();
  ResetEvent(game_process.events,EVENT_TURN);
}

void WorldTurnUpdate(void* context){
  MapTurnStep(world.map);
  StepEvents(world.events[STEP_TURN]);

  StatIncrementValue(world.ctx->time,true);
  for(int i = 0; i < world.num_ent; i++){
    ent_t* e = world.ents[i];
    if(e->status != ENT_STATUS_ALIVE)
      continue;

    EntTurnSync(e);
  }
  
  MapSync(world.map);
}

world_context_t* InitWorldContext(void){
  world_context_t *ctx = calloc(1,sizeof(world_context_t));

  ctx->time = InitStatOnMin(STAT_TIME,0,180);
  ctx->time->on_stat_full = StatReverse; 
  ctx->time->on_stat_full = StatReverse; 

  ctx->tables[OBJ_ENT] = InitLocals(player, MAX_ENTS);
  ctx->tables[OBJ_ENV] = InitLocals(player, MAX_ENVS);
  ctx->tables[OBJ_MAP_CELL] = InitLocals(player, MAX_ENVS);
  
  return ctx;

}

void PrepareWorldRegistry(void){
  world = (world_t){0};

  world.bus = InitEventBus(256);
  for(int i = 0; i < STEP_DONE; i++){
    world.events[i] = InitEvents();
  }

  InitActionManager();

  world.ctx = InitWorldContext();

  world.items = InitItemPool(); 

  for (int i = 0; i < GEAR_DONE; i++){
    if(room_items[i].id== GEAR_DONE)
      break;

    RegisterItem(room_items[i]);
  }

  int f_count = ARRAY_COUNT(FACTION_DEFS);
  for(int i = 0; i < f_count; i++){
    RegisterFaction(FACTION_DEFS[i].name);
    FACTIONS[i]->species = FACTION_DEFS[i].species;
    FACTIONS[i]->rules = FACTION_DEFS[i].rules;
    for(int j = 0; j < ENT_DONE; j++)
      FACTIONS[i]->member_ratio[j] = FACTION_DEFS[i].member_ratio[j];

    for(int j = 0; j < BIO_DONE; j++){
      if(FACTION_DEFS[i].bio_pref[j] == 0)
        continue; 
      FACTIONS[i]->bio_pref[j] = FACTION_DEFS[i].bio_pref[j]; 
    }
  }
}

void InitWorld(void){
  if(MapGetStatus()==GEN_DONE){
    world.map =  InitMapGrid();
    Cell player_pos = MapApplyContext(world.map);
    
    if(!cell_compare(player_pos,CELL_UNSET))
      RegisterEnt(InitEnt(ENT_PERSON,player_pos));
    GameReady(); 
    ScreenCameraSetBounds(CELL_NEW(world.map->width,world.map->height));

    world.level = InitLevel(world.map);
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
  ClearBackground(world.map->floor);

  for(int i = 0; i < world.num_env; i++)
    EnvRender(world.envs[i]);


  for(int i = 0; i < world.num_spr;i++)
    if(world.sprs[i]->state < ANIM_KILL)
      if(world.sprs[i]->owner)
        DrawSprite(world.sprs[i]);
      else
        i-=RemoveSprite(i);

  ScreenRender(); 
}

void InitGameProcess(){
  for(int i = 0; i < BN_COUNT; i++){
    if(room_behaviors[i].id == BN_COUNT)
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
  game_process.prep[SCREEN_OPTIONS] = PrepareWorldRegistry;
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
  InitWorld();
  game_process.children[SCREEN_GAMEPLAY].process = PROCESS_LEVEL;
  game_process.game_frames = 0;

  cooldown_t* turnEvent = InitCooldown(10,EVENT_TURN,WorldTurnUpdate,NULL);
  turnEvent->is_recycled = true;
  AddEvent(game_process.events,turnEvent);

}

bool WorldAddEvent(event_uid_i eid, cooldown_t* cd, StepType when){

  return AddEvent(world.events[when], cd) > 0;
}

bool GameTransitionScreen(){
  GameScreen current = game_process.screen;
  GameScreen prepare = game_process.next[current];
  if(game_process.state[current] >= GAME_FINISHED)
    return false;
  if(game_process.prep[prepare])
    game_process.prep[prepare]();
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

const char* GetWorldTime(){
  int max = world.ctx->time->max;
  int cur = world.ctx->time->current;

  int time_world_speed = max + cur;

  int hour = (int)(time_world_speed / 15);
  int minute = (int)((time_world_speed%15)*4);

  return TextFormat("%i : %02d", hour, minute);
}

const char* GetGameTime(){
  return TextFormat("%09i",(int)(game_process.game_frames/fixedFPS));
}

int WorldGetTurn(void){
  return world.ctx->turn;
}

bool WorldDebugInsert(debug_info_t d){
  for(int i = 0; i < MAX_DEBUG_ITEMS; i++){
    if(world.debug[d.type][i].type != DEBUG_NONE)
      continue;

    world.debug[d.type][i] = d;
  }
}

void WorldDebugCell(Cell c, Color col){
  return;
  param_t* info = calloc(1, sizeof(param_t));
  *info = ParamMake(DATA_CELL, sizeof(Cell), &c);

  debug_info_t d = (debug_info_t){
    .type = DEBUG_CELL,
    .color = col,
    .info = info
  };

  WorldDebugInsert(d);
}
