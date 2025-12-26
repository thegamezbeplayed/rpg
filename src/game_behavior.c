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

BehaviorStatus BehaviorCheckAggro(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(e->control->target)
    return BEHAVIOR_SUCCESS;
}


BehaviorStatus BehaviorAcquireTarget(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(e->control->target && CheckEntAvailable(e->control->target))
    return BEHAVIOR_SUCCESS;

  e->control->target = NULL;
  if(!CheckEntAvailable(player))
    return BEHAVIOR_FAILURE;

  if(CellDistGrid(e->pos,player->pos) > e->stats[STAT_AGGRO]->current)
    return BEHAVIOR_FAILURE;

  ent_t* tar = player;
  Cell next;

  if(!FindPath(e->map, e->pos.x, e->pos.y, tar->pos.x, tar->pos.y, &next,50,TileBlocksSight))
    return BEHAVIOR_FAILURE;
  e->control->target = tar;
  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorMoveToTarget(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(!e->control->target || e->control->target->state == STATE_DIE)
    return BEHAVIOR_FAILURE;

  ent_t* tar = e->control->target;
  Cell next;
  if (!FindPath(e->map, e->pos.x, e->pos.y, tar->pos.x, tar->pos.y, &next,70,TileBlocksMovement))
    return BEHAVIOR_FAILURE;

  e->control->destination = cell_dir(e->pos,next);
  if(!SetAction(e,ACTION_MOVE,&e->control->destination,DES_NONE))
    return BEHAVIOR_FAILURE;

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorAcquireDestination(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;


  if(cell_compare(e->control->destination,CELL_UNSET))
    e->control->destination = random_direction();

  if(CellDistGrid(e->pos,e->control->start) > e->control->ranges[RANGE_ROAM])
    e->control->destination = cell_dir(e->pos, e->control->start);
  else{
    switch(rand()%4){
      case 0:
        e->control->destination = random_direction();
        break;
      case 1:
        e->control->destination = CELL_EMPTY;
        return BEHAVIOR_SUCCESS;
        break;
      default:
        break;
    }
  }  

  Cell tar = CellInc(e->pos,e->control->destination);
  Cell next;
  if(!FindPath(e->map, e->pos.x, e->pos.y, tar.x, tar.y, &next,35,TileBlocksMovement)){
    e->control->destination = CELL_UNSET;
    return BEHAVIOR_FAILURE;
  }

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorMoveToDestination(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;
  
  if(cell_compare(e->control->destination,CELL_UNSET))
    return BEHAVIOR_FAILURE;
  
  if(!SetAction(e,ACTION_MOVE,&e->control->destination,DES_NONE)){
    e->control->destination = CELL_UNSET;
    return BEHAVIOR_FAILURE;
  }

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorCanAttackTarget(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if( !e->control->target)
    return BEHAVIOR_FAILURE;

  if(cell_distance(e->pos,e->control->target->pos) >  e->stats[STAT_REACH]->current)
    return BEHAVIOR_FAILURE;

  return BEHAVIOR_SUCCESS;

}
   
BehaviorStatus BehaviorCheckTurn(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;


  if(EntCanTakeAction(e))
    return BEHAVIOR_SUCCESS;

  return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorTakeTurn(behavior_params_t *params){
  struct ent_s* e = params->owner;

  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  ActionType next = ActionGetEntNext(e);

  if(next == ACTION_NONE)
    return BEHAVIOR_FAILURE;

  action_turn_t* action = e->actions[next];
  if(TakeAction(e,action))
    return BEHAVIOR_SUCCESS;

  return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorAttackTarget(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  ability_t* a = EntChooseWeightedAbility(e,e->stats[STAT_STAMINA]->current, SLOT_ATTACK);
  if(!a){
    a = EntChooseWeightedAbility(e, e->stats[STAT_ENERGY]->current, SLOT_SPELL);
  }
 if(SetAction(e,ACTION_ATTACK,a, a->targeting)) 
    return BEHAVIOR_SUCCESS;

  return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorCanSeeTarget(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  return BEHAVIOR_SUCCESS;
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

