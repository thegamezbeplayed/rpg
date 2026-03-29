#include "game_ui.h"
#include "game_common.h"
#include "game_types.h"
#include "game_systems.h"
#include "game_strings.h"
#include "game_helpers.h"

#include <stdio.h>

line_item_t* InitLineItem(element_value_t **val, int num_val, const char* format){
  line_item_t* ln = GameCalloc("InitLineItem", 1,sizeof(line_item_t));

  for (int i = 0; i < num_val; i++)
    ln->values[ln->num_val++]=val[i];

  ln->text_format = strdup(format);

}
void PrintSyncLine(line_item_t* ln, FetchRate poll){
    for(int i = 0; i < ln->num_val; i++){
      if(!ln->values[i]->get_val)
        continue;

      if(ln->values[i]->rate <= poll)
        ln->values[i]->get_val(ln->values[i],ln->values[i]->context);
    }
}

char *TextFormatLineItem(line_item_t *item) {
    char *buffer = GameMalloc("TextFormatLineItem", 2048 * sizeof(char));
    char temp[256];

    buffer[0] = 0;
    const char *fmt = item->text_format;

    item->r_len = 0;
    int arg_index = 0;

    for (int i = 0; fmt[i] != 0; i++) {
      // Check for '%' format token
      if (fmt[i] == '%' && fmt[i+1] != 0 && arg_index < item->num_val) {
        element_value_t *val = item->values[arg_index];
        size_t v_len = 0;
        size_t p_len = 0;
        size_t before = strlen(buffer);
        switch (fmt[i+1]) {
          case 'V':
            if (val->type == VAL_CHAR){
              StringPrependPadding(val->c, LIST_RIGHT_HAND_PAD);  
              strcat(buffer, val->c);
            }
            else
              strcat(buffer, "<BAD%S>");
            break;

          case 'S':
            if (val->type == VAL_CHAR){
              v_len = strlen(val->c);
              StringAppendPadding(val->c, LIST_LEFT_HAND_PAD);
              strcat(buffer, val->c);
              
            }
            else
              strcat(buffer, "<BAD%S>");
            break;
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
        size_t after = strlen(buffer);
        item->r_len += after - before;
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

element_value_t* CtxGetAbility(param_t ctx, GameObjectParam p, int index){
  if(ctx.type_id != DATA_ABILITY)
    return NULL;
  
  
  ability_t* a = ParamRead(&ctx, ability_t);
  if(!a)
    return NULL;

  element_value_t* ev = GameCalloc("CtxGetItem", 1,sizeof(element_value_t));

  
  ev->type = VAL_ICO;
  ev->rate = FETCH_EVENT;
  ev->index = index;
  ev->context = ctx;
//  ev->get_val = InventoryGetItem;

  if(a->spr){
    ev->s = a->spr;
    ev->s->slice->color = DAMAGE_SCHOOL[a->school].col; 
  }
  return ev;

} 

element_value_t* CtxGetItem(param_t ctx, GameObjectParam p, int index){
  if(ctx.type_id != DATA_ITEM)
    return NULL;

  element_value_t* ev = GameCalloc("CtxGetItem", 1,sizeof(element_value_t));

  ev->type = VAL_ICO;
  ev->rate = FETCH_EVENT;
  ev->index = index;
  ev->context = ctx;
  ev->get_val = InventoryGetItem;

  item_t* item = ParamRead(&ctx, item_t);

  if(!item || !item->sprite)
    return ev;

  ev->s = item->sprite;


  return ev;
}

int CtxGetSkill(element_value_t **fill, local_ctx_t* ctx, GameObjectParam p){
  if(ctx->params[p].type_id != DATA_SKILL)
    return 0;

  skill_t* skill = ParamRead(&ctx->params[p], skill_t);
  element_value_t* lbl = GameCalloc("CtxGetSkill", 1,sizeof(element_value_t));

  lbl->type = VAL_CHAR;
  lbl->c = malloc(sizeof(char)*MAX_NAME_LEN);
  strcpy(lbl->c ,SKILL_NAMES[skill->id]);

  lbl->rate = FETCH_NONE;

  element_value_t* cur = GameCalloc("CtxGetSkill", 1,sizeof(element_value_t));

  cur->rate = FETCH_TURN;
  cur->context = ctx->params[p];
  cur->c = malloc(sizeof(char)*MAX_NAME_LEN);
   
  cur->type = VAL_CHAR;
  cur->get_val = SkillGetPretty;

  fill[0] = lbl;
  fill[1] = cur;
  return 2;
}
 
int CtxGetSkillDetails(element_value_t **fill, local_ctx_t* ctx, GameObjectParam p){
  if(ctx->params[p].type_id != DATA_SKILL)
    return 0;

  skill_t* skill = ParamRead(&ctx->params[p], skill_t);
  element_value_t* lbl = GameCalloc("CtxGetSkillDetails", 1,sizeof(element_value_t));

  lbl->type = VAL_INT;
  lbl->c = malloc(sizeof(char)*MAX_NAME_LEN);
  int exp = skill->threshold - (int)skill->point;
  *lbl->i = exp;

  lbl->rate = FETCH_NONE;

  element_value_t* cur = GameCalloc("CtxGetSkillDetails", 1,sizeof(element_value_t));

  cur->rate = FETCH_TURN;
  cur->context = ctx->params[p];
  cur->i = GameMalloc("CtxGetSkillDetails", sizeof(int));
   
  cur->type = VAL_INT;
  int *next_lvl = cur->i;
  *next_lvl = skill->val+1;
  fill[0] = lbl;
  fill[1] = cur;
  return 2;
}

int GetInventoryDetails(element_value_t **fill, param_t p){

  element_value_t* name = GameCalloc("GetInventoryDetails", 1,sizeof(element_value_t));

  inventory_t* inv = ParamRead(&p, inventory_t);
  name->type = VAL_CHAR;
  name->c = GameCalloc("GetInventoryDetails", 1,sizeof(element_value_t));

  strcpy(name->c, INV_STRINGS[inv->id]);

  name->rate = FETCH_ONCE;

  element_value_t* amnt = GameCalloc("GetInventoryDetails", 1,sizeof(element_value_t));


  amnt->type = VAL_INT;
  amnt->rate = FETCH_EVENT;
  amnt->i = &inv->count;

  fill[0] = name;
  fill[1] = amnt;

  return 2;
}

int CtxGetAttr(element_value_t **fill, local_ctx_t* ctx, GameObjectParam p){
  if(ctx->params[p].type_id != DATA_ATTR)
    return 0;
      
  attribute_t* attribute = ParamRead(&ctx->params[p], attribute_t);
  element_value_t* lbl = GameCalloc("CtxGetAttr", 1,sizeof(element_value_t)); 

  lbl->type = VAL_CHAR;
  lbl->c = GameMalloc("CtxGetAttr", sizeof(char)*MAX_NAME_LEN);
  strcpy(lbl->c ,ATTR_STRING[attribute->type].name);
    
  lbl->rate = FETCH_NONE;
    
  element_value_t* cur = GameCalloc("CtxGetAttr", 1,sizeof(element_value_t));
 
  cur->type = VAL_CHAR;
  cur->rate = FETCH_TURN;
  cur->get_val = AttrGetPretty;
  cur->context = ctx->params[p];
  cur->c = GameMalloc("CtxGetAttr", sizeof(char)*MAX_NAME_LEN);
        
  fill[0] = lbl;
  fill[1] = cur;
  return 2;

}

int CtxGetStat(element_value_t **fill, local_ctx_t* ctx, GameObjectParam p){
  if(ctx->params[p].type_id != DATA_STAT)
    return 0;

  stat_t* stat = ParamRead(&ctx->params[p], stat_t);
  element_value_t* lbl = GameCalloc("CtxGetStat", 1,sizeof(element_value_t));

  lbl->context = ctx->params[p];
  lbl->type = VAL_CHAR;
  lbl->c = GameMalloc("CtxGetStat", sizeof(char)*MAX_NAME_LEN);
  strcpy(lbl->c ,STAT_STRING[stat->type].name);

  lbl->rate = FETCH_NONE;

  element_value_t* cur = GameCalloc("CtxGetStat", 1,sizeof(element_value_t));
  
  cur->type = VAL_CHAR;
  cur->rate = FETCH_TURN;
  cur->get_val = StatGetPretty;
  cur->context = ctx->params[p];
  cur->c = GameMalloc("CtxGetStat", sizeof(char)*MAX_NAME_LEN);

  fill[0] = lbl;
  fill[1] = cur;
  return 2;

}

int GetConsumeDesc(line_item_t** li, item_t* item){
  element_value_t* name = GameCalloc("GetConsumeDesc", 1,sizeof(element_value_t));
  element_value_t* base[MAX_LINE_VAL];
    
  int count = 0;
  name->type = VAL_CHAR;
  name->rate = FETCH_ONCE;
  name->c = strdup(item->def->name);
  name->get_val = NULL;
  base[0] = name;
  li[count++] = InitLineItem(base, 1, "%s");
  
  detail_format_t fmt = ITEM_FORMAT[ITEM_CONSUMABLE][item->def->type]; 
  consume_def_t *def = item->def->type_def;
  
  DataType dtype = -1;
  for (int i = 0; i < fmt.num_lines; i++){
    char str[MAX_NAME_LEN] = {0}; 
    int ival = -1;
    int num_tokens = 0;
    param_t token_params[TOKE_ALL];
    for (int j = 0; j < TOKE_ALL; j++){
      param_t token;
      ParseToken t = fmt.tokens[i][j];
      switch(t){
        case TOKE_RESOURCE:
          dtype = DATA_STRING;
          strcpy(str, STAT_STRING[def->chain_id].name);
          break;
        case TOKE_ABILITY:
          if(!def)
            continue;
          dtype = DATA_STRING;
          strcpy(str, GetAbilityName(def->chain_id));
          break;
        case TOKE_SCHOOL:
          break;
        case TOKE_SKILL:
          if(!def)
            continue;
          dtype = DATA_STRING;
          strcpy(str, SKILL_NAMES[def->chain_id]);
          break;
        case TOKE_AMNT:
          int min = item->values[VAL_DMG_DIE]->val;
          int max = item->values[VAL_DMG]->val * min;

          dtype = DATA_STRING;
          snprintf(str, sizeof(str), "%i to %i",min, max);
          break;
        case TOKE_DMG:
          if(!item->ability)
            continue;
          ability_t* a = item->ability;
          int min_roll = 1 * a->dc->num_die;

          int max_roll = a->dc->sides * a->dc->num_die;

          dtype = DATA_STRING;
          snprintf(str, sizeof(str), "%i to %i",min_roll, max_roll);
          break;
        case TOKE_STAT:
          break;
        case TOKE_DOES:
          dtype = DATA_STRING;
          switch(item->def->type){
            case CONS_POT:
              strcpy(str, "Restores\0");
              break;
          };
          break;
        case TOKE_USES:
         dtype = DATA_INT;
         ival = item->values[VAL_QUANT]->val;
         break;
        case TOKE_NAME:
          break;
        default:
          continue;

      };

      switch(dtype){
        case DATA_STRING:
          str[strlen(str)+1] = '\0';
          token_params[t] = ParamMake(DATA_STRING, strlen(str)+1, str);
          break;
        case DATA_INT:
          token_params[t] = ParamMake(DATA_INT, sizeof(int), &ival);
          break;
      };
    }
    num_tokens++;

    li[count++] = StringDetailsFormat(fmt.fmt[i], num_tokens, token_params);
  }

  return count;


}

int GetAbilityDesc(line_item_t** li, ability_t* a){
  element_value_t* name = GameCalloc("GetAbilityDesc", 1,sizeof(element_value_t));
  element_value_t* base[1];

  int count = 0;
  name->type = VAL_CHAR;
  name->rate = FETCH_ONCE;
  name->c = strdup(GetAbilityName(a->id));

  base[0] = name;

  element_value_t* school[1];

  element_value_t* type = GameCalloc("GetWeapDesc",1,sizeof(element_value_t));

  type->type = VAL_CHAR;
  type->rate = FETCH_ONCE;
  type->c = strdup(SKILL_NAMES[a->skills[0]]);

 
  element_value_t* dmg = GameCalloc("GetAbilityDesc", 1,sizeof(element_value_t));

  dmg->type = VAL_CHAR;
  dmg->rate = FETCH_ONCE;
  dmg->c = strdup(DAMAGE_STRING[a->school]);
 element_value_t* min_dmg = GameCalloc("GetWeapDesc", 1,sizeof(element_value_t));
  min_dmg->i = GameMalloc("GetAbilityDesc",sizeof(int));
  element_value_t* max_dmg = GameCalloc("GetWeapDesc", 1,sizeof(element_value_t));


  max_dmg->i = GameMalloc("GetAbilityDesc",sizeof(int));
  min_dmg->type = VAL_INT;
  min_dmg->rate = FETCH_ONCE;
  int min_roll = 1 * a->dc->num_die;
  *min_dmg->i =  min_roll;

  int max_roll = a->dc->sides * a->dc->num_die;

  max_dmg->type = VAL_INT;
  max_dmg->rate = FETCH_ONCE;
  *max_dmg->i = max_roll;


  school[0] = type;

  element_value_t* vals[4];

  element_value_t* tar = GameCalloc("GetAbilityDesc", 1,sizeof(element_value_t));

  tar->type = VAL_CHAR;
  tar->rate = FETCH_ONCE;

  tar->c = strdup(TargetingStringDesc(a->targeting));

  vals[0] = min_dmg;
  vals[1] = max_dmg;
  vals[2] = dmg;
  vals[3] = tar;
  
  element_value_t* ranges[1];
  element_value_t* range = GameCalloc("GetAbilityDesc", 1,sizeof(element_value_t));
 

  range->type = VAL_INT;
  range->rate = FETCH_ONCE;
  range->i = GameMalloc("GetAbilityDesc", sizeof(int));
  *range->i = a->stats[STAT_REACH]->current;
  ranges[0] = range;
 
  element_value_t* costs[2];

  element_value_t* cost = GameCalloc("GetAbilityDesc", 1,sizeof(element_value_t));
 
  cost->type = VAL_INT;
  cost->rate = FETCH_ONCE;
  cost->i = GameMalloc("GetAbilityDesc", sizeof(int));
  *cost->i = a->cost;
  
  element_value_t* resource = GameCalloc("GetAbilityDesc", 1,sizeof(element_value_t));

  resource->type = VAL_CHAR;
  resource->rate = FETCH_ONCE;
  resource->c = strdup(STAT_STRING[a->resource].name);

  costs[0] = cost;
  costs[1] = resource;
  li[count++] = InitLineItem(base, 1, "%s");

  li[count++] = InitLineItem(school, 1, "School of %s");
  li[count++] = InitLineItem(costs, 2, "Costs %i %s");
  li[count++] = InitLineItem(vals, 4, "Deals %i - %i %s damage %s.");
  li[count++] = InitLineItem(ranges, 1, "Range: %i tiles");

    return count;
}

int GetWeapDesc(line_item_t** li, item_t* item){

  element_value_t* name = GameCalloc("GetWeapDesc", 1,sizeof(element_value_t));
  element_value_t* base[MAX_LINE_VAL];

  int count = 0;
  name->type = VAL_CHAR;
  name->rate = FETCH_ONCE;
  name->c = strdup(item->def->name);

  base[0] = name;
  li[count++] = InitLineItem(base, 1, "%s");

  element_value_t* min_dmg = GameCalloc("GetWeapDesc", 1,sizeof(element_value_t));
  min_dmg->i = malloc(sizeof(int));
  element_value_t* max_dmg = GameCalloc("GetWeapDesc", 1,sizeof(element_value_t));

  max_dmg->i = malloc(sizeof(int));
  min_dmg->type = VAL_INT;
  min_dmg->rate = FETCH_ONCE;
  int min_roll = 1 * item->ability->dc->num_die;
  *min_dmg->i =  min_roll;

  int max_roll = item->ability->dc->sides * item->ability->dc->num_die;

  max_dmg->type = VAL_INT;
  max_dmg->rate = FETCH_ONCE;
  *max_dmg->i = max_roll;

  element_value_t* type = GameCalloc("GetWeapDesc",1,sizeof(element_value_t));
  type->type = VAL_CHAR;
  type->rate = FETCH_ONCE;
  type->c = strdup(DAMAGE_STRING[item->ability->school]);
  
  element_value_t* dmg[MAX_LINE_VAL];
  element_value_t* hit[MAX_LINE_VAL];
  
  dmg[0] = min_dmg;
  dmg[1] = max_dmg;
  dmg[2] = type;
  li[count++] = InitLineItem(dmg, 3, "Deals %i to %i %s Damage");
  return count;
}

int GetArmorDesc(line_item_t** li, item_t* item){
  element_value_t* name = GameCalloc("GetArmorDesc", 1,sizeof(element_value_t));
  element_value_t* base[1];
  
  int count = 0;
  name->type = VAL_CHAR;
  name->rate = FETCH_ONCE;
  name->c = strdup(item->def->name);

  base[0] = name;
  li[count++] = InitLineItem(base, 1, "%s");

  
  element_value_t* ac = GameCalloc("GetArmorDesc", 1,sizeof(element_value_t));

  ac->type = VAL_INT;
  ac->rate = FETCH_ONCE;
  ac->i = GameMalloc("GetArmorDesc", sizeof(int));

  
  *ac->i = item->values[VAL_SAVE]->val;
  element_value_t* base_ac[1];
 
  base_ac[0] = ac;
  li[count++] = InitLineItem(base_ac, 1, "Armor Class: %i");

 
  for (int i = 0; i < DMG_DONE; i++){
    element_value_t* fill[MAX_LINE_VAL];

    if (item->def->dr->resist_types[i] < 1)
      continue;
    element_value_t* tag = GameCalloc("GetArmorDesc", 1,sizeof(element_value_t));
    tag->c = strdup(DAMAGE_STRING[i]);
    tag->type = VAL_CHAR;
    
    element_value_t* dr = GameCalloc("GetArmorDesc", 1,sizeof(element_value_t));

    dr->type = VAL_INT;
    dr->rate = FETCH_ONCE;
    dr->i = GameMalloc("GetArmorDesc", sizeof(int));
    *dr->i = item->def->dr->resist_types[i];
    fill[0] = tag;
    fill[1] = dr;
    li[count++] = InitLineItem(fill, 2, "%s Damage Reduction: %i");
  }

  return count;


}

int CtxGetItemDesc(element_value_t **fill, void* ctx, GameObjectParam p){
  item_t* item = ctx;
  int lines = 0;
  switch(item->def->category){

    case ITEM_WEAPON:
      break;
    case ITEM_ARMOR:
      //lines = GetArmorDesc(fill, item);
      
      break;
    case ITEM_CONSUMABLE:
      break;
    default:
      break;

  }


}

int CtxGetStatDetails(element_value_t **fill, local_ctx_t* ctx, GameObjectParam p){
  if(ctx->params[p].type_id != DATA_STAT)
    return 0;

  stat_t* stat = ParamRead(&ctx->params[p], stat_t);
  element_value_t* lbl = GameCalloc("CtxGetStatDetails", 1,sizeof(element_value_t));

  lbl->type = VAL_CHAR;
  lbl->c = GameMalloc("CtxGetStatDetails", sizeof(char)*MAX_NAME_LEN);
  strcpy(lbl->c ,STAT_STRING[stat->type].name);

  lbl->rate = FETCH_NONE;

  element_value_t* cur = GameCalloc("CtxGetStatDetails", 1,sizeof(element_value_t));
  
  cur->type = VAL_CHAR;
  cur->rate = FETCH_TURN;
  cur->get_val = StatGetPretty;
  cur->context = ctx->params[p];
  cur->c = GameMalloc("CtxGetStatDetails",sizeof(char)*MAX_NAME_LEN);

  fill[0] = lbl;
  fill[1] = cur;
  return 2;

}

int CtxGetString(element_value_t **fill, local_ctx_t* ctx, GameObjectParam p){
  if(ctx->params[p].type_id != DATA_STRING)
    return 0;

  element_value_t* txt = GameCalloc("CtxGetString", 1,sizeof(element_value_t));

  txt->type = VAL_CHAR;

 
  txt->c = GameCalloc("CtxGetString", MAX_NAME_LEN, sizeof(char)); 
  strncpy(txt->c, ParamReadString(&ctx->params[p]), MAX_NAME_LEN -1);
  txt->c[MAX_NAME_LEN-1] = '\0';
  txt->rate = FETCH_NONE;

  fill[0] = txt;

  return 1;
}

element_value_t* AttrGetPretty(element_value_t* self, param_t context){
  attribute_t* attribute =  ParamRead(&context, attribute_t);

  switch(self->type){
    case VAL_INT:
      *self->i = (int)attribute->val;
      break;
    case VAL_FLOAT:
      break;
    case VAL_CHAR:
      strcpy(self->c,TextFormat("%i",(int)attribute->val));
      break;
  }

}

element_value_t* StatGetPretty(element_value_t* self, param_t context){
  if(context.type_id != DATA_STAT)
    return NULL;

  stat_t* stat =  ParamRead(&context, stat_t);

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

element_value_t* InventoryGetItem(element_value_t* self, param_t context){
  if(context.type_id != DATA_ITEM)
    return NULL;

  item_t* item = ParamRead(&context, item_t);

  if(!item || !item->sprite)
    return NULL;

  self->s = item->sprite;

  return self;
}

element_value_t* SkillGetDetailed(element_value_t* self, param_t context){
  if(context.type_id != DATA_SKILL)
    return NULL;

  skill_t* skill =  ParamRead(&context, skill_t);

  
  element_value_t* cur = GameCalloc("CtxGetSkill", 1,sizeof(element_value_t));

  cur->rate = FETCH_EVENT;
  cur->type = VAL_INT;
  cur->context = context;
  cur->i = GameMalloc("SkillGetPretty", sizeof(int));
  
  *cur->i = skill->val; 


  cur->get_val = SkillGetPretty;

  element_value_t* val[1];
  val[0] = cur;

  self->l[self->num_ln++] = InitLineItem(val, 1, "Level %i");

  for (int i = 0; i < self->num_ln; i++){
    PrintSyncLine(self->l[i],FETCH_EVENT);
    const char* ln = TextFormatLineItem(self->l[i]);
    TraceLog(LOG_INFO, "MEASURE: %s", ln);

    Vector2 size = MeasureTextEx(ui.font, ln, ui.text_size, ui.text_spacing);

    self->l[i]->r_wid = 16 + size.x;
    self->l[i]->r_hei = size.y;
  }
  return self;
}

element_value_t* SkillGetValues(element_value_t* e, param_t p){
  if(p.type_id != DATA_SKILL)
    return e;

  skill_t* s = ParamRead(&p, skill_t);
  switch(p.gouid){
    case 0:
      *e->i = s->val;
      break;
    case 1:
      *e->i = s->min;
      break;
    case 2:
      *e->i = s->max;
      break;
    case 3:
      *e->i = s->threshold;
      break;
    case 4:
      *e->i = s->point;
      break;

  }
}

element_value_t* SkillGetPretty(element_value_t* self, param_t context){
  if(context.type_id != DATA_SKILL)
    return NULL;

  skill_t* skill =  ParamRead(&context, skill_t);

  switch(self->type){
    case VAL_INT:
      *self->i = skill->val;
      break;
    case VAL_FLOAT:
      *self->f = skill->val;
      break;
    case VAL_CHAR:
      if(skill->id == SKILL_LVL)
      strcpy(self->c,TextFormat("%i",skill->val));
      else{
      self->rate = FETCH_EVENT;
      strcpy(self->c,SkillGetTitle(skill));
      }
      break;
    case VAL_LN:
      return SkillGetDetailed(self, context);
      break;
    case VAL_PROG:
      self->p->val = GameMalloc("SkillGetPretty", sizeof(float));
      *self->p->val = skill->point;
      self->p->min = ZERO; 
      self->p->max = skill->threshold; 
      element_value_t *right[2];
      element_value_t *left[1];
      element_value_t* cur = GameMalloc("SkillGetPretty", sizeof(element_value_t));
      element_value_t* lvl = GameMalloc("SkillGetPretty", sizeof(element_value_t));
      element_value_t* thresh = GameMalloc("SkillGetPretty", sizeof(element_value_t));

      cur->type = VAL_INT;
      cur->rate = FETCH_EVENT;
      cur->context = context;
      cur->context.gouid = 4;
      cur->get_val = SkillGetValues;
      cur->i = GameMalloc("SkillGetPretty", sizeof(int));
      *cur->i = (int)skill->point;


      thresh->type = VAL_INT;
      thresh->get_val = SkillGetValues;
      thresh->context = context;
      thresh->rate = FETCH_EVENT;
      thresh->i = GameMalloc("SkillGetPretty", sizeof(int));
      *thresh->i = (int)skill->threshold;

      thresh->context.gouid = 3;
      right[0] = cur;
      right[1] = thresh;
      self->p->right = InitLineItem(right, 2, "%i / %i");
 
      lvl->type = VAL_INT;
      lvl->rate = FETCH_EVENT;
      lvl->context = context;
      lvl->context.gouid = 0;
      lvl->get_val = SkillGetValues;
      lvl->i = GameMalloc("SkillGetPretty", sizeof(int));
      *lvl->i = (int)skill->val;

      left[0] = lvl;
      self->p->left = InitLineItem(left, 1, "Level: %i");
      
      PrintSyncLine(self->p->left,FETCH_EVENT);
      PrintSyncLine(self->p->right,FETCH_EVENT);

      break;
  }
      
  return self;
}

int SetCtxDetails(local_ctx_t* ctx , line_item_t** li, const char fmt[PARAM_ALL][MAX_NAME_LEN], int pad[UI_POSITIONING], bool combo){
  int count = 0, num_p = 0;

  for(int i = 0; i < PARAM_ALL; i++){
    if(fmt[i][0] == '\0')
      continue;

    num_p++;
  }
  for (int i = 0; i < PARAM_ALL; i++){
    if(fmt[i][0] == '\0')
      continue;


    int p_left = pad[UI_PADDING_LEFT];
    int p_right = pad[UI_PADDING_RIGHT];

    GameObjectParam param = i;
    int des_len = li[count]->des_len;
    char* format = strdup(fmt[i]);

    int items = 0;
    element_value_t* base[MAX_LINE_VAL];
    switch(ctx->params[param].type_id){
      case DATA_STAT:
        items = CtxGetStatDetails(base, ctx, param);
        break;
      case DATA_SKILL:
        format = "%i exp till Level %i";
        items = CtxGetSkillDetails(base, ctx, param);
        break;
      //case DATA
      default:
        continue;
        break;
    }

    li[count] = InitLineItem(base,items,format);
    PrintSyncLine(li[count],FETCH_ONCE);
    const char* ln = TextFormatLineItem(li[count]);
    TraceLog(LOG_INFO, "MEASURE: %s", ln);

    Vector2 size = MeasureTextEx(ui.font, ln, ui.text_size, ui.text_spacing);
    if(size.x > des_len){
      int space = size.x - des_len;
      p_right += space/2;
      //p_left += space/2;

    }
    li[count]->r_wid = size.x + p_right;
    li[count]->r_hei = size.y;

    count++;
  }

  return count;
}

int SetCtxDescription(param_t ctx , line_item_t** li, int pad[UI_POSITIONING]){

  int p_left = pad[UI_PADDING_LEFT];
  int p_right = pad[UI_PADDING_RIGHT];

  element_value_t* base[MAX_LINE_VAL];
  int lines = 0;
  int des_len = li[0]->des_len;
  char* format = "%s";

  switch (ctx.type_id){
    case DATA_ITEM:
      item_t* item = ParamRead(&ctx, item_t);
      if(!item || !item->def)
        return 0;

      switch(item->def->category){
        case ITEM_WEAPON:
          lines = GetWeapDesc(li, item);
          break;
        case ITEM_ARMOR:
          lines = GetArmorDesc(li, item);

          break;
        case ITEM_CONSUMABLE:
          lines = GetConsumeDesc(li, item);
          break;
        default:
          break;

      }
      break;
    case DATA_ABILITY:
      ability_t *a = ParamRead(&ctx, ability_t);
      lines = GetAbilityDesc(li, a);
      break;
  }

  for(int i = 0; i < lines; i++){
    PrintSyncLine(li[i],FETCH_ONCE);
    const char* ln = TextFormatLineItem(li[i]);

    TraceLog(LOG_INFO, "MEASURE: %s", ln);

    Vector2 size = MeasureTextEx(ui.font, ln, ui.text_size, ui.text_spacing);
    if(size.x > des_len){
      int space = size.x - des_len;
      p_right += space/2;
      //p_left += space/2;

    }
    li[i]->r_wid = size.x + p_right;
    li[i]->r_hei = size.y;
  }
  return lines;
}

int SetActivityLines(element_value_t* ev, int pad[UI_POSITIONING]){
  char* fmt = "%s";
  element_value_t* base[MAX_LINE_VAL];

  int *pos = ParamRead(&ev->context, int);
  int items = ActivitiesAssignValues(base, *pos);
  ev->l[0] = InitLineItem(base, items, fmt);

  int des_len = 128;
  int p_left = pad[UI_PADDING_LEFT];
  int p_right = pad[UI_PADDING_RIGHT];

  PrintSyncLine(ev->l[0],FETCH_ONCE);
  const char* ln = TextFormatLineItem(ev->l[0]);
  TraceLog(LOG_INFO, "MEASURE: %s", ln);

  Vector2 size = MeasureTextEx(ui.font, ln, ui.text_size, ui.text_spacing);
  if(size.x > des_len){
    int space = size.x - des_len;
    p_right += space/2;
    //p_left += space/2;

  }
  ev->l[0]->r_wid = size.x + p_right;
  ev->l[0]->r_hei = size.y;


  return 1;
}

element_value_t* SetCtxIcon(param_t ctx, GameObjectParam params[4], int index){

  switch(ctx.type_id){
    case DATA_ITEM:
      return CtxGetItem(ctx, params[0], index);
      break;
    case DATA_ABILITY:
      return CtxGetAbility(ctx, params[0], index);
      break;
    default:
      return NULL;
  }
}

int SetParamDescription(line_item_t** li, int count, param_t param){

  int lines = 0;
  element_value_t* base[MAX_LINE_ITEMS];
  switch(param.type_id){
    case DATA_INV:
      lines = GetInventoryDetails(base, param);

      li[count] = InitLineItem(base,lines,"%s: %i");
      PrintSyncLine(li[count],FETCH_ONCE);
      const char* ln = TextFormatLineItem(li[count]);
      TraceLog(LOG_INFO, "MEASURE: %s", ln);

      Vector2 size = MeasureTextEx(ui.font, ln, ui.text_size, ui.text_spacing);
      li[count]->r_wid = size.x;
      li[count]->r_hei = size.y;
      break;
    default:
      return 0;
      break;
  }

  return 1;
}

int SetCtxParams(local_ctx_t* ctx, line_item_t** li, const char fmt[PARAM_ALL][MAX_NAME_LEN], int pad[UI_POSITIONING], bool combo){
  int count = 0, num_p = 0;
  
  for(int i = 0; i < PARAM_ALL; i++){
    if(fmt[i][0] == '\0')
      continue;

    num_p++;
  }

  if(combo  && num_p > 1)
    return 0; // OTHER FUNC TODO
  
  for (int i = 0; i < PARAM_ALL; i++){
    if(fmt[i][0] == '\0')
      continue;

    int p_left = pad[UI_PADDING_LEFT];
    int p_right = pad[UI_PADDING_RIGHT];

    GameObjectParam param = i;
    int des_len = li[count]->des_len;

    int items = 0;
    element_value_t* base[MAX_LINE_VAL];
    switch(ctx->params[param].type_id){
      case DATA_STRING:
        items = CtxGetString(base, ctx, param);
        break;
      case DATA_STAT:
        items = CtxGetStat(base, ctx, param);
        break;
      case DATA_SKILL:
        items = CtxGetSkill(base, ctx, param);
        break;
      case DATA_ATTR:
        items = CtxGetAttr(base, ctx, param);
        break;
      default:
        continue;
        break;
    }
        
    li[count] = InitLineItem(base,items,fmt[i]);
    PrintSyncLine(li[count],FETCH_ONCE);
    const char* ln = TextFormatLineItem(li[count]);
    TraceLog(LOG_INFO, "MEASURE: %s", ln);

    Vector2 size = MeasureTextEx(ui.font, ln, ui.text_size, ui.text_spacing);
    if(size.x > des_len){
      int space = size.x - des_len;
      p_right += space/2;
      //p_left += space/2;

    }
    li[count]->r_wid = size.x + p_right;
    li[count]->r_hei = size.y;

    count++;
  }

  return count;
}

char* PrintElementValue(element_value_t* ev, int spacing[UI_POSITIONING], char* out){
  //char* out = GameCalloc("PrintElementValue", 1, 1024);

  out[0] = '\0';
  int p_top = imax(spacing[UI_PADDING], spacing[UI_PADDING_TOP]);
  int p_bot = 1 + imax(spacing[UI_PADDING], spacing[UI_PADDING_BOT]);

  char start_ln[16];
  char end_ln[16];
  RepeatChar(end_ln, sizeof(end_ln), '\n', p_bot);
  RepeatChar(start_ln, sizeof(start_ln), '\n', p_top);
  for( int n = 0; n < ev->num_ln; n++){

    int i = ev->reverse
      ? (ev->num_ln - 1 - n)
      : n;

    const char* ln = TextFormatLineItem(ev->l[i]);
    strcat(out,start_ln);
    strcat(out,ln);
    strcat(out,end_ln);
  }

  return out;
  
}

char* ParseString(const char* fmt, char* buffer, size_t buf_size, int num_tokens, param_t tokens[num_tokens]){
  size_t out = 0;
  for (int i = 0; fmt[i] != 0 && out < buf_size - 1; i++)
  {
    if (fmt[i] == '{')
    {
      char tok[32];
      int end = ReadToken(fmt, i + 1, tok, sizeof(tok));

      if (end != -1)
      {
        ParseToken t = TokenFromString(tok);

        param_t* p = &tokens[t];

        char tmp[64];
        const char* str = "";

        switch (p->type_id)
        {
          case DATA_STRING:
            str = ParamReadString(p);
            break;

          case DATA_INT:
            snprintf(tmp, sizeof(tmp), "%i", ParamReadInt(p));
            str = tmp;
            break;

          case DATA_FLOAT:
            snprintf(tmp, sizeof(tmp), "%.1f", ParamReadFloat(p));
            str = tmp;
            break;
        }

        out += snprintf(buffer + out, buf_size - out, "%s", str);

        i = end; // skip past '}'
        continue;
      }
    }

    buffer[out++] = fmt[i];
    //buffer[out] = 0;
  }
}
char* ParseActivity(activity_t* act, char* buffer, size_t buf_size){
  activity_format_t a = ACT_LOG_FMT[act->kind];
  const char* fmt = a.fmt;

  buffer[0] = '\0';

  ParseString(fmt, buffer, buf_size, TOKE_ALL, act->tokens);
  

  return buffer;
}

