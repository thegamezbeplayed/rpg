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
        .o_state = data.state,
        .o_action = ACTION_NONE,
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

BehaviorStatus BehaviorChangeState(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  if(!e->control->next)
    return BEHAVIOR_FAILURE;

  if(SetState(e, e->control->next,NULL))
    return BEHAVIOR_SUCCESS;
    
  return BEHAVIOR_FAILURE;

}

BehaviorStatus BehaviorCheckReady(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  for(int i = 0; i < INV_DONE; i++){
    InventorySetPrefs(e->inventory[i], e->control->behave_traits);
  }

  if(e->control->pref == NULL)
    e->control->pref = EntChoosePreferredAbility(e);

  e->control->next = params->state;
  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorCheckAggro(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;


  return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorAcquireDestination(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  param_t f;
  Score stype = SCORE_CR;
  Score ctype = SCORE_PATH;
  ActionParam p = ACT_PARAM_NONE;
  switch(e->state){
    case STATE_IDLE:
      uint64_t dat = SPEC_RELATE_POSITIVE;
      f = ParamMake(DATA_UINT64, sizeof(uint64_t), &dat);
      p = ACT_PARAM_REL;
      break;

  }

  EntityState s = e->state;
  if(p == ACT_PARAM_NONE)
    return BEHAVIOR_FAILURE;

  bool running = false;
  e->control->decider[s] = StartDecision(&e->control->decider[s], 32, e, s, &running);
  if(!running){
    e->control->decider[s]->ouid = e->gouid;

    local_ctx_t* ctx_pool[32] = {0};

    int count = LocalContextFilter(e->local, 32, ctx_pool, f, p);

    for (int i = 0; i < count; i++){
      LocalSetPath(e, ctx_pool[i]);
      AddDestination( e->control->decider[s], ctx_pool[i], s, stype, ctype);
    }
  }

  if ( MakeDecision( e->control->decider[s], DecisionSortEconomic)){
    e->control->action = ACTION_MOVE;
    e->control->next = s;
    return BEHAVIOR_SUCCESS;
  }

  return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorCheckRange(behavior_params_t *params){
  struct ent_s* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  EntityState s = e->control->next;
  decision_t* sel = e->control->decider[s]->selected;

  if(sel->decision == ACTION_NONE)
    return BEHAVIOR_SUCCESS;

  if(EntCheckRange(e, sel)){
    e->control->next = sel->state;
    return BEHAVIOR_SUCCESS;
  }

  e->control->action = ACTION_MOVE;

  return BEHAVIOR_SUCCESS;
}

BehaviorStatus BehaviorExecuteDecision(behavior_params_t *params){
  ent_t* e = params->owner;
  if(!e || !e->control)
    return BEHAVIOR_FAILURE;

  decision_t* sel = e->control->decider[e->control->next]->selected;

  if(sel->decision == ACTION_NONE || sel->score == 0){
    if (sel->state == STATE_NONE)
      return BEHAVIOR_FAILURE;

    e->control->next = sel->state;
    return BEHAVIOR_SUCCESS;
  }

  return ActionExecute(sel, e->control->action);
}

BehaviorStatus BehaviorFillNeed(behavior_params_t *params){
  ent_t* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;

  decision_pool_t* prior_dec = e->control->decider[STATE_NONE];

  if(!prior_dec || prior_dec->selected->state != e->state)
    return BEHAVIOR_FAILURE;

  if(prior_dec->selected->params[ACT_PARAM_NEED].type_id != DATA_NEED)
    return BEHAVIOR_FAILURE;

  param_t p_need = prior_dec->selected->params[ACT_PARAM_NEED];
  need_t* n = ParamReadNeed(&p_need);

  bool running = false;
  e->control->decider[e->state] = StartDecision(&e->control->decider[e->state], 32, e, e->state, &running);
  if(!running){
    e->control->decider[e->state]->ouid = e->gouid;

    local_ctx_t* ctx_pool[32] = {0};

    int count = EntGetCtxByNeed(e, n, 32, ctx_pool);

    for (int i = 0; i < count; i++){
      uint64_t res = n->resource;


      AddCandidate( e->control->decider[e->state], ctx_pool[i], ACT_PARAM_RES, SCORE_RES, SCORE_PATH);
    }
  }

    if(MakeDecision( e->control->decider[e->state], DecisionSortEconomic)){
      e->control->action = e->control->decider[e->state]->selected->decision;
      e->control->decider[e->state]->selected->params[ACT_PARAM_NEED] = p_need; 
      return BEHAVIOR_SUCCESS;
    }    

    return BEHAVIOR_FAILURE;
}

BehaviorStatus BehaviorGetPriority(behavior_params_t *params){
  ent_t* e = params->owner;
  if(!e)
    return BEHAVIOR_FAILURE;
  EntityState s = STATE_NONE;
  bool running = false;
  e->control->decider[s] = StartDecision(&e->control->decider[s], e->control->priorities->count, e, s, &running);

  if(!running) 
    for(int i = 0; i < e->control->priorities->count; i++)
      AddPriority(e->control->decider[s], &e->control->priorities->entries[i]);

  e->control->next = STATE_NONE;
  e->control->action = ACTION_NONE;
  if( MakeDecision( e->control->decider[s], DecisionSortByScore)){
    if(e->control->decider[s]->selected->score > 15)
      return BEHAVIOR_SUCCESS;
  }

  return BEHAVIOR_FAILURE;

}

BehaviorStatus BehaviorTickLeaf(behavior_tree_node_t *self, void *context) {
    
  behavior_tree_leaf_t *leaf = (behavior_tree_leaf_t *)self->data;
    if (!leaf || !leaf->action)
      return BEHAVIOR_FAILURE;
    if(context != leaf->params->owner){
      leaf->params->state = leaf->params->o_state;
      leaf->params->action = leaf->params->o_action;
    }
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

