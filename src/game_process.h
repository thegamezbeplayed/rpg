#ifndef __GAME_PROCESS__
#define __GAME_PROCESS__

#include <unistd.h>
#include <pthread.h>
#include "game_types.h"
#include "game_common.h"
#include "screens.h"
#include "room_data.h"

#define MAX_INTERACTIONS 256
#define DEBUG false
#define TURN WorldGetTurn()

static int WORLD_TICK;
static int CURRENT_ENT_IDENTIFIER = 0;
extern ent_t* player;

extern Font font;
static int fixedFPS = 60;

typedef void (*UpdateFn)(void);
typedef bool EntFilterFn(ent_t* e, void* params); 

static bool FilterEntInRect(ent_t* e, void* params){
  Rectangle* rect = params;

  if(cell_in_rect(e->pos, *rect))
    return e;

  return NULL;
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
  cooldown_t*       timer;
  bool              refresh_on_update;
  InteractionCB     on_update, on_add;
};

static inline interaction_uid_i InteractionMakeUID(EventType type, uint16_t context_id,
    uint16_t source_id,
    uint16_t target_id){
  return ((uint64_t)(type       & 0xFFFF) << 48) |
    ((uint64_t)(context_id & 0xFFFF) << 32) |
    ((uint64_t)(source_id  & 0xFFFF) << 16) |
    ((uint64_t)(target_id  & 0xFFFF));  
}

int InitInteractions();
interaction_t* RegisterInteraction(uint16_t source, uint16_t target, EventType event, int duration, void* ctx, uint16_t ctx_id);
interaction_t* StartInteraction(uint16_t source, uint16_t target, EventType event,
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
  GameScreen     screen;
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

void InitGameEvents();
void InitGameProcess();
void GameProcessStep();
void GameProcessSync(bool wait);
bool GameTransitionScreen();
void GameProcessEnd();

void AddPoints(float mul,float points,Vector2 pos);
//===WORLD_T===>

typedef struct{
  int     num_turn;
}world_data_t;

world_data_t* InitWorldData(void);

typedef struct world_s{
  map_grid_t    *map;
  ent_t*        ents[MAX_ENTS];
  unsigned int  num_ent;
  sprite_t*     sprs[MAX_ENTS];
  unsigned int  num_spr;
  item_pool_t   *items;
  unsigned int  num_env;
  env_t*        envs[MAX_ENVS];
  render_text_t *texts[MAX_EVENTS];
  bool          floatytext_used[MAX_EVENTS];
  events_t      *events[STEP_DONE]; 
  world_data_t  *data;
  stat_t        *time;
} world_t;

void PrepareWorldRegistry(void);
ent_t* WorldGetEnt(const char* name);
ent_t* WorldGetEntById(unsigned int uid);
ent_t* WorldPlayer(void);
env_t* WorldGetEnvById(unsigned int uid);
int WorldGetEntSprites(sprite_t** pool);
Cell GetWorldCoordsFromIntGrid(Cell pos, float len);
ent_t* WorldGetEntAtTile(Cell tile);
map_cell_t* WorldGetTile(Cell pos);
map_grid_t* WorldGetMap(void);
int WorldGetEnts(ent_t** results,EntFilterFn fn, void* params);
bool WorldGetTurnState(void);
bool WorldAddEvent(event_uid_i eid, cooldown_t* cd, StepType when);
bool RegisterBehaviorTree(BehaviorData data);
bool RegisterEnt( ent_t *e);
bool RegisterEnv( env_t *e);
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
const char* GetWorldTime();
int WorldGetTurn(void);
#endif

