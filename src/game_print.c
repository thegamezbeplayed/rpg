#include "game_ui.h"
#include "game_common.h"
#include "game_types.h"
#include "game_systems.h"
#include "game_strings.h"
#include "game_helpers.h"
#include "game_process.h"

#include <stdio.h>

line_item_t* InitLineItem(element_value_t *val, const char* format){
  line_item_t* ln = GameCalloc("InitLineItem", 1,sizeof(line_item_t));

  ln->values[ln->num_val++]=val;

  ln->text_format = strdup(format);

  return ln;
}

int CtxGetDebugAction(line_item_t **fill, local_ctx_t* ctx, param_t vctx){
  if(ctx->other.type_id != DATA_ENTITY)
    return 0;
 
  ent_t* e = ctx->other.data;

  if(!e->control || !e->control->actions)
    return 0;

  int count = 0;
  for(int i = 0; i < ACT_DONE; i++){
    action_queue_t* q = e->control->actions->queues[i];

    if(q->count < 1)
      continue;

    for(int j = 0; j < q->count; j++){
      action_t* a = &q->entries[j];
  
      element_value_t* acts[4];

      element_value_t* turn = GameCalloc("CtxGetDebugPrio", 1,sizeof(element_value_t));

      turn->context = vctx;
      turn->type = VAL_INT;
      turn->rate = FETCH_UPDATE;
      turn->i = GameCalloc("CtxGetDebugPrio", 1, sizeof(int));
      *turn->i = a->turn;

      element_value_t* score = GameCalloc("CtxGetDebugPrio", 1,sizeof(element_value_t));

      score->context = vctx;
      score->type = VAL_INT;
      score->rate = FETCH_UPDATE;
      score->i = GameCalloc("CtxGetDebugPrio", 1, sizeof(int));
      *score->i = a->score;

      element_value_t* initiative = GameCalloc("CtxGetDebugPrio", 1,sizeof(element_value_t));

      initiative->type = VAL_INT;
      initiative->context = vctx;
      initiative->rate = FETCH_UPDATE;
      initiative->i = GameCalloc("CtxGetDebugPrio", 1, sizeof(int));
      *initiative->i = a->initiative;



      element_value_t* name = GameCalloc("CtxGetDebugPrio", 1,sizeof(element_value_t));

      name->type = VAL_CHAR;
      name->rate = FETCH_UPDATE;
      name->c = strdup(ACTIONTYPE_STRINGS[a->type]);

      acts[0] = turn;
      acts[1] = score; 
      acts[2] = name;
      acts[3] = initiative;

      line_item_t* line = InitLineItems(acts, 4,
          "Turn %i | Score: %i | Action: %s | Speed: %i");
   
       element_value_t* item = GameCalloc("CtxGetDebugPrio", 1,sizeof(element_value_t));

      item->index = count;
      item->type = VAL_LN;
      item->rate = FETCH_UPDATE;
      item->context = vctx;
      item->get_val = DebugGetAction;
      item->l[0] = line;
      
     fill[count++] = InitLineItem(item, "%l"); 
    }
  }

  return count;
}

int CtxGetDebugPrio(line_item_t **fill, local_ctx_t* ctx, param_t vctx){
  if(ctx->other.type_id != DATA_ENTITY)
    return 0;
 
  ent_t* e = ctx->other.data;

  if(!e->control || !e->control->priorities)
    return 0;

  int count = 0;
  int total = imin(MAX_SUB_ELE, e->control->priorities->count);
  for(int i = 0; i < total; i++){
    priority_t* p = &e->control->priorities->entries[i];

    if(p->score < 1 || p->prune)
      continue;

    element_value_t* prio[3];

    element_value_t* index = GameCalloc("CtxGetDebugPrio", 1,sizeof(element_value_t));

    index->type = VAL_INT;
    index->rate = FETCH_UPDATE;
    index->i = GameCalloc("CtxGetDebugPrio", 1, sizeof(int));
    *index->i = count;

    element_value_t* name = GameCalloc("CtxGetDebugPrio", 1,sizeof(element_value_t));

    name->type = VAL_CHAR;
    name->rate = FETCH_UPDATE;
    name->c = strdup(NEED_STRINGS[p->type]);

    element_value_t* score = GameCalloc("CtxGetDebugPrio", 1,sizeof(element_value_t));

    score->type = VAL_INT;
    score->rate = FETCH_UPDATE;
    score->i = GameCalloc("CtxGetDebugPrio", 1, sizeof(int));
    *score->i = p->score; 

    prio[0] = index;
    prio[1] = name; 
    prio[2] = score;
    
    line_item_t* line = InitLineItems(prio, 3, "Priority %i: %s score: %i");
    element_value_t* item = GameCalloc("CtxGetDebugPrio", 1,sizeof(element_value_t));

      item->index = count;
      item->type = VAL_LN;
      item->rate = FETCH_UPDATE;
      item->context = vctx;
      item->get_val = DebugGetPrio;
      item->l[0] = line;

     fill[count++] = InitLineItem(item, "%l");

  }

  return count;
}

element_value_t* DebugGetPrio(element_value_t* ev, param_t p){
  if(p.type_id != DATA_LOCAL_CTX)
    return ev;

  local_ctx_t* ctx = p.data;
  if(ctx->other.type_id != DATA_ENTITY)
    return ev;

  ent_t* e = ctx->other.data;
  if(ev->index > e->control->priorities->count){
    ev = NULL;
    return NULL;
  }

  priority_t* prio = &e->control->priorities->entries[ev->index];

  if(prio->score < 1 || prio->prune){
    ev = NULL;
    return NULL;
  }
  element_value_t* vals[3];

  element_value_t* index = GameCalloc("CtxGetDebugPrio", 1,sizeof(element_value_t));

  index->type = VAL_INT;
  index->rate = FETCH_UPDATE;
  index->i = GameCalloc("CtxGetDebugPrio", 1, sizeof(int));
  *index->i = ev->index;

  element_value_t* name = GameCalloc("CtxGetDebugPrio", 1,sizeof(element_value_t));

  name->type = VAL_CHAR;
  name->rate = FETCH_UPDATE;
  name->c = strdup(PRIO_STRINGS[prio->type]);

  element_value_t* score = GameCalloc("CtxGetDebugPrio", 1,sizeof(element_value_t));

  score->type = VAL_INT;
  score->rate = FETCH_UPDATE;
  score->i = GameCalloc("CtxGetDebugPrio", 1, sizeof(int));
  *score->i = prio->score;

  vals[0] = index;
  vals[1] = name; 
  vals[2] = score;

  ev->l[0]  = InitLineItems(vals, 3, "Priority %i: %s score: %i");  


  return ev;
}

element_value_t* DebugGetAction(element_value_t* ev, param_t p){
  if(p.type_id != DATA_LOCAL_CTX)
    return ev;

  ev->num_ln = CtxGetDebugAction(ev->l, p.data, p);

  return ev;

}



element_value_t* DebugGetState(element_value_t* ev, param_t p){
   local_ctx_t* ctx = p.data;

   if(ctx->other.type_id != DATA_ENTITY)
     return ev;

   ent_t* e = ctx->other.data;

   switch(ev->index){
     case 0:
      ev->c = strdup( STATE_STRING[e->state]);
      break;
     case 1:
      ev->c = strdup( STATE_STRING[e->previous]);
      break;
     case 2:
      ev->c = strdup( STATE_STRING[e->control->next]);
      break;
   }

   return ev;
}

int CtxGetDebugPos(line_item_t **fill, local_ctx_t* ctx, param_t vctx){
  
  element_value_t* x = GameCalloc("CtxGetString", 1,sizeof(element_value_t));
  element_value_t* y = GameCalloc("CtxGetString", 1,sizeof(element_value_t));

  x->type = VAL_INT;
  x->rate = FETCH_TURN;
  x->i = GameCalloc("CtxGetDebugPos", 1, sizeof(int));
  x->context = vctx;
  x->index = 0;
  *x->i = ctx->pos.x;

  y->type = VAL_INT;
  y->rate = FETCH_TURN;
  y->context = vctx;
  y->index = 1;

  y->i = GameCalloc("CtxGetDebugPos", 1, sizeof(int));
  *y->i = ctx->pos.y;
  element_value_t* pos[2];
  pos[0] = x;
  pos[1] = y; 
  fill[0] = InitLineItems(pos, 2, "Cell: [%i, %i]");

  return 1;
}



int CtxGetDebugState(line_item_t **fill, local_ctx_t* ctx, param_t vctx){
  EntityState curs, prevs, nexts;
  switch(ctx->other.type_id){
    case DATA_ENTITY:
      ent_t* e = ctx->other.data;
      curs = e->state;
      prevs = e->previous;
      nexts = e->control->next;
      break;
    default:
      return 0;
      break;
  }
  
  element_value_t* state = GameCalloc("CtxGetString", 1,sizeof(element_value_t));
  element_value_t* prev = GameCalloc("CtxGetString", 1,sizeof(element_value_t));
  element_value_t* next = GameCalloc("CtxGetString", 1,sizeof(element_value_t));

  state->type = VAL_CHAR;
  state->rate = FETCH_TURN;
  state->c = strdup( STATE_STRING[curs]);
  state->context = vctx;
  state->index = 0;

  prev->type = VAL_CHAR;
  prev->rate = FETCH_TURN;
  prev->c = strdup( STATE_STRING[prevs]);
  prev->context = vctx;
  prev->index = 1;

  next->type = VAL_CHAR;
  next->rate = FETCH_TURN;
  next->c = strdup( STATE_STRING[nexts]);
  next->context = vctx;
  next->index = 2;

  state->get_val = DebugGetState;
  prev->get_val = DebugGetState;
  next->get_val = DebugGetState;
  
  fill[0] = InitLineItem(state, "Current State: %s");
  fill[1] = InitLineItem(prev, "Previous State: %s");
  fill[2] = InitLineItem(state, "Next State:    %s");

  return 3;
}


element_value_t* DebugGetTarget(element_value_t* ev, param_t p){
  local_ctx_t* ctx = p.data;

  if(ctx->other.type_id != DATA_ENTITY)
   return NULL;

  ent_t* e = ctx->other.data;
  if(!e || !CheckEntAlive(e) || e->control->target.type_id == DATA_NONE)
    return NULL;

  local_ctx_t* tartx = e->control->target.data;

  if(!tartx)
    return NULL;

  switch(ev->type){
    case VAL_INT:
      int uid = -1;

      switch(tartx->other.type_id){
        case DATA_ENTITY:
          ent_t* tar = tartx->other.data;
          uid = tar->uid;
          break;
      }
      *ev->i = uid;
      break;
    case VAL_CHAR:
      strncpy(ev->c, ParamReadString(&tartx->params[PARAM_NAME]), MAX_NAME_LEN -1);

      break;
  }

}

element_value_t* DebugGetID(element_value_t* e, param_t p){
  local_ctx_t* ctx = p.data;
  switch(e->type){
    case VAL_INT:
      int uid = -1;

      switch(ctx->other.type_id){
        case DATA_ENTITY:
          ent_t* e = ctx->other.data;
          uid = e->uid;
          break;
      }
      *e->i = uid;
      break;
    case VAL_CHAR:
      strncpy(e->c, ParamReadString(&ctx->params[PARAM_NAME]), MAX_NAME_LEN -1);

      break;
  }

}

int CtxGetDebugAggro(line_item_t **fill, local_ctx_t* ctx, param_t vctx){

  if(ctx->other.type_id != DATA_ENTITY)
    return 0;

  ent_t* e = ctx->other.data;
  local_ctx_t* tartx = NULL;
  switch(e->control->target.type_id){
    case DATA_LOCAL_CTX:
      tartx = e->control->target.data;
      break;
    case DATA_NONE:
      return 0;
    default:
      return 0;
      break;
  }
  if(!tartx || tartx->other.type_id != DATA_ENTITY)
    return 0;


  element_value_t* txt = GameCalloc("CtxGetString", 1,sizeof(element_value_t));
  element_value_t* id = GameCalloc("CtxGetString", 1,sizeof(element_value_t));

  txt->type = VAL_CHAR;
  txt->get_val = DebugGetTarget;
  txt->context = e->control->target;


  id->type = VAL_INT;
  id->get_val = DebugGetTarget;

  id->context = e->control->target;

  int uid = -1;

  switch(tartx->other.type_id){
    case DATA_ENTITY:
      ent_t* e = ctx->other.data;
      uid = e->uid;
      break;
  }

  id->i = GameCalloc("CtxGetDebug", 1, sizeof(int));
  *id->i = uid;
  txt->c = GameCalloc("CtxGetDebug", MAX_NAME_LEN, sizeof(char));
  strncpy(txt->c, ParamReadString(&tartx->params[PARAM_NAME]), MAX_NAME_LEN -1);
  txt->c[MAX_NAME_LEN-1] = '\0';
  txt->rate = FETCH_UPDATE;
  id->rate = FETCH_UPDATE;

  element_value_t* debid[2];
  debid[0] = txt;
  debid[1] = id;
  fill[0] = InitLineItems(debid, 2, "Target: %s - %i");

  return 1;
}


int CtxGetDebugID(line_item_t **fill, local_ctx_t* ctx, param_t vctx){

  element_value_t* txt = GameCalloc("CtxGetString", 1,sizeof(element_value_t));
  element_value_t* id = GameCalloc("CtxGetString", 1,sizeof(element_value_t));

  txt->type = VAL_CHAR;
  txt->get_val = DebugGetID;
  txt->context = vctx;


  id->type = VAL_INT;
  id->get_val = DebugGetID;
  id->context = vctx;


  int uid = -1;

  switch(ctx->other.type_id){
    case DATA_ENTITY:
      ent_t* e = ctx->other.data;
      WorldSetDebug(e);
      uid = e->uid;
      break;
  }

  id->i = GameCalloc("CtxGetDebug", 1, sizeof(int));
  *id->i = uid;
  txt->c = GameCalloc("CtxGetDebug", MAX_NAME_LEN, sizeof(char));
  strncpy(txt->c, ParamReadString(&ctx->params[PARAM_NAME]), MAX_NAME_LEN -1);
  txt->c[MAX_NAME_LEN-1] = '\0';
  txt->rate = FETCH_UPDATE;
  id->rate = FETCH_UPDATE;

  element_value_t* debid[2];
  debid[0] = txt;
  debid[1] = id;
  fill[0] = InitLineItems(debid, 2, "%s - %i");

  return 1;
}


line_item_t* InitLineItems(element_value_t **val, int num_val, const char* format){
  line_item_t* ln = GameCalloc("InitLineItems", 1,sizeof(line_item_t));

  for (int i = 0; i < num_val; i++)
    ln->values[ln->num_val++]=val[i];

  ln->text_format = strdup(format);

  return ln;
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
          case 'l':
            if(val->type == VAL_LN)
              strcat(buffer, TextFormatLineItem(val->l[0]));
            else
              strcat(buffer, "<BAD LINE>");
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

int GetMaterialDesc(line_item_t** li, item_t* item){
  element_value_t* name = GameCalloc("GetMaterialDesc", 1,sizeof(element_value_t));
  element_value_t* base[1];

  int count = 0;

  name->type = VAL_CHAR;
  name->rate = FETCH_ONCE;
  name->c = strdup(item->def->name);
  name->get_val = NULL;
  base[0] = name;
  li[count++] = InitLineItems(base, 1, "%s");

  element_value_t* quant[1];
  element_value_t* weight[1];
  element_value_t* worth[2];

  int amnt = item->values[VAL_QUANT]->val;
  int mass = item->values[VAL_WEIGHT]->val;
  int value = item->values[VAL_WORTH]->val * amnt;

  element_value_t* amount = GameCalloc("GetMaterialDesc", 1,sizeof(element_value_t));

  amount->type = VAL_INT;
  amount->rate = FETCH_TURN;
  amount->i = GameMalloc("GetMaterialDesc", sizeof(int));
  *amount->i = amnt;

  element_value_t* kg = GameCalloc("GetMaterialDesc", 1,sizeof(element_value_t));

  kg->type = VAL_FLOAT;
  kg->rate = FETCH_TURN;
  kg->f = GameMalloc("GetMaterialDesc", sizeof(float));
  *kg->f = mass/1000;

  element_value_t* gold = GameCalloc("GetMaterialDesc", 1,sizeof(element_value_t));
  element_value_t* silver = GameCalloc("GetMaterialDesc", 1,sizeof(element_value_t));

  gold->type = VAL_INT;
  gold->rate = FETCH_TURN;
  gold->i = GameMalloc("GetMaterialDesc", sizeof(int));
  *gold->i = value/100;


  silver->type = VAL_INT;
  silver->rate = FETCH_TURN;
  silver->i = GameMalloc("GetMaterialDesc", sizeof(int));
  *silver->i = value%100;

  quant[0] = amount;
  weight[0] = kg;
  worth[0] = gold;
  worth[1] = silver;

  li[count++] = InitLineItems(quant, 1, "Quantity: %i");
  li[count++] = InitLineItems(weight, 1, "Weight: %f kg");
  li[count++] = InitLineItems(worth, 2, "Worth: %i G / %i S");

  return count;
  //worth[1] = silver;
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
  li[count++] = InitLineItems(base, 1, "%s");
  
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

  element_value_t* type = GameCalloc("GetAbilityDesc",1,sizeof(element_value_t));

  type->type = VAL_CHAR;
  type->rate = FETCH_ONCE;
  type->c = strdup(SKILL_NAMES[a->skills[0]]);
 
  element_value_t* dmg = GameCalloc("GetAbilityDesc", 1,sizeof(element_value_t));

  dmg->type = VAL_CHAR;
  dmg->rate = FETCH_ONCE;
  dmg->c = strdup(DAMAGE_STRING[a->school]);
 element_value_t* min_dmg = GameCalloc("GetAbilityDesc", 1,sizeof(element_value_t));
  min_dmg->i = GameMalloc("GetAbilityDesc",sizeof(int));
  element_value_t* max_dmg = GameCalloc("GetAbilityDesc", 1,sizeof(element_value_t));


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
  *cost->i = a->values[VAL_DRAIN]->val;
  
  element_value_t* resource = GameCalloc("GetAbilityDesc", 1,sizeof(element_value_t));

  resource->type = VAL_CHAR;
  resource->rate = FETCH_ONCE;
  resource->c = strdup(STAT_STRING[a->resource].name);

  costs[0] = cost;
  costs[1] = resource;
  li[count++] = InitLineItems(base, 1, "%s");

  char *action_str = strdup(ACTION_STRINGS[a->action]);
  
  if(action_str && action_str[0] != '\0')
    li[count++] = InitLineItems(school, 1, action_str);

  li[count++] = InitLineItems(costs, 2, "Costs %i %s");
  li[count++] = InitLineItems(vals, 4, "Deals %i - %i %s damage %s.");
  li[count++] = InitLineItems(ranges, 1, "Range: %i tiles");

    return count;
}

int GetToolDesc(line_item_t** li, item_t* item){

  element_value_t* name = GameCalloc("GetToolDesc", 1,sizeof(element_value_t));
  element_value_t* base[MAX_LINE_VAL];

  tool_def_t* tdef = item->def->type_def;
  int count = 0;
  name->type = VAL_CHAR;
  name->rate = FETCH_ONCE;
  name->c = strdup(item->def->name);

  base[0] = name;
  li[count++] = InitLineItems(base, 1, "%s");

  element_value_t* min_dmg = GameCalloc("GetToolDesc", 1,sizeof(element_value_t));
  min_dmg->i = malloc(sizeof(int));
  element_value_t* max_dmg = GameCalloc("GetToolDesc", 1,sizeof(element_value_t));

  max_dmg->i = malloc(sizeof(int));
  min_dmg->type = VAL_INT;
  min_dmg->rate = FETCH_ONCE;
  int min_roll = 1 * item->ability->dc->num_die;
  *min_dmg->i =  min_roll;

  int max_roll = item->ability->dc->sides * item->ability->dc->num_die;

  max_dmg->type = VAL_INT;
  max_dmg->rate = FETCH_ONCE;
  *max_dmg->i = max_roll;

  element_value_t* type = GameCalloc("GetToolDesc",1,sizeof(element_value_t));
  type->type = VAL_CHAR;
  type->rate = FETCH_ONCE;
  type->c = strdup(ABILITY_STRINGS[tdef->use].target);
  
  element_value_t* dmg[3];
  element_value_t* hit[3];
  
  dmg[0] = min_dmg;
  dmg[1] = max_dmg;
  dmg[2] = type;

  element_value_t* duri[2];

  element_value_t* base_d = GameCalloc("GetToolDesc",1,sizeof(element_value_t));

  element_value_t* cur_d = GameCalloc("GetToolDesc",1,sizeof(element_value_t));

  base_d->type = VAL_INT;
  base_d->rate  = FETCH_TURN;
  base_d->i = GameCalloc("GetToolDesc",1,sizeof(int));

  cur_d->type = VAL_INT;
  cur_d->rate  = FETCH_TURN;
  cur_d->i = GameCalloc("GetToolDesc",1,sizeof(int));

  *cur_d->i = item->values[VAL_DURI]->val;
  *base_d->i = item->values[VAL_DURI]->base;
  duri[0] = cur_d;
  duri[1] = base_d;
  li[count++] = InitLineItems(dmg, 3, "Extracts %i to %i %s");
  li[count++] = InitLineItems(duri, 2, "Duribility %i out of %i");
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
  li[count++] = InitLineItems(base, 1, "%s");

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
  
  element_value_t* dmg[3];
  element_value_t* hit[3];
  
  dmg[0] = min_dmg;
  dmg[1] = max_dmg;
  dmg[2] = type;

  element_value_t* duri[2];

  element_value_t* base_d = GameCalloc("GetWeapDesc",1,sizeof(element_value_t));

  element_value_t* cur_d = GameCalloc("GetWeapDesc",1,sizeof(element_value_t));

  base_d->type = VAL_INT;
  base_d->rate  = FETCH_TURN;
  base_d->i = GameCalloc("GetWeapDesc",1,sizeof(int));

  cur_d->type = VAL_INT;
  cur_d->rate  = FETCH_TURN;
  cur_d->i = GameCalloc("GetWeapDesc",1,sizeof(int));

  *cur_d->i = item->values[VAL_DURI]->val;
  *base_d->i = item->values[VAL_DURI]->base;
  duri[0] = cur_d;
  duri[1] = base_d;
  li[count++] = InitLineItems(dmg, 3, "Deals %i to %i %s Damage");
  li[count++] = InitLineItems(duri, 2, "Duribility %i out of %i");
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
  li[count++] = InitLineItems(base, 1, "%s");

  
  element_value_t* ac = GameCalloc("GetArmorDesc", 1,sizeof(element_value_t));

  ac->type = VAL_INT;
  ac->rate = FETCH_ONCE;
  ac->i = GameMalloc("GetArmorDesc", sizeof(int));

  
  *ac->i = item->values[VAL_SAVE]->val;
  element_value_t* base_ac[1];
 
  base_ac[0] = ac;
  li[count++] = InitLineItems(base_ac, 1, "Armor Class: %i");

 
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
    li[count++] = InitLineItems(fill, 2, "%s Damage Reduction: %i");
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

int CtxGetStatRelated(element_value_t **fill, local_ctx_t* ctx, GameObjectParam p){


}

int CtxGetStatDetails(element_value_t **fill, local_ctx_t* ctx, GameObjectParam p){
  if(ctx->params[p].type_id != DATA_STAT)
    return 0;

  stat_t* stat = ParamRead(&ctx->params[p], stat_t);
 
 /* 
  if(stat->related != STAT_NONE){
    return CtxGetStatRelated(fill, ctx, p);
  }
  */
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

  self->l[self->num_ln++] = InitLineItems(val, 1, "Level %i");

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
      self->p->right = InitLineItems(right, 2, "%i / %i");
 
      lvl->type = VAL_INT;
      lvl->rate = FETCH_EVENT;
      lvl->context = context;
      lvl->context.gouid = 0;
      lvl->get_val = SkillGetValues;
      lvl->i = GameMalloc("SkillGetPretty", sizeof(int));
      *lvl->i = (int)skill->val;

      left[0] = lvl;
      self->p->left = InitLineItems(left, 1, "Level: %i");
      
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

    li[count] = InitLineItems(base,items,format);
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
        case ITEM_MATERIAL:
          lines = GetMaterialDesc(li, item);
          break;
        case ITEM_TOOL:
          lines = GetToolDesc(li, item);
          break;
      }
      break;
    case DATA_ABILITY:
      ability_t *a = ParamRead(&ctx, ability_t);
      switch(a->action){
        case ACTION_MAGIC:
        case ACTION_WEAPON:
        lines = GetAbilityDesc(li, a);
        break;
      }
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
  ev->l[0] = InitLineItems(base, items, fmt);

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

      li[count] = InitLineItems(base,lines,"%s: %i");
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

int SetCtxDebug(local_ctx_t* ctx, param_t vctx, line_item_t** li, GameObjectParam p){

  int lines = 1;
  element_value_t* base[MAX_LINE_ITEMS];

  char* fmt;
  switch(p){
    case PARAM_NAME:
      lines = CtxGetDebugID(li, ctx, vctx);
      break;
    case PARAM_STATE:
      lines = CtxGetDebugState(li, ctx, vctx);
      break;
    case PARAM_PRIO:
      lines = CtxGetDebugPrio(li, ctx, vctx);
      break;
    case PARAM_ACTION:
      lines = CtxGetDebugAction(li, ctx, vctx);
      break;
    case PARAM_AGGRO:
      lines = CtxGetDebugAggro(li, ctx, vctx);
      break;
    case PARAM_POS:
      lines = CtxGetDebugPos(li, ctx, vctx);
      break;
  }
    
  for (int i = 0; i < lines; i++){
    PrintSyncLine(li[i],FETCH_ONCE);
    const char* ln = TextFormatLineItem(li[i]);
    TraceLog(LOG_INFO, "MEASURE: %s", ln);

    Vector2 size = MeasureTextEx(ui.font, ln, ui.text_size, ui.text_spacing);
    li[i]->r_wid = size.x;
    li[i]->r_hei = size.y;

  }
  return lines;
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
        
    li[count] = InitLineItems(base,items,fmt[i]);
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
            snprintf(tmp, sizeof(tmp), "%f", ParamReadFloat(p));
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

