#ifndef __GAME_TYPES__
#define __GAME_TYPES__

#include "game_common.h"
#include "game_assets.h"
#include "game_utils.h"
#include "raylib.h"
#include "game_math.h"

#define MAX_ENTS 128  
#define MAX_ENVS 2048  
#define CARRY_SIZE 4
#define NUM_ABILITIES 6

typedef struct ent_s ent_t;
typedef struct ability_s ability_t;

typedef enum{
  RANGE_AGGRO,
  RANGE_LOITER,
  RANGE_NEAR,
  RANGE_ROAM,
  RANGE_EMPTY 
}RangeType;   

typedef bool (*AbilityCallback)(struct ent_s* owner,  struct ability_s* chain, struct ent_s* target);

typedef struct ability_s{
  AbilityID        id;
  DamageType       school;
  StatType         resource;
  DesignationType  targeting;
  int              weight,cost,hdie,die,side,bonus,reach;
  AttributeType    save,mod;
  AbilityID        chain;
  dice_roll_t*     dc,*hit;
  stat_t*          stats[STAT_DONE];
  struct ability_s *on_success;
  AbilityCallback  on_success_fn;
}ability_t;

extern ability_t ABILITIES[ABILITY_DONE];

ability_t AbilityLookup(AbilityID id);

typedef struct item_def_s{
  int id;
  char name[32];
  ItemCategory        category;        // weapon / armor / potion / scroll
  DamageType          damage;
  damage_reduction_t  *dr;
  int                 weight;
  stat_t              *stats[STAT_DONE];     // modifiers (e.g. +2 STR)
  AbilityID           ability;
  sprite_t            *sprite;     // icon
}item_def_t;

struct item_s;
typedef bool (*ItemEquipCallback)(struct ent_s* owner, struct item_s* item);

typedef struct item_s{
  const item_def_t *def;
  struct ent_s*    owner;
  bool             equipped;
  int              durability;
  ability_t        *ability;     
  ItemEquipCallback on_equip,on_use;
}item_t;

bool ItemApplyStats(struct ent_s* owner, item_t* item);
bool ItemConsume(struct ent_s* owner, item_t* item);
bool ItemAddAbility(struct ent_s* owner, item_t* item);

typedef struct{
  ItemCategory      cat;
  ItemEquipCallback on_equip, on_use;
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
item_t* InitItem(item_def_t* def);

item_def_t* GetItemDefByID(GearID id);

typedef struct{
  struct ent_s*           target;
  Cell                    start,destination;
  int                     ranges[RANGE_EMPTY];
  behavior_tree_node_t*   bt[STATE_END];
}controller_t;
controller_t* InitController();

//===ENT_T===>
typedef struct ent_s{
  int                   uid;
  char                  name[MAX_NAME_LEN];
  MonsterSize           size;
  stat_t*               stats[STAT_DONE];
  attribute_t*          attribs[ATTR_DONE];
  skill_t*              skills[SKILL_DONE];
  int                   num_abilities;
  ability_t*            abilities[6];
  EntityType            type;
  map_grid_t*           map;
  Cell                  pos,facing;
  EntityState           state,previous;
  action_turn_t         *actions[ACTION_DONE];
  controller_t          *control;
  events_t              *events;
  int                   num_items;
  item_t                *gear[CARRY_SIZE];
  sprite_t              *sprite;
  float                 challenge;
  struct ent_s*         last_hit_by;
} ent_t;

ent_t* InitEnt(ObjectInstance data, Cell pos);
ent_t* InitMob(EntityType mob, Cell pos);
void EntCalcStats(ent_t* e);
void EntPollInventory(ent_t* e);
item_t* EntGetItem(ent_t* e, ItemCategory cat, bool equipped);
bool EntAddItem(ent_t* e, item_t* item, bool equip);
void EntToggleTooltip(ent_t* e);
void EntInitOnce(ent_t* e);
ability_t* InitAbility(ent_t* owner, AbilityID);
ability_t* EntChooseWeightedAbility(ent_t* e, int budget);
//attack_t* InitWeaponAttack(ent_t* owner, item_t* w);
int EntDamageReduction(ent_t* e, ability_t* a, int dmg);
bool EntTarget(ent_t* e, ability_t* a, ent_t* source);
bool EntUseAbility(ent_t* owner, ability_t* a, ent_t* target);
void EntSync(ent_t* e);
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
typedef void (*StateChangeCallback)(ent_t *e, EntityState old, EntityState s);
void SetViableTile(ent_t*, EntityState old, EntityState s);
bool CheckEntAvailable(ent_t* e);
bool CheckEntPosition(ent_t* e, Vector2 pos);
bool SetState(ent_t *e, EntityState s,StateChangeCallback callback);
void StepState(ent_t *e);
void OnStateChange(ent_t *e, EntityState old, EntityState s);
bool CanChangeState(EntityState old, EntityState s);

void EntActionsTaken(stat_t* self, float old, float cur);
bool EntCanTakeAction(ent_t* e);
void InitActions(action_turn_t* actions[ACTION_DONE]);
action_turn_t* InitAction(ActionType t, DesignationType targeting, TakeActionCallback fn, OnActionCallback cb);
bool ActionPlayerAttack(ent_t* e, ActionType a, KeyboardKey k);
bool ActionMove(ent_t*, ActionType a, KeyboardKey k);
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
  {ACTION_MOVE,8,{KEY_D,KEY_A,KEY_W,KEY_S,KEY_LEFT, KEY_RIGHT,KEY_UP,KEY_DOWN},ActionMove},
  {ACTION_ATTACK,1,{KEY_F},ActionPlayerAttack,1},
  {ACTION_MAGIC,1,{KEY_M},ActionPlayerAttack,0},
};

typedef struct env_s{
  int       uid;
  EnvTile   type;
  Vector2   vpos;
  Cell      pos;
  sprite_t  *sprite;
}env_t;

env_t* InitEnv(EnvTile t,Cell pos);
#endif
