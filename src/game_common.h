#ifndef __GAME_COMMON__
#define __GAME_COMMON__

#include "raylib.h"
#include "game_tools.h"

#define CELL_WIDTH 16
#define CELL_HEIGHT 16

#define GRID_WIDTH 40
#define GRID_HEIGHT 40

#define MAX_NAME_LEN 64

#define RATIO(s) ((s)->ratio((s)))

#define DIE(num,side) (dice_roll_t){(side),(num),RollDie}

struct dice_roll_s;
typedef int (*DiceRollFunction)(struct dice_roll_s* d);

typedef struct dice_roll_s{
  int              sides,num_die;
  DiceRollFunction roll;
}dice_roll_t;

int RollDie(dice_roll_t* d);
dice_roll_t* Die(int side, int num);
typedef struct ent_s ent_t;
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

typedef enum{
  MOD_NONE,
  MOD_SQRT,
  MOD_ADD,
  MOD_DONE
}ModifierType;

typedef enum{
  STAT_NONE = -1,
  STAT_REACH,
  STAT_DAMAGE,
  STAT_HEALTH,
  STAT_ARMOR,
  STAT_AGGRO,
  STAT_ACTIONS,
  STAT_DONE,
}StatType;

typedef enum{
  ATTR_NONE,
  ATTR_CON,
  ATTR_STR,
  ATTR_DEX,
  ATTR_INT,
  ATTR_WIS,
  ATTR_CHAR,
  ATTR_DONE
}AttributeType;

typedef struct{
  AttributeType t;
  const char* name;
}attribute_name_t;

typedef struct{
  StatType       stat;
  ModifierType   modifier[ATTR_DONE];
}stat_attribute_relation_t;

static attribute_name_t attributes[ATTR_DONE]={
  {ATTR_NONE,"REROLL"},
  {ATTR_CON,"CONSTITUTION"},
  {ATTR_STR,"STRENGTH"},
  {ATTR_DEX,"DEXTERITY"},
  {ATTR_INT,"INTELLIGENCE"},
  {ATTR_WIS,"WISDOM"},
  {ATTR_CHAR,"CHARISMA"}
};

static stat_attribute_relation_t stat_modifiers[STAT_DONE]={
  [STAT_REACH]={STAT_REACH,{}},
  [STAT_DAMAGE]={STAT_DAMAGE,{[ATTR_STR]=MOD_SQRT}},
  [STAT_HEALTH]={STAT_HEALTH,{[ATTR_CON]=MOD_SQRT}},
  [STAT_ARMOR]={STAT_ARMOR,{[ATTR_DEX]=MOD_SQRT}},
  [STAT_AGGRO]={STAT_AGGRO,{}},
  [STAT_ACTIONS]={STAT_ACTIONS,{}},
};

struct attribute_s;
typedef bool (*AttributeCallback)(struct attribute_s *a);

typedef struct attribute_s{ 
  int                 val,min,max;
  AttributeCallback   expand;
}attribute_t;

attribute_t* InitAttribute(AttributeType type, int val);

typedef enum{
  ENT_PERSON,
  ENT_GOBLIN,
  ENT_ORC,
  ENT_OGRE,
  ENT_ORC_FIGHTER,
  ENT_BERSERKER,
  ENT_HOBGOBLIN,
  ENT_OROG,
  ENT_SCORPION,
  ENT_SPIDER,
  ENT_TROLL,
  ENT_TROLL_CAVE,
  ENT_DONE
}EntityType;

typedef enum{
  TILEFLAG_NONE        = 0,
  TILEFLAG_EMPTY       = 1 << 0,   // blocks movement
  TILEFLAG_SOLID       = 1 << 1,   // blocks movement
  TILEFLAG_GRASS       = 1 << 2,
  TILEFLAG_TREE        = 1 << 3,
  TILEFLAG_ROAD        = 1 << 4,
  TILEFLAG_FOREST      = 1 << 5,
  TILEFLAG_NATURAL     = 1 << 6,   // plants, rocks, nature
  TILEFLAG_INTERACT    = 1 << 7,
  TILEFLAG_DECOR       = 1 <<8,
  TILEFLAG_BORDER      = 1 << 9,
  TILEFLAG_SPAWN       = 1 << 10
}TileFlags;

typedef enum{
  ENV_BONES_BEAST,
  ENV_BOULDER,
  ENV_COBBLE,
  ENV_COBBLE_WORN,
  ENV_FLOWERS,
  ENV_FLOWERS_THIN,
  ENV_FOREST_FIR,
  ENV_GRASS,
  ENV_GRASS_SPARSE,
  ENV_GRASS_WILD,
  ENV_LEAVES,
  ENV_TREE_MAPLE,
  ENV_MEADOW,
  ENV_TREE_OLDGROWTH,
  ENV_TREE_PINE,
  ENV_ROAD,
  ENV_ROAD_CROSS,
  ENV_ROAD_FORK,
  ENV_ROAD_TURN,
  ENV_TREE_BIGLEAF,
  ENV_TREE_CEDAR,
  ENV_TREE_DEAD,
  ENV_TREE_DYING,
  ENV_TREE_FELLED,
  ENV_TREE_FIR,
  ENV_FOREST,
  ENV_WEB,
  ENV_DONE
}EnvTile;

static const uint32_t EnvTileFlags[ENV_DONE] = {
    [ENV_BONES_BEAST]    = TILEFLAG_INTERACT | TILEFLAG_DECOR,
    [ENV_BOULDER]        = TILEFLAG_SOLID | TILEFLAG_NATURAL,
    [ENV_COBBLE]         = TILEFLAG_ROAD,
    [ENV_COBBLE_WORN]    = TILEFLAG_ROAD,
    [ENV_FLOWERS]        = TILEFLAG_GRASS | TILEFLAG_DECOR | TILEFLAG_NATURAL,
    [ENV_FLOWERS_THIN]   = TILEFLAG_GRASS | TILEFLAG_DECOR | TILEFLAG_NATURAL,
    [ENV_FOREST_FIR]     = TILEFLAG_SOLID | TILEFLAG_FOREST | TILEFLAG_NATURAL,
    [ENV_GRASS]          = TILEFLAG_GRASS | TILEFLAG_NATURAL,
    [ENV_GRASS_SPARSE]   = TILEFLAG_GRASS | TILEFLAG_NATURAL,
    [ENV_GRASS_WILD]     = TILEFLAG_GRASS | TILEFLAG_NATURAL,
    [ENV_LEAVES]         = TILEFLAG_DECOR | TILEFLAG_NATURAL,
    [ENV_TREE_MAPLE]     = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
    [ENV_MEADOW]         = TILEFLAG_GRASS | TILEFLAG_FOREST | TILEFLAG_NATURAL,
    [ENV_TREE_OLDGROWTH] = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
    [ENV_TREE_PINE]      = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
    [ENV_ROAD]           = TILEFLAG_ROAD,
    [ENV_ROAD_CROSS]     = TILEFLAG_ROAD,
    [ENV_ROAD_FORK]      = TILEFLAG_ROAD,
    [ENV_ROAD_TURN]      = TILEFLAG_ROAD,
    [ENV_TREE_BIGLEAF]   = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
    [ENV_TREE_CEDAR]     = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
    [ENV_TREE_DEAD]      = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST,
    [ENV_TREE_DYING]     = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST,
    [ENV_TREE_FELLED]    = TILEFLAG_INTERACT | TILEFLAG_FOREST | TILEFLAG_NATURAL,  // updated
    [ENV_TREE_FIR]       = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
    [ENV_FOREST]         = TILEFLAG_BORDER | TILEFLAG_FOREST | TILEFLAG_GRASS | TILEFLAG_NATURAL,
    [ENV_WEB]            = TILEFLAG_INTERACT | TILEFLAG_DECOR,
};

static inline bool TileHasFlag(EnvTile t, uint32_t flag) {
    return (EnvTileFlags[t] & flag) != 0;
}

static inline bool TileHasAllFlags(EnvTile t, uint32_t flags) {
    return ( (EnvTileFlags[t] & flags) == flags );
}

static inline bool TileHasAnyFlags(EnvTile t, uint32_t flags) {
    return (EnvTileFlags[t] & flags) != 0;
}

static inline EnvTile GetTileByFlags(uint32_t flags) {
    for (int i = 0; i < ENV_DONE; i++) {
        if (TileHasAllFlags(i, flags))
            return (EnvTile)i;
    }
    return (EnvTile)-1; // NONE
}

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
  MOB_PLAYER,
  MOB_DONE
}MobCategory;

typedef enum{
  SPEC_NONE,
  SPEC_HUMAN,
  SPEC_GREENSKIN,
  SPEC_ETHEREAL,
  SPEC_ROTTING,
  SPEC_VAMPIRIC,
  SPEC_DONE
}SpeciesType;

typedef enum{
  SIZE_TINY,
  SIZE_SMALL,
  SIZE_MEDIUM,
  SIZE_LARGE,
  SIZE_HUGE,
  SIZE_CAP
}MonsterSize;

typedef struct{
  MobCategory cat;
  int         stats[SIZE_CAP][STAT_DONE];
  int         attr[SIZE_CAP][ATTR_DONE];
}size_category_t;

static const size_category_t MOB_SIZE[MOB_DONE]={
  {MOB_HUMANOID,{[SIZE_SMALL]={[STAT_HEALTH]=-4,[STAT_ARMOR]=-1},
                  [SIZE_LARGE] = {[STAT_HEALTH]=4,[STAT_AGGRO]=1},
                  [SIZE_HUGE] = {[STAT_HEALTH]=8,[STAT_AGGRO]=2}},
  {[SIZE_SMALL]={[ATTR_DEX]=3,[ATTR_STR]=-3},
    [SIZE_LARGE]={[ATTR_STR]=3,[ATTR_CON]=2},
    [SIZE_HUGE] ={[ATTR_STR]=6,[ATTR_CON]=4,[ATTR_DEX]=-2}}
  },
};
typedef struct {
  MobCategory category;
  int     stats[STAT_DONE];
  int     attr[ATTR_DONE];
} category_stats_t;

typedef struct {
  SpeciesType species;
  int     stats[STAT_DONE];
  int     attr[ATTR_DONE];
}species_stats_t;

extern species_stats_t RACIALS[SPEC_DONE];
extern category_stats_t CATEGORY_STATS[MOB_DONE];

static const SpeciesType RACE_MAP[ENT_DONE] = {
  [ENT_PERSON] = SPEC_HUMAN,
//  [ENT_ARMSMAN] = SPEC_HUMAN,
//  [ENT_KNIGHT] = SPEC_HUMAN,
//  [ENT_CAPTAIN] = SPEC_HUMAN,
//  [ENT_SQUIRE] = SPEC_HUMAN,
//  [ENT_COMMANDER] = SPEC_HUMAN,
//  [ENT_TRAVELER] = SPEC_HUMAN,
//  [ENT_MAN] = SPEC_HUMAN,

  [ENT_GOBLIN] = SPEC_GREENSKIN,
  [ENT_HOBGOBLIN] = SPEC_GREENSKIN,
  [ENT_ORC] = SPEC_GREENSKIN,
  [ENT_OGRE] = SPEC_GREENSKIN,
  [ENT_TROLL] = SPEC_GREENSKIN,
  [ENT_BERSERKER] = SPEC_GREENSKIN,

//  [ENT_REVENANT] = SPEC_ETHEREAL,
//  [ENT_GHOST] = SPEC_ETHEREAL,
//  [ENT_SPECTRE] = SPEC_ETHEREAL,

//  [ENT_SKELETON] = SPEC_ROTTING,
//  [ENT_CADAVER] = SPEC_ROTTING,
//  [ENT_ABOMINATION] = SPEC_ROTTING,
};

static const MobCategory ENTITY_CATEGORY_MAP[ENT_DONE] = {

  // === HUMANOIDS ===
  [ENT_PERSON] = MOB_PLAYER,
/*  [ENT_SPEARMAN] = MOB_HUMANOID,
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
*/
  [ENT_GOBLIN] = MOB_HUMANOID,
  [ENT_HOBGOBLIN] = MOB_HUMANOID,
  [ENT_ORC] = MOB_HUMANOID,

  // === MONSTROUS ===
  [ENT_OGRE] = MOB_MONSTROUS,
  [ENT_TROLL] = MOB_MONSTROUS,
  [ENT_BERSERKER] = MOB_MONSTROUS,
/*
  // === BEASTS ===
  [ENT_DOG] = MOB_BEAST,
  [ENT_CAT] = MOB_BEAST,
  [ENT_COW] = MOB_BEAST,
  [ENT_HORSE] = MOB_BEAST,
  [ENT_BEAR] = MOB_BEAST,
  [ENT_RAT] = MOB_BEAST,
  [ENT_SNAKE] = MOB_BEAST,
  [ENT_CROC] = MOB_BEAST,
 [ENT_CRAB] = MOB_BEAST,
  [ENT_WASP] = MOB_BEAST,
  [ENT_BEETLE] = MOB_BEAST,
  [ENT_STAG] = MOB_BEAST,
  [ENT_SPIDER] = MOB_BEAST,
  [ENT_SCARAB] = MOB_BEAST,
  [ENT_HEN] = MOB_BEAST,
  [ENT_DUCK] = MOB_BEAST,
  [ENT_OCTOPUS] = MOB_BEAST,

*/
  [ENT_SCORPION] = MOB_BEAST,
  [ENT_SPIDER] = MOB_BEAST,
   // === UNDEAD ===
/*
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
*/
};

MobCategory GetEntityCategory(EntityType t);
SpeciesType GetEntitySpecies(EntityType t);

typedef enum{
  SPAWN_NONE,
  SPAWN_SOLO,
  SPAWN_PACK,
  SPAWN_DONE
}SpawnType;

typedef struct{
  SpawnType   type;
  int         min,max,weight;
}spawn_rules_t;

typedef enum{
  ITEM_NONE,
  ITEM_WEAPON,
  ITEM_ARMOR,
  ITEM_POTION,
  ITEM_SCROLL,
  ITEM_TOME,
  ITEM_DONE
}ItemCategory;

typedef enum{
  GEAR_NONE = -1,
  GEAR_MACE,
  GEAR_LEATHER_ARMOR,
  GEAR_LEATHER_CAP,
  GEAR_DONE
}GearID;

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
  bool                on_deck;
  ActionType          action;
  void*               context;
  TakeActionCallback  fn;
  OnActionCallback    cb;
}action_turn_t;

typedef enum{
  BEHAVIOR_NONE,          
  BEHAVIOR_CHANGE_STATE,      //1
  BEHAVIOR_GET_TARGET,        //2
  BEHAVIOR_GET_DEST,          //3
  BEHAVIOR_MOVE_TO_TARGET,    //4
  BEHAVIOR_MOVE_TO_DEST,      //4
  BEHAVIOR_CAN_ATTACK,        //5
  BEHAVIOR_ATTACK,            //6
  BEHAVIOR_CHECK_TURN_STATE,  //7
  BEHAVIOR_TAKE_TURN,         //8
  BEHAVIOR_MOVE,              //9
  BEHAVIOR_CHECK_AGGRO,       //10
  BEHAVIOR_ACQUIRE,           //11
  BEHAVIOR_TRY_ATTACK,        //12
  BEHAVIOR_APPROACH,          //13
  BEHAVIOR_WANDER,            //14
  BEHAVIOR_SEEK,              //15
  BEHAVIOR_TAKE_ACTION,       //16
  BEHAVIOR_ACTION,            //17
  BEHAVIOR_NO_ACTION,         //18
  BEHAVIOR_COMBAT,            //19
  BEHAVIOR_MOB_AGGRO,         //20
  BEHAVIOR_STANDBY,           //21
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
typedef void (*StatFormula)(struct stat_s* self);

typedef bool (*StatOwnerCallback)(struct ent_s* owner);
typedef void (*StatCallback)(struct ent_s* owner, float old, float cur);
typedef float (*StatGetter)(struct stat_s* self);
typedef struct stat_s{
  StatType      attribute;
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
  StatCallback on_stat_change;
  StatOwnerCallback on_stat_empty;
} stat_t;

void FormulaDieAddAttr(stat_t* self);
void FormulaDie(stat_t* self);

stat_t* InitStatHealth(float val);
stat_t* InitStatOnMin(StatType attr, float min, float max);
stat_t* InitStatOnMax(StatType attr, float val);
stat_t* InitStatEmpty(void);
stat_t* InitStat(StatType attr,float min, float max, float amount);
bool StatExpand(stat_t* s, int val, bool fill);
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
  BehaviorID  behaviors[STATE_END];
} TileInstance;

typedef struct{
  int          id;
  ItemCategory cat;
  int          stats[STAT_DONE];
}ItemInstance;

typedef struct {
  int         id;
  MonsterSize size;
  char        name[MAX_NAME_LEN];
  int         amount;
  Cell        pos[6];
  GearID      items[ITEM_DONE];
  BehaviorID  behaviors[STATE_END];
} ObjectInstance;

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

typedef enum{
  TILE_EMPTY,
  TILE_SUCCESS,
  TILE_ISSUES,
  TILE_COLLISION,
  TILE_OCCUPIED,
  TILE_BORDER,
  TILE_OUT_OF_BOUNDS,
  TILE_ERROR
}TileStatus;

typedef struct{
  Cell        coords;
  TileStatus  status;
  env_t*      tile;
  ent_t*      occupant;
}map_cell_t;

typedef struct{
  map_cell_t   tiles[GRID_WIDTH][GRID_HEIGHT];
  int          x,y,width,height;
  int          step_size;
  Color        floor;
}map_grid_t;

typedef struct{
  TileFlags enviroment[GRID_WIDTH][GRID_HEIGHT];
}map_build_t;

map_grid_t* InitMapGrid(void);
TileStatus MapChangeOccupant(map_grid_t* m, ent_t* e, Cell old, Cell c);
TileStatus MapSetOccupant(map_grid_t* m, ent_t* e, Cell c);
ent_t* MapGetOccupant(map_grid_t* m, Cell c, TileStatus* status);
map_cell_t* MapGetTile(map_grid_t* map,Cell tile);
TileStatus MapRemoveOccupant(map_grid_t* m, Cell c);
TileStatus MapSetTile(map_grid_t* m, env_t* e, Cell c);

void MapBuilderSetFlags(TileFlags flags, int x, int y);
void MapRoomBuild(map_grid_t* m);
void MapRoomGen(map_grid_t* m);
void MapSpawn(TileFlags flags, int x, int y);
#endif
