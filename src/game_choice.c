#include "game_utils.h"
#include "game_control.h"

static decision_pool_t* g_sort_decisions;

choice_pool_t* StartChoice(choice_pool_t** pool, int size, ChoiceFn fn, bool* result){
  if(*pool==NULL){
    *pool = InitChoicePool(size, fn);
    *result = false;
  }
  else{
    (*pool)->choose = fn;
    *result = (*pool)->count > 0;
  }
  return *pool;
}

choice_t* ChoiceById(choice_pool_t* pool, int id){
  if(!pool || pool->count == 0)
    return NULL;

  for (int i = 0; i < pool->count; i++){
    if(pool->choices[i]->id == id)
      return pool->choices[i];
  }

  return NULL;
}

void EndChoice(choice_pool_t* pool, bool reset){
  pool->total = 0;
  for(int i = 0; i < pool->count; i++)
    pool->total+= pool->choices[i]->score;

  if(pool->total < pool->count || reset){
    pool->total = 0;
    for(int i = 0; i < pool->count; i++){
      choice_t* c = pool->choices[i];
      c->score = c->orig_score;
      pool->total+=c->score;
    }
  }

  pool->filtered = 0;
}

choice_pool_t* InitChoicePool(int size, ChoiceFn fn){
  // Allocate the pool
  choice_pool_t *pool = calloc(1, sizeof(choice_pool_t));
  if (!pool) return NULL;

  // Store selection callback
  pool->choose = fn;

  // Ensure count starts at zero
  pool->count = 0;

  // Clamp size so we don't exceed MAX_OPTIONS
  if (size > MAX_OPTIONS) size = MAX_OPTIONS;

  // Allocate choice entries (NULL-initialized if you prefer lazy allocation)
  for (int i = 0; i < size; i++) {
    pool->choices[i] = calloc(1, sizeof(choice_t));
  }

  return pool;
}

void DiscardChoice(choice_pool_t* pool, choice_t* self){
  if (!pool || !self || pool->count <= 0)
    return;

  for (int i = 0; i < pool->count; i++) {
    if (pool->choices[i]->id == self->id) {
      // Remove from choices by swapping last element
      pool->choices[i] = pool->choices[pool->count - 1];
      pool->choices[pool->count - 1] = NULL;

      pool->count--;

      return;
    }
  }
}

void ChoiceReduceScore(choice_pool_t* pool, choice_t* self){
  int avg = pool->total/pool->count;
  self->score -= avg/2;
  if(self->score < 1)
    self->score = 1;
  pool->total -= avg/2;
}

bool AddFilter(choice_pool_t *pool, int id, void *ctx){
  if(!pool)
    return false;

  if(pool->filtered >= MAX_OPTIONS)
    return false;

  choice_t *c = calloc(1, sizeof(choice_t));
  if (!c) return false;

  c->id = id;
  pool->filter[pool->filtered++] = c;
  return true;
}

bool AddPurchase(choice_pool_t *pool, int id, int score, int cost, void *ctx, OnChosen fn){
  if (!pool) return false;

  // Ensure we do not exceed capacity
  if (pool->count >= MAX_OPTIONS)
    return false;

  // Allocate a new choice
  choice_t *c = calloc(1, sizeof(choice_t));
  if (!c) return false;

  c->score   = score;
  c->orig_score   = score;
  c->cost    = cost;
  c->context = ctx;
  c->id = id;
  c->cb = fn;
  
  // Store in pool
  pool->choices[pool->count++] = c;
  pool->total+= score;
  return true;

}

bool AddChoice(choice_pool_t *pool, int id, int score, void *ctx, OnChosen fn){
    if (!pool) return false;

    // Ensure we do not exceed capacity
    if (pool->count >= MAX_OPTIONS)
        return false;

    // Allocate a new choice
    choice_t *c = calloc(1, sizeof(choice_t));
    if (!c) return false;

    c->score   = score;
    c->orig_score   = score;
    c->context = ctx;
    c->id = id;
    c->cb = fn;
    // Store in pool
    pool->choices[pool->count++] = c;
    pool->total+= score;
    return true;
}

choice_t* ChooseCheapest(choice_pool_t* pool){
  if(pool->count == 0)
    return NULL;

  int cheapest   = 99999;
  int best       = -1;
  int best_index = -1;

  for (int i = 0; i < pool->count; i++){
    choice_t* c = pool->choices[i];
    if(!ChoiceAllowed(pool, c))
      continue;

    if(c->cost > cheapest)
      continue;

    if(c->cost == cheapest && c->score < best)
      continue;

    cheapest =  c->cost;
    best_index = i;
    best = c->score;
  }

  choice_t* out = pool->choices[best_index];
  if(pool->choices[best_index]->cb)
    pool->choices[best_index]->cb(pool,pool->choices[best_index]);
  return out;


}

choice_t* ChooseBest(choice_pool_t* pool){
  if(pool->count == 0)
    return NULL;

  int best = -1;
  int best_index = -1;

  for (int i = 0; i < pool->count; i++){
    choice_t* c = pool->choices[i];
    if(!ChoiceAllowed(pool, c))
      continue;
    
    if(c->score < best)
      continue;

    best_index = i;
    best = c->score;
  }

  if(best_index < 0)
    return NULL;
  
  choice_t* out = pool->choices[best_index];
  if(pool->choices[best_index]->cb)
    pool->choices[best_index]->cb(pool,pool->choices[best_index]);
  return out;

}

choice_t* ChooseByWeight(choice_pool_t* pool){
  if(!pool || pool->count == 0)
    return NULL;

  // 1. Compute total weight
  int total = 0;
  for (int i = 0; i < pool->count; i++) {
    choice_t* c = pool->choices[i];
    if(!ChoiceAllowed(pool, c))
      continue;

    int w = c->score;
    if (w > 0) total += w;
  }

  if (total <= 0)
    return NULL; // all weights were zero or negative

  // 2. Pick random number in range
  int r = rand() % total;

  // 3. Find the weighted entry
  int running = 0;
  for (int i = 0; i < pool->count; i++) {
    if(!ChoiceAllowed(pool, pool->choices[i]))
      continue;

    int w = pool->choices[i]->score;
    if (w <= 0) continue;

    running += w;
    if (r >= running)
      continue;

    choice_t* out = pool->choices[i];
    if(pool->choices[i]->cb)
      pool->choices[i]->cb(pool,pool->choices[i]);
    return out;
  }

  return NULL;
}

choice_t* ChooseByBudget(choice_pool_t* pool){
  if(!pool || pool->count == 0 || pool->budget <= 0)
    return NULL;

  int total = 0;
  for (int i = 0; i < pool->count; i++) {
    choice_t* c = pool->choices[i];
    if(!ChoiceAllowed(pool, c))
      continue;

    int w = c->score;
    if (w > 0) total += w;
  }

  if (total <= 0)
    return NULL; // all weights were zero or negative

  for (int i = 0; i < pool->count; i++) {
    int w = pool->choices[i]->score;
    if (w <= 0) continue;
    if(w > pool->budget)
      continue;

    if(pool->choices[i]->cb)
      pool->choices[i]->cb(pool,pool->choices[i]);

    return pool->choices[i];
  }

  return NULL;

}

choice_t* ChooseByWeightInBudget(choice_pool_t* pool){
  if(!pool || pool->count == 0 || pool->budget <= 0)
    return NULL;

  int total = 0;
  for (int i = 0; i < pool->count; i++) {
    choice_t* c = pool->choices[i];
    if(!ChoiceAllowed(pool, c))
      continue;

    int cost = c->cost;
    int w = c->score;
    if (cost <= pool->budget) total += w;
  }

  if (total <= 0)
    return NULL; // all weights were zero or negative

  int r = rand() % total;

  int running = 0;
  for (int i = 0; i < pool->count; i++) {
    choice_t* c = pool->choices[i];
    if(!ChoiceAllowed(pool,c))
      continue;

    if(c->cost > pool->budget)
      continue;

    int w = c->score;
    if (w <= 0) continue;

    running += w;
    if (r >= running)
      continue;

    if(c->cb)
      c->cb(pool, c);

    return c;
  }

  return NULL;
}

void DecisionsEnsureCap(decision_pool_t* t){
  if (t->count < t->cap)
    return;

  size_t new_cap = next_pow2_int(t->cap + 1);

  decision_t* new_entries =
    realloc(t->entries, new_cap * sizeof(decision_t));

  if (!new_entries) {
    // Handle failure explicitly
    //TraceLog(LOG_WARNING,"==== LOCAL CONTEXT ERROR ===\n REALLOC FAILED");
  }

  t->entries = new_entries;
  t->cap = new_cap;

  if(t->map.count * 4 >= t->map.cap *3)
    HashExpand(&t->map);
}

decision_pool_t* InitDecisionPool(int size, ent_t* e, EntityState id){
  decision_pool_t* t = calloc(1,sizeof(decision_pool_t));
  int cap = next_pow2_int(size);
  t->owner = e;
  t->id = id;
  t->cap = cap;
  t->entries = calloc(size, sizeof(decision_t));
  HashInit(&t->map, cap*2);

  return t; 
}

decision_pool_t* StartDecision(decision_pool_t** pool, int size, ent_t* e, EntityState id, bool* result){

  if(*pool==NULL){
    *pool = InitDecisionPool(size, e, id);
    *result = false;
  }
  else{
    *result = (*pool)->cap != size;
  }
  return *pool;

}

static int DecisionCompareScoreDesc(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;

    const decision_t* A = &g_sort_decisions->entries[ia];
    const decision_t* B = &g_sort_decisions->entries[ib];

    if (A->score < B->score) return 1;
    if (A->score > B->score) return -1;
    return 0;
}

static int DecisionCompareRatioDesc(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;

    const decision_t* A = &g_sort_decisions->entries[ia];
    const decision_t* B = &g_sort_decisions->entries[ib];

    float arat = f_safe_divide(A->score, A->cost);
    float brat = f_safe_divide(B->score, B->cost);
    if (arat < brat) return 1;
    if (arat > brat) return -1;
    return 0;
}

static int DecisionCompareCostAsc(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;

    const decision_t* A = &g_sort_decisions->entries[ia];
    const decision_t* B = &g_sort_decisions->entries[ib];

    if (A->cost < B->cost) return -1;
    if (A->cost > B->cost) return 1;
    return 0;
}

void DecisionBuildSortedIndices(decision_pool_t* table) {
    for (int i = 0; i < table->count; i++){
        table->costs[i] = i;
        table->scores[i] = i;
        table->econ[i] = i;
    }
}

decision_t* DecisionSortEconomic(decision_pool_t* table){
  if (!table || table->count <= 1)
    return 0;
  DecisionBuildSortedIndices(table);

  g_sort_decisions = table;

  qsort(table->econ,
      table->count,
      sizeof(int),
      DecisionCompareRatioDesc);

  return &table->entries[table->econ[0]];

}

decision_t* DecisionSortByCost(decision_pool_t* table){
  if (!table || table->count <= 1)
    return 0;
  DecisionBuildSortedIndices(table);

  g_sort_decisions = table; 

  qsort(table->costs,
      table->count,
      sizeof(int),
      DecisionCompareCostAsc);

  return &table->entries[table->costs[0]];
}


decision_t* DecisionSortByScore(decision_pool_t* table){
  if (!table || table->count <= 1)
    return 0;
  DecisionBuildSortedIndices(table);

  g_sort_decisions = table; 

  qsort(table->scores,
      table->count,
      sizeof(int),
      DecisionCompareScoreDesc);
  
  return &table->entries[table->scores[0]];
}


bool MakeDecision(decision_pool_t* t, DecisionSortFn fn){
  if(!t || t->count == 0)
    return false;

  if(t->count == 1)
    t->selected = &t->entries[0];
  else
    t->selected = fn(t);
  
  return t->selected != NULL;
}

decision_t* InitDecision(decision_pool_t* t, game_object_uid_i other){
  if(!t)
    return NULL;
  
  game_object_uid_i id = hash_combine_64(other, t->ouid);
  decision_t* exists = DecisionGetEntry(t, id);
  if(exists)
    return exists;

  if(t->count >= 128)
    DO_NOTHING();
  
  DecisionsEnsureCap(t);
  decision_t* d = &t->entries[t->count++];

  d->id = id;
  d->params[ACT_PARAM_OWNER] = ParamMake(DATA_ENTITY, 0, t->owner);
  HashPut(&t->map, id, d);

  return d;
}

bool AddPriority(decision_pool_t* t, priority_t* p){
    if (!t) return false;

    decision_t *d = InitDecision(t, p->gouid);
    if(!d)
      return false;

    d->score = p->score;
    switch(p->method){
      case I_FLEE:
      case I_KILL:
        d->state = STATE_AGGRO;
        d->params[ACT_PARAM_TAR] = p->ctx;
        break;
      case I_CONSUME:
        d->state = STATE_NEED;
        d->params[ACT_PARAM_NEED] = p->ctx;
        break;
    }
    return true;
}

bool AddDestination(decision_pool_t* t, local_ctx_t* ctx, EntityState s, Score score, Score cost){
  if(!ctx->path)
    return false;
  decision_t *d = InitDecision(t, ctx->gouid);

  if(!d)
    return false;

  d->score = ctx->scores[score];
  d->cost = ctx->scores[cost];

  param_t p = ParamMake(DATA_LOCAL_CTX, 0, ctx);

  d->decision = ACTION_MOVE;
  d->state = s;
  d->params[ACT_PARAM_DEST] = p;
  d->params[ACT_PARAM_INTER] = p;

  return true; 
}

bool AddCandidate(decision_pool_t* t, local_ctx_t* ctx, ActionParam type, Score score, Score cost){
    decision_t *d = InitDecision(t, ctx->gouid);

    if(!d)
      return false;

    d->score = ctx->scores[score];
    d->cost = ctx->scores[cost];

    param_t p = ParamMake(DATA_LOCAL_CTX, 0, ctx);

    switch (ctx->method){
      case I_KILL:
        d->decision = ACTION_ATTACK;
        d->state = STATE_AGGRO;
        d->params[ACT_PARAM_DEST] = p;
        d->params[ACT_PARAM_TAR] = p;
        break;
      case I_CONSUME:
        d->decision = ACTION_INTERACT;
        d->state = STATE_NEED;
        d->params[ACT_PARAM_DEST] = p;
        d->params[ACT_PARAM_INTER] = p;
        break;
    }

    d->params[ACT_PARAM_RES] = ctx->params[PARAM_RESOURCE];
    return true; 
}

bool AddEnemy(decision_pool_t* t, local_ctx_t* ctx){
  decision_t *d = InitDecision(t, ctx->gouid);
  if(!d)
    return false;

  param_t tar = ParamMake(DATA_LOCAL_CTX, sizeof(local_ctx_t), ctx);
  float threat = *ParamRead(&ctx->params[PARAM_AGGRO], float);
  d->score = threat;
  d->cost = ctx->scores[SCORE_PATH] * (1 + sqrt(ctx->scores[SCORE_CR]));

  d->decision = ACTION_ATTACK;
  d->state = STATE_AGGRO;
  d->params[ACT_PARAM_DEST] = tar;
  d->params[ACT_PARAM_TAR] = tar;

  return true;
}

bool AddDecision(decision_pool_t* t, ActionType a){

}

void OnDecisionAction(EventType event, void* data, void* user){
  decision_pool_t* pool = user;
  action_t* action = data;

  switch(event){
    case EVENT_ACT_TAKEN:
      pool->status = ACT_STATUS_TAKEN;
      if(pool->selected)
        pool->selected->status = 0;

      pool->selected = NULL;
      break;
  }



}
