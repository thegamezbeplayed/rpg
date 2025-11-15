#ifndef __GAME_UTIL__
#define __GAME_UTIL__

#include <stdlib.h>
#include <string.h>
#include "game_common.h"

#define MAX_PLAYERS 32

#define MAX_BEHAVIOR_TREE 12
#define MAX_NAME_LEN 64
#define COMBO_KEY(a, b) ((a << 8) | b)
#define CALL_FUNC(type, ptr, ...) ((type)(ptr))(__VA_ARGS__)
#define MAKE_ADAPTER(name, T) \
    static void name##_Adapter(void *p) { name((T)p); }
#pragma once
#ifdef __EMSCRIPTEN__
  #include <emscripten/emscripten.h>
#else
  #define EMSCRIPTEN_KEEPALIVE
#endif
void UploadScore(void);

//====FILE & STRINGS====>
char* GetFileStem(const char* filename);
//<==========
//
static inline void DO_NOTHING(void){}
static inline bool BOOL_DO_NOTHING(){return false;}
static inline const char* CHAR_DO_NOTHING(){return "\0";}
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
  TurnState             turn;
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

typedef struct {
    const char *name;  // "CanSeeTarget", "MoveToTarget", ...
    behavior_tree_node_t* (*factory)(behavior_params_t *params); // params is leaf-specific (can be NULL)
} BTLeafRegistryEntry;

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
BehaviorStatus BehaviorChangeOwnerState(behavior_params_t *params);
BehaviorStatus BehaviorChangeChildState(behavior_params_t *params);
BehaviorStatus BehaviorInitChild(behavior_params_t *params);
BehaviorStatus BehaviorMatchNeighbors(behavior_params_t *params);
BehaviorStatus BehaviorMatchChild(behavior_params_t *params);
BehaviorStatus BehaviorProgressWorldState(behavior_params_t *params);
BehaviorStatus BehaviorCheckOthersState(behavior_params_t *params);
BehaviorStatus BehaviorClearMatchState(behavior_params_t *params);
BehaviorStatus BehaviorCheckOwnersState(behavior_params_t *params);
BehaviorStatus BehaviorCheckChildState(behavior_params_t *params);
BehaviorStatus BehaviorCheckWorldState(behavior_params_t *params);
BehaviorStatus BehaviorCheckMoves(behavior_params_t *params);
BehaviorStatus BehaviorCheckSolutions(behavior_params_t *params);
BehaviorStatus BehaviorSelectShape(behavior_params_t *params);
BehaviorStatus BehaviorSelectHelpfulShape(behavior_params_t *params);

static inline behavior_tree_node_t* LeafCheckOthersState(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckOthersState,params); }
static inline behavior_tree_node_t* LeafClearMatchState(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorClearMatchState,params); }
static inline behavior_tree_node_t* LeafCheckOwnersState(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckOwnersState,params); }
static inline behavior_tree_node_t* LeafCheckChildState(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckChildState,params); }
static inline behavior_tree_node_t* LeafCheckWorldState(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckWorldState,params); }
static inline behavior_tree_node_t* LeafCheckMoves(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckMoves,params); }
static inline behavior_tree_node_t* LeafCheckSolutions(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorCheckSolutions,params); }
static inline behavior_tree_node_t* LeafInitChild(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorInitChild,params); }
static inline behavior_tree_node_t* LeafMatchNeighbors(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorMatchNeighbors,params); }
static inline behavior_tree_node_t* LeafMatchChild(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorMatchChild,params); }
static inline behavior_tree_node_t* LeafProgressWorldState(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorProgressWorldState,params); }
static inline behavior_tree_node_t* LeafChangeState(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorChangeState,params); }
static inline behavior_tree_node_t* LeafChangeOwnerState(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorChangeOwnerState,params); }
static inline behavior_tree_node_t* LeafChangeChildState(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorChangeChildState,params); }
static inline behavior_tree_node_t* LeafSelectShape(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorSelectShape,params); }
static inline behavior_tree_node_t* LeafSelectHelpfulShape(behavior_params_t *params)  { return BehaviorCreateLeaf(BehaviorSelectHelpfulShape,params); }
#endif
