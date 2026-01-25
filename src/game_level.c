#include "game_process.h"

level_t* InitLevel(map_grid_t* m){
  level_t* l = calloc(1,sizeof(level_t));

  l->paths = InitPathPool(MAX_ENVS);
  map_gen_t mdef = MAPS[m->id];
  for (int i = 0; i < NUM_FACTIONS; i++){
    int ratio = FACTIONS[i]->bio_pref[mdef.biome];

    if(ratio == 0)
      continue;

    int size = MAX_ENTS * ratio / 100;
    l->factions[l->num_factions++] = InitFactionGroups(i, size);
  
  }

  return l;
}

void InitMobGroup(faction_groups_t** f, MobRule size, int index)
{
  mob_group_t m = {
    .party = size
  };

  (*f)->groups[index] = m;

}

faction_groups_t* InitFactionGroups(Faction id, int desired){
  faction_groups_t* g = calloc(1,sizeof(faction_groups_t));

  faction_t* faction = FACTIONS[id];

  MobRules grouping = faction->rules & MOB_GROUPING_MASK;
  int count = 0;
  while(grouping){
    uint64_t group = grouping & -grouping;
    grouping &= grouping -1;
    InitMobGroup(&g, group, count);
    count++;
  }

  return g;
}
