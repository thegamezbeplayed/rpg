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
      if(r->num_mobs > 0)
        return;

      AssignFaction(r);
      break;
  }
}

level_t* InitLevel(void){
  uint64_t addr = (uint64_t)&Level;
  Level.seed = hash_combine_64(WorldSeed(), addr);
  InitRng(&Level.rng, Level.seed);
  Level.paths = InitPathPool(MAX_ENVS);

  WorldSubscribe(EVENT_ROOM_READY, OnLevelReady, &Level);

  Level.item_defs = InitItemGenPool(ABILITY_DONE + SKILL_DONE);

  for(int i = 0; i < ABILITY_DONE; i++){
    if(CLASS_ABILITIES[i].tome)
      ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGenerateKnowledge(i, CONS_TOME));     
    if(CLASS_ABILITIES[i].scroll)
      ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGenerateKnowledge(i, CONS_SCROLL));     
  }

  for (int i = 1; i < SKILL_DONE; i++)
      ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGenerateKnowledge(i, CONS_SKILLUP));     

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

  map_gen_t mgen = MAPS[m->id];
  loot_ctx_t *ctx = GameCalloc("LevelReady", 1, sizeof(loot_ctx_t));
  
  ctx->dungeon_lvl = iround(mgen.max_diff);
  ctx->biome = mgen.biome;
  ctx->luck = -1;
  ctx->enemy_cr = iround(mgen.diff);
  ctx->seed = hash_combine_64(Level.seed, mgen.biome);
  Level.loot = GenerateLootPool(64 + Level.item_defs->count, ctx);
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

loot_pool_t* GenerateLootPool(int count, loot_ctx_t *ctx){

  loot_pool_t* lp = GameCalloc("GenerateLootPool", 1, sizeof(loot_pool_t));

  lp->rules = ctx;

  bool result = false;
  choice_pool_t *cp = StartChoice(&lp->flags, count * 2, ChooseByWeight, &result);

  if(!result){
    for (int i = 0; i < Level.item_defs->count; i++){
      item_type_d* item = &Level.item_defs->entries[i];

      item_def_t* def = DefineConsumableByDef(&item->data.cons);
      
      AddPurchase(cp, def->id, 1, 5, def, ChoiceReduceScore);

    }
    cp->desired = count;
    /*
    while(cp->count > cp->cap){
      param_t p[LOOT_PARAM_END] = {0};

      int cat = RngRoll(Level.rng, ITEM_WEAPON, ITEM_DONE);
      p[LOOT_PARAM_CATEGORY] = ParamMake(DATA_INT, sizeof(int),&cat);
      int start = 0, end = 0;
      int type_prop_end = -1;
      LootParams type_param = -1;
      switch (cat){
        case ITEM_WEAPON:
          start = WEAP_MACE;
          end =  WEAP_DONE-1;
          type_prop_end = 9;
          type_param = LOOT_PARAM_WEAP;
          break;
        case ITEM_ARMOR:
          start = ARMOR_NATURAL;
          end = ARMOR_DONE-1;
          type_prop_end = 0;
          type_param = LOOT_PARAM_ARMOR;
          break;
        case ITEM_CONSUMABLE:
          continue;
        default:
          continue;
          break;

      }

      int type = RngRoll(Level.rng, start, end);
      uint64_t type_props = RngRollUID(Level.rng, 0, type_prop_end);

      p[LOOT_PARAM_TYPE] = ParamMake(DATA_INT, sizeof(int), &type);
      p[type_param] = ParamMake(DATA_UINT64, sizeof(uint64_t), &type_props);

      uint64_t qual = RngRollUID(Level.rng, QUAL_BIT_START, QUAL_BIT_START+QUAL_BIT_COUNT);
      uint64_t mat = RngRollUID(Level.rng, MAT_BIT_START, MAT_BIT_START+MAT_BIT_COUNT);
     
      uint64_t props = qual | mat;
      p[LOOT_PARAM_PROPS] = ParamMake(DATA_UINT64, sizeof(uint64_t), &props);
      int amnt = RngRoll(Level.rng, 1, 4);

      p[LOOT_PARAM_AMNT] = ParamMake(DATA_INT, sizeof(int), &amnt);

      item_def_t* item = GenerateItem(p);

      AddPurchase(cp, cp->count, 1, 1, item, ChoiceReduceScore);
    }
    */
  }


  bool ready = false;
  choice_pool_t* dp = StartChoice(&lp->drops, count, ChooseByWeightInBudget, &ready);
  cp->budget = 10;
  dp->budget = 20;
  while(count > 0 && cp->budget > 0){
    choice_t* draw = cp->choose(cp);
    if(!draw || draw->context == NULL)
      continue;

    item_def_t* idef = draw->context;
    if(!ItemCurate(idef))
      continue;

    AddPurchase(dp, count, 1, 2, idef, ChoiceReduceScore);
    //loot_item_t* loot = &lp->loots[lp->count++];
    //loot->ref = GameCalloc("GenerateLootPool", 1, sizeof(item_def_t));
    //loot->ref =idef;
    count--;

  }

  return lp;
}

void LootDraw(ent_t* e, int amnt){
  for(int i = 0; i < amnt; i++){
    choice_t* choice = Level.loot->drops->choose(Level.loot->drops);

    if(!choice || !choice->context)
      continue;

    item_def_t* def = choice->context;
    EntAddItem(e, InitItem(def), false);
  }

}
