#include "game_process.h"

static combat_system_t COMBAT;

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
