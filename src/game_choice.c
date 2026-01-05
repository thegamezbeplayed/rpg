#include "game_utils.h"

choice_pool_t* StartChoice(choice_pool_t** pool, int size, ChoiceFn fn, bool* result){
  if(*pool==NULL){
    *pool = InitChoicePool(size, fn);
    *result = false;
  }
  else{
    (*pool)->choose = fn;
    *result = true;
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
