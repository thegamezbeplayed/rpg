#ifndef __GAME_UTIL__
#define __GAME_UTIL__

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#define MAX_DEBUG_ITEMS 128
#define MAX_PLAYERS 32
#define MAX_BEHAVIOR_TREE 12
#define MAX_CONTEXT 8192
#include "game_common.h"

#define COMBO_KEY(a, b) ((a << 8) | b)
#define CALL_FUNC(type, ptr, ...) ((type)(ptr))(__VA_ARGS__)
#define MAKE_ADAPTER(name, T) \
    static void name##_Adapter(void *p) { name((T)p); }

#define MAKE_EVENT_ADAPTER(name, T)\
  static void name##_Event(EventType event, void* data, void* user)\
{name((T)p);}

#define ParamRead(o, T) ((T*)((o)->data))

typedef struct ability_s ability_t;
typedef struct local_ctx_s local_ctx_t;
typedef struct{
  bool                initiated;
  int                 challenge;
  int                 offensive_rating, defensive_rating;
  float               threat_mul, threat;
  int                 last_turn;
}aggro_t;

typedef struct{
  game_object_uid_i gouid;
  DataType          type_id;
  void*             data;
  size_t            size;
}param_t;

typedef struct{
  DebugType type;
  Color     color;
  param_t   *info;
}debug_info_t;


static param_t ParamMakeObj(DataType type, game_object_uid_i uid, void* src) {
  param_t o;
  o.type_id = type;
  o.size = 0;
  o.gouid = uid;
  o.data = src;
  return o;
}

static param_t ParamMake(DataType type, size_t size, void* src) {
  param_t o;
  o.type_id = type;
  o.size = size;
  o.gouid = -1;
  o.data = malloc(size);
  memcpy(o.data, src, size);
  return o;
}

static inline param_t ParamMakePtr(DataType type, void* ptr){
  return (param_t){
    .type_id = type,
    .data = ptr
  };
}

static param_t ParamClone(const param_t* src) {
    param_t p = *src;

    if (src->size > 0 && src->data) {
        p.data = malloc(src->size);
        memcpy(p.data, src->data, src->size);
    }

    return p;
}

static int ParamScore(DataType type, param_t *a, param_t* b){
  assert(a->type_id == type);
  assert(b->type_id == type);

}
typedef bool (*ParamCompareFn)(param_t *a, param_t *b);
static bool ParamCompareGreater(param_t *a, param_t *b){
  int aval = 0, bval = 0;
  int adec = 0, bdec = 0;

  switch(a->type_id){
    case DATA_FLOAT:
      Cell afval = float_to_ints(*ParamRead(a, float));
      Cell bfval = float_to_ints(*ParamRead(b, float));
      aval = afval.x;
      adec = afval.y;
      
      bval = bfval.x;
      bdec = bfval.y;
      aval = aval*100 + adec;
      bval = bval*100 + bdec;
      break;
    case DATA_INT:
      aval = *ParamRead(a, int);
      bval = *ParamRead(b, int);
      break;

  }
  return GREATER_THAN(aval, bval);
}
static bool ParamCompareAnd(param_t *a, param_t *b){
  uint64_t abits = *ParamRead(a, uint64_t);
  uint64_t bbits = *ParamRead(b, uint64_t);

  return ((abits & bbits) > 0);
}

static bool ParamCompareLesser(param_t *a, param_t *b){
  return LESS_THAN(*ParamRead(a, int) ,*ParamRead(b, int));
}


static bool ParamCompare(param_t *a, param_t *b, ParamCompareFn fn){
  assert(a->type_id == b->type_id);

  return fn(a ,b);

}

static inline const char* ParamReadString(param_t* o){
  assert(o);
  assert(o->type_id == DATA_STRING);
  assert(o->data);
  assert(o->size > 0);

  return (const char*)o->data;
}
static inline need_t* ParamReadNeed(const param_t* o) {
    assert(o->type_id == DATA_NEED);
    return (need_t*)o->data;
}
 
static inline float ParamReadFloat(const param_t* o) {
    assert(o->type_id == DATA_FLOAT);
    assert(o->size == sizeof(float));
    return *(float*)o->data;
} 

static inline int ParamReadInt(const param_t* o) {
    assert(o->type_id == DATA_INT);
    assert(o->size == sizeof(int));
    return *(int*)o->data;
}

static inline Cell ParamReadCell(const param_t* o){
  assert(o->type_id == DATA_CELL);
  return *(Cell*)o->data;

}

static inline game_object_uid_i ParamReadGOUID(const param_t* o){
  assert(o->type_id == DATA_GOUID);
  assert(o->size == sizeof(game_object_uid_i));
  return *(game_object_uid_i*)o->data;

}

local_ctx_t* ParamReadCtx(const param_t* o);
ent_t* ParamReadEnt(const param_t* o);
env_t* ParamReadEnv(const param_t* o);
map_cell_t* ParamReadMapCell(const param_t* o);
//ability_t* ParamReadAbility(const param_t* o);
static void ParamFree(param_t* o) {
    free(o->data);
    o->data = NULL;
}

//====FILE & STRINGS====>
char* GetFileStem(const char* filename);
//<==========
//
static inline void DO_NOTHING(void){}
static inline bool BOOL_DO_NOTHING(){return false;}
static inline const char* CHAR_DO_NOTHING(){return "\0";}
//<===== OPTION CHOOSE

typedef struct choice_s choice_t;
typedef struct choice_pool_s choice_pool_t;
typedef void (*OnChosen)(choice_pool_t* pool, choice_t* self);

void DiscardChoice(choice_pool_t* pool, choice_t* self);

typedef choice_t* (*ChoiceFn)(choice_pool_t *pool);
choice_t* ChooseBest(choice_pool_t* pool);
choice_t* ChooseByWeight(choice_pool_t* pool);
choice_t* ChooseByBudget(choice_pool_t* pool);
choice_t* ChooseByWeightInBudget(choice_pool_t* pool);
choice_t* ChooseCheapest(choice_pool_t* pool);
void ChoiceReduceScore(choice_pool_t* pool, choice_t* self);

struct choice_s{
  unsigned int id;
  int          score, orig_score, cost;
  void*        context;
  OnChosen     cb;
};

choice_t* ChoiceById(choice_pool_t* pool, int id);

struct choice_pool_s{
  unsigned int  id;
  int           count,budget,filtered, total;
  ChoiceFn      choose;
  choice_t      *choices[MAX_OPTIONS];
  choice_t      *filter[MAX_OPTIONS];
};

static inline bool ChoiceAllowed(choice_pool_t* pool, choice_t* c){
  if (pool->filtered == 0)  // empty filter → allow all
    return true;

  for (int i = 0; i < pool->filtered; i++){
    if (pool->filter[i]->id == c->id)
      return true;
  }
  return false;
}

choice_pool_t* StartChoice(choice_pool_t** pool, int size, ChoiceFn fn, bool* result);
void EndChoice(choice_pool_t* pool, bool reset);
choice_pool_t* InitChoicePool(int size, ChoiceFn fn);
bool AddChoice(choice_pool_t *pool, int id, int score, void *ctx, OnChosen fn);
bool AddPurchase(choice_pool_t *pool, int id, int score, int cost, void *ctx, OnChosen fn);
bool AddFilter(choice_pool_t *pool, int id, void *ctx);

void PriorityEvent(EventType, void* e, void* u);

typedef struct{
  game_object_uid_i gouid;
  Priorities        type;
  param_t           ctx;
  Interactive       method;
  int               score;
  bool              prune;
}priority_t;
  
typedef struct{
  ent_t*     owner;     
  bool       valid;     
  int        cap, count;
  priority_t *entries;
}priorities_t;
  
priorities_t* InitPriorities(ent_t* e, int cap);
priority_t*  PriorityAdd(priorities_t* table, Priorities type, param_t entry);
void PrioritiesSync(priorities_t* t);

typedef struct{
  uint64_t          id;
  ActionType        decision;
  ActionStatus      status;
  ActionCategory    cat;
  game_object_uid_i auid;
  EntityState       state;
  param_t           params[ACT_PARAM_ALL];
  int               score, cost;
}decision_t;

void OnDecisionAction(EventType event, void* data, void* user);
typedef struct{
  EntityState   id;
  ent_t*        owner;
  uint64_t      ouid;
  ActionStatus  status;
  int           count, cap;
  int           scores[128],costs[128], econ[128];
  decision_t    *selected;
  decision_t*   entries;
  hash_map_t    map;
}decision_pool_t;

typedef decision_t* (*DecisionSortFn)(decision_pool_t*);
decision_t* DecisionSortEconomic(decision_pool_t* table);
decision_t* DecisionSortByScore(decision_pool_t* table);
decision_t* DecisionSortByCost(decision_pool_t* table);

static decision_t* DecisionGetEntry(decision_pool_t* t, game_object_uid_i id){
    return HashGet(&t->map, id);

}
decision_pool_t* StartDecision(decision_pool_t** pool, int size, ent_t* e, EntityState id, bool* result);
bool AddPriority(decision_pool_t* t, priority_t*);
bool AddDecision(decision_pool_t* t, decision_t*);
bool AddCandidate(decision_pool_t*, local_ctx_t*, ActionParam, Score s, Score c);
bool AddDestination(decision_pool_t*, local_ctx_t*, EntityState, Score s, Score c);
bool MakeDecision(decision_pool_t* t, DecisionSortFn);
decision_t* InitDecision(decision_pool_t* t, game_object_uid_i other);
decision_pool_t* InitDecisionPool(int size, ent_t* e, EntityState id);
bool AddEnemy(decision_pool_t* t, local_ctx_t* ctx);
bool AddAbility(decision_pool_t* t, local_ctx_t* tar, ability_t* a);
void OnActionSuccess(EventType event, void* data, void* user);

struct local_ctx_s{
  param_t             other, need;
  path_cache_entry_t* path;
  game_object_uid_i   puid;
  game_object_uid_i   gouid;
  aggro_t*            aggro;
  param_t             params[PARAM_ALL];
  int                 scores[SCORE_ALL];
  uint64_t            resource;
  Cell                pos;
  Vector2             v_pos;
  ActionType          how_to;
  Interactive         method;
  bool                valid, prune;
  float               awareness;
  float               treatment[TREAT_DONE];
  int                 prio, dist, index;
  uint32_t            ctx_revision;
  int                 last_update;
};

typedef struct{
  ent_t*             owner;
  bool               valid;
  local_ctx_t*       entries;
  int                count,cap;
  choice_pool_t*     choice_pool;
  int                sorted_indices[MAX_CONTEXT];
  hash_map_t         ctx_by_gouid;
  int                last_update;
}local_table_t;

local_table_t* InitLocals(ent_t* e, int cap);
local_ctx_t* MakeLocalContext(local_table_t* s, param_t* e, Cell p);
void AddLocalFromCtx(local_table_t *s, local_ctx_t* ctx);
void LocalSyncPos(EventType, void* e, void* u);
void LocalSync(local_table_t* s, bool sort);
void LocalSyncCtx(local_table_t* s, local_ctx_t*);
bool LocalCheck(local_ctx_t* ctx);
void LocalPruneCtx(local_table_t* t, game_object_uid_i other);
int LocalContextFilter(local_table_t* t, int num, local_ctx_t* pool[num], param_t filter, GameObjectParam type, ParamCompareFn);
void LocalSortByDist(local_table_t* table);
aggro_t* LocalAggroByCtx(local_ctx_t* ctx);
int LocalAddAggro(local_table_t* t, ent_t* e, int threat, float mul, bool init);
aggro_t* LocalGetAggro(local_table_t* table, game_object_uid_i other);
local_ctx_t* LocalGetThreat(local_table_t* t);
local_ctx_t* LocalGetEntry(local_table_t* table, game_object_uid_i other);
void LocalSetPath(ent_t* e, local_ctx_t* dest);
//<===BEHAVIOR TREES

//forward declare
struct behavior_tree_node_s;

struct behavior_tree_node_s *BehaviorGetTree( BehaviorID id);

typedef enum{
  BEHAVIOR_FAILURE,
  BEHAVIOR_RUNNING,
  BEHAVIOR_SUCCESS,
}BehaviorStatus;

typedef enum{
  BT_LEAF,
  BT_SEQUENCE,
  BT_SELECTOR,
  BT_CONCURRENT,
  BT_DECIDER,
}BehaviorTreeType;

typedef struct {
  BehaviorID           id;
  struct behavior_tree_node_s *root;
} TreeCacheEntry;

extern TreeCacheEntry tree_cache[18];
extern int tree_cache_count;

static struct behavior_tree_node_s* BehaviorFindLeafFactory(const char *name);

typedef BehaviorStatus (*BehaviorTreeTickFunc)(struct behavior_tree_node_s* self, void* context);

typedef struct behavior_params_s{
  struct ent_s*         owner;
  EntityState           state, o_state;
  ActionType            action, o_action;
}behavior_params_t;

struct behavior_tree_node_s *BuildTreeNode(BehaviorID id, behavior_params_t* parent_params);

typedef struct behavior_tree_node_s{
  BehaviorID            id;
  BehaviorTreeType      bt_type;
  BehaviorTreeTickFunc  tick;
  void*                 data;
}behavior_tree_node_t;

typedef struct{
  behavior_tree_node_t  **children;
  int                   num_children;
  int                   current;
}behavior_tree_sequence_t;

typedef struct{
  behavior_tree_node_t  **children;
  int                   num_children;
  int                   current;
}behavior_tree_selector_t;

typedef BehaviorStatus (*BehaviorTreeLeafFunc)(behavior_params_t* params);

typedef struct{
  BehaviorTreeLeafFunc  action;
  behavior_params_t*    params;
}behavior_tree_leaf_t;

behavior_tree_node_t* InitBehaviorTree( BehaviorID id);
void FreeBehaviorTree(behavior_tree_node_t* node);
BehaviorStatus BehaviorTickSequence(behavior_tree_node_t *self, void *context);
BehaviorStatus BehaviorTickSelector(behavior_tree_node_t *self, void *context);
BehaviorStatus BehaviorTickConcurrent(behavior_tree_node_t *self, void *context);
behavior_tree_node_t* BehaviorCreateLeaf(BehaviorTreeLeafFunc fn, behavior_params_t* params);
behavior_tree_node_t* BehaviorCreateSequence(behavior_tree_node_t **children, int count);
behavior_tree_node_t* BehaviorCreateSelector(behavior_tree_node_t **children, int count);
behavior_tree_node_t* BehaviorCreateConcurrent(behavior_tree_node_t **children, int count);

BehaviorStatus BehaviorChangeState(behavior_params_t *params);
BehaviorStatus BehaviorCheckAggro(behavior_params_t *params);
BehaviorStatus BehaviorCheckAbility(behavior_params_t *params);
BehaviorStatus BehaviorGetPriority(behavior_params_t *params);
BehaviorStatus BehaviorCheckUrgency(behavior_params_t *params);
BehaviorStatus BehaviorCheckReady(behavior_params_t *params);
BehaviorStatus BehaviorCheckRange(behavior_params_t *params);
BehaviorStatus BehaviorAcquireDestination(behavior_params_t *params);
BehaviorStatus BehaviorAttackTarget(behavior_params_t *params);
BehaviorStatus BehaviorTakeTurn(behavior_params_t *params);
BehaviorStatus BehaviorFillNeed(behavior_params_t *params);
BehaviorStatus BehaviorFindResource(behavior_params_t *params);
BehaviorStatus BehaviorSeekResource(behavior_params_t *params);
BehaviorStatus BehaviorCheckResource(behavior_params_t *params);
BehaviorStatus BehaviorAcquireResource(behavior_params_t *params);
BehaviorStatus BehaviorExecuteDecision(behavior_params_t *params);
BehaviorStatus BehaviorActionDecision(behavior_params_t *params);

static inline behavior_tree_node_t* LeafChangeState(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorChangeState,params); }
static inline behavior_tree_node_t* LeafCheckReady(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckReady,params); }
static inline behavior_tree_node_t* LeafCheckRange(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckRange,params); }
static inline behavior_tree_node_t* LeafCheckAggro(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckAggro,params); }
static inline behavior_tree_node_t* LeafCheckAbility(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckAbility,params); }
static inline behavior_tree_node_t* LeafGetPriority(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorGetPriority,params); }
static inline behavior_tree_node_t* LeafCheckUrgency(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckUrgency,params); }
static inline behavior_tree_node_t* LeafAcquireDestination(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorAcquireDestination,params); }
static inline behavior_tree_node_t* LeafAttackTarget(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorAttackTarget,params); }
static inline behavior_tree_node_t* LeafTakeTurn(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorTakeTurn,params); }
static inline behavior_tree_node_t* LeafFillNeed(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorFillNeed,params); }
static inline behavior_tree_node_t* LeafFindResource(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorFindResource,params); }
static inline behavior_tree_node_t* LeafAcquireResource(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorAcquireResource,params); }
static inline behavior_tree_node_t* LeafCheckResource(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckResource,params); }
static inline behavior_tree_node_t* LeafSeekResource(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorSeekResource,params); }
static inline behavior_tree_node_t* LeafExecuteDecision(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorExecuteDecision,params); }
static inline behavior_tree_node_t* LeafActionDecision(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorActionDecision,params); }




#endif
