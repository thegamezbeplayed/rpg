#include "game_process.h"

static combat_system_t COMBAT;

void InitCombatSystem(int cap){

  COMBAT.cap = cap;
  COMBAT.entries = calloc(cap,sizeof(interaction_t));
  HashInit(&COMBAT.map, cap*3);
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

  interaction_t* i = NULL;
  i = CombatGetEntry(c->exid);

  if(!i){
    CombatEnsureCap();
    interaction_t* i = &COMBAT.entries[COMBAT.count++];

    i->ctx = c;
    i->uid = c->exid;
    HashPut(&COMBAT.map, i->uid, i);
  }

  return i;
}

interaction_t* StartCombat(ent_t* agg, ent_t* tar, ability_t* a){
  combat_t* c = calloc(1,sizeof(combat_t));

  combat_exchange_i exid = CombatMakeExID(agg, tar->gouid, a->id, WorldGetTime());
  c->cctx[IM_AGGR] = CombatContext(agg, IM_AGGR, a);
  c->cctx[IM_TAR] = CombatContext(tar, IM_TAR, NULL);

  interaction_t* i = RegisterCombat(c);

  return i; 
}

combat_context_t* CombatContext(ent_t* e, InteractMember type, ability_t* a){
  combat_context_t* c = calloc(1, sizeof(combat_context_t));

  c->type = type;
  c->ctx[IP_OWNER] = ParamMakeObj(DATA_LOCAL_CTX, e->gouid, WorldGetContext(DATA_ENTITY, e->gouid));
  c->ctx[IP_ABILITY] = ParamMakeObj(DATA_ABILITY, a->id, a);


  if(a->item)
    c->ctx[IP_ITEM] = ParamMakeObj(DATA_ITEM, a->item->gouid, a->item);

  return c;  
}

void CombatOnStep(combat_t* c, BattleStep step){

  combat_context_t* a = c->cctx[IM_AGGR];
  combat_context_t* t = c->cctx[IM_TAR];
  param_t p_agg = a->ctx[IP_OWNER];
  param_t p_tar =t->ctx[IP_OWNER];
  ent_t* agg = ParamReadEnt(&p_agg);
  ent_t* tar = ParamReadEnt(&p_tar);
  switch(step){
    case BAT_HIT:
      break;
    case BAT_DMG:
      WorldEvent(EVENT_DAMAGE_TAKEN, tar, tar->uid);
      WorldEvent(EVENT_DAMAGE_DEALT, agg, agg->uid);
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
      return res;
      break;
    default:
      break;
  }

  combat_t* c = i->ctx;
  switch(c->phase){
    case COM_INIT:
      return CombatStepPhase(c, COM_BATTLE);
      break;
    case COM_BATTLE:
      return CombatStepPhase(c, COM_RESPONSE);
      break;
    case COM_RESPONSE:
      return CombatStepPhase(c, COM_END);
      break;
    case COM_END:
      break;

  }
}
