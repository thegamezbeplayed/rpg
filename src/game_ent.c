#include "game_types.h"
#include "game_tools.h"
#include "game_helpers.h"
#include "game_math.h"
#include "game_process.h"

MAKE_ADAPTER(StepState, ent_t*);

ent_t* InitEnt(EntityType id,Cell pos){
  ent_t* e = InitEntByRace(MONSTER_MASH[id] ,0);
  e->type = id;

  e->props->base_diff = 5;
  item_def_t* w = GetItemDefByID(GEAR_MACE);
  EntAddItem(e, InitItem(w), true);
  item_def_t* p = GetItemDefByID(GEAR_POT_HEALTH);
  EntAddItem(e, InitItem(p), true);
  item_def_t* a = GetItemDefByID(GEAR_LEATHER_ARMOR);
  EntAddItem(e, InitItem(a), true);
  ActionSlotAddAbility(e,InitAbility(e,ABILITY_MAGIC_MISSLE));
  e->stats[STAT_ACTIONS]->current = 1;
  e->pos = pos;
  e->control = NULL;
  strcpy(e->name, "Michael");
  e->stats[STAT_ACTIONS]->on_stat_empty = EntActionsTaken;
  e->stats[STAT_ACTIONS]->current = 1;
  e->stats[STAT_ACTIONS]->max = 1;
  
  e->actions[ACTION_MOVE] = InitAction(ACTION_MOVE, DES_FACING, NULL,NULL);
  e->actions[ACTION_ATTACK] = InitAction(ACTION_ATTACK, DES_FACING, ActionAttack,NULL);
  e->actions[ACTION_MAGIC] = InitAction(ACTION_MAGIC, DES_MULTI_TARGET, ActionMultiTarget,NULL);

  e->aggro = calloc(1,sizeof(aggro_table_t));
  InitAggroTable(e->aggro, 10, e);
  TraceLog(LOG_INFO,"%s has ====>\n<====STATS=====>",e->name);
  for(int i = 0; i < STAT_ENT_DONE; i++){
    if(e->stats[i] == NULL)
      continue;
    
    int val = e->stats[i]->max;
    const char* sname = STAT_STRING[i].name;
    TraceLog(LOG_INFO,"%i %s \n",val,sname);
  }

  TraceLog(LOG_INFO,"<=====ATTRIBUTES=====>\n");
  for(int i = 0; i < ATTR_DONE; i++){
    if(e->attribs[i]==NULL)
      continue;

    int val = e->attribs[i]->val;
    const char* name = attributes[i].name;
    TraceLog(LOG_INFO,"%i %s",val,name);
  }

  SetState(e,STATE_SPAWN,NULL);
  return e;
}

ent_t* InitMob(EntityType mob, Cell pos){
  ent_t* e = malloc(sizeof(ent_t));
  *e = (ent_t){0};  // zero initialize if needed
  return e;
}

ent_t* InitEntByRace(mob_define_t def, MobRules rules){
  race_define_t racial = DEFINE_RACE[__builtin_ctzll(def.race)];

  ent_t* e = malloc(sizeof(ent_t));
  *e = (ent_t){0};
  e->type = racial.base_ent;

  strcpy(e->name, TextFormat("%s",racial.name));
  e->props = InitProperties(racial);
  e->pos = CELL_UNSET;
  e->facing = CELL_UNSET;
  e->sprite = InitSpriteByID(e->type,SHEET_ENT);
  e->sprite->owner = e;
  e->events = InitEvents();

  e->control = InitController();

  e->skills[SKILL_LVL] = InitSkill(SKILL_LVL,e,0,20);
  e->skills[SKILL_LVL]->on_skill_up = EntOnLevelUp;
  for(int i = 0; i < SKILL_DONE; i++){
    if(e->skills[i] == NULL)
      e->skills[i] = InitSkill(i,e,0,100);
    if(SKILL_TRAITS[i]&TRAIT_EXP_MASK)
      SkillIncreaseUncapped(e->skills[i], 400);
  }
  
  trait_pool_t res[4] = {
    {TRAIT_RESIST_SCHOOL_MASK, e->props->traits & TRAIT_RESIST_SCHOOL_MASK},
    {TRAIT_RESIST_TAG_MASK, e->props->traits & TRAIT_RESIST_TAG_MASK},
  };


  e->traits = calloc(1,sizeof(traits_t));

  for (int i = 0; i< 4;i++)
    EntApplyTraits(e->traits,res[i].mask, res[i].shift);

  e->control->bt[STATE_IDLE] = InitBehaviorTree(BEHAVIOR_SEEK);
  e->control->bt[STATE_WANDER] = InitBehaviorTree(BEHAVIOR_WANDER);
  e->control->bt[STATE_AGGRO] = InitBehaviorTree(BEHAVIOR_MOB_AGGRO);
  e->control->bt[STATE_ACTION] = InitBehaviorTree(BEHAVIOR_TAKE_ACTION);
  e->control->bt[STATE_ATTACK] = InitBehaviorTree(BEHAVIOR_COMBAT);

  e->control->ranges[RANGE_NEAR] = 2;
  e->control->ranges[RANGE_LOITER] = 3;
  e->control->ranges[RANGE_ROAM] = 4;

  InitActions(e->actions);
  e->actions[ACTION_MOVE] = InitAction(ACTION_MOVE, DES_NONE, ActionTraverseGrid,NULL);
  e->actions[ACTION_ATTACK] = InitAction(ACTION_ATTACK, DES_FACING, ActionAttack,NULL);

  for (int i = 0; i < ATTR_BLANK; i++){
    e->attribs[i] = InitAttribute(i,0);
    int bonus = 0;
      for(int j = 0; j < ASI_DONE; j++){
        asi_bonus_t *asi = GetAsiBonus(i,e->props->body,e->props->mind,j);
        if(!asi)
          continue;

        e->attribs[i]->event[i] = true;
        
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
  
  for (int i = 0; i < STAT_ENT_DONE;i++){
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
  
  for(int i = 0; i < SLOT_ALL; i++)
    e->slots[i] = InitActionSlot(i, e, 1, 1);  

  return e;
}

ent_t* IntEntCommoner(mob_define_t def, MobRules rules){
  define_prof_t prof[PROF_END];

  int jobs = GetProfessionsBySociety(def.civ,prof);

  MobRule loc = rules && MOB_LOC_MASK;

  define_prof_t local_jobs[jobs];
  int avail = FilterProfsByRules(rules, prof, jobs, local_jobs);

  if(avail == 0)
    return NULL;

  choice_pool_t* picker = InitChoicePool(avail, ChooseByWeight);

  for(int i = 0; i < avail; i++)
    AddChoice(picker,local_jobs[i].social_weights[def.civ], &local_jobs[i]);

  choice_t* chosen = picker->choose(picker);
  if(!chosen)
    return NULL;

  define_prof_t* sel = chosen->context;

  if(!sel)
    return NULL;

  
  race_define_t racial = DEFINE_RACE[__builtin_ctzll(def.race)];

  ent_t* e = InitEntByRace(def,rules);
  strcpy(e->name, TextFormat("%s %s",racial.name,sel->social_name[def.civ]));
  e->aggro = calloc(1,sizeof(aggro_table_t));
  InitAggroTable(e->aggro, 4, e);

  for(int i = 0; i < SKILL_DONE; i++){
    if(sel->skills[i]==0)
      continue;

    SkillIncreaseUncapped(e->skills[i], sel->skills[i]);

  }
 
  for (int i = 0; i < ATTR_DONE; i++){
    if(!e->attribs[i])
      continue;
    e->attribs[i]->asi += sel->attributes[i];
    e->attribs[i]->expand = AttributeScoreIncrease;
  }
/*
  for (int i = 0; i < STAT_ENT_DONE;i++){
    int val = base.stats[i] + size.stats[e->size][i];
    int base = val;

    if(i == STAT_HEALTH)
      base = e->size;

    e->stats[i] = InitStat(i,0,val,base);

    e->stats[i]->owner = e;
    e->stats[i]->start(e->stats[i]);

    switch(i){
      case STAT_STAMINA:
      case STAT_ENERGY:
        e->stats[i]->on_stat_change = EntResetRegen;
        break;
      case STAT_STAMINA_REGEN_RATE:
      case STAT_ENERGY_REGEN_RATE:
        e->stats[i]->on_turn = StatIncreaseValue;
        e->stats[i]->on_stat_full = EntRestoreResource;
        break;
      default:
        break;
    }
  }
  */
  e->stats[STAT_HEALTH]->on_stat_empty = EntKill;

  int num_w = 0;
  for (int i = SKILL_RANGE_WEAP.x; i < SKILL_RANGE_WEAP.y; i++){
    if(e->skills[i]->val==0)
      continue;

    bool eq = num_w < 1;
    item_def_t* idef = BuildAppropriateItem(e, ITEM_WEAPON, i);
    EntAddItem(e, InitItem(idef), eq);
    num_w++;

  }

  return e;
}

ent_t* InitEntByRaceClass(uint64_t class_id, SpeciesType race){
  define_race_class_t arch = RACE_CLASS_DEFINE[__builtin_ctzll( race)][class_id];

/*
  race_define_t racial = DEFINE_RACE[__builtin_ctzll(race)];

  define_archetype_t data = CLASS_DATA[__builtin_ctzll(arch.base)];


  ent_t* e = malloc(sizeof(ent_t));
  *e = (ent_t){0};  // zero initialize if needed

  e->type = racial.base_ent;

  strcpy(e->name, TextFormat("%s %s",racial.name,arch.b_name));

  e->pos = CELL_UNSET;
  e->facing = CELL_UNSET;
  e->sprite = InitSpriteByID(e->type,SHEET_ENT);
  e->sprite->owner = e;
  e->events = InitEvents();

  e->control = InitController();

  e->skills[SKILL_LVL] = InitSkill(SKILL_LVL,e,0,20);
  e->traits = calloc(1,sizeof(traits_t));
  for (int i = 0; i < ATTR_DONE; i++){
    int val = BASE_ATTR_VAL + size.attr[e->size][i];
    e->attribs[i] = InitAttribute(i,val);
    e->attribs[i]->asi = data.ASI[i];
    e->attribs[i]->expand = AttributeScoreIncrease;
  }

  for (int i = 0; i < STAT_ENT_DONE;i++){
    int val = base.stats[i] + size.stats[e->size][i];
    int base = val;

    if(i == STAT_HEALTH)
      base = data.hitdie;

    e->stats[i] = InitStat(i,0,val,base);

    e->stats[i]->owner = e;
    e->stats[i]->start(e->stats[i]);

    switch(i){
      case STAT_STAMINA:
      case STAT_ENERGY:
        e->stats[i]->on_stat_change = EntResetRegen;
        break;
      case STAT_STAMINA_REGEN_RATE:
      case STAT_ENERGY_REGEN_RATE:
        e->stats[i]->on_turn = StatIncreaseValue;
        e->stats[i]->on_stat_full = EntRestoreResource;
        break;
      default:
        break;
    }
  }
  AbilityID abilities[MAX_ABILITIES];
  int num_abilities = FilterAbilities(abilities, data.archtype, race);
  for (int i = 0; i < num_abilities; i++){
    define_ability_class_t def_ab = CLASS_ABILITIES[abilities[i]];
    if(def_ab.lvl > 1)
      continue;

    ability_t* a = InitAbility(e, def_ab.id);

    ActionSlotAddAbility(e,a);
  }
  return e;
  */
}

void PromoteEntClass(ent_t* e, race_define_t racial, race_class_t* race_class){
  define_archetype_t data = CLASS_DATA[__builtin_ctzll(race_class->base)];

  strcpy(e->name, TextFormat("%s %s",racial.name,race_class->main));

  for (int i = 0; i < ATTR_DONE; i++){
    if(e->attribs[i] == NULL)
      continue;

    e->attribs[i]->asi += data.ASI[i];
    e->attribs[i]->expand = AttributeScoreIncrease;
  }

  for (int i = 0; i < STAT_ENT_DONE;i++){
  }

  AbilityID abilities[MAX_ABILITIES];
  int num_abilities = FilterAbilities(abilities, data.archtype, racial.race);
  for (int i = 0; i < num_abilities; i++){
    define_ability_class_t def_ab = CLASS_ABILITIES[abilities[i]];
    if(def_ab.lvl > 1)
      continue;

    ability_t* a = InitAbility(e, def_ab.id);

    ActionSlotAddAbility(e,a);
  }
}

int EntBuild(mob_define_t def, MobRules rules, ent_t **pool){
  int count = 0;
  MobRules tmp = def.rules&=rules;
  rules&=tmp;

  race_define_t racial = DEFINE_RACE[ __builtin_ctzll(def.race)];
  MobRules spawn = GetMobRulesByMask(rules,MOB_SPAWN_MASK);
  MobRules group = GetMobRulesByMask(rules,MOB_GROUPING_MASK);
  MobRules modif = GetMobRulesByMask(rules,MOB_MOD_MASK);

  int beef = 0;
  bool enlarge = false,arm=false,don=false;

  if(modif > 0){
    while(modif){
      uint64_t mod = modif & -modif;
      modif &= modif -1;
      switch(mod){
        case MOB_MOD_ENLARGE:
          enlarge = true;         
          break;
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

  bool swarm=false,outfit=false, promote=false,diverse=false,pat=false,suprise=false,elite=false;
  if(spawn > 0){
    while(spawn){
      uint64_t stype = spawn & -spawn;
      spawn &= spawn -1;

      switch(stype){
        case MOB_SPAWN_TRAP:
        case MOB_SPAWN_SECRET:
          //suprise = true;         
          diverse = true;
          break;
        case MOB_SPAWN_LAIR:
          elite = true;
          break;
        case MOB_SPAWN_CHALLENGE:
          beef++;
          promote=true;
          diverse = true;
          break;
        case MOB_SPAWN_CAMP:
          count+=2;
          diverse = true;
          beef++;
          arm = true;
          don =true;
          break;
        case MOB_SPAWN_PATROL:
          diverse = true;
          promote = true;
          arm = true;
          don = true;
          pat = true;
          count++;
          break;
        default:
          break;
      }
    }
  }


  int min=99,max=-99;
  if (group > 0){
    while(group){
      uint64_t size = group & -group;
      group &= group -1;

      if(size > MOB_GROUPING_TROOP)
        diverse = true;

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
            min = 6 + (group==MOB_GROUPING_SQUAD)?1:0;
          if (max < 6)
            max = 7 + (group==MOB_GROUPING_SQUAD)?2:0;
          break;
        case MOB_GROUPING_WARBAND:
          promote=true;
          outfit=true;
          beef++;
          elite=true;
          if (max < 8)
            max = 9;
          break;
        case MOB_GROUPING_SWARM:
          swarm=true;
          break;

      }
    }
  }

  int monster_size = 0;
  if(enlarge){
    monster_size++;
    if(beef)
      monster_size++;
  }

  int amount = 0;
  if(max>min)
    amount = CLAMP(RandRange(min,max+count),1,MOB_ROOM_MAX);
  else
    amount = max+count;

  if(amount>2)
    monster_size =0;

  int chief_w = 0, captain_w = 0, commander_w =0;
  choice_pool_t* class_choice = GetRaceClassPool(def.race,7,ChooseByWeight);
  if(diverse && amount>1){
    RaceProps tactics = GET_FLAG(racial.props, RACE_TACTICS_MASK);
    
    while(tactics){
      uint64_t tactic = tactics & -tactics;
      tactics &= tactics-1;

      switch(tactic){
        case RACE_TACTICS_ARCANA:
          if(amount>4)
            chief_w= amount*12;
          break;
        case RACE_TACTICS_MARTIAL:
          if(amount>3)
            captain_w += amount*10;
          if(amount>7)
            commander_w += amount*7;
          break;
        case RACE_TACTICS_SIMPLE:
        case RACE_TACTICS_CRUDE:
          break;
        case RACE_TACTICS_RANKS:
          if(amount>4){
            captain_w+=2;
          }
          break;
        default:
          break;
      }
    }

  }
  count = 0;

  for(int i = 0; i < amount; i++){
    ent_t* e = IntEntCommoner(def,rules);
    if(diverse && promote){
      choice_t *selection = class_choice->choose(class_choice); 
      race_class_t* drc = selection->context;

      PromoteEntClass(e,racial,drc);

      if(arm){
        item_def_t* item = GetItemDefByID(GEAR_DAGGER);
        EntAddItem(e, InitItem(item), true);
        /*
           for (int w = 0; w < w_num; i++){
           EntAddItem(e,InitItem(BuildWeapon(arms[w], QUAL_TRASH)),true);
           }
           */
      }
    }
    EntAddExp(e, 200);
    EntAddExp(e, 200);
    for(int j = 0; j < beef; j++)
      EntAddExp(e, 400);

//    class_weight[selection]-=amount;
    pool[count++] = e;
    }
  return count;
}

properties_t* InitProperties(race_define_t racials){
  properties_t* p = calloc(1,sizeof(properties_t));

  p->race = racials.race;
  p->base_diff = racials.base_challenge;
  p->body = racials.body;
  p->mind = racials.mind;
  mind_result_t mind = GetMindResult(p->mind);
  body_result_t body = GetBodyResult(p->body);

  p->traits = mind.traits | body.traits;
  p->feats  = mind.feats | body.feats;

  return p;
}

env_t* InitEnv(EnvTile t,Cell pos){
  env_t* e = malloc(sizeof(ent_t));
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

void EntApplyTraits(traits_t* t, uint64_t mask, uint64_t shift){
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

void EntCalcStats(ent_t* e, race_define_t* racial){
}

item_t* InitItem(item_def_t* def){
  item_t* item = malloc(sizeof(item_t));

  *item = (item_t){
    .def = def
  };

  if(item_funcs[def->category].cat != ITEM_NONE){
    for(int i = 0; i < item_funcs[def->category].num_equip; i++)
      item->on_equip = item_funcs[def->category].on_equip[i];
  }

  return item;
}

item_pool_t* InitItemPool(void) {
    item_pool_t* ip = calloc(1, sizeof(item_pool_t));
    ip->size = 0;
    return ip;
}

item_def_t* DefineConsumable(ItemInstance data){
  item_def_t* item = calloc(1,sizeof(item_def_t));
  item->id = data.id;

  item->category = ITEM_CONSUMABLE;
  
  consume_def_t temp = CONSUME_TEMPLATES[data.equip_type];

  item->values[VAL_WORTH] = InitValue(VAL_WORTH,temp.cost);
  item->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,temp.weight);
  item->values[VAL_DURI] = InitValue(VAL_DURI,temp.quanity);
  item->skills[item->num_skills++] = temp.skill;
  item->ability = temp.ability;


  ApplyItemProps(item, data.props, data.et_props);
  
  return item;
}

item_def_t* DefineItem(ItemInstance data){
  
  switch(data.cat){
    case ITEM_ARMOR:
      return DefineArmor(data);
      break;
    case ITEM_WEAPON:
      return DefineWeapon(data);
    case ITEM_CONSUMABLE:
      return DefineConsumable(data);
    default:
      return NULL;
      break;
  }
}

item_def_t* DefineWeapon(ItemInstance data){
  item_def_t* item = DefineWeaponByType(data.equip_type, data.props, data.et_props);

  item->id = data.id;

  return item;
}

item_def_t* DefineWeaponByType(WeaponType t, ItemProps props, WeaponProps w_props){
  item_def_t* item = calloc(1,sizeof(item_def_t));
  item->category = ITEM_WEAPON;

  weapon_def_t temp = WEAPON_TEMPLATES[t];

  //TODO CHANGE TO VALUE_T
  //item->weight = temp.weight;

  item->values[VAL_WORTH] = InitValue(VAL_WORTH,temp.cost);
  item->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,temp.weight);
  item->values[VAL_PENN] = InitValue(VAL_PENN,temp.penn);
  item->values[VAL_DURI] = InitValue(VAL_DURI,temp.durability);
  
  item->values[VAL_ADV_HIT] = InitValue(VAL_ADV_HIT,0);
  item->values[VAL_ADV_DMG] = InitValue(VAL_ADV_DMG,0);
 
  item->skills[item->num_skills++] = temp.skill; 
  item->ability = temp.ability; 
  ApplyItemProps(item, props, w_props);

  for(int i = 0; i < VAL_ALL; i++){
    if(!item->values[i])
      continue;

    item->values[i]->val = ValueRebase(item->values[i]);
  }
  return item;

};

item_def_t* DefineArmor(ItemInstance data){
  item_def_t* item = calloc(1,sizeof(item_def_t));
  item->id = data.id;
  item->category = ITEM_ARMOR;

  item->dr = calloc(1,sizeof(damage_reduction_t));

  item->ability = ABILITY_NONE;
  armor_def_t temp = ARMOR_TEMPLATES[data.equip_type];
  //item->stats[STAT_ARMOR] = InitStat(STAT_ARMOR,0,temp.armor_class, temp.armor_class);

  item->values[VAL_DURI] = InitValue(VAL_DURI,temp.durability);
  item->values[VAL_WEIGHT] = InitValue(VAL_WEIGHT,temp.weight);
  item->values[VAL_WORTH] = InitValue(VAL_WORTH,temp.cost);
  item->values[VAL_ADV_SAVE] = InitValue(VAL_ADV_SAVE,0);
  item->values[VAL_SAVE] = InitValue(VAL_SAVE,temp.armor_class);
  

  *item->dr = temp.dr_base;
  ApplyItemProps(item, data.props, data.et_props);

  //item->weight = temp.weight;

  item->ability = ABILITY_ARMOR_SAVE;
  item->skills[item->num_skills++] = temp.skill; 
 
  for(int i = 0; i < VAL_ALL; i++){
    if(!item->values[i])
      continue;

    item->values[i]->val = ValueRebase(item->values[i]);
  }
 
  return item;
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
    TraceLog(LOG_INFO,"====Restore %s by %i====\n, %s now %i",STAT_STRING[resource].name, amount, STAT_STRING[resource].name, (int)self->owner->stats[resource]->current);
}

void EntKill(stat_t* self, float old, float cur){
  ent_t* e = self->owner;
  SetState(e, STATE_DIE,NULL);
}

void EntInitOnce(ent_t* e){
  EntSync(e);

  EntPollInventory(e);
  //if(e->items[0]
  /*
   * if(e->attack==NULL)
    e->attack = InitBasicAttack(e);
 */

  for(int i = 0; i < STAT_ENT_DONE; i++){
    if(!e->stats[i])
      continue;
   
    if(e->type == ENT_PERSON)
     DO_NOTHING();

      StatMaxOut(e->stats[i]);
  }

  cooldown_t* spawner = InitCooldown(3,EVENT_SPAWN,StepState_Adapter,e);
  AddEvent(e->events, spawner);
}

void EntPollInventory(ent_t* e){
  for (int i = 0; i < CARRY_SIZE; i++){
    if(!e->gear[i])
      break;
    if(!e->gear[i]->equipped)
      continue;

    if(e->gear[i]->on_equip)
      e->gear[i]->on_equip(e,e->gear[i]);
  
  }
}

bool ItemApplyStats(struct ent_s* owner, item_t* item){
  /*
  for(int i = 0; i < STAT_DONE; i++){
    if(item->def->stats[i])
      StatExpand(owner->stats[i],item->def->stats[i]->current,true);
  }
  */
}

item_t* EntGetItem(ent_t* e, ItemCategory cat, bool equipped){
  for (int i = 0; i < CARRY_SIZE; i++){
    if(!e->gear[i])
      break;

    if(e->gear[i]->equipped && e->gear[i]->def->category == cat)
      return e->gear[i];
  }

  return NULL;
}

bool EntAddItem(ent_t* e, item_t* item, bool equip){
  for(int i = 0; i < CARRY_SIZE; i++){
    if(e->gear[i])
      continue;

    item->equipped = equip;
    e->gear[i] = item;

    item->owner = e;

    /*if(item->def->ability>ABILITY_NONE)
      ItemAddAbility(e,item);
*/
    e->num_items++;
    return true;
  }

  return false;
}

void EntAddExp(ent_t *e, int exp){
  SkillIncrease(e->skills[SKILL_LVL], exp);
}

void EntDestroy(ent_t* e){
  if(!e || !SetState(e, STATE_END,NULL))
    return;

  MapRemoveOccupant(e->map,e->pos);

  if(e->sprite!=NULL){
    e->sprite->owner = NULL;
    SpriteSetAnimState(e->sprite,ANIM_KILL);
  }

  e->control = NULL;
}

ability_t* InitAbilityDummy(ent_t* owner, ability_t copy){
  ability_t* a = calloc(1,sizeof(ability_t));
  *a = copy;

  return a;
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
int AbilityAddPB(ent_t* e, ability_t* a, StatType s){
  if(e->skills[a->skills[0]]->val <= SR_NONE)
    return 0;

  return a->stats[s]->current;
}

ability_sim_t* AbilitySimDmg(ent_t* owner,  ability_t* a, ent_t* target){
  ability_sim_t* res = calloc(1,sizeof(ability_sim_t));

  res->id = a->id;
  res->type = a->type;
  res->d_type = a->school;
  res->d_bonus = AbilityAddPB(owner, a, STAT_DAMAGE);
  res->dmg_die = a->dc->num_die;
  res->dmg_sides = a->dc->sides;
  res->penn = a->values[VAL_PENN]->val;

  res->hit_calc = a->hit->roll(a->hit, res->hit_res);
  res->dmg_calc = a->dc->roll(a->dc, res->dmg_res);

  res->final_dmg = res->dmg_calc + res->d_bonus;
  return res;
}

InteractResult EntTarget(ent_t* e, ability_t* a, ent_t* source){
  InteractResult result = IR_NONE;
  //int base_dmg = a->dc->roll(a->dc);
  

  ability_sim_t* dummy = a->sim_fn(source, a, e);

  int og_dmg = dummy->final_dmg;
  ability_t* dr = EntFindAbility(e, ABILITY_ARMOR_DR);

  InteractResult dres = IR_FAIL;
  if(dr)
    dres = AbilityUse(e, dr, source, dummy);

  if(dres == IR_TOTAL_SUCC){
    TraceLog(LOG_INFO,"%s shruggs off the attack..",e->name);

    result = IR_CRITICAL_FAIL;
  }
  else{
    int damage = -1 * dummy->final_dmg; 
    e->last_hit_by = source; 

    AggroAdd(e->aggro, source, -1*damage, source->props->base_diff);
    if(StatChangeValue(e,e->stats[a->damage_to], damage)){
      TraceLog(LOG_INFO,"%s level %i hits %s with %i %s damage\n %s %s now %0.0f/%0.0f",
          source->name, 
          source->skills[SKILL_LVL]->val,
          e->name,
          damage*-1,
          DAMAGE_STRING[a->school],
          e->name,
          STAT_STRING[a->damage_to].name,
          e->stats[STAT_HEALTH]->current,e->stats[STAT_HEALTH]->max);

      result = IR_SUCCESS;
      if(StatIsEmpty(e->stats[STAT_HEALTH]))
        result = IR_TOTAL_SUCC;

      if(og_dmg<dummy->final_dmg)
        TraceLog(LOG_INFO,"(%i damage reduction)",og_dmg-dummy->final_dmg); 

      return result;
    }
    else
      result = IR_FAIL;
  }

  return result;
}

int EntDamageReduction(ent_t* e, ability_t* a, int dmg){

  DamageTag tag = DamageTypeTags[a->school];
  
  dmg -= e->traits->resistances_school[a->school];

  for(int i = 0; i < e->num_items; i++){
    item_t* item = e->gear[i];
    if(item==NULL)
      continue;

    if(!item->equipped)
      continue;

    if(item->def->dr == NULL)
      continue;
    if(item->def->dr->resist_types[a->school] > 0){
      dmg-=item->def->dr->resist_types[a->school];   
    }

    if(item->def->dr->resist_tags[tag] > 0){
      dmg-=item->def->dr->resist_tags[tag];
    }

    if(dmg<1){
      dmg =1;
      break;
    }
    
  }

  return dmg;
}

ability_t* EntChooseWeightedAbility(ent_t* e, int budget, ActionSlot slot){
  bool running = false;
  int count = e->slots[slot]->count;
  if(count == 0)
    return NULL;

  choice_pool_t* p = StartChoice(e->control->choices[slot], count, ChooseByBudget,&running);

  p->budget = budget;
  
  if(!running){
    for(int j = 0; j < e->slots[slot]->count; j++){
      ability_t* a = e->slots[slot]->abilities[j];
      AddChoice(p, a->cost, a);

    }
  }

  choice_t* choice = p->choose(p);
  if(choice && choice->context)
    return choice->context;

  return NULL;
}

bool ItemAddAbility(struct ent_s* owner, item_t* item){
  const item_def_t* def = item->def;

  ability_t* a = InitAbility(owner, def->ability);
//  a->cost = def->
  for(int i = 0; i < VAL_WORTH; i++){
    if(def->values[i]==NULL)
      continue;
    if(def->values[i]->val == 0)
      continue;

    switch(i){
      case VAL_ADV_HIT:
      case VAL_ADV_DMG:
      case VAL_ADV_SAVE:
      case VAL_HIT:
      case VAL_SAVE:
        a->values[i]->on_change = ValueUpdateDie;
        break;
      default:
        break;
    }

    AbilityApplyValues(a, def->values[i]);
  }
  a->dr = def->dr;
  for(int i = 0; i < def->num_skills; i++)
    a->skills[a->num_skills++] = def->skills[i];

  if(a->chain){
    a->chain->skills[a->chain->num_skills++] = def->skills[0];

    if(a->chain->type == AT_DR){
      a->chain->dr = def->dr;
      ActionSlotAddAbility(owner,a->chain);
    }
  }

  return ActionSlotAddAbility(owner, a);
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
  bool success = true;
  InteractResult ires = IR_NONE;

  int cr = AggroAdd(e->aggro, target, 1, target->props->base_diff);
  AggroAdd(target->aggro, e, 1, 1);//e->props->base_diff);

  if(a->resource>STAT_NONE && a->cost > 0)
    if(!StatChangeValue(e,e->stats[a->resource],-1*a->cost)){
      TraceLog(LOG_INFO,"%s not enough %s",e->name, STAT_STRING[a->resource].name);
      success = false;
    }

  bool save_roll = false;

  ability_t* save = EntFindAbility(target, ABILITY_ARMOR_SAVE);
  if(save)
    save_roll = AbilityUse(target,save, e, a->sim_fn(e,a,target));
  if(!success || save_roll){
    AggroAdd(target->aggro, e, a->cost, e->props->base_diff);
    TraceLog(LOG_INFO,"%s misses",e->name);
    success = false;
    ires = IR_FAIL;
  }

  if(success){
    TraceLog(LOG_INFO,"%s now %i",STAT_STRING[a->resource].name, (int)e->stats[a->resource]->current);
    a->stats[STAT_DAMAGE]->start(a->stats[STAT_DAMAGE]);

    StatMaxOut(a->stats[STAT_DAMAGE]);
    ires = EntTarget(target, a,e);
  }

  int tiid = target->uid;
/*
  if(e->type == ENT_PERSON)
    tiid = e->map->id; //Prevents just grinding mobs 
*/

  return ires;

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

controller_t* InitController(){
  controller_t* ctrl = malloc(sizeof(controller_t));
  *ctrl = (controller_t){0};

  ctrl->destination = CELL_UNSET;

  return ctrl;
}

ability_t* InitAbility(ent_t* owner, AbilityID id){
  ability_t* a = calloc(1,sizeof(ability_t));

  *a = AbilityLookup(id);

  a->hit = Die(a->hdie,1);

  a->dc = Die(a->side,a->die);

  if(a->use_fn == NULL)
    a->use_fn = EntUseAbility;

  if(a->type == AT_DMG)
   a->sim_fn = AbilitySimDmg;

  a->on_use_cb = AbilitySkillup;

  a->stats[STAT_REACH] = InitStat(STAT_REACH,1,a->reach,a->reach);
  a->stats[STAT_DAMAGE] = InitStatOnMax(STAT_DAMAGE,a->bonus,a->mod);

  for (int i = 0; i < STAT_DONE; i++){
    if(!a->stats[i])
      continue;
    a->stats[i]->owner = owner;
  }

  for(int i = 0; i < VAL_WORTH; i++){
    a->values[i] = InitValue(i,0);
  }

  if(a->chain_id > ABILITY_NONE){
    a->chain = InitAbility(owner, a->chain_id);
    a->chain_fn = EntUseAbility;
  }

  return a;
}

bool AbilitySkillup(ent_t* owner, ability_t* a, ent_t* target, InteractResult result){
  
  aggro_entry_t* e = AggroGetEntry(owner->aggro,target);

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

bool AbilityUse(ent_t* owner, ability_t* a, ent_t* target, ability_sim_t* other){
  InteractResult ires = IR_NONE;
  switch(a->type){
    case AT_DMG:
    case AT_HEAL:
      if(a->use_fn == NULL)
        return false;

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

   if(a->type < AT_SAVE)
   if(a->chain && a->chain_fn)
     a->chain_fn(owner, a->chain, target);
  }

  if(ires >=IR_SUCCESS && a->on_success_cb)
    a->on_success_cb(owner, a, target, ires);
  

  return ires>=IR_SUCCESS?true:false;
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
  int radius = e->stats[STAT_AGGRO]->current/2;

  for (int oct = 0; oct < 8; oct++)
    CastLight(e->map, e->pos, 1, 1.0, 0.0, radius,
              mult[oct][0], mult[oct][1],
              mult[oct][2], mult[oct][3]);

}

void EntTurnSync(ent_t* e){
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

    e->pos = newPos;
    e->facing = CellInc(e->pos,step);
  }
  else
    e->facing = newPos;

  return status;
}

void EntSetCell(ent_t *e, Cell pos){
  e->pos = pos;
}

void EntControlStep(ent_t *e){
  if(!e->control || !e->control->bt || !e->control->bt[e->state])
    return;

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
      return false;
      break;
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
      if(e->sprite)
        e->sprite->is_visible = true;
      break;
    default:
      break;
  }

  switch(s){
    case STATE_DIE:
      EntDestroy(e);
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

  for(int i = 0; i < STAT_ENT_DONE; i++)
    if(e->stats[i] && e->stats[i]->lvl){
      if(i==STAT_ACTIONS)
        continue;
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

  return (e->state < STATE_DIE);
}


SpeciesType GetEntitySpecies(EntityType t){
  if (t >= 0 && t < ENT_DONE)
        return RACE_MAP[t];
    return SPEC_HUMAN; // fallback default

}

int GetWeaponByTrait(Traits t, weapon_def_t *arms){
  int count = 0;
  for (int i = 0; i < WEAP_DONE; i++){
    if(t & WEAPON_TEMPLATES[i].skill != 0)
      continue;
    
    arms[count++] = WEAPON_TEMPLATES[i];
  }

  return count;
}
void ApplyItemProps(item_def_t *w, ItemProps props, uint64_t e_props){
  while(props){
    uint64_t prop = props & -props;
    props &= props - 1;
    for(int i = 0; i < NUM_ITEM_PROPS; i++){
      if(PROP_MODS[ITEM_NONE][i].propID != prop)
        continue;

      item_prop_mod_t mod = PROP_MODS[ITEM_NONE][i];
      for(int i = 0; i < mod.num_aff; i++){
        if(w->values[mod.val_change[i].modifies])
          ValueAddBaseMod(w->values[mod.val_change[i].modifies], mod.val_change[i]);
      }
    }
  }

  while(e_props){
    uint64_t wprop = e_props & -e_props;
    e_props &= e_props -1;

    for(int i = 0; i < NUM_WEAP_PROPS; i++){
      
      if(PROP_MODS[w->category][i].propID != wprop)
        continue;

      item_prop_mod_t mod = PROP_MODS[w->category][i];

      if(mod.add_skill > SKILL_LVL)
        w->skills[w->num_skills++] = mod.add_skill;
      for(int i = 0; i < mod.num_aff; i++){
      if(mod.val_change[i].affix>AFF_NONE)
        ValueAddBaseMod(w->values[mod.val_change[i].modifies], mod.val_change[i]);
      }
    }
  }
}

ItemProps GetItemQualByRaceProp(RaceProp prop){
  //Increments the quality by 1
  switch(prop){
    case RACE_BUILD_CRUDE:
    case RACE_BUILD_SIMPLE:
    case RACE_BUILD_BASIC:
    case RACE_BUILD_SOPH:
    case RACE_ARMS_ARTISAN:
      return PROP_QUAL_TRASH;
      break;
    default:
      break;
  }
}

ItemProps GetItemMatsByRaceProp(RaceProp prop){
  switch(prop){
    case RACE_ARMS_CRUDE:
    case RACE_ARMS_SIMPLE:
      return PROP_MAT_BONE | PROP_MAT_WOOD | PROP_MAT_STONE;
      break;
    case RACE_ARMOR_CRUDE:
      return PROP_MAT_CLOTH | PROP_MAT_LEATHER | PROP_MAT_BONE;
      break;
    case RACE_ARMS_ARTISAN:
      return PROP_WEAP_MARTIAL;
      break;
    default:
      break;
  }
}

ArmorProps GetArmorPropsByRaceProp(RaceProp prop){

}

WeaponProps GetWeaponPropsByRaceProp(RaceProp prop){
  switch(prop){
    case RACE_ARMS_CRUDE:
    case RACE_ARMS_SIMPLE:
      return PROP_WEAP_SIMP;
      break;


  }
}

item_def_t* BuildArmorForMob(ent_t* e, RaceProps props){
  RaceProps a_props = props & RACE_ARMOR_MASK;
}

item_def_t* BuildSpecialForMob(ent_t* e, RaceProps props){
  RaceProps s_props = props & RACE_SPECIAL_MASK;

}

item_def_t* BuildWeaponForMob(ent_t* e, RaceProps props, SkillType sk){
  RaceProps a_props = props & RACE_ARMS_MASK;
  a_props |= props & RACE_BUILD_MASK;
  ItemProps qual_props = 0;//PROP_QUAL_TRASH;
  ItemProps mat_props = PROP_NONE;
  WeaponProps w_props = PROP_NONE;

  while(a_props){
    uint64_t rprop = a_props & - a_props;
    a_props &= a_props -1;
    w_props |= GetWeaponPropsByRaceProp(rprop);
    mat_props |= GetItemMatsByRaceProp(rprop);
    qual_props += GetItemQualByRaceProp(rprop);
  }

  return BuildWeapon(sk, qual_props | mat_props, w_props);
}

item_def_t* BuildAppropriateItem(ent_t* e, ItemCategory cat, SkillType s){
  item_def_t* item;
  RaceProps r_props = GetRaceByFlag(e->props->race).props;

  switch(cat){
    case ITEM_WEAPON:
    item = BuildWeaponForMob(e, r_props, s);
    break;
    case ITEM_ARMOR:
    item = BuildArmorForMob(e, r_props);
    break;
    case ITEM_CONSUMABLE:
    item = BuildSpecialForMob(e, r_props);
    break;
  }

  return item;
}

item_def_t* BuildWeapon(SkillType skill, ItemProps props, WeaponProps w_props){

  WeaponType type = GetWeapTypeBySkill(skill);

  item_def_t* item = DefineWeaponByType(type, props, w_props);

  return item;
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
  int num_acts = e->stats[STAT_ACTIONS]->max;

  for(int i = 0; i < SLOT_ALL; i++){
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

Cell EntSimulateBattle(ent_t* e, ent_t* t, int sims){
  int e_wins = 0;
  int t_wins = 0;
  for(int i = 0; i < sims; i++){
    float e_hp = e->stats[STAT_HEALTH]->max;
    float t_hp = t->stats[STAT_HEALTH]->max;

    int e_rnds = 0;
    int t_rnds = 0;

    while(e_hp > 0){
      t_rnds++;
      e_hp-=EntGetDPR(t);

    }

    while(t_hp > 0){
      e_rnds++;
      t_hp-=EntGetDPR(e);

    }
    e_wins+= e_rnds<t_rnds?1:0;
    t_wins+= t_rnds<e_rnds?1:0;
    //if(e_rnds == t_rnds)
    //
      //e_wins++;
  }

  return CELL_NEW(e_wins,t_wins);
}

int EntGetChallengeRating(ent_t* e, ent_t* t){
  Cell results = EntSimulateBattle(e,t, MAX_SKILL_GAIN);

  return MAX_SKILL_GAIN-results.x;
}
