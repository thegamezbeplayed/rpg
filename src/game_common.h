#ifndef __GAME_COMMON__
#define __GAME_COMMON__

#include "raylib.h"
#include "game_tools.h"

#define CELL_WIDTH 16
#define CELL_HEIGHT 16

#define GRID_WIDTH 50
#define GRID_HEIGHT 50

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
  MOD_NEG_SQRT,
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
  STAT_STEALTH,
  STAT_ACTIONS,
  STAT_ENERGY,
  STAT_STAMINA,
  STAT_STAMINA_REGEN,
  STAT_ENERGY_REGEN,
  STAT_STAMINA_REGEN_RATE,
  STAT_ENERGY_REGEN_RATE,
  STAT_ENT_DONE,
  STAT_TIME,
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
  ATTR_BLANK,
  ATTR_DONE
}AttributeType;

typedef enum{
  DMG_NONE = 0,

  DMG_BLUNT,
  DMG_PIERCE,
  DMG_SLASH,

  DMG_FIRE,
  DMG_COLD,
  DMG_LIGHTNING,
  DMG_ACID,
  DMG_POISON,

  DMG_PSYCHIC,
  DMG_RADIANT,
  DMG_NECROTIC,
  DMG_FORCE,
  DMG_TRUE,

  DMG_DONE
}DamageType;

typedef enum {
  DMGTAG_NONE        = 0,

  // broad categories
  DMGTAG_PHYSICAL    = 1 << 0,
  DMGTAG_ELEMENTAL   = 1 << 1,
  DMGTAG_MAGIC       = 1 << 2,

  // delivery types
  DMGTAG_MELEE       = 1 << 3,
  DMGTAG_RANGED      = 1 << 4,

  // special subcategories
  DMGTAG_NATURAL     = 1 << 5,   // claws, bites
  DMGTAG_WEAPON      = 1 << 6,   // swords, axes
  DMGTAG_STATUS      = 1 << 7,   // poison, disease
  DMGTAG_ABSOLUTE    = 1 << 8,
  DMGTAG_DONE        = 1 << 9
} DamageTag;

static const uint32_t DamageTypeTags[DMG_DONE] = {
  [DMG_NONE]      = DMGTAG_NONE,

  // Physical
  [DMG_BLUNT]  = DMGTAG_PHYSICAL | DMGTAG_MELEE  | DMGTAG_WEAPON,
  [DMG_PIERCE]    = DMGTAG_PHYSICAL | DMGTAG_RANGED | DMGTAG_WEAPON,
  [DMG_SLASH]     = DMGTAG_PHYSICAL | DMGTAG_MELEE  | DMGTAG_WEAPON,

  // Elemental (also magical)
  [DMG_FIRE]      = DMGTAG_ELEMENTAL | DMGTAG_MAGIC,
  [DMG_COLD]      = DMGTAG_ELEMENTAL | DMGTAG_MAGIC,
  [DMG_LIGHTNING] = DMGTAG_ELEMENTAL | DMGTAG_MAGIC,
  [DMG_ACID]      = DMGTAG_ELEMENTAL | DMGTAG_MAGIC,
  [DMG_POISON]    = DMGTAG_STATUS    | DMGTAG_MAGIC,

  // Magical
  [DMG_PSYCHIC]   = DMGTAG_MAGIC,
  [DMG_RADIANT]   = DMGTAG_MAGIC,
  [DMG_NECROTIC]  = DMGTAG_MAGIC,
  [DMG_FORCE]     = DMGTAG_MAGIC,
  [DMG_TRUE]      = DMGTAG_ABSOLUTE,
};

static const char* DAMAGE_STRING[DMG_DONE]={
  "Unarmed",
  "Blunt",
  "Piercing",
  "Slashing",
  "Fire",
  "Frost",
  "Electric",
  "Corrosive",
  "Poison",
  "Physic",
  "Radiant",
  "Necrotic",
  "Force",
  "True"
};

typedef struct{
  int      resist_types[DMG_DONE];   
  int      resist_tags[DMGTAG_DONE];
}damage_reduction_t;

typedef enum{
  ABILITY_NONE,
  ABILITY_BITE,
  ABILITY_CLAW,
  ABILITY_SWIPE,
  ABILITY_BITE_POISON,
  ABILITY_POISON,
  ABILITY_MAGIC_MISSLE,
  ABILITY_DONE
}AbilityID;

static inline bool DamageHasTag(DamageType t, DamageTag tag) {
  return (DamageTypeTags[t] & tag) != 0;
}

static inline bool DamageIsPhysical(DamageType t) {
  return DamageHasTag(t, DMGTAG_PHYSICAL);
}

static inline bool DamageIsMagic(DamageType t) {
  return DamageHasTag(t, DMGTAG_MAGIC);
}

static inline bool DamageIsElemental(DamageType t) {
    return DamageHasTag(t, DMGTAG_ELEMENTAL);
}

static inline bool DamageIsRanged(DamageType t) {
    return DamageHasTag(t, DMGTAG_RANGED);
}

static inline bool DamageIsMelee(DamageType t) {
    return DamageHasTag(t, DMGTAG_MELEE);
}


typedef struct{
  AttributeType t;
  const char* name;
}attribute_name_t;

typedef struct{
  StatType s;
  const char* name;
}stat_name_t;

static stat_name_t STAT_STRING[STAT_ENT_DONE]={
  {STAT_REACH,"Reach"},
  {STAT_DAMAGE,"Damage"},
  {STAT_HEALTH,"Health"},
  {STAT_ARMOR, "Armor"},
  {STAT_AGGRO, "Sight"},
  {STAT_STEALTH, "Stealth"},
  {STAT_ACTIONS, "Actions"},
  {STAT_ENERGY,"Spell Energy"},
  {STAT_STAMINA, "Stamina"},
  {STAT_STAMINA_REGEN,"Stamina Regen"},
  {STAT_ENERGY_REGEN, "Spell Regen"},
  {STAT_STAMINA_REGEN_RATE,"Stamina Regen Rate"},
  {STAT_ENERGY_REGEN_RATE, "Spell Regen Rate"},
};

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
  {ATTR_CHAR,"CHARISMA"},
  {ATTR_BLANK,"REROLL"}
};

static stat_attribute_relation_t stat_modifiers[STAT_DONE]={
  [STAT_REACH]={STAT_REACH,{}},
  [STAT_DAMAGE]={STAT_DAMAGE,{[ATTR_STR]=MOD_SQRT}},
  [STAT_HEALTH]={STAT_HEALTH,{[ATTR_CON]=MOD_SQRT}},
  [STAT_ARMOR]={STAT_ARMOR,{[ATTR_DEX]=MOD_SQRT}},
  [STAT_AGGRO]={STAT_AGGRO,{}},
  [STAT_ACTIONS]={STAT_ACTIONS,{}},
  [STAT_ENERGY] = {STAT_ENERGY,{[ATTR_INT]=MOD_ADD,[ATTR_WIS]=MOD_ADD}},
  [STAT_STAMINA]= {STAT_STAMINA,{[ATTR_CON]=MOD_ADD,[ATTR_DEX]=MOD_ADD,[ATTR_STR]=MOD_ADD}},
  [STAT_STAMINA_REGEN] = {STAT_STAMINA_REGEN,{[ATTR_CON]=MOD_SQRT}},
  [STAT_ENERGY_REGEN] = {STAT_ENERGY_REGEN,{[ATTR_WIS]=MOD_SQRT}},
  [STAT_STAMINA_REGEN] = {STAT_STAMINA_REGEN,{[ATTR_CON]=MOD_NEG_SQRT}},
  [STAT_ENERGY_REGEN] = {STAT_ENERGY_REGEN,{[ATTR_WIS]=MOD_NEG_SQRT}},


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
  ENT_BEAR,
  ENT_WOLF,
  ENT_DONE
}EntityType;

typedef enum{
  TILEFLAG_NONE        = 0,
  TILEFLAG_EMPTY       = 1 << 0,   // blocks movement
  TILEFLAG_SOLID       = 1 << 1,   // blocks movement
  TILEFLAG_NATURAL     = 1 << 2,
  TILEFLAG_TREE        = 1 << 3,
  TILEFLAG_ROAD        = 1 << 4,
  TILEFLAG_FOREST      = 1 << 5,
  TILEFLAG_DEBRIS      = 1 << 6,
  TILEFLAG_DECOR       = 1 << 7,
  TILEFLAG_OBSTRUCT    = 1 << 8,
  TILEFLAG_BORDER      = 1 << 9,
  TILEFLAG_SPAWN       = 1 << 10,
  TILEFLAG_FLOOR       = 1 << 11,
  TILEFLAG_WALL        = 1 << 12,
  TILEFLAG_DOOR        = 1 << 13,
  TILEFLAG_START       = 1 << 14,
  TILEFLAG_INTERACT    = 1 << 15,
  MAPFLAG_DUNGEON      = 1 << 16,
  MAPFLAG_FOREST       = 1 << 17,
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
  ENV_DIRT,
  ENV_DIRT_PATCH,
  ENV_CAMP,
  ENV_FLOOR_DUNGEON,
  ENV_STONE_WALL,
  ENV_WALL_DUNGEON,
  ENV_WALL_RUIN,
  ENV_DOOR_DUNGEON,
  ENV_DOOR_HEAVY,
  ENV_DOOR_VAULT,
  ENV_BORDER_DUNGEON,
  ENV_DONE
}EnvTile;

static const uint32_t EnvTileFlags[ENV_DONE] = {
  [ENV_BONES_BEAST]    = TILEFLAG_DEBRIS | TILEFLAG_DECOR | MAPFLAG_FOREST | MAPFLAG_DUNGEON,
  [ENV_BOULDER]        = TILEFLAG_SOLID | TILEFLAG_DEBRIS | TILEFLAG_NATURAL,
  [ENV_COBBLE]         = TILEFLAG_ROAD,
  [ENV_COBBLE_WORN]    = TILEFLAG_ROAD,
  [ENV_FLOWERS]        = TILEFLAG_DEBRIS | TILEFLAG_DECOR | TILEFLAG_NATURAL,
  [ENV_FLOWERS_THIN]   = TILEFLAG_DEBRIS | TILEFLAG_DECOR | TILEFLAG_NATURAL,
  [ENV_FOREST_FIR]     = TILEFLAG_SOLID | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_GRASS]          = TILEFLAG_FLOOR | TILEFLAG_NATURAL,
  [ENV_GRASS_SPARSE]   = TILEFLAG_FLOOR | TILEFLAG_NATURAL,
  [ENV_GRASS_WILD]     = TILEFLAG_OBSTRUCT | TILEFLAG_NATURAL,
  [ENV_LEAVES]         = TILEFLAG_DECOR | TILEFLAG_NATURAL,
  [ENV_TREE_MAPLE]     = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_MEADOW]         = TILEFLAG_OBSTRUCT | TILEFLAG_NATURAL,
  [ENV_TREE_OLDGROWTH] = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_TREE_PINE]      = TILEFLAG_SOLID | TILEFLAG_WALL | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_ROAD]           = TILEFLAG_ROAD,
  [ENV_ROAD_CROSS]     = TILEFLAG_ROAD,
  [ENV_ROAD_FORK]      = TILEFLAG_ROAD,
  [ENV_ROAD_TURN]      = TILEFLAG_ROAD,
  [ENV_TREE_BIGLEAF]   = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_TREE_CEDAR]     = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_TREE_DEAD]      = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST,
  [ENV_TREE_DYING]     = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST,
  [ENV_TREE_FELLED]    = TILEFLAG_SOLID | TILEFLAG_DEBRIS | TILEFLAG_FOREST | TILEFLAG_NATURAL,  // updated
  [ENV_TREE_FIR]       = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_FOREST]         = TILEFLAG_BORDER | TILEFLAG_SOLID| TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_WEB]            = TILEFLAG_DECOR|MAPFLAG_FOREST|MAPFLAG_DUNGEON,
  [ENV_DIRT]            = TILEFLAG_FLOOR,
  [ENV_DIRT_PATCH]            = TILEFLAG_FLOOR,
  [ENV_CAMP]          = TILEFLAG_SPAWN,
  [ENV_FLOOR_DUNGEON]  = TILEFLAG_FLOOR | MAPFLAG_DUNGEON, 
  [ENV_STONE_WALL]    = MAPFLAG_DUNGEON | TILEFLAG_WALL | TILEFLAG_SOLID,
  [ENV_WALL_DUNGEON]  = MAPFLAG_DUNGEON | TILEFLAG_WALL | TILEFLAG_SOLID,
  [ENV_WALL_RUIN]     = MAPFLAG_DUNGEON | TILEFLAG_WALL | TILEFLAG_SOLID,
  [ENV_DOOR_DUNGEON]  = MAPFLAG_DUNGEON | TILEFLAG_DOOR | TILEFLAG_OBSTRUCT | TILEFLAG_INTERACT,
  [ENV_DOOR_HEAVY]    = MAPFLAG_DUNGEON | TILEFLAG_DOOR | TILEFLAG_OBSTRUCT | TILEFLAG_INTERACT,
  [ENV_DOOR_VAULT]    = MAPFLAG_DUNGEON | TILEFLAG_DOOR | TILEFLAG_OBSTRUCT | TILEFLAG_INTERACT,
  [ENV_BORDER_DUNGEON]  = MAPFLAG_DUNGEON | TILEFLAG_BORDER | TILEFLAG_SOLID,
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
  SPEC_ARTHROPOD,
  SPEC_ETHEREAL,
  SPEC_ROTTING,
  SPEC_VAMPIRIC,
  SPEC_CANIFORM,
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
  {MOB_HUMANOID,{[SIZE_SMALL]={[STAT_HEALTH]=-2,[STAT_ARMOR]=-1},
                  [SIZE_LARGE] = {[STAT_HEALTH]=4,[STAT_AGGRO]=1},
                  [SIZE_HUGE] = {[STAT_HEALTH]=8,[STAT_AGGRO]=2}},
  {[SIZE_SMALL]={[ATTR_DEX]=3,[ATTR_STR]=-2},
    [SIZE_LARGE]={[ATTR_STR]=2,[ATTR_CON]=2},
    [SIZE_HUGE] ={[ATTR_STR]=6,[ATTR_CON]=4,[ATTR_DEX]=-2}}
  },
  {MOB_MONSTROUS},
  {MOB_BEAST,{[SIZE_TINY]={[STAT_HEALTH]=-4},
                  [SIZE_SMALL] = {[STAT_HEALTH]=-2},
                  [SIZE_LARGE] = {[STAT_HEALTH]=4,[STAT_ARMOR]=2,[STAT_AGGRO]=1}},
  {[SIZE_TINY]={[ATTR_DEX]=6,[ATTR_STR]=-4},
    [SIZE_SMALL]={[ATTR_STR]=-3,[ATTR_DEX]=4},
    [SIZE_LARGE] ={[ATTR_STR]=2,[ATTR_CON]=2}}
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

  [ENT_SPIDER] = SPEC_ARTHROPOD,
  [ENT_SCORPION] = SPEC_ARTHROPOD,
  [ENT_BEAR] = SPEC_CANIFORM,
  [ENT_WOLF] = SPEC_CANIFORM,
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
  [ENT_SCARAB] = MOB_BEAST,
  [ENT_HEN] = MOB_BEAST,
  [ENT_DUCK] = MOB_BEAST,
  [ENT_OCTOPUS] = MOB_BEAST,

*/
  [ENT_SCORPION] = MOB_BEAST,
  [ENT_SPIDER] = MOB_BEAST,
  [ENT_BEAR] = MOB_BEAST,
  [ENT_WOLF] = MOB_BEAST,
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

typedef enum{
  SPAWN_NONE,
  SPAWN_PLAYER,
  SPAWN_SOLO,
  SPAWN_PACK,
  SPAWN_SWARM,
  SPAWN_DONE
}SpawnType;

typedef struct{
  EntityType  mob;
  SpawnType   rule;
  int         weight;
}spawn_rules_t;

typedef enum {
    CONF_START = 1,
    CONF_END   = 2,
    CONF_FLAG  = 3, // special slot for a room flag
} RoomConfigOpcode;

typedef enum {

    // ----- Size (bits 12–15) -----
    ROOM_SIZE_SMALL   = 0x1000,
    ROOM_SIZE_MEDIUM  = 0x2000,
    ROOM_SIZE_LARGE   = 0x3000,
    ROOM_SIZE_HUGE    = 0x4000,
    ROOM_SIZE_MASK    = 0xF000,

    // ----- Layout type (bits 8–11) -----
    ROOM_LAYOUT_ROOM  = 0x0100,
    ROOM_LAYOUT_HALL  = 0x0200,
    ROOM_LAYOUT_OPEN  = 0x0300,
    ROOM_LAYOUT_MAZE  = 0x0400,
    ROOM_LAYOUT_MASK  = 0x0F00,

    // ----- Purpose (bits 4–7) -----
    ROOM_PURPOSE_NONE            = 0x0000,
    ROOM_PURPOSE_TRAPPED         = 0x0010,
    ROOM_PURPOSE_SECRET          = 0x0020,
    ROOM_PURPOSE_TREASURE        = 0x0030,
    ROOM_PURPOSE_TREASURE_FALSE  = 0x0030,
    ROOM_PURPOSE_CHALLENGE = 0x0040,
    ROOM_PURPOSE_START     = 0x0050,
    ROOM_PURPOSE_MASK      = 0x00F0,

    // ----- Shape (bits 0–3) -----
    ROOM_SHAPE_SQUARE   = 0x0001,
    ROOM_SHAPE_CIRCLE   = 0x0002,
    ROOM_SHAPE_FORKED   = 0x0003,
    ROOM_SHAPE_CROSS    = 0x0004,
    ROOM_SHAPE_DIAMOND  = 0x0005,
    ROOM_SHAPE_MASK     = 0x000F,

} RoomFlags;

typedef struct {
    RoomConfigOpcode op;
    RoomFlags flags;   // only used when op == CONF_FLAG
} RoomInstr;

typedef struct {
    RoomFlags flags;   // the fully combined bitmask
    int size;          // extracted from mask
    int layout;
    int purpose;
    int shape;
} RoomDefinition;

typedef enum{
  DARK_FOREST,
  DANK_DUNGEON,
  MAP_DONE,
}MapID;

typedef struct{
  MapID           id;
  TileFlags       map_flag;
  int             num_mobs,spacing,border;
  spawn_rules_t   mobs[6];
}map_gen_t;

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
  GEAR_CLUB,
  GEAR_HAND_AXE,
  GEAR_CLOTH_ARMOR,
  GEAR_DAGGER,
  GEAR_BOW_LIGHT,
  GEAR_DONE
}GearID;

typedef struct{
  unsigned int  uid;
  ItemCategory  cat;
  int           item_id;
  int           weight,rarity;
}loot_t;

typedef enum{
  ACTION_NONE,
  ACTION_MOVE,
  ACTION_ATTACK,
  ACTION_MAGIC,
  ACTION_SELECT,
  ACTION_DONE
}ActionType;

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

typedef enum{
  DES_NONE,
  DES_FACING,
  DES_SELECT_TARGET,
  DES_MULTI_TARGET,
  DES_AREA,
  DES_DIR,
}DesignationType;

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

typedef enum{
  BEHAVIOR_NONE,          
  BEHAVIOR_CHANGE_STATE,      //1
  BEHAVIOR_GET_TARGET,        //2
  BEHAVIOR_GET_DEST,          //3
  BEHAVIOR_MOVE_TO_TARGET,    //4
  BEHAVIOR_MOVE_TO_DEST,      //4
  BEHAVIOR_CAN_ATTACK,        //5
  BEHAVIOR_ATTACK,            //6
  BEHAVIOR_SEE,               //8
  BEHAVIOR_CHECK_TURN_STATE,  //9
  BEHAVIOR_TAKE_TURN,         //10
  BEHAVIOR_MOVE,              //11
  BEHAVIOR_CHECK_AGGRO,       //12
  BEHAVIOR_ACQUIRE,           //13
  BEHAVIOR_TRY_ATTACK,        //14
  BEHAVIOR_APPROACH,          //15
  BEHAVIOR_WANDER,            //16
  BEHAVIOR_SEEK,              //17
  BEHAVIOR_TAKE_ACTION,       //18
  BEHAVIOR_ACTION,            //19
  BEHAVIOR_NO_ACTION,         //20
  BEHAVIOR_COMBAT,            //21
  BEHAVIOR_MOB_AGGRO,         //22
  BEHAVIOR_STANDBY,           //23
  BEHAVIOR_COUNT
}BehaviorID;

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
typedef void (*StatCallback)(struct stat_s* self, float old, float cur);
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
  StatCallback on_stat_change,on_stat_full, on_stat_empty;
} stat_t;

typedef enum{
  SKILL_NONE,
  SKILL_LVL,
  SKILL_DONE
}SkillType;

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

stat_t* InitStatHealth(float val);
stat_t* InitStatOnMin(StatType attr, float min, float max);
stat_t* InitStatOnMax(StatType attr, float val, AttributeType modified_by);
stat_t* InitStatEmpty(void);
stat_t* InitStat(StatType attr,float min, float max, float amount);
bool StatExpand(stat_t* s, int val, bool fill);
bool StatIncrementValue(stat_t* attr,bool increase);
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
typedef enum{
  STATE_NONE,//if ent_t is properly initalized to {0} this is already set
  STATE_SPAWN,//Should only be set after NONE
  STATE_IDLE, //should be able to move freely between these ==>
  STATE_WANDER,
  STATE_ACTION,
  STATE_AGGRO,
  STATE_ATTACK,
  STATE_STANDBY,
  STATE_SELECTION,
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
  int          equip_type,rarity;
}ItemInstance;

typedef enum{
  ARMOR_NONE,
  ARMOR_NATURAL,
  ARMOR_PADDED,
  ARMOR_LEATHER,
  ARMOR_CHAIN,
  ARMOR_PLATE,
  ARMOR_SHIELD,
  ARMOR_DONE
}ArmorType;

typedef enum{
  WEAP_NONE,
  WEAP_MACE,
  WEAP_SWORD,
  WEAP_AXE,
  WEAP_DAGGER,
  WEAP_BOW,
  WEAP_DONE
}WeaponType;

typedef enum{
  PROP_NONE,
  PROP_LIGHT,
  PROP_HEAVY,
  PROP_MARTIAL,
  PROP_TWO_HANDED,
  PROP_REACH,
  PROP_RANGED,
  PROP_AMMO,
  PROP_DONE
}WeaponProp;

typedef enum{
  QUAL_NONE = PROP_DONE,
  QUAL_TRASH,
  QUAL_COMMON,
  QUAL_RARE,
  QUAL_DONE,
  PROP_ALL
}ItemQuality;

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
  BehaviorID  behaviors[STATE_END];
} ObjectInstance;

typedef enum{
  UI_SELECTOR_CHOSEN=1,
  UI_SELECTOR_EMPTY,
  UI_SELECTOR_VALID,
  UI_ALL,
}UiType;

typedef enum{
  ELEMENT_EMPTY,
  UI_DONE = UI_ALL,
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
  int          step_size,spawn_rate,rooms;
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

void MapLoad(map_grid_t* m);
void MapBuilderSetFlags(TileFlags flags, int x, int y,bool safe);
Cell MapGetTileByFlag(map_grid_t* m, TileFlags f);
void MapRoomBuild(map_grid_t* m);
void MapRoomGen(map_grid_t* m, Cell *poi_list, int poi_count);
void MapSpawn(TileFlags flags, int x, int y);
void MapSpawnMob(int x, int y);
int MapPOIs(map_grid_t* map, Cell *list, map_gen_t* rules,int start, int count);
bool FindPath(map_grid_t *m, int sx, int sy, int tx, int ty, Cell *outNextStep);
bool TooClose(Cell a, Cell b, int min_dist);
void MapGenerateRoomAt(Cell center);
MobCategory GetEntityCategory(EntityType t);
SpeciesType GetEntitySpecies(EntityType t);
ObjectInstance GetEntityData(EntityType t);
item_prop_mod_t GetItemProps(ItemInstance data);

#endif
