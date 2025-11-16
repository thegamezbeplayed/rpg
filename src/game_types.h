#ifndef __GAME_TYPES__
#define __GAME_TYPES__

#include "game_common.h"
#include "game_assets.h"
#include "game_utils.h"
#include "raylib.h"
#include "game_math.h"

#define MAX_ENTS 128  

typedef enum{
  RANGE_AGGRO,
  RANGE_LOITER,
  RANGE_NEAR,
  RANGE_EMPTY 
}RangeType;   

typedef struct{
  stat_t*     stats[STAT_ATTACK_DONE-STAT_ATTACK_STATS];
}attack_t;

typedef struct{
  struct ent_s*           target;
  Cell                    destination;
  bool                    has_arrived;
  int                     ranges[RANGE_EMPTY];
  behavior_tree_node_t*   bt[STATE_END];
}controller_t;
controller_t* InitController();

//===ENT_T===>
typedef struct ent_s{
  int                   uid;
  stat_t*               stats[STAT_DONE];
  attack_t*             attack;
  EntityType            type;
  Cell                  pos;
  EntityState           state,previous;
  action_turn_t         *actions[ACTION_DONE];
  controller_t          *control;
  events_t              *events;
  sprite_t              *sprite;
} ent_t;

ent_t* InitEnt(ObjectInstance data);
ent_t* InitEntStatic(TileInstance data,Vector2 pos);

void EntToggleTooltip(ent_t* e);
void EntInitOnce(ent_t* e);
attack_t* InitBasicAttack(void);
attack_t* EntGetCurrentAttack(ent_t* e);
void EntSync(ent_t* e);
bool EntKill(ent_t* e);
void EntDestroy(ent_t* e);
bool FreeEnt(ent_t* e);
void EntPrepStep(ent_t *e);
void EntGridStep(ent_t *e, Cell step);
void EntSetCell(ent_t *e, Cell pos);
void EntAddPos(ent_t *e, Vector2 pos);
void EntSetPos(ent_t *e, Vector2 pos);
void EntControlStep(ent_t *e);
typedef void (*StateChangeCallback)(ent_t *e, EntityState old, EntityState s);
void SetViableTile(ent_t*, EntityState old, EntityState s);
bool CheckEntAvailable(ent_t* e);
bool CheckEntPosition(ent_t* e, Vector2 pos);
bool SetState(ent_t *e, EntityState s,StateChangeCallback callback);
void StepState(ent_t *e);
void OnStateChange(ent_t *e, EntityState old, EntityState s);
bool CanChangeState(EntityState old, EntityState s);

bool EntCanTakeAction(ent_t* e, ActionType a);
void InitActions(action_turn_t* actions[ACTION_DONE]);
action_turn_t* InitAction(ActionType t, TakeActionCallback fn, OnActionCallback cb);
bool ActionMove(ent_t*, ActionType a, KeyboardKey k);
void ActionSync(ent_t* e);
bool ActionInput(void);
bool SetAction(ent_t* e, ActionType a, void* context);
bool ActionTaken(ent_t* e, ActionType a);
bool TakeAction(ent_t* e, action_turn_t* action);
bool ActionTraverseGrid(ent_t* e,  ActionType a, OnActionCallback cb);

static action_key_t action_keys[ACTION_DONE] = {
  {ACTION_NONE},
  {ACTION_MOVE,8,{KEY_D,KEY_A,KEY_W,KEY_S,KEY_LEFT, KEY_RIGHT,KEY_UP,KEY_DOWN},ActionMove},
};
#endif
