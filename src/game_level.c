#include "game_process.h"
#include "game_helpers.h"


level_t Level;

spawn_pool_t* InitSpawnPool(int cap){
  spawn_pool_t* p = GameCalloc("InitSpawnPool", 1, sizeof(spawn_pool_t));

  p->cap = cap;
  p->entries = GameCalloc("InitSpawnPool", cap, sizeof(spawner_t));

  return p;
}

spawner_t* InitSpawner(spawn_pool_t* p, int cap){
  spawner_t *s = &p->entries[p->count++];
  s->cap = cap;
  s->pool = GameCalloc("InitSpawner", cap, sizeof(mob_t));

  return s;
}

void LevelAddSpawner(mob_group_t* m, map_room_t* r){
  spawn_pool_t* pool = Level.spawns;
  if(pool->count >= pool->cap)
    return;

  choice_pool_t* p = m->party_builder;
  spawner_t* s = InitSpawner(pool, p->desired);

  s->room = r;

  while(p->desired > 0){
    choice_t* c = p->choose(p);
    if(!c || c->context == NULL)
      return;

     mob_t* base = c->context;

     mob_t* m = &s->pool[s->num_mobs++];
     *m = *base;

     p->desired--;
  }

  WorldEvent(EVENT_LEVEL_SPAWNER_READY, s, r->id);
}

void AssignFaction(map_room_t* r){
  bool ready = false;
  choice_pool_t* p = StartChoice(&Level.assignments, NUM_FACTIONS, ChooseByWeight, &ready);
  if(!ready)
    return;

  p->flags = r->flags;

  choice_t* choice = p->choose(p);
  if(!choice || !choice->context) 
    return;

  mob_group_t* m = choice->context;

  LevelAddSpawner(m, r);
}

void OnLevelReady(EventType event, void* data, void* user){
  switch(event){
    case EVENT_ROOM_READY:
      map_room_t* r = data;
      AssignFaction(r);
      break;
  }
}

level_t* InitLevel(void){
  Level.paths = InitPathPool(MAX_ENVS);

  WorldSubscribe(EVENT_ROOM_READY, OnLevelReady, &Level);

  return &Level;
}

void LevelReady(map_grid_t* m){

  map_gen_t mdef = MAPS[m->id];

  int total = 0;
  for (int i = 0; i < NUM_FACTIONS; i++){
    int ratio = FACTIONS[i]->bio_pref[mdef.biome];

    if(ratio == 0)
      continue;

    int size = MAX_ENTS * ratio / 100;
    Level.factions[Level.num_factions++] = InitFactionGroups(i, size);
  
    total += size;
  }

  Level.spawns = InitSpawnPool(total);
  for(int x = 0; x < m->width; x++){
    for(int y = 0; y < m->height; y++){
      map_cell_t* mc = &m->tiles[x][y];
      Cell c = CELL_NEW(x,y);
      //if(!HasLOS(m, player->pos, c))
        //continue;
      WorldTargetSubscribe(EVENT_ENT_STEP, MapVisEvent, mc, player->gouid);      
      mc->vis = VIS_UNSEEN;
    }
  }

}

mob_group_t* InitMobGroup(faction_t* f, MobRule size)
{
  EntityType types[f->num_variation];
  memset(types, 0, sizeof(types));

  CopyIf(types, f->member_ratio, ENT_DONE, f->num_variation, GREATER_THAN);
  choice_pool_t* pb = InitChoicePool(f->num_variation*2, ChooseByWeight);

  for (int i = 0; i < f->num_variation; i++){
    EntityType t = types[i];
    for (int j = 0; j < PROF_LABORER; j++){
      define_race_class_t *drc = &RACE_CLASS_DEFINE[t][j];

      for (int k = 0; k < drc->count; k++){
        mob_t* mob = GameCalloc("InitMobGroup", 1, sizeof(mob_t));
        mob->type = t;
        mob->prof = j;
        mob->def = &drc->classes[k];
        AddChoice(pb, t, f->member_ratio[t] * mob->def->weight, mob, ChoiceReduceScore);
      }
    }
  }
  mob_group_t *m = GameCalloc("InitMobGroup", 1, sizeof(mob_group_t));
  m->party = size;

  pb->desired = GetPartySize(size);
  m->party_builder = pb;
  return m;

}

faction_groups_t* InitFactionGroups(Faction id, int desired){
  faction_groups_t* g = GameCalloc("InitFactionGroups", 1,sizeof(faction_groups_t));

  faction_t* faction = FACTIONS[id];
  bool ready = false;
  choice_pool_t* p = StartChoice(&Level.assignments, NUM_FACTIONS, ChooseByWeight, &ready);

  MobRules grouping = faction->rules & MOB_GROUPING_MASK;
  MobRules flags = faction->rules & ~MOB_GROUPING_MASK;
  p->flags = flags;
  int count = 0;
  while(grouping){
    uint64_t group = grouping & -grouping;
    grouping &= grouping -1;
    mob_group_t* mg = InitMobGroup(faction, group);
    g->groups[count] = mg;
    uint64_t p_flags = group | flags;
    AddPurchaseFlags(p, id, desired, mg, p_flags, DiscardChoice);
    count++;
  }

  return g;
}

void LevelBury(game_object_uid_i gouid){


}
