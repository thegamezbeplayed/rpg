#include <raylib.h>
#include "game_tools.h"
#include "game_process.h"

category_stats_t CATEGORY_STATS[MOB_DONE] = {
  {MOB_NONE},
  {MOB_HUMANOID, 
    {[STAT_HEALTH]=10, [STAT_AGGRO]=10,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 3, [ATTR_STR]=3, [ATTR_DEX]=3, [ATTR_INT]=3,[ATTR_WIS]=3,[ATTR_CHAR]=3}
  },
  {MOB_MONSTROUS,     
    {[STAT_HEALTH]=10, [STAT_AGGRO]=10,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 5, [ATTR_STR]=5, [ATTR_DEX]=3, [ATTR_INT]=3,[ATTR_WIS]=2,[ATTR_CHAR]=2}
  },
  {MOB_BEAST,
    {[STAT_HEALTH]=10, [STAT_AGGRO]=10,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 4, [ATTR_STR]=4, [ATTR_DEX]=4, [ATTR_INT]=3,[ATTR_WIS]=1,[ATTR_CHAR]=1}
  },
  {MOB_UNDEAD, 
    {[STAT_HEALTH]=10, [STAT_AGGRO]=10,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 4, [ATTR_STR]=5, [ATTR_DEX]=3, [ATTR_INT]=1,[ATTR_WIS]=1,[ATTR_CHAR]=1}
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
    {[STAT_HEALTH]=10, [STAT_AGGRO]=10,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 3, [ATTR_STR]=2, [ATTR_DEX]=2, [ATTR_INT]=3,[ATTR_WIS]=2,[ATTR_CHAR]=1}
  },
  {MOB_PLAYER, 
    {[STAT_HEALTH]=10, [STAT_AGGRO]=10,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 0, [ATTR_STR]=0, [ATTR_DEX]=0, [ATTR_INT]=0,[ATTR_WIS]=0,[ATTR_CHAR]=0}
  },
};

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

void FormulaDieAddAttr(stat_t* self){
  
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
  }; return s;
}

stat_t* InitStatOnMax(StatType attr, float val){
 stat_t* s = malloc(sizeof(stat_t));
 *s =(stat_t){
    .attribute = attr,
      .min = 0,
      .max = val,
      .current = val,
      .ratio = StatGetRatio,
      .increment = 1
  }; return s;

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
