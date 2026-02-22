#include "game_types.h"
#include "game_helpers.h"
#include "game_process.h"

item_t* InitItem(item_def_t* def){
  item_t* item = malloc(sizeof(item_t));
  game_object_uid_i gouid = GameObjectMakeUID(def->name, def->id, WorldGetTime());
  *item = (item_t){
    .gouid = gouid,
    .def = def
  };

  if(item_funcs[def->category].cat != ITEM_NONE){
    for(int i = 0; i < item_funcs[def->category].num_equip; i++)
      item->on_equip[i] = item_funcs[def->category].on_equip[i];
    for(int i = 0; i < item_funcs[def->category].num_use; i++)
      item->on_use = item_funcs[def->category].on_use[i];
  }

  return item;
}
void InventoryEnsureCap(inventory_t* t){
  if(t->count < t->cap)
    return;

 size_t new_cap = t->cap + 4;

  item_t* new_items =
    realloc(t->items, new_cap * sizeof(item_t));

  if (!new_items) {
    // Handle failure explicitly
    TraceLog(LOG_WARNING,"==== INVENTORY ===\n REALLOC FAILED");
  }

  t->items = new_items;
  t->cap = new_cap;

}

item_pool_t* InitItemPool(void) {
    item_pool_t* ip = GameCalloc("InitItemPool", 1, sizeof(item_pool_t));
    ip->size = 0;
    return ip;
}

inventory_t* InitInventory(ItemSlot id, ent_t* e, int cap, int limit){
  inventory_t* a = GameCalloc("InitInventory", 1,sizeof(inventory_t));

  *a = (inventory_t){
    .id     = id,
      .owner  = e,
      .cap  = cap,
      .limit = limit
  };

  define_inventory_t def = ITEMS_ALLOWED[id];

  if(def.cap>0)
    a->cap = def.cap;

  //a->cap = next_pow2_int(a->cap);
  for(int i = 0; i < ITEM_DONE; i++){
    a->limits[i] = def.limits[i];
    a->size = def.base_size;
  }

  a->space = a->size;
  a->unburdened = a->limit;

  a->items = GameCalloc("InitInventory items", a->cap, sizeof(item_t));
  return a;

}

void InventoryPoll(ent_t* e, ItemSlot id){
  inventory_t* inv = e->inventory[id];
  if(inv==NULL || !inv->active)
    return;

  for (int i = 0; i < inv->count; i++){
    if(!inv->items[i].equipped)
      continue;

    for(int j = 0; j < 2; j++)
    if(inv->items[i].on_equip[j])
      inv->items[i].on_equip[j](e, &inv->items[i]);
  }
}

void InventorySetPrefs(inventory_t* inv, uint64_t traits){
  if (!inv->active || inv->count < 2)
    return;

  choice_pool_t* items = InitChoicePool(inv->count, ChooseBest);
  for(int i = 0; i < inv->count; i++){
    item_t* item = &inv->items[i];
    item->equipped = false;
    SkillType s = item->def->skills[0];
    if(!item->owner->skills[s])
      continue; 

    int score = item->owner->skills[s]->val;
    AddChoice(items, i, score, item, NULL);
  }

  choice_t* sel = items->choose(items);

  item_t* pref = sel->context;

  pref->equipped = true;
}

ItemSlot InventoryGetAvailable(ent_t* e, ItemCategory cat, int size, int weight){
  for (int i = 0; i < INV_DONE; i++){
    if(e->inventory[i] == NULL || !e->inventory[i]->active)
      continue;

    if(e->inventory[i]->limits[cat] == 0)
      continue;

    if(e->inventory[i]->current[cat] >= e->inventory[i]->limits[cat])
      continue;

    if(e->inventory[i]->space < size)
      continue;

    if(e->inventory[i]->unburdened < weight)
      continue;

    return i;

  }

  return INV_NONE;
}

ItemSlot InventoryAddStorage(ent_t* e, item_t* i){
  ItemSlot s = i->def->type;
  inventory_t* slot = e->inventory[s];
  if(slot->container){

  }
  else{
    slot->active = true;
    slot->container = i;
    slot->space = slot->size = i->def->values[VAL_STORAGE]->val;
    slot->cap = i->def->values[VAL_QUANT]->val;
    slot->items = GameCalloc("InventoryAddStorage", slot->cap, sizeof(item_t));
    slot->limit = slot->unburdened = i->def->values[VAL_WEIGHT]->val;
    i->equipped = true;
    i->owner = e;
    for(int j = 0; j < ITEM_DONE; j++){
      if(slot->limits[j]>0)
        slot->limits[j] = slot->cap;
    }
  }
}

int InventorySlotAddItem(ent_t* e, ItemSlot id, item_t* i){
  inventory_t* inv = e->inventory[id];

  InventoryEnsureCap(inv);
  item_t* item = &inv->items[inv->count++];
  *item = *i;

  item->owner = e;
  item->equipped = inv->method[i->def->pref];

  inv->space-= item->def->values[VAL_STORAGE]->val;
  inv->unburdened-= item->def->values[VAL_WEIGHT]->val;

  inv->current[item->def->category]++;

  return inv->count-1;
    
}

item_t* InventoryAddItem(ent_t* e, item_t* i){
  int size   = i->def->values[VAL_STORAGE]->val;
  int weight = i->def->values[VAL_WEIGHT]->val;
  ItemSlot slot = INV_NONE;
  switch(i->def->category){
    case ITEM_CONTAINER:
      slot = InventoryAddStorage(e, i);
      return e->inventory[i->def->type]->container;
      break;
    default:
      slot = InventoryGetAvailable(e, i->def->category, size, weight);
      break;
  }
  if(slot <= INV_NONE)
    return NULL;
  else if (slot >= INV_DONE)
    DO_NOTHING();

  int index = InventorySlotAddItem(e, slot, i);
  return &e->inventory[slot]->items[index];
}

item_t* InventoryGetEquipped(ent_t* e, ItemSlot id){
  for (int i = 0; i < e->inventory[id]->count; i++){
    if(e->inventory[id]->items[i].equipped)
      return &e->inventory[id]->items[i];
  }

  return NULL;
}

item_def_t* DefineArmor(ItemInstance data){
  item_def_t* item = DefineArmorByType(data.equip_type, data.props, data.et_props);
  item->id = data.id;

  return item;
}

item_def_t* DefineArmorByType(ArmorType t, ItemProps p, ArmorProps a){
  item_def_t* item = GameCalloc("DefineArmorByType", 1,sizeof(item_def_t));
  item->category = ITEM_ARMOR;

  item->type = t;
  item->dr = GameCalloc("DefineArmorByType dr",1,sizeof(damage_reduction_t));

  item->ability = ABILITY_NONE;
  armor_def_t temp = ARMOR_TEMPLATES[t];
  //item->stats[STAT_ARMOR] = InitStat(STAT_ARMOR,0,temp.armor_class, temp.armor_class);

  item->values[VAL_DURI] = InitValue(VAL_DURI,temp.durability);
  item->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,temp.weight);
  item->values[VAL_WORTH] = InitValue(VAL_WORTH,temp.cost);
  item->values[VAL_ADV_SAVE] = InitValue(VAL_ADV_SAVE,0);
  item->values[VAL_SAVE] = InitValue(VAL_SAVE,temp.armor_class);
  item->values[VAL_STORAGE] = InitValue(VAL_STORAGE,temp.size);


  *item->dr = temp.dr_base;
  ApplyItemProps(item, p, a);

  //item->weight = temp.weight;

  item->ability = ABILITY_ARMOR_SAVE;
  item->skills[item->num_skills++] = temp.skill;

  for(int i = 0; i < VAL_ALL; i++){
    if(!item->values[i])
      continue;

    item->values[i]->val = ValueRebase(item->values[i]);
  }
  return item;

}

item_def_t* DefineWeapon(ItemInstance data){
  item_def_t* item = DefineWeaponByType(data.equip_type, data.props, data.et_props);

  item->id = data.id;

  return item;
}

item_def_t* DefineWeaponByType(WeaponType t, ItemProps props, WeaponProps w_props){
  item_def_t* item = GameCalloc("DefineWeaponByType", 1,sizeof(item_def_t));
  item->category = ITEM_WEAPON;
  item->type = t;
  weapon_def_t temp = WEAPON_TEMPLATES[t];

  //TODO CHANGE TO VALUE_T
  //item->weight = temp.weight;

  strcpy(item->name, temp.name);
  item->values[VAL_WORTH] = InitValue(VAL_WORTH,temp.cost);
  item->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,temp.weight);
  item->values[VAL_PENN] = InitValue(VAL_PENN,temp.penn);
  item->values[VAL_DURI] = InitValue(VAL_DURI,temp.durability);
  item->values[VAL_REACH] = InitValue(VAL_REACH,temp.reach_bonus);
  
  item->values[VAL_ADV_HIT] = InitValue(VAL_ADV_HIT,0);
  item->values[VAL_ADV_DMG] = InitValue(VAL_ADV_DMG,0);
 
  item->values[VAL_STORAGE] = InitValue(VAL_STORAGE,temp.size);
  
  item->skills[item->num_skills++] = temp.skill; 
  item->ability = temp.ability; 
  ApplyItemProps(item, props, w_props);
 
  for(int i = 0; i < VAL_ALL; i++){
    if(!item->values[i])
      continue;

    item->values[i]->val = ValueRebase(item->values[i]);
  } 
  return item;

}

item_def_t* DefineConsumable(ItemInstance data){
  item_def_t* item = GameCalloc("DefineConsumable", 1,sizeof(item_def_t));
  item->id = data.id;
  item->type = data.equip_type; 
  item->category = ITEM_CONSUMABLE;

  consume_def_t temp = CONSUME_TEMPLATES[data.equip_type];

  item->values[VAL_WORTH] = InitValue(VAL_WORTH,temp.cost);
  item->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,temp.weight);
  item->values[VAL_DURI] = InitValue(VAL_DURI,temp.quanity);
  item->values[VAL_EXP] = InitValue(VAL_EXP,temp.exp);
  item->values[VAL_STORAGE] = InitValue(VAL_STORAGE,temp.size);
 
  item->skills[item->num_skills++] = temp.skill;
  item->ability = temp.ability;


  ApplyItemProps(item, data.props, data.et_props);

  for(int i = 0; i < VAL_ALL; i++){
    if(!item->values[i])
      continue;

    item->values[i]->val = ValueRebase(item->values[i]);
  }
  return item;
}

item_def_t* DefineContainer(ItemInstance data){
  item_def_t* item = GameCalloc("DefineContainer", 1,sizeof(item_def_t));
  item->id = data.id;
  item->type = data.equip_type; 

  item->category = ITEM_CONTAINER;

  container_def_t temp = CONTAINER_TEMPLATES[item->type];

  item->values[VAL_WORTH] = InitValue(VAL_WORTH,temp.cost);
  item->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,temp.weight);
  item->values[VAL_QUANT] = InitValue(VAL_QUANT,temp.slots);
  item->values[VAL_STORAGE] = InitValue(VAL_STORAGE,temp.size);

  ApplyItemProps(item, data.props, data.et_props);

  for(int i = 0; i < VAL_ALL; i++){
    if(!item->values[i])
      continue;

    item->values[i]->val = ValueRebase(item->values[i]);
  }
  return item;
}

item_def_t* DefineItem(ItemInstance data){
  item_def_t* item;
  switch(data.cat){
    case ITEM_ARMOR:
      item = DefineArmor(data);
      break;
    case ITEM_WEAPON:
      item = DefineWeapon(data);
      break;
    case ITEM_CONSUMABLE:
      item = DefineConsumable(data);
      break;
    case ITEM_CONTAINER:
      item = DefineContainer(data);
      break;
    default:
      return NULL;
      break;
  }

  item->sprite = InitSpriteByID(data.icon, SHEET_ICON);
  item->sprite->slice->color = data.color;

  return item;
}

bool ItemApplyStats(struct ent_s* owner, item_t* item){
  for(int i = 0; i < VAL_ALL; i++){
    if(item->def->values[i] == NULL)
      continue;
    if(item->def->values[i]->stat_relates_to == STAT_NONE)
      continue;

    StatType rel = item->def->values[i]->stat_relates_to;
    owner->stats[rel]->bonus = item->def->values[i]->val;
    StatExpand(owner->stats[rel],item->def->values[i]->val,true);
  }
}

ability_t* InitAbilitySave(ent_t* owner, AbilityID id, define_natural_armor_t* def){
  ability_t* a = InitAbility(owner, id);

  a->hit = Die(def->armor_class, 1);

  a->skills[a->num_skills++] = def->skill;

  return a;
}

ability_t* InitAbilityDR(ent_t* owner, AbilityID id, define_natural_armor_t* def){
  ability_t* a = InitAbility(owner, id);
   a->dr = &def->dr;

   a->skills[a->num_skills++] = def->skill;
    
   return a;

}
ability_t* InitAbilityInnate(ent_t* e, AbilityID id, define_natural_armor_t* def){
  ability_t a = ABILITIES[id];

  switch(a.type){
    case AT_DMG:
      return InitAbility(e,id);
      break;
    case AT_SAVE:
      return InitAbilitySave(e, id, def);
      break;
    case AT_DR:
      return InitAbilityDR(e,id, def);
      break;
  }

  return NULL;
}

ability_t* InitAbility(ent_t* owner, AbilityID id){
  ability_t* a = GameCalloc("InitAbility", 1,sizeof(ability_t));

  *a = AbilityLookup(id);

  a->hit = Die(20,1);

  a->dc = Die(a->side,a->die);

  if(a->use_fn == NULL)
    a->use_fn = EntUseAbility;

  if(a->type == AT_DMG)
   a->sim_fn = AbilitySimDmg;

  a->on_use_cb = AbilitySkillup;

  a->stats[STAT_REACH] = InitStat(STAT_REACH,1,a->reach,a->reach);
  a->stats[STAT_DAMAGE] = InitStatOnMax(STAT_DAMAGE,a->bonus,a->mod);

  for (int i = 0; i < STAT_DONE; i++){
    if(!a->stats[i])
      continue;
    a->stats[i]->owner = owner;
  }

  for(int i = 0; i < VAL_WORTH; i++){
    a->values[i] = InitValue(i,0);
  }

  if(a->chain_id > ABILITY_NONE){
    a->chain = InitAbility(owner, a->chain_id);
    a->chain_fn = EntUseAbility;
  }

  return a;
}

bool ItemAddAbility(struct ent_s* owner, item_t* item){
  const item_def_t* def = item->def;

  ability_t* a = InitAbility(owner, def->ability);
//  a->cost = def->
  for(int i = 0; i < VAL_WORTH; i++){
    if(def->values[i]==NULL)
      continue;
    if(def->values[i]->val == 0)
      continue;

    switch(i){
      case VAL_ADV_HIT:
      case VAL_ADV_DMG:
      case VAL_ADV_SAVE:
      case VAL_HIT:
      case VAL_SAVE:
        a->values[i]->on_change = ValueUpdateDie;
        break;
      case VAL_REACH:
        a->values[i]->on_change = ValueUpdateStat;
      default:
        break;
    }

    AbilityApplyValues(a, def->values[i]);
  }

  a->dr = def->dr;
  for(int i = 0; i < def->num_skills; i++)
    a->skills[a->num_skills++] = def->skills[i];

  int pref = owner->skills[def->skills[0]]->val * 10;
  if(a->chain){
    a->chain->skills[a->chain->num_skills++] = def->skills[0];

    if(a->chain->type == AT_DR){
      a->chain->dr = def->dr;
      ActionSlotAddAbility(owner,a->chain);
    }
    else
      a->chain->weight+=pref;
  }
  if(a->type != AT_SAVE)
    a->weight+=pref;
  
  a->item = item;
  item->ability = a;
  return ActionSlotAddAbility(owner, a);
}

bool ItemSkillup(ent_t* owner, item_t* item, InteractResult result){
  int exp = item->def->values[VAL_EXP]->val;
  if(exp == 0)
    return false;

  for(int i = 0; i < item->def->num_skills; i++)
    SkillUse(owner->skills[item->def->skills[i]],owner->uid,item->def->category,exp,result);

  return true;
}

int GetWeaponByTrait(Traits t, weapon_def_t *arms){
  int count = 0;
  for (int i = 0; i < WEAP_DONE; i++){
    if(t & WEAPON_TEMPLATES[i].skill != 0)
      continue;

    arms[count++] = WEAPON_TEMPLATES[i];
  }

  return count;
}


void ApplyItemProps(item_def_t *w, ItemProps props, uint64_t e_props){
  while(props){
    uint64_t prop = props & -props;
    props &= props - 1;
    for(int i = 0; i < NUM_ITEM_PROPS; i++){
      if(PROP_MODS[ITEM_NONE][i].propID != prop)
        continue;

      item_prop_mod_t mod = PROP_MODS[ITEM_NONE][i];
      for(int i = 0; i < mod.num_aff; i++){
        if(w->values[mod.val_change[i].modifies])
          ValueAddBaseMod(w->values[mod.val_change[i].modifies], mod.val_change[i]);
      }
    }
  }

  while(e_props){
    uint64_t wprop = e_props & -e_props;
    e_props &= e_props -1;

    for(int i = 0; i < NUM_WEAP_PROPS; i++){

      if(PROP_MODS[w->category][i].propID != wprop)
        continue;

      item_prop_mod_t mod = PROP_MODS[w->category][i];

      if(mod.add_skill > SKILL_LVL)
        w->skills[w->num_skills++] = mod.add_skill;
      for(int i = 0; i < mod.num_aff; i++){
      if(mod.val_change[i].affix>AFF_NONE)
        ValueAddBaseMod(w->values[mod.val_change[i].modifies], mod.val_change[i]);
      }
    }
  }
}

ItemProps GetItemQualByRaceProp(RaceProp prop){
  //Increments the quality by 1
  switch(prop){
    case RACE_BUILD_CRUDE:
    case RACE_BUILD_SIMPLE:
    case RACE_BUILD_BASIC:
    case RACE_BUILD_SOPH:
    case RACE_ARMS_ARTISAN:
      return PROP_QUAL_TRASH;
      break;
    default:
      break;
  }
}

ItemProps GetItemMatsByRaceProp(RaceProp prop){
  switch(prop){
    case RACE_ARMS_CRUDE:
    case RACE_ARMS_SIMPLE:
      return PROP_MAT_BONE | PROP_MAT_WOOD | PROP_MAT_STONE;
      break;
    case RACE_ARMOR_CRUDE:
      return PROP_MAT_CLOTH | PROP_MAT_LEATHER | PROP_MAT_BONE;
      break;
    case RACE_ARMS_ARTISAN:
      return PROP_WEAP_MARTIAL;
      break;
    default:
      break;
  }
}

ArmorProps GetArmorPropsByRaceProp(RaceProp prop){
  switch(prop){
    case RACE_ARMOR_CRUDE:
    case RACE_ARMOR_SIMPLE:
    default:
      return PROP_ARMOR_NONE;
      break;
  }
}

WeaponProps GetWeaponPropsByRaceProp(RaceProp prop){
  switch(prop){
    case RACE_ARMS_CRUDE:
    case RACE_ARMS_SIMPLE:
      return PROP_WEAP_SIMP;
      break;
  }
}

item_def_t* BuildArmor(SkillType skill, ItemProps props, ArmorProps w_props){

  ArmorType type = GetArmorTypeBySkill(skill);

  item_def_t* item = DefineArmorByType(type, props, w_props);
  item->pref = INV_WORN;
  return item;
}

item_def_t* BuildArmorForMob(ent_t* e, RaceProps props,SkillType sk){
  RaceProps r_props = props & RACE_ARMOR_MASK;
  r_props |= props & RACE_BUILD_MASK;

  ItemProps qual_props = 0;//PROP_QUAL_TRASH;
  ItemProps mat_props = PROP_NONE;
  ArmorProps a_props = PROP_NONE;

  while(r_props){
    uint64_t rprop = r_props & - r_props;
    r_props &= r_props -1;
    a_props |= GetArmorPropsByRaceProp(rprop);
    mat_props |= GetItemMatsByRaceProp(rprop);
    qual_props += GetItemQualByRaceProp(rprop);
  }

  return BuildArmor(sk, qual_props | mat_props, a_props);

}

item_def_t* BuildSpecialForMob(ent_t* e, RaceProps props){
  RaceProps s_props = props & RACE_SPECIAL_MASK;

}

item_def_t* BuildWeaponForMob(ent_t* e, RaceProps props, SkillType sk){
  RaceProps a_props = props & RACE_ARMS_MASK;
  a_props |= props & RACE_BUILD_MASK;
  ItemProps qual_props = 0;//PROP_QUAL_TRASH;
  ItemProps mat_props = PROP_NONE;
  WeaponProps w_props = PROP_NONE;

  while(a_props){
    uint64_t rprop = a_props & - a_props;
    a_props &= a_props -1;
    w_props |= GetWeaponPropsByRaceProp(rprop);
    mat_props |= GetItemMatsByRaceProp(rprop);
    qual_props += GetItemQualByRaceProp(rprop);
  }

  return BuildWeapon(sk, qual_props | mat_props, w_props);
}

item_def_t* BuildAppropriateItem(ent_t* e, ItemCategory cat, SkillType s){
  item_def_t* item;
  RaceProps r_props = GetRaceByFlag(e->props->race).props;

  switch(cat){
    case ITEM_WEAPON:
    item = BuildWeaponForMob(e, r_props, s);
    break;
    case ITEM_ARMOR:
    item = BuildArmorForMob(e, r_props, s);
    break;
    case ITEM_CONSUMABLE:
    item = BuildSpecialForMob(e, r_props);
    break;
  }

  return item;
}

item_def_t* BuildWeapon(SkillType skill, ItemProps props, WeaponProps w_props){

  WeaponType type = GetWeapTypeBySkill(skill);

  item_def_t* item = DefineWeaponByType(type, props, w_props);

  item->pref = INV_HELD;
  return item;
}

int AbilityAddPB(ent_t* e, ability_t* a, StatType s){
  SkillRank rank = SkillRankGet(e->skills[a->skills[0]]);
  if(rank < SR_SKILLED)
    return 0;

  define_skill_rank_t dsr = SKILL_RANKS[rank];
  if(dsr.proficiency > MOD_NONE){
    a->stats[s]->base = e->stats[s]->base;
    for (int i = 0; i < ATTR_DONE; i++){
      if(a->stats[s]->modified_by[i] > MOD_NONE)
        a->stats[s]->modified_by[i] = dsr.proficiency;
    }
    a->stats[s]->start(a->stats[s]);
  }
  return a->stats[s]->current;
}

ability_sim_t* AbilitySimDmg(ent_t* owner,  ability_t* a, ent_t* target){
  ability_sim_t* res = GameCalloc("AbilitySimDmg", 1,sizeof(ability_sim_t));

  res->id = a->id;
  res->type = a->type;
  res->d_type = a->school;
  res->d_bonus = AbilityAddPB(owner, a, STAT_DAMAGE);
  res->dmg_die = a->dc->num_die;
  res->dmg_sides = a->dc->sides;
  res->penn = a->values[VAL_PENN]->val;

  res->hit_calc = a->hit->roll(a->hit, res->hit_res);
  res->dmg_calc = a->dc->roll(a->dc, res->dmg_res);

  res->final_dmg = res->dmg_calc + res->d_bonus;
  return res;
}

bool AbilityRankup(ent_t* owner, ability_t* a){
  ability_t base = ABILITIES[a->id];
  for(int i = 0; i < VAL_EXP; i++){
    if(base.rankup[i]==0)
      continue;
  }
}

ability_sim_t* AbilitySimMax(ent_t* owner,  ability_t* a){
  ability_sim_t* res = GameCalloc("AbilitySimMax", 1,sizeof(ability_sim_t));

  res->id = a->id;
  res->type = a->type;
  res->d_type = a->school;
  res->d_bonus = AbilityAddPB(owner, a, STAT_DAMAGE);
  res->dmg_die = a->dc->num_die;
  res->dmg_sides = a->dc->sides;
  res->penn = a->values[VAL_PENN]->val;

  res->hit_calc = DieMax(a->hit);
  res->dmg_calc = DieMax(a->dc);

  for(int i = 0; i < res->dmg_die; i++)
    res->dmg_res[i] = res->dmg_sides;

  res->final_dmg = res->dmg_calc + res->d_bonus;
  return res;
}

int AbilitySimulate(ability_t* a, local_ctx_t* ctx){
  ent_t* owner = a->stats[STAT_REACH]->owner;
  ent_t* e = ParamReadEnt(&ctx->other);
  ability_t* dr = EntFindAbility(e, ABILITY_ARMOR_DR);

  ability_sim_t* sim = AbilitySimMax(owner, a);

  if(dr && dr->save_fn)
    dr->save_fn(e, dr, sim);

  return sim->final_dmg;
}

bool AbilityCanTarget(ability_t* a, local_ctx_t* target){
  if(!a)
    return false;

  if(!target)
    return false;

  if(a->type == SLOT_SAVE)
    return false;

  ent_t* e = a->stats[STAT_REACH]->owner;

  if(a->resource != STATE_NONE){
    int res = e->stats[a->resource]->current;
    if(a->cost > res)
      return false;
  }

  if(!HasLOS(e->map, e->pos, target->pos))
    return false;

  int reach = a->stats[STAT_REACH]->current;
  if( target->dist <= reach)
    return true;

  return false;
}

void DamageEvent(EventType ev, void* edata, void* udata){
  ent_t* e = udata;
  ability_t* a = edata;

  SetState(e, STATE_AGGRO, NULL);
}


trigger_t* InitTriggerAffect(ent_t* e, trigger_t t, affect_t* aff){
  trigger_t* trig = GameCalloc("InitTriggerAffect", 1, sizeof(trigger_t));
   *trig =  t;
   trig->owner = e;

   event_fuid_i fuid = EventMakeFlexID(e->gouid, trig->id);
   if(fuid == -1)
     return NULL;

   WorldTargetSubscribe(trig->event, aff->apply, aff, fuid);

   return trig;
}

trigger_t* InitTriggerTick(ent_t* e, trigger_t t, affect_t* aff){
  trigger_t* trig = GameCalloc("InitTriggerTick", 1, sizeof(trigger_t));
   *trig =  t;
   trig->owner = e;

   WorldTargetSubscribe(aff->trigger.event, aff->tick, aff, e->gouid);

   return trig;
}

void InitAffect(ent_t* e, AffectID id, int val){
  affect_t* aff = GetAffectByID(id);
  if(!aff)
    return;//TODO ERROR/WARN
  
  aff->hit = Die(aff->vals[VAL_HIT], 1);
  aff->aff = Die(aff->vals[VAL_ADV_HIT], 1);
  trigger_t* apply = InitTriggerAffect(e, aff->source, aff);

}

void AffectHitRoll(EventType, void* edat, void* udat){
  affect_t* aff = udat;
  ent_t* e = edat;

  if(!aff->hit)
    return;

  int res[1];
  if(!aff->hit->roll(aff->hit,res))
    return;
  
  //aff->trigger = InitTriggerTick(e, aff->trigger, aff);
  WorldTargetSubscribe(aff->trigger.event, aff->tick, aff, e->gouid);
}

void AffectRollAdv(EventType, void* edat, void* udat){
  affect_t* aff = udat;
  combat_t* c = edat;

  combat_context_t* cc = c->cctx[IM_TAR];
  ent_t* e = ParamReadEnt(&cc->ctx[IP_OWNER]);
  switch(aff->trigger.cause){
    case PARAM_ABILITY:
      ability_t* a = ParamRead(&cc->ctx[IP_ABILITY], ability_t);
      if(aff->vals[VAL_DMG_DIE] != 0)
        a->dc->advantage += aff->vals[VAL_DMG_DIE];
      break;
  }
}


void AffectTick(EventType, void* edat, void* udat){
  affect_t* aff = udat;
  ent_t* e = edat;



}

void InitFeat(ent_t* e, feat_t* f){
  for (int i = 0; i < f->num_rewards; i++){
    reward_t rew = f->rewards[i];
    switch(rew.type){
      case AFF_MOD_ABI:
        InitAffect(e, rew.type_id, rew.val);
        break;
    }
  }

}
