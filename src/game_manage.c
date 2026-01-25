#include "game_tools.h"

void HashInit(hash_map_t* m, uint32_t cap) {
    assert((cap & (cap - 1)) == 0); // power of two
    m->cap = cap;
    m->count = 0;
    m->slots = calloc(cap, sizeof(hash_slot_t));
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
  hash_slot_t* new_entries = calloc(new_cap, sizeof(hash_slot_t));

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
