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

void HashPut(hash_map_t* m, hash_key_t key, void* value) {
    assert(m->count * 2 < m->cap); // load factor < 0.5

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
