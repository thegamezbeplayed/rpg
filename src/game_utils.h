#ifndef __GAME_UTIL__
#define __GAME_UTIL__

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "game_common.h"

#define MAX_PLAYERS 32

#define MAX_BEHAVIOR_TREE 12

#define COMBO_KEY(a, b) ((a << 8) | b)
#define CALL_FUNC(type, ptr, ...) ((type)(ptr))(__VA_ARGS__)
#define MAKE_ADAPTER(name, T) \
    static void name##_Adapter(void *p) { name((T)p); }


typedef struct{
  uint32_t type_id;
  void*    data;
  size_t   size;
}param_t;

static param_t ParamMake(uint32_t type, size_t size, const void* src) {
    param_t o;
    o.type_id = type;
    o.size = size;
    o.data = malloc(size);
    memcpy(o.data, src, size);
    return o;
}

static inline int ParamReadInt(const param_t* o) {
    assert(o->type_id == DATA_INT);
    assert(o->size == sizeof(int));
    return *(int*)o->data;
}

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

struct choice_s{
  int       score;
  void*     context;
  OnChosen  cb;
};

struct choice_pool_s{
  unsigned int  id;
  int           count,budget;
  ChoiceFn      choose;
  choice_t      *choices[MAX_OPTIONS];
};

choice_pool_t* StartChoice(choice_pool_t* pool, int size, ChoiceFn fn, bool* result);
choice_pool_t* InitChoicePool(int size, ChoiceFn fn);
bool AddChoice(choice_pool_t *pool, int score, void *ctx);
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
BehaviorStatus BehaviorMoveToDestination(behavior_params_t *params);
BehaviorStatus BehaviorAcquireDestination(behavior_params_t *params);
BehaviorStatus BehaviorAcquireTarget(behavior_params_t *params);
BehaviorStatus BehaviorCanAttackTarget(behavior_params_t *params);
BehaviorStatus BehaviorCheckTurn(behavior_params_t *params);
BehaviorStatus BehaviorAttackTarget(behavior_params_t *params);
BehaviorStatus BehaviorTakeTurn(behavior_params_t *params);
BehaviorStatus BehaviorCanSeeTarget(behavior_params_t *params);

static inline behavior_tree_node_t* LeafChangeState(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorChangeState,params); }
static inline behavior_tree_node_t* LeafMoveToTarget(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorMoveToTarget,params); }
static inline behavior_tree_node_t* LeafCheckAggro(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckAggro,params); }
static inline behavior_tree_node_t* LeafMoveToDestination(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorMoveToDestination,params); }
static inline behavior_tree_node_t* LeafAcquireDestination(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorAcquireDestination,params); }
static inline behavior_tree_node_t* LeafAcquireTarget(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorAcquireTarget,params); }
static inline behavior_tree_node_t* LeafCanAttackTarget(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCanAttackTarget,params); }
static inline behavior_tree_node_t* LeafCheckTurn(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckTurn,params); }
static inline behavior_tree_node_t* LeafAttackTarget(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorAttackTarget,params); }
static inline behavior_tree_node_t* LeafTakeTurn(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorTakeTurn,params); }
static inline behavior_tree_node_t* LeafCanSeeTarget(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCanSeeTarget,params); }

#endif
