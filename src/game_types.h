#ifndef __GAME_TYPES__
#define __GAME_TYPES__

#include "game_common.h"
#include "game_assets.h"
#include "game_utils.h"
#include "raylib.h"
#include "game_math.h"

#define MAX_ENTS 128  
#define MAX_ENVS 4096  
#define CARRY_SIZE 4
#define NUM_ABILITIES 6
#define MAX_ABILITIES 16

#define INV_HELD_SIZE 0x1000
#define INV_WORN_SIZE 0x3000


typedef struct ent_s ent_t;
typedef struct ability_s ability_t;
typedef struct ability_sim_s ability_sim_t;
typedef struct item_s item_t;

typedef struct{
  ent_t*    enemy;
  int       challenge;
  int       offensive_rating, defensive_rating;
  float     threat_mul, threat;
  int       last_turn;
}aggro_entry_t;

typedef struct{
  ent_t*         owner;
  aggro_entry_t  *entries;
  int            count,cap;
  event_uid_i    event_id;
}aggro_table_t;

void InitAggroTable(aggro_table_t* t, int cap, ent_t* owner);
static void AggroEnsureCapacity(aggro_table_t* t);
int AggroAdd(aggro_table_t* table, ent_t* source, int threat_gain, float mul);
void AggroDecayCallback(void* params);
void AggroPrune(aggro_table_t* t);
aggro_entry_t* AggroGetEntry(aggro_table_t* table, ent_t* source);
int AggroGetEntries(aggro_table_t* table, int count, aggro_entry_t  *entries);
aggro_entry_t* AggroGetHighest(aggro_table_t* t);

typedef struct{
  ent_t* ally;          

  float  danger;        // how threatened they are
  float  need;          // how badly they need help
  float  offense;       // how dangerous they are if supported
  float  defense;       // how much cover / tanking they provide

  float  distance;      // tiles away
  float  exposure;      // how exposed helping them makes me

  float  cost;          // total cost to help
  float  value;         // total value gained
  float  score;         // value - cost

  int    last_eval_turn;
}ally_context_t;

typedef struct{
  ent_t*          owner;
  ally_context_t  *entries;
  int             count,cap;
  event_uid_i    event_id;
}ally_table_t;

void InitAllyTable(ally_table_t* t, int cap, ent_t* owner);
static void AllyEnsureCapacity(ally_table_t* t);
int AllyAdd(ally_table_t* t, ent_t* source, int dist);
void AllySync(void* params);
typedef struct{
  SpeciesType   race;
  int           rank;
  Archetype     class_arch, class_rank;
  Profession    prof;
  char          race_name[MAX_NAME_LEN];
  char          role_name[MAX_NAME_LEN];
  float         base_diff;
  PhysQual      body;
  MentalQual    mind;
  PhysBody      covering;
  PhysWeapon    natural_weaps;
  Feats         feats;
  Traits        traits;
  int           cr, offr, defr;
}properties_t;

properties_t* InitProperties(race_define_t racials, mob_define_t m);

typedef bool (*AbilityCb)(ent_t* owner,  ability_t* chain, struct ent_s* target, InteractResult result);
typedef InteractResult (*AbilityFn)(ent_t* owner,  ability_t* a, ent_t* target);
typedef ability_sim_t* (*AbilitySim)(ent_t* owner,  ability_t* a, ent_t* target);
InteractResult AbilityConsume(ent_t* owner,  ability_t* a, ent_t* target);
typedef InteractResult (*AbilitySave)(ent_t* owner,  ability_t* a, ability_sim_t* source);
bool AbilitySkillup(ent_t* owner, ability_t* a, ent_t* target, InteractResult result);
struct ability_s{
  AbilityID        id;
  AbilityType      type;
  ActionType       action;
  DamageType       school;
  StatType         resource;
  DesignationType  targeting;
  int              weight,cost,hdie,die,side,bonus,reach;
  StatType         damage_to;
  AttributeType    save,mod;
  AbilityID        chain_id;
  int              num_skills, size;
  SkillType        skills[3];
  dice_roll_t*     dc,*hit;
  stat_t*          stats[STAT_ENT_DONE];
  ability_t        *chain;
  value_t*            values[VAL_WORTH];
  int                 rankup[VAL_WORTH];
  damage_reduction_t* dr;
  AbilityCb           on_success_cb, on_use_cb;
  AbilityFn           use_fn, chain_fn;
  AbilitySave         save_fn;
  AbilitySim          sim_fn;
  item_t*             item;
};

struct ability_sim_s{
  AbilityID     id;
  AbilityType   type;
  DamageType    d_type;
  int           dmg_die,dmg_sides,d_bonus, h_die, h_sides, h_bonus, penn,dmg_calc, hit_calc, final_dmg;
  int           dmg_res[10];
  int           hit_res[2];
};

ability_sim_t* AbilitySimDmg(ent_t* owner,  ability_t* a, ent_t* target);
typedef struct{
  ActionSlot     id;
  ActionType     allowed[ACTION_SLOTTED];
  ent_t*         owner;
  StatType       resource;
  int            count, cap, size, space, rank, pref;
  ability_t      **abilities;
}action_slot_t;

typedef struct{
  ItemSlot      id;
  item_t*       container;
  bool          active;
  int           limits[ITEM_DONE];
  int           current[ITEM_DONE];
  bool          method[STORE_DONE];
  ent_t*        owner;
  int           count, cap, size, space, unburdened, limit;
  item_t        **items;
}inventory_t;

inventory_t* InitInventory(ItemSlot id, ent_t* e, int cap, int limit);
void InventoryPoll(ent_t*, ItemSlot id);
void InventorySetPrefs(inventory_t* inv, uint64_t traits);
bool InventoryAddItem(ent_t* e, item_t* i);
item_t* InventoryGetEquipped(ent_t* e, ItemSlot id);
void ActionSlotSortByPref(ent_t* owner, int *pool, int count);
action_slot_t* InitActionSlot(ActionSlot id, ent_t* owner, int rank, int cap);
bool ActionSlotAddAbility(ent_t* owner, ability_t* a);
extern ability_t ABILITIES[ABILITY_DONE];
void AbilityApplyValues(ability_t* self, value_t* v);
ability_t AbilityLookup(AbilityID id);
ability_t* EntFindAbility(ent_t* e, AbilityID id);
ability_t* InitAbility(ent_t* owner, AbilityID);
bool AbilityRankup(ent_t* owner, ability_t* a);
ability_t* InitAbilityDummy(ent_t* owner, ability_t copy);
bool AbilityUse(ent_t* owner, ability_t* a, ent_t* target, ability_sim_t* other);
ability_t* EntChoosePreferredAbility(ent_t* e, int budget);
ability_t* EntChooseWeightedAbility(ent_t* e, int budget, ActionSlot slot);
InteractResult EntAbilitySave(ent_t* e, ability_t* a, ability_sim_t* source);
InteractResult EntAbilityReduce(ent_t* e, ability_t* a, ability_sim_t* source);

typedef struct item_def_s{
  int id;
  char name[32];
  int                 type;
  ItemCategory        category;        // weapon / armor / potion / scroll
  damage_reduction_t  *dr; //TODO MOVE TO VALUE_T
  value_t             *values[VAL_ALL];
  AbilityID           ability;
  int                 num_skills;
  SkillType           skills[3];
  StorageMethod       pref;
  bool                allowed[STORE_DONE];
  sprite_t            *sprite;     // icon
}item_def_t;

typedef bool (*ItemEquipCallback)(struct ent_s* owner, struct item_s* item);
typedef bool (*ItemUseCallback)(ent_t* owner, item_t* item, InteractResult res);

struct item_s{
  const item_def_t *def;
  struct ent_s*    owner;
  bool             equipped;
  int              durability;
  StorageMethod    location;
  ability_t        *ability;     
  ItemEquipCallback on_equip[2];
  ItemUseCallback   on_use;
};

bool ItemApplyStats(struct ent_s* owner, item_t* item);
bool ItemConsume(struct ent_s* owner, item_t* item);
bool ItemAddAbility(struct ent_s* owner, item_t* item);
bool ItemSkillup(ent_t* owner, item_t* item, InteractResult res);

typedef struct{
  ItemCategory      cat;
  int               num_equip, num_use;
  ItemEquipCallback on_equip[2];
  ItemUseCallback   on_use[2];
}item_fn_t;

extern item_fn_t item_funcs[ITEM_DONE];
typedef struct{
  int         size;
  item_def_t* pool[GEAR_DONE];
}item_pool_t;

item_pool_t* InitItemPool(void);
item_def_t* DefineItem(ItemInstance data);
item_def_t* DefineArmor(ItemInstance data);
item_def_t* DefineWeapon(ItemInstance data);
item_def_t* DefineWeaponByType(WeaponType t, ItemProps p, WeaponProps w);
item_def_t* DefineArmorByType(ArmorType t, ItemProps p, ArmorProps w);
item_t* InitItem(item_def_t* def);

item_def_t* GetItemDefByID(GearID id);

typedef struct{
  struct ent_s*           target;
  Cell                    start,destination;
  int                     ranges[RANGE_EMPTY];
  behavior_tree_node_t*   bt[STATE_END];
  ability_t*              pref;
  uint64_t                behave_traits;
  choice_pool_t           *choices[ACTION_PASSIVE];
}controller_t;

controller_t* InitController(ent_t* e);

typedef struct{
  bool      immunities_status[DMG_DONE];
  int       resistances_status[DMG_DONE];
  bool      immunities_school[DMG_DONE];
  int       resistances_school[DMG_DONE];
}traits_t;

void EntApplyTraits(ent_t* e);

typedef struct {
 uint64_t   shift;
 uint64_t   mask;
}trait_pool_t;
//===ENT_T===>
struct ent_s{
  int                   uid;
  char                  name[MAX_NAME_LEN];
  uint64_t              class_id;
  stat_t*               stats[STAT_DONE];
  attribute_t*          attribs[ATTR_DONE];
  skill_t*              skills[SKILL_DONE];
  traits_t              *traits;
  properties_t          *props;
  int                   num_abilities;
  action_slot_t         *slots[SLOT_ALL];
  EntityType            type;
  map_grid_t*           map;
  Cell                  pos,facing;
  EntityState           state,previous;
  action_turn_t         *actions[ACTION_DONE];
  controller_t          *control;
  events_t              *events;
  inventory_t           *inventory[INV_DONE];
  sprite_t              *sprite;
  float                 challenge;
  aggro_table_t*        aggro;
  ally_table_t*         allies;
  struct ent_s*         last_hit_by;
  int                   team;
};

ent_t* InitEntByRace(mob_define_t def, MobRules rules);
ent_t* InitEnt(EntityType id, Cell pos);
ent_t* InitMob(EntityType mob, Cell pos);
ent_t* InitEntByRaceClass(uint64_t class_id, SpeciesType race);
void GrantEntClass(ent_t* e, race_define_t racial, race_class_t* race_class);
int EnhanceEnts(ent_t** pool, MobRules rule, int num_count);
int PromoteEntClass(ent_t* e, int ranks);
int EntBuild(mob_define_t def, MobRules rules, ent_t** pool);
void EntAddTraits(traits_t* t, uint64_t mask, uint64_t shift);
void EntCalcStats(ent_t* e, race_define_t* racial);
void EntPrepare(ent_t* e);
void EntPollInventory(ent_t* e);
item_t* EntGetItem(ent_t* e, ItemCategory cat, bool equipped);
bool EntAddItem(ent_t* e, item_t* item, bool equip);
void EntToggleTooltip(ent_t* e);
void EntInitOnce(ent_t* e);
//attack_t* InitWeaponAttack(ent_t* owner, item_t* w);
int EntDamageReduction(ent_t* e, ability_t* a, int dmg);
InteractResult EntTarget(ent_t* e, ability_t* a, ent_t* source);
InteractResult EntUseAbility(ent_t* owner, ability_t* a, ent_t* target);
void EntSync(ent_t* e);
void EntTurnSync(ent_t* e);
void EntResetRegen(stat_t* self, float old, float cur);
void EntRestoreResource(stat_t* self, float old, float cur);
void EntKill(stat_t* self, float old, float cur);
void EntDestroy(ent_t* e);
bool FreeEnt(ent_t* e);
void EntPrepStep(ent_t *e);
void EntOnLevelUp(struct skill_s* self, float old, float cur);
TileStatus EntGridStep(ent_t *e, Cell step);
void EntSetCell(ent_t *e, Cell pos);
void EntAddExp(ent_t *e, int exp);
void EntAddPos(ent_t *e, Vector2 pos);
void EntSetPos(ent_t *e, Vector2 pos);
void EntControlStep(ent_t *e);
int EntGetChallengeRating(ent_t* e);
int EntGetDefRating(ent_t* e);
int EntGetOffRating(ent_t* e);
int EntAddAggro(ent_t* owner, ent_t* source, int threat_gain, float mul);
bool EntCanSee(ent_t* owner, ent_t* e, int depth);

typedef void (*StateChangeCallback)(ent_t *e, EntityState old, EntityState s);
void SetViableTile(ent_t*, EntityState old, EntityState s);
bool CheckEntAvailable(ent_t* e);
bool CheckEntPosition(ent_t* e, Vector2 pos);
bool SetState(ent_t *e, EntityState s,StateChangeCallback callback);
void StepState(ent_t *e);
void OnStateChange(ent_t *e, EntityState old, EntityState s);
bool CanChangeState(EntityState old, EntityState s);
void ApplyItemProps(item_def_t * w, ItemProps props, uint64_t e_props);
int GetWeaponByTrait(Traits t, weapon_def_t *arms);
item_def_t* BuildWeapon(SkillType skill, ItemProps props, WeaponProps w_props);
item_def_t* BuildItem(GearID id, ItemProps props, WeaponProps w_props);
item_def_t* BuildAppropriateItem(ent_t* e, ItemCategory cat, SkillType s);
bool EntSyncSight(ent_t* e, ActionType a);
void EntComputeFOV(ent_t* e);
char* EntGetClassNamePretty(ent_t* e);
void EntActionsTaken(stat_t* self, float old, float cur);
bool EntCanTakeAction(ent_t* e);
void InitActions(action_turn_t* actions[ACTION_DONE]);
action_turn_t* InitAction(ActionType t, DesignationType targeting, TakeActionCallback fn, OnActionCallback cb);
bool ActionPlayerAttack(ent_t* e, ActionType a, KeyboardKey k, ActionSlot slot);
bool ActionMove(ent_t*, ActionType a, KeyboardKey k, ActionSlot slot);
void ActionStandby(ent_t* e);
void ActionSync(ent_t* e);
bool ActionInput(void);
bool SetAction(ent_t* e, ActionType a, void* context, DesignationType targeting);
bool ActionTaken(ent_t* e, ActionType a);
bool TakeAction(ent_t* e, action_turn_t* action);
bool ActionTraverseGrid(ent_t* e,  ActionType a, OnActionCallback cb);
ActionType ActionGetEntNext(ent_t* e);
bool ActionAttack(ent_t* e, ActionType a, OnActionCallback cb);
bool ActionMultiTarget(ent_t* e, ActionType a, OnActionCallback cb);
void ActionSetTarget(ent_t* e, ActionType a, void* target);

static action_key_t action_keys[ACTION_DONE] = {
  {ACTION_NONE},
  {ACTION_MOVE,8,{KEY_D,KEY_A,KEY_W,KEY_S,KEY_LEFT, KEY_RIGHT,KEY_UP,KEY_DOWN},ActionMove,SLOT_NONE},
  {ACTION_ATTACK,1,{KEY_F},ActionPlayerAttack,SLOT_ATTACK},
  {ACTION_WEAPON,0},
  {ACTION_ITEM,1,{KEY_V},ActionPlayerAttack,SLOT_ITEM},
  {ACTION_MAGIC,1,{KEY_M},ActionPlayerAttack,SLOT_SPELL},
};

typedef struct env_s{
  int       uid;
  EnvTile   type;
  Vector2   vpos;
  Cell      pos;
  sprite_t  *sprite;
}env_t;

env_t* InitEnv(EnvTile t,Cell pos);


static int CompareEntByCR(const void *a, const void *b)
{
    const ent_t *ea = *(const ent_t * const *)a;
    const ent_t *eb = *(const ent_t * const *)b;


    if (ea->props->cr < eb->props->cr) return  1;
    if (ea->props->cr > eb->props->cr) return -1;
    return 0;
}
#endif
