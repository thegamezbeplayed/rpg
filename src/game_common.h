#ifndef __GAME_COMMON__
#define __GAME_COMMON__

#include "raylib.h"
#include "game_tools.h"
#include "game_enum.h"
#include "game_gen.h"
#include "game_define.h"
#include "game_lists.h"

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

struct dice_roll_s;
typedef int (*DiceRollFunction)(struct dice_roll_s* d);

typedef struct dice_roll_s{
  int              sides,num_die;
  DiceRollFunction roll;
}dice_roll_t;

int RollDie(dice_roll_t* d);
dice_roll_t* Die(int side, int num);
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
  int                 val,min,max;
  float               rollover,asi;
  AttributeCallback   expand;
}attribute_t;

bool AttributeScoreIncrease(attribute_t* a);

attribute_t* InitAttribute(AttributeType type, int val);

typedef bool (*ActionKeyCallback)(struct ent_s* e, ActionType a, KeyboardKey k);

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
  cooldown_t  cooldowns[MAX_EVENTS];
  bool        cooldown_used[MAX_EVENTS];
}events_t;

events_t* InitEvents();
void UnloadEvents(events_t* ev);
int AddEvent(events_t* pool, cooldown_t* cd);
void StepEvents(events_t* pool);
void StartEvent(events_t* pool, EventType type);
void ResetEvent(events_t* pool, EventType type);
bool CheckEvent(events_t* pool, EventType type);

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
  struct ent_s  *owner;
  StatGetter ratio;
  StatCallback on_stat_change,on_stat_full, on_stat_empty,on_turn;
} stat_t;

struct skill_s;
typedef void (*SkillCallback)(struct skill_s* self, float old, float cur);

typedef struct skill_s{
 SkillType     id;
 int           val,min,max;
 int           point,threshold;
 SkillCallback on_skill_up;
 struct ent_s  *owner;
}skill_t;

skill_t* InitSkill(SkillType id, struct ent_s* owner, int min, int max);
bool SkillIncrease(struct skill_s* self, int amnt);
void FormulaDieAddAttr(stat_t* self);
void FormulaDie(stat_t* self);

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

typedef struct{
  float   rating;
  int     exp, out_lvl;
}challenge_rating_t;

challenge_rating_t GetChallengeScore(float cr);

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
  int          equip_type,rarity;
}ItemInstance;

typedef struct{
  ArmorType          type;
  int                armor_class;
  damage_reduction_t dr_base,dr_rarity;
  int                weight,cost;
  AttributeType      modifier,required;
  int                mod_max, req_min;
  bool               disadvantage[STAT_DONE];
}armor_def_t;

typedef struct{
  WeaponType      type;
  int             cost,weight,die,side;
  DamageType      dtype;
  int             stats[STAT_ENT_DONE];
  bool            props[PROP_ALL];
  AbilityID       ability;
  uint64_t        skill;
}weapon_def_t;

extern armor_def_t ARMOR_TEMPLATES[ARMOR_DONE];
extern weapon_def_t WEAPON_TEMPLATES[WEAP_DONE];

typedef struct{
  int     propID;
  int     stat_change[STAT_DONE];
}item_prop_mod_t;

typedef struct {
  int         id;
  MonsterSize size;
  char        name[MAX_NAME_LEN];
  int         min,max;
  float       cr;
  int         budget;
  SpawnType   spawn;
  GearID      items[ITEM_DONE];
  AbilityID   abilities[6];
} ObjectInstance;

typedef struct{
  Cell        coords;
  TileStatus  status;
  env_t*      tile;
  ent_t*      occupant;
  Color       fow;
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
ObjectInstance GetEntityData(EntityType t);
item_prop_mod_t GetItemProps(ItemInstance data);

int ResistDmgLookup(uint64_t trait);
#endif
