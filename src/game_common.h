#ifndef __GAME_COMMON__
#define __GAME_COMMON__

#include "raylib.h"
#include "game_tools.h"

#define CELL_WIDTH 16
#define CELL_HEIGHT 16

#define GRID_WIDTH 3
#define GRID_HEIGHT 3

typedef struct ent_s ent_t;
static inline bool LESS_THAN(int a, int b){
  return a<b;
}

static inline bool GREATER_THAN(int a, int b){
  return a>b;
}

static inline bool EQUAL_TO(int a, int b){
  return a==b;
}

typedef enum{
  STAT_NONE,
  STAT_HEALTH,
  STAT_AGGRO,
  STAT_ACTIONS,
  STAT_DONE,
  STAT_ATTACK_STATS,
  STAT_ATTACK_REACH,
  STAT_ATTACK_DONE
}StatType;

typedef enum{
  ENT_NONE,
  ENT_PERSON,
  ENT_TREE,
  ENT_SPEARMAN,
  ENT_ARMSMAN,
  ENT_KNIGHT,
  ENT_CAPTAIN,
  ENT_SQUIRE,
  ENT_COMMANDER,
  ENT_FERN,
  ENT_FERN_ALT,
  ENT_WARLOCK,
  ENT_PEASANT,
  ENT_MAIDEN,
  ENT_VILLAGER,
  ENT_TRAVELER,
  ENT_MAN,
  ENT_ACOLYTE_RECRUIT,
  ENT_ACOLYTE,
  ENT_WIZARD,
  ENT_GOBLIN,
  ENT_HOBGOBLIN,
  ENT_DEMON,
  ENT_BERSERKER,
  ENT_SATYR,
  ENT_DUELIST,
  ENT_FARMER,
  ENT_GOLEM,
  ENT_CONSTRUCT,
  ENT_SENTINEL,
  ENT_REVENANT,
  ENT_LORD,
  ENT_LADY,
  ENT_BOY,
  ENT_GIRL,
  ENT_HAG,
  ENT_HERMIT,
  ENT_FARMHAND,
  ENT_CADAVER,
  ENT_YOUNGMAN,
  ENT_SAGE,
  ENT_SCORPION,
  ENT_CRAB,
  ENT_WASP,
  ENT_BEETLE,
  ENT_STAG,
  ENT_SPIDER,
  ENT_SCARAB,
  ENT_PIXIE,
  ENT_CONJUROR,
  ENT_MAGE,
  ENT_GHOST,
  ENT_SPECTRE,
  ENT_BOGGART,
  ENT_SKELETON,
  ENT_TROLL,
  ENT_ABOMINATION,
  ENT_DUCK,
  ENT_HEN,
  ENT_COW,
  ENT_HORSE,
  ENT_CAT,
  ENT_DOG,
  ENT_OCTOPUS,
  ENT_BAT,
  ENT_SNAKE,
  ENT_CROC,
  ENT_BEAR,
  ENT_RAT,
  ENT_OGRE,
  ENT_ORC,
  ENT_APPRENTICE,
  ENT_WITCHDOCTOR,
  ENT_THIEF,
  ENT_BANDIT,
  ENT_MERCHANT,
  ENT_MERMAID,
  ENT_DONE
}EntityType;

typedef enum{
  MOB_NONE,
  MOB_HUMANOID,
  MOB_MONSTROUS,
  MOB_BEAST,
  MOB_UNDEAD,
  MOB_CONSTRUCT,
  MOB_DEMONIC,
  MOB_FEY,
  MOB_CIVILIAN,
  MOB_DONE
}MobCategory;

typedef struct {
    MobCategory category;
    int     stats[STAT_DONE];
} category_stats_t;

static category_stats_t CATEGORY_STATS[MOB_DONE] = {
  {MOB_NONE},
  {MOB_HUMANOID, {[STAT_HEALTH]=10, [STAT_AGGRO]=10,[STAT_ACTIONS]= 5}},

};

static const MobCategory ENTITY_CATEGORY_MAP[ENT_DONE] = {

    // === HUMANOIDS ===
    [ENT_PERSON] = MOB_HUMANOID,
    [ENT_SPEARMAN] = MOB_HUMANOID,
    [ENT_ARMSMAN] = MOB_HUMANOID,
    [ENT_KNIGHT] = MOB_HUMANOID,
    [ENT_CAPTAIN] = MOB_HUMANOID,
    [ENT_SQUIRE] = MOB_HUMANOID,
    [ENT_COMMANDER] = MOB_HUMANOID,
    [ENT_TRAVELER] = MOB_HUMANOID,
    [ENT_MAN] = MOB_HUMANOID,
    [ENT_SAGE] = MOB_HUMANOID,
    [ENT_HERMIT] = MOB_HUMANOID,
    [ENT_LORD] = MOB_HUMANOID,
    [ENT_LADY] = MOB_HUMANOID,
    [ENT_DUELIST] = MOB_HUMANOID,
    [ENT_BANDIT] = MOB_HUMANOID,
    [ENT_THIEF] = MOB_HUMANOID,
    [ENT_APPRENTICE] = MOB_HUMANOID,
    [ENT_ACOLYTE] = MOB_HUMANOID,
    [ENT_ACOLYTE_RECRUIT] = MOB_HUMANOID,
    [ENT_MERCHANT] = MOB_HUMANOID,
    [ENT_GOBLIN] = MOB_HUMANOID,
    [ENT_HOBGOBLIN] = MOB_HUMANOID,
    [ENT_ORC] = MOB_HUMANOID,

    // === MONSTROUS ===
    [ENT_OGRE] = MOB_MONSTROUS,
    [ENT_TROLL] = MOB_MONSTROUS,
    [ENT_BERSERKER] = MOB_MONSTROUS,

    // === BEASTS ===
    [ENT_DOG] = MOB_BEAST,
    [ENT_CAT] = MOB_BEAST,
    [ENT_COW] = MOB_BEAST,
    [ENT_HORSE] = MOB_BEAST,
    [ENT_BEAR] = MOB_BEAST,
    [ENT_RAT] = MOB_BEAST,
    [ENT_SNAKE] = MOB_BEAST,
    [ENT_CROC] = MOB_BEAST,
    [ENT_SCORPION] = MOB_BEAST,
    [ENT_CRAB] = MOB_BEAST,
    [ENT_WASP] = MOB_BEAST,
    [ENT_BEETLE] = MOB_BEAST,
    [ENT_STAG] = MOB_BEAST,
    [ENT_SPIDER] = MOB_BEAST,
    [ENT_SCARAB] = MOB_BEAST,
    [ENT_HEN] = MOB_BEAST,
    [ENT_DUCK] = MOB_BEAST,
    [ENT_OCTOPUS] = MOB_BEAST,

    // === UNDEAD ===
    [ENT_SKELETON] = MOB_UNDEAD,
    [ENT_REVENANT] = MOB_UNDEAD,
    [ENT_CADAVER] = MOB_UNDEAD,
    [ENT_GHOST] = MOB_UNDEAD,
    [ENT_SPECTRE] = MOB_UNDEAD,

    // === CONSTRUCTS ===
    [ENT_GOLEM] = MOB_CONSTRUCT,
    [ENT_CONSTRUCT] = MOB_CONSTRUCT,
    [ENT_SENTINEL] = MOB_CONSTRUCT,

    // === DEMONIC / ARCANE ===
    [ENT_DEMON] = MOB_DEMONIC,
    [ENT_ABOMINATION] = MOB_DEMONIC,
    [ENT_WARLOCK] = MOB_DEMONIC,
    [ENT_CONJUROR] = MOB_DEMONIC,
    [ENT_MAGE] = MOB_DEMONIC,
    [ENT_WIZARD] = MOB_DEMONIC,
    [ENT_WITCHDOCTOR] = MOB_DEMONIC,

    // === FEY / SPIRITS ===
    [ENT_PIXIE] = MOB_FEY,
    [ENT_BOGGART] = MOB_FEY,
    [ENT_SATYR] = MOB_FEY,
    [ENT_HAG] = MOB_FEY,
    [ENT_MERMAID] = MOB_FEY,
    [ENT_FERN] = MOB_FEY,
    [ENT_FERN_ALT] = MOB_FEY,

    // === CIVILIANS ===
    [ENT_PEASANT] = MOB_CIVILIAN,
    [ENT_MAIDEN] = MOB_CIVILIAN,
    [ENT_VILLAGER] = MOB_CIVILIAN,
    [ENT_FARMER] = MOB_CIVILIAN,
    [ENT_FARMHAND] = MOB_CIVILIAN,
    [ENT_BOY] = MOB_CIVILIAN,
    [ENT_GIRL] = MOB_CIVILIAN,
    [ENT_YOUNGMAN] = MOB_CIVILIAN
};

MobCategory GetEntityCategory(EntityType t);

typedef enum{
  ACTION_NONE,
  ACTION_MOVE,
  ACTION_ATTACK,
  ACTION_DONE
}ActionType;

typedef bool (*ActionKeyCallback)(struct ent_s* e, ActionType a, KeyboardKey k);

typedef struct{
  ActionType        action;
  int               num_keys;
  KeyboardKey       keys[8];
  ActionKeyCallback fn;
}action_key_t;

typedef bool (*OnActionCallback)(struct ent_s* e, ActionType a);
typedef bool (*TakeActionCallback)(struct ent_s* e, ActionType a, OnActionCallback cb);

typedef struct{
  ActionType          action;
  void*               context;
  TakeActionCallback  fn;
  OnActionCallback    cb;
}action_turn_t;

typedef enum{
  BEHAVIOR_NONE,
  BEHAVIOR_CHANGE_STATE,
  BEHAVIOR_GET_TARGET,
  BEHAVIOR_GET_DEST,
  BEHAVIOR_MOVE_TO_TARGET,
  BEHAVIOR_MOVE_TO_DEST,
  BEHAVIOR_CAN_ATTACK,
  BEHAVIOR_ATTACK,
  BEHAVIOR_MOVE,
  BEHAVIOR_CHECK_AGGRO,
  BEHAVIOR_ACQUIRE,
  BEHAVIOR_TRY_ATTACK,
  BEHAVIOR_APPROACH,
  BEHAVIOR_WANDER,
  BEHAVIOR_SEEK,
  BEHAVIOR_ACTION,
  BEHAVIOR_NO_ACTION,
  BEHAVIOR_MOB_AGGRO,
  BEHAVIOR_COUNT
}BehaviorID;

typedef struct sub_texture_s {
  int tag;
  int originX, originY;
  int positionX, positionY;
  int sourceWidth, sourceHeight;
  int padding;
  bool trimmed;
  int trimRecX, trimRecY, trimRecWidth, trimRecHeight;
  int colliderType;
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
typedef enum{
  EVENT_GAME_PROCESS,
  EVENT_INTERACTION,
  EVENT_TURN,
  EVENT_PLAY_SFX,
  EVENT_SONG_END,
  EVENT_SONG_FADE_IN,
  EVENT_SONG_FADE_OUT,
  EVENT_WAIT,
  EVENT_SPAWN,
  EVENT_FINISH,
  EVENT_NONE,
  MAX_EVENTS
} EventType;

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
typedef bool (*StatOwnerCallback)(struct ent_s* owner);
typedef void (*StatCallback)(struct ent_s* owner, float old, float cur);
typedef float (*StatGetter)(struct stat_s* self);
typedef struct stat_s{
  StatType  attribute;
  float     min;
  float     max;
  float     current;
  float     increment;
  StatGetter ratio;
  StatCallback on_stat_change;
  StatOwnerCallback on_stat_empty;
} stat_t;

stat_t* InitStatOnMin(StatType attr, float min, float max);
stat_t* InitStatOnMax(StatType attr, float val);
stat_t* InitStatEmpty(void);
stat_t* InitStat(StatType attr,float min, float max, float amount);
bool StatIncrementValue(stat_t* attr,bool increase);
bool StatChangeValue(struct ent_s* owner, stat_t* attr, float val);
void StatMaxOut(stat_t* s);
void StatEmpty(stat_t* s);
bool StatIsEmpty(stat_t* s);
float StatGetRatio(stat_t *self);
//<====STATS

typedef enum{
  STATE_NONE,//if ent_t is properly initalized to {0} this is already set
  STATE_SPAWN,//Should only be set after NONE
  STATE_IDLE, //should be able to move freely between these ==>
  STATE_WANDER,
  STATE_ACTION,
  STATE_AGGRO,
  STATE_ATTACK,
  STATE_STANDBY,
  STATE_DIE,//<===== In MOST cases. Should not be able to go down from DIE
  STATE_END//sentinel entity state should never be this or greater
}EntityState;

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
  Cell        pos;
  BehaviorID  behaviors[STATE_END];
} ObjectInstance;

typedef struct {
  int         id;
  BehaviorID  behaviors[STATE_END];
} TileInstance;

typedef enum{
  ELEMENT_EMPTY,
  ELEMENT_PANEL_GRAY,
  ELEMENT_PANEL_GRAY_WIDE,
  ELEMENT_BUTTON_GRAY_ACTIVE,
  ELEMENT_BUTTON_GRAY,
  ELEMENT_BOX_GRAY,
  ELEMENT_ERROR_WIDE,
  ELEMENT_COUNT
}ElementID;

#endif
