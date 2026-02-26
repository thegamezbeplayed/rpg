#include "game_types.h"
#include "game_tools.h"
#include "game_helpers.h"
#include "game_math.h"
#include "game_process.h"

static choice_pool_t* race_profs[ENT_DONE];
static choice_pool_t* race_classes[ENT_DONE];

MAKE_ADAPTER(StepState, ent_t*);

ent_t* InitEnt(EntityType id,Cell pos){
  ent_t* e = InitEntByRace(MONSTER_MASH[id]);
  e->type = id;

  e->props->base_diff = 5;
  item_def_t* w = GetItemDefByID(GEAR_MACE);
  EntAddItem(e, InitItem(w), true);
  
  item_def_t* r = GetItemDefByID(GEAR_BOW_LIGHT);
  EntAddItem(e, InitItem(r),true);

  item_def_t* b = GetItemDefByID(GEAR_BANDOLIER);
  EntAddItem(e, InitItem(b),true);
  item_def_t* p = GetItemDefByID(GEAR_POT_HEALTH);
  EntAddItem(e, InitItem(p), true);
  item_def_t* a = GetItemDefByID(GEAR_LEATHER_ARMOR);
  EntAddItem(e, InitItem(a), true);
  ActionSlotAddAbility(e,InitAbility(e,ABILITY_MAGIC_MISSLE));
  e->pos = pos;
  strcpy(e->name, "Michael");
 
  for(int i = 1; i < STAT_ENT_DONE; i++){
    if(e->stats[i] == NULL)
      continue;
    
    int val = e->stats[i]->max;
    const char* sname = STAT_STRING[i].name;
  }

  for(int i = 1; i < ATTR_DONE; i++){
    if(e->attribs[i]==NULL)
      continue;

    int val = e->attribs[i]->val;
    const char* name = attributes[i].name;
  }
  EntPrepare(e);
  SetState(e,STATE_SPAWN,NULL);
  return e;
}

ent_t* InitEntByRace(mob_define_t def){
  race_define_t racial = DEFINE_RACE[__builtin_ctzll(def.race)];

  ent_t* e = GameMalloc("InitEntByRace", sizeof(ent_t));
  *e = (ent_t){0};
  e->props = InitProperties(racial, def);
  if(racial.base_ent == ENT_NONE){
    e->type = def.id;
    strcpy(e->props->race_name, def.name);
  }
  else{
    e->type = racial.base_ent;
    strcpy(e->props->race_name, racial.name);
  }
  strcpy(e->name, e->props->race_name);
  e->pos = CELL_UNSET;
  e->facing = CELL_UNSET;
  e->sprite = InitSpriteByID(e->type,SHEET_ENT);
  e->sprite->owner = e;

  e->events = InitEvents();

  e->control = InitController(e);

  e->skills[SKILL_LVL] = InitSkill(SKILL_LVL,e,0,20);
  e->skills[SKILL_LVL]->on_skill_up = EntOnLevelUp;

  
  trait_pool_t res[4] = {
    {TRAIT_RESIST_SCHOOL_MASK, e->props->traits & TRAIT_RESIST_SCHOOL_MASK},
    {TRAIT_RESIST_TAG_MASK, e->props->traits & TRAIT_RESIST_TAG_MASK},
  };

  e->traits = GameCalloc("InitEntByRace traits", 1,sizeof(traits_t));

  for (int i = 0; i< 4;i++)
    EntAddTraits(e->traits,res[i].mask, res[i].shift);

  e->control->bt[STATE_SPAWN] = InitBehaviorTree(BN_SPAWN);
  e->control->bt[STATE_IDLE] = InitBehaviorTree(BN_IDLE);
  e->control->bt[STATE_AGGRO] = InitBehaviorTree(BN_AGGRO);
  e->control->bt[STATE_NEED] = InitBehaviorTree(BN_NEED);
  e->control->bt[STATE_STANDBY] = InitBehaviorTree(BN_PRIO);

  for (int i = 0; i < ATTR_BLANK; i++){
    e->attribs[i] = InitAttribute(i,0);
    int bonus = 0;
    for(int j = 0; j < ASI_DONE; j++){
      asi_bonus_t *asi = GetAsiBonus(i,e->props->body,e->props->mind,j);
      if(!asi)
        continue;

      e->attribs[i]->event[j] = true;

      int asi_bonus = 1;

      if(j > ASI_INIT)
        e->attribs[i]->cap++;
      else if(ASI_INIT > j)
        asi_bonus = j - ASI_INIT;
      else
        continue;


      e->attribs[i]->development=j; 

      if(asi->pq)
        bonus+=asi_bonus;

      if(asi->mq)
        bonus+=asi_bonus;

    }
    int val = CLAMP(bonus + BASE_ATTR_VAL,1,10);
    AttributeReset(e->attribs[i],val);
  }
  
  for (int i = 1; i < STAT_ENT_DONE;i++){
    StatClassif classif = GetStatClassif(i, e->props->body, e->props->mind);
    int val = STAT_STANDARDS[i][classif];
    int base = val;

    if(i == STAT_HEALTH)
      base = classif;

    e->stats[i] = InitStat(i,0,val,base);

    e->stats[i]->classif = classif;
    e->stats[i]->owner = e;
    e->stats[i]->start(e->stats[i]);

    switch(i){
      case STAT_STAMINA:
      case STAT_ENERGY:
      case STAT_HEALTH:
        e->stats[i]->on_stat_change = EntResetRegen;
        break;
      case STAT_STAMINA_REGEN_RATE:
      case STAT_ENERGY_REGEN_RATE:
      case STAT_HEALTH_REGEN_RATE:
        e->stats[i]->on_turn = StatIncreaseValue;
        e->stats[i]->on_stat_full = EntRestoreResource;
        break;
      default:
        break;
    }
  }
  
  e->stats[STAT_HEALTH]->on_stat_empty = EntKill;

  for (int i = 0; i < SEN_DONE; i++){
    int val = GetSenseVal(i, e->props->body, e->props->mind);
    e->senses[i] = InitSense(e, i, val);
  }

  for(int i = 0; i < SKILL_DONE; i++){
    if(e->skills[i] == NULL)
      e->skills[i] = InitSkill(i,e,0,100);
    Traits t = SKILL_TRAITS[i];
    if(t && (e->props->traits & t)){
      int rank = SKILL_RANKS[SR_SKILLED].skill_thresh;
      for(int j = 0; j < rank; j++){  
        int inc = e->skills[i]->threshold;

        SkillIncreaseUncapped(e->skills[i], inc);
      }
    }
  } 
  for(int i = 0; i < SLOT_ALL; i++)
    e->slots[i] = InitActionSlot(i, e, 1, 1);  

  if(e->props->mind > 0){
    for(int i = 0; i < 16; i++){
      qualities_benefits_t w = MIND[i];
      if((e->props->mind & w.qual) == 0)
        continue;

      for(int j = 0; j < SKILL_DONE; j++)
        if(w.skillup[j]>0)
        SkillIncreaseUncapped(e->skills[j], w.skillup[j]);

      for(int j = 0; j < w.num_abilities; j++){
        ability_t* a = InitAbility(e, w.abilities[j]);
        a->cost--;

        ActionSlotAddAbility(e,a);
      }
    }
  }

  if(e->props->natural_weaps > 0){
    for(int i = 0; i < 16; i++){
      qualities_benefits_t w = NAT_WEAPS[i];
      if((e->props->natural_weaps & w.qual) == 0)
        continue;

      for(int j = SKILL_RANGE_WEAP.x; j < SKILL_RANGE_WEAP.y; j++)
        if(w.skillup[j]>0)
        SkillIncreaseUncapped(e->skills[j], w.skillup[j]);

      for(int j = 0; j < w.num_abilities; j++){
        ability_t* a = InitAbility(e, w.abilities[j]);
        a->cost--;

        ActionSlotAddAbility(e,a);
      }
    }
  }

  if(e->props->covering > 0){
    for(int i = 0; i < 16; i++){
      qualities_benefits_t w = COVERINGS[i];
      if((e->props->covering & w.qual) == 0)
        continue;

     for(int j = 0; j < SKILL_DONE; j++)
        if(w.skillup[j]>0)
        SkillIncreaseUncapped(e->skills[j], w.skillup[j]);

      define_natural_armor_t *nat = GetNaturalArmor(e->props->covering);
      if(nat){
        ability_t* a = InitAbilityInnate(e, ABILITY_ARMOR_SAVE, nat);
        a->cost--;

        ActionSlotAddAbility(e,a);
        ability_t* dr = InitAbilityInnate(e, ABILITY_ARMOR_DR, nat);
        ActionSlotAddAbility(e,dr);

      }
    }
  }

  uint64_t size = 0; //TODO MAKE BETTERER
  for(int i = 0; i < INV_DONE; i++){

    define_burden_t limits = BURDEN_LIMITS[i][5];
    int limit = limits.str_mul * e->attribs[ATTR_STR]->max;
    e->inventory[i] = InitInventory(i, e, 0, limit);
    if(i < INV_SLING)
      e->inventory[i]->active = true;
  }
  e->team = RegisterFactionByType(e); 
  
  e->local = InitLocals(e, MAX_ENTS + MAX_ENVS*2);
  WorldSubscribe(EVENT_ADD_LOCAL_CTX, OnWorldCtx, e->local);
  WorldSubscribe(EVENT_DEL_LOCAL_CTX, OnWorldByGOUID, e->local);
  e->allies = GameCalloc("InitEntByRace allies", 1,sizeof(ally_table_t));
  InitAllyTable(e->allies, 8, e);


  return e;
}

ent_t* InitEntCommoner(mob_define_t def, define_prof_t* sel){
    
  race_define_t racial = DEFINE_RACE[__builtin_ctzll(def.race)];

  ent_t* e = InitEntByRace(def);
  if(!sel )
    return NULL;

  if (!sel->soc[def.civ].name || sel->soc[def.civ].name[0] == '\0')
    return NULL;

  strcpy(e->props->role_name, sel->soc[def.civ].name);
  strcpy(e->name, TextFormat("%s %s", e->props->race_name, e->props->role_name));
  e->props->prof = sel->id;

  for(int i = 0; i < SKILL_DONE; i++){
    for(int j = 0; j < sel->skills[i]; j++){
      int inc = e->skills[i]->threshold;

      SkillIncreaseUncapped(e->skills[i], inc);
    }
  }
 
  for (int i = 0; i < ATTR_DONE; i++){
    if(!e->attribs[i])
      continue;
    e->attribs[i]->asi += sel->attributes[i];
    e->attribs[i]->expand = AttributeScoreIncrease;
  }
  
  e->stats[STAT_HEALTH]->on_stat_empty = EntKill;

 
  if(sel->id >= PROF_LABORER){
    int num_w = 0;
    for (int i = SKILL_RANGE_WEAP.x; i < SKILL_RANGE_WEAP.y; i++){
      if(e->skills[i]->val==0)
        continue;

      bool eq = num_w < 1;
      item_def_t* idef = BuildAppropriateItem(e, ITEM_WEAPON, i);
      EntAddItem(e, InitItem(idef), eq);
      num_w++;

    }
  }

  if(e->skills[SKILL_LVL]->val == 2)
    DO_NOTHING();

  return e;
}

void RankUpEnt(ent_t* e, race_class_t* race_class){
  for (int i = 0; i < SKILL_DONE; i++){
    int beef = race_class->beefups[i];
    if(beef == 0)
      continue;
    for(int j = 0; j < beef; j++){
    int increase = e->skills[i]->threshold;
    SkillIncreaseUncapped(e->skills[i], increase);
    }
  }
}

int PromoteEntClass(ent_t* e, int ranks){
  define_rankup_t ladder = CLASS_LADDER[e->type][e->props->class_arch];

  if(ladder.ranks <= e->props->rank)
    return 0;

  Archetype rank = ladder.ladder[e->props->rank];
  race_class_t pro = PROMOTE_RACE[e->type][rank];

  e->props->class_rank = pro.base;

  for(int i = 0; i < SKILL_DONE; i++){
    if(pro.skills[i] == 0)
      continue;
    for(int j = 0; j < pro.skills[i]; j++){
      int increase = e->skills[i]->threshold;
      SkillIncreaseUncapped(e->skills[i], increase);
    }

  }
  e->props->rank++;

  return 1;
}

ent_t* InitMob(mob_t* mob, Cell pos){
  mob_define_t def = MONSTER_MASH[mob->type];
  define_prof_t prof = DEFINE_PROF[mob->prof];

  ent_t* e = InitEntCommoner(def, &prof);
  e->pos = pos;
  race_define_t rdef = GetRaceByFlag(e->props->race);
  if(mob->def)
    GrantEntClass(e, rdef, mob->def);

  return e;
}

bool ModifyEnt(ent_t* e, int amnt, MobMod mod){
  mob_modification_t modif = MOB_MODS[e->type].mods[mod];

  uint64_t pq = modif.body;
  uint64_t mq = modif.mind;

  int level_end = modif.lvl_boost + e->skills[SKILL_LVL]->val;
 
  strcpy(e->name,TextFormat(modif.name, e->props->race_name));

  for (int i = 1; i < STAT_ENT_DONE;i++){
    StatClassif classif = GetStatClassif(i, pq, mq);
    int val = STAT_STANDARDS[i][classif];

    if(classif <= e->stats[i]->classif)
      continue;

    e->stats[i]->classif = classif;
    e->stats[i]->base += val;
    e->stats[i]->start(e->stats[i]);
  }

  for (int i = 0; i < ATTR_BLANK; i++){
    for(int j = 0; j < ASI_DONE; j++){
      asi_bonus_t *asi = GetAsiBonus(i,pq,mq,j);
      if(!asi)
        continue;

      e->attribs[i]->event[j] = true;

      if(j == ASI_INIT){
        e->attribs[i]->cap++;

        if(asi->pq)
          e->attribs[i]->max++;
        if(asi->pq)
          e->attribs[i]->max++;
      }

      AttributeMaxOut(e->attribs[i]);
    }
  }

   if(modif.weaps > 0){
    for(int i = 0; i < 16; i++){
      qualities_benefits_t w = NAT_WEAPS[i];
      if((modif.weaps & w.qual) == 0)
        continue;

      for(int j = SKILL_RANGE_WEAP.x; j < SKILL_RANGE_WEAP.y; j++)
        if(w.skillup[j]>0)
        SkillIncreaseUncapped(e->skills[j], w.skillup[j]);

      for(int j = 0; j < w.num_abilities; j++){
        ability_t* a = InitAbility(e, w.abilities[j]);
        a->cost--;

        ActionSlotAddAbility(e,a);
      }
    }
  }

   if(modif.covering > 0){
    for(int i = 0; i < 16; i++){
      qualities_benefits_t c = COVERINGS[i];
      if(!modif.covering & c.qual)
        continue;
      e->props->traits |= c.traits;


      for(int j = 0; j < c.num_abilities; j++){
        ability_t* a = InitAbility(e, c.abilities[j]);
        a->cost--;

        ActionSlotAddAbility(e,a);

      }
    } 
  }

  if(modif.lvl_boost > 0){
    int rounds = 0;
    while(e->skills[SKILL_LVL]->val < level_end){
      rounds++;
      for (int i = 0; i < 5; i++){
        SkillIncreaseUncapped(e->skills[SKILL_LVL],rounds*(level_end+10));
        SkillType s = modif.skillups[i];
        if(s == SKILL_NONE)
          continue;

        int increase = e->skills[s]->threshold;
        SkillIncreaseUncapped(e->skills[s], increase);
      }
      DO_NOTHING();
    }
  }

}

void GrantClassByProf(ent_t* e, Profession p){
  choice_pool_t* pool = InitChoicePool(CLASS_BASE_DONE, ChooseByWeight);

  define_prof_t prof = DEFINE_PROF[p];
  int found = 0;
  for (int i = 0; i < CLASS_BASE_DONE; i++){
    if(prof.combat_rel[i] == 0)
      break;
    for(int k = 0; k < PROF_LABORER; k++){
      define_race_class_t c = RACE_CLASS_DEFINE[e->type][k];
      if(c.race != e->type)
        continue;
      for(int j = 0; j < c.count; j++){
        if(c.classes[j].base != prof.combat_rel[i])
          continue;

        found++;
        AddChoice(pool, c.classes[j].base, c.classes[j].weight, &c.classes[j], NULL);
      }
    }
  }

  if(found == 0){
    define_race_class_t def = RACE_CLASS_DEFINE[ENT_PERSON][PROF_NONE];
    for(int i = 0; i < def.count; i++)
    AddChoice(pool, def.classes[i].base, def.classes[i].weight, &def.classes[i], NULL);
  }

  choice_t* sel = pool->choose(pool);

  if(sel==NULL){
    TraceLog(LOG_WARNING,"Issue finding class for %s", e->name);
    return;
  }

  race_define_t racial = DEFINE_RACE[__builtin_ctzll(e->props->race)]; 
  GrantEntClass(e, racial, sel->context);
    
}

int EnhanceEnt(ent_t* e, bool promote, bool *promoted, RaceProps props){

  if(promote){
    if((props&RACE_MOD_CLASS)>0)
      *promoted = (PromoteEntClass(e,1)>0);
    else if ((props&RACE_MOD_ALPHA)>0)
      *promoted = ModifyEnt(e,1, MM_ALPHA);
  }
  else{
    if((props&RACE_MOD_CLASS)>0){
      if(e->props->class_arch > 0){
        race_class_t* rc = GetRaceClass(e->type, e->props->class_arch);
        RankUpEnt(e, rc);
      }
      else{
        GrantClassByProf(e, e->props->prof);
      }
    }
    else if((props&RACE_MOD_ENLARGE)>0)
      ModifyEnt(e, 1, MM_BRUTE);
    else{
      int inc = e->skills[SKILL_LVL]->threshold;
      SkillIncreaseUncapped(e->skills[SKILL_LVL], inc);
    }

  }

  return EntGetChallengeRating(e);
}

int EnhanceEnts(ent_t** pool, MobRules rule, int count ){
  int promotions = 0;
  switch(rule){
    case MOB_SPAWN_LAIR:
      promotions = 1;
      break;
    case MOB_SPAWN_CHALLENGE:
      promotions = count>3?1:0;
      break;
  }
  int total_cr = 0;
  for(int i = 0; i < count; i++){
    ent_t* e = pool[i];
    MobRules  m_rules = MONSTER_MASH[e->type].rules;
    RaceProps modif = DEFINE_RACE[__builtin_ctzll(e->props->race)].props & RACE_MOD_MASK;


    bool promoted = false;
    int cr = EnhanceEnt(e, (promotions>0), &promoted, modif);

    if(cr == e->props->cr){
      while(modif){
        uint64_t mod = modif & -modif;
        modif &= modif -1;
        switch(mod){
          case MM_ALPHA:
          case MM_BRUTE:
            cr = ModifyEnt(e,1,mod);
            break;
          default:
            while(cr==e->props->cr){
              int inc = e->skills[SKILL_LVL]->threshold;
              SkillIncreaseUncapped(e->skills[SKILL_LVL], inc);
              cr = EntGetChallengeRating(e);
            }
            break;
        };
      }
    }

    TraceLog(LOG_INFO,"====MOB ENHANCED===\n %s challenge rating now %i, from %i",e->name, cr, e->props->cr);
    e->props->cr = cr;
    total_cr += cr;

    if(promoted)
      promotions--;
  }

  return total_cr;

}


void GrantEntClass(ent_t* e, race_define_t racial, race_class_t* race_class){
  define_archetype_t data = CLASS_DATA[race_class->base];

  e->props->traits |= data.traits;
  e->control->behave_traits |= data.traits;

  strcpy(e->props->role_name,race_class->name);

  strcpy(e->name, TextFormat("%s %s", e->props->race_name, e->props->role_name));
  for (int i = 0; i < SKILL_DONE; i++){
    if(race_class->skills[i] == 0)
      continue;
    for(int j = 0; j < race_class->skills[i]; j++){
      int inc = e->skills[i]->threshold;
      SkillIncreaseUncapped(e->skills[i], inc);
    }

  }

  for (int i = 0; i < ATTR_DONE; i++){
    if(e->attribs[i] == NULL)
      continue;

    e->attribs[i]->asi += data.ASI[i];
    e->attribs[i]->expand = AttributeScoreIncrease;
  }

  for (int i = 0; i < STAT_ENT_DONE;i++){
  }
  
  int num_w = 0;
  for (int i = SKILL_RANGE_WEAP.x; i < SKILL_RANGE_WEAP.y; i++){
    if(e->skills[i]->val==0)
      continue;

    bool eq = num_w < 2;
    item_def_t* idef = BuildAppropriateItem(e, ITEM_WEAPON, i);
    EntAddItem(e, InitItem(idef), eq);
    num_w++;
  }

  for (int i = SKILL_ARMOR_NATURAL; i < SKILL_ATH; i++){
    if(e->skills[i]->val==0)
      continue;

    item_def_t* idef = BuildAppropriateItem(e, ITEM_ARMOR, i);
    EntAddItem(e, InitItem(idef), true);
  }

  AbilityID abilities[MAX_ABILITIES];
  int num_abilities = FilterAbilities(abilities, data.archtype, racial.race);
  for (int i = 0; i < num_abilities; i++){
    define_ability_class_t def_ab = CLASS_ABILITIES[abilities[i]];
    if(def_ab.lvl > 1)
      continue;

    ability_t* a = InitAbility(e, def_ab.id);
    int sr = 1 + e->skills[a->skills[0]]->val;
    a->weight+=sr*def_ab.priority;
    if(ActionSlotAddAbility(e,a)){
      switch(a->action){
        case ACTION_ATTACK:
        case ACTION_WEAPON:
          if(a->reach > 1)
            e->control->behave_traits |= TRAIT_CAN_SHOOT;
          else
            e->control->behave_traits |= TRAIT_CAN_MELEE;
          break;
        case ACTION_MAGIC:
          if(a->type == AT_DMG)
            e->control->behave_traits |= TRAIT_CAN_CAST;
          else
            e->control->behave_traits |= TRAIT_CAN_HEAL;
          break;
      }
    }
  }

  for(int i = 0; i < SLOT_ALL; i++){
    for (int j = 0; j < ACTION_SLOTTED; j++)
      if(e->slots[i]->allowed[j])
        e->slots[i]->pref+= data.pref_act[j];
  }

}

int EntBuild(mob_define_t def, MobRules rules, ent_t **pool){
  int count = 0;
  MobRules tmp = def.rules&=rules;
  rules&=tmp;

  if(def.id == ENT_PERSON)
    return 0;

  race_define_t racial = DEFINE_RACE[ __builtin_ctzll(def.race)];
  MobRules spawn = GetMobRulesByMask(rules,MOB_SPAWN_MASK);
  MobRules group = GetMobRulesByMask(rules,MOB_GROUPING_MASK);
  MobRules modif = GetMobRulesByMask(rules,MOB_MOD_MASK);

  int beef = 0;
  bool arm=false,don=false;

  if(modif > 0){
    while(modif){
      uint64_t mod = modif & -modif;
      modif &= modif -1;
      switch(mod){
        case MOB_MOD_WEAPON:
          arm = true;
          break;
        case MOB_MOD_ARMOR:
          don = true;
          break;
        case MOB_MOD_BEEF:
          beef++;
          break;
        default:
          break;
      }
    }
  }

  int min=99,max=-99;
  while(group){
    uint64_t size = group & -group;
    group &= group -1;

    if((size & def.rules)==0)
      continue;

    if(size > MOB_GROUPING_CREW)
      don = arm = true;

    switch(size){
      case MOB_GROUPING_SOLO:
        if (min > 1)
          min = 1;
        break;
      case MOB_GROUPING_PAIRS:
        if (min >2)
          min = 2;
        if (max < 2)
          max = 2;
        break;
      case MOB_GROUPING_TROOP:
        if (min >3)
          min = 3;
        if (max < 4)
          max = 4;
        break;
      case MOB_GROUPING_PARTY:
        if (min >5)
          min = 5;
        if (max < 5)
          max = 5;
        break;
      case MOB_GROUPING_CREW:
      case MOB_GROUPING_SQUAD:
        if (min >6)
          min = 6 + (group==MOB_GROUPING_SQUAD?1:0);
        if (max < 6)
          max = 7 + (group==MOB_GROUPING_SQUAD?2:0);
        break;
      case MOB_GROUPING_WARBAND:
        beef++;
        if (max < 8)
          max = 9;
        break;
      case MOB_GROUPING_SWARM:
        break;

    }
  }

  int amount = 0;
  if(max>min)
    amount = CLAMP(RandRange(min,max+count),1,MOB_ROOM_MAX);
  else
    amount = max+count;


  int chief = 0, captain = 0, commander =0;
  if(!race_classes[def.id])
    race_classes[def.id] = GetRaceClassPool(def.id,7,ChooseByWeight);

  choice_pool_t* class_choice = race_classes[def.id];
  
  if(amount>1){
    RaceProps tactics = GET_FLAG(racial.props, RACE_TACTICS_MASK);
    
    while(tactics){
      uint64_t tactic = tactics & -tactics;
      tactics &= tactics-1;

      switch(tactic){
        case RACE_TACTICS_ARCANA:
          if(amount>4)
            chief++;
          break;
        case RACE_TACTICS_MARTIAL:
          if(amount>3)
            captain ++;
          if(amount>7){
            captain++;
            commander ++;
          }
          break;
        case RACE_TACTICS_SIMPLE:
        case RACE_TACTICS_CRUDE:
          break;
        case RACE_TACTICS_RANKS:
          if(amount>4){
            captain+=2;
          }
          break;
        default:
          break;
      }
    }
  }
  
  count = 0;

  define_prof_t prof[PROF_END];

  int jobs = GetProfessionsBySociety(def.civ,prof);

  MobRule loc = rules && MOB_LOC_MASK;

  define_prof_t local_jobs[jobs];
  int avail = FilterProfsByRules(rules, prof, jobs, local_jobs);

  if(avail == 0)
    return 0;

  bool picking = false;
  choice_pool_t* picker = StartChoice(&race_profs[def.id],avail, ChooseByWeight, &picking);

  for(int i = 0; i < avail; i++)
    AddChoice(picker, i, local_jobs[i].soc[def.civ].weight, &local_jobs[i],NULL);


  for(int i = 0; i < amount; i++){
    choice_t* chosen = picker->choose(picker);

    define_prof_t* sel = chosen->context;

    ent_t* e = InitEntCommoner(def, sel);
    if(e == NULL)
      continue;

    if(sel->id < PROF_LABORER){
      define_race_class_t* pc = GetRaceClassForSpec(def.id,sel->id);
      for (int j = 0; j < pc->count; j++)
        AddFilter(class_choice, pc->classes[j].base, &pc->classes[j]);
      choice_t *selection = class_choice->choose(class_choice); 
      if(selection == NULL)
        continue;

      race_class_t* drc = selection->context;
      if(drc == NULL)
        continue;

      GrantEntClass(e,racial,drc);

      for(int j = 0; j < beef; j++)
        RankUpEnt(e,drc);

      //class_choice->filtered = 0;
    }

    EntPrepare(e);    
    pool[count++] = e;

//    EndChoice(race_profs[def.id], false);
  //  EndChoice(race_classes[def.id], false);

  }
  return count;
}

properties_t* InitProperties(race_define_t racials, mob_define_t m){
  properties_t* p = GameCalloc("InitProperties", 1,sizeof(properties_t));

  p->race = racials.race;
  p->base_diff = racials.base_challenge;
  p->body = racials.body | m.flags.body;
  p->mind = racials.mind | m.flags.mind;
  p->covering = racials.covering | m.flags.covering;
  p->natural_weaps = racials.weaps | m.flags.weaps;
  mind_result_t mind = GetMindResult(p->mind);
  body_result_t body = GetBodyResult(p->body);

  for(int i = 0; i < 16; i++){
    qualities_benefits_t w = NAT_WEAPS[i];
    if(!p->natural_weaps & w.qual)
      continue;
    p->traits |= w.traits;
  }

  for(int i = 0; i < 16; i++){
    qualities_benefits_t c = COVERINGS[i];
    if(!p->covering & c.qual)
      continue;
    p->traits |= c.traits;
  }

  p->size = GetSizeByFlags(p->body, p->covering);
  p->mass = GetMassByFlags(p->body, p->covering);
  p->traits |= racials.traits | mind.traits | body.traits;
  p->feats  = mind.feats | body.feats;

  Resource res = m.has;

  while(res){
    Resource r = res & -res;
    res &= res -1;

    resource_t* resource = GameCalloc("InitProperties resources", 1,sizeof(resource_t));
    resource->type = r;
    define_resource_t* def = GetResourceDef(r);
    resource->name = def->name;
    resource->smell = def->smell;
    uint64_t amnt = 0;
    switch(r){
      case RES_VEG:
        amnt = p->size;        
        break;
      case RES_MEAT:
        amnt = p->size * 0x200;
        break;
      case RES_BONE:
        amnt = p->size * 0x080;
        break;
      case RES_METAL:
        break;
      case RES_BLOOD:
        amnt = p->mass * 0x20;
        break;
      case RES_WOOD:
        break;

    }

    if(amnt == 0)
      continue;

    p->smell+=def->smell;
    resource->amount = amnt;
    p->resources[BCTZL(r)] = resource;
  }
  return p;
}

void PropAddFeat(ent_t* e, FeatFlag f, SkillType skill){
  feat_t* feat = GetSkillFeat(f, skill);

  if(!feat)
    return;

  InitFeat(e, feat);
}

env_t* InitEnvFromEnt(ent_t* e){
  env_t* env = InitEnv(ENV_BONES_BEAST, e->pos);

  bool empty = true;
  for(int i = 0; i < RES_DONE; i++){
    resource_t* tmp = e->props->resources[i];
    if(!tmp || tmp->amount == 0)
      continue;

    empty = false;

    resource_t *res = GameCalloc("InitEnvFromEnt", 1,sizeof(resource_t));

    *res = (resource_t){
      .type = tmp->type,
        .amount = tmp->amount,
        .smell = tmp->smell
    };

    env->resources[i] = res;
    env->has_resources |= tmp->type;
  }

  if(empty)
    return NULL;

  return env;

}

env_t* InitEnv(EnvTile t,Cell pos){
  env_t* e = GameMalloc("InitEnv",sizeof(ent_t));
  *e = (env_t){0};  // zero initialize if needed
  e->type = t;

  e->vpos =CellToVector2(pos,CELL_WIDTH);
  e->pos = pos;
  env_t* batch =WorldGetEnvById(e->type);
  if(!batch){
    e->sprite = InitSpriteByID(t,SHEET_ENV);
  }
  else
    e->sprite = batch->sprite;
//  e->sprite->owner = e;
  //e->pos = pos;// = Vector2Add(Vector2Scale(e->sprite->slice->center,SPRITE_SCALE),pos);

  return e;
}

void EntRender(ent_t* e){
  switch(e->status){
    case ENT_STATUS_ALIVE:
      DrawSprite(e->sprite);
      break;
    case ENT_STATUS_DEAD:
      DO_NOTHING();
      break;
  } 
}
 
void EnvRender(env_t* e){
  switch(e->status){
    case ENV_STATUS_NORMAL:
      DrawSpriteAtPos(e->sprite,e->vpos);
      break;
    case ENV_STATUS_DEAD:
      DO_NOTHING();
      break;
  }
}

void OnEnvStatus(env_t* e, EnvStatus s, EnvStatus old){
  switch(s){
    case ENV_STATUS_DEAD:
      WorldEvent(EVENT_ENV_DEATH, &e->gouid, e->gouid);
      break;
  }
}

bool EnvSetStatus(env_t* e, EnvStatus s){
  if(e->status == s)
    return false;

  EnvStatus old = e->status;
  e->status = s;
  OnEnvStatus(e, s, old);
}

int EnvExtractResource(env_t* env, ent_t* ent, Resource res){
  resource_t* r = env->resources[__builtin_ctzll(res)];

  if(!r || r->amount == 0){
    env->has_resources &= ~res;
    return 0;
  }

  int take = ent->props->mass/16;

  if (take > r->amount){
    take = r->amount;
    env->has_resources &= ~res;
  }

  int extra = 0;
  if(r->attached > 0 && (env->has_resources&r->attached)>0){
    resource_t* attached = env->resources[BCTZL(r->attached)];
    if(attached->amount > take){
      attached->amount-=take;
      extra = take;
    }
    else if(take > attached->amount){
      extra = attached->amount;
      attached->amount = 0;
      env->has_resources &= ~r->attached;
    }
  }
  
  if(extra > 0){

    for(int i = 0; i < N_DONE; i++){
      if(ent->needs[i]->resource == N_NONE)
        continue;

      if((ent->needs[i]->resource&r->attached)==0)
        continue;

      ent->needs[i]->val-=extra;
      ent->needs[i]->activity = true;
      ent->needs[i]->meter = 0;
    }
  }
  
  r->amount-=take;

  return take;

}

void EntAddTraits(traits_t* t, uint64_t mask, uint64_t shift){
  while (mask) {
    uint64_t trait = mask & -mask;  // lowest bit
    mask &= mask - 1;               // clear that bit

    int index = -1;
    switch(shift){
      case TRAIT_RESIST_SCHOOL_MASK:
        index = ResistDmgLookup(trait);
        t->resistances_school[index] += 1;
        break;
      case TRAIT_RESIST_TAG_MASK:
        DamageType schools[DMG_DONE]={0};
        index = ResistDmgLookup(trait);
        int count = GetMatchingDamageTypes(index, schools, DMG_DONE);
        for(int i = 0; i < count;i++){
          if(schools[i]==DMG_NONE)
            continue;

          t->resistances_school[schools[i]] += 1;
        }
        break;
      default:
        break;
    }
  }
}

void EntResetRegen(stat_t* self, float old, float cur){

  StatType rel = self->related;

  if(rel > STAT_NONE)
  StatRestart(self->owner->stats[rel],0,0);

}

void EntRestoreResource(stat_t* self, float old, float cur){
  StatEmpty(self);

  StatType related = self->related;
  stat_t* rstat = self->owner->stats[related];
  StatType resource = rstat->related;

  int amount = self->owner->stats[related]->current;

  if (resource == STAT_NONE)
    return;

  if(StatChangeValue(self->owner, self->owner->stats[resource], amount))
    WorldEvent(EVENT_STAT_RESTORE, self, rstat->owner->gouid);
}

void EntKill(stat_t* self, float old, float cur){
  ent_t* e = self->owner;
  SetState(e, STATE_DIE,NULL);
}

void EntPrepare(ent_t* e){
  EntPollInventory(e);
 for(int i = 0; i < STAT_ENT_DONE; i++){
    if(!e->stats[i])
      continue;

    StatMaxOut(e->stats[i]);
  }

  if(e->slots[SLOT_SAVE]->count ==0){
    ability_t* a = InitAbility(e, ABILITY_ARMOR_SAVE);
    a->hit->sides = e->stats[STAT_ARMOR]->current;
    a->hit->num_die = 1;
    a->skills[0] = SKILL_ARMOR_NATURAL;
  }

  EntApplyTraits(e);

}

void EntBuildAllyTable(ent_t* e){
  AllyAdd(e->allies, e, 0);

  /*
  for (int i = 0; i < e->local->count; i++){
    local_ctx_t* ctx = RemoveEntryByRel(e->local, SPEC_KIN);
    if(ctx == NULL || ctx->other.type_id != DATA_ENTITY)
      break;

    ent_t* ally = ParamReadEnt(&ctx->other);
    int dist = cell_distance(e->pos, ally->pos);
    AllyAdd(e->allies, ally, dist);

  }
*/
}

void EntInitOnce(ent_t* e){
  EntSync(e);
  
  WorldApplyLocalContext(e);
  
  for(int i = 0; i < ACTION_PASSIVE; i++)
    e->control->speed[i] = InitInit(i, e);

  for(int i = 0; i < STAT_ENT_DONE; i++){
    if(!e->stats[i])
      continue;

    StatMaxOut(e->stats[i]);
  }

  WorldTargetSubscribe(EVENT_AGGRO, PriorityEvent, e->control->priorities, e->gouid);
  WorldTargetSubscribe(EVENT_DAMAGE_TAKEN, DamageEvent, e, e->gouid);

  WorldTargetSubscribe(EVENT_ENT_STEP, LocalSyncPos, e, e->gouid);
  EntBuildAllyTable(e);
}

void EntApplyTraits(ent_t* e){


    e->props->cr = EntGetChallengeRating(e);
}

void EntPollInventory(ent_t* e){
  for (int i = 0; i < INV_DONE; i++){
    if(e->inventory[i] == NULL)
      continue;

    InventoryPoll(e, i);
  }
}

item_t* EntGetItem(ent_t* e, ItemCategory cat, bool equipped){
  /*
  for (int i = 0; i < CARRY_SIZE; i++){
    if(!e->gear[i])
      break;

    if(e->gear[i]->equipped && e->gear[i]->def->category == cat)
      return e->gear[i];
  }
*/
  return NULL;
}

bool EntAddItem(ent_t* e, item_t* item, bool equip){
  item_t* added = InventoryAddItem(e, item);
  if(added){
    event_fuid_i fuid = EventMakeFlexID(e->gouid, 
        (flex_id_t){DATA_INT, .id = item->def->type});
    added->fuid = fuid;
    added->equipped = equip;
    return true;
  }

  return false;
}

void EntAddExp(ent_t *e, int exp){
  SkillIncreaseUncapped(e->skills[SKILL_LVL], exp);
}

void EntDestroy(ent_t* e){
  if(!e || !SetState(e, STATE_DEAD,NULL))
    return;

  MapRemoveOccupant(e->map,e->pos);

  if(e->sprite!=NULL){
    e->sprite->owner = NULL;
    SpriteSetAnimState(e->sprite,ANIM_KILL);
  }

  e->control = NULL;
}

InteractResult AbilityConsume(ent_t* owner,  ability_t* a, ent_t* target){
  int cr = 1;

  InteractResult ires = IR_SUCCESS;


  int rolls[a->dc->num_die];
  int base = a->dc->roll(a->dc,rolls);

  base = (base + a->stats[STAT_DAMAGE]->current);

  stat_t* damage_to = target->stats[a->damage_to];
 
  if(StatChangeValue(target, damage_to, base)){
    TraceLog(LOG_INFO,"%s consumes potion %s now %0.0f / %0.0f",
        target->name,
        STAT_STRING[a->damage_to].name,
        damage_to->current,
        damage_to->max);

  }

  return ires;
}

int EntAddAggro(ent_t* owner, ent_t* source, int threat, float mul, bool init){
  int cr = LocalAddAggro(owner->local, source, threat, mul, init);

  param_t gouid = ParamMake(DATA_UINT64, sizeof(uint64_t), &owner->gouid);
  for (int i = 0; i < owner->allies->count; i++){
    ent_t* e = owner->allies->entries[i].ally;
    Cell next;
    if(EntCanDetect(e, owner,SEN_SEE)){
      PriorityAdd(e->control->priorities, PRIO_HELP, gouid);
      LocalAddAggro(e->local, source, threat, 0.1f, init);
    }
  } 

  return cr;
}

int EntConsume(ent_t* e, param_t goal, Resource res){

  int consumed = 0;
  switch(goal.type_id){
    case DATA_LOCAL_CTX:
      local_ctx_t* ctx = ParamReadCtx(&goal);
      return EntConsume(e, ctx->other, res);
      break;
    case DATA_ENTITY:
      break;
    case DATA_ENV:
      env_t* env = ParamReadEnv(&goal);
      consumed = EnvExtractResource(env, e, res);
      //goal->resource = env->has_resources;
      if(consumed >0)
        TraceLog(LOG_INFO,"%s eats %i grams of food", e->name, consumed);
      break;
  }

  
  return consumed;
}

InteractResult EntMeetNeed(ent_t* e, need_t* n, param_t goal){
  InteractResult res = IR_NONE;
  int amount = 0;
  switch(n->id){
    case N_HUNGER:
      amount = EntConsume(e, goal, n->resource);
      break;
    default:
      break;
  }
  
  NeedFulfill(n, amount); 
  return res;
}

ability_t* EntChoosePreferredAbility(ent_t* e){
  ActionSlot slot_pool[SLOT_ALL];

  ActionSlotSortByPref(e, slot_pool, SLOT_ALL);

  for(int i = 0; i < SLOT_ALL; i++){
    if(e->slots[i]->count == 0)
      continue;

    int budget = e->stats[e->slots[i]->resource]->current;
    ability_t* a = EntChooseWeightedAbility(e, budget, i);
    if(a)
      return a;
  }

  return NULL;
}

ability_t* EntChooseWeightedAbility(ent_t* e, int budget, ActionSlot slot){
  bool running = false;
  int count = e->slots[slot]->count;
  if(count == 0)
    return NULL;

  choice_pool_t* p = StartChoice(&e->control->choices[slot], count, ChooseByWeightInBudget,&running);

  p->budget = budget;
  
  if(!running){
    for(int j = 0; j < e->slots[slot]->count; j++){
      ability_t* a = e->slots[slot]->abilities[j];
      AddPurchase(p, a->id, a->weight, a->cost, a, NULL);

    }
  }

  choice_t* choice = p->choose(p);
  if(choice && choice->context)
    return choice->context;

  return NULL;
}

local_ctx_t* EntGetTarget(ent_t* e, AbilityID id){
  ability_t* a = EntFindAbility(e, id);
  if(!a)
    return NULL;

  int range = a->stats[STAT_REACH]->current;
  local_table_t* t = e->local;
  for(int i = 0; i < t->count; i++){
    local_ctx_t* ctx = &t->entries[t->sorted_indices[i]];

    if(ctx->dist > range)
      break;

    if(ctx->other.type_id != DATA_ENTITY)
      continue;

    Cell dir = cell_dir(e->pos, ctx->pos);

    Cell facing = CellSub(e->facing, e->pos);
    if(!cell_compare(dir,facing))
      continue;

    return ctx;
  }

  return NULL;
}

bool EntPrepareAttack(ent_t* e, ability_t* a, local_ctx_t* t){
  return AbilityCanTarget(a, t);

}

InteractResult EntAbilitySave(ent_t* e, ability_t* a, ability_sim_t* source){
  int hit = source->hit_calc;

  int saves[a->hit->num_die];
  int save = a->hit->roll(a->hit,saves);

  return save < hit?IR_FAIL:IR_SUCCESS;
}

InteractResult EntAbilityReduce(ent_t* e, ability_t* a, ability_sim_t* source){
  InteractResult res = IR_FAIL;

  int dr = a->dr->resist_types[source->d_type];
  if(source->penn > dr)
    return IR_CRITICAL_FAIL;

  int total = 0;
  while (dr > 0 && source->dmg_calc > 0){
    for(int i = 0; i < source->dmg_die; i++){
      if(source->dmg_res[i] <= 0)
        continue;

      res = IR_SUCCESS;
      source->dmg_res[i]--;
      dr--;
      source->dmg_calc--;
      if(dr<=0)
        break;
    }
  }

  if(dr > 0 && source->d_bonus > 0)
    source->d_bonus-= dr;

  source->final_dmg = source->d_bonus + source->dmg_calc;

  return res;
}

InteractResult EntUseAbility(ent_t* e, ability_t* a, ent_t* target){
  interaction_t* combat = StartCombat(e, target, a);
  if(!combat)
    return IR_NONE;

  bool success = true;
  InteractResult ires = IR_NONE;

  int cr = LocalAddAggro(e->local, target, 1, target->props->base_diff, false);
  LocalAddAggro(target->local, e, 1, 1, false);//e->props->base_diff);

  if(a->resource>STAT_NONE && a->cost > 0)
    if(!StatChangeValue(e,e->stats[a->resource],-1*a->cost)){
      TraceLog(LOG_INFO,"%s not enough %s",e->name, STAT_STRING[a->resource].name);
      success = false;
    }

  while(ires != IR_DONE)
    ires = CombatStep(combat, ires);

  combat_t* c = combat->ctx;
  return c->step[BAT_DMG];

}

bool FreeEnt(ent_t* e){
  if(!e)
    return false;
/*
  for(int i = 0; i < STAT_ENT_DONE; i++)
    free(e->stats[i]);
*/
  
  /*for(int i = 0; i < ATTR_DONE; i++)
    free(e->attribs[i]);
*/
  e=NULL;
  free(e);
  return true;
}

controller_t* InitController(ent_t* e){
  controller_t* ctrl = GameCalloc("InitController", 1,sizeof(controller_t));

  ctrl->actions = InitActionPool(e);

  ctrl->priorities = InitPriorities(e, N_DONE + 20);
  ctrl->turn = -1;
  ctrl->phase = TURN_NONE;
  ctrl->behave_traits = e->props->traits & TRAIT_CAP_MASK;
  for(int i = 0; i < N_DONE; i++){
    need_t* n =  InitNeed(i,e);
    e->needs[i] = n;
    param_t np = ParamMakeObj(DATA_NEED, n->id, n);
    WorldTargetSubscribe(EVENT_NEED_STATUS, OnNeedStatus, e, e->gouid);
    np.gouid = hash_combine_64(e->gouid, hash_string_64(NEED_STRINGS[i]));
    PriorityAdd(ctrl->priorities, PRIO_NEEDS, np);
  }

  bool running = false;
  ctrl->decider[STATE_ACTION] = StartDecision(&ctrl->decider[STATE_ACTION],
      STATE_END, e, STATE_ACTION, &running);

  return ctrl;
}

bool AbilitySkillup(ent_t* owner, ability_t* a, ent_t* target, InteractResult result){
  
  aggro_t* e = LocalGetAggro(owner->local,target->gouid);

  if(e==NULL)
    return false;

  int cr = e->challenge;

  switch(a->type){
    case AT_SAVE:
    case AT_DR:
      cr = e->offensive_rating;
      break;
    case AT_DMG:
      cr = e->defensive_rating;
      break;
  }

  for(int i = 0; i < a->num_skills; i++)
    SkillUse(owner->skills[a->skills[i]],owner->uid,target->uid,cr,result);

  return true;
}

InteractResult AbilityUse(ent_t* owner, ability_t* a, ent_t* target, ability_sim_t* other){
  InteractResult ires = IR_NONE;
  switch(a->type){
    case AT_DMG:
    case AT_HEAL:
      if(a->use_fn == NULL)
        return IR_FAIL;

      ires = a->use_fn(owner, a, target);
      break;
    case AT_SAVE:
    case AT_DR:
      ires = a->save_fn(owner, a, other);
      break;
    default:
      break;
  }
  
  if(ires > IR_NONE){
   if(a->on_use_cb)
    a->on_use_cb(owner, a, target, ires);

   if(a->item && a->item->on_use)
     a->item->on_use(owner, a->item, ires);
   
   if(a->type < AT_SAVE)
   if(a->chain && a->chain_fn)
     a->chain_fn(owner, a->chain, target);
  }

  if(ires >=IR_SUCCESS && a->on_success_cb)
    a->on_success_cb(owner, a, target, ires);
  

  return ires;
}

bool ValueUpdateStat(value_t* v, void* ctx){
  ability_t* a = ctx;

  StatType s = v->stat_relates_to;

  a->stats[s]->max = v->val;
  a->stats[s]->current = v->val;

  return true;
}

bool ValueUpdateDie(value_t* v, void* ctx){
  ability_t* a = ctx;

  switch (v->cat) {
    case VAL_ADV_HIT:
    case VAL_ADV_SAVE:
      {
        int side = a->hit->sides;
        int num  = 1;

        a->hit = InitDie(side, num, v->val, RollDieAdvantage);
        return true;
      }
      break;
    case VAL_ADV_DMG:
      {
        int side = a->dc->sides;
        int num  = a->dc->num_die;

        a->dc = InitDie(side, num, v->val, RollDieAdvantage);
        return true;
      }
      break;
    case VAL_HIT:
    case VAL_SAVE:
      {
        a->hit->sides = v->val;
        return true;
      }
      break;
    default:
      return false;
  }

}

void AbilityApplyValues(ability_t* self, value_t* v){
  if(!self || !self->values[v->cat])
    return;

  value_t* sv = self->values[v->cat];
  sv->base = v->base;
  sv->val = v->base;

  if(sv->on_change){
    sv->context = self;
    sv->on_change(sv,self);
  }
}

void EntComputeFOV(ent_t* e){
  int radius = e->senses[SEN_SEE]->range;

  for (int oct = 0; oct < 8; oct++)
    CastLight(e->map, e->pos, 1, 1.0, 0.0, radius,
              mult[oct][0], mult[oct][1],
              mult[oct][2], mult[oct][3]);

}

void EntTurnSync(ent_t* e){
  for(int i = 0; i < e->map->num_changes; i++){
    map_cell_t* update = e->map->changes[i];

    if(!update->updates || update->tile == NULL)
      continue;


    //EntAddLocalEnv(e, update->tile);

  }
  
  for(int i = 0; i< STAT_ENT_DONE; i++){
    if(e->stats[i] ==NULL)
      continue;

    if(e->stats[i]->on_turn){
      if(e->type == ENT_PERSON)
        DO_NOTHING();

      e->stats[i]->on_turn(e->stats[i],0,0);
    }
  }
}

void EntSync(ent_t* e){
  if(e->events)
    StepEvents(e->events);

  if(!e->sprite)
    return;

  e->sprite->pos = CellToVector2(e->pos,CELL_WIDTH);// + abs(ent->sprite->offset.y);
}

TileStatus EntGridStep(ent_t *e, Cell step){
  Cell newPos = CellInc(e->pos,step);
  
  TileStatus status = MapSetOccupant(e->map,e,newPos);

  if(status < TILE_ISSUES){
    map_cell_t* mc = &e->map->tiles[newPos.x][newPos.y];
    if(mc){
      local_ctx_t* ctx = LocalGetEntry(e->local, mc->gouid);
      if(ctx)
        ctx->awareness*=1.1;
    }
    Cell oldPos = e->pos;
    //WorldDebugCell(e->pos, YELLOW);
    e->pos = newPos;
    e->old_pos = oldPos;
    e->facing = CellInc(e->pos,step);
    e->local->valid = false;
    WorldEvent(EVENT_UPDATE_LOCAL_CTX, &e->gouid, e->gouid);
    WorldEvent(EVENT_ENT_STEP, e, e->gouid);
    //WorldContextChange(OBJ_ENT, e->gouid);
    //WorldDebugCell(e->pos, GREEN);
  }
  else
    e->facing = newPos;

  return status;
}

void EntSetCell(ent_t *e, Cell pos){
  e->pos = pos;
}

void EntControlStep(ent_t *e, int turn, TurnPhase phase){
  if(!e->control) 
    return;

  if(turn != e->control->turn){
    for(int i = 0; i < N_DONE; i++)
      NeedStep(e->needs[i]);
  }
  /*
  if(turn == e->control->turn && e->control->phase == phase)
   return;
*/
  ActionPoolSync(e->control->actions);
  
  e->control->phase = phase;
  e->control->turn = turn;
  LocalSync(e->local, false);

  if(e->type == ENT_PERSON)
    return;

  if(ActionHasStatus(e->control->actions, ACT_STATUS_QUEUED))
    return;

  if(ActionHasStatus(e->control->actions, ACT_STATUS_NEXT))
    return;  


  if(!e->control->bt || !e->control->bt[e->state])
    return;


  PrioritiesSync(e->control->priorities);
  behavior_tree_node_t* current = e->control->bt[e->state];

  current->tick(current, e);
}

void EntToggleTooltip(ent_t* e){
}

bool SetState(ent_t *e, EntityState s,StateChangeCallback callback){
  if(CanChangeState(e->state,s)){
    EntityState old = e->state;
    e->state = s;

    if(callback!=NULL)
      callback(e,old,s);

    OnStateChange(e,old,s);
    return true;
  }

  return false;
}

void StepState(ent_t *e){
  SetState(e, e->state+1,NULL);
}

bool CanChangeState(EntityState old, EntityState s){
  if(old == s || old > STATE_END)
    return false;

  switch(old){
    case STATE_END:
      if(s==STATE_STANDBY)
        return false;
      break;
    case STATE_DIE:
      if(s!=STATE_DEAD)
        return false;
      break;
    case STATE_AGGRO:
      if(s == STATE_NEED)
        return false;
    default:
      return true;
      break;
  }

  switch(s){
    case STATE_NONE:
      return false;
      break;
    case STATE_DIE:
      if(old == STATE_END)
        return false;
    default:
      return true;
      break;
  }

  return true;
} 

void OnStateChange(ent_t *e, EntityState old, EntityState s){
  switch(old){
    case STATE_SPAWN:
      e->status = ENT_STATUS_ALIVE;
      item_t* item = InventoryGetEquipped(e, INV_HELD);
      if(e->sprite)
        e->sprite->is_visible = true;
      break;
    default:
      break;
  }

  switch(s){
    case STATE_SPAWN:
      e->status = ENT_STATUS_SPAWN;
      break;
    case STATE_DIE:
      env_t* corpse = InitEnvFromEnt(e);
      if(corpse)
        RegisterEnv(corpse);

      e->status = ENT_STATUS_DEAD;
      WorldEvent(EVENT_ENT_DEATH, &e->gouid, e->gouid);
      EntDestroy(e);
      break;
    case STATE_STANDBY:
      e->previous = old;
      break;
    case STATE_RETURN:
      if(e->previous < STATE_STANDBY && e->previous!=STATE_NONE){
        SetState(e, e->previous, NULL);
        e->previous = STATE_NONE;
      }
      break;
    default:
      break;
  }
}

void EntOnLevelUp(struct skill_s* self, float old, float cur){
  ent_t* e = self->owner;
  for(int i = 1; i < ATTR_BLANK; i++){
    if(e->attribs[i]->expand) 
      e->attribs[i]->expand(e->attribs[i]);
    AsiEvent asi = GetAsiEventForLevel(cur);
    if(e->attribs[i]->event[asi])
      AttributeScoreEvent(e->attribs[i]);
  }

  for(int i = 1; i < STAT_ENT_DONE; i++)
    if(e->stats[i] && e->stats[i]->lvl){
      e->stats[i]->lvl(e->stats[i]);
      StatMaxOut(e->stats[i]);
    }
}

bool CheckEntPosition(ent_t* e, Vector2 pos){
  if(!e)
    return false;

  Vector2 topCorner = Vector2Subtract(e->sprite->pos,e->sprite->slice->center);

  Rectangle bounds = RectPos(topCorner, e->sprite->slice->bounds);
  return point_in_rect(pos, bounds);
}

bool CheckEntAvailable(ent_t* e){
  if(!e)
    return false;

  return (e->status == ENT_STATUS_ALIVE);
}

bool CheckEnvAvailable(env_t* e){
  if(!e)
    return false;

  return (e->status < ENV_STATUS_DEAD);
}

ability_t* EntFindAbility(ent_t* e, AbilityID id){
  ActionType act = ABILITIES[id].action;

  for(int i = 0; i < SLOT_ALL; i++){
    if(!e->slots[i]->allowed[act])
      continue;
    int count = e->slots[i]->count;
    for (int j = 0; j < count; j++){
      if(e->slots[i]->abilities[j]->id == id)
        return e->slots[i]->abilities[j];
    }
  }

  return NULL;
}

ability_t AbilityLookup(AbilityID id){
  if(id >= ABILITY_NONE && id < ABILITY_DONE)
    return ABILITIES[id];
}

char* EntGetClassNamePretty(ent_t* e){
  return "Paragon";
}

float EntGetDPR(ent_t* e){
  int num_atk = 0;

  int damage = 0;
  int num_acts = 1;//e->stats[STAT_ACTIONS]->max;

  for(int i = 0; i < SLOT_SAVE; i++){
    if(e->slots[i]->count == 0)
      continue;

    num_atk++;

    for(int j = 0; j < num_acts; j++){
      ability_t* a = e->slots[i]->abilities[0];
      /*
         int hit = a->hit->roll(a->hit);
         int save = t->stats[STAT_ARMOR]->current;
         if(a->save > ATTR_NONE)
         save = t->attribs[a->save]->val;

         if(hit < save)
         continue;
         */
      int s_dmg = AbilityAddPB(e,a,STAT_DAMAGE);

      s_dmg += DieMax(a->dc);


      damage+=s_dmg;
    }
  }

  return damage/num_atk;
}
int EntGetOffRating(ent_t* e){
  return (int)(EntGetDPR(e) * 10.0f);
}

int EntGetDefRating(ent_t* e){
  float avoid = 1+( e->stats[STAT_ARMOR]->max/20);

  float ehp = e->stats[STAT_HEALTH]->max * avoid;

  return ehp;
}

int EntGetChallengeRating(ent_t* e){
  e->props->offr = EntGetOffRating(e);
  e->props->defr = EntGetDefRating(e);
  return  e->props->defr*10 + e->props->offr;

}
InteractResult EntSkillCheck(ent_t* owner, ent_t* tar, SkillType s){
  InteractResult res = IR_CRITICAL_FAIL;

  SkillType counter = tar->skills[s]->checks->counter;
  int diff = 0;
  if(owner->skills[counter] && owner->skills[counter]->checks){
    res = SkillCheck(owner->skills[counter], tar->skills[s]);

    diff = tar->skills[s]->val;

    SkillUse(owner->skills[counter], owner->gouid, tar->gouid, diff, res);
    
    int gain = owner->skills[counter]->val;
    InteractResult tarres = IR_NONE;
    switch(res){
      case IR_CRITICAL_FAIL:
        tarres = IR_TOTAL_SUCC;
        break;
      case IR_FAIL:
        tarres = IR_SUCCESS;
        break;
      case IR_SUCCESS:
        tarres = IR_FAIL;
        break;
      case IR_TOTAL_SUCC:
        tarres = IR_CRITICAL_FAIL;
        break;
    }

    SkillUse(tar->skills[s], tar->gouid, owner->gouid, gain, tarres);
  }

  return res;
}

InteractResult EntCanDetect(ent_t* e, ent_t* tar, Senses s){
  int depth = e->senses[s]->range;
  SkillType counter = tar->skills[SKILL_STEALTH]->checks->counter;
  depth += SkillCheckGetVal(e->skills[counter],VAL_REACH);
  int dist = cell_distance(e->pos, tar->pos);
  if( dist > depth)
    return IR_FAIL;

  if(e->props->race == tar->props->race)
    return IR_TOTAL_SUCC;

  bool los = HasLOS(e->map, e->pos, tar->pos);

  if(!los && s == SEN_SEE)
    return IR_FAIL;

  SkillUse(e->skills[counter], e->gouid, tar->gouid, dist, IR_SUCCESS);

  InteractResult res =  EntSkillCheck(e, tar, SKILL_STEALTH);

  if(res > IR_ALMOST)
    return res;

  return IR_ALMOST;
}

uint64_t EntGetResourceByNeed(ent_t* e, Needs n){

  switch(n){
    case N_HUNGER:
      mob_define_t def = MONSTER_MASH[e->type];
      return def.eats;
      break;
    default:
      return RES_NONE;
      break;
  }

}

uint64_t EntGetSize(ent_t* e){
  return PQ_SMALL;
}

uint64_t EntGetScents(ent_t* e){

  return MONSTER_MASH[e->type].has;
}

uint64_t EntGetTrackable(ent_t* e){

  return MONSTER_MASH[e->type].eats;
}

int EntGetTrackDist(ent_t* e, local_ctx_t* tar){

  uint64_t resources = EntGetTrackable(e);
  uint64_t scents = 0;
  map_cell_t* loc = NULL;
  switch(tar->other.type_id){
    case DATA_ENTITY:
      ent_t* other = ParamReadEnt(&tar->other);
      loc = MapGetTile(other->map, other->pos);
      scents = EntGetScents(other);
      break;
    case DATA_ENV:
      env_t* env = ParamReadEnv(&tar->other);
      loc = MapGetTile(e->map, env->pos);
      scents = env->has_resources;
      break;
    case DATA_MAP_CELL:
      loc = ParamReadMapCell(&tar->other);
      scents = loc->props->resources;
      break;
  }

  uint64_t matches = resources & scents;
  int count = __builtin_popcountll(matches);

  int strength = 1;
  for(int i = 0; i < SATUR_MAX; i++){
    if((loc->props->scents[i]&tar->gouid)==0)
      continue;

    strength+=i;
  }

  int potency = 0;
  while(matches){

    Resource r = matches & -matches;
    matches &= matches -1;

    potency+=DEF_RES[BCTZL(r)].smell; 
  }

  int radius = (1 + tar->awareness) * strength * (potency + count);

  return radius;
}

skill_check_t* EntGetSkillPB(SkillType s, ent_t* e, local_ctx_t* ctx, Senses sen){
  ent_t* other = ParamReadEnt(&ctx->other);

  skill_check_t* sc = GameCalloc("EntGetSkillPB", 1,sizeof(skill_check_t));

  if(e->skills[s]->checks)
  *sc = *e->skills[s]->checks;

  switch(sen){
    case SEN_SMELL:
      uint64_t resources = EntGetTrackable(e);
      uint64_t scents = EntGetScents(other);
      uint64_t matches = resources & scents;

      while(matches){
        Resource r = matches & -matches;
        matches &= matches -1;

        sc->vals[VAL_ADV_HIT]++;
        sc->vals[VAL_REACH]+=e->props->resources[BCTZL(r)]->smell;
      }
      break;
    default:
      break;

  }

  return sc;
}

bool EntCheckRange(ent_t* e, decision_t* d){
  local_ctx_t* ctx = NULL;
  switch(d->decision){
    case ACTION_MOVE:
    case ACTION_INTERACT:
      if(d->params[ACT_PARAM_DEST].type_id != DATA_LOCAL_CTX)
        return false;
      ctx = ParamReadCtx(&d->params[ACT_PARAM_DEST]);
      return ctx->dist < 1;
      break;
    case ACTION_ATTACK:
      if(d->params[ACT_PARAM_TAR].type_id != DATA_LOCAL_CTX)
        return false;

      if(d->params[ACT_PARAM_ABILITY].type_id != DATA_ABILITY)
        return false;

      ability_t* a = ParamRead(&d->params[ACT_PARAM_ABILITY], ability_t);
      ctx = ParamReadCtx(&d->params[ACT_PARAM_TAR]);
      if(ctx->dist < 2)
        return true;

      return AbilityCanTarget(a, ctx);
      break;
  }

  return false;
}

int EntGetCtxByNeed(ent_t* e, need_t* n, int num, local_ctx_t* pool[num]){

  uint64_t res = n->resource;

  param_t f = ParamMake(DATA_UINT64, sizeof(res), &res);

  return LocalContextFilter(e->local, num, pool, f, PARAM_RESOURCE, ParamCompareAnd);
}
