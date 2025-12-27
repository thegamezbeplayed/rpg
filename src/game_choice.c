#include "game_utils.h"

choice_pool_t* StartChoice(choice_pool_t* pool, int size, ChoiceFn fn, bool* result){
  if(pool==NULL){
    pool = InitChoicePool(size, fn);
    *result = false;
  }
  else{
    pool->choose = fn;
    *result = true;
  }
  return pool;
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

bool AddChoice(choice_pool_t *pool, int score, void *ctx){
    if (!pool) return false;

    // Ensure we do not exceed capacity
    if (pool->count >= MAX_OPTIONS)
        return false;

    // Allocate a new choice
    choice_t *c = calloc(1, sizeof(choice_t));
    if (!c) return false;

    c->score   = score;
    c->context = ctx;

    // Store in pool
    pool->choices[pool->count++] = c;

    return true;
}

choice_t* ChooseBest(choice_pool_t* pool){
  if(pool->count == 0)
    return NULL;

  int best = -1;
  int best_index = -1;

  for (int i = 0; i < pool->count; i++){
    if(pool->choices[i]->score < best)
      continue;

    best_index = i;
    best = pool->choices[i]->score;
  }

  return pool->choices[best_index];

}

choice_t* ChooseByWeight(choice_pool_t* pool){
  if(!pool || pool->count == 0)
    return NULL;

  // 1. Compute total weight
  int total = 0;
  for (int i = 0; i < pool->count; i++) {
    int w = pool->choices[i]->score;
    if (w > 0) total += w;
  }

  if (total <= 0)
    return NULL; // all weights were zero or negative

  // 2. Pick random number in range
  int r = rand() % total;

  // 3. Find the weighted entry
  int running = 0;
  for (int i = 0; i < pool->count; i++) {
    int w = pool->choices[i]->score;
    if (w <= 0) continue;

    running += w;
    if (r < running)
      return pool->choices[i];
  }

  return NULL;
}

choice_t* ChooseByBudget(choice_pool_t* pool){
  if(!pool || pool->count == 0 || pool->budget <= 0)
    return NULL;

  int total = 0;
  for (int i = 0; i < pool->count; i++) {
    int w = pool->choices[i]->score;
    if (w > 0) total += w;
  }

  if (total <= 0)
    return NULL; // all weights were zero or negative

  for (int i = 0; i < pool->count; i++) {
    int w = pool->choices[i]->score;
    if (w <= 0) continue;
    if(w > pool->budget)
      continue;

    return pool->choices[i];
  }

  return NULL;

}

