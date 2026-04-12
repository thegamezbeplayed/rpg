#include "game_unique.h"
#include "game_process.h"
#include "game_strings.h"

material_table_t* InitMaterialTable(int cap){
  material_table_t* t = GameCalloc("InitMaterialTable", 1, sizeof(material_table_t));

  *t = (material_table_t){
    .cap      = cap,
      .count    = 0,
      .entries  = GameCalloc("InitMaterialTable", cap, sizeof(material_spec_d))
  };
  HashInit(&t->map, next_pow2_int(cap*2));

  return t;
}

material_spec_d* MaterialsGetEntry(material_table_t* t, game_object_uid_i other){
  return HashGet(&t->map, other);

}

void MaterialsEnsureCap(material_table_t* t){
  if (t->count < t->cap)
    return;

  size_t new_cap = t->cap+32;

  material_spec_d* new_entries =
    GameRealloc("MaterialsEnsureCap", t->entries, new_cap * sizeof(material_spec_d));

  if (!new_entries) {
    // Handle failure explicitly
    TraceLog(LOG_WARNING,"==== MATERIALS TABLE ERROR ===\n REALLOC FAILED");
  }

  t->entries = new_entries;
  t->cap = new_cap;

  if(t->map.count * 4 >= t->map.cap *3)
    HashExpand(&t->map);

}

MaterialSpec RegisterMaterial(MaterialType type, DataType dtype, int id, char name[32]){

  material_table_t* table = LevelMaterials();
  uint64_t thash = GameObjectMakeUID(name, type, dtype);
  material_spec_d* exists = MaterialsGetEntry(table, thash);
  if(exists)
    return exists->spec;

  material_spec_d* mat = &table->entries[table->count++];

  mat->spec = thash;
  mat->type     = type;
  mat->dtype    = dtype;
  mat->data_id  = id;
  if(dtype == DATA_MATERIAL)
   mat->id = id;

  strcpy(mat->mat, MAT_STRINGS[type]);
  strcpy(mat->root, name);

  HashPut(&table->map, thash, mat);
  WorldEvent(EVENT_REGISTER_MATERIAL, mat, thash);
  return mat->spec;
}

MaterialSpec RegisterMaterialByID(MaterialID id, DataType type){
  material_data_t data = MATERIAL_DATA[id];

  MaterialType mat = data.type;
  
  return RegisterMaterial(mat, DATA_MATERIAL, id, data.name);
}

void ResourceDestroyOwner(game_object_uid_i gouid, resource_t* r, int amnt){
  local_ctx_t* ctx = WorldGetContext(DATA_ENV, gouid);

  if(!ctx || ctx->other.type_id != DATA_ENV)
    return;

  EnvSetStatus(ctx->other.data, ENV_STATUS_DEAD);
}

material_extraction_t* ResourceExtractComp(resource_t* r, material_properties_t* props, int amount, skill_t* skill){
    material_extraction_t* ext = GameCalloc("ResourceExtract", 1, sizeof(material_extraction_t));

    for(int i = 0; i < props->num_composition; i++){
      ext->output[ext->num_mats++] = props->composition[i];
    }

    bool running = false;
    choice_pool_t* distr = StartChoice(&props->distribute, props->num_composition, ChooseByWeight, &running);
    if(!running){
      for (int i = 0; i < props->num_composition; i++){
      material_composition_t mcomp = props->composition[i];

      if(mcomp.method !=  skill->id)
        continue;

      AddChoice(distr, i, mcomp.ratio, &mcomp, ChoiceReduceScore);
      }
    }

    distr->flags = skill->id;

    InteractResult res = IR_NONE;

    int take = amount;
    while(r->amount > 0 && amount > 0 && distr->count > 0){
      int cr = r->cr;
      choice_t* c = distr->choose(distr);
      if(!c || !c->context){
        res = IR_FAIL;
        amount-=1;
        take = 2;
      }
      else{
        material_composition_t comp = props->composition[c->id];
        res = SkillCheckVal(skill, comp.cr);
        cr = comp.cr;
        amount -= comp.mul;
        if(res > IR_ALMOST){
          ext->output[c->id].amnt += 1;
          take = 1 + comp.mul;
        }
      }
     
      amount -= 1; 
      r->amount-=take;
      if(r->on_change)
        r->on_change(r->owner, r, take);
      if(r->amount < 1 && r->on_empty)
        r->on_empty(r->owner, r, take);


      SkillUse(skill, skill->owner->gouid, r->owner, cr, res);
    }

    return ext;

}

material_extraction_t* ResourceExtract(resource_t* r, int amount, skill_t* skill){

  int take = imin(amount, r->amount);

  material_spec_d* mspec = MaterialsGetEntry(LevelMaterials(), r->spec);

  material_properties_t *props = NULL;
  if(mspec)
    props = &MATERIAL_COMP[mspec->id];

  if(!mspec || !props){ 
    material_extraction_t* ext = GameCalloc("ResourceExtract", 1, sizeof(material_extraction_t));

    r->amount -= take;

    if(r->on_change)
      r->on_change(r->owner, r, take);

    if(r->amount < 1 && r->on_empty)
      r->on_empty(r->owner, r, take);


    material_composition_t out = {
      .id = mspec->id,
      .cr = r->cr,
      .method = skill->id,
      .amnt = take
    };

    InteractResult res = IR_SUCCESS;
    SkillUse(skill, skill->owner->gouid, r->owner, r->cr, res);
    ext->output[ext->num_mats++] = out;
    return ext;
  }

  return ResourceExtractComp(r, props, amount, skill);
}

resource_t* InitResourceByMat(Resource resource, MaterialID id, int amnt){

  define_resource_t* drt = GetResourceDef(resource);

  resource_t* res = GameCalloc("InitResourceByMat", 1, sizeof(resource_t));

  res->amount = amnt * drt->quantity;
  res->spec = RegisterMaterialByID(id, DATA_ENV);
  res->type = drt->type;
  res->cr = drt->base_cr;

  res->on_empty = ResourceDestroyOwner;

  material_properties_t mpt = MATERIAL_COMP[id];
  for( int i = 0; i < mpt.num_composition; i++)
    RegisterMaterialByID(mpt.composition[i].id, DATA_ENV);

  return res;
}

void ItemMaterialGetName(MaterialSpec spec, ItemCategory item, char name[MAX_NAME_LEN], char buff[MAX_NAME_LEN]){
  material_spec_d* mspec = MaterialsGetEntry(LevelMaterials(), spec);

  switch(mspec->type){
    case MAT_BONE:
    case MAT_FUR:
    case MAT_HIDE:
    case MAT_LEATHER:
      sprintf(buff, "%s %s %s", mspec->root, mspec->mat, name);
      break;
    default:
      sprintf(buff, "%s %s", mspec->root, name);
      break;
  }

}
