#include "game_types.h"
#include "game_helpers.h"
#include "game_process.h"
#include "game_strings.h"

#include <stdio.h>

static int NUM_ITEMS = 0;

void ItemApplyMaterial(item_t* item){
  MaterialSpec spec = 0;
  switch(item->def->category){
    case ITEM_TOOL:
      tool_def_t* tool = item->def->type_def;
      spec = tool->material;
      break;
    case ITEM_WEAPON:
      weapon_def_t* weapon = item->def->type_def;
      spec = weapon->mat;
      break;
    default:
      return;
      break;
  }
  
  if(spec == 0)
    return;

  material_spec_d *mspec  = MaterialsGetEntry(LevelMaterials(), spec);

  if(!mspec)
    return;


  item_prop_mod_t mod = MAT_MODS[item->def->category][mspec->id];

  if(mod.propID != mspec->id || mod.num_aff == 0)
    return;

  for (int i = 0; i < mod.num_aff; i++){
    ValueAddBaseMod(item->values[mod.val_change[i].modifies], mod.val_change[i]);

  }

  if(item->def->props == 0)
    return;
}

void ItemArmorPropVals(item_t* i){
  armor_def_t *tmp = i->def->type_def;
  for(int j = 0; j < VAL_ALL; j++)
    i->values[j] = InitValue(j, tmp->vals[j]);

  if(tmp->skill > SKILL_NONE)
  i->skills[i->num_skills++] = tmp->skill;
  switch(tmp->type){
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
    case CONS_POT:
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
  i->values[VAL_EXP] = InitValue(VAL_EXP,def->exp);
  i->values[VAL_STORAGE] = InitValue(VAL_STORAGE,def->size);

  for(int j = 0; j < VAL_ALL; j++){
    if(def->vals[j] == 0)
      continue;
    i->values[j] = InitValue(j, def->vals[j]);
  }

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
    case CONS_POT:
      i->sprite = InitSpriteByID(ICON_POT_FULL, SHEET_ICON);
      switch(def->chain_id){
        case STAT_STAMINA:
          i->sprite->slice->color = ColorFromNormalized((Vector4){ 0.0f, 0.5f, 0.5f, 1.0f });
          break;
        case STAT_ENERGY:
          i->sprite->slice->color = SKYBLUE;
          break;
        default:
          i->sprite->slice->color = MAROON;
          break;
      }
      break;
  };
}

void ItemToolPropVals(item_t* item){
  tool_def_t* tmp = item->def->type_def;

  for(int i = 0; i < VAL_ALL; i++)
    item->values[i] = InitValue(i, tmp->vals[i]);

  item->sprite = InitSpriteByID(tmp->icon, SHEET_ICON);
  if(tmp->col.a > 0)
  item->sprite->slice->color = tmp->col;

  for (int i = 0; i < 3; i++){
    if(tmp->skills[i] > SKILL_NONE)
      item->skills[item->num_skills++] = tmp->skills[i];
  }

}

void ItemWeapPropVals(item_t* item){
  weapon_def_t *tmp = item->def->type_def;

  for(int i = 0; i < VAL_ALL; i++)
    item->values[i] = InitValue(i, tmp->vals[i]);

  item->skills[item->num_skills++] = tmp->skill;

  switch(tmp->type){
    case WEAP_MACE:
      item->sprite = InitSpriteByID(ICON_MACE_COM, SHEET_ICON);
      item->sprite->slice->color = DARKGRAY;
      break;
    case WEAP_SWORD:
      item->sprite = InitSpriteByID(ICON_SWORD, SHEET_ICON);
      item->sprite->slice->color = RAYWHITE;
      break;
    case WEAP_AXE:
      item->sprite = InitSpriteByID(ICON_HATCHET, SHEET_ICON);
      item->sprite->slice->color = DARKBROWN;
      break;
    case WEAP_DAGGER:
      item->sprite = InitSpriteByID(ICON_DAGGER, SHEET_ICON);
      item->sprite->slice->color = RAYWHITE;
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

void ItemMatPropVals(item_t* item){
  material_def_t* tmp = item->def->type_def;

  for(int i = 0; i < VAL_ALL; i++)
    item->values[i] = InitValue(i, tmp->vals[i]);

  item->sprite = InitSpriteByID(tmp->icon, SHEET_ICON);
  if(tmp->col.a > 0)
    item->sprite->slice->color = tmp->col;

  item->stack = 64;
}

void ItemContainerPropVals(item_t* i){
  container_def_t* temp = i->def->type_def;
  i->values[VAL_WORTH] = InitValue(VAL_WORTH,temp->cost);
  i->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,temp->weight);
  i->values[VAL_QUANT] = InitValue(VAL_QUANT,temp->slots);
  i->values[VAL_STORAGE] = InitValue(VAL_STORAGE,temp->size);
}

bool InitItemContext(item_def_t* def, Cell pos){
  item_t* item = InitItem(def);

  TraceLog(LOG_INFO, "=== DROP ITEM %s at %i/%i", def->name, pos.x, pos.y);
  if(RegisterItemContext(item, pos))
    item->sprite->is_visible = true;

  item->sprite->slice->scale = 0.25f;
  item->sprite->pos = CellToVector2(pos,CELL_WIDTH); 

  return item->sprite->is_visible;
}

item_t* InitItem(item_def_t* def){
  item_t* item = GameMalloc("InitItem", sizeof(item_t));
  game_object_uid_i gouid = GameObjectMakeUID(def->name, NUM_ITEMS++, WorldGetTime());
  def->hash = hash_string_64(def->name);

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
    case ITEM_TOOL:
      ItemToolPropVals(item);
      break;
    case ITEM_MATERIAL:
      ItemMatPropVals(item);
      break;
  }
  ItemApplyMaterial(item);
  //ApplyItemProps(item, item->def->props, item->def->t_props);
  
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
    if(item_funcs[def->category].on_acquire)
      item->on_acquire = item_funcs[def->category].on_acquire;
  }

  if(item->values[VAL_DURI] && item->values[VAL_DURI]->val > 0)
    item->on_use = ItemTakeDuribility;

  return item;
}

item_t* InventoryGetEntry(ent_t* e, uint64_t uid){
  for(int i = 0; i < INV_DONE; i++){
    inventory_t* t = e->inventory[i];
    item_t* exists = HashGet(&t->map, uid);
    if(exists)
      return exists;
  }

  return NULL;
}

item_t* InventoryGetStackable(ent_t* e, hash_key_t hash){
for(int i = 0; i < INV_DONE; i++){
    inventory_t* t = e->inventory[i];
    item_t* exists = HashGet(&t->hash, hash);
    if(exists){
      if(!exists->values[VAL_QUANT])
        continue;
      if(exists->values[VAL_QUANT]->val < exists->stack)
        return exists;
    }
  }

  return NULL;
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
  HashInit(&a->map, next_pow2_int(a->cap*2));
  HashInit(&a->hash, next_pow2_int(a->cap*2));
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

  HashPut(&inv->map, item->fuid, item);
  HashPut(&inv->hash, item->def->hash, item);
  WorldEvent(EVENT_ITEM_STORE, item, inv->gouid);
  WorldEvent(EVENT_ITEM_STORE, item, e->gouid);
  WorldTargetSubscribe(EVENT_ITEM_DESTROY, InventoryItemEvent, inv, item->gouid);
  return inv->count-1;

}

bool ItemTakeDuribility(ent_t* e, item_t* i, InteractResult res){

  res = IR_NONE;
  if(!i->values[VAL_DURI])
    return false;

  int dmg = 1;
  if(i->values[VAL_DRAIN])
    dmg = i->values[VAL_DRAIN]->val;

  ValueDecrease(i->values[VAL_DURI], dmg);

  res = IR_SUCCESS;
  return true;
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

bool ItemAbilityUse(item_t* i, local_ctx_t* tar){
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

  material_d str_def = ITEM_DEF_STRINGS[def->category][BCTZL(mat)];

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
item_def_t* DefineArmor(armor_def_t* adef){
  item_def_t* item = GameCalloc("DefineWeapon", 1,sizeof(item_def_t));

  item->category = ITEM_ARMOR;
  item->type = adef->type;
  item->props = adef->i_props;
  item->t_props = adef->a_props;
  item->type_def = adef;
  item->skills[item->num_skills++] = adef->skill;

  item->ability = ABILITY_ARMOR_SAVE;
  item->dr = GameCalloc("DefineArmor",1,sizeof(damage_reduction_t));
  *item->dr = adef->dr_base;
  strcpy(item->name, adef->name);
  material_spec_d *mspec = MaterialsGetEntry(LevelMaterials(), adef->mat);

  if(mspec->id > 0 && mspec->id < MAT_ALL){
    material_data_t data = MATERIAL_DATA[mspec->id];

    item->cost = data.vals[VAL_WORTH];
    item->weight = data.vals[VAL_SCORE];
  }

  item->cost += adef->vals[VAL_WORTH];
  item->weight += adef->vals[VAL_SCORE];

  return item;

}

item_def_t* DefineWeapon(weapon_def_t* wdef){
  item_def_t* item = GameCalloc("DefineWeapon", 1,sizeof(item_def_t));

  item->category = ITEM_WEAPON;
  item->type = wdef->type;
  item->props = wdef->i_props;
  item->t_props = wdef->w_props;
  item->type_def = wdef;
  item->ability = wdef->ability;
  item->skills[item->num_skills++] = wdef->skill;
  strcpy(item->name, wdef->name);
  material_spec_d *mspec = MaterialsGetEntry(LevelMaterials(), wdef->mat);

  if(mspec->id > 0 && mspec->id < MAT_ALL){
    material_data_t data = MATERIAL_DATA[mspec->id];

    item->cost = data.vals[VAL_WORTH];
    item->weight = data.vals[VAL_SCORE];
  }

  item->cost += wdef->vals[VAL_WORTH];
  item->weight += wdef->vals[VAL_SCORE];

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

  strcpy(item->name, temp.name);
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

  item->flags |= LF_CONS;
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

ability_sim_t* AbilitySimHeal(ent_t* owner,  ability_t* a){
  ability_sim_t* res = GameCalloc("AbilitySimDmg", 1,sizeof(ability_sim_t));
  res->id = a->id;
  res->type = a->type;
  res->d_type = a->school;
  res->d_bonus = AbilityAddPB(owner, a, STAT_DAMAGE);
  res->dmg_die = a->dc->num_die;
  res->dmg_sides = a->dc->sides;
  res->hit_calc = 20;// a->hit->roll(a->hit, res->hit_res);
  res->hit_res[0] = 20;
  res->final_dmg = res->dmg_calc + res->d_bonus;
  return res;
}

ability_sim_t* AbilitySimDmg(ent_t* owner,  ability_t* a){
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

ability_t* InitAbilitySave(ent_t* owner, AbilityID id, define_natural_armor_t* def){
  ability_t* a = InitAbility(owner, id);

  a->hit = Die(def->armor_class, 1);

  for(int i = 0; i < 3; i++){
    if(a->skills[i] != SKILL_NONE)
      continue;

    a->skills[i] = def->skill;
  }

  return a;
}

ability_t* InitAbilityDR(ent_t* owner, AbilityID id, define_natural_armor_t* def){
  ability_t* a = InitAbility(owner, id);
  a->dr = &def->dr;

  for(int i = 0; i < 3; i++){
    if(a->skills[i] != SKILL_NONE)
      continue;

    a->skills[i] = def->skill;
  }

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

bool AbilitySkillup(ent_t* owner, ability_t* a, local_ctx_t* ctx, InteractResult result){
  
  int cr = 0;
  switch(a->type){
    case AT_KNOWLEDGE:
      define_skill_gain_t dsg = GetAbilitySkillGain(a->type, a->id);

      cr = dsg.use_gain;
      if(result > IR_ALMOST)
        cr += dsg.succ_gain;

      cr*=a->rank;
      break;
    case AT_SKILL:
      cr = a->last_use_cr;
      break;
    default:
      if(ctx->other.type_id != DATA_ENTITY)
        return false;

      ent_t* target = ParamRead(&ctx->other, ent_t);


      aggro_t* e = LocalGetAggro(owner->local,target->gouid);

      if(e==NULL)
        return false;

      cr = e->challenge;

      switch(a->type){
        case AT_SAVE:
        case AT_DR:
          cr = e->offensive_rating;
          break;
        case AT_DMG:
          cr = e->defensive_rating;
          break;
      }
      break;
  }

  for(int i = 0; i < 3; i++){
    if(a->skills[i] == SKILL_NONE)
      break;
    SkillUse(owner->skills[a->skills[i]],owner->uid,ctx->gouid,cr,result);
  }
  return true;
}

InteractResult AbilityGrantExp(ent_t* owner,  ability_t* a, local_ctx_t* ctx){
  if(ctx->other.type_id != DATA_ENTITY)
    return IR_NONE;

  ent_t* target = ParamRead(&ctx->other, ent_t);

  int res[a->dc->num_die];
  int exp = a->dc->roll(a->dc, res);
  skill_t* s = target->skills[a->chain_id];
  if(SkillIncrease(s, exp))
    return IR_SUCCESS;

  return IR_FAIL;
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

  switch(a->type){
    case AT_DMG:
    a->sim_fn = AbilitySimDmg;
    break;
    case AT_HEAL:
    a->sim_fn = AbilitySimHeal;
    break;
  }

  switch(a->action){
    case ACTION_MAGIC:
      a->skills[1] = SKILL_ARCANA;
      break;
  }

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

void AbilityApplyValues(ability_t* self, value_t* v){
  if(!self || !self->values[v->cat])
    return;

  value_t* sv = self->values[v->cat];
  sv->base = v->base; 
  sv->val = v->base;

  if(sv->on_change){ 
    sv->context = self;
    sv->on_change(sv,self);
  }
}

bool ItemAddUse(ent_t* owner, item_t* item){
  if(item->def->category != ITEM_TOOL)
    return false;

  tool_def_t* def = item->def->type_def;

  if(def->use == 0)
    return false;

  item->on_use = NULL;

  ability_t* a =  GameCalloc("ItemAddUse", 1,sizeof(ability_t));
  *a = AbilityLookup(def->use);

  a->owner = owner;
  for(int i = 0; i < VAL_WORTH; i++){
    a->values[i] = InitValue(i, a->vals[i]);
    a->values[i]->val = ValueRebase(a->values[i]);
  }
  a->stats[STAT_REACH] = STAT(STAT_REACH, a->values[VAL_REACH]->val);
  a->stats[STAT_DAMAGE] = InitStatOnMax(STAT_DAMAGE,a->values[VAL_DMG_BONUS]->val,a->mod);

  a->hit = Die(a->values[VAL_HIT]->val, 1);

  int side = a->values[VAL_DMG]->val;
  int die = a->values[VAL_DMG_DIE]->val;
  a->dc = Die(side,die);

  int j = 0;
  for(int i = 0; i < item->num_skills; i){
    if(a->skills[j] != SKILL_NONE){
      j++;
      continue;
    }
    a->skills[j++] = item->skills[i++];
  }

  WorldEvent(EVENT_LEARN, a, owner->gouid);
  for (int i = 0; i < STAT_DONE; i++){
    if(!a->stats[i])
      continue;
    a->stats[i]->owner = owner;
  }


  a->item = item;
  item->ability = a;
  item->use_fn = ItemAbilityUse;

  a->on_use_cb = AbilitySkillup;
  a->sim_fn = AbilitySimDmg;

  return ActionSlotAddAbility(owner, a);

}

bool ItemAddAbility(ent_t* owner, item_t* item){
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
      case VAL_DMG:
      case VAL_DMG_DIE:
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
  int j = 0;
  for(int i = 0; i < item->num_skills; i){
    if(a->skills[j] != SKILL_NONE){
      j++;
      continue;
    }
    a->skills[j++] = item->skills[i++];
  }

  int pref = owner->skills[item->skills[0]]->val * 10;
  if(a->chain){
    for(int i = 0; i < 3; i++){
      if(a->skills[j] != SKILL_NONE)        
        continue;

      a->chain->skills[i] = item->skills[0];
    }
  
    if(a->chain->type == AT_DR){
      a->chain->dr = item->def->dr;
      ActionSlotAddAbility(owner,a->chain);
    }
    else
      a->chain->weight+=pref;
  }
  switch(a->type){
    case AT_DMG:
    case AT_HEAL:
      if(a->action != ACTION_ITEM){
        WorldEvent(EVENT_LEARN, a, owner->gouid);

        a->weight+=pref;
      }
      break;
    case AT_KNOWLEDGE:
      if(item->def->category == ITEM_CONSUMABLE){
        if(item->def->type_def){
          consume_def_t* c_def = item->def->type_def;
          a->chain_id = c_def->chain_id;
        }
      }
      break;
    default:
      break;
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

local_ctx_t* AbilityTargetFilter(ent_t* e, CtxProps props, GameObjectParam param, uint64_t filter){

  local_ctx_t* filtered[32] = {0};

  CtxProps targeting = props & CTX_TAR_MASK;
  param_t f = ParamMake(DATA_UINT64, sizeof(CtxProps), &targeting);
  GameObjectParam p =  PARAM_PROPS;
  int count = LocalContextFilter(e->local, 32, filtered, f, p, ParamCompareAnd, 0);

  for(int i = 0; i < count; i++){
    uint64_t *params = ParamRead(&filtered[i]->params[param], uint64_t);
    if(((*params) & filter) > 0)
      return filtered[i];

  }

  return NULL;
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
    case DES_MULTI_TAR:
    case DES_SEL_TAR:
      int amnt = imax(1,a->values[VAL_QUANT]->val);

      player_input.sel_abi = a;
      ScreenActivateSelector(e->pos, amnt, true, InputSetTarget);
      return BEHAVIOR_RUNNING;
      break;
    case DES_REQ:
      tar = AbilityTargetFilter(e, a->req, PARAM_RESOURCE, a->params[PARAM_RESOURCE]);
      break;
  }

  if(!tar)
    return BEHAVIOR_FAILURE;
 
  player_input.decisions[ACTION_ATTACK]->params[ACT_PARAM_ABILITY] = ParamMakeObj(DATA_ABILITY, a->id , a);


  param_t p = ParamMake(DATA_LOCAL_CTX, sizeof(local_ctx_t), tar);
  player_input.decisions[ACTION_ATTACK]->params[ACT_PARAM_TAR] = p;

  action_t* act = InitActionByDecision(player_input.decisions[ACTION_ATTACK], ACTION_ATTACK);

  if(ActionExecute(ACTION_ATTACK, act) == BEHAVIOR_SUCCESS){
    WorldEvent(EVENT_PLAYER_INPUT, act, player->gouid);
    return BEHAVIOR_SUCCESS;
  }

  return BEHAVIOR_FAILURE;

}

void EntAddResource(ent_t* e, local_ctx_t* tar, resource_t* res){

  if(res->amount == 0)
    return;
  int amnt = res->amount;
  Resource r = res->type;
  item_def_t* def = DefineMaterialByResource(r, res->spec, amnt);
  item_t* item = InitItem(def);

  ValueSet(item->values[VAL_QUANT], amnt);
  
  EntAddItem(e, item, false);
}

InteractResult ExtractResource(ent_t* owner,  ability_t* a, local_ctx_t* target){
  uint64_t matches = target->resource & a->params[PARAM_RESOURCE];
  
  if(matches == 0)
    return IR_NONE;

  a->last_use_cr = 0;

  int results[a->dc->num_die];

  a->stats[STAT_DAMAGE]->start(a->stats[STAT_DAMAGE]);

  StatMaxOut(a->stats[STAT_DAMAGE]);

  skill_t* skill = owner->skills[a->skills[0]];
  ability_sim_t* sim = a->sim_fn(owner, a);
  int amnt = sim->final_dmg;

  while(matches && amnt > 0){
    Resource r = matches & -matches;
    matches &= matches -1;

    resource_t* extract = NULL;

    env_t* env = NULL;
    switch(target->other.type_id){
      case DATA_ENV:
        env = ParamRead(&target->other, env_t);
        extract = env->resources[__builtin_ctzll(r)];
        break;
    }

    material_extraction_t* ext = ResourceExtract(extract, amnt, skill);
    int take = 0;
    for(int i = 0; i < ext->num_mats; i++)
      take+= ext->output[i].amnt;

    amnt -= take;

    a->last_use_cr += extract->cr;
    if(take == 0)
      return IR_ALMOST;
    for(int i = 0; i < ext->num_mats; i++){
      material_composition_t comp = ext->output[i];

      resource_t* add = InitResourceByMat(r, comp.id, comp.amnt);
      EntAddResource(owner, target, add);
  
    }
    TraceLog(LOG_INFO, "==== EXTRACT RESOURCE ===\n remaining resource %i", extract->amount);
  }


  return IR_SUCCESS;
}

InteractResult InteractionExtract(ent_t* owner,  ability_t* a, local_ctx_t* target){
  CtxProp has = a->req & CTX_HAS_MASK;

  switch(has){
    case CTX_HAS_RESOURCE:
      return ExtractResource(owner, a, target);
      break;
    default:
      return IR_NONE;
      break;
  }
}

InteractResult AbilityProcess(ent_t* e,  ability_t* a, local_ctx_t* target){

}

InteractResult AbilityInteract(ent_t* e,  ability_t* a, local_ctx_t* target){
  int cost = a->values[VAL_DRAIN]->val;
  if(a->resource>STAT_NONE && cost > 0)
    if(!StatChangeValue(e,e->stats[a->resource],-1*cost))
      return IR_NONE;

  CtxProp method = a->req & CTX_METHOD_MASK;
  switch(method){
    case CTX_METHOD_EXTRACT:
      return InteractionExtract(e, a, target);
      break;
    default:
      return IR_NONE;
  }


}

InteractResult AbilityLearn(ent_t* owner,  ability_t* a, local_ctx_t* ctx){
  if(ctx->other.type_id != DATA_ENTITY)
    return IR_FAIL;

  ent_t* target = ParamRead(&ctx->other, ent_t);
  ability_t* abi = InitAbility(owner, a->chain_id);
  bool result = ActionSlotAddAbility(target, abi);

  InteractResult ires = result?IR_SUCCESS: IR_FAIL;
  if(a->on_use_cb)
    a->on_use_cb(owner, a, ctx, ires);

  if(result){
    if(a->on_success_cb)
      a->on_success_cb(owner, a, ctx, ires);

    WorldEvent(EVENT_LEARN, abi, owner->gouid);
  }

  return ires;
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
//  i->cost = a->cost + (2 * (1+a->type));
  //i->weight = a->weight + 10;

  return i;
}

item_def_t* GenerateWeapon(WeaponType type, ItemProps props, WeaponProps w_props, param_t params[LOOT_PARAM_END]){

  item_def_t* i = BuildWeapon(type, props, w_props);

  weapon_def_t *w = i->type_def;
  i->cost = w->vals[VAL_WORTH];

  i->weight = w->vals[VAL_SCORE];

  return i;
}

item_def_t* DefineTool(tool_def_t* def){
  item_def_t* item = GameCalloc("DefineTool", 1, sizeof(item_def_t));

  item->category = ITEM_TOOL;
  item->type = def->type;
  item->t_props = def->t_props;
  item->props = def->i_props;

  item->ability = def->ability;
  item->type_def = def;
  item->weight = def->vals[VAL_SCORE];
  item->cost = def->vals[VAL_WORTH];
  strcpy(item->name, def->name);

  item->flags = LF_TOOL;
  return item;
}

item_def_t* DefineMaterialByResource(Resource res, MaterialSpec spec, int amnt){

  item_type_d* itype = ItemPoolGetEntry(Level.item_defs, spec);
  if(!itype)
    return NULL;

  return DefineMaterial(&itype->data.mat);
}

item_def_t* DefineMaterial(material_def_t *def){
  item_def_t* item = GameCalloc("DefineeMaterial", 1, sizeof(item_def_t));

  //item->id = data.id;
  item->type = def->type;
  item->category = ITEM_MATERIAL;

  item->type_def = def;

  item->t_props = def->m_props;

  strcpy(item->name, def->name);

  return item;
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
      item->flags |= LF_WEAP;
      break;
    case ITEM_ARMOR:
      ArmorProps arm_p = *ParamRead(&params[LOOT_PARAM_ARMOR], uint64_t);
      item = GenerateArmor(type, props, arm_p, params);
      item->flags |= LF_ARMOR;
      break;
    case ITEM_CONSUMABLE:
      ConsumeProps con_p = *ParamRead(&params[LOOT_PARAM_CONS], uint64_t);
      item = GenerateConsume(type, props, con_p, params);
      item->flags |= LF_CONS;
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

ActionStatus AbilityCanTarget(ability_t* a, local_ctx_t* target){
  if(!a)
    return ACT_STATUS_BAD_ATTACK;

  if(!target)
    return ACT_STATUS_BAD_DATA;

  if(a->type == SLOT_SAVE)
    return ACT_STATUS_MISQUEUE;

  ent_t* e = a->stats[STAT_REACH]->owner;

  if(a->resource != STATE_NONE){
    int res = e->stats[a->resource]->current;
    if(a->cost > res)
      return ACT_STATUS_RESOURCE;
  }

  if(!HasLOS(e->map, e->pos, target->pos))
    return ACT_STATUS_BLOCK;

  int reach = a->stats[STAT_REACH]->current;
  int dist = cell_distance(e->pos, target->pos);

  if(dist <= reach)
    return ACT_STATUS_NONE;

  return ACT_STATUS_INVALID;
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
