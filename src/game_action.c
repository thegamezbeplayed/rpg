#include "game_types.h"
#include "game_process.h"

MAKE_ADAPTER(StepState, ent_t*);

static turn_action_manager_t ActionMan;

int ActionCompareImpDsc(const void* a, const void* b){
  const action_t* A = (const action_t*)a;
  const action_t* B = (const action_t*)b;
  
  if (A->score > B->score) return -1;
  if (A->score < B->score) return  1;

  if (A->weight > B->weight) return -1;
  if (A->weight < B->weight) return  1;

  if(A->initiative > B->initiative) return -1;
  if(A->initiative < B->initiative) return  1;

  return 0;
}

int ActionCompareInitDsc(const void* a, const void* b){
  const action_t* A = *(const action_t* const*)a;
  const action_t* B = *(const action_t* const*)b;

  if(A->initiative > B->initiative) return -1;
  if(A->initiative < B->initiative) return  1;

  ent_t *be = B->owner, *ae = A->owner;
  if (be->attribs[ATTR_DEX]->val > ae->attribs[ATTR_DEX]->val) return -1;
  if (be->attribs[ATTR_DEX]->val < ae->attribs[ATTR_DEX]->val) return  1;


  if (A->score > B->score) return -1;
  if (A->score < B->score) return  1;

  return 0;
}

void ActionQueueSortByImportance(action_queue_t* q){
  if (!q || q->count <= 1)
    return;

  qsort(q->entries,
      q->count,
      sizeof(action_t),
      ActionCompareImpDsc);

}

void ActionQueueSortByInitiative(action_queue_t* q){
  if (!q || q->count <= 1)
    return;
  
  qsort(q->entries,
      q->count,
      sizeof(action_t),
      ActionCompareInitDsc);

}

void InitActionRound(ActionCategory cat, TurnPhase phase, int cap){
  ActionMan.round[phase] = (action_round_t){
    .cap = cap,
      .allowed = cat,
      .phase = phase,

  }; 
}

void InitActionManager(void){

  for (int i = 0; i < TURN_ALL; i++)
    InitActionRound(ACT_MAIN, i, MAX_PHASE_ACTIONS);

  ActionMan.phase = TURN_INIT;
  ActionMan.next = TURN_START;
}

bool ActionManagerInsert(action_t* a, TurnPhase phase){
  if(ActionMan.round[phase].count >= MAX_PHASE_ACTIONS)
    return false;

  ActionMan.round[phase].entries[ ActionMan.round[phase].count++] = a;

  return true;
}

void ActionPrune(action_queue_t* q, int i) {
  if(q->count > 1)
    q->entries[i] = q->entries[--q->count];
  else
    q->count--;
}

void ActionManagerBuildQueue(TurnPhase phase, ActionCategory cat){
  int status = ENT_STATUS_ALIVE;
  param_t ctx = ParamMake(DATA_INT, sizeof(int), &status);

  ent_t* ents[MAX_ENTS] = {0};

  int alive = WorldGetEnts(ents, FilterEntByStatus, &ctx);
  for (int i = 0; i < alive; i++){
    ent_t* e = ents[i];
    if(!e->control->actions->valid)
      ActionPoolSync(e->control->actions);

    action_queue_t* q = e->control->actions->queues[cat]; 
    action_t* a = ActionForPhase(q, phase);
    if(a == NULL)
      continue;

    q->status = ACT_STATUS_QUEUED;

    a->status = ACT_STATUS_NEXT;

    if(!ActionManagerInsert(a, phase))
      break;
  }

  if (phase == TURN_MAIN && ActionMan.round[phase].count > 1)
    TraceLog(LOG_INFO,"==== PHASE %i built with %i actions ====", phase, ActionMan.round[phase].count);
}

bool ActionHasStatus(action_pool_t* p, ActionStatus s){
  for(int i = 0; i < ACT_DONE; i++){
    if(p->queues[i]->status == s)
      return true;
  }

  return false;
}

ActionStatus ActionAttack(action_t* a){
  if(a->ctx.type_id != DATA_ENTITY)
    a->status = ACT_STATUS_BAD_DATA;

  bool prepared = false;
  ability_t* ab;
  ent_t* tar = NULL;
  if(a->status == ACT_STATUS_RUNNING){
    tar = ParamReadEnt(&a->ctx);

    if(tar)
      prepared = EntPrepareAttack(a->owner, tar, &ab);
  }
  if(!prepared){
    a->status = ACT_STATUS_BAD_ATTACK;
  }
  else{
    AbilityUse(a->owner, ab, tar, NULL);
    a->status = ACT_STATUS_TAKEN;
  }

  return a->status;
}

ActionStatus ActionInteract(action_t* a){
  InteractResult res = IR_NONE;

  ActionStatus status = ACT_STATUS_ERROR;
  switch(a->ctx.type_id){
    case DATA_NEED:
      need_t* n = ParamReadNeed(&a->ctx);
      res = EntMeetNeed(a->owner, n);
      status = ACT_STATUS_TAKEN;
      break;
  }

  a->status = status;

  return status;
}


ActionStatus ActionMove(action_t* a){
  if(a->ctx.type_id != DATA_CELL)
    return ACT_STATUS_BAD_DATA;

  Cell dest = ParamReadCell(&a->ctx);
  TileStatus tstat = EntGridStep(a->owner,dest);
  if(tstat >= TILE_ISSUES)
    a->status = ACT_STATUS_BLOCK;
  else
    a->status = ACT_STATUS_TAKEN;

  return a->status;
}

ActionStatus ActionRun(action_t* a){
  if(!CheckEntAvailable(a->owner))
    a->status = ACT_STATUS_INVALID;


  if(a->status != ACT_STATUS_NEXT)
    return ACT_STATUS_MISQUEUE;

  a->status = ACT_STATUS_RUNNING;
  ActionStatus res = ACT_STATUS_ERROR;
  res = a->fn(a);

  if(a->cb)
    a->cb(a->owner);

  a->owner->control->actions->valid = false;
  return res;
}

void ActionRoundSort(TurnPhase phase, int count){
  if (count <= 1)
    return;
  
  qsort(ActionMan.round[phase].entries,
      count,
      sizeof(action_t*),
      ActionCompareInitDsc);
  
}   

void ActionManagerRunQueue(TurnPhase phase){
  int count = ActionMan.round[phase].count;

  ActionRoundSort(phase,count);

  for (int i = 0; i < count; i++){
    ActionStatus res = ActionRun(ActionMan.round[phase].entries[i]);
    switch(res){
      case ACT_STATUS_INVALID:
      case ACT_STATUS_MISQUEUE:
      case ACT_STATUS_BAD_DATA:
        
    }
  }
}

bool ActionTurnStep(TurnPhase phase){
  if(ActionMan.phase == TURN_STANDBY && phase != TURN_STANDBY)
    return false;

  if(ActionMan.phase != TURN_STANDBY && phase == TURN_STANDBY){
    ActionMan.phase = TURN_STANDBY;

    return true;;
  }

  if(ActionMan.next < TURN_END){
    ActionMan.phase = ActionMan.next;
    ActionMan.next++;

    return true;
  }
  else{
    ActionMan.phase = ActionMan.next;
    ActionMan.next = TURN_INIT;
    return true;
  }

 TraceLog(LOG_WARNING,"===== ACTION MANAGER TURN =====\nShouldnt reach here...");
 return false; 

}

void ActionPoolRestart(action_pool_t* p){
  p->valid = false;
  for (int i = 0; i < ACT_DONE; i++){
    p->queues[i]->charges = p->queues[i]->allowed;
    p->queues[i]->status = ACT_STATUS_DONE;
  }

}

void ActionManagerEndRound(TurnPhase phase){
  int count = ActionMan.round[phase].count;
  for(int i = 0; i < count; i++){
    action_t* a =ActionMan.round[phase].entries[i];
    if(CheckEntAvailable(a->owner))
      ActionPoolRestart(a->owner->control->actions);

  }
  ActionMan.round[phase].count = 0;

}

void ActionManagerReset(){
  for(int i = 0; i < TURN_ALL; i++)
    ActionManagerEndRound(i);
}

void ActionManagerEndTurn(){
  if(!ActionTurnStep(TURN_STANDBY))
    return;

  ActionManagerReset();
  ActionMan.turn++;
}

void ActionPhaseStep(TurnPhase phase){

  switch(phase){
    case TURN_NONE:
      ActionMan.next = TURN_INIT;
      ActionTurnStep(TURN_STANDBY);
      break;
    case TURN_INIT:
      if(ActionTurnStep(TURN_STANDBY)){
        ActionManagerBuildQueue(TURN_START, ACT_BONUS);
        ActionManagerBuildQueue(TURN_MAIN, ACT_MAIN);
      }
      break;
    case TURN_START:
    case TURN_MAIN:
      if(ActionTurnStep(TURN_STANDBY))
        ActionManagerRunQueue(phase);
      break;
    case TURN_STANDBY:
      ActionTurnStep(TURN_STANDBY);
      break;
    case TURN_POST:
      ActionTurnStep(TURN_STANDBY);
      break;
    case TURN_END:
      ActionManagerEndTurn();
      WorldEndTurn();
      break;
  }
}

TurnPhase ActionManagerPreSync(void){
  bool input = InputCheck(ActionMan.phase, ActionMan.turn);

  if(ActionMan.phase == TURN_INIT && !input)
    return TURN_STANDBY;

  ActionPhaseStep(ActionMan.phase);
    return ActionMan.phase;
}

void ActionManagerSync(void){
  InputSync(ActionMan.phase, ActionMan.turn);
}

action_t* InitAction(ent_t* e, ActionType type, ActionCategory cat, param_t ctx, int weight){
  action_t* a = calloc(1, sizeof(action_t));

  uint64_t id = hash_combine_64(e->gouid, ctx.gouid);

  *a = (action_t) {
    .owner = e,
      .id = id,
      .type = type,
      .cat = cat,
      .ctx = ctx,
      .weight = weight,
      .score = weight,
      .turn  = ActionMan.turn,
      .phase = ActionMan.next,
  };

  return a;
}

action_t* InitActionAttack(ent_t* e, ActionCategory cat, param_t tar, int weight){
    action_t* a = InitAction(e, ACTION_ATTACK, cat, tar, weight);

    a->fn = ActionAttack;

    a->cb = StepState_Adapter;
    return a;
}

action_t* InitActionFulfill(ent_t* e, ActionCategory cat, need_t* n, int weight){


  param_t ctx = ParamMake(DATA_NEED, sizeof(need_t), n);
  action_t* a = InitAction(e, ACTION_INTERACT, cat, ctx, weight);

  a->fn = ActionInteract;
  a->cb = StepState_Adapter;

  return a;
}

action_t* InitActionInteract(ent_t* e, ActionCategory cat, local_ctx_t* ctx, int weight){
  

  uint64_t id = hash_combine_64(e->gouid, ctx->gouid);


  action_t* a = InitAction(e,  ACTION_MOVE, cat, ctx->other, weight);
  
  a->fn = ActionInteract;

  a->cb = StepState_Adapter;
  return a;
}

action_t* InitActionMove(ent_t* e, ActionCategory cat, Cell dest, int weight){

  int id = IntGridIndex(dest.x, dest.y);

  param_t ctx = ParamMake(DATA_CELL, sizeof(Cell), &dest);

  ctx.gouid = id;
  action_t* a = InitAction(e, ACTION_MOVE, cat, ctx, weight);

  a->fn = ActionMove;
}

action_queue_t* InitActionQueue(ent_t* e, ActionCategory cat, int cap){

  action_queue_t* q = calloc(1, sizeof(action_queue_t));

  int charges = (cat == ACT_MAIN)?1:0;
  *q = (action_queue_t) {
    .owner = e,
    .id = cat,
    .cap = cap,
    .charges = charges,
    .allowed = charges
  };

  q->entries = calloc(q->cap, sizeof(action_t));

  return q;
}

action_t* ActionForPhase(action_queue_t* q, TurnPhase phase){
  if(q->charges <1)
    return NULL;

  if(q->count > 1)
    DO_NOTHING();

  for(int i = 0; i < q->count; i++){
    action_t* a = &q->entries[i];
    if(a->status > ACT_STATUS_QUEUED)
      continue;
    q->charges--;
    initiative_t* init = a->owner->control->speed[a->type];
    a->initiative = RollInitiative(init);
    return &q->entries[i];
  }

  return NULL;
}

action_pool_t* InitActionPool(ent_t* e){

  action_pool_t* p = calloc(1, sizeof(action_pool_t));

  for(int i = 0; i < ACT_DONE; i++)
    p->queues[i] = InitActionQueue(e, i, 5);

  return p;
}

bool ActionQueueEnsureCap(action_queue_t *q){
  if(q->count < q->cap)
    return true;

  if(q->cap >= MAX_ACTIONS)
    return false;

  q->cap++;

  q->entries = realloc(q->entries, q->cap * sizeof(action_t));

  return true;
}

action_t* ActionFindByID(action_queue_t *q, action_t* t){
  for(int i = 0; i < q->count; i++){
    if(q->entries[i].id == t->id)
      return &q->entries[i];
  }

  return NULL;
}

ActionStatus AddAction(action_queue_t *q, action_t* t){
  t->status = ACT_STATUS_QUEUED; 
  q->entries[q->count++] = *t;

  return t->status;
}

ActionStatus QueueAction(action_pool_t *p, action_t* t){
  action_queue_t* q = p->queues[t->cat];

  action_t* dupe = ActionFindByID(q,t);

  if(dupe){
    dupe->weight = t->weight;
    dupe->initiative = t->initiative;

    return dupe->status;

  }

  if(!ActionQueueEnsureCap(q))
    return ACT_STATUS_FULL;


  
  return AddAction(q, t);
}


bool ActionSync(action_t* a){
  if(!a || a->status >= ACT_STATUS_TAKEN)
    return false;


  return true;
}

void ActionQueueSync(action_queue_t* q){

  for (int i = 0; i < q->count; i++){
    action_t* a = &q->entries[i];
    if(ActionSync(&q->entries[i]))
      continue;

    ActionPrune(q, i);
  }

  if(q->count < 2)
    return;

  ActionQueueSortByImportance(q);
}

void ActionPoolSync(action_pool_t* p){
  if(p->valid)
    return;

  for(int i = 0; i < ACT_DONE; i++)
    ActionQueueSync(p->queues[i]);

  p->valid = true;
}
/*
bool ActionPlayerAttack(ent_t* e, ActionType type, KeyboardKey k,ActionSlot slot){
 if(!EntCanTakeAction(e))
    return false;

  if(cell_compare(e->facing,CELL_EMPTY) || cell_compare(e->facing,CELL_UNSET))
    return false;

  TileStatus* status =malloc(sizeof(TileStatus));
  
  ability_t* a = e->slots[slot]->abilities[0];
  ent_t* target = NULL;
  switch(a->targeting){
    case DES_NONE:
      target = MapGetOccupant(e->map, e->facing, status);
      break;
    case DES_SELF:
      target = e;
    default:
      break;
  }

  if(target)
    return AbilityUse(e, e->slots[slot]->abilities[0], target, NULL);

  return false;
}
*/


bool ActionMakeSelection(Cell start, int num, bool occupied){
  ScreenActivateSelector(start,num,occupied, ActionSetTarget);
}
/*
bool ActionInput(void){
  return false;
  if(player->state == STATE_SELECTION)
    return false;

  ActionType acted = ACTION_NONE;
  for(int i = 0; i < ACTION_DONE; i++){
    if(acted>ACTION_NONE)
      break;

    ActionType a = action_keys[i].action;
    ActionKeyCallback fn = action_keys[i].fn;
    for(int j = 0; j<action_keys[i].num_keys; j++){
      KeyboardKey k = action_keys[i].keys[j];
      if(!IsKeyDown(k))
        continue;

      int binding = action_keys[i].binding;
      switch(action_keys[i].action){
        case ACTION_MOVE:
          if(fn(player,a,k,binding))
            acted = ACTION_MOVE;
          break;
        case ACTION_ATTACK:
        case ACTION_MAGIC:
        case ACTION_ITEM:
          ability_t* ability = player->slots[binding]->abilities[0];
          switch(ability->targeting){
            case DES_SELECT_TARGET:
            case DES_MULTI_TARGET:
              ent_t* pool[8];
              int dist = ability->reach;
              Rectangle r = Rect(player->pos.x - dist, player->pos.y - dist, dist*2,2* dist);
               * int num_near = WorldGetEnts(pool, FilterEntInRect,&r);
              if(num_near < 2)
                break;
              else{
                SetState(player, STATE_SELECTION,NULL);
                SetAction(player, a, ability, ability->targeting);
                ActionMakeSelection(player->facing, ability->reach ,true);
              //}
              break;
            case DES_NONE:
            case DES_FACING:
            default:
              if(fn(player,a,k,binding))
                acted = a;
              break;
          }
      }
    }
  }

  if(acted > ACTION_NONE)
    return ActionTaken(player,acted);
  else
   return false;
    
}
*/
bool TakeAction(ent_t* e, action_turn_t* action){
  if(!action->fn(e,action->action,action->cb)){
    if(e->state == STATE_SELECTION)
      SetState(e,STATE_IDLE,NULL);
    return false;
  }

  if(action->cb)
    action->cb(e,action->action);

//  return ActionTaken(e, action->action);
}

ActionType ActionGetEntNext(ent_t* e){
  ActionType next = ACTION_NONE;

  for(int i = 0; i < ACTION_DONE; i++){
    if(!e->actions[i]->on_deck)
      continue;

    if(e->actions[i]->context)
      return i;
  }

  return next;
}

bool SetAction(ent_t* e, ActionType a, void *context, DesignationType targeting){
  if(e->actions[a]->action == ACTION_NONE)
    return false;

  for (int i = 0; i < ACTION_DONE;i++){
    if(e->actions[i]->on_deck && i!=a)
      return false;

  }

  e->actions[a]->targeting = targeting;
  e->actions[a]->num_targets = 0;
  //e->actions[a]->targets = ;
  e->actions[a]->context = context;
  e->actions[a]->on_deck = true;
  
  return true;
}

void ActionSetTarget(ent_t* e, ActionType a, void* target){

  map_cell_t* tile = target;
  ActionType next = ActionGetEntNext(e);
  action_turn_t* action = e->actions[next];

  action_target_t* at = calloc(1,sizeof(action_target_t));
  at->type = action->targeting;
  
  switch(action->targeting){
    case DES_SELECT_TARGET:
    case DES_MULTI_TARGET:
      at->target.mob = tile->occupant;
      break;
    default:
      break;
  }
      
  action->targets[action->num_targets++] = at;
}

bool ActionMultiTarget(ent_t* e, ActionType a, OnActionCallback cb){
  action_turn_t* inst = e->actions[a];
  if(!inst || !inst->context)
    return false;

  ability_t* ab = (ability_t*)inst->context;

  bool success = false;

  for (int i = 0; i < inst->num_targets; i++){
    switch(ab->targeting){
      case DES_SELECT_TARGET:
      case DES_MULTI_TARGET:
        ent_t* target = inst->targets[i]->target.mob;
        success = AbilityUse(e,ab,target,NULL)||success;
        break;
      default:
        break;
    }
  }

  if(!success)
    TraceLog(LOG_WARNING,"DEBUG");
  return success;
}

action_slot_t* InitActionSlot(ActionSlot id, ent_t* owner, int rank, int cap){
  action_slot_t* a = calloc(1,sizeof(action_slot_t));

  *a = (action_slot_t){
    .id     = id,
    .owner  = owner,
      .rank = rank,
      .cap  = cap,
  };

  define_slot_actions def = SLOTS_ALLOWED[id];

  for(int i = 0; i < ACTION_SLOTTED; i++)
    a->allowed[i] = def.allowed[i];

  a->resource = def.resource;
  a->abilities = calloc(a->cap, sizeof(ability_t));
  return a;
}

void ActionSlotEnsureCapacity(action_slot_t* t){
  if (t->count < t->cap)
    return;
  
  t->cap *= 2;
  t->abilities = realloc(t->abilities,
      t->cap * sizeof(ability_t*));
} 

bool ActionSlotAdd(action_slot_t* owner, ability_t* a){
  ActionSlotEnsureCapacity(owner);

  owner->abilities[owner->count++] = a;

  owner->space-= a->size;

  return true;
}

ActionSlot ActionSlotGetAvailable(ent_t* owner, ActionType type, int size){
  for(int i = 0; i < SLOT_ALL; i++){
    if(!owner->slots[i]->allowed[type])
      continue;

    if(size > owner->slots[i]->space)
      continue;

    return i;
  }

  return SLOT_NONE;
}

bool ActionSlotAddAbility(ent_t* owner, ability_t* a){
  ActionSlot slot = ActionSlotGetAvailable(owner, a->action, a->size);

  if(slot == SLOT_NONE)
    return false;

  if( ActionSlotAdd(owner->slots[slot], a)){
    owner->num_abilities++;
    return true;
  }

  return false;
  
}

int ActionSlotCompareDesc(const void* a, const void* b){
  // Primary: threat
  const int A = *(const int*)a;
  const int B = *(const int*)b;
  
  if (A > B) return -1;
  if (B < B) return  1;

  return 0;
}

void ActionSlotSortByPref(ent_t* owner, int *pool, int count){
  for(int i = 0; i < count; i++)
    pool[i] = owner->slots[i]->pref;

  qsort(pool, count, sizeof(int),ActionSlotCompareDesc);
}

priorities_t* InitPriorities(ent_t* e, int cap){
  priorities_t* p = calloc(1,sizeof(priorities_t));

  *p = (priorities_t){
    .owner = e,
      .cap = cap,
      .entries = calloc(cap, sizeof(priority_t))
  };

  return p;
}


void PrioritiesEnsureCap(priorities_t* t){
 if (t->count < t->cap)
    return;

  size_t new_cap = t->cap + 2;

  priority_t* new_entries =
    realloc(t->entries, new_cap * sizeof(priority_t));

  if (!new_entries) {
    // Handle failure explicitly
    //TraceLog(LOG_WARNING,"==== LOCAL CONTEXT ERROR ===\n REALLOC FAILED");
  }

  t->entries = new_entries;
  t->cap = new_cap;

}

priority_t* PrioritiesGetEntry(priorities_t* table, game_object_uid_i other){
  for (int i = 0; i < table->count; i++) {
    priority_t* ctx = &table->entries[i];
    if (ctx->gouid == other)
      return ctx;
  }

  return NULL;
}

priority_t* PriorityAdd(priorities_t* t, Priorities type, param_t entry){

  game_object_uid_i gouid = hash_combine_64(t->owner->gouid,
      hash_combine_64(type, entry.gouid));
  
  if(PrioritiesGetEntry(t, gouid))
    return NULL;

  Interactive method = I_NONE;

  switch(type){
    case PRIO_ENGAGE:
      method = I_KILL;
      break;
    case PRIO_FLEE:
      method = I_FLEE;
      break;
  }

  PrioritiesEnsureCap(t);
  priority_t* p = &t->entries[t->count++];

  *p = (priority_t){
    .gouid = gouid,
    .type = type,
      .ctx = entry,
      .method = method
  };

  return p;

}
void PriorityPrune(priorities_t* t, int i){
  if(t->count > 1)
    t->entries[i] = t->entries[--t->count];
  else
    t->count--;
}

bool PriorityScoreCtx(priority_t* p, ent_t* e){
  game_object_uid_i gouid = ParamReadGOUID(&p->ctx);

  local_ctx_t *l = LocalGetEntry(e->local, gouid);
  if(l->other.type_id != DATA_ENTITY)
    return false;

  int score = p->score;
  if(l->awareness == 0)
    return p->score == 0;

  float flee = l->awareness;
  float engage = l->awareness;

  for (int i = 0; i < TREAT_DONE; i++){
    switch(i){
      case TREAT_KILL:
      case TREAT_EAT:
        engage += l->treatment[i];
        if(l->aggro && l->aggro->initiated)
          engage += l->treatment[i];
        break;
      case TREAT_DEFEND:
        if(l->awareness > 1)
          engage+= l->treatment[i] / l->dist;
        break;
      case TREAT_FLEE:
        if(!l->aggro || !l->aggro->initiated)
          flee += l->treatment[i];
        else
          engage += l->treatment[TREAT_DEFEND];
        break;
    }
  }

  flee *= (l->cr - e->props->cr);
  engage *= l->awareness * (e->props->cr - l->cr);

  switch(p->type){
    case PRIO_FLEE:
      p->score = flee;

      if(engage > 0){
        priority_t* eng = PriorityAdd(e->control->priorities, PRIO_ENGAGE, p->ctx);
        if(eng){
          eng->score = engage;
          return true;
        }
      }
      break;
    case PRIO_ENGAGE:
      p->score = engage;
      if(flee > 0){
        priority_t* f = PriorityAdd(e->control->priorities, PRIO_FLEE, p->ctx);
        if(f){
          f->score = flee;
          return true;
        }
      }
      break;
  }

  return p->score == score;
}

int PrioritiesCompareDesc(const void* a, const void* b){
  const priority_t* A = (const priority_t*)a;
  const priority_t* B = (const priority_t*)b;

  if(A->score > B->score) return -1;
  if(A->score < B->score) return  1;

  return 0;
}

void PrioritizePriorities(priorities_t* t){
  if(!t || t->count < 2)
    return;

  qsort(t->entries, t->count,
      sizeof(priority_t),
      PrioritiesCompareDesc);
}

void PrioritiesSync(priorities_t* t){
  bool changes = !t->valid;
  for (int i = 0; i < t->count; i++){
    priority_t* p = &t->entries[i];
    if(!p || p->prune){
      PriorityPrune(t, i);

      continue;
    }

    switch(p->ctx.type_id){
      case DATA_NEED:
        need_t* n = ParamReadNeed(&p->ctx);
        if(p->score == n->prio)
          continue;

        p->score = n->prio;
        changes = true;
        break;
      case DATA_GOUID:
        if(PriorityScoreCtx(p, t->owner))
          changes = true;
        break;
    }
  }

  if(!changes)
    return;

  PrioritizePriorities(t);
  t->valid = true;
}
