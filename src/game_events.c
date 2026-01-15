#include "game_process.h"
#include "game_helpers.h"

static interaction_t *interactions[MAX_INTERACTIONS];
static bool interaction_used[MAX_INTERACTIONS]; 

const interaction_t default_interaction = {
  .timer = NULL
};

int InitInteractions(){
  int i;
  for (i=0; i < MAX_INTERACTIONS; i++){
    interactions[i] = NULL;//default_interaction;
    interaction_used[i] = false;
  }

  return i;
}

interaction_t* RegisterInteraction(uint16_t source, uint16_t target, EventType event, int duration, void* ctx, uint16_t ctx_id){
  interaction_t* inter = malloc(sizeof(interaction_t));

  interaction_uid_i uid = InteractionMakeUID(event, ctx_id, source, target);
 *inter = (interaction_t){
   .uid = uid,
   .timer = InitCooldown(duration,EVENT_INTERACTION,NULL,NULL)
 };

 if(!AddInteraction(inter))
   return NULL;

 return inter;
}

interaction_t* StartInteraction(uint16_t source, uint16_t target, EventType event,
    int duration, void* ctx, uint16_t ctx_id, param_t data, InteractionCB add, InteractionCB update, bool refresh){
  interaction_uid_i uid = InteractionMakeUID(event, ctx_id, source, target);

  interaction_t* result = GetInteractionByUID(uid);

  if(result){
    if(result->on_update){
      uint64_t result_id = result->on_update(result, ctx, data);
      if(result->refresh_on_update && result_id == result->uid){
          //TODO REFRESH COOLIES
      }
    }
  }
  else{
    result = RegisterInteraction(source, target, event, duration, ctx, ctx_id);
    result->on_update = update;
    result->on_add = add;
    result->refresh_on_update = refresh;
    if(result->on_add)
      result->on_add(result, ctx, data);
  }

  return result;

}

interaction_t* GetInteractionByUID(interaction_uid_i uid){
  int index = InteractionExists(uid);

  if(index < 0)
    return NULL;

  return interactions[index];
}

int InteractionExists(interaction_uid_i uid){
  for (int i = 0; i < MAX_INTERACTIONS; i++){
    if(!interaction_used[i])
      continue;
    
    if(interactions[i]->uid != uid)
      continue;

    return i;

  }

  return -1;
}

bool AddInteraction(interaction_t* inter){
  for (int i = 0; i < MAX_INTERACTIONS; i++){
    if(interaction_used[i])
      continue;

    interaction_used[i] = true;

    interactions[i] = inter;

    return true;
  }

  return false;
}

/*
bool CanInteract(int source, int target){
  for (int i = 0; i < MAX_INTERACTIONS; i++){
    if(interactions[i].source_uid != source)
      continue;

    if(interactions[i].target_uid == target)
      return false;
  }

  return true;
}
*/

/*
int GetInteractions(int source){
  int count = 0;
  for (int i = 0; i < MAX_INTERACTIONS; i++){
    if(interactions[i].source_uid != source)
      continue;

    count++;
  }

  return count;
}
*/

void FreeInteraction(interaction_t* item) {
  /*
  int index = &item - &interactions;
  if (index >= 0 && index < MAX_INTERACTIONS) {
    FreeInteractionByIndex(index);
  }
  */
}

void FreeInteractions(){
  for (int i = 0; i < MAX_INTERACTIONS; i++)
    FreeInteractionByIndex(i);
}

void FreeInteractionByIndex(int i) {
  if (i < 0 && i > MAX_INTERACTIONS)
    return;

  interaction_used[i] = false;
  interactions[i] = NULL;//default_interaction;
}

void InteractionStep(){
  for (int i = 0; i < MAX_INTERACTIONS; i++){
    if(!interaction_used[i])
      continue;

    if(interactions[i]->timer == NULL){
      FreeInteractionByIndex(i);
      continue;
    }

    if(interactions[i]->timer->elapsed >= interactions[i]->timer->duration){
      FreeInteractionByIndex(i);
      continue;
    }

    interactions[i]->timer->elapsed++;
  }
}

cooldown_t* InitCooldown(int dur, EventType type, CooldownCallback on_end_callback, void* params){
  cooldown_t* cd = malloc(sizeof(cooldown_t)); 

  *cd = (cooldown_t){
    .type = type,
      .is_complete = false,
      .duration = dur,
      .elapsed = 0,
      .on_end_params = params,
      .on_end = on_end_callback// ? on_end_callback : DO_NOTHING
  };
  
  return cd;
}
void UnloadCooldown(cooldown_t* cd){
  if(!cd)
    return;

  *cd = (cooldown_t){0};
}

void UnloadEvents(events_t* ev){
  for (int i = 0; i < MAX_EVENTS; i++){
    if(!ev->cooldown_used[i])
      continue;
    UnloadCooldown(&ev->cooldowns[i]);
    ev->cooldown_used[i] = false;
  }
}

int AddEvent(events_t* pool, cooldown_t* cd){
  if(CheckEvent(pool,cd->type))
    return -1;
  
  for (int i = 0; i < MAX_EVENTS; i++){
    if(pool->cooldown_used[i]){
      if(pool->cooldowns[i].type == EVENT_NONE){
        pool->cooldowns[i] = *cd;
        pool->cooldown_used[i] = true;
        pool->num_used++;
        return i;
      }
      else
        continue;
    }
    else{
      pool->cooldowns[i] = *cd;
      pool->cooldown_used[i] = true;
      pool->num_used++;
      return i;
    }
  }

  return -1;
}

events_t* InitEvents(){
  events_t* ev = malloc(sizeof(events_t));
  *ev =  (events_t) { 0 };

  for(int i = 0; i < MAX_EVENTS; i++){
    ev->cooldown_used[i] = false;
    ev->cooldowns[i].type = EVENT_NONE;
  }

  return ev;
}

event_uid_i RegisterEvent(EventType type, cooldown_t* cd, int ent_id, StepType when){
  event_uid_i uid = EventMakeUID(type, WORLD_TICK, ent_id);
  cd->eid = uid;

  if(WorldAddEvent(uid, cd, when))
    return uid;

  return  -1;
}

int GetEventIndex(events_t* pool, EventType type){
  for(int i = 0; i<MAX_EVENTS; i++){
    if(!pool->cooldown_used[i])
      continue;

    if(pool->cooldowns[i].type != type)
      continue;


    return i;
  }
  
  return -1;
}


bool CheckEvent(events_t* pool, EventType type){
  for(int i = 0; i<MAX_EVENTS; i++){
    if(!pool->cooldown_used[i])
      continue;

    if(pool->cooldowns[i].type != type)
      continue;

    if(pool->cooldowns[i].is_complete)
      return false;

    return true;
  }

  return false;
}

void ResetEvent(events_t* pool, EventType type){
  for (int i = 0; i < MAX_EVENTS; i++){
    if(!pool->cooldown_used[i])
      continue;

    if(pool->cooldowns[i].type != type)
      continue;

    pool->cooldowns[i].is_complete = false;
    pool->cooldowns[i].elapsed = 0;
  }

}

void StartEvent(events_t* pool, EventType type){
  for (int i = 0; i < MAX_EVENTS; i++){
    if(!pool->cooldown_used[i])
      continue;
 
    if(pool->cooldowns[i].type != type)
      continue;
  }
}

void StepEvents(events_t* pool){
  for (int i = 0; i < MAX_EVENTS; i++){
    if(!pool->cooldown_used[i])
      continue;

    if(pool->cooldowns[i].type == EVENT_NONE)
      continue;

    if(pool->cooldowns[i].is_complete && !pool->cooldowns[i].is_recycled){
      pool->cooldowns[i]=(cooldown_t){0};
      pool->cooldowns[i].type = EVENT_NONE;
      pool->cooldown_used[i] = false;
      continue;
    }
   /* 
    if(pool->cooldowns[i].is_complete){
      if(!pool->cooldowns[i].is_recycled)
        continue;

      pool->cooldowns[i].is_complete = false;
      pool->cooldowns[i].elapsed = 0;

      continue;
    }
    */
    if(pool->cooldowns[i].elapsed >= pool->cooldowns[i].duration){
      pool->cooldowns[i].is_complete = true;
      pool->cooldowns[i].elapsed = 0;
      if(pool->cooldowns[i].on_end){
        pool->cooldowns[i].on_end(pool->cooldowns[i].on_end_params);
      }
      continue;
    }

    if(pool->cooldowns[i].is_complete && pool->cooldowns[i].is_recycled)
      continue;

    pool->cooldowns[i].elapsed++;
    if(pool->cooldowns[i].on_step)
      pool->cooldowns[i].on_step(pool->cooldowns[i].on_step_params);
  }
}

void InitAllyTable(ally_table_t* t, int cap, ent_t* owner){
  t->owner = owner;
  t->count = 0;
  t->cap = cap;
  t->entries = calloc(t->cap, sizeof(ally_context_t));

  // Create decay event
  cooldown_t* cd = InitCooldown(
      1,                     // ticks before decay
      EVENT_TURN,
      AllySync,
      t
      );

  t->event_id = RegisterEvent(EVENT_TURN, cd, owner->uid, STEP_TURN);

}

static void AllyEnsureCapacity(ally_table_t* t){
  if (t->count < t->cap)
    return;

  t->cap *= 2;
  t->entries = realloc(t->entries,
      t->cap * sizeof(ally_context_t));
}

int AllyAdd(ally_table_t* t, ent_t* source, int dist){

  AllyEnsureCapacity(t);

  int off = EntGetOffRating(source);
  int def = EntGetDefRating(source);

  int value = off + def;
  int cost  = dist *10;
  float danger = 1 - RATIO(source->stats[STAT_HEALTH]);

  ally_context_t* e = &t->entries[t->count++];
  e->ally = source;
  e->danger = danger;
  e->offense = off;
  e->defense = def;
  e->distance = dist;
  e->cost = cost;
  e->value = value;
  e->score = value - cost;

  return e->score;
}

void AllySync(void* params){
  ally_table_t* t = params;
}

int LocalCompareAsc(const void* a, const void* b){
  const local_ctx_t* A = (const local_ctx_t*)a;
  const local_ctx_t* B = (const local_ctx_t*)b;

  // Primary: dist
  if (A->dist > B->dist) return  1;
  if (A->dist < B->dist) return -1;

  if (A->awareness > B->awareness) return -1;
  if (A->awareness < B->awareness) return  1;

  if (A->cr > B->cr) return -1;
  if (A->cr < B->cr) return  1;

  return 0;
}

local_table_t* InitLocals(ent_t* e){
  local_table_t* s = calloc(1,sizeof(local_table_t));

  *s = (local_table_t){
    .owner = e,
      .count = 0,
      .cap = 6,
      .entries = calloc(6, sizeof(local_ctx_t)),
  };

  return s;
}

void LocalSortByDist(local_table_t* table){
  if (!table || table->count <= 1)
    return;

  qsort(table->entries,
      table->count,
      sizeof(local_ctx_t),
      LocalCompareAsc);
}

local_ctx_t* RemoveEntryByRel(local_table_t* t, SpeciesRelate rel){
  for (int i = 0; i < t->count; i++){
    local_ctx_t *ctx = &t->entries[i];
    if(ctx->prune || ctx->rel != rel)
      continue;

    ctx->prune = true;

    return ctx;
  }

  return NULL;
}

local_ctx_t* LocalGetEntry(local_table_t* table, game_object_uid_i other){
  
  for (int i = 0; i < table->count; i++) {
    local_ctx_t* ctx = &table->entries[i];
    if (ctx->gouid == other)
      return ctx;
  }

  return NULL;
}

void LocalPrune(local_table_t* t){
  for (int i = 0; i < t->count; ) {
    local_ctx_t* ctx = &t->entries[i];

    if (!ctx || ctx->prune)
      t->entries[i] = t->entries[--t->count];
    else 
      i++;
  }

}

void LocalEnsureCap(local_table_t* t){
  if (t->count < t->cap)
    return;

  t->cap += 6;
  t->entries = realloc(t->entries,
      t->cap * sizeof(local_ctx_t));

}

void AddLocalMap(local_table_t* s, map_cell_t* m){
  LocalEnsureCap(s);

  int dist = cell_distance(s->owner->pos, m->coords);
  local_ctx_t* ctx = &s->entries[s->count++];
  param_t mc = ParamMake(DATA_MAP_CELL, sizeof(map_cell_t), m);

  ctx->gouid = m->gouid;
  ctx->resource = m->props->resources;

  float longest_possible = s->owner->map->width + s->owner->map->height;
  int count = __builtin_popcountll(ctx->resource);

  ctx->awareness = (float)(1.f+count) - (float)(dist / longest_possible);

  ctx->aggro = NULL;
  ctx->other = mc;
}

void AddLocalEnv(local_table_t* s, env_t* e, SpeciesRelate rel){
  if(LocalGetEntry(s, e->gouid))
    return;

  LocalEnsureCap(s);

  int dist = cell_distance(e->pos, s->owner->pos);
  local_ctx_t* ctx = &s->entries[s->count++];
  param_t env = ParamMake(DATA_ENV, sizeof(env_t), e);

  ctx->other = env;
  ctx->rel = rel;
  ctx->dist = dist;

  int count = 0;
  int score = 0;
  for(int i = 0; i < RES_DONE; i++){
    if(!e->resources[i] || e->resources[i]->amount == 0)
      continue;

    if(e->resources[i]->type == RES_MEAT)
      DO_NOTHING();

    ctx->resource |= e->resources[i]->type;
    count++;
    score += e->resources[i]->amount;
  }

  ctx->aggro = NULL;
  ctx->cr =  score/count;
  LocalSortByDist(s);

}

void AddLocals(local_table_t* s, ent_t* e, SpeciesRelate rel){
   if(LocalGetEntry(s, e->gouid))
    return;

  LocalEnsureCap(s);

  local_ctx_t* ctx = &s->entries[s->count++];

  if(e->type == s->owner->type)
    rel = SPEC_KIN;

  for (int i = 0; i < TREAT_DONE; i++)
    ctx->treatment[i] = TREATMENT[rel][i];

  param_t ent = ParamMake(DATA_ENTITY, sizeof(ent_t), e);
  ctx->other = ent;
  ctx->cr = EntGetChallengeRating(e);
  ctx->rel = rel;
  ctx->aggro = NULL;
  for(int i = 0; i < RES_DONE; i++){
    if(!e->props->resources[i])
      continue;
    if(e->props->resources[i]->amount == 0)
      continue;

    ctx->resource|=e->props->resources[i]->type;
  }
  LocalSortByDist(s);
}
bool LocalEntCheck(ent_t* e, local_ctx_t* ctx){
  bool change = false;  
  ent_t* other = ParamReadEnt(&ctx->other);
  int dist = cell_distance(e->pos, other->pos);

  if(dist==ctx->dist)
    return change;

  change = true;
  ctx->dist = dist;

  float base = -0.5f;
  bool detect = ctx->awareness > 1;
  for(int j = 0; j<SEN_DONE; j++){
    if(detect)
      break;

    e->skills[SKILL_PERCEPT]->ovrd = EntGetSkillPB(SKILL_PERCEPT, e, ctx, j);
    InteractResult res = EntCanDetect(e, other, j);
    if(res < IR_ALMOST)
      continue;

    base+=0.25f * (res-3);
  }

  if(base <= 0 && ctx->awareness <=0)
    return change;

  for(int j = 0; j < TREAT_DONE; j++){
    if(ctx->treatment[j] == 0)
      continue;

    float inc = ctx->treatment[j] * base;
    ctx->awareness += inc;
    //if(ctx->awareness > 1 && ((j & TREAT_AGGRO_MASK)>0))
  }

  if(e->type != ENT_PERSON)
    DO_NOTHING();

  ctx->awareness = fmax(0,ctx->awareness);
  return true;
}

void LocalSync(local_table_t* s){
  LocalPrune(s);
  bool dist_change = false;
  for(int i = 0; i < s->count; i++){
    local_ctx_t* ctx = &s->entries[i];

    switch(ctx->other.type_id){
      case DATA_ENTITY:
        dist_change = LocalEntCheck(s->owner, ctx);
        ent_t* other = ParamReadEnt(&ctx->other);
        break;
      case DATA_MAP_CELL:
        map_cell_t* mc = ParamReadMapCell(&ctx->other);
        int dist = cell_distance(s->owner->pos, mc->coords);
        if(dist != ctx->dist){
          dist_change = true;
          ctx->dist = dist;
        }
        if(!mc->updates)
          continue;

        ctx->resource = mc->props->resources;
        break;
    }
  }

  if(dist_change)
    LocalSortByDist(s);
}


local_ctx_t* LocalGetThreat(local_table_t* t){
  local_ctx_t* out = NULL;
  float highest = -1;

  for(int i = 0; i < t->count; i++){
    local_ctx_t* ctx = &t->entries[i];
    if(!ctx || !ctx->aggro)
      continue;

    if(ctx->aggro->threat < highest)
      continue;

    if(ctx->aggro->challenge < out->aggro->challenge)
      continue;

    highest = ctx->aggro->threat;
    out = &t->entries[i];
  }

  return out;
  
}

aggro_t* LocalGetAggro(local_table_t* table, game_object_uid_i other){
  local_ctx_t* ctx = LocalGetEntry(table, other);

  if(ctx == NULL || ctx->aggro == NULL)
    return NULL;

  return ctx->aggro;
}

int LocalAddAggro(local_table_t* table, ent_t* source, int threat_gain, float mul, bool init){
  // Check if already present

  float threat = mul * threat_gain;
  if(source->type == ENT_PERSON)
    mul = 1;

  local_ctx_t* ctx = LocalGetEntry(table,source->gouid);

  if(!ctx || ctx->other.type_id != DATA_ENTITY)
    return 0;

  if (ctx->aggro && ctx->gouid == source->gouid) {
    if(init)
      ctx->aggro->initiated = true;
    ctx->aggro->threat += threat;
    ctx->aggro->last_turn = TURN;
    return ctx->aggro->challenge;
  }

  int off = EntGetOffRating(source);
  int def = EntGetDefRating(source);

  float cr = (off*def) /10;

  ctx->aggro = calloc(1,sizeof(aggro_t));

  ctx->aggro->initiated = init;
  ctx->aggro->last_turn = TURN;
  ctx->aggro->offensive_rating = off;
  ctx->aggro->defensive_rating = def;
  ctx->aggro->challenge = cr*mul;
  ctx->aggro->threat = ctx->aggro->challenge>threat_gain?ctx->aggro->challenge:threat;

  return cr;
}
