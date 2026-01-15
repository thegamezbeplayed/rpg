#ifndef __GAME_UTIL__
#define __GAME_UTIL__

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#define MAX_DEBUG_ITEMS 128
#define MAX_PLAYERS 32
#define MAX_BEHAVIOR_TREE 12
#include "game_common.h"

#define COMBO_KEY(a, b) ((a << 8) | b)
#define CALL_FUNC(type, ptr, ...) ((type)(ptr))(__VA_ARGS__)
#define MAKE_ADAPTER(name, T) \
    static void name##_Adapter(void *p) { name((T)p); }

typedef struct{
  uint32_t type_id;
  void*    data;
  size_t   size;
}param_t;

typedef struct{
  DebugType type;
  Color     color;
  param_t   *info;
}debug_info_t;

static param_t ParamMake(uint32_t type, size_t size, const void* src) {
    param_t o;
    o.type_id = type;
    o.size = size;
    o.data = malloc(size);
    memcpy(o.data, src, size);
    return o;
}
static inline const char* ParamReadString(param_t* o){
  assert(o);
  assert(o->type_id == DATA_STRING);
  assert(o->data);
  assert(o->size > 0);

  return (const char*)o->data;
}
static inline need_t ParamReadNeed(const param_t* o) {
    assert(o->type_id == DATA_NEED);
    assert(o->size == sizeof(need_t));
    return *(need_t*)o->data;
}
  
static inline int ParamReadInt(const param_t* o) {
    assert(o->type_id == DATA_INT);
    assert(o->size == sizeof(int));
    return *(int*)o->data;
}

static inline Cell ParamReadCell(const param_t* o){
  assert(o->type_id == DATA_CELL);
  assert(o->size == sizeof(Cell));
  return *(Cell*)o->data;

}

ent_t* ParamReadEnt(const param_t* o);
env_t* ParamReadEnv(const param_t* o);
map_cell_t* ParamReadMapCell(const param_t* o);
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
//<===BEHAVIOR TREES

//forward declare
struct behavior_tree_node_s;

struct behavior_tree_node_s *BehaviorGetTree( BehaviorID id);

typedef enum{
  BEHAVIOR_SUCCESS,
  BEHAVIOR_FAILURE,
  BEHAVIOR_RUNNING
}BehaviorStatus;

typedef enum{
  BT_LEAF,
  BT_SEQUENCE,
  BT_SELECTOR,
  BT_CONCURRENT
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
  EntityState           state;
  ActionType            action;
}behavior_params_t;

struct behavior_tree_node_s *BuildTreeNode(BehaviorID id, behavior_params_t* parent_params);

typedef struct behavior_tree_node_s{
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
BehaviorStatus BehaviorMoveToTarget(behavior_params_t *params);
BehaviorStatus BehaviorCheckAggro(behavior_params_t *params);
BehaviorStatus BehaviorCheckAbilities(behavior_params_t *params);
BehaviorStatus BehaviorCheckInventory(behavior_params_t *params);
BehaviorStatus BehaviorCheckSenses(behavior_params_t *params);
BehaviorStatus BehaviorMoveToDestination(behavior_params_t *params);
BehaviorStatus BehaviorAcquireDestination(behavior_params_t *params);
BehaviorStatus BehaviorAcquireTarget(behavior_params_t *params);
BehaviorStatus BehaviorCanAttackTarget(behavior_params_t *params);
BehaviorStatus BehaviorCheckTurn(behavior_params_t *params);
BehaviorStatus BehaviorAttackTarget(behavior_params_t *params);
BehaviorStatus BehaviorTakeTurn(behavior_params_t *params);
BehaviorStatus BehaviorCanSeeTarget(behavior_params_t *params);
BehaviorStatus BehaviorCheckNeeds(behavior_params_t *params);
BehaviorStatus BehaviorCheckNeed(behavior_params_t *params);
BehaviorStatus BehaviorFindResource(behavior_params_t *params);
BehaviorStatus BehaviorSeekResource(behavior_params_t *params);
BehaviorStatus BehaviorTrackResource(behavior_params_t *params);
BehaviorStatus BehaviorFillNeeds(behavior_params_t *params);
BehaviorStatus BehaviorFillNeed(behavior_params_t *params);
BehaviorStatus BehaviorBuildAwareness(behavior_params_t *params);

static inline behavior_tree_node_t* LeafChangeState(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorChangeState,params); }
static inline behavior_tree_node_t* LeafMoveToTarget(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorMoveToTarget,params); }
static inline behavior_tree_node_t* LeafCheckAbilities(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckAbilities,params); }
static inline behavior_tree_node_t* LeafCheckAggro(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckAggro,params); }
static inline behavior_tree_node_t* LeafCheckInventory(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckInventory,params); }
static inline behavior_tree_node_t* LeafCheckSenses(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckSenses,params); }
static inline behavior_tree_node_t* LeafMoveToDestination(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorMoveToDestination,params); }
static inline behavior_tree_node_t* LeafAcquireDestination(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorAcquireDestination,params); }
static inline behavior_tree_node_t* LeafAcquireTarget(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorAcquireTarget,params); }
static inline behavior_tree_node_t* LeafCanAttackTarget(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCanAttackTarget,params); }
static inline behavior_tree_node_t* LeafCheckTurn(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckTurn,params); }
static inline behavior_tree_node_t* LeafAttackTarget(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorAttackTarget,params); }
static inline behavior_tree_node_t* LeafTakeTurn(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorTakeTurn,params); }
static inline behavior_tree_node_t* LeafCanSeeTarget(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCanSeeTarget,params); }
static inline behavior_tree_node_t* LeafCheckNeeds(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckNeeds,params); }
static inline behavior_tree_node_t* LeafCheckNeed(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckNeed,params); }
static inline behavior_tree_node_t* LeafFindResource(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorFindResource,params); }
static inline behavior_tree_node_t* LeafSeekResource(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorSeekResource,params); }
static inline behavior_tree_node_t* LeafTrackResource(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorTrackResource,params); }
static inline behavior_tree_node_t* LeafFillNeeds(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorFillNeeds,params); }
static inline behavior_tree_node_t* LeafFillNeed(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorFillNeed,params); }
static inline behavior_tree_node_t* LeafBuildAwareness(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorBuildAwareness,params); }




#endif
