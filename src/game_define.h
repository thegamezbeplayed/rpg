#ifndef __GAME_DEFINITION__
#define __GAME_DEFINITION__
#include "game_enum.h"

#define MAX_NAME_LEN 64
#define NUM_CLASS 15
typedef enum{
  SPEC_NONE       = BIT64(0),
  SPEC_HUMAN      = BIT64(1),
  SPEC_ELF        = BIT64(2),
  SPEC_ARCHAIN    = BIT64(3),
  SPEC_GOBLINOID  = BIT64(4),
  SPEC_ORC        = BIT64(5),
  SPEC_GIANT      = BIT64(6),
  SPEC_ARTHROPOD  = BIT64(7),
  SPEC_ETHEREAL   = BIT64(8),
  SPEC_ROTTING    = BIT64(9),
  SPEC_VAMPIRIC   = BIT64(10),
  SPEC_CANIFORM   = BIT64(11),
  SPEC_RODENT     = BIT64(12),
}SpeciesType;

typedef struct{
  EntityType  id;
  MobRules    rules;
  SpeciesType race;
  int         weight;
}mob_define_t;

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

  [DMG_BLEED]     = DMGTAG_STATUS,
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
typedef struct{
  int      resist_types[DMG_DONE];   
  int      resist_tags[DMGTAG_DONE];
}damage_reduction_t;

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
  StatType s;
  const char* name;
}stat_name_t;

typedef struct{
  AttributeType t;
  const char* name;
}attribute_name_t;

typedef struct{
  SkillType   sk;
  const char* name;
}skill_name_t;

typedef struct{
  StatType       stat;
  ModifierType   modifier[ATTR_DONE];
}stat_attribute_relation_t;

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
  TRAIT_EXP_AXE    = 1ULL <<24,
  TRAIT_EXP_BOW    = BIT64(25),
  TRAIT_EXP_DAGGER = BIT64(26),
  TRAIT_EXP_MACE   = BIT64(27),
  TRAIT_EXP_SWORD  = BIT64(28),
  TRAIT_EXP_MASK   = (0xFFULL <<24),
  
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

typedef struct{
  AbilityID     id;
  bool          tome,scroll;
  Archetype     base,has;
  int           lvl,ranks;
  SpeciesType   racial;
}define_ability_class_t;

typedef struct{
  uint64_t    race_class;
  int         weight;
  Archetype   base,sub,rank;
  const char  b_name[MAX_NAME_LEN],s_name[MAX_NAME_LEN],r_name[MAX_NAME_LEN];
}define_race_class_t;

typedef struct{
  MobCategory cat;
  int         stats[SIZE_CAP][STAT_DONE];
  int         attr[SIZE_CAP][ATTR_DONE];
}size_category_t;

typedef struct {
  MobCategory category;
  int           stats[STAT_ENT_DONE];
  int           attr[ATTR_DONE];
  MonsterTraits traits;
} category_stats_t;

typedef struct {
  SpeciesType species;
  int           stats[STAT_DONE];
  int           attr[ATTR_DONE];
  MonsterTraits traits;
}species_stats_t;

extern category_stats_t CATEGORY_STATS[MOB_DONE];

typedef struct{
  unsigned int  uid;
  ItemCategory  cat;
  int           item_id;
  int           weight,rarity;
}loot_t;


#endif
