#ifndef __GAME_UNIQUE__
#define __GAME_UNIQUE__
#include <stdint.h>
#include "game_enum.h"
#include "game_tools.h"

typedef uint64_t MaterialSpec;

typedef struct{
  MaterialSpec    spec;
  MaterialType    type;
  MaterialID      id;
  DataType        dtype;
  int             data_id;
  char            root[32];
  char            mat[32];
}material_spec_d;

typedef struct{
  int             cap, count;
  material_spec_d *entries;
  hash_map_t      map;
}material_table_t;

material_table_t* InitMaterialTable(int cap);
material_spec_d* MaterialsGetEntry(material_table_t*, game_object_uid_i);


MaterialSpec RegisterMaterial(MaterialType, DataType, int, char name[32]);
MaterialSpec RegisterMaterialByID(MaterialID, DataType);

void ItemMaterialGetName(MaterialSpec spec, ItemCategory item, char name[32], char buff[32]);


#endif
