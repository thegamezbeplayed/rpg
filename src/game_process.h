#ifndef __GAME_PROCESS__
#define __GAME_PROCESS__

#include "game_types.h"
#include "game_common.h"
#include "screens.h"
#include "room_data.h"

#define MAX_INTERACTIONS 256
#define DEBUG false
static int CURRENT_ENT_IDENTIFIER = 0;
extern ent_t* player;

extern Font font;
static int fixedFPS = 60;

typedef void (*UpdateFn)(void);
typedef bool EntFilterFn(ent_t* e, ent_t* other); 

//INTERACTIONS_T===>
typedef struct {
  int             source_uid; //uid of who interacted (body, ent)
  int             target_uid; //uid of interactee (body, ent)
  cooldown_t*     timer;
} interaction_t;

int InitInteractions();
interaction_t* EntInteraction(unsigned int source, unsigned int target, int duration);
bool AddInteraction(interaction_t* inter);
bool CanInteract(int source, int target);
int GetInteractions(int source);
void FreeInteraction(interaction_t* item);
void FreeInteractionByIndex(int i);
void FreeInteractions();
void InteractionStep();
//==INTERACTIONS_T==>
//EVENTS==>

typedef enum{
  PROCESS_NONE,
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
  ObjectInstance  ents[1];
  unsigned int    num_ents;
}world_data_t;

typedef struct world_s{
  map_grid_t    *map;
  ent_t*        ents[MAX_ENTS];
  unsigned int  num_ent;
  sprite_t*     sprs[MAX_ENTS];
  unsigned int  num_spr;
  item_pool_t   *items;
  unsigned int  num_env;
  env_t*        envs[MAX_ENTS];
  render_text_t *texts[MAX_EVENTS];
  bool          floatytext_used[MAX_EVENTS];
} world_t;

ent_t* WorldGetEnt(const char* name);
ent_t* WorldGetEntById(unsigned int uid);
ent_t* WorldPlayer(void);
env_t* WorldGetEnvById(unsigned int uid);
Cell GetWorldCoordsFromIntGrid(Cell pos, float len);
ent_t* WorldGetEntAtTile(Cell tile);
map_grid_t* WorldGetMap(void);
int WorldGetEnts(ent_t** results,EntFilterFn fn, void* params);
bool WorldGetTurnState(void);
bool RegisterBehaviorTree(BehaviorData data);
bool RegisterEnt( ent_t *e);
bool RegisterEnv( env_t *e);
bool RegisterSprite(sprite_t *s);
bool RegisterItem(ItemInstance g);
void WorldInitOnce();
void WorldPreUpdate();
void WorldFixedUpdate();
void WorldPostUpdate();
void WorldTurnUpdate(void* context);
void InitWorld(world_data_t data);
void WorldRender();
Rectangle WorldRoomBounds();
#endif

