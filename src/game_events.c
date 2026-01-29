#include "game_process.h"
#include "game_helpers.h"

static local_table_t* g_sort_table;

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

interaction_t* RegisterInteraction(uint64_t source, uint64_t target, EventType event, int duration, void* ctx, uint16_t ctx_id){
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

interaction_t* StartInteraction(uint64_t source, uint64_t target, EventType event,
    int duration, void* ctx, uint16_t ctx_id, param_t data, InteractionCB add, InteractionCB update, bool refresh){
  interaction_uid_i uid = InteractionMakeUID(event, ctx_id, source, target);

  interaction_t* result = GetInteractionByUID(uid);

  int turn = WorldGetTurn();
  if(result){
    if(turn == result->last_update_turn)
      return result;

    if(result->on_update){
      uint64_t result_id = result->on_update(result, ctx, data);
      if(result->refresh_on_update && result_id == result->uid)
        result->on_update(result, ctx, data);
    }
  }
  else{
    result = RegisterInteraction(source, target, event, duration, ctx, ctx_id);
    if(!result)
      return NULL;

    result->on_update = update;
    result->on_add = add;
    result->refresh_on_update = refresh;
    if(result->on_add)
      result->on_add(result, ctx, data);
  }

  result->last_update_turn = WorldGetTurn();

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

event_sub_t* EventSubscribe(event_bus_t* bus, EventType event, EventCallback cb, void* u_data){
  EventBusEnsureCap(bus);

  event_sub_t* sub =  &bus->subs[bus->count++];
  *sub = (event_sub_t){
    .event = event,
      .cb = cb,
      .user_data = u_data
  };
  
  return sub;
}

void EventEmit(event_bus_t* bus, event_t* e){
  for (int i = 0; i < bus->count; i++) {
    if (bus->subs[i].event != e->type)
      continue;

    if (bus->subs[i].uid != 0 
       && bus->subs[i].uid != e->iuid)
      continue;

    bus->subs[i].cb(e->type, e->data, bus->subs[i].user_data);
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
  event_uid_i uid = EventMakeUID(type, ent_id);
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
static int LocalCompareDistAsc(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;

    const local_ctx_t* A = &g_sort_table->entries[ia];
    const local_ctx_t* B = &g_sort_table->entries[ib];

    if (A->dist < B->dist) return -1;
    if (A->dist > B->dist) return 1;
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

  HashInit(&s->ctx_by_gouid, 16384);
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
      WorldTargetSubscribe(EVENT_ENT_DEATH, OnWorldByGOUID, s, mob->gouid);
      break;
    case  DATA_ENV:
      PRUNES--;
      env_t* tile = ParamReadEnv(entry);
      e->gouid = tile->gouid;
      e->pos = tile->pos;
      e->resource = tile->has_resources;
      e->dist = cell_distance(pos, tile->pos);
      WorldTargetSubscribe(EVENT_ENV_DEATH, OnWorldByGOUID, s, tile->gouid);
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
  
  WorldTargetSubscribe(EVENT_UPDATE_LOCAL_CTX, OnWorldByGOUID, s, e->gouid);
  e->params[PARAM_RESOURCE] = ParamMake(DATA_UINT64, sizeof(e->resource), &e->resource);
  HashPut(&s->ctx_by_gouid, e->gouid, e);
  WorldEvent(EVENT_ADD_LOCAL_CTX, e, 0);
  return e;
}

void LocalBuildSortedIndices(local_table_t* table) {
    for (int i = 0; i < table->count; i++)
        table->sorted_indices[i] = i;
}

void LocalSortByDist(local_table_t* table){
  if (!table || table->count <= 1)
    return;
  LocalBuildSortedIndices(table);
  
  g_sort_table = table;
  
  qsort(table->sorted_indices,
      table->count,
      sizeof(int),
      LocalCompareDistAsc);
}

local_ctx_t* LocalGetEntry(local_table_t* t, game_object_uid_i other){
  return HashGet(&t->ctx_by_gouid, other);
}

void LocalPruneCtx(local_table_t* t, game_object_uid_i other){
  for( int i = 0; i < t->count; ){
    if(t->entries[i].gouid == other
        || t->entries[i].prune){
      HashRemove(&t->ctx_by_gouid, other); 
      t->entries[i] = t->entries[--t->count];
      local_ctx_t* moved = &t->entries[i];
      HashPut(&t->ctx_by_gouid, moved->gouid, moved);
      
      if(!t->owner)
        WorldEvent(EVENT_DEL_LOCAL_CTX, &other, other);
    }
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
  param_t mc = ParamMakeObj(DATA_MAP_CELL, m->gouid, m);

  ctx->dist = dist;
  ctx->gouid = m->gouid;
  ctx->resource = m->props->resources;

  map_grid_t* map = WorldGetMap();
  float longest_possible = map->width + map->height;
  int count = __builtin_popcountll(ctx->resource);

  ctx->awareness = (float)(1.f+count) - (float)(dist / longest_possible);

  ctx->aggro = NULL;
  ctx->other = mc;
  int64_t eats = MONSTER_MASH[s->owner->type].eats;
    
  ctx->scores[SCORE_RES] += 1;
  if(m->occupant)
    ctx->scores[SCORE_RES] += 2;

  if(m->tile)
    ctx->scores[SCORE_RES] += 3;

  return ctx;
}

local_ctx_t* LocalAddEnv(local_table_t* s, env_t* e, SpeciesRelate rel){
  if(LocalGetEntry(s, e->gouid))
    return NULL;

  LocalEnsureCap(s);

  local_ctx_t* ctx = &s->entries[s->count++];
  ctx->gouid = e->gouid;
  param_t env = ParamMakeObj(DATA_ENV, e->gouid, e);

  ctx->other = env;

  uint64_t food = 0;
  uint64_t eats = MONSTER_MASH[s->owner->type].eats;
  for(int i = 0; i < RES_DONE; i++){
    if(!e->resources[i] || e->resources[i]->amount == 0)
      continue;

    ctx->resource |= e->resources[i]->type;

    if((e->resources[i]->type & eats) == 0)
      continue;

    rel = SPEC_WANTS;
    ctx->method = I_CONSUME;
    ctx->scores[SCORE_RES] += e->resources[i]->amount;
  }

  uint64_t matches = ctx->resource & eats;

  ctx->params[PARAM_RELATE] = ParamMake(DATA_UINT64, sizeof(uint64_t), &rel);

  ctx->params[PARAM_RESOURCE] = ParamMake(DATA_UINT64, sizeof(uint64_t), &matches);
  ctx->aggro = NULL;
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

  param_t ent = ParamMakeObj(DATA_ENTITY, e->gouid, e);
  

  ctx->params[PARAM_RELATE] = ParamMake(DATA_UINT64, sizeof(uint64_t), &rel);
  ctx->other = ent;
  ctx->aggro = NULL;
  for(int i = 0; i < RES_DONE; i++){
    if(!e->props->resources[i])
      continue;
    if(e->props->resources[i]->amount == 0)
      continue;

    ctx->resource|=e->props->resources[i]->type;
  }
  
  species_treatment_t treat = TREATMENT[BCTZL(rel)];
  for (int i = 0; i < TREAT_DONE; i++){
    float t = treat.val[i];

    ctx->treatment[i] = t;
  }
  
  return ctx;
}

void AddLocalFromCtx(local_table_t *s, local_ctx_t* ctx){
  if(LocalGetEntry(s, ctx->gouid))
    return;

  if(ctx->gouid == s->owner->gouid)
    return;

  LocalEnsureCap(s);

  local_ctx_t* lctx;

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
      return;
      map_cell_t* mc = ParamReadMapCell(&ctx->other);
      lctx = LocalAddMap(s, mc);
      break;
  }

  if(lctx){
    lctx->path = NULL;
    lctx->scores[SCORE_PATH] = -1;
    lctx->params[PARAM_RESOURCE] = ctx->params[PARAM_RESOURCE];
    lctx->last_update = -1;
    HashPut(&s->ctx_by_gouid, lctx->gouid, lctx);
    lctx->ctx_revision = -1;
    lctx->pos = ctx->pos;
  }
}

aggro_t* LocalAggroByCtx(local_ctx_t* ctx){
  if(ctx->aggro)
    return ctx->aggro;

  if(ctx->other.type_id != DATA_ENTITY)
    return NULL;

  aggro_t* a = calloc(1, sizeof(aggro_t));
  ent_t* e = ParamReadEnt(&ctx->other);
  
  float threat_mul = CLAMPF(ctx->awareness,0.05f,0.25f);
  int cr = EntGetChallengeRating(e);
  *a = (aggro_t){
    .initiated = false,
    .offensive_rating = EntGetOffRating(e),
    .defensive_rating = EntGetDefRating(e),
    .challenge = cr,
    .threat = threat_mul * cr
  };

  return a;
}

void LocalEntCheck(ent_t* e, local_ctx_t* ctx){
  ent_t* other = ParamReadEnt(&ctx->other);

  uint64_t rel = *ParamRead(&ctx->params[PARAM_RELATE], uint64_t);
  if((rel & SPEC_RELATE_POSITIVE) > 0 && ctx->awareness <= 0)
    ctx->awareness = f_safe_divide(1,1+ctx->dist);


  float base = -0.5f;
  bool detect = ctx->awareness > 1;

  if(ctx->dist > GRID_WIDTH && ctx->awareness < 0.5f)
    return;

  if(ctx->dist > MAX_SEN_DIST && ctx->awareness < 0.25f)
    return;

  for(int j = 0; j<SEN_DONE; j++){
    if(detect)
      break;

    e->skills[SKILL_PERCEPT]->ovrd = EntGetSkillPB(SKILL_PERCEPT, e, ctx, j);
    InteractResult res = EntCanDetect(e, other, j);
    if(res < IR_ALMOST)
      continue;

    base+=0.25f * (res-2);
    if(res == IR_TOTAL_SUCC)
      detect = true;
  }
  if(base <= 0 && ctx->awareness <=0)
    return;

  for(int j = 0; j < TREAT_DONE; j++){
    if(ctx->treatment[j] == 0)
      continue;

    float inc = ctx->treatment[j] * base;
    ctx->awareness += inc;
    if(ctx->awareness > 1 && ((j & TREAT_AGGRO_MASK)>0))
      ctx->aggro = LocalAggroByCtx(ctx);

    Priorities pt = PRIO_NONE;
    switch(j){
      case TREAT_KILL:
      case TREAT_EAT:
        ctx->method = I_KILL;
        pt = PRIO_ENGAGE;
        break;
      case TREAT_FLEE:
        pt = PRIO_FLEE;
        break;
    }
    if(pt==PRIO_NONE)
      continue;

    if(ctx->awareness <= ctx->treatment[j])
      continue;

    param_t l = ParamMake(DATA_GOUID, sizeof(game_object_uid_i), &ctx->gouid);
    l.gouid = ctx->gouid;
    PriorityAdd(e->control->priorities, pt, l);

  }

  if(ctx->aggro){
    ctx->scores[SCORE_THREAT] = ctx->aggro->threat;
    param_t threat = ParamMake(DATA_FLOAT, sizeof(float), &ctx->aggro->threat);
    ctx->params[PARAM_AGGRO] = threat;
  }
  ctx->awareness = fmax(0,ctx->awareness);
  ctx->scores[SCORE_CR] = EntGetChallengeRating(other);
  LocalSetPath(e, ctx);
}

void LocalSetPath(ent_t* e, local_ctx_t* dest){
  if(!dest->path || !dest->path->valid){
    bool res = false;
    dest->path = StartRoute(e, dest, (1+dest->awareness) * MAX_SEN_DIST, &res);
    if(!res){
      dest->path = NULL;
      return;
    }
  }

  dest->path->valid = true;
  dest->puid = dest->path->guid;
  dest->scores[SCORE_PATH] = RouteScore(e, dest->path);
}

void LocalEnvCheck(ent_t* e, local_ctx_t* ctx){
  env_t* other = ParamReadEnv(&ctx->other);

  if(!ctx->resource)
    return;

  if(ctx->awareness <= 0){
    uint64_t eats = MONSTER_MASH[e->type].eats;
    uint64_t matches = ctx->resource & eats;
    float count = __builtin_popcountll(matches) + 1.0f;

    ctx->awareness = f_safe_divide(1,(ctx->dist+1))*count;
  }

  if(ctx->awareness > 0.1f)
    LocalSetPath(e, ctx);
}

bool LocalCheck(local_ctx_t* ctx){
  if(!ctx || ctx->prune)
    return false;

  bool status = true;

  switch(ctx->other.type_id){
    case DATA_ENTITY:
      ent_t* e = ParamReadEnt(&ctx->other);
      status = CheckEntAvailable(e);
      ctx->pos = e->pos;
      break;
    case DATA_ENV:
      env_t* other = ParamReadEnv(&ctx->other);
      status = CheckEnvAvailable(other);
      ctx->pos = other->pos;
      ctx->resource = other->has_resources;
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

void LocalSyncCtx(local_table_t* s, local_ctx_t* ctx){
  
  if(LocalCheck(ctx))
    ctx->ctx_revision++;
}

void LocalSync(local_table_t* s, bool sort){
  LocalPrune(s);
  int dist_change = 0;

  bool this_changed = !cell_compare(s->owner->pos ,s->owner->old_pos);
  if(this_changed)
    s->valid = false;

  for(int i = 0; i < s->count; i++){
    local_ctx_t* ctx = &s->entries[i];

    local_ctx_t* wctx = WorldGetContext(ctx->other.type_id, ctx->gouid);

    if(!wctx)
      continue;

    if(!this_changed && wctx->ctx_revision == ctx->ctx_revision)
      continue;

    ctx->ctx_revision = wctx->ctx_revision;
    int dist = cell_distance(s->owner->pos, wctx->pos);
    if(dist != ctx->dist){
      dist_change += abs(dist - ctx->dist);
      ctx->dist = dist;
      ctx->pos = wctx->pos;
      /*if(this_changed)
        ctx->valid = false;
*/
      switch(ctx->other.type_id){
        case DATA_ENTITY:
          LocalEntCheck(s->owner, ctx);
          break;
        case DATA_ENV:
          LocalEnvCheck(s->owner, ctx);
          break;
        case DATA_MAP_CELL:
          map_cell_t* mc = ParamReadMapCell(&ctx->other);
          if(!mc->updates)
            continue;

          ctx->resource = mc->props->resources;
          if(ctx->dist  < 10 && ctx->resource)
            LocalSetPath(s->owner, ctx);
          break;
      }
      ctx->last_update = WorldGetTurn();
    }

    param_t pos = ParamMake(DATA_CELL, sizeof(Cell), &ctx->pos);
    ctx->params[PARAM_POS] = pos;
    param_t res = ParamMake(DATA_UINT64, sizeof(uint64_t), &ctx->resource);
    ctx->params[PARAM_RESOURCE] = res;
  }

  if(s->valid && !this_changed){
    s->valid = true;
    return;
  }

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
  }
  else{
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
  }
  if(init)
    ctx->awareness*=1.25f;
  else
    ctx->awareness*=1.05f;

  WorldEvent(EVENT_AGGRO, source, table->owner->gouid);

  return ctx->aggro->challenge;
}

int LocalContextFilter(local_table_t* t, int num, local_ctx_t* pool[num], param_t filter, GameObjectParam type, ParamCompareFn fn){

  if(!t || t->count == 0)
    return 0;

  int found = 0;
  int i = 0;
  int count = imin(num, t->count);
  while(i < t->count && found < count){
    int k = t->sorted_indices[i];
    local_ctx_t* ctx = &t->entries[k];

    i++;
    if(ctx->awareness < 1)
      continue;

    if(ctx->params[type].type_id != filter.type_id)
      continue;

    if(!ParamCompare(&filter, &ctx->params[type], fn))
      continue;
   
   pool[found++] = ctx; 

  }  

  return found;
};

