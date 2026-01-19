#include "game_process.h"
#include "game_helpers.h"

static int PRUNES = 0;
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
event_bus_t* InitEventBus(int cap){
  event_bus_t* bus = calloc(1,sizeof(event_bus_t));
  *bus = (event_bus_t){
    .cap = cap,
      .subs = calloc(cap, sizeof(event_sub_t))
  };

  return bus;
}

void EventBusEnsureCap(event_bus_t* bus){
  if (bus->count < bus->cap)
    return;

  int new_cap = bus->cap + 64;
  bus->subs = realloc(bus->subs, new_cap * sizeof(event_sub_t));
  bus->cap = new_cap;
}

void EventSubscribe(event_bus_t* bus, EventType event, EventCallback cb, void* u_data){
  EventBusEnsureCap(bus);

  bus->subs[bus->count++] = (event_sub_t){
    .event = event,
      .cb = cb,
      .user_data = u_data
  };
}

void EventEmit(event_bus_t* bus, EventType event,  void* e_data){
  for (int i = 0; i < bus->count; i++) {
    if (bus->subs[i].event == event)
      bus->subs[i].cb(event, e_data, bus->subs[i].user_data);
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

local_table_t* InitLocals(ent_t* e, int cap){
  local_table_t* s = calloc(1,sizeof(local_table_t));

  *s = (local_table_t){
    .owner = e,
      .count = 0,
      .cap = cap,
      .entries = calloc(cap, sizeof(local_ctx_t)),
  };

  return s;
}

void LocalEnsureCap(local_table_t* t){
  if (t->count < t->cap)
    return;

  size_t new_cap = t->cap + 64;

  local_ctx_t* new_entries =
    realloc(t->entries, new_cap * sizeof(local_ctx_t));

  if (!new_entries) {
    // Handle failure explicitly
    TraceLog(LOG_WARNING,"==== LOCAL CONTEXT ERROR ===\n REALLOC FAILED");
  }

  t->valid = false;
  t->entries = new_entries;
  t->cap = new_cap;
}


local_ctx_t* MakeLocalContext(local_table_t* s, param_t* entry, Cell pos){
  if(LocalGetEntry(s, entry->gouid))
    return NULL;

  LocalEnsureCap(s);
  local_ctx_t* e = &s->entries[s->count++];

  e->other = *entry; 
  switch(entry->type_id){
    case DATA_ENTITY:
      ent_t* mob = ParamReadEnt(entry);
      e->gouid = mob->gouid;
      e->resource = MONSTER_MASH[mob->type].has;
      e->pos = mob->pos;
      e->dist = cell_distance(pos, mob->pos);
      mob->this_world_ctx = e;
      WorldSubscribe(EVENT_ENT_DEATH, OnWorldCtx, s);
      break;
    case  DATA_ENV:
      PRUNES--;
      env_t* tile = ParamReadEnv(entry);
      e->gouid = tile->gouid;
      e->pos = tile->pos;
      e->resource = tile->has_resources;
      e->dist = cell_distance(pos, tile->pos);
      break;
    case DATA_MAP_CELL:
      PRUNES++;
      map_cell_t* mc = ParamReadMapCell(entry);
      e->gouid = mc->gouid;
      e->pos = mc->coords;
      e->resource = mc->props->resources;
      e->dist = cell_distance(pos, mc->coords);
      break;
  }

  WorldEvent(EVENT_ADD_LOCAL_CTX, e);
  return e;
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

void LocalPruneCtx(local_table_t* t, local_ctx_t* other){
  for( int i = 0; i < t->count; ){
    local_ctx_t* ctx =  &t->entries[i]; 

    if(ctx && ctx->gouid == other->gouid)
      ctx->prune = true;

    if(!ctx || ctx->prune)
      t->entries[i] = t->entries[--t->count];
    else
      i++;

  }
  t->valid = false;
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

local_ctx_t* LocalAddMap(local_table_t* s, map_cell_t* m){
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

  return ctx;
}

local_ctx_t* LocalAddEnv(local_table_t* s, env_t* e, SpeciesRelate rel){
  if(LocalGetEntry(s, e->gouid))
    return NULL;

  LocalEnsureCap(s);


  int dist = cell_distance(e->pos, s->owner->pos);
  local_ctx_t* ctx = &s->entries[s->count++];
  ctx->gouid = e->gouid;
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

  return ctx;
}

local_ctx_t* LocalAddEnt(local_table_t* s, ent_t* e, SpeciesRelate rel){
   if(LocalGetEntry(s, e->gouid))
    return NULL;

  LocalEnsureCap(s);

  local_ctx_t* ctx = &s->entries[s->count++];

  ctx->gouid = e->gouid;
  if(e->type == s->owner->type)
    rel = SPEC_KIN;

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
  for (int i = 0; i < TREAT_DONE; i++){
    float t = TREATMENT[rel][i];

    ctx->treatment[i] = t;
    if (t>0){
      Priorities pt = PRIO_NONE;
      switch(i){
        case TREAT_KILL:
        case TREAT_EAT:
          pt = PRIO_ENGAGE;
          break;
        case TREAT_FLEE:
          pt = PRIO_FLEE;
          break;
      }
      if(pt==PRIO_NONE)
        continue;
      param_t l = ParamMake(DATA_GOUID, sizeof(game_object_uid_i), &ctx->gouid);      
      l.gouid = ctx->gouid;
      PriorityAdd(s->owner->control->priorities, pt, l);
    }
  }
  return ctx;
}

void AddLocalFromCtx(local_table_t *s, local_ctx_t* ctx){
  if(LocalGetEntry(s, ctx->gouid))
    return;

  if(ctx->gouid == s->owner->gouid)
    return;

  LocalEnsureCap(s);

  local_ctx_t* lctx = NULL;

  ent_t* e = s->owner;
  switch(ctx->other.type_id){
    case DATA_ENTITY:
      ent_t* ent = ParamReadEnt(&ctx->other);
      SpeciesRelate rel = GetSpecRelation(e->props->race, ent->props->race);
      
      lctx = LocalAddEnt(s, ent, rel);
      break;
    case DATA_ENV:
      env_t* env = ParamReadEnv(&ctx->other);
      lctx = LocalAddEnv(s, env, SPEC_RELATE_NONE);
      break;
    case DATA_MAP_CELL:
      map_cell_t* mc = ParamReadMapCell(&ctx->other);
      lctx = LocalAddMap(s, mc);
      break;
  }

  if(lctx){
    lctx->ctx_revision = -1;
    lctx->pos = ctx->pos;
    lctx->world_ctx_ref = ctx;
  }
}

void LocalEntCheck(ent_t* e, local_ctx_t* ctx){
  ent_t* other = ParamReadEnt(&ctx->other);
  
  float base = -0.5f;
  bool detect = ctx->awareness > 1;
  for(int j = 0; j<SEN_DONE; j++){
    if(detect)
      break;

    e->skills[SKILL_PERCEPT]->ovrd = EntGetSkillPB(SKILL_PERCEPT, e, ctx, j);
    InteractResult res = EntCanDetect(e, other, j);
    if(res < IR_ALMOST)
      continue;

    base+=0.25f * (res-2);
  }

  if(base <= 0 && ctx->awareness <=0)
    return;

  for(int j = 0; j < TREAT_DONE; j++){
    if(ctx->treatment[j] == 0)
      continue;

    float inc = ctx->treatment[j] * base;
    ctx->awareness += inc;
    //if(ctx->awareness > 1 && ((j & TREAT_AGGRO_MASK)>0))
  }

  ctx->awareness = fmax(0,ctx->awareness);
}

void LocalEnvCheck(ent_t* e, local_ctx_t* ctx){
  env_t* other = ParamReadEnv(&ctx->other);



}

bool LocalCheck(local_ctx_t* ctx){
  if(!ctx || ctx->prune)
    return false;

  bool status = true;

  switch(ctx->other.type_id){
    case DATA_ENTITY:
      ent_t* e = ParamReadEnt(&ctx->other);
      status = CheckEntAvailable(e);
      break;
    case DATA_ENV:
      env_t* other = ParamReadEnv(&ctx->other);
      status = CheckEnvAvailable(other);
      break;
    case DATA_MAP_CELL:
      map_cell_t* mc = ParamReadMapCell(&ctx->other);
      status = mc->status < TILE_REACHABLE; 
      break;
  }

  if(!status)
    ctx->prune = true;

  return status;

}

void LocalSync(local_table_t* s, bool sort){
  LocalPrune(s);
  int dist_change = 0;

  bool this_changed = !cell_compare(s->owner->pos ,s->owner->old_pos);

  for(int i = 0; i < s->count; i++){
    local_ctx_t* ctx = &s->entries[i];

    local_ctx_t* wctx = ctx->world_ctx_ref;

    if(!this_changed && wctx->ctx_revision == ctx->ctx_revision)
      continue;

    s->valid = false;
    ctx->ctx_revision = wctx->ctx_revision;
    ctx->pos = wctx->pos;
    int dist = cell_distance(s->owner->pos, ctx->pos);
    if(dist != ctx->dist){
      dist_change += abs(dist - ctx->dist);
      ctx->dist = dist;
      if(this_changed)
        ctx->valid = false;

      switch(ctx->other.type_id){
        case DATA_ENTITY:
          LocalEntCheck(s->owner, ctx);
          break;
        case DATA_ENV:
          //dist_change = LocalEnvCheck(s->owner, ctx);
          break;
        case DATA_MAP_CELL:
          map_cell_t* mc = ParamReadMapCell(&ctx->other);
          if(!mc->updates)
            continue;

          ctx->resource = mc->props->resources;
          break;
      }
    }
  }

  if(dist_change < 32)
    return;

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

    if(out && ctx->aggro->challenge < out->aggro->challenge)
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


