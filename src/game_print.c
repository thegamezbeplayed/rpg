#include "game_ui.h"
#include "game_common.h"
#include "game_types.h"
#include "game_systems.h"
#include <stdio.h>

void StringPrependPadding(char* s, size_t padding){
    size_t len = strlen(s);

    if (len >= padding)
        return;

    size_t pad = padding - len;

    /* shift existing text right */
    memmove(s + pad, s, len + 1);   // +1 to move the '\0'

    /* fill the front with spaces */
    for (size_t i = 0; i < pad; i++)
        s[i] = ' ';
}

void StringAppendPadding(char* s, size_t padding){
  size_t len = strlen(s);

  if(len >= padding)
    return;


  s[padding] = '\0';
}

void PrintMobDetail(ent_t* e){
  faction_t* f = GetFactionByID(e->team);
  char* team = "";
  if(f)
    team = strdup(f->name);
  TraceLog(LOG_INFO,"%s lvl %i\n %s\n",e->name, e->skills[SKILL_LVL]->val,
      team);

  stat_t* health = e->stats[STAT_HEALTH];

  char* stat_str = strdup(STAT_STRING[STAT_HEALTH].name);
  TraceLog(LOG_INFO,"%s: [%i/%i] \n", stat_str, (int)health->current,(int)health->max);

   TraceLog(LOG_INFO,"<=====ATTRIBUTES=====>\n");
  for(int i = 0; i < ATTR_DONE; i++){
    if(e->attribs[i]==NULL)
      continue;

    int val = e->attribs[i]->val;
    const char* name = attributes[i].name;
    TraceLog(LOG_INFO,"%i %s",val,name);
  }
}

line_item_t* InitLineItem(element_value_t **val, int num_val, const char* format){
  line_item_t* ln = calloc(1,sizeof(line_item_t));

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
    static char buffer[2048];
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
              strcat(buffer, val->c);
              if(LIST_LEFT_HAND_PAD > v_len){
                p_len = LIST_LEFT_HAND_PAD - v_len;
                char l_pad[LIST_LEFT_HAND_PAD];
                RepeatChar(l_pad, p_len+1, ' ', p_len);

                strcat(buffer, l_pad);
              }
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


int CtxGetSkill(element_value_t **fill, local_ctx_t* ctx, GameObjectParam p){
  if(ctx->params[p].type_id != DATA_SKILL)
    return 0;

  skill_t* skill = ParamRead(&ctx->params[p], skill_t);
  element_value_t* lbl = calloc(1,sizeof(element_value_t));

  lbl->type = VAL_CHAR;
  lbl->c = malloc(sizeof(char)*MAX_NAME_LEN);
  strcpy(lbl->c ,SKILL_STRING[skill->id].name);

  lbl->rate = FETCH_NONE;

  element_value_t* cur = calloc(1,sizeof(element_value_t));

  cur->rate = FETCH_TURN;
  cur->context = skill;
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
  element_value_t* lbl = calloc(1,sizeof(element_value_t));

  lbl->type = VAL_INT;
  lbl->c = malloc(sizeof(char)*MAX_NAME_LEN);
  int exp = skill->threshold - (int)skill->point;
  *lbl->i = exp;

  lbl->rate = FETCH_NONE;

  element_value_t* cur = calloc(1,sizeof(element_value_t));

  cur->rate = FETCH_TURN;
  cur->context = skill;
  cur->c = malloc(sizeof(char)*MAX_NAME_LEN);
   
  cur->type = VAL_INT;
  int *next_lvl = cur->i;
  *next_lvl = skill->val+1;
  fill[0] = lbl;
  fill[1] = cur;
  return 2;
}

int CtxGetStat(element_value_t **fill, local_ctx_t* ctx, GameObjectParam p){
  if(ctx->params[p].type_id != DATA_STAT)
    return 0;

  stat_t* stat = ParamRead(&ctx->params[p], stat_t);
  element_value_t* lbl = calloc(1,sizeof(element_value_t));

  lbl->type = VAL_CHAR;
  lbl->c = malloc(sizeof(char)*MAX_NAME_LEN);
  strcpy(lbl->c ,STAT_STRING[stat->type].name);

  lbl->rate = FETCH_NONE;

  element_value_t* cur = calloc(1,sizeof(element_value_t));
  
  cur->type = VAL_CHAR;
  cur->rate = FETCH_TURN;
  cur->get_val = StatGetPretty;
  cur->context = stat;
  cur->c = malloc(sizeof(char)*MAX_NAME_LEN);

  fill[0] = lbl;
  fill[1] = cur;
  return 2;

}

int CtxGetStatDetails(element_value_t **fill, local_ctx_t* ctx, GameObjectParam p){
  if(ctx->params[p].type_id != DATA_STAT)
    return 0;

  stat_t* stat = ParamRead(&ctx->params[p], stat_t);
  element_value_t* lbl = calloc(1,sizeof(element_value_t));

  lbl->type = VAL_CHAR;
  lbl->c = malloc(sizeof(char)*MAX_NAME_LEN);
  strcpy(lbl->c ,STAT_STRING[stat->type].name);

  lbl->rate = FETCH_NONE;

  element_value_t* cur = calloc(1,sizeof(element_value_t));
  
  cur->type = VAL_CHAR;
  cur->rate = FETCH_TURN;
  cur->get_val = StatGetPretty;
  cur->context = stat;
  cur->c = malloc(sizeof(char)*MAX_NAME_LEN);

  fill[0] = lbl;
  fill[1] = cur;
  return 2;

}

int CtxGetString(element_value_t **fill, local_ctx_t* ctx, GameObjectParam p){
  if(ctx->params[p].type_id != DATA_STRING)
    return 0;

  element_value_t* txt = calloc(1,sizeof(element_value_t));

  txt->type = VAL_CHAR;

  
  txt->c = strdup(ParamReadString(&ctx->params[p]));
  txt->rate = FETCH_NONE;

  fill[0] = txt;

  return 1;
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
      strcpy(self->c,TextFormat("%i",skill->val));
      break;
  }
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

int SetActivityLines(element_value_t* ev, int pad[UI_POSITIONING]){
  char* fmt = "%s";
  element_value_t* base[MAX_LINE_VAL];

  int *pos = ev->context;
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

char* PrintElementValue(element_value_t* ev, int spacing[UI_POSITIONING]){
  char* out = calloc(1, 1024);

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

ParseToken TokenFromString(const char* s)
{
  for (int i = 0; i < TOKE_ALL; i++)
    if (strcmp(s, TOKEN_TABLE[i].name) == 0)
      return TOKEN_TABLE[i].token;

  return TOKE_ALL; // invalid / fallback
}

int ReadToken(const char* fmt, int start, char* out, int out_sz)
{
  int k = 0;
  int i = start;

  while (fmt[i] && fmt[i] != '}' && k < out_sz - 1) {
    out[k++] = fmt[i++];
  }

  out[k] = 0;

  return (fmt[i] == '}') ? i : -1;
}

char* ParseActivity(activity_t* act, char* buffer, size_t buf_size){
  activity_format_t a = ACT_LOG_FMT[act->kind];
  const char* fmt = a.fmt;

  size_t out = 0;
  int token_index = 0;

  buffer[0] = '\0';

  for (int i = 0; fmt[i] != 0 && out < buf_size - 1; i++)
  {
    if (fmt[i] == '{')
    {
      char tok[32];
      int end = ReadToken(fmt, i + 1, tok, sizeof(tok));

      if (end != -1)
      {
        ParseToken t = TokenFromString(tok);

        param_t* p = &act->tokens[t];

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
    buffer[out] = 0;
  }

  return buffer;
}
