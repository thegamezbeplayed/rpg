#ifndef __GAME_PROCESS__
#define __GAME_PROCESS__
#ifdef __GNUC__
extern void moncontrol(int);
#endif
#include <unistd.h>
#include <pthread.h>
#include "game_types.h"
#include "game_common.h"
#include "screens.h"
#include "room_data.h"

#define MAX_SUBS 1024
#define MAX_INTERACTIONS 2048
#define DEBUG false
#define TURN WorldGetTurn()

static int WORLD_TICK;
static int CURRENT_ENT_IDENTIFIER = 0;
extern ent_t* player;

extern Font font;
static int fixedFPS = 60;

typedef void (*UpdateFn)(void);
typedef bool EntFilterFn(ent_t* e, param_t* ctx); 

static bool FilterEntOtherTeam(ent_t* e, param_t* ctx){
  if(!ctx || ctx->type_id != DATA_INT)
    return false;

  ent_t* other = ParamReadEnt(ctx);

  int team = other->team;

  return(e->team != team);
}

static bool FilterEntByTeam(ent_t* e, param_t* ctx){
  if(!ctx || ctx->type_id != DATA_INT)
    return false;

  ent_t* other = ParamReadEnt(ctx);

  int team = other->team;

  return (e->team == team);
}

static bool FilterEntByStatus(ent_t* e,  param_t* ctx){
  if(!ctx || ctx->type_id != DATA_INT)
    return false;

  EntityStatus status = ParamReadInt(ctx);

  return (e->status == status);
}

static bool FilterEntByState(ent_t* e,  param_t* ctx){
  if(!ctx || ctx->type_id != DATA_INT)
    return false;

  EntityState state = ParamReadInt(ctx);

  return (e->state != state);
}

//INTERACTIONS_T===>
typedef struct interaction_s interaction_t;
typedef uint64_t interaction_uid_i;
typedef interaction_uid_i (*InteractionCB)(interaction_t* self, void* ctx, param_t payload);

struct interaction_s {
  interaction_uid_i uid;
  EventType         event;
  InteractResult    result;
  void*             ctx;
  int               last_update_turn;
  cooldown_t*       timer;
  bool              refresh_on_update;
  InteractionCB     on_update, on_add;
};

static inline interaction_uid_i InteractionMakeUID(EventType type, uint16_t context_id,
    uint64_t source_id,
    uint64_t target_id){
  uint64_t who = hash_combine_64(source_id, target_id);

  return hash_combine_64(who, context_id);  
}

int InitInteractions();
interaction_t* RegisterInteraction(uint64_t source, uint64_t target, EventType event, int duration, void* ctx, uint16_t ctx_id);
interaction_t* StartInteraction(uint64_t source, uint64_t target, EventType event,
    int duration, void* ctx, uint16_t ctx_id, param_t data, InteractionCB add, InteractionCB update, bool refresh);
int InteractionExists(interaction_uid_i uid);
interaction_t* GetInteractionByUID(interaction_uid_i uid);
bool AddInteraction(interaction_t* inter);
bool CanInteract(int source, int target);
int GetInteractions(int source);
void FreeInteraction(interaction_t* item);
void FreeInteractionByIndex(int i);
void FreeInteractions();
void InteractionStep();
//==INTERACTIONS_T==>
interaction_uid_i RegisterSkillEvent(interaction_t* self, void* ctx, param_t payload);
interaction_uid_i UpdateSkillEvent(interaction_t* self, void* ctx, param_t payload);
typedef uint64_t event_uid_i;

typedef void (*EventCallback)(
    EventType  event,
    void*      event_data,
    void*      user_data
);

typedef struct {
    EventType      event;
    EventCallback  cb;
    uint64_t       uid;
    void*          user_data;
} event_sub_t;

typedef struct {
    event_sub_t* subs;
    int count, cap;
} event_bus_t;

typedef struct{
  EventType   type;
  EventStatus status;
  void*       data;
  uint64_t    iuid;
}event_t;

event_bus_t* InitEventBus(int cap);
event_sub_t* EventSubscribe(event_bus_t* bus, EventType event, EventCallback cb, void* u_data);
void EventEmit(event_bus_t* bus, event_t*);
void EventRemove(event_bus_t* bus, uint64_t id);
static inline event_uid_i EventMakeUID(EventType type, uint64_t data_id){
  event_uid_i euid = hash_combine_64(type, data_id);

  return euid;
}
void OnWorldByGOUID(EventType event, void* data, void* user);
void OnWorldCtx(EventType, void*, void*);
//EVENTS==>

typedef enum{
  PROCESS_NONE = -1,
  PROCESS_LEVEL,
  PROCESS_DONE
}ProcessType;

typedef enum{
  UPDATE_FRAME,//update running at full fps
  UPDATE_DRAW,
  UPDATE_PRE,
  UPDATE_FIXED,
  UPDATE_POST,
  UPDATE_DONE
}UpdateType;

typedef enum{
  GAME_NONE,
  GAME_LOADING,
  GAME_READY,
  GAME_PAUSE,
  GAME_FINISHED,
  GAME_OVER
}GameState;

void GameSetState(GameState state);

typedef struct{
  ProcessType process;
  GameState   state[PROCESS_DONE];
  events_t    *events;
  UpdateFn    init[PROCESS_DONE];
  UpdateFn    update_steps[PROCESS_DONE][UPDATE_DONE];
  UpdateFn    finish[PROCESS_DONE];
}child_process_t;

typedef struct{
  path_pool_t*     paths;
  int              num_factions;
  faction_groups_t *factions[MAX_FACTIONS];
}level_t;
level_t* InitLevel(map_grid_t*);
void LevelBury(game_object_uid_i gouid);
void LevelReady(map_grid_t* m);

typedef struct{
  GameScreen     screen;
  level_t*       level;
  int            game_frames;
  child_process_t children[SCREEN_DONE];
  GameScreen     next[SCREEN_DONE];
  GameState      state[SCREEN_DONE];//TODO each screen needs a state
  events_t       *events;
  int            album_id[SCREEN_DONE];
  UpdateFn       init[SCREEN_DONE];
  UpdateFn       prep[SCREEN_DONE];
  UpdateFn       update_steps[SCREEN_DONE][UPDATE_DONE];
  UpdateFn       finish[SCREEN_DONE];
}game_process_t;
extern game_process_t game_process;

void InitGameEvents();
void InitGameProcess();
void GameProcessStep();
void GameProcessSync(bool wait);
bool GameTransitionScreen();
void GameProcessEnd();

void AddPoints(float mul,float points,Vector2 pos);
//===WORLD_T===>

typedef struct{
  int             turn;
  stat_t          *time;
  bool            updates;
  local_table_t*  tables[OBJ_ALL];
}world_context_t;
void WorldApplyLocalContext(ent_t* e);
local_ctx_t* WorldGetContext(DataType type, game_object_uid_i gouid);
void WorldContextChange(ObjectCategory cat, game_object_uid_i gouid);

typedef struct world_s{
  map_grid_t    *map;
  level_t*      level;
  ent_t*        ents[MAX_ENTS];
  unsigned int  num_ent;
  sprite_t*     sprs[MAX_ENTS];
  unsigned int  num_spr;
  item_pool_t   *items;
  unsigned int  num_env;
  env_t*        envs[MAX_ENVS];
  render_text_t *texts[MAX_EVENTS];
  bool          floatytext_used[MAX_EVENTS];
  event_bus_t*  bus;
  events_t      *events[STEP_DONE];
  debug_info_t  debug[DEBUG_ALL][MAX_DEBUG_ITEMS];
  world_context_t  *ctx;
} world_t;
extern world_t world;

void PrepareWorldRegistry(void);
ent_t* WorldGetEnt(const char* name);
ent_t* WorldGetEntById(unsigned int uid);
ent_t* WorldPlayer(void);
env_t* WorldGetEnvById(unsigned int uid);
void WorldSubscribe(EventType, EventCallback, void*);
void WorldTargetSubscribe(EventType event, EventCallback cb, void* data, uint64_t iid);
void WorldEvent(EventType, void*, uint64_t);
static void WorldUnsub(uint64_t id){
  EventRemove(world.bus, id);
}
int WorldGetEntSprites(sprite_t** pool);
Cell GetWorldCoordsFromIntGrid(Cell pos, float len);
ent_t* WorldGetEntAtTile(Cell tile);
map_cell_t* WorldGetTile(Cell pos);
map_grid_t* WorldGetMap(void);
static level_t* WorldLevel(void){
  return world.level;
}
int WorldGetEnts(ent_t** results,EntFilterFn fn, param_t* ctx);
bool WorldGetTurnState(void);
bool WorldAddEvent(event_uid_i eid, cooldown_t* cd, StepType when);
bool RegisterBehaviorTree(BehaviorData data);
bool RegisterEnt( ent_t *e);
bool RegisterEnv( env_t *e);
map_cell_t* RegisterMapCell(int x, int y);
bool RegisterSprite(sprite_t *s);
bool RegisterItem(ItemInstance g);
void WorldInitOnce();
void WorldPreUpdate();
void WorldFixedUpdate();
void WorldPostUpdate();
void WorldEndTurn(void);
void WorldTurnUpdate(void* context);
void InitWorld(void);
void WorldRender();
Rectangle WorldRoomBounds();
static int WorldGetTime(){
  return game_process.game_frames;
} 
  

const char* GetWorldTime();
int WorldGetTurn(void);
void WorldDebugShape(Rectangle r); 
void WorldDebugCell(Cell c, Color col);
#endif

