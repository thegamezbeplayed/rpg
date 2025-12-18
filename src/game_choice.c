#include "game_utils.h"

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

