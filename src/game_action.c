#include "game_types.h"
#include "game_process.h"
#include "game_ui.h"

MAKE_ADAPTER(StepState, ent_t*);

turn_action_manager_t ActionMan;

void ActionFree(action_t* a){
  for(int i = 0; i < ACT_PARAM_ALL; i++){
    if(a->params[i].type_id == DATA_NONE)
      continue;

    ParamFree(&a->params[i]);
  }

  GameFree("ActionFree", a);
}

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
  if(!ae || !be)
    return 0;

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

  ActionMan.phase = TURN_MAIN;
  ActionMan.next = TURN_END;
  WorldSubscribe(EVENT_PLAYER_INPUT, OnPlayerAction, &ActionMan); 
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

bool ActionSetStatus(action_t* a, ActionStatus status){
  if(a->status == status)
    return false;

  a->status = status;
  WorldEvent(EVENT_ACT_STATUS, a, a->id);

  return true;
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

    if(!ActionSetStatus(a, ACT_STATUS_NEXT))
      continue;

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
  ActionStatus s = a->status;
  if(a->params[ACT_PARAM_TAR].type_id != DATA_LOCAL_CTX)
    s = ACT_STATUS_BAD_DATA;

  bool prepared = false;
  ability_t* ab;
  ent_t* tar = NULL;
  if(s == ACT_STATUS_RUNNING){
    local_ctx_t* ctx = ParamReadCtx(&a->params[ACT_PARAM_TAR]);
    tar = ParamReadEnt(&ctx->other);
    switch(a->params[ACT_PARAM_ABILITY].type_id){
      case DATA_INT:
        AbilityID aid = ParamReadInt(&a->params[ACT_PARAM_ABILITY]);
        ab = EntFindAbility(a->owner, aid);
        break;
      case DATA_ABILITY:
        ab = ParamRead(&a->params[ACT_PARAM_ABILITY], ability_t);
        break;
    }
    prepared = ab!=NULL;
  }
  if(!prepared){
    s = ACT_STATUS_BAD_ATTACK;
  }
  else{
    AbilityUse(a->owner, ab, tar, NULL);
    s = ACT_STATUS_TAKEN;
  }

  ActionSetStatus(a, s);
  return a->status;
}

ActionStatus ActionInteract(action_t* a){
  InteractResult res = IR_NONE;

  ActionStatus status = ACT_STATUS_ERROR;
  param_t p = a->params[ACT_PARAM_NEED];
  param_t g = a->params[ACT_PARAM_INTER];
  if(p.type_id == DATA_NEED){
    need_t* n = ParamReadNeed(&p);
    res = EntMeetNeed(a->owner, n, g);
    status = ACT_STATUS_TAKEN;
  }

  ActionSetStatus(a ,status);

  return a->status;
}


ActionStatus ActionMove(action_t* a){
  param_t p = a->params[ACT_PARAM_STEP];

  ActionStatus s = a->status;
  if(p.type_id != DATA_CELL)
    s = ACT_STATUS_BAD_DATA;

  if(s == ACT_STATUS_RUNNING){
    Cell dest = ParamReadCell(&p);
    TileStatus tstat = EntGridStep(a->owner,dest);
    if(tstat >= TILE_ISSUES)
      s = ACT_STATUS_BLOCK;
    else
      s = ACT_STATUS_TAKEN;
  }
  
  ActionSetStatus(a,s);
  return a->status;
}

ActionStatus ActionRun(action_t* a){
  ActionStatus s = ACT_STATUS_RUNNING;
  if(!CheckEntAvailable(a->owner))
    s = ACT_STATUS_INVALID;

  if(s != ACT_STATUS_RUNNING)
    return ACT_STATUS_MISQUEUE;

  ActionSetStatus(a, s);
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
    action_t* a = ActionMan.round[phase].entries[i];
    ActionSetStatus(a, ActionRun(a));
    switch(a->status){
      case ACT_STATUS_TAKEN:
        WorldEvent(EVENT_ACT_TAKEN, a, a->id);
        break;
      default:
        break;

    }
  }
}

bool ActionTurnStep(void){
  if(ActionMan.round[ActionMan.phase].status != ACT_STATUS_DONE)
    return false;

  ActionMan.round[ActionMan.phase].status = ACT_STATUS_NONE;

  if(ActionMan.next < TURN_END){
    ActionMan.phase = ActionMan.next;
    ActionMan.next++;

    return true;
  }
  else{
    ActionMan.phase = ActionMan.next;
    ActionMan.next = TURN_MAIN;
    return true;
  }

 TraceLog(LOG_WARNING,"===== ACTION MANAGER TURN =====\nShouldnt reach here...");
 return false; 

}

void ActionManOnStatus(TurnPhase phase, ActionStatus s){
  switch(s){
    case ACT_STATUS_DONE:
      ActionTurnStep();
      UISync(FETCH_TURN);
      break;
  }
}

bool ActionManSetStatus(TurnPhase phase, ActionStatus next){
  if(ActionMan.round[phase].status == next)
    return false;

  if(ActionMan.round[phase].status > next)
    return false;

  ActionMan.round[phase].status = next;

  ActionManOnStatus(phase, next);
  return true;
}

void ActionPoolRestart(action_pool_t* p){
  p->valid = false;
  for (int i = 0; i < ACT_DONE; i++){
    p->queues[i]->charges = p->queues[i]->allowed;
    p->queues[i]->status = ACT_STATUS_NONE;
  }

}

void ActionManagerEndRound(TurnPhase phase){
  int count = ActionMan.round[phase].count;

  if (ActionMan.round[phase].status !=  ACT_STATUS_NONE)
    return;
  for(int i = 0; i < count; i++){
    action_t* a =ActionMan.round[phase].entries[i];
    if(a && a->owner && CheckEntAvailable(a->owner))
      ActionPoolRestart(a->owner->control->actions);

  }
  ActionMan.round[phase].count = 0;

}

void ActionManagerEndTurn(){
  for(int i = 0; i < TURN_ALL; i++)
    ActionManagerEndRound(i);

  WorldEvent(EVENT_TURN_END, &ActionMan, ActionMan.turn);
  ActionMan.turn++;
}

void ActionPhaseStep(TurnPhase phase){
  ActionStatus s = ActionMan.round[phase].status;
  ActionStatus next = ACT_STATUS_NONE;
  switch(phase){
    case TURN_NONE:
      ActionMan.next = TURN_MAIN;
      ActionTurnStep();
      break;
    case TURN_MAIN:
      switch(s){
        case ACT_STATUS_BUILD:
          if(ActionManSetStatus(phase, ACT_STATUS_QUEUED))
          ActionManagerBuildQueue(TURN_MAIN, ACT_MAIN);
          break;
        case ACT_STATUS_NEXT:
          next = ACT_STATUS_RUNNING;
          ActionManagerRunQueue(phase);
          break;
        case ACT_STATUS_TAKEN:
          next = ACT_STATUS_DONE;
          break;
      }
      break;
    
    case TURN_POST:
      next = ACT_STATUS_DONE;
      break;
    case TURN_END:
      if(!ActionManSetStatus(phase, ACT_STATUS_DONE))
        return;
      next = ACT_STATUS_NONE;
      ActionManagerEndTurn();
      WorldEndTurn();
      break;
  }

  ActionManSetStatus(phase, next);
}

void ActionManagerSync(void){
    ActionPhaseStep(ActionMan.phase);
}

action_t* InitAction(ent_t* e, ActionType type, ActionCategory cat, uint64_t gouid, int weight){

  uint64_t id = GameObjectMakeUID(e->name, 
      10*ActionMan.turn + ActionMan.phase,
      e->gouid);

  action_t* a = GameCalloc("InitAction", 1, sizeof(action_t));
  *a = (action_t) {
    .owner = e,
      .id = id,
      .type = type,
      .cat = cat,
      .weight = weight,
      .score = weight,
      .turn  = ActionMan.turn,
      .phase = ActionMan.phase,
  };

  return a;
}

action_t* InitActionByDecision(decision_t* d, ActionType t){
  if(d->params[ACT_PARAM_OWNER].type_id != DATA_ENTITY)
    return NULL;

  if(t == ACTION_NONE)
    t = d->decision;

  ent_t* e = ParamReadEnt(&d->params[ACT_PARAM_OWNER]);

  action_t* a = InitAction(e, t, ACT_MAIN, d->id, d->score);

  for(int i = 0; i < ACT_PARAM_ALL; i++){
    a->params[i] = ParamClone(&d->params[i]);
  }
  switch(t){
    case ACTION_MOVE:
      if(a->params[ACT_PARAM_STEP].type_id != DATA_CELL){
        if(a->params[ACT_PARAM_DEST].type_id != DATA_LOCAL_CTX)
          return NULL;

        local_ctx_t* dest = ParamReadCtx(&a->params[ACT_PARAM_DEST]);
        if(dest->path == NULL)
          return NULL;

        Cell next = RouteGetNext(e, dest->path);
        Cell step = cell_dir(e->pos,next);
        a->params[ACT_PARAM_STEP] =  ParamMake(DATA_CELL, sizeof(Cell),
            &step);

      }
      a->fn = ActionMove;
      break;
    case ACTION_ATTACK:
      a->fn = ActionAttack;
      break;
    case ACTION_INTERACT:
      a->fn = ActionInteract;
      break;
  }
  return a;
}

ActionStatus ActionValidate(action_t* a){
  ActionStatus status = a->status;
  switch(a->type){
    case ACTION_MOVE:
      if(a->params[ACT_PARAM_STEP].type_id != DATA_CELL){
        status = ACT_STATUS_BAD_DATA;    
        break;
        local_ctx_t* dest = ParamReadCtx(&a->params[ACT_PARAM_DEST]);
        if(!dest || dest->path == NULL){
          a->status = ACT_STATUS_BAD_DATA; 
          break;
        } 

        Cell step = ParamReadCell(&a->params[ACT_PARAM_STEP]);
        TileStatus tile = MapTileAvailable(a->owner->map, step);
        if(tile > TILE_SUCCESS){
          status = ACT_STATUS_BLOCK;
          break;
        }
      }
      break;
    case ACTION_ATTACK:
      if(a->params[ACT_PARAM_TAR].type_id != DATA_LOCAL_CTX){
        status = ACT_STATUS_BAD_DATA;
        break;
      }
      if(a->params[ACT_PARAM_ABILITY].type_id != DATA_ABILITY){
        status = ACT_STATUS_BAD_DATA;
        break;
      }
      local_ctx_t* tar = ParamReadCtx(&a->params[ACT_PARAM_TAR]);

      ability_t* abi = ParamRead(&a->params[ACT_PARAM_ABILITY],ability_t);
      if(!AbilityCanTarget(abi, tar))
       status = ACT_STATUS_BAD_ATTACK;
      break;
    case ACTION_INTERACT:
      break;
  }

  ActionSetStatus(a, status);
  return a->status;
}

BehaviorStatus ActionExecute(ActionType t, action_t* a){
  if(!a)
    return BEHAVIOR_FAILURE;

  ActionValidate(a);

  if(a->status == ACT_STATUS_NONE)
    QueueAction(a->owner->control->actions, a);

  return a->status==ACT_STATUS_QUEUED? BEHAVIOR_SUCCESS:BEHAVIOR_FAILURE;
}

action_queue_t* InitActionQueue(ent_t* e, ActionCategory cat, int cap){

  action_queue_t* q = GameCalloc("InitActionQueue", 1, sizeof(action_queue_t));

  int charges = (cat == ACT_MAIN)?1:0;
  *q = (action_queue_t) {
    .owner = e,
    .id = cat,
    .cap = cap,
    .charges = charges,
    .allowed = charges
  };

  q->entries = GameCalloc("InitActionQueue entries", q->cap, sizeof(action_t));

  return q;
}

action_t* ActionForPhase(action_queue_t* q, TurnPhase phase){
  if(q->charges <1)
    return NULL;

  for(int i = 0; i < q->count; i++){
    action_t* a = &q->entries[i];
    if(a->status > ACT_STATUS_QUEUED)
      continue;
    q->charges--;
    q->status = ACT_STATUS_NEXT;
    initiative_t* init = a->owner->control->speed[a->type];
    a->initiative = RollInitiative(init);
    return &q->entries[i];
  }

  return NULL;
}

action_pool_t* InitActionPool(ent_t* e){

  action_pool_t* p = GameCalloc("InitActionPool", 1, sizeof(action_pool_t));

  p->options[TURN_MAIN] = true;
  for(int i = 0; i < ACT_DONE; i++)
    p->queues[i] = InitActionQueue(e, i, 5);

  return p;
}

bool ActionQueueEnsureCap(action_queue_t *q){
  if(q->count < q->cap)
    return true;

  if(q->cap >= MAX_ACTIONS)
    return false;

  q->cap+=4;

  q->entries = GameRealloc("ActionQueueEnsureCap", q->entries, q->cap * sizeof(action_t));

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
  if (ActionSetStatus(t,ACT_STATUS_QUEUED)) 
    q->entries[q->count++] = *t;

  return t->status;
}

ActionStatus QueueAction(action_pool_t *p, action_t* t){
  action_queue_t* q = p->queues[t->cat];

  if(q->status >= ACT_STATUS_QUEUED)
    return ACT_STATUS_WAIT;

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

void OnPlayerAction(EventType event, void* data, void* user){
  action_t* a = data;
  switch(event){
    case EVENT_PLAYER_INPUT:
      if(ActionManSetStatus(a->phase, ACT_STATUS_BUILD))
        WorldTargetSubscribe(EVENT_ACT_STATUS, OnPlayerAction, a, a->id);

      InputSync(ActionMan.phase, ActionMan.turn);
      break;
    case EVENT_ACT_STATUS:        
      switch(a->status){
        case ACT_STATUS_TAKEN:
          ActionManSetStatus(a->phase, a->status);
          WorldUnsub(a->id);
          break;
        case ACT_STATUS_BUILD:
        case ACT_STATUS_QUEUED:
          TraceLog(LOG_INFO,"This was teh problem");
          break;
        case ACT_STATUS_NEXT:
        case ACT_STATUS_RUNNING:
          ActionManSetStatus(a->phase, a->status);
          break;
        default:
          TraceLog(LOG_WARNING,"==== GAME ACTIONS ===\n Unexpected player action Status %i",a->status);
          break;
      }
      break;
  }
}

action_slot_t* InitActionSlot(ActionSlot id, ent_t* owner, int rank, int cap){
  action_slot_t* a = GameCalloc("InitActionSlot", 1,sizeof(action_slot_t));

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
  a->abilities = GameCalloc("InitActionSlot entries", a->cap, sizeof(ability_t));
  return a;
}

void ActionSlotEnsureCapacity(action_slot_t* t){
  if (t->count < t->cap)
    return;
  
  t->cap *= 2;
  t->abilities = GameRealloc("ActionSlotEnsureCapacity", t->abilities,
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
  priorities_t* p = GameCalloc("InitPriorities", 1,sizeof(priorities_t));

  *p = (priorities_t){
    .owner = e,
      .cap = cap,
      .entries = GameCalloc("InitPriorities entries", cap, sizeof(priority_t))
  };

  return p;
}


void PrioritiesEnsureCap(priorities_t* t){
 if (t->count < t->cap)
    return;

  size_t new_cap = t->cap + 2;

  priority_t* new_entries =
    GameRealloc("PrioritiesEnsureCap", t->entries, new_cap * sizeof(priority_t));

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
  
  priority_t* exists = PrioritiesGetEntry(t, gouid);
  if(exists)  
    return exists;

  Interactive method = I_NONE;

  switch(type){
    case PRIO_ENGAGE:
    case PRIO_HELP:
      method = I_KILL;
      break;
    case PRIO_FLEE:
      method = I_FLEE;
      break;
    case PRIO_NEEDS:
      method = I_CONSUME;
      break;
  }

  PrioritiesEnsureCap(t);
  priority_t* p = &t->entries[t->count++];

  *p = (priority_t){
    .gouid = gouid,
    .type = type,
      .ctx = entry,
      .method = method,
      .score = 0,
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
  local_ctx_t *l = NULL; 
  switch(p->ctx.type_id){
    case DATA_GOUID:
      game_object_uid_i gouid = ParamReadGOUID(&p->ctx);
      l = LocalGetEntry(e->local, gouid);
      break;
    case DATA_LOCAL_CTX:
      l = ParamReadCtx(&p->ctx);
      break;
  }

  if(!l || l->other.type_id != DATA_ENTITY)
    return false;

  int score = p->score;
  if(l->awareness == 0)
    return p->score == 0;

  if(l->awareness >= 1 && !l->aggro)
    l->aggro = LocalAggroByCtx(l);

  int cr = l->scores[SCORE_CR];
  float flee = l->awareness;
  float engage = l->awareness;

  int cr_diff = abs(cr - e->props->cr);

  for (int i = 0; i < TREAT_DONE; i++){
    float t_mod = l->treatment[i];
    switch(i){
      case TREAT_KILL:
      case TREAT_EAT:
        if(cr < e->props->cr)
        engage += l->awareness * cr_diff*t_mod;
        if(l->aggro && l->aggro->initiated)
          engage += l->aggro->threat*t_mod;
        break;
      case TREAT_DEFEND:
        if(l->aggro)
          engage+= l->aggro->threat * t_mod;
        else if (cr < e->props->cr)
          engage+= cr * t_mod;
        else
          flee+= cr *t_mod;
        break;
      case TREAT_FLEE:
        flee += t_mod;
        if(!l->aggro){
          if(cr > e->props->cr)
            flee *= cr_diff;

        }
        else if(l->aggro && l->aggro->initiated)
          engage += (l->aggro->threat * l->treatment[TREAT_DEFEND]);
        else{
          engage += (l->aggro->threat * l->treatment[TREAT_DEFEND])/l->dist;
          flee += l->aggro->threat * t_mod;
        }
        break;
    }
  }

  switch(p->type){
    case PRIO_FLEE:
      p->score = flee;
      if(!l->aggro)
        l->aggro = LocalAggroByCtx(l);
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
    case PRIO_HELP:
      p->score = engage;
      break;
  }

  return p->score == score;
}


void PriorityEvent(EventType ev, void* edata, void* udata){
  priorities_t* table = udata;
  switch(ev){
    case EVENT_AGGRO:
      ent_t* e = edata;

      param_t p = ParamMakeObj(DATA_ENTITY, e->gouid, e);
      priority_t* eng = PriorityAdd(table, PRIO_ENGAGE, p);

      PriorityScoreCtx(eng, e);
      break;
  }

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
  for (int i = 0; i < t->count;){
    priority_t* p = &t->entries[i];
    if(!p || p->prune){
      PriorityPrune(t, i);

      continue;
    }

    i++;
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
