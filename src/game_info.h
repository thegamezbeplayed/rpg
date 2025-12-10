#ifndef __GAME_INFO__ 
#define __GAME_INFO__ 

#include "game_types.h"
#include "game_ui.h"

typedef struct{
  int               num_val;
  element_value_t   *values[4];
  const char*       text_format;
}line_item_t;

typedef struct{
  int           lines;
  line_item_t*  ln[12];
}stat_sheet_t;

line_item_t* InfoInitLineItem(element_value_t **val, int num_val, char* format);
const char* PrintLine(line_item_t* ln);
char *TextFormatLineItem(const line_item_t *item);
void PrintMobDetail(ent_t* e);
int EntGetStatPretty(element_value_t **fill, stat_t* stat);
int EntGetNamePretty(element_value_t **fill, ent_t* e );
element_value_t* StatGetPretty(element_value_t* self, void* context);
element_value_t* SkillGetPretty(element_value_t* self, void* context);
void PrintSyncLine(line_item_t* ln, FetchRate poll);
#endif
