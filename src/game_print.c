#include "game_info.h"
#include "game_common.h"
#include <stdio.h>

void PrintMobDetail(ent_t* e){
  faction_t* f = GetFactionByID(e->team);
  char* team = "";
  if(f)
    team = strdup(f->name);
  TraceLog(LOG_INFO,"%s lvl %i\n %s\n",e->name, e->skills[SKILL_LVL]->val,
      team);

  stat_t* health = e->stats[STAT_HEALTH];
  TraceLog(LOG_INFO,"%s: [%i/%i] \n",STAT_STRING[STAT_HEALTH].name, (int)health->current,(int)health->max);

   TraceLog(LOG_INFO,"<=====ATTRIBUTES=====>\n");
  for(int i = 0; i < ATTR_DONE; i++){
    if(e->attribs[i]==NULL)
      continue;

    int val = e->attribs[i]->val;
    const char* name = attributes[i].name;
    TraceLog(LOG_INFO,"%i %s",val,name);
  }
}

line_item_t* InfoInitLineItem(element_value_t **val, int num_val, char* format){
  line_item_t* ln = calloc(1,sizeof(line_item_t));

  for (int i = 0; i < num_val; i++)
    ln->values[ln->num_val++]=val[i];

  ln->text_format = format;

}
void PrintSyncLine(line_item_t* ln, FetchRate poll){
    for(int i = 0; i < ln->num_val; i++){
      if(!ln->values[i]->get_val)
        continue;

      if(ln->values[i]->rate <= poll)
        ln->values[i]->get_val(ln->values[i],ln->values[i]->context);
    }
}

const char* PrintLine(line_item_t* ln){
  for(int i = 0; i < ln->num_val; i++){
    element_value_t* ev = ln->values[i];
    /*switch(ev->type){
   case VAL_INT:
      *self->i = (int)stat->current;
      break;
    case VAL_FLOAT:
      *self->f = stat->ratio(stat);
      break;
    case VAL_CHAR:
      strcpy(self->c,TextFormat("%i / %i",(int)stat->current,(int)stat->max));
      break;
    }
*/
  }
}

char *TextFormatLineItem(const line_item_t *item) {
    static char buffer[2048];
    char temp[256];

    buffer[0] = 0;
    const char *fmt = item->text_format;

    int arg_index = 0;

    for (int i = 0; fmt[i] != 0; i++) {
        // Check for '%' format token
        if (fmt[i] == '%' && fmt[i+1] != 0 && arg_index < item->num_val) {
            element_value_t *val = item->values[arg_index];

            switch (fmt[i+1]) {
                case 's':
                    if (val->type == VAL_CHAR)
                        strcat(buffer, val->c);
                    else
                        strcat(buffer, "<BAD%S>");
                    break;

                case 'i':
                    if (val->type == VAL_INT) {
                        sprintf(temp, "%i", *val->i);
                        strcat(buffer, temp);
                    } else {
                        strcat(buffer, "<BAD%I>");
                    }
                    break;

                case 'f':
                    if (val->type == VAL_FLOAT) {
                        sprintf(temp, "%.2f", *val->f);
                        strcat(buffer, temp);
                    } else {
                        strcat(buffer, "<BAD%F>");
                    }
                    break;

                default:
                    // Copy unknown sequence literally
                    strncat(buffer, &fmt[i], 2);
                    break;
            }

            arg_index++;
            i++; // Skip format character
        } else {
            // Regular character
            int len = strlen(buffer);
            buffer[len] = fmt[i];
            buffer[len+1] = 0;
        }
    }

    return buffer;
}

int EntGetStatPretty(element_value_t **fill, stat_t* stat){
  element_value_t* lbl = calloc(1,sizeof(element_value_t));

  lbl->type = VAL_CHAR;
  lbl->c = malloc(sizeof(char)*MAX_NAME_LEN);
  strcpy(lbl->c ,STAT_STRING[stat->type].name);
  lbl->rate = FETCH_NONE;

  element_value_t* cur = calloc(1,sizeof(element_value_t));
  
  cur->type = VAL_CHAR;
  cur->rate = FETCH_UPDATE;
  cur->get_val = StatGetPretty;
  cur->context = stat;
  cur->c = malloc(sizeof(char)*MAX_NAME_LEN);

  fill[0] = lbl;
  fill[1] = cur;
  return 2;

}

int EntGetSkillPretty(element_value_t **fill, skill_t* skill){
  element_value_t* lbl = calloc(1,sizeof(element_value_t));

  lbl->type = VAL_CHAR;
  strcpy(lbl->c ,SKILL_STRING[skill->id].name);
  lbl->rate = FETCH_NONE;

  element_value_t* cur = calloc(1,sizeof(element_value_t));
  
  cur->type = VAL_INT;
  cur->rate = FETCH_UPDATE;
  cur->get_val = SkillGetPretty;
  cur->context = skill;

  fill[0] = lbl;
  fill[1] = cur;
  return 2;

}

int EntGetNamePretty(element_value_t **fill, ent_t* e ){
  element_value_t* name = calloc(1,sizeof(element_value_t));

  name->type = VAL_CHAR;
  name->c = calloc(1,sizeof(e->name));
  strcpy(name->c,e->name);
  name->rate = FETCH_NONE;

  element_value_t* role = calloc(1,sizeof(element_value_t));

  role->type = VAL_CHAR;
  role->rate = FETCH_NONE;
  role->c = EntGetClassNamePretty(e);

  fill[0] = name;
  fill[1] = role;

  return 2;
}

element_value_t* StatGetPretty(element_value_t* self, void* context){
  stat_t* stat =  context;

  switch(self->type){
    case VAL_INT:
      *self->i = (int)stat->current;
      break;
    case VAL_FLOAT:
      *self->f = stat->ratio(stat);
      break;
    case VAL_CHAR:
      strcpy(self->c,TextFormat("%i / %i",(int)stat->current,(int)stat->max));
      break;
  }
}

element_value_t* SkillGetPretty(element_value_t* self, void* context){
  skill_t* skill =  context;

  switch(self->type){
    case VAL_INT:
      *self->i = skill->val;
      break;
    case VAL_FLOAT:
      *self->f = skill->val;
      break;
    case VAL_CHAR:
      strcpy(self->c,TextFormat("%i - (%i / %i) EXP",skill->val, skill->point,skill->threshold));
      break;
  }
}
