#include <raylib.h>
#include "game_tools.h"
#include "game_process.h"


item_fn_t item_funcs[ITEM_DONE] = {
  {ITEM_NONE},
  {ITEM_WEAPON,.on_equip=ItemAddAttack},
  {ITEM_ARMOR,.on_equip=ItemApplyStats},
  {ITEM_DONE}
};

category_stats_t CATEGORY_STATS[MOB_DONE] = {
  {MOB_NONE},
  {MOB_HUMANOID, 
    {[STAT_HEALTH]=9, [STAT_ARMOR]=1,[STAT_AGGRO]=10,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 3, [ATTR_STR]=3, [ATTR_DEX]=3, [ATTR_INT]=3,[ATTR_WIS]=3,[ATTR_CHAR]=3}
  },
  {MOB_MONSTROUS,     
    {[STAT_HEALTH]=10, [STAT_AGGRO]=10,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 5, [ATTR_STR]=5, [ATTR_DEX]=3, [ATTR_INT]=3,[ATTR_WIS]=2,[ATTR_CHAR]=2}
  },
  {MOB_BEAST,
    {[STAT_HEALTH]=10, [STAT_AGGRO]=12,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 4, [ATTR_STR]=4, [ATTR_DEX]=4, [ATTR_INT]=3,[ATTR_WIS]=1,[ATTR_CHAR]=1}
  },
  {MOB_UNDEAD, 
    {[STAT_HEALTH]=10, [STAT_AGGRO]=10,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 4, [ATTR_STR]=5, [ATTR_DEX]=3, [ATTR_INT]=2,[ATTR_WIS]=1,[ATTR_CHAR]=1}
  },
  {MOB_CONSTRUCT, 
    {[STAT_HEALTH]=10, [STAT_AGGRO]=10,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 4, [ATTR_STR]=5, [ATTR_DEX]=4, [ATTR_INT]=0,[ATTR_WIS]=0,[ATTR_CHAR]=0}
  },
  {MOB_DEMONIC, 
    {[STAT_HEALTH]=10, [STAT_AGGRO]=10,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 4, [ATTR_STR]=4, [ATTR_DEX]=3, [ATTR_INT]=3,[ATTR_WIS]=3,[ATTR_CHAR]=4}
  },
  {MOB_FEY, 
    {[STAT_HEALTH]=10, [STAT_AGGRO]=10,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 4, [ATTR_STR]=4, [ATTR_DEX]=4, [ATTR_INT]=3,[ATTR_WIS]=1,[ATTR_CHAR]=1}
  },
  {MOB_CIVILIAN, 
    {[STAT_HEALTH]=10, [STAT_AGGRO]=9,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 3, [ATTR_STR]=2, [ATTR_DEX]=2, [ATTR_INT]=3,[ATTR_WIS]=2,[ATTR_CHAR]=1}
  },
  {MOB_PLAYER, 
    {[STAT_HEALTH]=15,[STAT_ARMOR]=2, [STAT_AGGRO]=10,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 6, [ATTR_STR]=6, [ATTR_DEX]=6, [ATTR_INT]=6,[ATTR_WIS]=6,[ATTR_CHAR]=6}
  },
};
species_stats_t RACIALS[SPEC_DONE]={
  {SPEC_NONE},
  {SPEC_HUMAN, {},
    {[ATTR_CON]=1,[ATTR_STR]=1,[ATTR_DEX]=1,[ATTR_INT]=1,[ATTR_WIS]=1,[ATTR_CHAR]=1}},
  {SPEC_GREENSKIN, {[STAT_HEALTH]=1, [STAT_ARMOR]=2},
    {[ATTR_CON]=1,[ATTR_STR]=2,[ATTR_INT]=-1,[ATTR_WIS]=-1}},
  {SPEC_ETHEREAL,{},
    {[ATTR_STR] -2, [ATTR_DEX]=3}},
  {SPEC_ROTTING, {},
    {[ATTR_INT]=-1, [ATTR_DEX]=-1}},
  {SPEC_VAMPIRIC, {[STAT_HEALTH]=2},
    {[ATTR_DEX]=2,[ATTR_CHAR]=3}}
  };

static dice_roll_t DTEN = {6,1,RollDie};

dice_roll_t* Die(int side, int num){
  dice_roll_t* die = malloc(sizeof(dice_roll_t));

  *die = (dice_roll_t){
    side,num,RollDie};

  return die;
}

void UploadScore(void){
TraceLog(LOG_INFO,"Call EMSCRIPTEN");
#ifdef __EMSCRIPTEN__
  
TraceLog(LOG_INFO,"Call JS");
EM_ASM({
      sendScore($0);
      }, GetPointsInt());
#endif
}
int RollDie(dice_roll_t* d){
  int results[d->num_die];
  int sum = 0;

  for(int i = 0; i < d->num_die; i++){
    int roll = (rand()% d->sides) + 1;
    sum+=roll;
    results[i]=roll;
  }

  return sum;
}

void FormulaDie(stat_t* self){

}

attribute_t* InitAttribute(AttributeType type, int val){
  attribute_t* t = malloc(sizeof(attribute_t));

  t->max = t->min = t->val = val;

  return t;
}

// Allocates a copy of the filename without extension
char* GetFileStem(const char* filename) {
    const char* dot = strrchr(filename, '.');
    size_t len = dot ? (size_t)(dot - filename) : strlen(filename);

    char* stem = malloc(len + 1);
    if (!stem) return NULL;
    memcpy(stem, filename, len);
    stem[len] = '\0';
    return stem;
}

stat_t* InitStatOnMin(StatType attr, float min, float max){
 stat_t* s = malloc(sizeof(stat_t));
 *s =(stat_t){
    .attribute = attr,
      .min = min,
      .max = max,
      .current = min,
      .ratio = StatGetRatio,
      .increment = 1.0f
  };

 return s;
}

stat_t* InitStatOnMax(StatType attr, float val){
 stat_t* s = malloc(sizeof(stat_t));
 stat_attribute_relation_t relate = stat_modifiers[attr];
 *s =(stat_t){
    .attribute = attr,
      .min = 0,
      .max = val,
      .current = val,
      .ratio = StatGetRatio,
      .increment = 1,
      .die = Die(1,val),
      .start = FormulaDieAddAttr
  };

 for(int i = 0; i < ATTR_DONE; i++)
   s->modified_by[i] = relate.modifier[i];

 return s;

}

bool StatIsEmpty(stat_t* s){
  return s->current <= s->min;
}

stat_t* InitStatEmpty(void){}
stat_t* InitStat(StatType attr,float min, float max, float amount){
 stat_t* s = malloc(sizeof(stat_t));
 *s =(stat_t){
    .attribute = attr,
      .min = min,
      .max = max,
      .current = amount,
      .ratio = StatGetRatio,
      .increment = 1.0f
  }; return s;

}

bool StatExpand(stat_t* s, int val, bool fill){
  s->max+= val;
  if(fill)
    StatMaxOut(s);

  return true;
}

bool StatIncrementValue(stat_t* attr,bool increase){
  float inc = attr->increment;
  if(!increase)
    inc*=-1;

  float old = attr->current;
  attr->current+=inc;
  attr->current = CLAMPF(attr->current,attr->min, attr->max);
  float cur = attr->current;
  return true;
}

bool StatChangeValue(struct ent_s* owner, stat_t* attr, float val){
  float old = attr->current;
  attr->current+=val;
  attr->current = CLAMPF(attr->current,attr->min, attr->max);
  float cur = attr->current;
  if(attr->current == old) 
    return false;

  if(attr->on_stat_change != NULL)
    attr->on_stat_change(owner,old, cur);
  
  if(attr->current == attr->min && attr->on_stat_empty!=NULL)
    attr->on_stat_empty(owner);

  return true;
}

void StatMaxOut(stat_t* s){
  s->current = s->max;

}

void StatEmpty(stat_t* s){
  s->current = s->min;
}

float StatGetRatio(stat_t *self){}

void FormulaDieAddAttr(stat_t* self){
  int modifier = 0;
  for(int i = 0; i < ATTR_DONE;i++){
    if(self->modified_by[i])
      modifier += isqrt(self->owner->attribs[i]->val); 
  }
  self->base += self->die->roll(self->die);

  self->max=self->base+modifier;

}
