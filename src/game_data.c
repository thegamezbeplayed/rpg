#include <raylib.h>
#include "game_tools.h"
#include "game_process.h"
#include "game_gen.h"
#include <stdio.h>

ability_t ABILITIES[ABILITY_DONE]={
  {ABILITY_NONE},
  {ABILITY_WACK,AT_DMG,ACTION_ATTACK, DMG_BLUNT,STAT_STAMINA, DES_NONE, 25,2, 5, 1, 3,0, 1, STAT_HEALTH, ATTR_NONE, ATTR_STR, 
    .skills[0] = SKILL_WEAP_NONE},
  {ABILITY_BITE, AT_DMG,ACTION_ATTACK, DMG_PIERCE,STAT_STAMINA, DES_NONE, 25,1, 4, 1, 4,4, 1, STAT_HEALTH, ATTR_NONE, ATTR_STR,
    .skills = SKILL_WEAP_NONE},
  {ABILITY_CHEW, AT_DMG,ACTION_ATTACK, DMG_PIERCE,STAT_STAMINA, DES_NONE, 25,1, 4, 1, 2,0, 1, STAT_ARMOR, ATTR_NONE, ATTR_NONE, ABILITY_GNAW, 
    .skills = SKILL_WEAP_NONE},
  {ABILITY_GNAW,AT_DMG,ACTION_ATTACK, DMG_PIERCE,STAT_STAMINA, DES_NONE, 25,1, 4, 1, 2,0, 1, STAT_HEALTH, ATTR_NONE, ATTR_STR, 
    .skills = SKILL_WEAP_NONE},
  {ABILITY_CLAW,AT_DMG,ACTION_ATTACK, DMG_SLASH, STAT_STAMINA, DES_NONE, 50,2, 6, 2, 3, 1, 1, STAT_HEALTH, ATTR_NONE, ATTR_STR, 
    .skills = SKILL_WEAP_NONE},
  {ABILITY_SWIPE, AT_DMG,ACTION_ATTACK, DMG_SLASH,STAT_STAMINA, DES_NONE, 50,2, 6, 2, 6, 3,1, STAT_HEALTH, ATTR_STR,
    .skills = SKILL_WEAP_NONE},
  {ABILITY_BITE_POISON, AT_DMG,ACTION_ATTACK, DMG_PIERCE, STAT_STAMINA, DES_NONE, 25,1, 4, 1, 2, 0,1, STAT_HEALTH, ATTR_NONE, ATTR_STR,ABILITY_POISON, SKILL_WEAP_NONE},
  {ABILITY_POISON, DMG_POISON, STAT_NONE, DES_NONE, 25,1, 9, 1, 3,0,1,STAT_HEALTH, ATTR_CON, ATTR_NONE,
    .skills = SKILL_POISON},
  {ABILITY_MAGIC_MISSLE ,ACTION_MAGIC, DMG_FORCE, STAT_ENERGY, DES_SELECT_TARGET, 20,4,99,1,4,1,3,STAT_HEALTH, ATTR_NONE, ATTR_NONE,ABILITY_NONE, SKILL_SPELL_EVO},
  {ABILITY_ELDRITCH_BLAST,ACTION_MAGIC, DMG_FORCE, STAT_ENERGY, DES_MULTI_TARGET, 20, 8, 0,1, 10,3, STAT_HEALTH, ATTR_NONE, ATTR_CHAR,
   .skills = SKILL_SPELL_EVO },
  {ABILITY_RESISTANCE,
    .skills = SKILL_SPELL_ABJ
  },
  {ABILITY_SACRED_FLAME, ACTION_MAGIC, DMG_RADIANT, STAT_ENERGY, DES_SELECT_TARGET, 15, 8, 4, 1, 8, 0, 5, STAT_HEALTH, ATTR_DEX, ATTR_WIS,
    .skills = SKILL_SPELL_EVO},
  {ABILITY_STARRY_WISP, ACTION_MAGIC,DMG_RADIANT, STAT_ENERGY, DES_SELECT_TARGET, 15, 8,0 , 1, 8, 0, 5, STAT_HEALTH, ATTR_NONE, ATTR_WIS},
  {ABILITY_MAGIC_STONE, ACTION_MAGIC,DMG_BLUNT, STAT_ENERGY, DES_MULTI_TARGET, 15, 3, 1, 1, 6, 0, 5, STAT_HEALTH, ATTR_NONE, ATTR_INT},
  {ABILITY_POISON_SPRAY, ACTION_MAGIC,DMG_POISON, STAT_ENERGY, DES_SELECT_TARGET, 12, 8, 0, 1,10,0,3, STAT_HEALTH, ATTR_CON, ATTR_NONE},
  {ABILITY_FIRE_BOLT, ACTION_MAGIC,DMG_FIRE, STAT_ENERGY, DES_SELECT_TARGET, 10, 8, 2, 1,10,0,4, STAT_HEALTH, ATTR_NONE, ATTR_INT},
  {ABILITY_REND, AT_DMG,ACTION_ATTACK,DMG_SLASH, STAT_STAMINA, DES_NONE, 25, 8, 4, 1, 4,1,1,STAT_HEALTH, ATTR_NONE,ATTR_STR,ABILITY_BLEED},
  {ABILITY_HAMSTRING},
  {ABILITY_RAGE, AT_DMG,ACTION_ATTACK,DMG_TRUE, STAT_RAGE, DES_NONE, 10, 1, 0,1, 2, 1,1, STAT_HEALTH, ATTR_NONE, ATTR_NONE },
  {ABILITY_WEAP},
  {ABILITY_WEAP_BLUDGEON, AT_DMG,ACTION_WEAPON,
    DMG_BLUNT, STAT_STAMINA, DES_NONE, 25,1,17,3,2,0,1,STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_CHOP,AT_DMG,ACTION_WEAPON,
    DMG_SLASH, STAT_STAMINA, DES_NONE, 25,1,17,2,3,0,1,STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_STAB,AT_DMG,ACTION_WEAPON,
    DMG_PIERCE, STAT_STAMINA, DES_NONE, 25,1,17,2,3,0,1,STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_SLASH,AT_DMG,ACTION_WEAPON,
    DMG_SLASH, STAT_STAMINA, DES_NONE, 25,1,15,1,6,0,1,STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_PIERCE,AT_DMG,ACTION_WEAPON,
    DMG_PIERCE, STAT_STAMINA, DES_NONE, 25,1,17,2,2,0,1,STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_RANGE_PIERCE,AT_DMG,ACTION_WEAPON,
    DMG_PIERCE, STAT_STAMINA, DES_NONE, 25,1,15,1,4,0,1,STAT_HEALTH,ATTR_NONE, ATTR_DEX},
  {ABILITY_STATUS},
  {ABILITY_BLEED, AT_DMG, ACTION_NONE,
    DMG_BLEED, STAT_STAMINA, DES_NONE, 0, 0, 4, 1, 6, 2,1, STAT_HEALTH, ATTR_CON,ATTR_STR},
  {ABILITY_ARMOR},
  {ABILITY_ARMOR_SAVE, AT_SAVE, ACTION_SLOTTED,
    .chain_id = ABILITY_ARMOR_DR, .save_fn = EntAbilitySave},
  {ABILITY_ARMOR_DR, AT_DR},
  {ABILITY_ITEM},
  {ABILITY_ITEM_HEAL, AT_HEAL, ACTION_ITEM,
    DMG_RADIANT, STAT_NONE, DES_SELF, 10, 1, 1, 2,4,2,0, STAT_HEALTH, ATTR_NONE, ATTR_NONE,.skills=SKILL_ALCH, .use_fn = AbilityConsume
  }
};

item_fn_t item_funcs[ITEM_DONE] = {
  {ITEM_NONE},
  {ITEM_WEAPON,.num_equip = 1, .on_equip= ItemAddAbility},
  {ITEM_ARMOR,.num_equip = 2, .on_equip=
    {
      ItemApplyStats, ItemAddAbility
    }
  },
  {ITEM_DONE}
};

weapon_def_t WEAPON_TEMPLATES[WEAP_DONE]= {
  {WEAP_NONE},
  {WEAP_MACE,5,1,0,1,0, 50,
    .ability = ABILITY_WEAP_BLUDGEON,
    .skill = SKILL_WEAP_MACE},
  {WEAP_SWORD,10,1,0,1,0, 30,
    .ability = ABILITY_WEAP_SLASH,
    .skill = SKILL_WEAP_SWORD},
  {WEAP_AXE,7,1,1,0,1, 50,
    .ability = ABILITY_WEAP_CHOP, 
    .skill = SKILL_WEAP_AXE},
  {WEAP_DAGGER, 2, 1,0, 1,0, 40,
    .ability = ABILITY_WEAP_STAB,
    .skill = SKILL_WEAP_DAGGER
  },
  {WEAP_BOW,25,2,0,2,0, 50,
    .w_props = PROP_WEAP_AMMO | PROP_WEAP_TWO_HANDED,
    .ability = ABILITY_WEAP_RANGE_PIERCE,
    .skill = SKILL_WEAP_BOW
  },
};

armor_def_t ARMOR_TEMPLATES[ARMOR_DONE]={
  {ARMOR_NONE},
  {ARMOR_NATURAL},
  {ARMOR_PADDED, 8},
  {ARMOR_LEATHER, 10, 
    {{[DMG_SLASH]=1,[DMG_BLUNT]=1},{}},
    {{[DMG_SLASH]=1,[DMG_PIERCE]=1},{}},
    8,10,200,ATTR_DEX,ATTR_NONE,10,0,
    .skill = SKILL_ARMOR_LEATHER
  },
  {ARMOR_CHAIN, 12, 
    {{[DMG_SLASH]=2,[DMG_PIERCE]=1},{}},
    {{[DMG_SLASH]=1,[DMG_PIERCE]=1},{}},
    16,25,400,ATTR_DEX,ATTR_STR,2,9,
    .skill = SKILL_ARMOR_CHAIN
  },
  {ARMOR_PLATE, 16,
    {{[DMG_SLASH]=2,[DMG_PIERCE]=2,[DMG_BLUNT]=1},{}},
    {{[DMG_SLASH]=1,[DMG_PIERCE]=1}},
    32,100,800,ATTR_NONE,ATTR_STR,0,12,
    .skill = SKILL_ARMOR_PLATE
  },
};

consume_def_t CONSUME_TEMPLATES[CONS_DONE] = {
  {CONS_POT, 50, 400, 1, 2,
    PROP_MAT_LIQUID, PROP_CONS_HEAL,
    ABILITY_ITEM_HEAL, SKILL_ALCH
  }
};

dice_roll_t* Die(int side, int num){
  dice_roll_t* die = malloc(sizeof(dice_roll_t));

  *die = (dice_roll_t){
    .sides = side,
    .num_die = num,
    .roll = RollDie
  };

  return die;
}

dice_roll_t* InitDie(int side, int num, int adv, DiceRollFunction fn){
  dice_roll_t* die = malloc(sizeof(dice_roll_t));

  DiceCompareCallback cb = NULL;
  if(adv > 0)
    cb = GREATER_THAN;
  else if (adv < 0)
    cb = LESS_THAN;

  *die = (dice_roll_t){
    .sides      = side,
    .num_die    = num,
    .advantage  = adv,
    .roll       = fn,
    .cb         = cb
  };

  return die;
}
int DieMax(dice_roll_t* d){

  return d->sides * d->num_die;
}
int RollDieAdvantage(dice_roll_t* d){
  int choice = RollDie(d);

  for (int i = 0; i < d->advantage; i++){
     choice = d->cb(choice, RollDie(d));
  }

  return choice;
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

attribute_t* InitAttribute(AttributeType type, int val){
  attribute_t* t = calloc(1,sizeof(attribute_t));

  t->max = t->min = t->val = val;
  t->asi = 0;
  t->rollover = 0;
  t->cap = 20;
  return t;
}

bool AttributeScoreEvent(attribute_t* a){
  if(a->max == a->cap)
    return false;
  a->max++;
  a->val++;
  return true;
}
bool AttributeScoreIncrease(attribute_t* a){
  if(a->asi==0)
    return false;

  a->rollover+=a->asi;
  if((a->asi+a->rollover+a->max) > a->cap)
    return false;

  float new = a->max+a->rollover;
  a->rollover = new - (int)new;
  a->max = a->val = (int)new;

  return true;
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

value_t* InitValue(ValueCategory cat, int base){
  value_t* v = calloc(1,sizeof(value_t));

  *v = (value_t){
    .cat = cat,
    .base = base
  };

  return v;
}

float AffixAdd(value_affix_t* self, int val){
  return val + self->val;
}

float AffixSub(value_affix_t* self, int val){
  return val - self->val;
}

float AffixMul(value_affix_t* self, int val){
  return val * self->val/100;
}

float AffixFract(value_affix_t* self, int val){
  return val * self->val/100;
}

float AffixBase(value_affix_t* self, int val){
  return self->val;
}

int ValueRebase(value_t* self){
  for(int i = 0; i < AFF_DONE; i++){
    if(self->base_app[i] == NULL)
      continue;

    if(self->base_app[i]->val == 0)
      continue;

    self->base = ValueApplyModsToVal(self->base, self->base_app[i]);
  }

  return self->base;

}

int ValueApplyModsToVal(int val, value_affix_t* aff){
  int result = aff->affix_mod(aff,val);

  return result;
}

void ValueAddBaseMod(value_t* self, value_affix_t mod){
  ValueAffix aff = mod.affix;

  if(self->base_app[aff]){
    int val = self->base_app[aff]->val;
    switch(aff){
      case AFF_ADD:
      case AFF_SUB:
        self->base_app[aff]->val+=mod.val;
        break;
      case AFF_FRACT:
        float fval = val/100 * mod.val/100;
        self->base_app[aff]->val = CLAMP(fval*100,1,99);
        break;
      case AFF_BASE:
        self->base_app[aff]->val = val>mod.val?val:mod.val;
        break;
      case AFF_MUL:
        self->base_app[aff]->val = imax(100,mod.val+val-100);
        break;
    }
  }
  else{
    self->base_app[aff] = InitValueAffixFromMod(mod);
  }
}

value_affix_t* InitValueAffixFromMod(value_affix_t mod){
  value_affix_t* va = calloc(1,sizeof(value_affix_t));

  *va = mod;
  va->affix_mod = AffixBase;
  switch(va->affix){
    case AFF_ADD:
      va->affix_mod = AffixAdd;
      break;
    case AFF_SUB:
      va->affix_mod = AffixSub;
      break;
    case AFF_MUL:
      va->affix_mod = AffixMul;
      break;
    case AFF_FRACT:
      va->affix_mod = AffixFract;
      break;
  }

  return va;
}

stat_t* InitStatOnMin(StatType attr, float min, float max){
 stat_t* s = calloc(1,sizeof(stat_t));
 *s =(stat_t){
    .type = attr,
      .min = min,
      .max = max,
      .current = min,
      .ratio = StatGetRatio,
      .increment = 1.0f
  };

 return s;
}

stat_t* InitStatOnMax(StatType attr, float val, AttributeType modified_by){

  return InitStat(attr, 0, val, val);
}

bool StatIsEmpty(stat_t* s){
  return s->current <= s->min;
}

stat_t* InitStatEmpty(void){}
stat_t* InitStat(StatType attr,float min, float max, float amount){
 stat_t* s = malloc(sizeof(stat_t));
 stat_attribute_relation_t relate = stat_modifiers[attr];

 stat_relate_t tandem = STAT_RELATION[attr];

 *s =(stat_t){
    .type = attr,
      .related    = tandem.related,
      .min       = min,
      .max       = max,
      .base      = amount,
      .current   = amount,
      .ratio     = StatGetRatio,
      .increment = 1.0f,
      .die       = Die(amount,1),
      .start     =relate.init,
      .lvl       = relate.lvl,
      .reverse   = relate.reverse
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
  if(s->reverse)
    s->current = s->min;
  else
    s->current = s->max;

}

void StatEmpty(stat_t* s){
  if(s->reverse)
    s->current = s->max;
  else
    s->current = s->min;
}

float StatGetRatio(stat_t *self){
  return self->current / self->max;
}

void FormulaAddAttr(stat_t* self){
  int modifier = 0;
      
  stat_attribute_relation_t* rel = &stat_modifiers[self->type];
  
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

  self->max = self->base+modifier;

}

void FormulaDie(stat_t* self){}
void FormulaDieAttr(stat_t* self){}
void FormulaBaseAttr(stat_t* self){}
void FormulaBaseDie(stat_t* self){}

void FormulaDieAddAttr(stat_t* self){
  int modifier = 0;
      
  stat_attribute_relation_t* rel = &stat_modifiers[self->type];
  
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

  int roll = self->die->roll(self->die);
  self->base+= roll;
  self->max+=roll+modifier;
}

skill_event_t* InitSkillEvent(skill_t* s, int cr){
  skill_event_t* skev = calloc(1,sizeof(skill_event_t));

  skill_decay_t* decay = SkillEventDecay(s->id, cr);
  *skev = (skill_event_t){
    .skill       = s->id,
      .challenge = cr,
      .decay     = decay
  };
}

bool SkillUseSecondary(skill_t* self, int gain, InteractResult result){
  
}
 
bool SkillUse(skill_t* self, int source, int target, int gain, InteractResult result){
  int* cr = malloc(sizeof(int));
  *cr = gain;
  
  param_t data = {
    .type_id  = DATA_INT,
    .data     = cr,
    .size = sizeof(int)
  };
 
  interaction_t* iter = StartInteraction(source,target,EVENT_SKILL, 9, self, self->id, data, RegisterSkillEvent, UpdateSkillEvent, true);

  skill_event_t* skev = iter->ctx;
  skill_decay_t* sk_decay = skev->decay;

  float amnt = sk_decay->weights[result];
  float decay = sk_decay->diminish[result];
  if(decay == 0)
   decay = 50; 

  if(amnt == 0)
    amnt = 75;

  decay*=0.01f;
  amnt*=0.01f;
  int increase = gain * amnt;


  for(int i = 0; i < skev->uses; i++)
    increase*=decay;

  if(increase > 0.5f)
    SkillIncrease(self,increase);

  skev->uses++;
  return true;
}

interaction_uid_i RegisterSkillEvent(interaction_t* self, void* ctx, param_t payload){
  skill_t* skill = ctx;
  int cr = 0;
  if(payload.type_id == DATA_INT){
    cr = ParamReadInt(&payload);
    ParamFree(&payload); 
  }

  skill_event_t* skev = InitSkillEvent(skill, cr);

  skev->interact_uid = self->uid;
  self->ctx = skev;

  return self->uid; 
}

interaction_uid_i UpdateSkillEvent(interaction_t* self, void* ctx, param_t payload){
  skill_event_t* skev = ctx;

  self->timer->elapsed = 0;
  skev->uses++;

  return self->uid;

}

skill_decay_t* SkillEventDecay(SkillType skill, int difficulty){

  SkillRate rating = SkillRateLookup(skill);

  if(rating == RATE_NONE)
    return NULL;

  define_skill_rate_t skr = SKILL_RATES[rating];

  skill_decay_t* decay = calloc(1,sizeof(skill_decay_t));

  *decay = (skill_decay_t){
    .skill = skill,
    .rate  = rating,
    .falloff = skr.duration
  };

  for(int i = 0; i < IR_MAX; i++){
    decay->weights[i] =skr.weights[i];
    decay->diminish[i] =skr.dr[i];

  }

  return decay;
}

skill_t* InitSkill(SkillType id, struct ent_s* owner, int min, int max){
  skill_t* s = calloc(1,sizeof(skill_t));

  *s = (skill_t){
    .id = id,
    .val = min,
    .min = min,
    .max = max,
    .point = 0,
    .threshold = 350,
    .owner = owner,
    .on_skill_up = SkillupRelated
  };
  return s;

}

bool SkillIncrease(struct skill_s* s, int amnt){
  s->point+=imin(MAX_SKILL_GAIN,amnt);

  if(s->point < s->threshold){
    TraceLog(LOG_INFO,"%s experience in %i now (%0f / %i)",s->owner->name, s->id, s->point, s->threshold);
    return false;
  }

  s->point = s->point - s->threshold;

  //s->threshold*=3;

  int old = s->val;
  s->val++;

  TraceLog(LOG_INFO,"%s has reached %i rank %i",s->owner->name, s->id, s->val);
  if(s->on_skill_up)
    s->on_skill_up(s,old,s->val);
}

void SkillupRelated(skill_t* self, float old, float cur){
  skill_relation_t related =  SKILLUP_RELATION[self->id];

  for (int i = 0; i < MAG_DONE; i++){
    SkillType rel = related.magnitude[i];
    if (rel == SKILL_NONE)
      continue;
    SkillIncrease(self->owner->skills[rel], cur * i);

  }
}

SkillRate SkillRateLookup(SkillType){
  for(int i = 0; i < RATE_DONE; i++){
    //TODO
  }

  return RATE_LINEAR;
}

int ResistDmgLookup(uint64_t trait){
  for (int i = 0; i < 15; i++){
    if(RESIST_LOOKUP[i].trait == trait)
      return RESIST_LOOKUP[i].school;
  }
}

EntityType MobGetByRules(MobRules rules){

}

int GetItemPropsMods(ItemProps props, uint64_t cat_props, ItemCategory cat, item_prop_mod_t **mods){
  int idx = 0;

  while(props){
    uint64_t prop = props & -props;
    props &= props - 1;
    mods[idx++] = GetItemPropMods(ITEM_NONE,prop);
  }

  while (cat_props) {
    uint64_t bit = cat_props & -cat_props;
    cat_props &= cat_props - 1;

    mods[idx++] = GetItemPropMods(cat,bit);
  }

  return idx;
}

item_prop_mod_t* GetItemPropMods(ItemCategory cat, uint64_t prop){
  for(int i = 0; i < NUM_WEAP_PROPS; i++){
    if(PROP_MODS[cat][i].propID != prop)
      continue;

    return &PROP_MODS[cat][i];
  }
  
  return NULL;

}
