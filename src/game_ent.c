#include "game_types.h"
#include "game_tools.h"
#include "game_helpers.h"
#include "game_math.h"
#include "game_process.h"

MAKE_ADAPTER(StepState, ent_t*);

ent_t* InitEnt(ObjectInstance data,Cell pos){
  ent_t* e = malloc(sizeof(ent_t));
  *e = (ent_t){0};  // zero initialize if needed
  e->type = data.id;

  e->size = data.size;
  e->map = WorldGetMap();
  e->pos = pos;
  e->facing = CELL_EMPTY;
  e->sprite = InitSpriteByID(data.id,SHEET_ENT);
  e->sprite->owner = e;

  strcpy(e->name,data.name);
  e->events = InitEvents();

  e->control = InitController();

  EntCalcStats(e);
  e->stats[STAT_HEALTH]->on_stat_empty = EntKill;
  e->stats[STAT_ACTIONS]->on_stat_empty = EntActionsTaken;
  
  InitActions(e->actions);
  e->actions[ACTION_MOVE] = InitAction(ACTION_MOVE, DES_FACING, NULL,NULL);
  e->actions[ACTION_ATTACK] = InitAction(ACTION_ATTACK, DES_FACING, ActionAttack,NULL);
  e->actions[ACTION_MAGIC] = InitAction(ACTION_MAGIC, DES_MULTI_TARGET, ActionMultiTarget,NULL);

  for(int i = 0; i < GEAR_DONE; i++){
    if(data.items[i]==GEAR_NONE)
      break;

    item_def_t* item = GetItemDefByID(data.items[i]);
    EntAddItem(e, InitItem(item), true);
  }

  for(int i = 0; i < NUM_ABILITIES; i++){
    if(data.abilities[i] == ABILITY_DONE)
      break;
  
    ability_t* a = InitAbility(e, data.abilities[i]);
    e->abilities[e->num_abilities++] = a;

    if(a->chain > ABILITY_NONE){
      ability_t* child = InitAbility(e, a->chain);
      child->weight = -1;

      a->on_success_fn = EntUseAbility;
      a->on_success = child;
      e->abilities[e->num_abilities++] = child;
    }
  }

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
  e->type = mob;

  ObjectInstance data = GetEntityData(mob);
  e->size = data.size;
  e->map = WorldGetMap();
  e->pos = pos;
  e->facing = CELL_EMPTY;
  e->sprite = InitSpriteByID(data.id,SHEET_ENT);
  e->sprite->owner = e;

  strcpy(e->name,data.name);
  e->events = InitEvents();

  e->control = InitController();
  e->control->start = pos;
  EntCalcStats(e);
  e->stats[STAT_HEALTH]->on_stat_empty = EntKill;

  e->challenge = data.cr;
  InitActions(e->actions);

  for(int i = 0; i < NUM_ABILITIES; i++){
    if(data.abilities[i] == ABILITY_DONE)
      break;

    ability_t* a = InitAbility(e, data.abilities[i]);
    e->abilities[e->num_abilities++] = a;

    if(a->chain > ABILITY_NONE){
      ability_t* child = InitAbility(e, a->chain);
      child->weight = -1;

      a->on_success_fn = EntUseAbility;
      a->on_success = child;
      e->abilities[e->num_abilities++] = child;
    }
  }

  for(int i = 0; i < GEAR_DONE; i++){
    if(data.items[i]==GEAR_NONE)
      break;

    int budget = data.budget*data.cr;

    item_def_t* item = GetItemDefByID(data.items[i]);
    EntAddItem(e, InitItem(item), true);
  }

  e->control->ranges[RANGE_NEAR] = (int)e->stats[STAT_AGGRO]->current/4;
  e->control->ranges[RANGE_LOITER] = (int)e->stats[STAT_AGGRO]->current/2;
  e->control->ranges[RANGE_ROAM] = 4;
  for (int i = STATE_SPAWN; i < STATE_END; i++){
    if(data.behaviors[i] == BEHAVIOR_NONE)
      continue;
    e->control->bt[i] = InitBehaviorTree(data.behaviors[i]);
  }

  e->actions[ACTION_MOVE] = InitAction(ACTION_MOVE, DES_NONE, ActionTraverseGrid,NULL);
  e->actions[ACTION_ATTACK] = InitAction(ACTION_ATTACK, DES_FACING, ActionAttack,NULL);

  SetState(e,STATE_SPAWN,NULL);
  return e;
}

ent_t* InitEntByRaceClass(uint64_t class_id, SpeciesType race){
  define_race_class_t arch = RACE_CLASS_DEFINE[race][__builtin_ctzll(class_id)];

  race_define_t racial = DEFINE_RACE[race];

  define_archetype_t data = CLASS_DATA[__builtin_ctzll(arch.base)];

  ObjectInstance todoremove = GetEntityData(racial.base_ent);

  ent_t* e = malloc(sizeof(ent_t));
  *e = (ent_t){0};  // zero initialize if needed

  e->type = racial.base_ent;

  strcpy(e->name, TextFormat("%s %s",racial.name,arch.b_name));
  e->size = todoremove.size;
  e->map = WorldGetMap();
  e->pos = CELL_UNSET;
  e->facing = CELL_UNSET;
  e->sprite = InitSpriteByID(e->type,SHEET_ENT);
  e->events = InitEvents();

  e->control = InitController();

  e->skills[SKILL_LVL] = InitSkill(SKILL_LVL,e,1,20);

  e->traits = calloc(1,sizeof(traits_t));

  e->skills[SKILL_LVL]->on_skill_up = EntMonsterOnLevelUp;
  MobCategory cat = GetEntityCategory(e->type);

  category_stats_t base = CATEGORY_STATS[cat];
  size_category_t size = MOB_SIZE[cat];


  trait_pool_t res[4] = {
    {TRAIT_RESIST_SCHOOL_MASK, GetTraits( racial.traits, TRAIT_RESIST_SCHOOL_MASK)},
    {TRAIT_RESIST_TAG_MASK,GetTraits( racial.traits, TRAIT_RESIST_TAG_MASK)},
    {TRAIT_RESIST_SCHOOL_MASK, GetTraits( base.traits, TRAIT_RESIST_SCHOOL_MASK)},
    {TRAIT_RESIST_TAG_MASK,GetTraits( base.traits, TRAIT_RESIST_TAG_MASK)},
  };
  for (int i = 0; i< 4;i++)
    EntApplyTraits(e->traits,res[i].mask, res[i].shift);

  for (int i = 0; i < ATTR_DONE; i++){
    int val = base.attr[i] + size.attr[e->size][i];
    e->attribs[i] = InitAttribute(i,val);
    e->attribs[i]->asi = data.ASI[i];
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

  return e;
}

int EntBuild(mob_define_t def, MobRules rules, ent_t **pool){
  int count = 0;
  MobRules tmp = def.rules&=rules;
  rules&=tmp;

  race_define_t racial = DEFINE_RACE[def.race];
  MobRules spawn = GetMobRulesByMask(rules,MOB_SPAWN_MASK);
  MobRules group = GetMobRulesByMask(rules,MOB_GROUPING_MASK);
  MobRules modif = GetMobRulesByMask(rules,MOB_MOD_MASK);

  int class_weight[7] = {0};
  
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
        default:
          break;
      }
    }
  }

  bool diverse=false,pat=false,suprise=false,elite=false;
  int beef = 0;
  if(spawn > 0){
    while(spawn){
      uint64_t stype = spawn & -spawn;
      spawn &= spawn -1;

      switch(stype){
        case MOB_SPAWN_TRAP:
        case MOB_SPAWN_SECRET:
          suprise = true;         
          break;
        case MOB_SPAWN_LAIR:
          elite = true;
          break;
        case MOB_SPAWN_CHALLENGE:
          beef = 1;
          diverse = true;
          break;
        case MOB_SPAWN_CAMP:
          count+=2;
          diverse = true;
          beef = 1;
          arm = true;
          don =true;
          break;
        case MOB_SPAWN_PATROL:
          diverse = true;
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

  bool swarm=false,outfit=false;
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
          if (max < 3)
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

  int amount = CLAMP(RandRange(min,max+count),1,MOB_ROOM_MAX);
  if(amount>2)
    monster_size =0;

  int chief_w = 0, captain_w = 0, commander_w =0;
  int magicians = 0, fighters=0, ranged=0,shock=0, medics=0;

  if(diverse && amount>1){
    RaceProps tactics = GET_FLAG(racial.props, RACE_TACTICS_MASK);
    RaceProps classes = GET_FLAG(racial.props, RACE_CLASS_MASK);
    
    while(tactics){
      uint64_t tactic = tactics & -tactics;
      tactics &= tactics-1;

      switch(tactic){
        case RACE_TACTICS_ARCANA:
          magicians += 5;
          medics +=2;
          if(amount>4)
            chief_w= amount*12;
          break;
        case RACE_TACTICS_MARTIAL:
          fighters +=5;
          ranged +=5;
          shock +=3;
          if(amount>3)
            captain_w += amount*10;
          if(amount>7)
            commander_w += amount*7;
          break;
        case RACE_TACTICS_SIMPLE:
          fighters+=2;
          break;
        case RACE_TACTICS_CRUDE:
          fighters+=2;
          break;
        case RACE_TACTICS_RANKS:
          fighters++;
          shock +=2;
          ranged+=2;
          if(amount>4){
            medics+=amount;
            captain_w+=2;
          }
        default:
          break;
      }
    }

    const define_race_class_t *rcw = RACE_CLASS_DEFINE[def.race];
    while(classes){
      uint64_t class = classes & -classes;
      classes &= classes -1;
      class_weight[__builtin_ctzll(class)]=rcw[__builtin_ctzll(class)].weight;
    }
  }

  if(monster_size > 0)
    class_weight[__builtin_ctzll(racial.brute)] *= monster_size*100;
  
  class_weight[__builtin_ctzll(racial.healer)] *= medics;
  class_weight[__builtin_ctzll(racial.shock)] += shock;
  class_weight[__builtin_ctzll(racial.magician)] *= magicians;
  class_weight[__builtin_ctzll(racial.chief)] += chief_w;
  class_weight[__builtin_ctzll(racial.captain)] += captain_w;
  class_weight[__builtin_ctzll(racial.commander)] += commander_w;
  class_weight[__builtin_ctzll(racial.ranged)] *= ranged;
  class_weight[__builtin_ctzll(racial.soldier)] *= fighters;

  //TODO REMOVE
  beef++;
  count = 0;
  for(int i = 0; i < amount; i++){
    int selection = weighted_choice(class_weight,7);
    ent_t* e = InitEntByRaceClass(BIT64(selection),def.race);

    for(int j = 0; j < beef; j++)
      EntAddExp(e, 300);

    class_weight[selection]-=amount;
    pool[count++] = e;
  }
  return count;
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

void EntCalcStats(ent_t* e){
  
  e->skills[SKILL_LVL] = InitSkill(SKILL_LVL,e,1,20);

  e->traits = calloc(1,sizeof(traits_t));

  e->skills[SKILL_LVL]->on_skill_up = EntOnLevelUp;
  MobCategory cat = GetEntityCategory(e->type);

  SpeciesType species = GetEntitySpecies(e->type);

  category_stats_t base = CATEGORY_STATS[cat];
  species_stats_t racial = RACIALS[species];
  size_category_t size = MOB_SIZE[cat];


  trait_pool_t res[4] = {
    {TRAIT_RESIST_SCHOOL_MASK, GetTraits( racial.traits, TRAIT_RESIST_SCHOOL_MASK)},
    {TRAIT_RESIST_TAG_MASK,GetTraits( racial.traits, TRAIT_RESIST_TAG_MASK)},
    {TRAIT_RESIST_SCHOOL_MASK, GetTraits( base.traits, TRAIT_RESIST_SCHOOL_MASK)},
    {TRAIT_RESIST_TAG_MASK,GetTraits( base.traits, TRAIT_RESIST_TAG_MASK)},
  };
  for (int i = 0; i< 4;i++)
    EntApplyTraits(e->traits,res[i].mask, res[i].shift);

  /*
  for (uint64_t i = 1ULL; i < (1ULL << TRAIT_DONE); i<<=1){
    if(IS_TRAIT(base.traits,TRAIT_RESIST_TAG_MASK, i)||
        IS_TRAIT(racial.traits,TRAIT_RESIST_TAG_MASK, i))
      e->traits->resistances_type[i]+=1;
    if(IS_TRAIT(base.traits,TRAIT_RESIST_SCHOOL_MASK, i)||
        IS_TRAIT(racial.traits,TRAIT_RESIST_SCHOOL_MASK, i))
      e->traits->resistances_school[i]+=1;
  }
*/
  for (int i = 0; i < ATTR_DONE; i++){
    int val = base.attr[i] + racial.attr[i] + size.attr[e->size][i];
    e->attribs[i] = InitAttribute(i,val);
  }
  for (int i = 0; i < STAT_ENT_DONE;i++){
    int val = base.stats[i] + racial.stats[i] + size.stats[e->size][i];
    e->stats[i] = InitStat(i,0,val,val);

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
}

item_t* InitItem(item_def_t* def){
  item_t* item = malloc(sizeof(item_t));

  *item = (item_t){
    .def = def
  };

  if(item_funcs[def->category].cat != ITEM_NONE){
    if(item_funcs[def->category].on_equip)
      item->on_equip = item_funcs[def->category].on_equip;
  }

  return item;
}

item_pool_t* InitItemPool(void) {
    item_pool_t* ip = calloc(1, sizeof(item_pool_t));
    ip->size = 0;
    return ip;
}

item_def_t* DefineItem(ItemInstance data){
  
  switch(data.cat){
    case ITEM_ARMOR:
      return DefineArmor(data);
      break;
    case ITEM_WEAPON:
      return DefineWeapon(data);
    default:
      return NULL;
      break;
  }
  /*
  item->damage =data.damage;

  for (int i = 0; i < STAT_DONE; i++){
    if(data.stats[i] == 0){
      item->stats[i] = 0;
      continue;
    }

    item->stats[i] = InitStat(i, 1, data.stats[i], data.stats[i]);
  }
  */
}

item_def_t* DefineWeapon(ItemInstance data){
  item_def_t* item = calloc(1,sizeof(item_def_t));
  item->id = data.id;
  item->category = ITEM_WEAPON;

  weapon_def_t temp = WEAPON_TEMPLATES[data.equip_type];

  item->weight = temp.weight;

  item->damage = temp.dtype;

  item->stats[STAT_DAMAGE] = InitStat(STAT_DAMAGE,0,temp.stats[STAT_DAMAGE],temp.stats[STAT_DAMAGE]);
  item->stats[STAT_REACH] = InitStat(STAT_REACH,0,temp.stats[STAT_REACH],temp.stats[STAT_REACH]);
  if(temp.stats[STAT_STAMINA] > 0)
    item->stats[STAT_STAMINA] = InitStat(STAT_STAMINA,0,temp.stats[STAT_STAMINA],temp.stats[STAT_STAMINA]);
  if(temp.stats[STAT_ENERGY] > 0)
    item->stats[STAT_ENERGY] = InitStat(STAT_ENERGY,0,temp.stats[STAT_ENERGY],temp.stats[STAT_ENERGY]);
  
  
  ItemApplyWeaponProps(item, &temp, data.rarity);

  return item;
}

item_def_t* DefineArmor(ItemInstance data){
  item_def_t* item = calloc(1,sizeof(item_def_t));
  item->id = data.id;
  item->category = ITEM_ARMOR;

  item->dr = calloc(1,sizeof(damage_reduction_t));

  armor_def_t temp = ARMOR_TEMPLATES[data.equip_type];
  item->stats[STAT_ARMOR] = InitStat(STAT_ARMOR,0,temp.armor_class, temp.armor_class);

  item->weight = temp.weight;
  *item->dr = temp.dr_base;

  //item->ability = temp.ability;
  
  return item;
}

void EntResetRegen(stat_t* self, float old, float cur){

  StatType rel = STAT_NONE;

  switch(self->attribute){
    case STAT_STAMINA:
      rel = STAT_STAMINA_REGEN_RATE;
      break;
    case STAT_ENERGY:
      rel = STAT_ENERGY_REGEN_RATE;
      break;
    default:
      break;

  }  
  if(rel > STAT_NONE)
  StatRestart(self->owner->stats[rel],0,0);

}

void EntRestoreResource(stat_t* self, float old, float cur){
  StatEmpty(self);

  StatType resource = STAT_NONE;
  int amount = 0;
  switch(self->attribute){
    case STAT_STAMINA_REGEN_RATE:
      amount = self->owner->stats[STAT_STAMINA_REGEN]->current;
      resource = STAT_STAMINA;
      break;
    case STAT_ENERGY_REGEN_RATE:
      amount = self->owner->stats[STAT_ENERGY_REGEN]->current;
      resource = STAT_ENERGY;
      break;
    default:
      break;
  }

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
    
    if(i<STAT_START_FULL)
      StatMaxOut(e->stats[i]);
    else
      StatEmpty(e->stats[i]);
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
  for(int i = 0; i < STAT_DONE; i++){
    if(item->def->stats[i])
      StatExpand(owner->stats[i],item->def->stats[i]->current,true);
  }
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

    if(item->def->ability>ABILITY_NONE)
      ItemAddAbility(e,item);

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

bool EntTarget(ent_t* e, ability_t* a, ent_t* source){
  int base_dmg = a->dc->roll(a->dc);
  
  base_dmg = (base_dmg + a->stats[STAT_DAMAGE]->current);
  int reduced =  EntDamageReduction(e,a,base_dmg); 
  int damage = -1 * reduced; 
  e->last_hit_by = source; 
  if(StatChangeValue(e,e->stats[a->damage_to], damage)){
   TraceLog(LOG_INFO,"%s hits %s with %i %s damage\n %s %s now %0.0f/%0.0f",
       source->name, e->name,
       damage*-1,
       DAMAGE_STRING[a->school],
       e->name,
       STAT_STRING[a->damage_to].name,
       e->stats[STAT_HEALTH]->current,e->stats[STAT_HEALTH]->max);
  
  if(reduced!=base_dmg)
   TraceLog(LOG_INFO,"(%i damage reduction)",base_dmg-reduced); 
    return true;
  }
  
  return false;

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

ability_t* EntChooseWeightedAbility(ent_t* e, int budget){
   int count = 0;

    // Temporary filtered list
    ability_t* allowed[NUM_ABILITIES];

    // 1. Filter abilities by budget
    for (int i = 0; i < e->num_abilities; i++) {
        ability_t* abil = e->abilities[i];

        if (abil->cost <= budget)
            allowed[count++] = abil;
    }

    // No ability fits the budget → fallback
    if (count == 0)
        return NULL;

    // 2. Weighted roll
    int total = 0;
    for (int i = 0; i < count; i++)
        total += allowed[i]->weight;

    int r = GetRandomValue(1, total);

    // 3. Select based on weight
    for (int i = 0; i < count; i++) {
        r -= allowed[i]->weight;
        if (r <= 0)
            return allowed[i];
    }

    return allowed[count - 1]; // safety
}

bool ItemAddAbility(struct ent_s* owner, item_t* item){
  ability_t* a = malloc(sizeof(ability_t));

  memset(a,0,sizeof(ability_t));

  const item_def_t* def = item->def;

  a->dc = Die(1,def->stats[STAT_DAMAGE]->current);
  a->hit = Die(20,1);

  memset(a->stats, 0, sizeof(a->stats));

  a->cost = def->weight;
  a->stats[STAT_REACH] = def->stats[STAT_REACH];
  a->stats[STAT_DAMAGE] = InitStat(STAT_DAMAGE, 0,0,0);

  a->damage_to = STAT_HEALTH;

  a->stats[STAT_ENERGY] = def->stats[STAT_ENERGY];
  a->stats[STAT_STAMINA] = def->stats[STAT_STAMINA];

  a->school = def->damage;

  for (int i = 0; i < STAT_DONE; i++){
    if(!a->stats[i])
      continue;
    if((i == STAT_ENERGY && a->stats[i]->max > 0)||
        i == STAT_STAMINA && a->stats[i]->max > 0){
      a->resource = i;
    }
    a->stats[i]->owner = owner;
  }

  a->cost = a->stats[a->resource]->max;
  owner->abilities[owner->num_abilities++] = a;
}

bool EntUseAbility(ent_t* e, ability_t* a, ent_t* target){
  int hit = a->hit->roll(a->hit);
  int save = target->stats[STAT_ARMOR]->current;
  if(a->save > ATTR_NONE)
    save = target->attribs[a->save]->val;

  if(a->resource>STAT_NONE)
    if(!StatChangeValue(e,e->stats[a->resource],-1*a->cost)){
      TraceLog(LOG_INFO,"%s not enough %s",e->name, STAT_STRING[a->resource].name);
      return false;
    }
  if (hit < save){
    TraceLog(LOG_INFO,"%s misses",e->name);
    return false;
  }

  a->stats[STAT_DAMAGE]->start(a->stats[STAT_DAMAGE]);

  StatMaxOut(a->stats[STAT_DAMAGE]);

  bool success = EntTarget(target, a,e);
  if(success && a->on_success_fn)
    a->on_success_fn(e, a->on_success,target);

  return success;

}

bool FreeEnt(ent_t* e){
  if(!e)
    return false;

  free(e);
  return true;
}

controller_t* InitController(){
  controller_t* ctrl = malloc(sizeof(controller_t));
  *ctrl = (controller_t){0};

  ctrl->destination = CELL_UNSET;

  return ctrl;
}

ability_t* InitWeaponAttack(ent_t* owner, item_t* w){
  ability_t* a = malloc(sizeof(ability_t));

  const item_def_t* def = w->def;
  
  a->dc = Die(1,def->stats[STAT_DAMAGE]->current);
  a->hit = Die(20,1);
  
  memset(a->stats, 0, sizeof(a->stats));

  a->school = def->damage;

  a->stats[STAT_REACH] = def->stats[STAT_REACH];
  a->stats[STAT_DAMAGE] = InitStatOnMax(STAT_DAMAGE, 0, ATTR_STR);

  for (int i = 0; i < STAT_DONE; i++){
    if(!a->stats[i])
      continue;
    a->stats[i]->owner = owner;
  }
  return a;
}

ability_t* InitAbility(ent_t* owner, AbilityID id){
  ability_t* a = malloc(sizeof(ability_t));

  *a = AbilityLookup(id);

  a->hit = Die(a->hdie+20,1);

  a->dc = Die(a->side,a->die);

  a->stats[STAT_REACH] = InitStat(STAT_REACH,1,a->reach,a->reach);
  a->stats[STAT_DAMAGE] = InitStatOnMax(STAT_DAMAGE,a->bonus,a->mod);

  for (int i = 0; i < STAT_DONE; i++){
    if(!a->stats[i])
      continue;
    a->stats[i]->owner = owner;
  }

  return a;
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
  if(e->control)  
    EntControlStep(e);

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

  switch(s){
    case STATE_NONE:
      return false;
      break;
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
      if(e->last_hit_by){
        challenge_rating_t cr = GetChallengeScore(e->challenge);
     
        if(e->last_hit_by->skills[SKILL_LVL]->val <= cr.out_lvl)
          EntAddExp(e->last_hit_by, cr.exp);
      }
      EntDestroy(e);
      break;
    default:
      break;
  }
}

void EntMonsterOnLevelUp(struct skill_s* self, float old, float cur){
  ent_t* e = self->owner;
  for(int i = 1; i < ATTR_BLANK; i++){
    if(e->attribs[i]->expand) 
    e->attribs[i]->expand(e->attribs[i]);
  }

  for(int i = 0; i < STAT_ENT_DONE; i++)
    if(e->stats[i] && e->stats[i]->lvl){
      if(i==STAT_ACTIONS)
        continue;
      e->stats[i]->lvl(e->stats[i]);
      StatMaxOut(e->stats[i]);
    }

}

void EntOnLevelUp(struct skill_s* self, float old, float cur){
  ent_t* e = self->owner;

  for(int i = 0; i < STAT_DONE; i++)
    if(e->stats[i] && e->stats[i]->lvl){
      if(i==STAT_ACTIONS)
        continue;
      e->stats[i]->lvl(e->stats[i]);
        StatMaxOut(e->stats[i]);
    }

  TraceLog(LOG_INFO,"You have reached level %i",self->val);
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

MobCategory GetEntityCategory(EntityType t){
 if (t >= 0 && t < ENT_DONE)
        return ENTITY_CATEGORY_MAP[t];
    return MOB_HUMANOID; // fallback default
}

SpeciesType GetEntitySpecies(EntityType t){
  if (t >= 0 && t < ENT_DONE)
        return RACE_MAP[t];
    return SPEC_HUMAN; // fallback default

}

ObjectInstance GetEntityData(EntityType t){
  if(t >= 0 && t < ENT_DONE)
    return room_instances[t];
}

item_prop_mod_t GetItemProps(ItemInstance data){
  for(int i = 0; i < PROP_ALL; i++){
    if(PROP_MODS[i].propID == PROP_DONE)
      return PROP_MODS[i];

    if(PROP_MODS[i].propID == data.rarity)
      return PROP_MODS[i];
  }
}

void ItemApplyWeaponProps(item_def_t * w, weapon_def_t* def, ItemQuality rarity){
  item_prop_mod_t* props = calloc(1,sizeof(item_prop_mod_t));
  *props  = PROP_MODS[rarity];

  for (int p = 0; p < PROP_ALL; p++){
    if(!def->props[p])
      continue;

    for(int s = 0; s < STAT_ENT_DONE; s++){
      props->stat_change[s] +=PROP_MODS[p].stat_change[s];
    }

  }

  for (int i = 0; i < STAT_ENT_DONE; i++){
    if(props->stat_change[i] < 1 && w->stats[i]== NULL)
      continue;

    int val = w->stats[i]->max + props->stat_change[i];
    w->stats[i]->max = val;
  }

}

ability_t AbilityLookup(AbilityID id){
  if(id >= ABILITY_NONE && id < ABILITY_DONE)
    return ABILITIES[id];
}
