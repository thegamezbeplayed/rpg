#ifndef __GAME_COMMON__
#define __GAME_COMMON__

#include "raylib.h"
#include "game_tools.h"

#define BIT64(n) (1ULL << (n))

#define CELL_WIDTH 16
#define CELL_HEIGHT 16

#define GRID_WIDTH 67
#define GRID_HEIGHT 67
#define MAX_MAP_SIZE 128

#define MAX_NAME_LEN 64

#define RATIO(s) ((s)->ratio((s)))

#define DIE(num,side) (dice_roll_t){(side),(num),RollDie}

#define ABSTRACT_MAP_MIN   0
#define ABSTRACT_MAP_MAX   128
#define SECTION_SIZE 16
#define MAX_ROOM_SIZE 8
#define MAX_ROOMS  12
#define MAX_EDGES   128
#define MAX_ROOM_WIDTH 16
#define MAX_ROOM_HEIGHT 16
#define HAS_ANY_IN_CATEGORY(value, mask) ((value) & (mask))
#define IS_TRAIT(value, mask, trait) (((value) & (mask)) == (trait))
#define GET_FLAG(flag,mask) (flag&mask)
#define MOB_ROOM_MAX 10
#define MOB_MAP_MAX 64


#define ROOM_MOBS_SHIFT 20
#define ROOM_LAYOUT_SHIFT 8
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
  STAT_SIGHT,
  STAT_STEALTH,
  STAT_ACTIONS,
  STAT_ENERGY,
  STAT_STAMINA,
  STAT_STAMINA_REGEN,
  STAT_ENERGY_REGEN,
  STAT_START_FULL,
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
  DMG_NONE = -1,

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
  DMGTAG_NONE        = -1,

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
  ABILITY_CHEW,
  ABILITY_GNAW,
  ABILITY_CLAW,
  ABILITY_SWIPE,
  ABILITY_BITE_POISON,
  ABILITY_POISON,
  ABILITY_MAGIC_MISSLE,
  ABILITY_DONE
}AbilityID;

static int GetMatchingDamageTypes(uint32_t tags, DamageType* out, int max) {
    int count = 0;
    for (int type = 0; type < DMG_DONE && count < max; type++) {
        if ((DamageTypeTags[type] & tags) == tags) {
            out[count++] = type;
        }
        else
          out[count++] = DMG_NONE;
    }
    return count;
}

static DamageType GetDamageTypeFromTags(uint32_t tags) {
    for (int type = 0; type < DMG_DONE; type++) {
        if (DamageTypeTags[type] == tags)
            return type;
    }
    return DMG_NONE;
}

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
  {STAT_START_FULL, "N/A"},
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
  [STAT_STAMINA_REGEN_RATE] = {STAT_STAMINA_REGEN_RATE,{[ATTR_CON]=MOD_NEG_SQRT}},
  [STAT_ENERGY_REGEN_RATE] = {STAT_ENERGY_REGEN_RATE,{[ATTR_WIS]=MOD_NEG_SQRT}},
};

struct attribute_s;
typedef bool (*AttributeCallback)(struct attribute_s *a);

typedef struct attribute_s{ 
  int                 val,min,max;
  float               rollover,asi;
  AttributeCallback   expand;
}attribute_t;

bool AttributeScoreIncrease(attribute_t* a);

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
  ENT_RAT,
  ENT_DONE
}EntityType;

typedef enum{
  TILEFLAG_NONE        = 0,
  TILEFLAG_BORDER      = 1 << 0,
  TILEFLAG_EMPTY       = 1 << 1,   // blocks movement
  TILEFLAG_SOLID       = 1 << 2,   // blocks movement
  TILEFLAG_NATURAL     = 1 << 3,
  TILEFLAG_TREE        = 1 << 4,
  TILEFLAG_ROAD        = 1 << 5,
  TILEFLAG_FOREST      = 1 << 6,
  TILEFLAG_DEBRIS      = 1 << 7,
  TILEFLAG_DECOR       = 1 << 8,
  TILEFLAG_OBSTRUCT    = 1 << 9,
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
  ENV_FURNITURE_CHAIR,
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
  [ENV_FLOOR_DUNGEON]  = MAPFLAG_DUNGEON | TILEFLAG_FLOOR, 
  [ENV_STONE_WALL]    = MAPFLAG_DUNGEON | TILEFLAG_WALL | TILEFLAG_SOLID,
  [ENV_WALL_DUNGEON]  = MAPFLAG_DUNGEON | TILEFLAG_WALL | TILEFLAG_SOLID,
  [ENV_WALL_RUIN]     = MAPFLAG_DUNGEON | TILEFLAG_WALL | TILEFLAG_SOLID,
  [ENV_DOOR_DUNGEON]  = MAPFLAG_DUNGEON | TILEFLAG_DOOR | TILEFLAG_OBSTRUCT | TILEFLAG_INTERACT,
  [ENV_DOOR_HEAVY]    = MAPFLAG_DUNGEON | TILEFLAG_DOOR | TILEFLAG_OBSTRUCT | TILEFLAG_INTERACT,
  [ENV_DOOR_VAULT]    = MAPFLAG_DUNGEON | TILEFLAG_DOOR | TILEFLAG_OBSTRUCT | TILEFLAG_INTERACT,
  [ENV_BORDER_DUNGEON]  = MAPFLAG_DUNGEON | TILEFLAG_BORDER | TILEFLAG_SOLID,
  [ENV_FURNITURE_CHAIR]  = MAPFLAG_DUNGEON | TILEFLAG_SPAWN | TILEFLAG_DECOR,
};

typedef uint64_t MonsterTraits;

typedef enum{
  TRAIT_PHYS_RESIST      = 1ULL <<0,
  TRAIT_ELE_RESIST       = 1ULL <<1,
  TRAIT_MAGIC_RESIST     = 1ULL <<2,

  TRAIT_RESIST_TAG_MASK  = 0xFFULL,
  TRAIT_BLUNT_RESIST     = 1ULL << 8,
  TRAIT_PIERCE_RESIST    = 1ULL << 9,
  TRAIT_SLASH_RESIST     = 1ULL << 10,
  TRAIT_FIRE_RESIST      = 1ULL << 11,
  TRAIT_COLD_RESIST      = 1ULL << 12,
  TRAIT_ACID_RESIST      = 1ULL << 13,
  TRAIT_POISON_RESIST    = 1ULL << 14,
  TRAIT_PSYCHIC_RESIST   = 1ULL << 15,
  TRAIT_RADIANT_RESIST   = 1ULL << 16,
  TRAIT_NECROTIC_RESIST  = 1ULL << 17,
  TRAIT_FORCE_RESIST     = 1ULL << 18,
  TRAIT_RESIST_SCHOOL_MASK = (0xFFFFULL<<8),
  TRAIT_EXPERTISE_BOW     = 1ULL <<24,
  TRAIT_EXPERTISE_MASK   = (0xFFULL <<24),
  
  TRAIT_VISION_DARK     = BIT64(32),
  TRAIT_VISION_MASK     = 0xFFULL <<32,

  TRAIT_ADV_FEAR        = BIT64(40),
  TRAIT_ADV_CHARM       = BIT64(41),
  TRAIT_ADV_MASK        = 0xFFULL <<40,
  TRAIT_DONE,
}MonsterTrait;

typedef struct{
  MonsterTrait  trait;
  int           school;
}trait_defense_t;

static trait_defense_t RESIST_LOOKUP[15]={
  {TRAIT_PHYS_RESIST, DMGTAG_PHYSICAL},
  {TRAIT_ELE_RESIST, DMGTAG_ELEMENTAL},
  {TRAIT_MAGIC_RESIST, DMGTAG_MAGIC},
  {TRAIT_BLUNT_RESIST, DMG_BLUNT},
  {TRAIT_PIERCE_RESIST, DMG_PIERCE},
  {TRAIT_SLASH_RESIST, DMG_SLASH},
  {TRAIT_PIERCE_RESIST, DMG_PIERCE},
  {TRAIT_FIRE_RESIST, DMG_FIRE},
  {TRAIT_COLD_RESIST, DMG_COLD},
  {TRAIT_ACID_RESIST, DMG_ACID},
  {TRAIT_POISON_RESIST, DMG_POISON},
  {TRAIT_PSYCHIC_RESIST, DMG_PSYCHIC},
  {TRAIT_RADIANT_RESIST, DMG_RADIANT},
  {TRAIT_NECROTIC_RESIST, DMG_NECROTIC},
  {TRAIT_FORCE_RESIST, DMG_FORCE},
};
static uint64_t GetTraits(uint64_t in, uint64_t mask){
  return (in & mask);
}   
static inline int trait_index(uint64_t trait, uint64_t mask_shift) {
    return __builtin_ctzll(trait) -  __builtin_ctzll(mask_shift);
}

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
  SPEC_ELF,
  SPEC_ARCHAIN,
  SPEC_GOBLINOID,
  SPEC_ORC,
  SPEC_GIANT,
  SPEC_ARTHROPOD,
  SPEC_ETHEREAL,
  SPEC_ROTTING,
  SPEC_VAMPIRIC,
  SPEC_CANIFORM,
  SPEC_RODENT,
  SPEC_DONE
}SpeciesType;

typedef uint64_t RaceProps;

typedef enum{
 RACE_CLASS_SOLDIER   = BIT64(0),
 RACE_CLASS_ROGUE     = BIT64(1),
 RACE_CLASS_BERSERKER = BIT64(2),
 RACE_CLASS_ARCHER    = BIT64(3),
 RACE_CLASS_DRUID     = BIT64(4),
 RACE_CLASS_WARLOCK   = BIT64(5),
 RACE_CLASS_CLERIC    = BIT64(6),

 RACE_CLASS_MASK       = 0xFFULL,

 RACE_ARMOR_CRUDE     = BIT64(8),
 RACE_ARMOR_SIMPLE    = BIT64(9),
 RACE_ARMOR_ARTISAN   = BIT64(10),
 RACE_ARMOR_LIGHT     = BIT64(11),
 RACE_ARMOR_MEDIUM    = BIT64(12),
 RACE_ARMOR_HEAVY     = BIT64(13),
 RACE_ARMOR_FORGED    = BIT64(14),
 RACE_ARMOR_MAGIC     = BIT64(15),

 RACE_ARMOR_MASK      = 0xFFULL << 8,

 RACE_ARMS_CRUDE     = BIT64(16),
 RACE_ARMS_SIMPLE    = BIT64(17),
 RACE_ARMS_ARTISAN   = BIT64(18),
 RACE_ARMS_LIGHT     = BIT64(19),
 RACE_ARMS_HEAVY     = BIT64(20),
 RACE_ARMS_FORGED    = BIT64(21),
 RACE_ARMS_MAGIC     = BIT64(22),
 RACE_ARMS_SKILLED   = BIT64(23),
 RACE_ARMS_MASK      = 0xFFULL << 16,

 RACE_SIZE_SMALL     = BIT64(24),
 RACE_SIZE_BIG       = BIT64(25),
 RACE_SIZE_GIANT     = BIT64(26),
 RACE_SIZE_MASK      = 0xFFULL << 24,
 
 RACE_TACTICS_CRUDE   = BIT64(32),
 RACE_TACTICS_SIMPLE  = BIT64(33),
 RACE_TACTICS_MARTIAL = BIT64(34),
 RACE_TACTICS_RANKS   = BIT64(35),
 RACE_TACTICS_ARCANA  = BIT64(36),
 RACE_TACTICS_MYSTIC  = BIT64(37),

 RACE_TACTICS_MASK    = 0xFFULL << 32,

 RACE_DIFF_LVL        = BIT64(40),
 RACE_DIFF_SKILL      = BIT64(41),
 RACE_DIFF_GEAR       = BIT64(42),
 RACE_DIFF_SPELLS     = BIT64(43),
 RACE_DIFF_PETS       = BIT64(44),
 RACE_DIFF_ALPHA      = BIT64(45),
 RACE_DIFF_STRAT      = BIT64(46),

 RACE_DIFF_MASK       = 0xFFULL << 40,

 RACE_SPECIAL_POTIONS = BIT64(48),
 RACE_SPECIAL_TRAPS   = BIT64(49),
 RACE_SPECIAL_SCROLLS = BIT64(50),
 RACE_SPECIAL_FOCI    = BIT64(51),
 RACE_SPECIAL_WARDS   = BIT64(52),
 RACE_SPECIAL_RUNES   = BIT64(53),
 RACE_SPECIAL_BUILD   = BIT64(54),
 RACE_SPECIAL_MASK    = 0xFFULL << 48,

 RACE_BUILD_CRUDE     = BIT64(56),
 RACE_BUILD_BASIC     = BIT64(57),
 RACE_BUILD_POST      = BIT64(58),
 RACE_BUILD_FORT      = BIT64(59),
 RACE_BUILD_KEEP      = BIT64(60),

 RACE_BUILD_MASK      = 0xFFULL << 56,
}RaceProp;

typedef struct {
  SpeciesType   race;
  const char    name[MAX_NAME_LEN];
  EntityType    base_ent;
  RaceProps     props;
  uint64_t      healer,chief,captain,commander,ranged,shock,soldier,brute,magician;
  MonsterTraits traits;

}race_define_t;

static const race_define_t DEFINE_RACE[ SPEC_DONE] = {
  {SPEC_NONE},
  {SPEC_HUMAN},
  {SPEC_ELF},
  {SPEC_ARCHAIN},
  {SPEC_GOBLINOID, "Goblin", ENT_GOBLIN,
    RACE_CLASS_SOLDIER | RACE_CLASS_ROGUE | RACE_CLASS_ARCHER | RACE_CLASS_DRUID | RACE_CLASS_WARLOCK |
      RACE_ARMOR_CRUDE | RACE_ARMOR_LIGHT| RACE_ARMS_CRUDE | RACE_ARMS_LIGHT |
      RACE_SIZE_SMALL | RACE_TACTICS_CRUDE |
      RACE_DIFF_LVL | RACE_DIFF_SKILL | RACE_DIFF_SPELLS | RACE_DIFF_PETS | RACE_DIFF_ALPHA |
      RACE_SPECIAL_TRAPS | RACE_SPECIAL_FOCI | RACE_SPECIAL_WARDS,
    RACE_CLASS_CLERIC,RACE_CLASS_DRUID, RACE_CLASS_ROGUE,RACE_CLASS_SOLDIER,RACE_CLASS_ARCHER, RACE_CLASS_ROGUE, RACE_CLASS_SOLDIER, RACE_CLASS_SOLDIER, RACE_CLASS_WARLOCK,
    TRAIT_POISON_RESIST,
  },
  {SPEC_ORC, "Orc", ENT_ORC,
    RACE_CLASS_SOLDIER | RACE_CLASS_BERSERKER | RACE_CLASS_ARCHER | RACE_CLASS_DRUID | RACE_CLASS_WARLOCK |
      RACE_ARMOR_CRUDE | RACE_ARMOR_SIMPLE | RACE_ARMOR_LIGHT | RACE_ARMOR_MEDIUM | RACE_ARMOR_HEAVY | RACE_ARMOR_FORGED |
      RACE_ARMS_SIMPLE | RACE_ARMS_LIGHT | RACE_ARMS_HEAVY | RACE_ARMS_FORGED | RACE_ARMS_SKILLED |
      RACE_SIZE_BIG |
      RACE_TACTICS_MARTIAL | RACE_TACTICS_SIMPLE |
      RACE_DIFF_LVL | RACE_DIFF_SKILL | RACE_DIFF_GEAR | RACE_DIFF_SPELLS | RACE_DIFF_ALPHA |
      RACE_SPECIAL_FOCI
  },
  {SPEC_ARTHROPOD},
  {SPEC_ETHEREAL},
  {SPEC_ROTTING},
  {SPEC_VAMPIRIC},
  {SPEC_CANIFORM},
  {SPEC_RODENT},

};

typedef uint64_t Archetypes;

typedef enum{
  CLASS_BASE_BARD    = BIT64(0),
  CLASS_BASE_CLERIC  = BIT64(1),
  CLASS_BASE_DRUID   = BIT64(2),
  CLASS_BASE_FIGHTER = BIT64(3),
  CLASS_BASE_MONK    = BIT64(4),
  CLASS_BASE_RANGER  = BIT64(5),
  CLASS_BASE_ROGUE   = BIT64(6),
  CLASS_BASE_LOCK    = BIT64(7),
  CLASS_BASE_WIZ     = BIT64(8),
  CLASS_BASE_MASK    = 0x1FFULL,

  CLASS_SUB_BERZ     = BIT64(9),
  CLASS_SUB_CHAMP    = BIT64(10),
  CLASS_SUB_ASSASSIN = BIT64(11),
  CLASS_SUB_SHOOTER  = BIT64(12),
  CLASS_SUB_SHAMAN   = BIT64(13),
  CLASS_SUB_HEX      = BIT64(14),

}Archetype;

typedef struct{
  Archetype     archtype;
  int           hitdie;
  AttributeType spell,prof,save;
  float         ASI[ATTR_DONE];
}define_archetype_t;

static const define_archetype_t CLASS_DATA[11] = {
  {CLASS_BASE_BARD},
  {CLASS_BASE_CLERIC, 8, ATTR_WIS,ATTR_INT,ATTR_CHAR,
    {[ATTR_WIS]=1,[ATTR_INT]=0.5f,[ATTR_INT]=0.5f, [ATTR_CHAR] =.75f}
  },
  {CLASS_BASE_DRUID, 8, ATTR_WIS,ATTR_WIS,ATTR_INT,
    {[ATTR_CON]=0.25f, [ATTR_WIS]=0.25f, [ATTR_CHAR]=0.5f, [ATTR_INT]=0.25f}
  },
  {CLASS_BASE_FIGHTER, 10, ATTR_NONE,ATTR_STR,ATTR_CON,
    {[ATTR_STR]=1,[ATTR_DEX]=1,[ATTR_CON]=.25f}
  },
  {CLASS_BASE_MONK},
  {CLASS_BASE_RANGER, 10, ATTR_WIS, ATTR_DEX, ATTR_STR,
      {[ATTR_STR]=0.5f,[ATTR_DEX]=0.5f,[ATTR_CON]=.125f,[ATTR_WIS]=0.125f}
  },
  {CLASS_BASE_ROGUE, 8, ATTR_NONE, ATTR_DEX, ATTR_INT,
    {[ATTR_DEX]=0.5f, [ATTR_STR]=0.125,[ATTR_INT]=0.25}
  },
  {CLASS_BASE_LOCK, 8, ATTR_WIS, ATTR_CHAR, ATTR_CHAR,
    {[ATTR_CON]=0.25f, [ATTR_WIS]=0.25f, [ATTR_CHAR]=0.75f}

  },
  {CLASS_BASE_WIZ, 6, ATTR_INT, ATTR_WIS, ATTR_INT,
    {[ATTR_WIS]=0.5f, [ATTR_CHAR]=0.25f, [ATTR_INT]=0.75f}
  },
  {CLASS_SUB_BERZ, 12, ATTR_NONE, ATTR_STR, ATTR_CON},
  {CLASS_SUB_CHAMP, 10, ATTR_NONE, ATTR_STR, ATTR_CON},
};

typedef struct{
  uint64_t    race_class;
  int         weight;
  Archetype   base,sub,rank;
  const char  b_name[MAX_NAME_LEN],s_name[MAX_NAME_LEN],r_name[MAX_NAME_LEN];
}define_race_class_t;


static const define_race_class_t RACE_CLASS_DEFINE[SPEC_DONE][7] = {
  [SPEC_GOBLINOID] = {
    [__builtin_ctzll(RACE_CLASS_SOLDIER)] = { 
      RACE_CLASS_SOLDIER, 15, CLASS_BASE_FIGHTER,CLASS_BASE_ROGUE, CLASS_SUB_CHAMP,
      "Brawler","Saboteur","Boss"
    },
    [__builtin_ctzll(RACE_CLASS_ROGUE)] = {
      RACE_CLASS_ROGUE, 10, CLASS_BASE_ROGUE, CLASS_SUB_ASSASSIN, CLASS_SUB_CHAMP,
      "Skulker", "Infiltrator", "Saboteur"}, 
    [__builtin_ctzll(RACE_CLASS_ARCHER)] = {
      RACE_CLASS_ARCHER, 10, CLASS_BASE_RANGER , CLASS_BASE_ROGUE , CLASS_SUB_SHOOTER,
      "Tracker", "Prowler", "Stinger"},
    [__builtin_ctzll(RACE_CLASS_DRUID)] = {
      RACE_CLASS_DRUID, 5, CLASS_BASE_DRUID, CLASS_SUB_SHAMAN, CLASS_SUB_CHAMP,
      "Pestcaller","Seer", "Mystic"
    },
    [__builtin_ctzll(RACE_CLASS_WARLOCK)] = {
      RACE_CLASS_WARLOCK,1, CLASS_BASE_LOCK, -1,-1,
      "Filthcaller","\0","\0"
    },
    [__builtin_ctzll(RACE_CLASS_CLERIC)] = {
      RACE_CLASS_CLERIC,5, CLASS_BASE_CLERIC, RACE_CLASS_WARLOCK, CLASS_SUB_HEX,
      "Giver", "Witch-doctor", "Shadow Priest"
    }

  },
};

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
  {MOB_UNDEAD},
  {MOB_CONSTRUCT},
  {MOB_DEMONIC},
  {MOB_FEY, {
              [SIZE_TINY]={[STAT_HEALTH]=-3,[STAT_ARMOR]=-1},
              [SIZE_SMALL]={[STAT_HEALTH]=-2},
            },
  {[SIZE_TINY]={[ATTR_DEX]=3,[ATTR_STR]=-1},
    [SIZE_SMALL]={[ATTR_DEX]=2},
    [SIZE_LARGE] ={[ATTR_STR]=1,[ATTR_CON]=1}}
  },

};

typedef uint64_t MobRules;

typedef enum{
  MOB_SPAWN_TRAP        = BIT64(0),
  MOB_SPAWN_LAIR        = BIT64(1),
  MOB_SPAWN_CHALLENGE   = BIT64(2),
  MOB_SPAWN_SECRET      = BIT64(3),
  MOB_SPAWN_CAMP        = BIT64(4),
  MOB_SPAWN_PATROL      = BIT64(5),
  MOB_SPAWN_MASK        = 0xFFULL,

  MOB_MOD_ENLARGE       = BIT64(8),
  MOB_MOD_WEAPON        = BIT64(9),
  MOB_MOD_ARMOR         = BIT64(10),
  MOB_MOD_MASK          = 0xFFULL << 8,

  MOB_LOC_DUNGEON       = BIT64(16),
  MOB_LOC_CAVE          = BIT64(17),
  MOB_LOC_FOREST        = BIT64(18),
  MOB_LOC_MASK          = 0xFFULL << 16,

  MOB_THEME_CRITTER     = BIT64(24),
  MOB_THEME_PRIMITIVE   = BIT64(25),
  MOB_THEME_MARTIAL     = BIT64(26),
  MOB_THEME_MASK        = 0xFFULL << 24,

  MOB_FREQ_COMMON      = BIT64(32),
  MOB_FREQ_UNCOMMON    = BIT64(33),
  MOB_FREQ_RARE        = BIT64(34),
  MOB_FREQ_ELUSIVE     = BIT64(35),
  MOB_FREQ_LIMITED     = BIT64(36),
  MOB_FREQ_MASK        = 0xFFULL << 32,

  MOB_GROUPING_SOLO   = BIT64(40),
  MOB_GROUPING_PAIRS  = BIT64(41),
  MOB_GROUPING_TROOP  = BIT64(42),
  MOB_GROUPING_PARTY  = BIT64(43),
  MOB_GROUPING_CREW   = BIT64(44),
  MOB_GROUPING_SQUAD  = BIT64(45),
  MOB_GROUPING_WARBAND= BIT64(46),
  MOB_GROUPING_SWARM  = BIT64(47),
  MOB_GROUPING_MASK   = 0xFFULL << 40
}MobRule;

typedef struct{
  EntityType  id;
  MobRules    rules;
  SpeciesType race;
  int         weight;
}mob_define_t;

static const mob_define_t MONSTER_MASH[ENT_DONE] = {
  {ENT_PERSON},
  {ENT_GOBLIN,
      MOB_SPAWN_CHALLENGE | MOB_SPAWN_CAMP | MOB_SPAWN_PATROL |
      MOB_MOD_ENLARGE | MOB_MOD_WEAPON | MOB_MOD_ARMOR |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_FREQ_COMMON | MOB_FREQ_UNCOMMON|
      MOB_THEME_PRIMITIVE |
      MOB_GROUPING_TROOP | MOB_GROUPING_CREW | MOB_GROUPING_SQUAD | MOB_GROUPING_WARBAND,
      SPEC_GOBLINOID,
      3
  },
  {ENT_ORC,
      MOB_SPAWN_CHALLENGE | MOB_SPAWN_CAMP | MOB_SPAWN_PATROL |
      MOB_MOD_ENLARGE | MOB_MOD_WEAPON | MOB_MOD_ARMOR |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_FREQ_COMMON | MOB_FREQ_UNCOMMON | MOB_FREQ_RARE|
      MOB_THEME_MARTIAL |
      MOB_GROUPING_PAIRS | MOB_GROUPING_TROOP | MOB_GROUPING_CREW | MOB_GROUPING_SQUAD | MOB_GROUPING_WARBAND,
      SPEC_ORC,
      2
  },
  {ENT_OGRE},
  {ENT_ORC_FIGHTER},
  {ENT_BERSERKER},
  {ENT_HOBGOBLIN},
  {ENT_OROG},
  {ENT_SCORPION,
    MOB_SPAWN_LAIR | 
      MOB_MOD_ENLARGE | 
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_THEME_CRITTER |
      MOB_FREQ_COMMON | MOB_FREQ_UNCOMMON | MOB_FREQ_RARE |
      MOB_GROUPING_SOLO | MOB_GROUPING_SWARM,
    2
  },
  {ENT_SPIDER,
    MOB_SPAWN_LAIR | 
      MOB_MOD_ENLARGE | 
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_THEME_CRITTER |
      MOB_FREQ_COMMON | MOB_FREQ_UNCOMMON | MOB_FREQ_RARE |
      MOB_GROUPING_SOLO | MOB_GROUPING_SWARM,
    4
  },
  {ENT_TROLL,
      MOB_SPAWN_LAIR | 
      MOB_MOD_ENLARGE | 
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST|
      MOB_THEME_PRIMITIVE |
      MOB_GROUPING_SOLO | MOB_GROUPING_PAIRS,
      5
  },
  {ENT_TROLL_CAVE,
    MOB_SPAWN_LAIR | 
      MOB_MOD_ENLARGE | 
      MOB_LOC_DUNGEON | MOB_LOC_CAVE |
      MOB_THEME_PRIMITIVE |
      MOB_GROUPING_SOLO,
    6
  },
  {ENT_BEAR,
      MOB_SPAWN_LAIR | 
      MOB_MOD_ENLARGE | 
      MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_GROUPING_SOLO | MOB_GROUPING_PAIRS | MOB_GROUPING_SQUAD,
      5
  },
  {ENT_WOLF,
      MOB_SPAWN_LAIR | 
      MOB_MOD_ENLARGE | 
      MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_GROUPING_SOLO | MOB_GROUPING_SQUAD,
      4
  },
  {ENT_RAT,
      MOB_MOD_ENLARGE | 
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_THEME_CRITTER |
      MOB_FREQ_COMMON | MOB_FREQ_UNCOMMON | MOB_FREQ_RARE|
      MOB_GROUPING_SOLO | MOB_GROUPING_PAIRS | MOB_GROUPING_TROOP | MOB_GROUPING_CREW | MOB_GROUPING_SQUAD | MOB_GROUPING_SWARM,
      3
  },

};

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

static inline int FilterMobsByRules(uint64_t rules, mob_define_t *pool) {
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

typedef struct {
  MobCategory category;
  int           stats[STAT_DONE];
  int           attr[ATTR_DONE];
  MonsterTraits traits;
} category_stats_t;

typedef struct {
  SpeciesType species;
  int           stats[STAT_DONE];
  int           attr[ATTR_DONE];
  MonsterTraits traits;
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

  [ENT_GOBLIN] = SPEC_GOBLINOID,
  [ENT_HOBGOBLIN] = SPEC_GOBLINOID,
  [ENT_ORC] = SPEC_ORC,
  [ENT_OGRE] = SPEC_GOBLINOID,
  [ENT_TROLL] = SPEC_GIANT,
  [ENT_BERSERKER] = SPEC_ORC,

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
  [ENT_GOBLIN] = MOB_FEY,
  [ENT_HOBGOBLIN] = MOB_FEY,
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
  MobRules    rules;
  int         max_mobs,max_room_mobs;
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
  Color       fow;
}map_cell_t;

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
    ROOM_SIZE_XL      = 0x4000,
    ROOM_SIZE_HUGE    = 0x5000,
    ROOM_SIZE_MASSIVE = 0x6000,
    ROOM_SIZE_MAX     = 0x7000,
    ROOM_SIZE_MASK    = 0xF000,

    // ----- Layout type (bits 8–11) -----
    ROOM_LAYOUT_NONE  = 0x0000,
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
    ROOM_PURPOSE_TREASURE_FALSE  = 0x0040,
    ROOM_PURPOSE_CHALLENGE = 0x0050,
    ROOM_PURPOSE_LAIR      = 0x0060,
    ROOM_PURPOSE_START     = 0x0070,
    ROOM_PURPOSE_CONNECT   = 0x0080,
    ROOM_PURPOSE_MAX      = 0x0090,
    ROOM_PURPOSE_MASK      = 0x00F0,

    // ----- Shape (bits 0–3) -----
    ROOM_SHAPE_SQUARE   = 0x0001,
    ROOM_SHAPE_CIRCLE   = 0x0002,
    ROOM_SHAPE_FORKED   = 0x0003,
    ROOM_SHAPE_CROSS    = 0x0004,
    ROOM_SHAPE_RECT     = 0x0005,
    ROOM_SHAPE_ANGLED   = 0x0006,
    ROOM_SHAPE_MAX      = 0x0007,
    ROOM_SHAPE_MASK     = 0x000F,

    // --- Orientation (bits 16-19) ---
    ROOM_ORIENT_HOR   = 0x00010000,
    ROOM_ORIENT_VER   = 0x00020000,
    ROOM_ORIENT_MASK   = 0x00030000,


    /*
    ROOM_MOBS_EASY   = 0x00010000,
    ROOM_MOBS_NORMAL   = 0x00020000,
    ROOM_MOBS_HARD   = 0x00030000,
    ROOM_MOBS_HARDER   = 0x00040000,
    ROOM_MOBS_HARDEST   = 0x00050000,
    ROOM_MOBS_MASK      = 0x000F0000,
 */
    ROOM_PLACING_C = 0x00000000,
    ROOM_PLACING_N = 0x00100000,
    ROOM_PLACING_S = 0x00200000,
    ROOM_PLACING_E = 0x00300000,
    ROOM_PLACING_W = 0x00400000,
    ROOM_PLACING_NW = 0x00500000,
    ROOM_PLACING_NE = 0x00600000,
    ROOM_PLACING_SE = 0x00700000,
    ROOM_PLACING_SW = 0x00800000,
    ROOM_PLACING_MAX = 0x00900000,
    ROOM_PLACING_MASK = 0x00F00000,

    ROOM_SPAWN_SOLO = 0x01000000,
    ROOM_SPAWN_PAIR = 0x02000000,
    ROOM_SPAWN_TRIO = 0x03000000,
    ROOM_SPAWN_GROUP = 0x04000000,
    ROOM_SPAWN_PACK = 0x05000000,
    ROOM_SPAWN_CAMP = 0x06000000,
    ROOM_SPAWN_MAX = 0x07000000,
    ROOM_SPAWN_MASK = 0x0F000000,
   

} RoomFlags;

static int room_size_weights[7] = {0,9,26,41,70,82,96};
static int room_purpose_weights[8] = {50,10,3,9,6,22,0,0};
static int room_shape_weights[7] = {50,20,10,10,5,20,15};

typedef enum{
  DARK_FOREST,
  DANK_DUNGEON,
  MAP_DONE,
}MapID;

typedef struct{

}map_section_t;

typedef struct{
  MapID        id;
  map_cell_t   **tiles;
  map_section_t **sections;
  int          x,y,width,height;
  int          step_size;
  Color        floor;
}map_grid_t;

typedef struct {
    TileFlags required_floor;   // must be standing on: TILEFLAG_FLOOR, FOREST, etc
    TileFlags forbidden;        // cannot overlap
    TileFlags deco_flag;        // TILEFLAG_DECOR, TILEFLAG_DEBRIS, etc
    int chance;                 // 1 = always, 5 = 1/5, 10 = 1/10
} DecorRule;

static DecorRule dungeon_decor[] = {
    { TILEFLAG_FLOOR, TILEFLAG_SPAWN | TILEFLAG_START, TILEFLAG_DEBRIS, 12 },
    { TILEFLAG_FLOOR, TILEFLAG_SPAWN,                  TILEFLAG_DECOR,  15 },
    { TILEFLAG_FLOOR, TILEFLAG_WALL,                  TILEFLAG_DECOR,  20 }
};
static const int dungeon_decor_count = sizeof(dungeon_decor)/sizeof(dungeon_decor[0]);

typedef struct {
    Cell min, max;
} cell_bounds_t;

typedef struct room_s room_t;

typedef struct{
  bool          entrance;
  struct room_s *sub;
  Cell          dir, pos;
}room_opening_t;

struct room_s{
    cell_bounds_t   bounds;
    int             num_children;
    RoomFlags       flags;
    room_opening_t  openings[16];
    Cell            center,dir;
    ent_t           *mobs[MOB_ROOM_MAX];
};

typedef enum {
    MAP_NODE_SUCCESS,
    MAP_NODE_FAILURE
} MapNodeResult;

typedef enum {
  MAP_NODE_ROOT,
  MAP_NODE_SEQUENCE,
  MAP_NODE_LEAF,
} MapNodeType;

typedef enum {
  MN_GRID,
  MN_SELECT_ROOMS,
  MN_FILL_MISSING_ROOMS,
  MN_GENERATE_ROOMS,
  MN_GRID_GEN,
  MN_MAP_OUT_GRID,
  MN_APPLY_SHAPES,
  MN_PLACE_ROOMS,
  MN_COMPUTE_BOUNDS,
  MN_ALLOCATE_TILES,
  MN_CARVE_TILES,
  MN_GRAPH_ROOTS,
  MN_BUILD,
  MN_CONNECT_SUB,
  MN_DETAIL,
  MN_ENCASE_FLOORS,
  MN_PLACE_POI,
  MN_SET_PLAYER,
  MN_PLACE_SPAWNS,
  MAP_NODE_PLACE_ROOMS,
  MAP_NODE_BUILD_GRAPH,
  MAP_NODE_CONNECT_HALLS,
  MAP_NODE_PLACE_SPAWNS,
  MAP_NODE_DECORATE,
  MAP_NODE_DONE
} MapNodeID;

typedef struct{
  MapID           id;
  TileFlags       map_flag;
  int             spacing,border,max_rooms,num_rooms;
  spawn_rules_t   mobs;
  RoomFlags       rooms[12];
  MapNodeID       node_rules;
}map_gen_t;

EntityType MobGetByRules(MobRules rules);

typedef struct {
  map_gen_t   *map_rules;
  int         width, height, num_rooms;
  room_t      rooms[MAX_ROOMS];
  Cell        player_start;
  TileFlags **tiles;
  int alloc_w, alloc_h;
  // random seed etc if you want
  unsigned int seed;
  int         decor_density;
} map_context_t;

struct map_node_s;
typedef struct map_node_s map_node_t;

typedef MapNodeResult (*MapNodeFn)(map_context_t *ctx, map_node_t *node);

MapNodeResult MapFillMissing(map_context_t *ctx, map_node_t *node);
MapNodeResult MapPlaceSpawns(map_context_t *ctx, map_node_t *node);
MapNodeResult MapPlayerSpawn(map_context_t *ctx, map_node_t *node);
MapNodeResult MapFillWalls(map_context_t *ctx, map_node_t *node);
MapNodeResult MapGraphRooms(map_context_t *ctx, map_node_t *node);
MapNodeResult MapApplyRoomShapes(map_context_t *ctx, map_node_t *node);
MapNodeResult MapPlaceSubrooms(map_context_t *ctx, map_node_t *node);
MapNodeResult MapCarveTiles(map_context_t *ctx, map_node_t *node);
MapNodeResult MapAllocateTiles(map_context_t *ctx, map_node_t *node);
MapNodeResult MapAssignPositions(map_context_t *ctx, map_node_t *node);
MapNodeResult MapGridLayout(map_context_t *ctx, map_node_t *node);
MapNodeResult MapGenerateRooms(map_context_t *ctx, map_node_t *node);
MapNodeResult MapComputeBounds(map_context_t *ctx, map_node_t *node);
MapNodeResult MapConnectSubrooms(map_context_t *ctx, map_node_t *node);

struct map_node_s {
    MapNodeType type;
    MapNodeID   id;
    MapNodeFn   run;
    int         num_children;
    map_node_t  **children;
};

typedef struct{
  MapNodeID     id;
  MapNodeType   type;
  map_node_t*   (*fn)(MapNodeID id );
  int           num_children;
  MapNodeType   children[6];
}map_node_data_t;

map_node_t* MapCreateLeafNode(MapNodeFn fn, MapNodeID id);
map_node_t* MapCreateSequence( MapNodeID id, map_node_t **children, int count);

static inline map_node_t* LeafMapFillMissing(MapNodeID id)  { return MapCreateLeafNode(MapFillMissing,id); }
static inline map_node_t* LeafMapPlaceSpawns(MapNodeID id)  { return MapCreateLeafNode(MapPlaceSpawns,id); }
static inline map_node_t* LeafMapFillWalls(MapNodeID id)  { return MapCreateLeafNode(MapFillWalls,id); }
static inline map_node_t* LeafMapPlayerSpawn(MapNodeID id)  { return MapCreateLeafNode(MapPlayerSpawn,id); }
static inline map_node_t* LeafMapConnectSubrooms(MapNodeID id)  { return MapCreateLeafNode(MapConnectSubrooms,id); }
static inline map_node_t* LeafMapGraphRooms(MapNodeID id)  { return MapCreateLeafNode(MapGraphRooms,id); }
static inline map_node_t* LeafMapApplyRoomShapes(MapNodeID id)  { return MapCreateLeafNode(MapApplyRoomShapes,id); }
static inline map_node_t* LeafMapPlaceSubrooms(MapNodeID id)  { return MapCreateLeafNode(MapPlaceSubrooms,id); }
static inline map_node_t* LeafMapAllocateTiles(MapNodeID id)  { return MapCreateLeafNode(MapAllocateTiles,id); }
static inline map_node_t* LeafMapCarveTiles(MapNodeID id)  { return MapCreateLeafNode(MapCarveTiles,id); }
static inline map_node_t* LeafMapComputeBounds(MapNodeID id)  { return MapCreateLeafNode(MapComputeBounds,id); }
static inline map_node_t* LeafMapAssignPositions(MapNodeID id)  { return MapCreateLeafNode(MapAssignPositions,id); }
static inline map_node_t* LeafMapGridLayout(MapNodeID id)  { return MapCreateLeafNode(MapGridLayout,id); }
static inline map_node_t* LeafMapGenerateRooms(MapNodeID id)  { return MapCreateLeafNode(MapGenerateRooms,id); }


map_node_t* MapBuildNodeRules(MapNodeType id);
MapNodeResult MapNodeRunSequence(map_context_t *ctx, map_node_t *node);
map_node_t* MapBuildRootPipeline(void);
bool MapGenerate(map_context_t *ctx);
MapNodeResult NodeAssignPositions(map_context_t *ctx, map_node_t *node);
MapNodeResult NodeBuildRoomGraph(map_context_t *ctx, map_node_t *node);
void CarveHallBetween(map_context_t *ctx, Cell a, Cell b);
MapNodeResult NodeAllocateTiles(map_context_t *ctx, map_node_t *node);
MapNodeResult NodeCarveToTiles(map_context_t *ctx, map_node_t *node);
MapNodeResult NodeConnectHalls(map_context_t *ctx, map_node_t *node);
MapNodeResult NodeDecorate(map_context_t *ctx, map_node_t *node);
TileFlags RoomSpecialDecor(RoomFlags p);

bool InitMap(void);
map_grid_t* InitMapGrid(void);
TileStatus MapChangeOccupant(map_grid_t* m, ent_t* e, Cell old, Cell c);
TileStatus MapSetOccupant(map_grid_t* m, ent_t* e, Cell c);
ent_t* MapGetOccupant(map_grid_t* m, Cell c, TileStatus* status);
map_cell_t* MapGetTile(map_grid_t* map,Cell tile);
TileStatus MapRemoveOccupant(map_grid_t* m, Cell c);
TileStatus MapSetTile(map_grid_t* m, env_t* e, Cell c);

Cell MapApplyContext(map_grid_t* m);
void MapBuilderSetFlags(TileFlags flags, int x, int y,bool safe);
Cell MapGetTileByFlag(map_grid_t* m, TileFlags f);
void MapSpawn(TileFlags flags, int x, int y);
void MapSpawnMob(map_grid_t* m, int x, int y);
void RoomSpawnMob(map_grid_t* m, room_t* r);
bool FindPath(map_grid_t *m, int sx, int sy, int tx, int ty, Cell *outNextStep);
bool TooClose(Cell a, Cell b, int min_dist);

static int RoomsOverlap(room_t *a, room_t *b, int spacing) {

    int dist = CellDistGrid(a->center, b->center);

    RoomFlags asize = a->flags & ROOM_SIZE_MASK;
    RoomFlags bsize = b->flags & ROOM_SIZE_MASK;
    int ar = (asize >> 12) + spacing;
    int br = (bsize >> 12) + spacing;
    
    return imax(ar-dist,br-dist);
}

MobCategory GetEntityCategory(EntityType t);
SpeciesType GetEntitySpecies(EntityType t);
ObjectInstance GetEntityData(EntityType t);
item_prop_mod_t GetItemProps(ItemInstance data);

int ResistDmgLookup(uint64_t trait);
#endif
