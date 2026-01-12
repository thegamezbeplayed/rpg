#ifndef __GAME_CTRL__
#define __GAME_CTRL__

#define MAX_ACTIONS 16
#define MAX_PHASE_ACTIONS 128

typedef struct ability_s ability_t;

typedef bool (*OnActionCallback)(struct ent_s* e, ActionType a);
typedef bool (*TakeActionCallback)(struct ent_s* e, ActionType a, OnActionCallback cb);

typedef struct{
  DesignationType   type;
  union {
    Cell* tile;
    struct ent_s* mob;
  } target; 
}action_target_t;

typedef struct{
  bool                on_deck;
  ActionType          action;
  DesignationType     targeting;
  void*               context;
  int                 num_targets;
  action_target_t*    targets[5];
  TakeActionCallback  fn;
  OnActionCallback    cb;
}action_turn_t;

typedef bool (*ActionKeyCallback)(struct ent_s* e, ActionType a, KeyboardKey k,ActionSlot slot);
  
typedef struct{
  ActionType        action;
  int               num_keys;
  KeyboardKey       keys[8];
  ActionKeyCallback fn;
  int               binding;
}action_key_t;

typedef struct{
  ActionSlot     id;
  ActionType     allowed[ACTION_SLOTTED];
  ent_t*         owner;
  StatType       resource;
  int            count, cap, size, space, rank, pref;
  ability_t      **abilities;
}action_slot_t;

typedef struct{
  uint64_t        id;
  ActionType      type;
  ActionCategory  cat;
  ActionStatus    status;
  TurnPhase       phase;
  ent_t*          owner;
  param_t         ctx;
  int             initiative;
  int             turn, weight, score;
}action_t;

typedef struct{
  ActionCategory  id;
  int             count, cap, charges, allowed;
  int             inititive_mod;
  action_t*       entries;
  ent_t*          owner;
}action_queue_t;

typedef struct{
  bool           valid;
  action_queue_t *queues[ACT_DONE];
}action_pool_t;

typedef struct{
  int             count, cap;
  TurnPhase       phase;
  ActionCategory  allowed;
  action_t*       entries[MAX_PHASE_ACTIONS];
}action_round_t;

typedef struct{
  int             turn;
  TurnPhase       phase, next;
  action_round_t  round[TURN_ALL];
}turn_action_manager_t;

void InitActionManager(void);
void InitActions(action_turn_t* actions[ACTION_DONE]);

action_t* InitActionMove(ent_t* e, ActionCategory cat, Cell dest, int weight);
action_t* InitAction(ent_t* e, uint64_t id, ActionType type, ActionCategory cat, param_t ctx, int weight);
action_queue_t* InitActionQueue(ent_t* e, ActionCategory cat, int cap);

ActionStatus QueueAction(action_pool_t *p, action_t* t);
action_t* ActionForPhase(action_queue_t* q, TurnPhase phase);
void ActionPoolSync(action_pool_t* p);
action_pool_t* InitActionPool(ent_t* e);

void ActionManagerSync(void);
TurnPhase ActionManagerPreSync(void);
action_turn_t* InitActionTurn(ActionType t, DesignationType targeting, TakeActionCallback fn, OnActionCallback cb);
bool ActionPlayerAttack(ent_t* e, ActionType a, KeyboardKey k, ActionSlot slot);
bool ActionTurnMove(ent_t*, ActionType a, KeyboardKey k, ActionSlot slot);
void ActionStandby(ent_t* e);
void ActionTurnSync(ent_t* e);
bool ActionInput(void);
bool SetAction(ent_t* e, ActionType a, void* context, DesignationType targeting);
bool ActionTaken(ent_t* e, ActionType a);
bool TakeAction(ent_t* e, action_turn_t* action);
bool ActionTraverseGrid(ent_t* e,  ActionType a, OnActionCallback cb);
ActionType ActionGetEntNext(ent_t* e);
bool ActionAttack(ent_t* e, ActionType a, OnActionCallback cb);
bool ActionMultiTarget(ent_t* e, ActionType a, OnActionCallback cb);
void ActionSetTarget(ent_t* e, ActionType a, void* target);

static action_key_t action_keys[ACTION_DONE] = {
  {ACTION_NONE},
  {ACTION_MOVE,8,{KEY_D,KEY_A,KEY_W,KEY_S,KEY_LEFT, KEY_RIGHT,KEY_UP,KEY_DOWN},ActionTurnMove,SLOT_NONE},
  {ACTION_ATTACK,1,{KEY_F},ActionPlayerAttack,SLOT_ATTACK},
  {ACTION_WEAPON,0},
  {ACTION_ITEM,1,{KEY_V},ActionPlayerAttack,SLOT_ITEM},
  {ACTION_MAGIC,1,{KEY_M},ActionPlayerAttack,SLOT_SPELL},
};


action_slot_t* InitActionSlot(ActionSlot id, ent_t* owner, int rank, int cap);
void ActionSlotSortByPref(ent_t* owner, int *pool, int count);
bool ActionSlotAddAbility(ent_t* owner, ability_t* a);

typedef struct{
  ent_t*        owner;
  int           turn;
  TurnPhase     phase;
  bool          key_event;
  action_key_t  actions[ACTION_DONE];
}game_input_t;

void InitInput(ent_t* player);
void InputSync(TurnPhase phase, int turn);
bool InputCheck(TurnPhase phase, int turn);
#endif
