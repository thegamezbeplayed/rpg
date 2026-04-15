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
  int budget = e->props->base_diff * 100;

  LootFlags flags = e->props->loot;

  int amnt = RandRange(0, e->props->base_diff*3);

  if(flags == 0 || amnt < 1)
   return;

  Rectangle area = {e->pos.x -2, e->pos.y -2, 5, 5};
  LootDrop(flags, budget, amnt, area);

}

void ItemGenerateItemFromMaterial(material_def_t* mat){

  for (int i = 2; i < TOOL_DONE; i++){
    Resources res = TOOL_TEMPLATES[i].reagents;

      if((mat->m_props & PROP_MAT_TOOLING) == 0)
        continue;

      if((MAT_RES_MAP[mat->type].other & res) == 0)
        continue;

      tool_def_t* tdef = ToolGenerate(i, mat->spec);
      ItemGenAdd(Level.item_defs, ITEM_TOOL, tdef);

  }
  for (int i = 1; i < WEAP_DAGGER; i++){
    weapon_def_t* weap = &WEAPON_TEMPLATES[i];
    if((mat->m_props & weap->m_props) < weap->m_props)
      continue;

    weapon_def_t *wdef = WeaponGenerate(weap, mat);
    ItemGenAdd(Level.item_defs, ITEM_WEAPON, wdef);
  }

  for (int i = 2; i < ARMOR_DONE; i++){
    armor_def_t* armor = &ARMOR_TEMPLATES[i];
      if((mat->m_props & armor->m_props) < armor->m_props)
        continue;

      if((mat->i_props & armor->i_props) == 0)
        continue;

      armor_def_t *adef = ArmorGenerate(armor, mat);
      ItemGenAdd(Level.item_defs, ITEM_ARMOR, adef);
  }
}

void OnLevelEvent(EventType event, void* data, void* user){
  switch(event){
    case EVENT_REGISTER_MATERIAL:
    material_spec_d* spec = data;
    material_def_t base = MATERIAL_TEMPLATES[spec->type];
    material_def_t* mdef = InitMaterial(base, spec);
    ItemGenAdd(Level.item_defs, ITEM_MATERIAL, mdef);
    ItemGenerateItemFromMaterial(mdef);
    break;
    case EVENT_LOAD_MAP:
    map_grid_t* m = data;

    map_gen_t mgen = MAPS[m->id];

    int total = 0;
    for (int i = 0; i < NUM_FACTIONS; i++){
      int ratio = FACTIONS[i]->bio_pref[mgen.biome];

      if(ratio == 0)
        continue;

      int size = MAX_ENTS * ratio / 100;
      Level.factions[Level.num_factions++] = InitFactionGroups(i, size);

      total += size;
    }

    Level.spawns = InitSpawnPool(total);
    break;
  }
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

  Level.materials = InitMaterialTable(ENV_DONE + ENT_DONE);

  WorldSubscribe(EVENT_ROOM_READY, OnLevelReady, &Level);
  WorldSubscribe(EVENT_MAP_LOADED, OnLevelReady, &Level);
  WorldSubscribe(EVENT_LOAD_MAP, OnLevelEvent, &Level);


  Level.item_defs = InitItemGenPool(ENV_DONE + ENT_DONE + ABILITY_DONE + SKILL_DONE);

  for(int i = 0; i < ABILITY_DONE; i++){
    if(ABILITY_DATA[i].tome && ABILITY_DATA[i].lvl == 0)
      ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGenerateKnowledge(i, CONS_TOME));     
    if(ABILITY_DATA[i].scroll)
      ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGenerateKnowledge(i, CONS_SCROLL));     
      }

  for (int i = 2; i < SKILL_DONE; i++)
      ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGenerateKnowledge(i, CONS_SKILLUP));     
 
  ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGeneratePotion(STAT_ENERGY_REGEN)); 
  ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGeneratePotion(STAT_HEALTH_REGEN)); 
  ItemGenAdd(Level.item_defs, ITEM_CONSUMABLE, ConsumeGeneratePotion(STAT_STAMINA_REGEN)); 

  WorldSubscribe(EVENT_REGISTER_MATERIAL, OnLevelEvent, &Level);
  return &Level;
}

void LevelReady(map_grid_t* m){

  for(int x = 0; x < m->width; x++){
    for(int y = 0; y < m->height; y++){
      map_cell_t* mc = &m->tiles[x][y];
      Cell c = CELL_NEW(x,y);
      
      WorldTargetSubscribe(EVENT_ENT_STEP, MapVisEvent, mc, player->gouid);      
      mc->vis = VIS_UNSEEN;
      WorldTargetSubscribe(SCREEN_EVENT_ACTIVATE_SEL, MapVisEvent, mc, mc->gouid);
      WorldTargetSubscribe(SCREEN_EVENT_SEL_END, MapVisEvent, mc, mc->gouid);

    }
  }

  map_gen_t mgen = MAPS[m->id];
  loot_ctx_t *ctx = GameCalloc("LevelReady", 1, sizeof(loot_ctx_t));
  
  ctx->dungeon_lvl = iround(mgen.max_diff);
  ctx->biome = mgen.biome;
  ctx->luck = -1;
  ctx->enemy_cr = iround(mgen.diff);
  ctx->seed = hash_combine_64(Level.seed, mgen.biome);

  Level.loot = GenerateLootPool(Level.materials->count + Level.item_defs->count, ctx);

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

  bool ready = false;
  choice_pool_t* dp;

  dp = StartChoice(&lp->drops, count, WeightedPurchaseByFlags, &ready);

  for (int i = 0; i < Level.item_defs->count; i++){
    LootFlags lootflags = 0;
    item_type_d* item = &Level.item_defs->entries[i];
    item_def_t* def;
    int weight = 0, cost = 0;
    switch(item->cat){
      case ITEM_CONSUMABLE:
        def = DefineConsumableByDef(&item->data.cons);
        lootflags = item->data.cons.loot;
        weight = item->data.cons.weight;
        def->cost = item->data.cons.cost;
        break;
      case ITEM_MATERIAL:
        def = DefineMaterial(&item->data.mat);
        lootflags = item->data.mat.loot;
        weight = def->weight;
        cost = def->cost;
        break;
      case ITEM_TOOL:
        def = DefineTool(&item->data.tool);
        lootflags = item->data.tool.loot;
        weight = def->weight;
        cost = def->cost;
        break;
      case ITEM_WEAPON:
        def = DefineWeapon(&item->data.weap);
        lootflags = item->data.weap.loot;
        weight = def->weight;
        cost = def->cost;
        break;
      case ITEM_ARMOR:
        def = DefineArmor(&item->data.armor);
        weight = def->weight;
        lootflags = item->data.armor.loot;
        cost = def->cost;
        break;
    }
    def->id = item->data.cons.chain_id; 
      
    AddChoiceCostFlags(dp, def->id, weight, def->cost, def, lootflags, ChoiceReduceScore);
  }

  ShuffleChoices(dp);
  return lp;
}

void LootDraw(ent_t* e, LootFlags flags, bool unique, bool equip, int budget, int amnt){
  if(Level.loot->drops->count < 1)
    return;

  int i = 0;
  int attempts = 0;
  Level.loot->drops->budget = budget;
  Level.loot->drops->flags = flags;
  
  ItemAddFn fn = unique? ItemAddUnique: ItemAdd;

  while (i < amnt && Level.loot->drops->budget > 0){
    attempts++;
    choice_t* choice = Level.loot->drops->choose(Level.loot->drops);

    if(!choice || !choice->context){
      if(attempts > (amnt * 3))
        return;
       
      TraceLog(LOG_INFO, "==== LOOT DRAW ====\n Could not find item %llu with budget %i pool size of %i", flags, budget, Level.loot->drops->count);
      continue;
    }

    item_def_t* def = choice->context;
    item_t* item = InitItem(def);
    if(fn(e, item, equip)){
      Level.loot->drops->budget += choice->cost;
      i++;
      TraceLog(LOG_INFO, "Added item - %s", def->name);
    }
    else{
      TraceLog(LOG_INFO, "Item - %s not added ====\n size %i weight %i", def->name, item->values[VAL_STORAGE]->val, item->values[VAL_WEIGHT]->val);
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

    if(InitItemContext(def, mc))
      i++;

  }
}
