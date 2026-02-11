#include "game_process.h"

static combat_system_t COMBAT;

static activity_tracker_t ACT_TRACK;

void ActivityAddEntry(activity_t* act){
  ACT_TRACK.entries[ACT_TRACK.head] = *act;
  ACT_TRACK.head = (ACT_TRACK.head + 1) % ACT_TRACK.cap;
  
  if (ACT_TRACK.count < ACT_TRACK.cap)
    ACT_TRACK.count++;
  /*
  entry->uid = i->uid;
  entry->event = i->event;
  entry->ctx = i->ctx;
  */
}

activity_t* InitActivity(EventType event, interaction_t* i){
  activity_t* act = calloc(1, sizeof(activity_t));

  param_t params[IM_DONE][IP_DONE];
  switch (event){
    case EVENT_COMBAT:
      combat_t* com = i->ctx;
      for(int i = 0; i < IM_DONE; i++)
        for (int j = 0; j < IP_DONE; j++)
          params[i][j] = com->cctx[i]->ctx[j];
      break;
  }

  act->tokens[TOKE_AGG] = params[IM_AGGR][IP_OWNER];
  act->tokens[TOKE_TAR] = params[IM_TAR][IP_OWNER];

  for(int i = 0; i < IM_DONE; i++){
    for (int j = 0;  j < IP_DONE; j++){
      switch(j){
        case IP_OWNER:
          break;
        case IP_ITEM:
          break;
        case IP_ABILITY:

          break;
        case IP_ACTION: 
          break;
      };
    }
  }

  return act;
}

void OnActivityEvent(EventType event, void* data, void* user){

  interaction_t* i = data;

  activity_t* act = InitActivity(event, i);

  ActivityAddEntry(act);
}

void InitActivities(int cap){
  ACT_TRACK.cap = cap;
  WorldTargetSubscribe(EVENT_COMBAT_ACTIVITY, OnActivityEvent, &ACT_TRACK, player->gouid);
}

line_item_t* ActivitiesAssignValues(int){
  //line_item_t* li = InitLineItem(

  //return li;
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
  i->event = EVENT_COMBAT;
  HashPut(&COMBAT.map, i->uid, i);

  return i;
}

void OnCombatStep(interaction_t* i, InteractResult res){

  i->result = res;
  combat_t *combat = i->ctx;
  uint64_t agg_id = combat->cctx[IM_AGGR]->ctx[IP_OWNER].gouid;
  uint64_t tar_id = combat->cctx[IM_TAR]->ctx[IP_OWNER].gouid;
  WorldEvent(EVENT_COMBAT_ACTIVITY, i, agg_id);
  if(agg_id != tar_id)
    WorldEvent(EVENT_COMBAT_ACTIVITY, i, tar_id);
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

  ability_t* dr = EntFindAbility(e, ABILITY_ARMOR_DR);
  c->type = type;
  c->ctx[IP_OWNER] = ParamMakeObj(DATA_LOCAL_CTX, e->gouid, WorldGetContext(DATA_ENTITY, e->gouid));
  if(a){
    c->ctx[IP_ABILITY] = ParamMakeObj(DATA_ABILITY, a->id, a);

  if(dr)
   c->ctx[IP_ARMOR] = ParamMakeObj(DATA_ABILITY, dr->id, dr);

  if(a->item)
    c->ctx[IP_ITEM] = ParamMakeObj(DATA_ITEM, a->item->gouid, a->item);
  }
  return c;  
}

InteractResult CombatDamage(combat_t *c, bool init){
  InteractResult result = IR_FAIL;  

  ability_sim_t* sim = ParamRead(&c->cctx[IM_AGGR]->ctx[IP_DMG], ability_sim_t);

  param_t p_agg = c->cctx[IM_AGGR]->ctx[IP_OWNER];
  local_ctx_t* a_ctx = ParamReadCtx(&p_agg);
  ent_t* agg = ParamReadEnt(&a_ctx->other);
  param_t p_tar = c->cctx[IM_TAR]->ctx[IP_OWNER];

  local_ctx_t* t_ctx = ParamReadCtx(&p_tar);
  ent_t* tar = ParamReadEnt(&t_ctx->other);


  param_t p_abi = c->cctx[IM_AGGR]->ctx[IP_ABILITY];
  ability_t* atk = ParamRead(&p_abi, ability_t);

  int damage = -1 * sim->final_dmg;
  tar->last_hit_by = agg;
  stat_t* damaged = tar->stats[atk->damage_to];
  if (StatChangeValue(tar, damaged, damage)){
    result = IR_SUCCESS;
    if(StatIsEmpty(damaged))
      result = IR_TOTAL_SUCC;

    EntAddAggro(tar, agg, damage, agg->props->base_diff, init);
    if(init)
      WorldEvent(EVENT_DAMAGE_TAKEN, atk, tar->gouid);

  }

  return result;
}

InteractResult CombatCalcDmg(combat_t* c){
  InteractResult result = IR_NONE;
  
  ability_t* dr = ParamRead(&c->cctx[IM_TAR]->ctx[IP_ARMOR], ability_t);

  ability_sim_t* sim = ParamRead(&c->cctx[IM_AGGR]->ctx[IP_DMG], ability_sim_t);
  
  param_t p_agg = c->cctx[IM_AGGR]->ctx[IP_OWNER];
  local_ctx_t* a_ctx = ParamReadCtx(&p_agg);
  ent_t* agg = ParamReadEnt(&a_ctx->other);
  param_t p_tar = c->cctx[IM_TAR]->ctx[IP_OWNER];

  local_ctx_t* t_ctx = ParamReadCtx(&p_tar);
  ent_t* tar = ParamReadEnt(&t_ctx->other);

  InteractResult dres = IR_FAIL;
  if(dr)
    dres = AbilityUse(tar, dr, agg, sim);

  if(dres == IR_TOTAL_SUCC)
    return IR_CRITICAL_FAIL;


  result = CombatDamage(c, true);
  

  return result;
}

InteractResult CombatCalcHit(combat_t* c){
  InteractResult res = IR_NONE;

  param_t p_agg = c->cctx[IM_AGGR]->ctx[IP_OWNER];
  local_ctx_t* a_ctx = ParamReadCtx(&p_agg);
  ent_t* agg = ParamReadEnt(&a_ctx->other);
  param_t p_tar = c->cctx[IM_TAR]->ctx[IP_OWNER];

  local_ctx_t* t_ctx = ParamReadCtx(&p_tar);
  ent_t* tar = ParamReadEnt(&t_ctx->other);

  param_t p_abi = c->cctx[IM_AGGR]->ctx[IP_ABILITY];
  ability_t* atk = ParamRead(&p_abi, ability_t);

  if(atk->resource>STAT_NONE && atk->cost > 0)
    if(!StatChangeValue(agg,agg->stats[atk->resource],-1*atk->cost))
      return IR_FAIL;

  param_t p_save = c->cctx[IM_TAR]->ctx[IP_ABILITY];
  ability_t* save = ParamRead(&p_save, ability_t);

  atk->stats[STAT_DAMAGE]->start(atk->stats[STAT_DAMAGE]);

  StatMaxOut(atk->stats[STAT_DAMAGE]);

  ability_sim_t* sim = atk->sim_fn(agg,atk,tar);
  c->cctx[IM_AGGR]->ctx[IP_DMG] = ParamMakeObj(DATA_DMG, atk->id, sim);
  res = AbilityUse(tar,save, agg, sim);

  return res;
}

InteractResult CombatOnStep(combat_t* c){
  BattleStep step = BAT_DONE;

  for(int i = 0; i < BAT_DONE; i++)
    if(c->step[i] == IR_NONE){
      step = i;
      break;
    }

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
      c->step[step] = CombatCalcHit(c);      
      break;
    case BAT_DMG:
      /*
      WorldEvent(EVENT_DAMAGE_TAKEN, tar->other.data, tar->gouid);
      WorldEvent(EVENT_DAMAGE_DEALT, agg->other.data, agg->gouid);
      if(i)
        WorldEvent(EVENT_DAMAGE_DEALT, tar->other.data, i->fuid);
      */
      c->step[step] = CombatCalcDmg(c);
      break;
    case BAT_DONE:
      c->step[step] = IR_DONE;
      break;
  }

  return c->step[step];
}

InteractResult CombatStepPhase(combat_t* c, CombatPhase phase){
  InteractResult res = IR_DONE;

  switch(phase){
    case COM_BATTLE:
      res = CombatOnStep(c);
      if(res == IR_DONE)
        c->phase[phase] = IR_DONE;
      break;
    default:
      break;
  }

  return res;
}

InteractResult CombatStep(interaction_t* i, InteractResult res){
  combat_t* c = i->ctx;

  switch (res){
    case IR_FAIL:
    case IR_CRITICAL_FAIL:
    case IR_ALMOST:
      c->result = IR_DONE;
      break;
    default:
      CombatPhase phase = COM_END;
      for (int i = 0; i < COM_END; i++)
        if(c->phase[i] == IR_NONE){
          phase = i;
          break;
        }

      switch(phase){
        case COM_INIT:
          c->phase[phase] = IR_DONE;
          //res = CombatStepPhase(c, COM_BATTLE);
          break;
        case COM_BATTLE:
          res = CombatStepPhase(c, COM_BATTLE);
          break;
        case COM_RESPONSE:
          c->phase[phase] = IR_DONE;
          //res = CombatStepPhase(c, COM_END);
          break;
        case COM_END:
          c->result = IR_DONE;
          break;

      }
      break;
  }

  OnCombatStep(i, res);

  return c->result;
}
