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
typedef struct action_key_s action_key_t;
typedef BehaviorStatus (*ActionKeyCallback)(struct ent_s* e, action_key_t a, KeyboardKey k);
  
struct action_key_s{
  ActionType        action;
  int               num_keys;
  KeyboardKey       keys[8];
  ActionKeyCallback fn;
  int               binding;
};

typedef struct{
  ActionSlot     id;
  ActionType     allowed[ACTION_SLOTTED];
  ent_t*         owner;
  StatType       resource;
  int            count, cap, size, space, rank, pref;
  ability_t      **abilities;
}action_slot_t;

typedef struct action_s action_t;

typedef void (*ActionCb)(void*);
typedef ActionStatus (*ActionFn)(action_t* a);
struct action_s{
  uint64_t        id;
  ActionType      type;
  ActionCategory  cat;
  ActionStatus    status;
  TurnPhase       phase;
  ent_t*          owner;
  param_t         params[ACT_PARAM_ALL];
  int             initiative;
  int             turn, weight, score;
  ActionFn        fn;
  ActionCb        cb;
};

typedef struct{
  ActionCategory  id;
  ActionStatus    status;
  int             count, cap, charges, allowed;
  action_t*       entries;
  ent_t*          owner;
}action_queue_t;

typedef struct{
  bool           valid;
  bool           options[TURN_ALL];
  action_queue_t *queues[ACT_DONE];
}action_pool_t;

typedef struct{
  int             count, cap;
  ActionStatus  status;
  TurnPhase       phase;
  ActionCategory  allowed;
  action_t*       entries[MAX_PHASE_ACTIONS];
}action_round_t;

typedef struct{
  int             turn;
  TurnPhase       phase, next;
  action_round_t  round[TURN_ALL];
}turn_action_manager_t;
extern turn_action_manager_t ActionMan;

action_t* InitActionByDecision(decision_t* d, ActionType t);
BehaviorStatus ActionExecute(ActionType, action_t*);
void InitActionManager(void);
void OnPlayerAction(EventType event, void* data, void* user);
void InitActions(action_turn_t* actions[ACTION_DONE]);
bool ActionHasStatus(action_pool_t* p, ActionStatus s);
action_queue_t* InitActionQueue(ent_t* e, ActionCategory cat, int cap);

ActionStatus QueueAction(action_pool_t *p, action_t* t);
action_t* ActionForPhase(action_queue_t* q, TurnPhase phase);
void ActionPoolSync(action_pool_t* p);
action_pool_t* InitActionPool(ent_t* e);

void ActionManagerSync(void);
action_turn_t* InitActionTurn(ActionType t, DesignationType targeting, TakeActionCallback fn, OnActionCallback cb);
void ActionStandby(ent_t* e);
void ActionTurnSync(ent_t* e);
bool ActionInput(void);
//bool SetAction(ent_t* e, ActionType a, void* context, DesignationType targeting);
//bool ActionTaken(ent_t* e, ActionType a);
//bool TakeAction(ent_t* e, action_turn_t* action);
ActionType ActionGetEntNext(ent_t* e);
bool ActionMultiTarget(ent_t* e, ActionType a, OnActionCallback cb);
void ActionSetTarget(ent_t* e, ActionType a, void* target);


action_slot_t* InitActionSlot(ActionSlot id, ent_t* owner, int rank, int cap);
void ActionSlotSortByPref(ent_t* owner, int *pool, int count);
bool ActionSlotAddAbility(ent_t* owner, ability_t* a);

typedef struct{
  ent_t*          owner;
  int             turn;
  TurnPhase       phase;
  decision_t*     decisions[ACTION_PASSIVE];
  action_key_t    actions[ACTION_DONE];
}game_input_t;

void InitInput(ent_t* player);
void InputSync(TurnPhase phase, int turn);
bool InputCheck(TurnPhase phase, int turn);
void InputSetTarget(ent_t* e, ActionType a, local_ctx_t* target);


#endif
