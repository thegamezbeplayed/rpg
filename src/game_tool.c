#include "game_tools.h"
static int TOTAL_SIZE = 0;
void HashInit(hash_map_t* m, uint32_t cap) {
    assert((cap & (cap - 1)) == 0); // power of two
    m->cap = cap;
    m->count = 0;
    m->slots = GameCalloc("HashInit", cap, sizeof(hash_slot_t));
}

void HashClear(hash_map_t* m){
  free(m->slots);
  m->count = 0;
}

void HashFree(hash_map_t* m) {
    free(m->slots);
    memset(m, 0, sizeof(*m));
}

void* HashGet(hash_map_t* m, hash_key_t key) {
    uint64_t h = Hash64(key);
    uint32_t mask = m->cap - 1;

    for (uint32_t i = 0; i < m->cap; i++) {
        hash_slot_t* s = &m->slots[(h + i) & mask];

        if (s->state == 0)
            return NULL;

        if (s->state == 1 && s->key == key)
            return s->value;
    }

    return NULL;
}

void HashExpand(hash_map_t* m){
  int old_cap = m->cap;
  size_t new_cap = old_cap * 2;
  hash_slot_t* new_entries = GameCalloc("HashExpand", new_cap, sizeof(hash_slot_t));

  for (int i = 0; i < old_cap; i++) {
    hash_slot_t* e = &m->slots[i];

    if (e->state != 1)
      continue;

    uint64_t h = Hash64(e->key);
    uint32_t mask = new_cap - 1;

    size_t idx = h  & mask;

    while (new_entries[idx].state == 1)
      idx = (idx + 1) & mask;

    new_entries[idx].key   = e->key;
    new_entries[idx].value = e->value;
    new_entries[idx].state = 1;
  }

  free(m->slots);
  m->slots = new_entries;
  m->cap = new_cap;
}

void HashPut(hash_map_t* m, hash_key_t key, void* value) {
    if(m->count * 4 > m->cap*3) // load factor < 0.5
      return;
     
    uint64_t h = Hash64(key);
    uint32_t mask = m->cap - 1;
    hash_slot_t* tomb = NULL;

    for (uint32_t i = 0; i < m->cap; i++) {
        hash_slot_t* s = &m->slots[(h + i) & mask];

        if (s->state == 1 && s->key == key) {
            s->value = value;
            return;
        }

        if (s->state == 2 && !tomb)
            tomb = s;

        if (s->state == 0) {
            s = tomb ? tomb : s;
            s->key = key;
            s->value = value;
            s->state = 1;
            m->count++;
            return;
        }
    }

    assert(0 && "HashPut failed");
}

void HashRemove(hash_map_t* m, hash_key_t key) {
    uint64_t h = Hash64(key);
    uint32_t mask = m->cap - 1;

    for (uint32_t i = 0; i < m->cap; i++) {
        hash_slot_t* s = &m->slots[(h + i) & mask];

        if (s->state == 0)
            return;

        if (s->state == 1 && s->key == key) {
            s->state = 2; // tombstone
            s->value = NULL;
            m->count--;
            return;
        }
    }
}

void* GameCalloc(const char* func, int count, size_t size){
  size_t total_size = count * size;
  size_t total = total_size + sizeof(size_t);

  size_t* raw = calloc(1, total);  // zero-initialized
  if (!raw) return NULL;

  *raw = total_size;
  TOTAL_SIZE += total_size;

  double mb = TOTAL_SIZE / (1024.0 * 1024.0);
  double gb = TOTAL_SIZE / (1024.0 * 1024.0 * 1024.0);
  //TraceLog(LOG_INFO,"%s is Allocating %zu bytes. Total %.2f MB (%.2f GB)\n", func, total_size, mb, gb);

  return (void*)(raw + 1);
}

void* GameMalloc(const char* func, size_t size){
  size_t total = size + sizeof(size_t);
  size_t* raw = malloc(total);
  if (!raw) return NULL;

  *raw = size;               // store user size
  TOTAL_SIZE += size;

  double mb = TOTAL_SIZE / (1024.0 * 1024.0);
  double gb = TOTAL_SIZE / (1024.0 * 1024.0 * 1024.0);
  //TraceLog(LOG_INFO, "%s is Allocating %zu bytes. Total %.2f MB (%.2f GB)\n", func, size, mb, gb);

  return (void*)(raw + 1);   // return pointer after header
}

void GameFree(const char* func, void* ptr){
  if (!ptr)
    return;

  size_t* raw = ((size_t*)ptr) - 1;
  size_t size = *raw;
  if(size == 0)
    return;

  TOTAL_SIZE-= size;
  double mb = TOTAL_SIZE / (1024 * 1024);
  double gb = TOTAL_SIZE / (1024 * 1024 * 1024);
  //TraceLog(LOG_INFO, "%s is freeing %zu\n TOTAL SIZE %.2f MB (%.2f GB)\n", func, size, mb, gb);

  free(raw);
}

void* GameRealloc(const char* func, void* ptr, size_t new_size)
{
    if (!ptr)
        return GameMalloc(func, new_size);

    // Move back to header
    size_t* raw = ((size_t*)ptr) - 1;
    size_t old_size = *raw;

    // Allocate new memory with header
    size_t* new_raw = realloc(raw, new_size + sizeof(size_t));
    if (!new_raw) return NULL;

    *new_raw = new_size;

    TOTAL_SIZE += new_size;
    TOTAL_SIZE -= old_size;

    double mb = TOTAL_SIZE / (1024.0*1024.0);
    double gb = TOTAL_SIZE / (1024.0*1024.0*1024.0);
//    TraceLog(LOG_INFO,"%s realloc: %zu bytes (old %zu). Total %.2f MB (%.2f GB)\n",
   //        func, new_size, old_size, mb, gb);

    return (void*)(new_raw + 1);
};

