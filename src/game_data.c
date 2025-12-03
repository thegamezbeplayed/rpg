#include <raylib.h>
#include "game_tools.h"
#include "game_process.h"

ability_t ABILITIES[ABILITY_DONE]={
  {ABILITY_NONE},
  {ABILITY_BITE, DMG_PIERCE,STAT_STAMINA, DES_NONE, 25,1, 4, 1, 4,4, 1, STAT_HEALTH, ATTR_NONE, ATTR_STR},
  {ABILITY_CHEW, DMG_PIERCE,STAT_STAMINA, DES_NONE, 25,1, 4, 1, 2,0, 1, STAT_ARMOR, ATTR_NONE, ATTR_NONE, ABILITY_GNAW},
  {ABILITY_GNAW, DMG_PIERCE,STAT_STAMINA, DES_NONE, 25,1, 4, 1, 2,0, 1, STAT_HEALTH, ATTR_NONE, ATTR_STR},
  {ABILITY_CLAW, DMG_SLASH, STAT_STAMINA, DES_NONE, 50,2, 6, 2, 3, 1, 1, STAT_HEALTH, ATTR_NONE, ATTR_STR},
  {ABILITY_SWIPE, DMG_SLASH,STAT_STAMINA, DES_NONE, 50,2, 6, 2, 6, 3,1, STAT_HEALTH, ATTR_STR},
  {ABILITY_BITE_POISON, DMG_PIERCE, STAT_STAMINA, DES_NONE, 25,1, 4, 1, 2, 0,1, STAT_HEALTH, ATTR_NONE, ATTR_STR,ABILITY_POISON},
  {ABILITY_POISON, DMG_POISON, STAT_NONE, DES_NONE, 25,1, 9, 1, 3,0,1,STAT_HEALTH, ATTR_CON, ATTR_NONE},
  {ABILITY_MAGIC_MISSLE ,DMG_FORCE, STAT_ENERGY, DES_SELECT_TARGET, 20,4,99,1,4,1,3,STAT_HEALTH, ATTR_NONE, ATTR_NONE},
};

item_fn_t item_funcs[ITEM_DONE] = {
  {ITEM_NONE},
  {ITEM_WEAPON,.on_equip=ItemAddAbility},
  {ITEM_ARMOR,.on_equip=ItemApplyStats},
  {ITEM_DONE}
};

category_stats_t CATEGORY_STATS[MOB_DONE] = {
  {MOB_HUMANOID, 
    {[STAT_HEALTH]=8, [STAT_REACH]=1,[STAT_ARMOR]=0,[STAT_AGGRO]=4,[STAT_ACTIONS]= 1,[STAT_STAMINA]=4},
    {[ATTR_CON]= 4, [ATTR_STR]=4, [ATTR_DEX]=3, [ATTR_INT]=4,[ATTR_WIS]=3,[ATTR_CHAR]=3}
  },
  {MOB_MONSTROUS,     
    {[STAT_HEALTH]=10, [STAT_REACH]=1,[STAT_AGGRO]=4,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 5, [ATTR_STR]=5, [ATTR_DEX]=4, [ATTR_INT]=3,[ATTR_WIS]=2,[ATTR_CHAR]=2}
  },
  {MOB_BEAST,
    {[STAT_REACH]=1,[STAT_HEALTH]=10, [STAT_AGGRO]=5,[STAT_ACTIONS]= 1,[STAT_STAMINA]=10},
    {[ATTR_CON]= 4, [ATTR_STR]=4, [ATTR_DEX]=5, [ATTR_INT]=3,[ATTR_WIS]=1,[ATTR_CHAR]=1}
  },
  {MOB_UNDEAD, 
    {[STAT_REACH]=1,[STAT_HEALTH]=10, [STAT_AGGRO]=3,[STAT_ACTIONS]= 1,[STAT_STAMINA]=20},
    {[ATTR_CON]= 4, [ATTR_STR]=5, [ATTR_DEX]=3, [ATTR_INT]=2,[ATTR_WIS]=1,[ATTR_CHAR]=1}
  },
  {MOB_CONSTRUCT, 
    {[STAT_REACH]=1,[STAT_HEALTH]=10, [STAT_AGGRO]=4,[STAT_ACTIONS]= 1, [STAT_STAMINA]=25},
    {[ATTR_CON]= 4, [ATTR_STR]=5, [ATTR_DEX]=4, [ATTR_INT]=0,[ATTR_WIS]=0,[ATTR_CHAR]=0}
  },
  {MOB_DEMONIC, 
    {[STAT_REACH]=1,[STAT_HEALTH]=10, [STAT_AGGRO]=4,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 4, [ATTR_STR]=4, [ATTR_DEX]=3, [ATTR_INT]=3,[ATTR_WIS]=3,[ATTR_CHAR]=4}
  },
  {MOB_FEY, 
    {[STAT_REACH]=1,[STAT_HEALTH]=10, [STAT_AGGRO]=6,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 3, [ATTR_STR]=4, [ATTR_DEX]=5, [ATTR_INT]=3,[ATTR_WIS]=4,[ATTR_CHAR]=3},
    TRAIT_MAGIC_RESIST|TRAIT_PSYCHIC_RESIST,
  },
  {MOB_CIVILIAN, 
    {[STAT_REACH]=1,[STAT_HEALTH]=10, [STAT_AGGRO]=2,[STAT_ACTIONS]= 1},
    {[ATTR_CON]= 3, [ATTR_STR]=2, [ATTR_DEX]=2, [ATTR_INT]=3,[ATTR_WIS]=2,[ATTR_CHAR]=1}
  },
  {MOB_PLAYER, 
    {[STAT_HEALTH]=15,[STAT_ARMOR]=2, [STAT_AGGRO]=10,[STAT_ACTIONS]= 1, [STAT_STAMINA] = 4, [STAT_ENERGY] = 4, [STAT_STAMINA_REGEN] = 1, [STAT_ENERGY_REGEN] = 1,[STAT_STAMINA_REGEN_RATE] = 10, [STAT_ENERGY_REGEN_RATE] = 15,},
    {[ATTR_CON]= 0, [ATTR_STR]=0, [ATTR_DEX]=0, [ATTR_INT]=0,[ATTR_WIS]=0,[ATTR_CHAR]=0}
  },
};
species_stats_t RACIALS[SPEC_DONE]={
  {SPEC_NONE},
  {SPEC_HUMAN, {},
    {[ATTR_CON]=1,[ATTR_STR]=1,[ATTR_DEX]=1,[ATTR_INT]=1,[ATTR_WIS]=1,[ATTR_CHAR]=1}},
  {SPEC_GREENSKIN, {[STAT_ARMOR]=1},
    {[ATTR_CON]=1,[ATTR_STR]=1,[ATTR_INT]=-1,[ATTR_WIS]=-1},
    TRAIT_POISON_RESIST
  },
  {SPEC_ARTHROPOD, {[STAT_HEALTH]=-3,[STAT_ARMOR]=3},
    {[ATTR_DEX]=2},
    TRAIT_POISON_RESIST
  },
  {SPEC_ETHEREAL,{},
    {[ATTR_STR] -2, [ATTR_DEX]=3}},
  {SPEC_ROTTING, {},
    {[ATTR_INT]=-1, [ATTR_DEX]=-1}},
  {SPEC_VAMPIRIC, {[STAT_HEALTH]=2},
    {[ATTR_DEX]=2,[ATTR_CHAR]=3}},
  {SPEC_CANIFORM, {[STAT_HEALTH]=1,[STAT_AGGRO]=2,},
    {[ATTR_STR]=1,[ATTR_DEX]=1}
  },
  {SPEC_RODENT, {[STAT_STEALTH]=1,[STAT_AGGRO]=-1},
    {[ATTR_DEX]=1}},
};
weapon_def_t WEAPON_TEMPLATES[WEAP_DONE]= {
  {WEAP_NONE},
  {WEAP_MACE,5,1,1,8,DMG_BLUNT,{[STAT_DAMAGE]=8,[STAT_REACH]=1,[STAT_STAMINA]=1},{},0},
  {WEAP_SWORD},
  {WEAP_AXE,7,1,1,8,DMG_SLASH,{[STAT_DAMAGE]=8,[STAT_REACH]=1,[STAT_STAMINA]=1},{},0},
  {WEAP_DAGGER},
  {WEAP_BOW,25,2,1,6,DMG_PIERCE,
    {[STAT_DAMAGE]=8,[STAT_REACH]=4,[STAT_STAMINA]=1},
    {[PROP_AMMO]=true,[PROP_TWO_HANDED]=true}},
};


armor_def_t ARMOR_TEMPLATES[ARMOR_DONE]={
  {ARMOR_NONE},
  {ARMOR_NATURAL},
  {ARMOR_PADDED, 8},
  {ARMOR_LEATHER, 10, 
    {{[DMG_SLASH]=1,[DMG_BLUNT]=1},{}},
    {{[DMG_SLASH]=1,[DMG_PIERCE]=1},{}},
    8,10,ATTR_DEX,ATTR_NONE,10,0,{}},
  {ARMOR_CHAIN, 12, 
    {{[DMG_SLASH]=2,[DMG_PIERCE]=1},{}},
    {{[DMG_SLASH]=1,[DMG_PIERCE]=1},{}},
    16,25,ATTR_DEX,ATTR_STR,2,9,{}},
  {ARMOR_PLATE, 16,
    {{[DMG_SLASH]=2,[DMG_PIERCE]=2,[DMG_BLUNT]=1},{}},
    {{[DMG_SLASH]=1,[DMG_PIERCE]=1},{}},
    32,100,ATTR_NONE,ATTR_STR,0,12,{}},
};

static const challenge_rating_t CR[34]={
  {0,10,3},
  {0.125,25,4},
  {0.25,50,5},
  {0.5,100,6},
  {1,200,7},
  {2,450,8},
  {3,700,9},
  {4,1100,10}, 
  {5,1800,11}, 
  {6,2300,12},
  {7,2900,13},
  {8,3900,14},
  {9,5000,15},
  {10,5900,16},
  {11,7200,17},
  {12,8400,18},
  {13,10000,19},
  {14,11500,20},
  {15,13000,21},
  {16,15000,21}, 
  {17,18000,21}, 
  {18,20000,21}, 
  {19,22000,21}, 
  {20,25000,21}, 
  {21,33000,21}, 
  {22,41000,21}, 
  {23,50000,21}, 
  {24,62000,21}, 
  {25,75000,21}, 
  {26,90000,21}, 
  {27,105000,21}, 
  {28,120000,21}, 
  {29,135000,21}, 
  {30,155000,21}, 
};

challenge_rating_t GetChallengeScore(float cr){
  
  if(cr == 0)
    return CR[(int)cr];
  else if (cr < 1){
    if(cr <0.25)
      return CR[1];
    else if(cr < .5)
      return CR[2];
    else
      return CR[3];
  }
  else{
    int index = cr+3;
    return CR[index];
  }
}

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
 stat_t* s = calloc(1,sizeof(stat_t));
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

stat_t* InitStatOnMax(StatType attr, float val, AttributeType modified_by){
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
      .start = FormulaDieAddAttr,
      .lvl = FormulaDieAddAttr
  };

 if(modified_by > ATTR_NONE)
   s->modified_by[modified_by] = relate.modifier[modified_by];

 return s;

}

bool StatIsEmpty(stat_t* s){
  return s->current <= s->min;
}

stat_t* InitStatEmpty(void){}
stat_t* InitStat(StatType attr,float min, float max, float amount){
 stat_t* s = malloc(sizeof(stat_t));
 stat_attribute_relation_t relate = stat_modifiers[attr];

 *s =(stat_t){
    .attribute = attr,
      .min = min,
      .max = max,
      .current = amount,
      .ratio = StatGetRatio,
      .increment = 1.0f,
      .die = Die(1,amount),
      .start = FormulaDieAddAttr,
      .lvl = FormulaDieAddAttr
 }; 
 for(int i = 0; i < ATTR_DONE; i++)
   s->modified_by[i] = relate.modifier[i];

 return s;

}

bool StatExpand(stat_t* s, int val, bool fill){
  s->max+= val;
  if(fill)
    StatMaxOut(s);

  return true;
}
void StatIncreaseValue(stat_t* self, float old, float cur){
  StatChangeValue(self->owner, self, self->increment);
}

bool StatIncrementValue(stat_t* attr,bool increase){
  float inc = attr->increment;
  if(!increase)
    inc*=-1;

  float old = attr->current;
  if(old+inc<attr->min)
    return false;

  attr->current+=inc;
  attr->current = CLAMPF(attr->current,attr->min, attr->max);
  float cur = attr->current;

  if(attr->current == attr->max && old != attr->max)
    if(attr->on_stat_full)
      attr->on_stat_full(attr,old,cur);

  if(StatIsEmpty(attr)&& attr->on_stat_empty)
    attr->on_stat_empty(attr,old,cur);

  if(attr->current != old)
    if(attr->on_stat_change != NULL)
      attr->on_stat_change(attr,old, cur);

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
    attr->on_stat_change(attr,old, cur);
  
  if(attr->current == attr->min && attr->on_stat_empty!=NULL)
    attr->on_stat_empty(attr,old,cur);
  
  if(attr->current == attr->max && old != attr->max)
    if(attr->on_stat_full)
      attr->on_stat_full(attr,old,cur);

  return true;
}

void StatRestart(struct stat_s* self, float old, float cur){
  self->current = self->min;
}

void StatReverse(struct stat_s* self, float old, float cur){
  self->increment *= -1;
}

void StatMaxOut(stat_t* s){
  s->current = s->max;

}

void StatEmpty(stat_t* s){
  s->current = s->min;
}

float StatGetRatio(stat_t *self){
  return self->current / self->max;
}

void FormulaDieAddAttr(stat_t* self){
  int modifier = 0;
      
  stat_attribute_relation_t* rel = &stat_modifiers[self->attribute];
  
  for(int i = 0; i < ATTR_DONE;i++){
    if(self->modified_by[i]){
      ModifierType mod = rel->modifier[i];
      switch(mod){
        case MOD_SQRT:  
          modifier += isqrt(self->owner->attribs[i]->val); 
          break;
        case MOD_NEG_SQRT:
          modifier -= isqrt(self->owner->attribs[i]->val); 
          break;
        case MOD_ADD:
          modifier += self->owner->attribs[i]->val;
          break;
        default:
          break;
      }
    }
  }


  self->base += self->die->roll(self->die);

  self->max=self->base+modifier;

}

skill_t* InitSkill(SkillType id, struct ent_s* owner, int min, int max){
  skill_t* s = calloc(1,sizeof(skill_t));

  *s = (skill_t){
    .id = id,
    .val = min,
    .min = min,
    .max = max,
    .point = 0,
    .threshold = 300,
    .owner = owner
  };
  return s;

}

bool SkillIncrease(struct skill_s* s, int amnt){
  s->point+=amnt;

  if(s->point < s->threshold)
    return false;


  s->point = s->point - s->threshold;

  s->threshold*=3;

  int old = s->val;
  s->val++;

  if(s->on_skill_up)
    s->on_skill_up(s,old,s->val);
}

int ResistDmgLookup(uint64_t trait){
  for (int i = 0; i < 15; i++){
    if(RESIST_LOOKUP[i].trait == trait)
      return RESIST_LOOKUP[i].school;
  }
}
