#include "game_types.h"
#include "game_helpers.h"
#include "game_process.h"
#include "game_strings.h"
static int NUM_ITEMS = 0;

void ItemArmorPropVals(item_t* i){
  armor_def_t temp = ARMOR_TEMPLATES[i->def->type];
  i->values[VAL_DURI] = InitValue(VAL_DURI,temp.durability);
  i->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,temp.weight);
  i->values[VAL_WORTH] = InitValue(VAL_WORTH,temp.cost);
  i->values[VAL_ADV_SAVE] = InitValue(VAL_ADV_SAVE,0);
  i->values[VAL_SAVE] = InitValue(VAL_SAVE,temp.armor_class);
  i->values[VAL_STORAGE] = InitValue(VAL_STORAGE,temp.size);

  switch(temp.type){
  case ARMOR_CLOTH:
    i->sprite = InitSpriteByID(ICON_SHIRT, SHEET_ICON);
    i->sprite->slice->color = BEIGE;
    break;
  case ARMOR_PADDED:
    i->sprite = InitSpriteByID(ICON_SHIRT, SHEET_ICON);
    i->sprite->slice->color = BROWN;
    break;
  case ARMOR_LEATHER:
    i->sprite = InitSpriteByID(ICON_TUNIC_COM, SHEET_ICON);
    i->sprite->slice->color = BROWN;
    break;
  case ARMOR_CHAIN:
    i->sprite = InitSpriteByID(ICON_CURAISS, SHEET_ICON);
    i->sprite->slice->color = GRAY;
    break;
  case ARMOR_PLATE:
    i->sprite = InitSpriteByID(ICON_CURAISS, SHEET_ICON);
    i->sprite->slice->color = RAYWHITE;
    break;
  case ARMOR_SHIELD:
    break;
  }
}

void ItemConsumePropVals(item_t* i){
  switch(i->def->type){
    case CONS_SCROLL:
    case CONS_TOME:
    case CONS_SKILLUP:
      i->on_acquire = ItemAddAbility;
      break;
    default:
      break;
  }
  consume_def_t *def = i->def->type_def;

  i->values[VAL_WORTH] = InitValue(VAL_WORTH,def->cost);
  i->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,def->weight);
  i->values[VAL_DURI] = InitValue(VAL_DURI,def->quanity);
  i->values[VAL_QUANT] = InitValue(VAL_QUANT,def->quanity);
  i->values[VAL_EXP] = InitValue(VAL_EXP,def->exp);
  i->values[VAL_STORAGE] = InitValue(VAL_STORAGE,def->size);

  i->values[VAL_DURI]->on_empty = ItemDestroy;
  i->values[VAL_QUANT]->on_empty = ItemDestroy;
  i->skills[i->num_skills++] = def->skill;


  switch(def->type){
    case CONS_SCROLL:
      i->sprite = InitSpriteByID(ICON_SCROLL, SHEET_ICON);
      break;
    case CONS_TOME:
      i->sprite = InitSpriteByID(ICON_TOME, SHEET_ICON);
      ability_t a = ABILITIES[def->chain_id];
      i->sprite->slice->color = DAMAGE_SCHOOL[a.school].col;
      break;
    case CONS_SKILLUP:
      i->sprite = InitSpriteByID(ICON_TOME, SHEET_ICON);
      break;

  };
}

void ItemWeapPropVals(item_t* i){
  weapon_def_t *temp = i->def->type_def;

  i->values[VAL_WORTH] = InitValue(VAL_WORTH,temp->cost);
  i->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,temp->weight);
  i->values[VAL_PENN] = InitValue(VAL_PENN,temp->penn);
  i->values[VAL_DURI] = InitValue(VAL_DURI,temp->durability);
  i->values[VAL_REACH] = InitValue(VAL_REACH,temp->reach_bonus);

  i->values[VAL_ADV_HIT] = InitValue(VAL_ADV_HIT,0);
  i->values[VAL_ADV_DMG] = InitValue(VAL_ADV_DMG,0);

  i->values[VAL_STORAGE] = InitValue(VAL_STORAGE,temp->size);

  i->skills[i->num_skills++] = temp->skill;

  switch(temp->type){
    case WEAP_MACE:
      i->sprite = InitSpriteByID(ICON_MACE_COM, SHEET_ICON);
      i->sprite->slice->color = DARKGRAY;
      break;
    case WEAP_SWORD:
      i->sprite = InitSpriteByID(ICON_SWORD, SHEET_ICON);
      i->sprite->slice->color = RAYWHITE;
      break;
    case WEAP_AXE:
      i->sprite = InitSpriteByID(ICON_HATCHET, SHEET_ICON);
      i->sprite->slice->color = DARKBROWN;
      break;
    case WEAP_DAGGER:
      i->sprite = InitSpriteByID(ICON_DAGGER, SHEET_ICON);
      i->sprite->slice->color = RAYWHITE;
      break;
    case WEAP_JAVELIN:
    case WEAP_BOW:
    case WEAP_CROSS_BOW:
    case WEAP_POLE_ARM:
    case WEAP_STAFF:
    case WEAP_PICK:
    case WEAP_WHIP:
    case WEAP_DART:
    case WEAP_SLING:
    case WEAP_NET:
    case WEAP_BLOW:
    case WEAP_SICKLE:
      break;
  }
}

void ItemContainerPropVals(item_t* i){
  container_def_t* temp = i->def->type_def;
  i->values[VAL_WORTH] = InitValue(VAL_WORTH,temp->cost);
  i->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,temp->weight);
  i->values[VAL_QUANT] = InitValue(VAL_QUANT,temp->slots);
  i->values[VAL_STORAGE] = InitValue(VAL_STORAGE,temp->size);
}

item_t* InitItem(item_def_t* def){
  item_t* item = GameMalloc("InitItem", sizeof(item_t));
  game_object_uid_i gouid = GameObjectMakeUID(def->name, NUM_ITEMS++, WorldGetTime());
  *item = (item_t){
    .gouid = gouid,
      .def = def 
  };

  switch(def->category){
    case ITEM_ARMOR:
      ItemArmorPropVals(item);
      break;
    case ITEM_WEAPON:
      ItemWeapPropVals(item);
      break;
    case ITEM_CONSUMABLE:
      ItemConsumePropVals(item);
      break;
    case ITEM_CONTAINER:
      ItemContainerPropVals(item);
      break;
  }

  ApplyItemProps(item, item->def->props, item->def->t_props);
  
  for (int i = 0; i < VAL_ALL; i++){
    if(item->values[i] == NULL)
      continue;

    item->values[i]->val = ValueRebase(item->values[i]);

    item->values[i]->context = item;
  }
  if(def->category == ITEM_CONSUMABLE){
    switch(def->type){
      case CONS_SCROLL:
      case CONS_TOME: 
      case CONS_SKILLUP:
        item->on_acquire = ItemAddAbility;
        break;
      default:
        break;
    }
  }

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
    GameRealloc("InventoryEnsureCap", t->items, new_cap * sizeof(item_t));

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

  game_object_uid_i gouid = GameObjectMakeUID(e->name, id, e->gouid);
  *a = (inventory_t){
    .gouid  = gouid,
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
    slot->space = slot->size = i->values[VAL_STORAGE]->val;
    slot->cap = i->values[VAL_QUANT]->val;
    slot->items = GameCalloc("InventoryAddStorage", slot->cap, sizeof(item_t));
    slot->limit = slot->unburdened = i->values[VAL_WEIGHT]->val;
    i->equipped = true;
    i->owner = e;
    for(int j = 0; j < ITEM_DONE; j++){
      if(slot->limits[j]>0)
        slot->limits[j] = slot->cap;
    }
  }
}
void InventoryItemEvent(EventType ev, void* edata, void* udata){
  inventory_t* inv = udata;
  item_t* item = edata;

  if(!item)
    return;

  switch(ev){
    case EVENT_ITEM_DESTROY:
      TraceLog(LOG_INFO, "Attempt to destroy item in inv:%s", INV_STRINGS[inv->id]);
      TraceLog(LOG_INFO, "DESTROY ITEM: %s", item->def->name);
      /*
         int index = item->index;
         for(int i = index; i < (inv->count -1); i++)
         inv->items[i] = inv->items[i+1];
         */
      inv->count--;

      uint64_t gouid = item->gouid;
      WorldEventOnce(EVENT_INV_REMOVE, inv, gouid );
      GameFree("InventoryItemEvent", item);
      break;
    default:
      break;
  }

}

int InventorySlotAddItem(ent_t* e, ItemSlot id, item_t* i){
  inventory_t* inv = e->inventory[id];

  InventoryEnsureCap(inv);
  item_t* item = &inv->items[inv->count++];
  *item = *i;

  item->owner = e;
  item->equipped = inv->method[i->def->pref];

  item->index = inv->count-1;
  inv->space-= item->values[VAL_STORAGE]->val;
  inv->unburdened-= item->values[VAL_WEIGHT]->val;

  WorldEvent(EVENT_ITEM_ACQUIRE, i, inv->current[item->def->category]);
  inv->current[item->def->category]++;

  WorldEvent(EVENT_ITEM_STORE, item, inv->gouid);
  WorldTargetSubscribe(EVENT_ITEM_DESTROY, InventoryItemEvent, inv, item->gouid);
  return inv->count-1;

}

item_t* InventoryAddItem(ent_t* e, item_t* i){
  int size   = i->values[VAL_STORAGE]->val;
  int weight = i->values[VAL_WEIGHT]->val;
  ItemSlot slot = INV_NONE;
  switch(i->def->category){
    case ITEM_CONTAINER:
      slot = InventoryAddStorage(e, i);
      return e->inventory[i->def->type]->container;
      break;
    case ITEM_CONSUMABLE:
      slot = InventoryGetAvailable(e, i->def->category, size, weight);
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

bool ItemAbilityUse(item_t* i, ent_t* tar){
  i->ability->use_fn(i->owner, i->ability, tar);

  ValueDecrease(i->values[VAL_QUANT], 1);
}

item_t* InventoryGetEquipped(ent_t* e, ItemSlot id){
  for (int i = 0; i < e->inventory[id]->count; i++){
    if(e->inventory[id]->items[i].equipped)
      return &e->inventory[id]->items[i];
  }

  return NULL;
}

char* ItemGenerateName(item_def_t* def){

  ItemProps mat = def->props & MAT_MASK;
  ItemProps qual = def->props & QUAL_MASK;

  material_def_t str_def = ITEM_DEF_STRINGS[def->category][BCTZL(mat)];

  const char* format = "{QUAL} {MATERIAL} {NAME}";

}

item_def_t* DefineArmorByType(ArmorType t, ItemProps p, ArmorProps a){
  item_def_t* item = GameCalloc("DefineArmorByType", 1,sizeof(item_def_t));
  item->category = ITEM_ARMOR;

  item->props = p;
  item->t_props = a;
  item->type = t;
  item->dr = GameCalloc("DefineArmorByType dr",1,sizeof(damage_reduction_t));

  item->ability = ABILITY_NONE;
  armor_def_t temp = ARMOR_TEMPLATES[t];
  item->type_def = &ARMOR_TEMPLATES[t];

  strcpy(item->name, temp.name);

  *item->dr = temp.dr_base;
  //ApplyItemProps(item, p, a);

  //item->weight = temp.weight;

  item->ability = ABILITY_ARMOR_SAVE;
  item->skills[item->num_skills++] = temp.skill;
  return item;

}

item_def_t* DefineWeaponByType(WeaponType t, ItemProps props, WeaponProps w_props){
  item_def_t* item = GameCalloc("DefineWeaponByType", 1,sizeof(item_def_t));
  item->category = ITEM_WEAPON;
  item->type = t;
  item->props = props;
  item->t_props = w_props;
  weapon_def_t temp = WEAPON_TEMPLATES[t];
  item->type_def = &WEAPON_TEMPLATES[t];
  //TODO CHANGE TO VALUE_T
  //item->weight = temp.weight;

  strcpy(item->name, temp.name);
  /*
     item->values[VAL_WORTH] = InitValue(VAL_WORTH,temp.cost);
     item->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,temp.weight);
     item->values[VAL_PENN] = InitValue(VAL_PENN,temp.penn);
     item->values[VAL_DURI] = InitValue(VAL_DURI,temp.durability);
     item->values[VAL_REACH] = InitValue(VAL_REACH,temp.reach_bonus);

     item->values[VAL_ADV_HIT] = InitValue(VAL_ADV_HIT,0);
     item->values[VAL_ADV_DMG] = InitValue(VAL_ADV_DMG,0);

     item->values[VAL_STORAGE] = InitValue(VAL_STORAGE,temp.size);

     ApplyItemProps(item, props, w_props);
     item->skills[item->num_skills++] = temp.skill; 

     for(int i = 0; i < VAL_ALL; i++){
     if(!item->values[i])
     continue;

     item->values[i]->val = ValueRebase(item->values[i]);
     } 

*/
  item->ability = temp.ability; 
  return item;

}

bool ItemDestroy(value_t* v, void* ctx){
  if(ctx==NULL)
    return false;

  item_t* item = ctx;
  WorldEventOnce(EVENT_ITEM_DESTROY, item, item->gouid);
}

item_def_t* DefineConsumableByDef(consume_def_t *def){

  item_def_t* item = GameCalloc("DefineConsumable", 1,sizeof(item_def_t));

  item->type = def->type;

  strcpy(item->name, def->name);

  item->category = ITEM_CONSUMABLE;
  item->pref = STORE_CONTAINER;
  item->allowed[STORE_CONTAINER] = true;

  item->type_def = GameMalloc("consume_def", sizeof(consume_def_t));
  memcpy(item->type_def, def, sizeof(consume_def_t));
  item->ability = def->ability;

  return item;
}

item_def_t* DefineConsumable(ItemInstance data){
  item_def_t* item = GameCalloc("DefineConsumable", 1,sizeof(item_def_t));
  item->id = data.id;
  item->type = data.equip_type; 
  item->category = ITEM_CONSUMABLE;

  item->type_def = &CONSUME_TEMPLATES[data.equip_type];

  consume_def_t temp = CONSUME_TEMPLATES[data.equip_type];
  item->t_props = temp.w_props;
  /*
     item->values[VAL_WORTH] = InitValue(VAL_WORTH,temp.cost);
     item->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,temp.weight);
     item->values[VAL_DURI] = InitValue(VAL_DURI,temp.quanity);
     item->values[VAL_EXP] = InitValue(VAL_EXP,temp.exp);
     item->values[VAL_STORAGE] = InitValue(VAL_STORAGE,temp.size);
     item->values[VAL_DMG] = InitValue(VAL_DMG,temp.amount);
     item->values[VAL_QUANT] = InitValue(VAL_QUANT,temp.quanity);

     item->skills[item->num_skills++] = temp.skill;
     */
  item->ability = temp.ability;

  strcpy(item->name, strcat(temp.name, ABILITY_STRINGS[temp.ability].name));

  return item;
}

item_def_t* DefineItem(ItemInstance data){
  item_def_t* item = GameCalloc("DefineItem", 1, sizeof(item_def_t));
  item->type = data.equip_type;
  item->id = data.id;
  item->category = data.cat;

  item->t_props = data.et_props;
  item->props = data.props;
  switch(data.cat){
    case ITEM_ARMOR:
      item->type_def = &ARMOR_TEMPLATES[data.equip_type];
      item->ability = ABILITY_ARMOR_SAVE;
      break;
    case ITEM_WEAPON:
      item->type_def = &WEAPON_TEMPLATES[data.equip_type];
      item->ability = WEAPON_TEMPLATES[data.equip_type].ability;
      break;
    case ITEM_CONSUMABLE:
      item->type_def = &CONSUME_TEMPLATES[data.equip_type];
      item->ability = CONSUME_TEMPLATES[data.equip_type].ability;
      strcpy(item->name, CONSUME_TEMPLATES[data.equip_type].name);
      strcat(item->name, ABILITY_STRINGS[item->ability].name);
      break;
    case ITEM_CONTAINER:
      item->type_def = &CONTAINER_TEMPLATES[data.equip_type];
      break;
    default:
      return NULL;
      break;
  }

  return item;
}

bool ItemApplyStats(struct ent_s* owner, item_t* item){
  for(int i = 0; i < VAL_ALL; i++){
    if(item->values[i] == NULL)
      continue;
    if(item->values[i]->stat_relates_to == STAT_NONE)
      continue;

    StatType rel = item->values[i]->stat_relates_to;
    owner->stats[rel]->bonus = item->values[i]->val;
    StatExpand(owner->stats[rel],item->values[i]->val,true);
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

  a->owner = owner;
  a->dc = Die(a->side,a->die);

  a->rank++;
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
    a->values[i] = InitValue(i,a->vals[i]);
  }

  if(a->chain_id > ABILITY_NONE){
    a->chain = InitAbility(owner, a->chain_id);
    a->chain_fn = EntUseAbility;
  }

  a->spr = InitSpriteByID(a->image_id, SHEET_SPELLS);
  return a;
}

bool ItemAddAbility(struct ent_s* owner, item_t* item){
  ability_t* a = InitAbility(owner, item->def->ability);
  //  a->cost = def->
  for(int i = 0; i < VAL_WORTH; i++){
    if(item->values[i]==NULL)
      continue;
    if(item->values[i]->val == 0)
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

    AbilityApplyValues(a, item->values[i]);
  }

  a->dr = item->def->dr;
  for(int i = 0; i < item->num_skills; i++)
    a->skills[a->num_skills++] = item->skills[i];

  int pref = owner->skills[item->skills[0]]->val * 10;
  if(a->chain){
    a->chain->skills[a->chain->num_skills++] = item->skills[0];

    if(a->chain->type == AT_DR){
      a->chain->dr = item->def->dr;
      ActionSlotAddAbility(owner,a->chain);
    }
    else
      a->chain->weight+=pref;
  }
  if(a->type != AT_SAVE)
    a->weight+=pref;

  if(item->def->category == ITEM_CONSUMABLE && a->type == AT_KNOWLEDGE){
    if(item->def->type_def){
      consume_def_t* c_def = item->def->type_def;
      a->chain_id = c_def->chain_id;
    }
  } 
  a->item = item;
  item->ability = a;
  item->use_fn = ItemAbilityUse;

  return ActionSlotAddAbility(owner, a);
}

bool ItemSkillup(ent_t* owner, item_t* item, InteractResult result){
  int exp = item->values[VAL_EXP]->val;
  if(exp == 0)
    return false;

  for(int i = 0; i < item->num_skills; i++)
    SkillUse(owner->skills[item->skills[i]],owner->uid,item->def->category,exp,result);

  return true;
}

BehaviorStatus AbilityExecute(ability_t* a, ent_t* e){
  local_ctx_t* tar = NULL;

  switch(a->targeting){
    case DES_NONE:
    case DES_FACING:
      tar = EntGetTarget(e, a->id);
      break;
    case DES_SELF:
      tar = WorldGetContext(DATA_ENTITY, e->gouid);
      break;
    case DES_SEL_TAR:
      int amnt = imax(1,a->values[VAL_QUANT]->val);

      player_input.sel_abi = a;
      ScreenActivateSelector(e->pos, amnt, true, InputSetTarget);
      return BEHAVIOR_RUNNING;
      break;
    case DES_MULTI_TAR:
  }

  if(!tar)
    return BEHAVIOR_FAILURE;
  
  player_input.decisions[ACTION_ATTACK]->params[ACT_PARAM_ABILITY] = ParamMakeObj(DATA_ABILITY, a->id , a);


  param_t p = ParamMake(DATA_LOCAL_CTX, sizeof(local_ctx_t), tar);
  player_input.decisions[ACTION_ATTACK]->params[ACT_PARAM_TAR] = p;

  action_t* act = InitActionByDecision(player_input.decisions[ACTION_ATTACK], ACTION_ATTACK);
  ;

  if(ActionExecute(ACTION_ATTACK, act) == BEHAVIOR_SUCCESS){
    WorldEvent(EVENT_PLAYER_INPUT, act, player->gouid);
    return BEHAVIOR_SUCCESS;
  }

  return BEHAVIOR_FAILURE;

}

InteractResult AbilityLearn(ent_t* owner,  ability_t* a, ent_t* target){
  ability_t* abi = InitAbility(owner, a->chain_id);
  bool result = ActionSlotAddAbility(target, abi);

  if(result)
    WorldEvent(EVENT_LEARN, abi, owner->gouid);
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


void ApplyItemProps(item_t *w, ItemProps props, uint64_t e_props){
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

      if(PROP_MODS[w->def->category][i].propID != wprop)
        continue;

      item_prop_mod_t mod = PROP_MODS[w->def->category][i];

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

item_def_t* BuildArmor(ArmorType type, ItemProps props, ArmorProps w_props){


  item_def_t* item = DefineArmorByType(type, props, w_props);
  item->pref = INV_WORN;
  return item;
}

item_def_t* BuildArmorForMob(ent_t* e, RaceProps props,ArmorType type){
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

  return BuildArmor(type, qual_props | mat_props, a_props);

}

item_def_t* BuildSpecialForMob(ent_t* e, RaceProps props){
  RaceProps s_props = props & RACE_SPECIAL_MASK;

}

item_def_t* GenerateConsume(ConsumeType type, ItemProps props, ConsumeProps c_props, param_t params[LOOT_PARAM_END]){

}
item_def_t* GenerateArmor(ArmorType type, ItemProps props, ArmorProps a_props, param_t params[LOOT_PARAM_END]){

  item_def_t* i = BuildArmor(type, props, a_props);

  armor_def_t* a = i->type_def;
  i->cost = a->cost + (2 * (1+a->type));
  i->weight = a->weight + 10;

  return i;
}

item_def_t* GenerateWeapon(WeaponType type, ItemProps props, WeaponProps w_props, param_t params[LOOT_PARAM_END]){

  item_def_t* i = BuildWeapon(type, props, w_props);

  weapon_def_t *w = i->type_def;
  i->cost = w->cost;
  i->weight = w->weight + 20;

  return i;
}

item_def_t* BuildWeaponForMob(ent_t* e, RaceProps props, WeaponType type){
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

  return BuildWeapon(type, qual_props | mat_props, w_props);
}

item_def_t* BuildAppropriateItem(ent_t* e, ItemCategory cat, SkillType s){
  item_def_t* item;
  RaceProps r_props = GetRaceByFlag(e->props->race).props;

  switch(cat){
    case ITEM_WEAPON:
      WeaponType w = GetWeapTypeBySkill(s);
      item = BuildWeaponForMob(e, r_props, w);
      break;
    case ITEM_ARMOR:
      ArmorType a = GetArmorTypeBySkill(s);
      item = BuildArmorForMob(e, r_props, a);
      break;
    case ITEM_CONSUMABLE:
      item = BuildSpecialForMob(e, r_props);
      break;
  }

  return item;
}

item_def_t* GenerateItem(param_t params[LOOT_PARAM_END]){
  item_def_t* item = GameCalloc("GenerateItem", 1, sizeof(item_def_t));

  ItemCategory cat = *ParamRead(&params[LOOT_PARAM_CATEGORY], int);
  ItemProps props = *ParamRead(&params[LOOT_PARAM_PROPS], uint64_t);
  int type = *ParamRead(&params[LOOT_PARAM_TYPE], int);
  switch(cat){
    case ITEM_WEAPON:
      WeaponProps weap_p = *ParamRead(&params[LOOT_PARAM_WEAP], uint64_t);

      item = GenerateWeapon(type, props, weap_p, params);
      break;
    case ITEM_ARMOR:
      ArmorProps arm_p = *ParamRead(&params[LOOT_PARAM_ARMOR], uint64_t);

      item = GenerateArmor(type, props, arm_p, params);
      break;
    case ITEM_CONSUMABLE:
      ConsumeProps con_p = *ParamRead(&params[LOOT_PARAM_CONS], uint64_t);
      item = GenerateConsume(type, props, con_p, params);
      break;
  }


  return item;
}

bool ItemCurate(item_def_t *def){
  if(!def)
    return false;

  return true;
}

item_def_t* BuildWeapon(WeaponType type, ItemProps props, WeaponProps w_props){


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
  int dist = cell_distance(e->pos, target->pos);

  if(dist <= reach)
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
