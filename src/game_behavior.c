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

  behavior_tree_node_t *out = NULL;
  if(data.bt_type == BT_LEAF)
    out = room_behaviors[id].func(parent_params);
  else{
    behavior_tree_node_t **kids = calloc(1,sizeof(*kids) * data.num_children);
    for (int j = 0; j < data.num_children; ++j)
      kids[j] = BuildTreeNode(data.children[j],parent_params);

    switch(data.bt_type){
      case BT_SEQUENCE:
        out = BehaviorCreateSequence(kids, data.num_children);
        break;
      case BT_SELECTOR:
        out = BehaviorCreateSelector(kids, data.num_children);
        break;
      case BT_CONCURRENT:
        out = BehaviorCreateConcurrent(kids, data.num_children);
        break;
      default:
        TraceLog(LOG_WARNING,"Behavior Node Type %d NOT FOUND!",data.bt_type);
        return NULL;
        break;
    }

  }

  out->id = id;

  return out;

}

behavior_tree_node_t* InitBehaviorTree( BehaviorID id){
  if(id ==BN_NONE)
    return NULL;
  behavior_tree_node_t* node = BehaviorGetTree(id);

  if(node != NULL)
    return node;

  TraceLog(LOG_WARNING,"<=====Behavior Tree %i not found=====>",id);
  return NULL;
}

BehaviorStatus BehaviorClearState(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(!LocalCheck(e->control->goal))
    e->control->goal = NULL;

  if(!LocalCheck(e->control->target))
    e->control->target = NULL;

  if(!LocalCheck(e->control->destination))
    e->control->destination = NULL;

  return BEHAVIOR_SUCCESS;
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

BehaviorStatus BehaviorCheckSenses(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorCheckInventory(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  for(int i = 0; i < INV_DONE; i++){
    InventorySetPrefs(e->inventory[i], e->control->behave_traits);
  }

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorCheckAbilities(behavior_params_t *params){
struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(e->control->pref == NULL)
    e->control->pref = EntChoosePreferredAbility(e);

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorCheckTarget(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(e->control->target == NULL)
    return BEHAVIOR_FAILURE;

  if(e->control->target->method == I_KILL)
    return BEHAVIOR_SUCCESS;

  return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorCheckAggro(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(e->control->target != NULL)
    return BEHAVIOR_SUCCESS;

  local_ctx_t* ctx = LocalGetThreat(e->local);  

  if(ctx == NULL)
    return BEHAVIOR_FAILURE;

  ent_t* enemy = ParamReadEnt(&ctx->other);

  if(enemy == NULL && e->control->target == NULL)
    return BEHAVIOR_FAILURE;

  if(e->control->target == NULL)
    e->control->target = ctx;

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorTargetGoal(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(e->control->goal == NULL)
    return BEHAVIOR_FAILURE;

  if(e->control->target == e->control->goal)
    return BEHAVIOR_SUCCESS;

  if(e->control->goal->other.type_id != DATA_ENTITY
      || e->control->goal->method != I_KILL)
    return BEHAVIOR_FAILURE;

  e->control->target = e->control->goal;

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorAcquireTarget(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  ent_t* target = NULL;
  
  if(e->control->target && e->control->target->other.type_id == DATA_ENTITY)
    target = ParamReadEnt(&e->control->target->other);
  else
    DO_NOTHING();

  if(target && CheckEntAvailable(target))
    return BEHAVIOR_SUCCESS;

  e->control->target = NULL;

  if(e->local->count == 0)
    return BEHAVIOR_FAILURE;
  local_ctx_t* ctx = LocalGetThreat(e->local);

  if(ctx == NULL)
    return BEHAVIOR_FAILURE;

  ent_t* aggro = ParamReadEnt(&ctx->other);

  if(!aggro)
    return BEHAVIOR_FAILURE;

  e->control->target = ctx;

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorMoveToTarget(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(!e->control->target)
    return BEHAVIOR_FAILURE;

   if(cell_distance(*e->control->target->pos, e->pos)<2)
    return BEHAVIOR_SUCCESS;


  int depth = EntGetTrackDist(e, e->control->target);
  bool found = false;
  e->control->target->path = StartRoute(e, e->control->target, depth, &found);
  if(!found)
    return BEHAVIOR_FAILURE;

  Cell dest = RouteGetNext(e, e->control->target->path);
  Cell next = cell_dir(e->pos, dest);

  action_t* a = InitActionMove(e, ACT_MAIN, next, 75);

  ActionStatus a_status = QueueAction(e->control->actions, a);

  if(a_status > ACT_STATUS_ERROR)
    return BEHAVIOR_FAILURE;

  return BEHAVIOR_RUNNING;
}

BehaviorStatus BehaviorAcquireDestination(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorMoveToDestination(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;
  
  if(!e->control->destination)
    return BEHAVIOR_FAILURE;
 
  if(cell_distance(*e->control->destination->pos, e->pos)<2)
    return BEHAVIOR_SUCCESS;
 
  int depth = MAX_PATH_LEN;
  bool found = false;
  e->control->destination->path = StartRoute(e, e->control->destination, depth, &found);
 
  if(!found){
    e->control->destination = NULL;
    return BEHAVIOR_FAILURE;
  }
  Cell dest = RouteGetNext(e, e->control->destination->path);
  if(cell_compare(dest, CELL_UNSET))
    return BEHAVIOR_FAILURE;

  Cell next = cell_dir(e->pos, dest);
  action_t* a = InitActionMove(e, ACT_MAIN, next, 50);

  ActionStatus a_status = QueueAction(e->control->actions, a);

  if(a_status > ACT_STATUS_ERROR)
    return BEHAVIOR_FAILURE;
 
  //e->control->destination = CELL_UNSET; 
  return BEHAVIOR_RUNNING;
}

BehaviorStatus BehaviorCanAttackTarget(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if( !e->control->target)
    return BEHAVIOR_FAILURE;

  if(e->control->pref == NULL)
    e->control->pref = EntChoosePreferredAbility(e);

  if(!e->control->pref)
    return BEHAVIOR_FAILURE;

  ent_t* tar = ParamReadEnt(&e->control->target->other);

  if(cell_distance(e->pos,tar->pos) >  e->control->pref->stats[STAT_REACH]->current)
    return BEHAVIOR_FAILURE;

  return BEHAVIOR_SUCCESS;
}
   
BehaviorStatus BehaviorCombatCheck(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  if(!e->control->target ||
      !LocalCheck(e->control->target)){
    params->state = STATE_IDLE;
    return BEHAVIOR_FAILURE;
  }

  if(e->control->pref == NULL)
    e->control->pref = EntChoosePreferredAbility(e);

  if(!e->control->pref || 
      !AbilityCanTarget(e->control->pref, e->control->target)){
    params->state = STATE_AGGRO;
    return BEHAVIOR_FAILURE;
  }

  params->state = STATE_STANDBY;

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorAttackTarget(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  ability_t* a = e->control->pref;

  if(!a)
    return BEHAVIOR_FAILURE;

  action_t* act = InitActionAttack(e, ACT_MAIN, e->control->target->other, 100);

  ActionStatus a_status = QueueAction(e->control->actions, act);

  if(a_status > ACT_STATUS_ERROR)
    return BEHAVIOR_FAILURE;

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorCheckNeeds(behavior_params_t *params){
  ent_t* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorCheckNeed(behavior_params_t *params){
  ent_t* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  if(e->control->goal == NULL)
    return BEHAVIOR_FAILURE;

  if(e->control->priority->type == PRIO_NEEDS)
    return BEHAVIOR_FAILURE;

  /*if(!HasResource(e->control->goal->resource, prio->resource))
    return BEHAVIOR_FAILURE;
*/
  return BEHAVIOR_SUCCESS;
}
  
BehaviorStatus BehaviorFindSafe(behavior_params_t *params){
 ent_t* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  if(e->control->target == NULL)
    return BEHAVIOR_FAILURE;

  if(e->control->target->method != I_FLEE)
    return BEHAVIOR_FAILURE;
  
  local_ctx_t *dest = e->control->destination; 

  if(!dest || dest->method != I_FLEE)
   dest = EntFindLocation(e, e->control->target, I_FLEE);

  if(!dest)
    return BEHAVIOR_FAILURE;
  dest->method = I_FLEE;
  e->control->destination = dest;

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorTrackResource(behavior_params_t *params){
 ent_t* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorSeekResource(behavior_params_t *params){
 ent_t* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  if(e->control->goal == NULL)
    return BEHAVIOR_FAILURE;

  if(e->control->goal == e->control->destination)
    return BEHAVIOR_SUCCESS;

  bool found;
   
  int depth = EntGetTrackDist(e, e->control->goal) + e->control->goal->dist;
  e->control->goal->path = StartRoute(e, e->control->goal, depth, &found);

  if(!found)
    return BEHAVIOR_FAILURE;

  e->control->destination = e->control->goal; 
  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorCheckResource(behavior_params_t *params){
 ent_t* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  if(e->control->goal == NULL)
    return BEHAVIOR_FAILURE;

  /*if(e->control->goal->how_to != ACTION_INTERACT)
    return BEHAVIOR_FAILURE;
*/
  if(e->control->goal->dist < 2)
    return BEHAVIOR_SUCCESS;

  return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorAcquireResource(behavior_params_t *params){
 ent_t* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  if(e->control->goal == NULL)
    return BEHAVIOR_FAILURE;

  action_t* a = NULL;
  switch(e->control->goal->method){
    case I_CONSUME:
      local_ctx_t* goal = e->control->goal;
      need_t* n = ParamReadNeed(&goal->need);

      if(n == NULL)
        return BEHAVIOR_FAILURE;

      n->goal = goal;
      a = InitActionFulfill(e, ACT_MAIN, n, 75);
      break;
    case I_KILL:
      e->control->target = e->control->goal;
      params->state = STATE_AGGRO;
      return BEHAVIOR_SUCCESS;
      break;
    default:
      TraceLog(LOG_WARNING,"=== BEHAVIOR ACQUIRE RES ===\n unknown method for %s", e->name);
      break;
  }

  if(a == NULL)
    return BEHAVIOR_FAILURE;

  ActionStatus a_status = QueueAction(e->control->actions, a);

  if(a_status > ACT_STATUS_ERROR)
    return BEHAVIOR_FAILURE;

  return BEHAVIOR_SUCCESS;

}

BehaviorStatus BehaviorGetPriority(behavior_params_t *params){
  ent_t* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  if(!e->control->priority)
    return BEHAVIOR_FAILURE;

  priority_t* prio = e->control->priority; 

  local_ctx_t* ctx = NULL;
  switch(prio->ctx.type_id){
    case DATA_NEED:
      need_t* n = ParamReadNeed(&prio->ctx);
      if(e->control->goal){
        if(HasResource(e->control->goal->resource, n->resource)){
          e->control->goal->need = prio->ctx;
          return BEHAVIOR_SUCCESS;
        }
      }

      ctx = EntLocateResource(e, n->resource);
      if(!ctx)
        return BEHAVIOR_FAILURE;

      ctx->need = prio->ctx;
      if(ctx->method == I_KILL){
        params->state = STATE_AGGRO;
        e->control->target = ctx;
      }
      else
        e->control->goal = ctx;
      break;
    case DATA_GOUID:
      game_object_uid_i other = ParamReadGOUID(&prio->ctx);
      ctx = LocalGetEntry(e->local, other);
      if(ctx){
        ctx->method = prio->method;
        e->control->target = ctx;
      }
      break;
    case DATA_LOCAL_CTX:
      ctx = ParamReadCtx(&prio->ctx);
      e->control->target = ctx;
      params->state = STATE_AGGRO;
      break;
  }

  if(!ctx)
    return BEHAVIOR_FAILURE;


  return BEHAVIOR_SUCCESS;

}

BehaviorStatus BehaviorCheckPriority(behavior_params_t *params){
  ent_t* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  params->state = STATE_IDLE;

  priority_t* prio = &e->control->priorities->entries[0];
  if(prio->score == 0)
    return BEHAVIOR_FAILURE;

  switch(prio->type){
    case PRIO_NEEDS:
      params->state = STATE_REQ;
      break;
    case PRIO_ENGAGE:
    case PRIO_FLEE:
      params->state = STATE_AGGRO;
      break;
    default:
      return BEHAVIOR_FAILURE;
      break;

  }

  e->control->priority = prio;
  return BEHAVIOR_SUCCESS;

}

BehaviorStatus BehaviorFillNeeds(behavior_params_t *params){
  ent_t* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;


  return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorTickLeaf(behavior_tree_node_t *self, void *context) {
    
  behavior_tree_leaf_t *leaf = (behavior_tree_leaf_t *)self->data;
    if (!leaf || !leaf->action)
      return BEHAVIOR_FAILURE;
    leaf->params->owner = context;
    leaf->params->owner->control->current = self->id;
    BehaviorStatus status = leaf->action(leaf->params);
    if(status == BEHAVIOR_FAILURE)
      leaf->params->owner->control->failure = self->id;

    return status;
}

behavior_tree_node_t* BehaviorCreateLeaf(BehaviorTreeLeafFunc fn, behavior_params_t* params){
  behavior_tree_leaf_t *data = calloc(1, sizeof(behavior_tree_leaf_t));

  data->action = fn;
  data->params = params;
  
  behavior_tree_node_t* node = calloc(1, sizeof(behavior_tree_node_t));
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

    behavior_tree_node_t *node = calloc(1, sizeof(behavior_tree_node_t));
    node->bt_type = BT_SEQUENCE;
    node->tick = BehaviorTickSequence;
    node->data = data;
    return node;
}

behavior_tree_node_t* BehaviorCreateSelector(behavior_tree_node_t **children, int count) {
    behavior_tree_selector_t *data = calloc(1, sizeof(behavior_tree_selector_t));
    data->children = children;
    data->num_children = count;
    data->current = 0;

    behavior_tree_node_t *node = calloc(1, sizeof(behavior_tree_node_t));
    node->bt_type = BT_SELECTOR;
    node->tick = BehaviorTickSelector;
    node->data = data;
    return node;
}

behavior_tree_node_t* BehaviorCreateConcurrent(behavior_tree_node_t **children, int count) {
    behavior_tree_selector_t *data = calloc(1, sizeof(behavior_tree_selector_t));
    data->children = children;
    data->num_children = count;
    data->current = 0;

    behavior_tree_node_t *node = calloc(1, sizeof(behavior_tree_node_t));
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

