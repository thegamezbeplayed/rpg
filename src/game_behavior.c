#include <raylib.h>
#include "game_utils.h"
#include "game_process.h"
#include "game_tools.h"
#include "game_helpers.h"

behavior_tree_node_t *BehaviorGetTree(BehaviorID id) {
   for (int i = 0; i < tree_cache_count; i++){
    if (tree_cache[i].id == id)
      return tree_cache[i].root;
  }

   return NULL;
}

behavior_tree_node_t *BuildTreeNode(BehaviorID id,behavior_params_t* parent_params) {
  if (room_behaviors[id].id != id)
    return NULL;

  BehaviorData data = room_behaviors[id];
  if(data.param_overide || parent_params == NULL){
    parent_params = malloc(sizeof(behavior_params_t));
    *parent_params =(behavior_params_t){
      .owner = NULL,
        .state = data.state,
        .turn = data.turn
    };
  }
  if(data.bt_type == BT_LEAF)
    return room_behaviors[id].func(parent_params);
  else{
    behavior_tree_node_t **kids = malloc(sizeof(*kids) * data.num_children);
    for (int j = 0; j < data.num_children; ++j)
      kids[j] = BuildTreeNode(data.children[j],parent_params);

    switch(data.bt_type){
      case BT_SEQUENCE:
        return BehaviorCreateSequence(kids, data.num_children);
        break;
      case BT_SELECTOR:
        return BehaviorCreateSelector(kids, data.num_children);
        break;
      case BT_CONCURRENT:
        return BehaviorCreateConcurrent(kids, data.num_children);
        break;
      default:
        TraceLog(LOG_WARNING,"Behavior Node Type %d NOT FOUND!",data.bt_type);
        break;
    }

  }

  return NULL;
}

behavior_tree_node_t* InitBehaviorTree( BehaviorID id){
  if(id ==BEHAVIOR_NONE)
    return NULL;
  behavior_tree_node_t* node = BehaviorGetTree(id);

  if(node != NULL)
    return node;

  TraceLog(LOG_WARNING,"<=====Behavior Tree %i not found=====>",id);
  return NULL;
}

BehaviorStatus BehaviorChangeChildState(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(!params->state)
    return BEHAVIOR_FAILURE;

  if(!e->child)
    return BEHAVIOR_FAILURE;

  if(SetState(e->child, params->state,NULL))
    return BEHAVIOR_SUCCESS;

  return BEHAVIOR_FAILURE;
}  

BehaviorStatus BehaviorChangeOwnerState(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(!params->state)
    return BEHAVIOR_FAILURE;
  
  if(!e->owner)
    return BEHAVIOR_FAILURE;

  if(SetState(e->owner, params->state,NULL))
    return BEHAVIOR_SUCCESS;

  return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorChangeState(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(!params->state)
    return BEHAVIOR_FAILURE;

  if(SetState(e, params->state,NULL))
    return BEHAVIOR_SUCCESS;
    
  return BEHAVIOR_FAILURE;

}

BehaviorStatus BehaviorMatchChild(behavior_params_t *params){
struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(!e->child)
    return BEHAVIOR_FAILURE;
 
  ent_t* shape_pool[GRID_WIDTH * GRID_HEIGHT];
  int num_shapes = WorldGetEnts(shape_pool,FilterEntShape, NULL);
  bool shape_match_row[GRID_WIDTH]={0};
  bool shape_match_col[GRID_HEIGHT]={0};

  bool color_match_row[GRID_WIDTH]={0};
  bool color_match_col[GRID_HEIGHT]={0};

  color_match_row[e->intgrid_pos.y]=true;
  shape_match_row[e->intgrid_pos.y]=true;

  color_match_col[e->intgrid_pos.x]=true;
  shape_match_col[e->intgrid_pos.x]=true;

  ShapeFlags compareShape = SHAPE_TYPE(e->child->shape);
  ShapeFlags compareColor = SHAPE_COLOR(e->child->shape);
  for (int i = 0; i < num_shapes; i++){
    Cell otherPos = shape_pool[i]->intgrid_pos;

    ShapeFlags otherShape = SHAPE_TYPE(shape_pool[i]->shape);
    ShapeFlags otherColor = SHAPE_COLOR(shape_pool[i]->shape);

    if(!IS_TYPE(compareShape,otherShape))
      continue;

    if(otherPos.x == e->intgrid_pos.x){
      shape_match_row[otherPos.y] = true;

      if(IS_COLOR(compareColor,otherColor))
        color_match_row[otherPos.y] = true;
    }

    if(otherPos.y == e->intgrid_pos.y){
      shape_match_col[otherPos.x] = true;

      if(IS_COLOR(compareColor,otherColor))
        color_match_col[otherPos.x] = true;
    }
  }

  bool has_match = false;
  if(COMPARE_ALL_BOOL(shape_match_row,GRID_WIDTH)){
    bool add_color_match_row = COMPARE_ALL_BOOL(color_match_row,GRID_WIDTH);
    TraceLog(LOG_INFO,"Match on row %i",e->intgrid_pos.y);
    has_match = true;
    WorldTurnAddMatch(e->child,add_color_match_row);
  }

  if(COMPARE_ALL_BOOL(shape_match_col,GRID_HEIGHT)){
    bool add_color_match_col = COMPARE_ALL_BOOL(color_match_col,GRID_HEIGHT);
    TraceLog(LOG_INFO,"Match on col %i",e->intgrid_pos.x);
    has_match = true;

    WorldTurnAddMatch(e->child,add_color_match_col);
  }

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorMatchNeighbors(behavior_params_t *params){
struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  ent_t* shape_pool[GRID_WIDTH * GRID_HEIGHT];
  int num_shapes = WorldGetEnts(shape_pool,FilterEntShape, e);
  
  bool shape_match_row[GRID_WIDTH]={0};
  bool shape_match_col[GRID_HEIGHT]={0};

  bool color_match_row[GRID_WIDTH]={0};
  bool color_match_col[GRID_HEIGHT]={0};

  color_match_row[e->intgrid_pos.y]=true;
  shape_match_row[e->intgrid_pos.y]=true;
  
  color_match_col[e->intgrid_pos.x]=true;
  shape_match_col[e->intgrid_pos.x]=true;

  ShapeFlags compareShape = SHAPE_TYPE(e->shape);
  ShapeFlags compareColor = SHAPE_COLOR(e->shape);
  for (int i = 0; i < num_shapes; i++){
    Cell otherPos = shape_pool[i]->intgrid_pos;

    ShapeFlags otherShape = SHAPE_TYPE(shape_pool[i]->shape);
    ShapeFlags otherColor = SHAPE_COLOR(shape_pool[i]->shape);

    if(!IS_TYPE(compareShape,otherShape))
      continue;

    if(otherPos.x == e->intgrid_pos.x){
      shape_match_row[otherPos.y] = true;

      if(IS_COLOR(compareColor,otherColor)){
        color_match_row[otherPos.y] = true;
      }
    }

    if(otherPos.y == e->intgrid_pos.y){
      shape_match_col[otherPos.x] = true;
      if(IS_COLOR(compareColor,otherColor)){
        color_match_col[otherPos.x] = true;
      }
    }
  }

  bool has_match = false;
  if(COMPARE_ALL_BOOL(shape_match_row,GRID_WIDTH)){
    bool add_color_match_row = COMPARE_ALL_BOOL(color_match_row,GRID_WIDTH);
    TraceLog(LOG_INFO,"Match on row %i",e->intgrid_pos.y);
    has_match = true;
    WorldTurnAddMatch(e,add_color_match_row);
  }

  if(COMPARE_ALL_BOOL(shape_match_col,GRID_HEIGHT)){
    bool add_color_match_col = COMPARE_ALL_BOOL(color_match_col,GRID_HEIGHT);
    TraceLog(LOG_INFO,"Match on col %i",e->intgrid_pos.x);
    has_match = true;
      
    WorldTurnAddMatch(e,add_color_match_col);
  }

  if(has_match)
    return BEHAVIOR_SUCCESS;

  return BEHAVIOR_FAILURE;

}

BehaviorStatus BehaviorInitChild(behavior_params_t *params){
struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(!e->child)
    return BEHAVIOR_FAILURE;

  if(!RegisterEnt(e->child))
    return BEHAVIOR_FAILURE;


  SetState(e->child,STATE_IDLE,NULL);
  return BEHAVIOR_SUCCESS;

}

BehaviorStatus BehaviorSelectHelpfulShape(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(e->child)
    return BEHAVIOR_SUCCESS;


  ShapeID shape = SelectHelpfulShape();

  TraceLog(LOG_INFO,"New Shape Selected: %i",shape);
  EntSetOwner(InitEnt(GetObjectInstanceByShapeID(shape)),e,false,NULL);

  return BEHAVIOR_SUCCESS;

}

BehaviorStatus BehaviorSelectShape(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(e->child)
    return BEHAVIOR_SUCCESS;

  ShapeID shape = SelectShapeFromRange(SHAPE_COLOR_NONE, SHAPE_TYPE_SQUARE,WorldGetPossibleShape());

  EntSetOwner(InitEnt(GetObjectInstanceByShapeID(shape)),e,false,NULL);

  return BEHAVIOR_SUCCESS;

}

BehaviorStatus BehaviorCheckChildState(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(!e->child)
    return BEHAVIOR_FAILURE;

  if(e->child->state==params->turn)
    return BEHAVIOR_SUCCESS;

  return BEHAVIOR_RUNNING;

}
 
BehaviorStatus BehaviorCheckOwnersState(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(!e->owner && e->type == ENT_SHAPE)
    return BEHAVIOR_FAILURE;
 
  ent_t* shape_pool[GRID_WIDTH * GRID_HEIGHT];

  int num_shapes = WorldGetEnts(shape_pool,FilterEntShape, NULL);
  for (int i = 0; i < num_shapes; i++){
    if(!shape_pool[i]->owner)
      return BEHAVIOR_RUNNING;
    
    if(!CanChangeState(shape_pool[i]->owner->state, params->state))
      return BEHAVIOR_RUNNING;
  }

  return BEHAVIOR_SUCCESS;

}

BehaviorStatus BehaviorClearMatchState(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;
/*
  if(!e->matches)
    return BEHAVIOR_SUCCESS;

  //EntClearMatches(e);
*/
  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorCheckOthersState(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  ent_t* shape_pool[GRID_WIDTH * GRID_HEIGHT];
  int num_shapes = WorldGetEnts(shape_pool,FilterEntShape, NULL); 
  for (int i = 0; i < num_shapes; i++){
    if(e->uid = shape_pool[i]->uid)
      continue;

    if(!CanChangeState(shape_pool[i]->state, params->state))
      return BEHAVIOR_RUNNING;
  }

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorProgressWorldState(behavior_params_t *params){
struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(TurnSetState(params->turn))
    return BEHAVIOR_SUCCESS;

  return BEHAVIOR_RUNNING;
}

BehaviorStatus BehaviorCheckSolutions(behavior_params_t *params){
struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorCheckMoves(behavior_params_t *params){
struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(e->control->moves<1)
    return BEHAVIOR_FAILURE;

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorCheckWorldState(behavior_params_t *params){
struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(TurnGetState()==params->turn)
    return BEHAVIOR_SUCCESS;

  return BEHAVIOR_RUNNING;
}

BehaviorStatus BehaviorTickLeaf(behavior_tree_node_t *self, void *context) {
    behavior_tree_leaf_t *leaf = (behavior_tree_leaf_t *)self->data;
    if (!leaf || !leaf->action) return BEHAVIOR_FAILURE;
    leaf->params->owner = context;
    return leaf->action(leaf->params);
}

behavior_tree_node_t* BehaviorCreateLeaf(BehaviorTreeLeafFunc fn, behavior_params_t* params){
  behavior_tree_leaf_t *data = malloc(sizeof(behavior_tree_leaf_t));

  data->action = fn;
  data->params = params;
  
  behavior_tree_node_t* node = malloc(sizeof(behavior_tree_node_t));
  node->bt_type = BT_LEAF;
  node->tick = BehaviorTickLeaf;
  node->data = data;

  return node;
}

behavior_tree_node_t* BehaviorCreateSequence(behavior_tree_node_t **children, int count) {
    behavior_tree_sequence_t *data = calloc(1,sizeof(behavior_tree_sequence_t));
    data->children = children;
    data->num_children = count;
    data->current = 0;

    behavior_tree_node_t *node = malloc(sizeof(behavior_tree_node_t));
    node->bt_type = BT_SEQUENCE;
    node->tick = BehaviorTickSequence;
    node->data = data;
    return node;
}

behavior_tree_node_t* BehaviorCreateSelector(behavior_tree_node_t **children, int count) {
    behavior_tree_selector_t *data = malloc(sizeof(behavior_tree_selector_t));
    data->children = children;
    data->num_children = count;
    data->current = 0;

    behavior_tree_node_t *node = malloc(sizeof(behavior_tree_node_t));
    node->bt_type = BT_SELECTOR;
    node->tick = BehaviorTickSelector;
    node->data = data;
    return node;
}

behavior_tree_node_t* BehaviorCreateConcurrent(behavior_tree_node_t **children, int count) {
    behavior_tree_selector_t *data = malloc(sizeof(behavior_tree_selector_t));
    data->children = children;
    data->num_children = count;
    data->current = 0;

    behavior_tree_node_t *node = malloc(sizeof(behavior_tree_node_t));
    node->bt_type = BT_CONCURRENT;
    node->tick = BehaviorTickConcurrent;
    node->data = data;
    return node;
}

BehaviorStatus BehaviorTickSequence(behavior_tree_node_t *self, void *context) {
    behavior_tree_sequence_t *seq = (behavior_tree_sequence_t *)self->data;
    while (seq->current < seq->num_children) {
        BehaviorStatus status = seq->children[seq->current]->tick(seq->children[seq->current], context);
        if (status == BEHAVIOR_RUNNING) return BEHAVIOR_RUNNING;
        if (status == BEHAVIOR_FAILURE) {
            seq->current = 0;
            return BEHAVIOR_FAILURE;
        }
        seq->current++;
    }

    seq->current = 0;
    return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorTickSelector(behavior_tree_node_t *self, void *context) {
    behavior_tree_selector_t *sel = (behavior_tree_selector_t *)self->data;

    while (sel->current < sel->num_children) {
        BehaviorStatus status = sel->children[sel->current]->tick(sel->children[sel->current], context);
        if (status == BEHAVIOR_RUNNING) return BEHAVIOR_RUNNING;
        if (status == BEHAVIOR_SUCCESS) {
            sel->current = 0;
            return BEHAVIOR_SUCCESS;
        }
        sel->current++;
    }

    sel->current = 0;
    return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorTickConcurrent(behavior_tree_node_t *self, void *context) {
  behavior_tree_selector_t *sel = (behavior_tree_selector_t *)self->data;

  bool anyRunning = false;
  bool anySuccess = false;
  bool anyFailure = false;

  for (int i = 0; i < sel->num_children; i++) {
    BehaviorStatus status = sel->children[i]->tick(sel->children[i], context);
    if (status == BEHAVIOR_RUNNING) anyRunning = true;
    else if (status == BEHAVIOR_SUCCESS) anySuccess = true;
    else if (status == BEHAVIOR_FAILURE) anyFailure = true;
  }

  // Rule set: "success if all succeed"
  if (!anyRunning && !anyFailure) return BEHAVIOR_SUCCESS;
  if (anyRunning) return BEHAVIOR_RUNNING;
  return BEHAVIOR_FAILURE;

}

