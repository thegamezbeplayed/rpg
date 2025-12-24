#ifndef __GAME_COMMON__
#define __GAME_COMMON__

#include "raylib.h"
#include "game_tools.h"
#include "game_enum.h"
#include "game_gen.h"
#include "game_define.h"
#include "game_lists.h"

#define MAX_SKILL_GAIN 200
#define CELL_WIDTH 16
#define CELL_HEIGHT 16

#define GRID_WIDTH 128
#define GRID_HEIGHT 128
#define MAX_MAP_SIZE 128

#define RATIO(s) ((s)->ratio((s)))

#define DIE(num,side) (dice_roll_t){(side),(num),RollDie}

static inline bool RaceHasAbility(AbilityID id, SpeciesType race){
  return (CLASS_ABILITIES[id].racial & race) !=0;
}
static inline bool ClassHasAbility(AbilityID id, Archetype arch){
  return (CLASS_ABILITIES[id].base & arch) !=0;
}

static inline int FilterAbilities(AbilityID *pool, Archetype arch, SpeciesType race){
  int count = 0;
  for(int i = 0; i < ABILITY_DONE; i++){
    if(!ClassHasAbility(i, arch))
      continue;
    if(!RaceHasAbility(i, race))
      continue;

    pool[count++] = i;
  }
  return count;
}

static inline int MobGetFrequencyTier(uint64_t rules)
{
    uint64_t f = rules & MOB_FREQ_MASK;
    if (!f) return -1; // none or invalid

    return __builtin_ctzll(f) - 32;
}

static inline mob_define_t GetMobByFrequencyLTE(uint64_t rules)
{
    int reqTier = MobGetFrequencyTier(rules);

    mob_define_t best = MONSTER_MASH[0]; // fallback
    for (int i = 0; i < ENT_DONE; i++) {
        int mobTier = MobGetFrequencyTier(MONSTER_MASH[i].rules);
        if (mobTier < 0) continue;

        if (mobTier <= reqTier)
            return MONSTER_MASH[i];
    }
    return best;
}

static inline bool MobFitsRule(EntityType t, uint64_t rule) {
  int reqTier = MobGetFrequencyTier(rule);

  int mobTier = MobGetFrequencyTier(MONSTER_MASH[t].rules);

  if (mobTier <= reqTier)
    return true;

  return false;
}

static inline bool MobHasRule(EntityType t, uint64_t rule) {
    return (MONSTER_MASH[t].rules & rule) != 0;
}

static inline bool MobHasAllRules(EntityType t, uint64_t rules) {
    return ( (MONSTER_MASH[t].rules & rules) == rules );
}

static inline bool MobHasAnyRules(EntityType t, uint64_t rules) {
    return (MONSTER_MASH[t].rules & rules) != 0;
}

static inline mob_define_t GetMobByRules(uint64_t rules) {
    for (int i = 0; i < ENT_DONE; i++) {
        if (MobHasAllRules(i, rules))
            return MONSTER_MASH[i];
    }
    return MONSTER_MASH[0]; // NONE
}

static inline int FilterMobsInRules(uint64_t rules, mob_define_t* in, int size, mob_define_t *pool) {
    int count = 0;


    for (int i = 0; i < size; i++) {
      if(MobFitsRule(in[i].id,rules))
        pool[count++] = in[i];
    }
    return count;

}
 
static inline int FilterMobsByRules(uint64_t rules, mob_define_t* in, int size, mob_define_t *pool) {
    int count = 0;

    for (int i = 0; i < size; i++) {

        if (!MobHasAnyRules(in[i].id, rules))
          continue;

        pool[count++] = in[i];
    }
    return count;
}

static inline int GetMobsByRules(uint64_t rules, mob_define_t *pool) {
    int count = 0;

    for (int i = 0; i < ENT_DONE; i++) {
        if (!MobHasAnyRules(i, rules))
          continue;

        pool[count++] = MONSTER_MASH[i];
    }
    return count;
}

static inline int GetMobRulesInBudget(int budget, uint64_t rules, MobRule mask, uint64_t *out, int shift){

  int count = 0;
  uint64_t masked = rules & mask;

  while (masked){
    uint64_t rule = masked & -masked;
    masked &= -1;

    if((rule>>shift) > budget)
      return count;

    *out|rule;
  }

  return count;
}

static inline MobRules GetMobRulesByMask(uint64_t rules, MobRule mask){
  return rules & mask;
}

static inline bool ProfHasAnyRules(Profession p, uint64_t rules) {
    return (DEFINE_PROF[p].rules & rules) != 0;
}

static inline int FilterProfsByRules(uint64_t rules, define_prof_t* in, int size, define_prof_t *pool) {
    int count = 0;

    for (int i = 0; i < size; i++) {

      if (!ProfHasAnyRules(in[i].id, rules))
        continue;

      pool[count++] = in[i];
    }
    return count;
}

static inline int GetProfessionsBySociety(SocietyType s, define_prof_t *pool) {
    int count = 0;

    for (int i = 0; i < PROF_END; i++) {
        if (DEFINE_PROF[i].social_weights[s]==0)
          continue;
    
        pool[count++] = DEFINE_PROF[i];
    }
    return count;
}

struct dice_roll_s;
typedef int (*DiceRollFunction)(struct dice_roll_s* d);
typedef bool (*DiceCompareCallback)(int a, int b);
typedef struct dice_roll_s{
  int                 sides,num_die, advantage;
  DiceCompareCallback cb;
  DiceRollFunction roll;
}dice_roll_t;

int DieMax(dice_roll_t* d);
int RollDie(dice_roll_t* d);
int RollDieAdvantage(dice_roll_t* d);
dice_roll_t* Die(int side, int num);
dice_roll_t* InitDie(int side, int num, int adv, DiceRollFunction fn);
typedef struct env_s env_t;
static inline bool LESS_THAN(int a, int b){
  return a<b;
}

static inline bool GREATER_THAN(int a, int b){
  return a>b;
}

static inline bool EQUAL_TO(int a, int b){
  return a==b;
}

struct attribute_s;
typedef bool (*AttributeCallback)(struct attribute_s *a);

typedef struct attribute_s{ 
  int                 val,min,max,cap;
  float               rollover,asi;
  bool                event[ASI_DONE];
  AttributeCallback   expand;
  int                 development;
}attribute_t;

bool AttributeScoreIncrease(attribute_t* a);
bool AttributeScoreEvent(attribute_t* a);
static void AttributeReset(attribute_t* a, int val){
  a->max = a->min = a->val = val;
}
static bool AttributeMaxOut(attribute_t* a){
  if(a->val < a->max){
    a->val = a->max;
    return true;
  }
  else
    return false;
}

static AsiEvent GetAsiEventForLevel(int lvl) {
    if (lvl == 20) return ASI_LVL_CAP;
    if (is_perfect_square(lvl)) return ASI_LVL_SQUARE;
    if (is_prime(lvl)) return ASI_LVL_PRIME;
    if (lvl % 2 == 0) return ASI_LVL_EVEN;
    if (is_composite(lvl)) return ASI_LVL_COMPOSITE;
    if (lvl == 1) return ASI_INIT;
    return ASI_LVL_EVERY; // fallback
}
attribute_t* InitAttribute(AttributeType type, int val);

typedef bool (*ActionKeyCallback)(struct ent_s* e, ActionType a, KeyboardKey k,ActionSlot slot);

typedef struct{
  ActionType        action;
  int               num_keys;
  KeyboardKey       keys[8];
  ActionKeyCallback fn;
  int               binding;
}action_key_t;

typedef bool (*OnActionCallback)(struct ent_s* e, ActionType a);
typedef bool (*TakeActionCallback)(struct ent_s* e, ActionType a, OnActionCallback cb);

typedef struct{
  DesignationType   type;
  union {
    Cell* tile;
    struct ent_s* mob;
  } target;
}action_target_t;

typedef struct{
  bool                on_deck;
  ActionType          action;
  DesignationType     targeting;
  void*               context;
  int                 num_targets;
  action_target_t*    targets[5];
  TakeActionCallback  fn;
  OnActionCallback    cb;
}action_turn_t;

typedef struct sub_texture_s {
  int   tag;
  int   originX, originY;
  int   positionX, positionY;
  int   sourceWidth, sourceHeight;
  Color color;
  int colliderPosX, colliderPosY, colliderSizeX, colliderSizeY;
} sub_texture_t;

typedef struct{
  const char* text;
  Vector2     pos;
  int         size;
  Color       color;
  int         duration;
}render_text_t;

void AddFloatingText(render_text_t *rt);

struct ent_s;   // forward declaration

//====EVENTS===>
typedef void (*CooldownCallback)(void* params);

typedef struct{
  event_uid_i       eid;
  EventType         type;
  int               duration;
  int               elapsed;
  bool              is_complete;
  bool              is_recycled;
  void*             on_end_params;
  void*             on_step_params;
  CooldownCallback  on_end;
  CooldownCallback  on_step;
}cooldown_t;

cooldown_t* InitCooldown(int dur, EventType type, CooldownCallback on_end_callback, void* params);
void UnloadCooldown(cooldown_t* cd);

typedef struct{
  StepType    when_step;
  cooldown_t  cooldowns[MAX_EVENTS];
  bool        cooldown_used[MAX_EVENTS];
}events_t;

events_t* InitEvents();
void UnloadEvents(events_t* ev);
event_uid_i RegisterEvent(EventType type, cooldown_t* cd, int ent_id, StepType when);
int AddEvent(events_t* pool, cooldown_t* cd);
void StepEvents(events_t* pool);
void StartEvent(events_t* pool, EventType type);
void ResetEvent(events_t* pool, EventType type);
bool CheckEvent(events_t* pool, EventType type);
static inline event_uid_i EventMakeUID(EventType type, uint64_t tick, int iid){
    return ((uint64_t)type << 48) | ((uint64_t)iid << 32) | (tick & 0x0000FFFFFFFFFFFFULL);
}
typedef struct value_s value_t;

typedef bool (*ValueOnChange)(value_t* self, void* ctx);
bool ValueUpdateDie(value_t* v, void* ctx);
struct value_s{
  ValueCategory  cat;
  int            base, val;
  value_affix_t* base_app[AFF_DONE];
  value_affix_t* affixes[AFF_DONE];
  AttributeType  attr_aff_by, attr_relates_to;
  StatType       stat_aff_by, stat_relates_to;
  ValueOnChange  on_change;
  void*          context;
};

value_t* InitValue(ValueCategory cat, int base);
int ValueRebase(value_t* self);
int ValueApplyModsToVal(int val, value_affix_t* aff);
void ValueAddBaseMod(value_t* self, value_affix_t mod);

struct stat_s;
typedef void (*StatFormula)(struct stat_s* self);

typedef bool (*StatOwnerCallback)(struct ent_s* owner);
typedef void (*StatCallback)(struct stat_s* self, float old, float cur);
typedef float (*StatGetter)(struct stat_s* self);
typedef struct stat_s{
  StatType      type;
  StatType      related;
  float         min;
  float         max;
  float         current;
  float         increment;
  int           base;
  dice_roll_t*  die;
  StatFormula   start,lvl;
  ModifierType  modified_by[ATTR_DONE];
  bool          reverse,internal;
  struct ent_s  *owner;
  StatClassif   classif;
  StatGetter ratio;
  StatCallback on_stat_change,on_stat_full, on_stat_empty,on_turn;
} stat_t;

typedef struct{ 
  StatType       stat; 
  ModifierType   modifier[ATTR_DONE]; 
  StatFormula    init,lvl;
  bool           reverse; 
}stat_attribute_relation_t; 

void FormulaDieAddAttr(stat_t* self);
void FormulaAddAttr(stat_t* self);

void FormulaDie(stat_t* self);
void FormulaDieAttr(stat_t* self);
void FormulaBaseAttr(stat_t* self);
void FormulaBaseDie(stat_t* self);
static void FormulaNothing(stat_t* self){}


static stat_attribute_relation_t stat_modifiers[STAT_ENT_DONE]={
  [STAT_REACH]={STAT_REACH,{}, FormulaBaseAttr, FormulaNothing},
  [STAT_DAMAGE]={STAT_DAMAGE,{[ATTR_STR]=MOD_SQRT},FormulaAddAttr,FormulaAddAttr },
  [STAT_HEALTH]={STAT_HEALTH,{[ATTR_CON]=MOD_SQRT},FormulaDieAddAttr,FormulaDieAddAttr},
  [STAT_ARMOR]={STAT_ARMOR,{[ATTR_DEX]=MOD_SQRT},FormulaAddAttr,FormulaAddAttr},
  [STAT_AGGRO]={STAT_AGGRO,{},FormulaNothing,FormulaNothing},
  [STAT_ACTIONS]={STAT_ACTIONS,{},FormulaNothing,FormulaNothing},
  [STAT_ENERGY] = {STAT_ENERGY,{[ATTR_INT]=MOD_ADD,[ATTR_WIS]=MOD_ADD},FormulaDieAddAttr,FormulaDieAddAttr},
  [STAT_STAMINA]= {STAT_STAMINA,{[ATTR_CON]=MOD_ADD,MOD_ADD,[ATTR_STR]=MOD_ADD},FormulaDieAddAttr,FormulaDieAddAttr},
  [STAT_STAMINA_REGEN] = {STAT_STAMINA_REGEN,{[ATTR_CON]=MOD_SQRT},FormulaDieAddAttr,FormulaDieAddAttr},
  [STAT_ENERGY_REGEN] = {STAT_ENERGY_REGEN,{[ATTR_WIS]=MOD_SQRT},FormulaDieAddAttr,FormulaDieAddAttr},
  [STAT_STAMINA_REGEN_RATE] = {STAT_STAMINA_REGEN_RATE,{[ATTR_CON]=MOD_NEG_SQRT},FormulaDieAddAttr,FormulaDieAddAttr, true},
  [STAT_ENERGY_REGEN_RATE] = {STAT_ENERGY_REGEN_RATE,{[ATTR_WIS]=MOD_NEG_SQRT},FormulaDieAddAttr,FormulaDieAddAttr,true},
  [STAT_RAGE] = {.init = FormulaNothing, .lvl = FormulaNothing},
  [STAT_ENDURANCE] = {STAT_ENDURANCE,
    {
      [ATTR_CON]=MOD_ADD,[ATTR_STR]=MOD_ADD,[ATTR_DEX] = MOD_ADD
    },
    FormulaAddAttr,FormulaAddAttr
  },
  [STAT_WILL] = {STAT_WILL,
    {
      [ATTR_CHAR]=MOD_ADD,[ATTR_CON]=MOD_SQRT,[ATTR_WIS] = MOD_SQRT, [ATTR_INT] = MOD_SQRT
    },
    FormulaAddAttr,FormulaAddAttr
  },
  [STAT_HEALTH_REGEN] = {STAT_HEALTH_REGEN,
    {
      [ATTR_CON]=MOD_CBRT
    },
    FormulaAddAttr,FormulaAddAttr
  },
  [STAT_HEALTH_REGEN_RATE] = {STAT_HEALTH_REGEN_RATE,
    {
      [ATTR_CON]=MOD_SQRT
    },
    FormulaDieAddAttr,FormulaDieAddAttr,true
  },
};

typedef struct{
  SkillType   skill;
  SkillRate   rate;
  int         weights[IR_MAX];
  int         diminish[IR_MAX];
  int         falloff;  
}skill_decay_t;

typedef struct skill_s skill_t;
typedef void (*SkillCallback)(struct skill_s* self, float old, float cur);
typedef bool (*SkillOnEvent)(skill_t* self, int gain);
typedef void (*SkillProficiencyFormula)(skill_t* self);

void SkillupRelated(skill_t* self, float old, float cur);

struct skill_s{
 SkillType                id;
 int                      val,min,max,threshold;
 float                    point;
 SkillCallback            on_skill_up;
 SkillOnEvent             on_success,on_use,on_fail;
 ProficiencyChecks        checks;
 SkillProficiencyFormula  get_bonus;
 struct ent_s             *owner;
 SkillRate                rate;
};

typedef struct{
  SkillType           skill;
  uint16_t            source, target;
  uint64_t            interact_uid;
  int                 uses;
  float               challenge;
  skill_decay_t*        decay;
}skill_event_t;

skill_event_t* InitSkillEvent(skill_t* s, int challenge);
bool SkillUse(skill_t* self, int source, int target, int gain, InteractResult result);
bool SkillUseSecondary(skill_t* self, int gain, InteractResult result);
skill_decay_t* SkillEventDecay(SkillType skill, int difficulty);
bool SkillWeightedGain(skill_t* self, int gain);

skill_t* InitSkill(SkillType id, struct ent_s* owner, int min, int max);
bool SkillIncrease(struct skill_s* self, int amnt);

stat_t* InitStatOnMin(StatType attr, float min, float max);
stat_t* InitStatOnMax(StatType attr, float val, AttributeType modified_by);
stat_t* InitStatEmpty(void);
stat_t* InitStat(StatType attr,float min, float max, float amount);
bool StatExpand(stat_t* s, int val, bool fill);
bool StatIncrementValue(stat_t* attr,bool increase);
void StatIncreaseValue(stat_t* self, float old, float cur);
bool StatChangeValue(struct ent_s* owner, stat_t* attr, float val);
void StatMaxOut(stat_t* s);
void StatEmpty(stat_t* s);
void StatRestart(struct stat_s* self, float old, float cur);
void StatReverse(struct stat_s* self, float old, float cur);
bool StatIsEmpty(stat_t* s);
float StatGetRatio(stat_t *self);
//<====STATS

typedef bool (*StateComparator)(int a, int b);

typedef struct{
  int             state;
  StateComparator can;
  int             required;
}state_change_requirement_t;

typedef struct bounds_s {
  Vector2     pos,offset;
  float       radius;
  float       width,height;
  //BoundsCheckCall col_check;
} bounds_t;

typedef struct {
  int         id;
  BehaviorID  behaviors[STATE_END];
} TileInstance;

typedef struct{
  int          id;
  ItemCategory cat;
  int          equip_type;
  ItemProps    props;
  uint64_t     et_props;
}ItemInstance;

typedef struct{
  Cell        coords;
  TileStatus  status;
  env_t*      tile;
  ent_t*      occupant;
  Color       fow;
  bool        explored;
}map_cell_t;

typedef struct{

}map_section_t;

typedef struct{
  MapID        id;
  map_cell_t   **tiles;
  int          x,y,width,height;
  int          step_size;
  Color        floor;
}map_grid_t;

bool InitMap(void);
map_grid_t* InitMapGrid(void);
TileStatus MapChangeOccupant(map_grid_t* m, ent_t* e, Cell old, Cell c);
TileStatus MapSetOccupant(map_grid_t* m, ent_t* e, Cell c);
ent_t* MapGetOccupant(map_grid_t* m, Cell c, TileStatus* status);
map_cell_t* MapGetTile(map_grid_t* map,Cell tile);
TileStatus MapRemoveOccupant(map_grid_t* m, Cell c);
TileStatus MapSetTile(map_grid_t* m, env_t* e, Cell c);
void MapBuilderSetFlags(TileFlags flags, int x, int y,bool safe);
void MapSpawn(TileFlags flags, int x, int y);
void MapSpawnMob(map_grid_t* m, int x, int y);
void RoomSpawnMob(map_grid_t* m, room_t* r);

Cell MapApplyContext(map_grid_t* m);

EntityType MobGetByRules(MobRules rules);
MobCategory GetEntityCategory(EntityType t);
SpeciesType GetEntitySpecies(EntityType t);

int ResistDmgLookup(uint64_t trait);
#endif
