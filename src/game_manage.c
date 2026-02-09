#include "game_process.h"

static combat_system_t COMBAT;

static activity_tracker_t ACT_TRACK;

void ActivityAddEntry(interaction_t* i){
  ACT_TRACK.entries[ACT_TRACK.head] = *i;
  ACT_TRACK.head = (ACT_TRACK.head + 1) % ACT_TRACK.cap;
  
  if (ACT_TRACK.count < ACT_TRACK.cap)
    ACT_TRACK.count++;
  /*
  entry->uid = i->uid;
  entry->event = i->event;
  entry->ctx = i->ctx;
  */
}

void OnActivityEvent(EventType event, void* data, void* user){

  interaction_t* i = data;

  ActivityAddEntry(i);
}

void InitActivities(int cap){
  ACT_TRACK.cap = cap;
  WorldTargetSubscribe(EVENT_COMBAT_ACTIVITY, OnActivityEvent, &ACT_TRACK, player->gouid);
}

void InitCombatSystem(int cap){

  COMBAT.cap = cap;
  COMBAT.entries = calloc(cap,sizeof(interaction_t));
  HashInit(&COMBAT.map, cap*2);
}

interaction_t* CombatGetEntry(game_object_uid_i other){
  return HashGet(&COMBAT.map, other);
}

void CombatEnsureCap(void){
  if(COMBAT.count < COMBAT.cap)
    return;

  size_t new_cap = COMBAT.cap + 64;

  interaction_t* new_entries = realloc(COMBAT.entries, new_cap * sizeof(interaction_t));

  if (!new_entries) {
    TraceLog(LOG_WARNING,"==== COMBAT SYSTEM ERROR ===\n REALLOC FAILED");
    return;
  }

  COMBAT.cap = new_cap;
  COMBAT.entries = new_entries;
}

interaction_t* RegisterCombat(combat_t* c){

  interaction_t* exists = CombatGetEntry(c->exid);

  if(exists)
    return exists;


  CombatEnsureCap();
  interaction_t* i = &COMBAT.entries[COMBAT.count++];

  i->ctx = c;
  i->uid = c->exid;
  HashPut(&COMBAT.map, i->uid, i);

  return i;
}

void OnCombatStep(interaction_t* i, InteractResult res){

  combat_t *combat = i->ctx;
  uint64_t agg_id = combat->cctx[IM_AGGR]->ctx[IP_OWNER].gouid;
  uint64_t tar_id = combat->cctx[IM_TAR]->ctx[IP_OWNER].gouid;
  WorldEvent(EVENT_COMBAT_ACTIVITY, i, agg_id);
}

interaction_t* StartCombat(ent_t* agg, ent_t* tar, ability_t* a){
  combat_t* c = calloc(1,sizeof(combat_t));

  combat_exchange_i exid = CombatMakeExID(agg, tar->gouid, a->id, WorldGetTime());
  c->cctx[IM_AGGR] = CombatContext(agg, IM_AGGR, a);
  
  ability_t* save = EntFindAbility(tar, ABILITY_ARMOR_SAVE);

  c->cctx[IM_TAR] = CombatContext(tar, IM_TAR, save);

  interaction_t* i = RegisterCombat(c);

  return i; 
}

combat_context_t* CombatContext(ent_t* e, InteractMember type, ability_t* a){
  combat_context_t* c = calloc(1, sizeof(combat_context_t));

  c->type = type;
  c->ctx[IP_OWNER] = ParamMakeObj(DATA_LOCAL_CTX, e->gouid, WorldGetContext(DATA_ENTITY, e->gouid));
  if(a){
    c->ctx[IP_ABILITY] = ParamMakeObj(DATA_ABILITY, a->id, a);

  if(a->item)
    c->ctx[IP_ITEM] = ParamMakeObj(DATA_ITEM, a->item->gouid, a->item);
  }
  return c;  
}

void CombatOnStep(combat_t* c, BattleStep step){

  combat_context_t* a = c->cctx[IM_AGGR];
  combat_context_t* t = c->cctx[IM_TAR];
  param_t p_agg = a->ctx[IP_OWNER];
  param_t p_tar =t->ctx[IP_OWNER];
  local_ctx_t* agg = ParamReadCtx(&p_agg);
  local_ctx_t* tar = ParamReadCtx(&p_tar);
  item_t* i = NULL;
  if(a->ctx[IP_ITEM].type_id == DATA_ITEM)
    i = ParamRead(&a->ctx[IP_ITEM], item_t);
  switch(step){
    case BAT_HIT:
      
      break;
    case BAT_DMG:
      WorldEvent(EVENT_DAMAGE_TAKEN, tar->other.data, tar->gouid);
      WorldEvent(EVENT_DAMAGE_DEALT, agg->other.data, agg->gouid);
      if(i)
        WorldEvent(EVENT_DAMAGE_DEALT, tar->other.data, i->fuid);
      break;
    case BAT_DONE:
      break;
  }
}

InteractResult CombatStepPhase(combat_t* c, CombatPhase phase){
  switch(c->phase){
    case COM_BATTLE:
      CombatOnStep(c, c->step);
      c->step++;
      if(c->step == BAT_DONE){
        c->phase++;
        c->step = BAT_NONE;
      }
      break;
    case BAT_DONE:
      c->phase = COM_INIT;
      break;
    default:
      c->phase++;
      break;
  }
}

InteractResult CombatStep(interaction_t* i, InteractResult res){

  switch (res){
    case IR_FAIL:
    case IR_CRITICAL_FAIL:
    case IR_ALMOST:
      break;
    default:
      combat_t* c = i->ctx;
      switch(c->phase){
        case COM_INIT:
          res = CombatStepPhase(c, COM_BATTLE);
          break;
        case COM_BATTLE:
          res = CombatStepPhase(c, COM_RESPONSE);
          break;
        case COM_RESPONSE:
          res = CombatStepPhase(c, COM_END);
          break;
        case COM_END:
          break;

      }
      break;
  }

  OnCombatStep(i, res);

  return res;
}
