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

void InitAggroTable(aggro_table_t* t, int cap, ent_t* owner){
  t->owner = owner;
  t->count = 0;
  t->cap = cap;
  t->entries = calloc(t->cap, sizeof(aggro_entry_t));

  // Create decay event
  cooldown_t* cd = InitCooldown(
      1,                     // ticks before decay
      EVENT_DECAY,
      AggroDecayCallback,
      t 
      );

  t->event_id = RegisterEvent(EVENT_DECAY, cd, owner->uid, STEP_TURN);
}

void AggroEnsureCapacity(aggro_table_t* t){
  if (t->count < t->cap)
    return;

  t->cap *= 2;
  t->entries = realloc(t->entries,
      t->cap * sizeof(aggro_entry_t));
}

int AggroAdd(aggro_table_t* table, ent_t* source, int threat_gain, float mul, bool init){
  // Check if already present
  
  float threat = mul * threat_gain;
  if(source->type == ENT_PERSON)
    mul = 1;

  aggro_entry_t* e = AggroGetEntry(table,source);
  if (e && e->enemy == source) {
    if(init)
      e->initiated = true;
    e->threat += threat;
    e->last_turn = TURN;
    //            ResetEvent(events, e->decay_event);
    return e->challenge;
  }

  // New entry
  AggroEnsureCapacity(table);

  
  int off = EntGetOffRating(source);
  int def = EntGetDefRating(source);

  float cr = (off*def) /10;
   
  e = &table->entries[table->count++];
  e->initiated = init;
  e->enemy = source;
  e->last_turn = TURN;
  e->offensive_rating = off;
  e->defensive_rating = def;
  e->challenge = cr*mul;
  e->threat = e->challenge>threat_gain?e->challenge:threat;

  return cr;
}

aggro_entry_t* AggroGetEntry(aggro_table_t* table, ent_t* source){
  for (int i = 0; i < table->count; i++) {
    aggro_entry_t* e = &table->entries[i];
    if (e->enemy == source)
      return e;
  }

  return NULL;
}

int AggroCompareDesc(const void* a, const void* b){
  const aggro_entry_t* A = (const aggro_entry_t*)a;
  const aggro_entry_t* B = (const aggro_entry_t*)b;

  // Primary: threat
  if (A->threat > B->threat) return -1;
  if (A->threat < B->threat) return  1;

  // Secondary: threat multiplier
  if (A->threat_mul > B->threat_mul) return -1;
  if (A->threat_mul < B->threat_mul) return  1;

  // Tertiary: most recent turn wins
  if (A->last_turn > B->last_turn) return -1;
  if (A->last_turn < B->last_turn) return  1;

  return 0;
}

void AggroSortByThreat(aggro_table_t* table){
  if (!table || table->count <= 1)
    return;

  qsort(table->entries,
      table->count,
      sizeof(aggro_entry_t),
      AggroCompareDesc);
}

int AggroGetEntries(aggro_table_t* t, int count, aggro_entry_t  *entries){
  int out = 0;
  if(t->count < 1)
    return out;

  int check = count < t->count? count: t->count;
  if(check < t->count)
    AggroSortByThreat(t);

  for(int i = 0; i < check; i++)
    entries[out++] = t->entries[i];

  return out;

}

aggro_entry_t* AggroGetHighest(aggro_table_t* t){
  if(t->count < 1)
    return NULL;

  if(t->count == 1)
    return &t->entries[0];

  AggroSortByThreat(t);

  return &t->entries[0];
}
 
void AggroDecayCallback(void* params){

}

void AggroPrune(aggro_table_t* t) {
  for (int i = 0; i < t->count; ) {
    if (!t->entries[i].enemy ||
        t->entries[i].enemy->state == STATE_DEAD ||
        t->entries[i].threat <= 0)
    {
      t->entries[i] = t->entries[--t->count];
    } else {
      i++;
    }
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
  if(t->owner->aggro->count == 0)
    return;

  for(int i = 0; i < t->count; i++){

  }
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

ent_t* RemoveEntryByRel(local_table_t* t, SpeciesRelate rel){
  for (int i = 0; i < t->count; i++){
    local_ctx_t *ctx = &t->entries[i];
    if(ctx->prune || ctx->rel != rel)
      continue;

    ctx->prune = true;

    return ctx->other;
  }

  return NULL;
}

void LocalPrune(local_table_t* t){
  for (int i = 0; i < t->count; ) {
    local_ctx_t* ctx = &t->entries[i];

    bool prune = false;
    switch(ctx->cat){
      case OBJ_ENT:
        if (!ctx->other || ctx->prune ||
            ctx->other->state == STATE_DEAD)
          prune = true;
        break;
      case OBJ_ENV:
        if(ctx->env->has_resources == 0)
          prune = true;
        break;
    }

    if(prune)
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

void AddLocalEnv(local_table_t* s, env_t* e, SpeciesRelate rel){
  LocalEnsureCap(s);

  int dist = cell_distance(e->pos, s->owner->pos);
  local_ctx_t* ctx = &s->entries[s->count++];

  ctx->env = e;
  ctx->rel = rel;
  ctx->cat = OBJ_ENV;
  ctx->dist = dist;

  int count = 0;
  int score = 0;
  for(int i = 0; i < RES_DONE; i++){
    if(!e->resources[i] || e->resources[i]->amount == 0)
      continue;

    ctx->resource |= i;
    count++;
    score += e->resources[i]->amount;
  }

  ctx->cr =  score/count;
  LocalSortByDist(s);

}

void AddLocals(local_table_t* s, ent_t* e, SpeciesRelate rel){
  LocalEnsureCap(s);

  int dist = cell_distance(e->pos, s->owner->pos);
  local_ctx_t* ctx = &s->entries[s->count++];

  if(e->type == s->owner->type)
    rel = SPEC_KIN;

  for (int i = 0; i < TREAT_DONE; i++)
    ctx->treatment[i] = TREATMENT[rel][i];

  ctx->cr = EntGetChallengeRating(e);
  ctx->other = e;
  ctx->rel = rel;
  ctx->cat = OBJ_ENT;
  ctx->dist = dist;

  LocalSortByDist(s);
}
bool LocalEntCheck(ent_t* e, local_ctx_t* ctx){
  bool change = false;  
  ent_t* other = ctx->other;
  int dist = cell_distance(e->pos, other->pos);

  if(dist==ctx->dist)
    return change;

  change = true;
  ctx->dist = dist;

  if(dist > MAX_SEN_DIST){
    if(ctx->awareness > 0)
      ctx->awareness*=.25;

    return change;
  }

  if((ctx->rel & SPEC_RELATE_NEGATIVE)==0)
    return change;

  bool detect = ctx->awareness > 1;
  for(int j = 0; j<SEN_DONE; j++){
    if(detect)
      break;

    if(EntCanDetect(e, other, j))
      detect = true;

  }

  for(int j = 0; j < TREAT_DONE; j++){
    if(ctx->treatment[j] == 0)
      continue;

    if(!detect){
      if(ctx->awareness > 0)
        ctx->awareness*=0.5;

      break;
    }
    float inc = ctx->treatment[j] / dist;
    ctx->awareness += inc;
    if(ctx->awareness > 1 && ((j & TREAT_AGGRO_MASK)>0))
      ctx->engage = true;
  }

  return true;
}

void LocalSync(local_table_t* s){
  LocalPrune(s);
  bool dist_change = false;
  for(int i = 0; i < s->count; i++){
    local_ctx_t* ctx = &s->entries[i];

    if(ctx->cat != OBJ_ENT)
      continue;
  
    dist_change = LocalEntCheck(s->owner, ctx);
  }

  if(dist_change)
    LocalSortByDist(s);
}
