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
    case EVENT_MAP_LOADED:
      map_grid_t* m = data;
      LevelReady(m);
      break;
  }
}

void LootDrops(ent_t* e){
  TraceLog(LOG_INFO, "%s base challenge %f", e->name, e->props->base_diff);
  int budget = e->props->base_diff * 100;

  mob_define_t mdef =  MONSTER_MASH[e->type];
  LootFlags flags = mdef.loot;

  int amnt = RandRange(0, mdef.cost);

  if(flags == 0 || amnt < 1)
   return;

  TraceLog(LOG_INFO, "Generate %i loot with budget %i", amnt, budget);
  Rectangle area = {e->pos.x -2, e->pos.y -2, 5, 5};
  LootDrop(flags, budget, amnt, area);

}

void LevelEntityEvent(EventType event, void* data, void* user){
  ent_t* e = data;

  switch(event){
    case EVENT_ENT_DEATH:
      if(e->last_hit_by == player)
      LootDrops(e);
      break;
  }


}

level_t* InitLevel(void){
  uint64_t addr = (uint64_t)&Level;
  Level.seed = hash_combine_64(WorldSeed(), addr);
  InitRng(&Level.rng, Level.seed);
  Level.paths = InitPathPool(MAX_ENVS);

  WorldSubscribe(EVENT_ROOM_READY, OnLevelReady, &Level);
  WorldSubscribe(EVENT_MAP_LOADED, OnLevelReady, &Level);

  Level.item_defs = InitItemGenPool(ENT_DONE + ABILITY_DONE + SKILL_DONE);

  for(int i = 0; i < ABILITY_DONE; i++){
    if(CLASS_ABILITIES[i].tome && CLASS_ABILITIES[i].lvl == 0)
      ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGenerateKnowledge(i, CONS_TOME));     
    if(CLASS_ABILITIES[i].scroll)
      ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGenerateKnowledge(i, CONS_SCROLL));     
      }

  for (int i = 2; i < SKILL_DONE; i++)
      ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGenerateKnowledge(i, CONS_SKILLUP));     
 
  ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGeneratePotion(STAT_ENERGY_REGEN)); 
  ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGeneratePotion(STAT_HEALTH_REGEN)); 
  ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGeneratePotion(STAT_STAMINA_REGEN)); 
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

  material_def_t* mats[MAX_ENTS];
  size_t num_mats = 0; 
  for (int i = 0; i < MAT_DONE; i++)
  GetMaterialByBiome(i, mats, &num_mats, MAX_ENTS);

  for(int i = 0; i < num_mats; i++)
      ItemGenAdd(Level.item_defs, ITEM_MATERIAL, mats[i]);

  Level.loot = GenerateLootPool((num_mats * 4) + Level.item_defs->count, ctx);

  WorldSubscribe(EVENT_ENT_DEATH, LevelEntityEvent, &Level);

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

  int num_armor =0, num_weapon = 0, num_consume = 0;
  loot_pool_t* lp = GameCalloc("GenerateLootPool", 1, sizeof(loot_pool_t));

  lp->rules = ctx;

  /*
     choice_pool_t *cp = StartChoice(&lp->flags, count * 2, ChooseByWeightInBudget, &result);
     */
  bool ready = false;
  choice_pool_t* dp;

  dp = StartChoice(&lp->drops, count, WeightedPurchaseByFlags, &ready);

  while(num_weapon < 15 && num_armor < 25){
    param_t p[LOOT_PARAM_END] = {0};

    int cat = RngRoll(Level.rng, ITEM_WEAPON, ITEM_DONE);
    p[LOOT_PARAM_CATEGORY] = ParamMake(DATA_INT, sizeof(int),&cat);
    int start = 0, end = 0;
    int type_prop_end = -1;
    LootParams type_param = -1;
    switch (cat){
      case ITEM_WEAPON:
        start = WEAP_MACE;
        end =  WEAP_DAGGER;
        type_prop_end = 9;
        type_param = LOOT_PARAM_WEAP;
        num_weapon++;
        break;
      case ITEM_ARMOR:
        start = ARMOR_NATURAL;
        end = ARMOR_SHIELD-1;
        type_prop_end = 0;
        type_param = LOOT_PARAM_ARMOR;
        num_armor++;
        break;
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

    if(!ItemCurate(item))
      continue;

    switch(qual){
      case PROP_QUAL_TRASH:
        item->flags |= LF_TRASH;
        break;
      case PROP_QUAL_POOR:
        item->flags |= LF_POOR;
        break;
      default:
        item->flags |= LF_COMMON;
        break;
    }
    switch(mat){
      case PROP_MAT_BONE:
        item->flags |= LF_MAT_BONE;
        break;
      case PROP_MAT_LEATHER:
        item->flags |= LF_MAT_HIDE;
        break;
    }

    AddChoiceCostFlags(dp, type, item->weight, item->cost, item, item->flags, ChoiceReduceScore);
  }

  for (int i = 0; i < Level.item_defs->count; i++){
    item_type_d* item = &Level.item_defs->entries[i];

    item_def_t* def = DefineConsumableByDef(&item->data.cons);
    int weight = item->data.cons.weight;
    def->flags |= item->flags;
    def->cost = item->data.cons.cost;
 TraceLog(LOG_INFO, "%s: score %i cost %i", def->name, weight, def->cost);  
    def->id = item->data.cons.chain_id; 
      
    AddChoiceCostFlags(dp, def->id, weight, def->cost, def, def->flags, ChoiceReduceScore);
  }

  ShuffleChoices(dp);
  return lp;
}

void LootDraw(ent_t* e, LootFlags flags, bool equip, int budget, int amnt){
  if(Level.loot->drops->count < 1)
    return;

  int i = 0;
  Level.loot->drops->budget = budget;
  Level.loot->drops->flags = flags;
  
  while (i < amnt && Level.loot->drops->budget > 0){
    choice_t* choice = Level.loot->drops->choose(Level.loot->drops);

    if(!choice || !choice->context)
      break;

    item_def_t* def = choice->context;
    if(EntAddItem(e, InitItem(def), equip)){
      i++;
      TraceLog(LOG_INFO, "Added item - %s", def->name);
    }
  }
}

choice_pool_t* LootPlacements(map_grid_t* m, Rectangle r, int amount){
  int area = r.width * r.height;
  Cell center = rect_center(r);
  choice_pool_t* cp = InitChoicePool(area, ChooseByWeight);
  for (int x = r.x; x < r.x + r.width; x++){
    for (int y = r.y; y < r.y + r.height; y++){

      map_cell_t* c = &m->tiles[x][y];

      if(TileBlocksMovement(c))
        continue;

      int dist = 1 + cell_distance(center, c->coords);
      AddChoice(cp, x*1000+y, area/dist, c, DiscardChoice);

    }
  }

  if(cp->count < 1)
    return NULL;

  return cp;
}

void LootDrop(LootFlags flags, int budget, int amnt, Rectangle r){

  if(Level.loot->drops->count < 1)
    return;

  int i = 0;
  Level.loot->drops->budget = budget;
  Level.loot->drops->flags = flags;

  choice_pool_t* placements = LootPlacements(WorldGetMap(), r, amnt);
  if(!placements || placements->count < 1)
    return;

  while (placements->count > 0 && i < amnt && Level.loot->drops->budget > 0){
    choice_t* choice = Level.loot->drops->choose(Level.loot->drops);

    if(!choice || !choice->context)
      break;

    item_def_t* def = choice->context;

    choice_t* pos = placements->choose(placements);

    if(!pos || !pos->context)
      break;

    map_cell_t* mc = pos->context;

    if(InitItemContext(def, mc->coords))
      i++;

  }
}
