#ifndef __GAME_DEFINITION__
#define __GAME_DEFINITION__
#include "game_enum.h"

#define BASE_ATTR_VAL 4
#define MAX_NAME_LEN 64
#define NUM_CLASS 15

#define NUM_ITEM_PROPS 18
#define NUM_WEAP_PROPS 8
#define NUM_ARMOR_PROPS 8

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
  SocietyType civ;
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
  StatType   main;
  StatType   related;
}stat_relate_t;

typedef struct{
  AttributeType t;
  const char* name;
}attribute_name_t;

typedef struct{
  SkillType   sk;
  const char* name;
}skill_name_t;

typedef uint64_t ProficiencyChecks;
typedef enum{
  PC_NONE     = 0,
  PC_BONUS    = 1ULL << 0,
  PC_HIT      = 1ULL << 1,
  PC_SAVE     = 1ULL << 2,
}ProficiencyCheck;

typedef struct{
  SkillRate   rate;
  int         weights[IR_MAX];
  int         dr[IR_MAX];
  int         duration;
}define_skill_rate_t;

typedef struct{
  SkillRate   rate;
  bool        skills[SKILL_DONE];
}skill_rate_relation_t;

typedef struct{
  SkillType     main;
  SkillType     magnitude[MAG_DONE];
}skill_relation_t;

SkillRate SkillRateLookup(SkillType);
typedef enum {
    PQ_NONE            = 0,

    // Natural weapons (0–3)
    PQ_SHARP_TEETH     = 1ULL << 0,
    PQ_SHARP_CLAWS     = 1ULL << 1,
    PQ_SHARP_HORNS     = 1ULL << 2,
    PQ_HORNED          = 1ULL << 3,   // general horn trait

    // Body covering / protection (4–11)
    PQ_THICK_FUR       = 1ULL << 4,
    PQ_THICK_HIDE      = 1ULL << 5,
    PQ_THICK_SCALES    = 1ULL << 6,
    PQ_THICK_SKIN      = 1ULL << 7,
    PQ_THICK_FAT       = 1ULL << 8,
    PQ_TOUGH_HIDE      = 1ULL << 9,
    PQ_TOUGH_SCALES    = 1ULL << 10,
    PQ_STONE_SKIN      = 1ULL << 11,   // stone-like defense
    PQ_METALLIC        = 1ULL << 12,   // metal-like defense

    // Size (13–17)
    PQ_TINY            = 1ULL << 13,
    PQ_SMALL           = 1ULL << 14,
    PQ_LARGE           = 1ULL << 15,
    PQ_HUGE            = 1ULL << 16,
    PQ_GIG             = 1ULL << 17,   // gargantuan

    PQ_SHORT           = 1ULL << 18,
    PQ_TALL            = 1ULL << 19,
    PQ_LONG            = 1ULL << 20,
    PQ_WIDE            = 1ULL << 21,
    // Weight / density 
    PQ_LIGHT           = 1ULL << 22,
    PQ_HEAVY           = 1ULL << 23,
    PQ_DENSE_MUSCLE    = 1ULL << 24,

    // Limb proportions 
    PQ_LONG_LIMB       = 1ULL << 25,
    PQ_SHORT_LIMB      = 1ULL << 26,

    // Locomotion
    PQ_BIPED           = 1ULL << 27,
    PQ_QUADPED         = 1ULL << 28,
    PQ_OCTPED          = 1ULL << 29,
    PQ_WINGED          = 1ULL << 30,

    // Environmental biology 
    PQ_FINNED          = 1ULL << 31,
    PQ_GILLED          = 1ULL << 32,
    PQ_TAIL            = 1ULL << 33,

    // Special biology
    PQ_ETHEREAL        = 1ULL << 34,
    PQ_SHAPELESS       = 1ULL << 35,
    PQ_SMALL_HEAD      = 1ULL << 36,
    PQ_TINY_HEAD       = 1ULL << 37,
    PQ_TWIN_HEADED     = 1ULL << 38,
    PQ_TRI_HEADED      = 1ULL << 39,
    PQ_MANY_HEADED     = 1ULL << 40,
    PQ_LARGE_FEET      = 1ULL << 41,
    PQ_LARGE_HANDS     = 1ULL << 42

} PhysQual;

typedef enum {
    MQ_NONE            = 0,

    // Mind presence (0–3)
    MQ_ABSENT          = 1ULL << 0,   // no mind (ooze, undead)
    MQ_SIMPLE          = 1ULL << 1,   // basic instincts
    MQ_OBLIVIOUS       = 1ULL << 2,   // unaware / easily confused
    MQ_SENTIENT        = 1ULL << 3,   // understands tools / problem solving

    // Core temperament (4–11)
    MQ_AGGRESSIVE      = 1ULL << 4,
    MQ_TERRITORIAL     = 1ULL << 5,
    MQ_ALERT           = 1ULL << 6,
    MQ_CAUTIOUS        = 1ULL << 7,
    MQ_ANXIOUS         = 1ULL << 8,
    MQ_FEARLESS        = 1ULL << 9,
    MQ_DETERMINED      = 1ULL << 10,
    MQ_PATIENT         = 1ULL << 11,

    // Cognitive strength (12–15)
    MQ_CUNNING         = 1ULL << 12,
    MQ_STRATEGIC       = 1ULL << 13,
    MQ_RESOLUTE        = 1ULL << 14,
    MQ_UNREADABLE      = 1ULL << 15, // emotionless or inscrutable

    // Social behavior (16–19)
    MQ_HIVE_MIND       = 1ULL << 16,
    MQ_OBEDIENT        = 1ULL << 17,
    MQ_LEADER          = 1ULL << 18,
    MQ_PROTECTIVE      = 1ULL << 19, // already present, but good here
    // (we can remove duplicate above if desired)

    // Perceptive aptitude (20–22)
    MQ_PERCEPTIVE      = 1ULL << 20,
    MQ_AWARE           = 1ULL << 21,

    // Supernatural mind (23–24)
    MQ_EMPATH          = 1ULL << 23,
    MQ_TELEPATH        = 1ULL << 24,
    MQ_DISCIPLINED     = 1ULL << 25,
    MQ_CALCULATING     = 1ULL << 26,
    MQ_ATTUNED         = 1ULL << 27,
} MentalQual;

typedef struct {
    bool      pq;   // phys traits that apply
    bool      mq;  // mental traits that apply
    AsiEvent  event;
} asi_bonus_t;

typedef enum {
    FEAT_NONE               = 0,

    // --- Combat Offense ---
    FEAT_POWER_ATTACK       = 1ULL << 0,   // stronger melee swings
    FEAT_PRECISE_ATTACK     = 1ULL << 1,   // accuracy / reduced penalties
    FEAT_CRITICAL_FOCUS     = 1ULL << 2,   // better crit chance or crit effects
    FEAT_WEAPON_MASTERY     = 1ULL << 3,   // proficiency or rerolls with chosen weapon
    FEAT_DUAL_WIELDER       = 1ULL << 4,   // effective two-weapon fighting

    // --- Combat Defense ---
    FEAT_TOUGHNESS          = 1ULL << 5,   // extra HP / durability
    FEAT_DODGE              = 1ULL << 6,   // improved AC vs one target / evasion
    FEAT_SHIELD_TRAINING    = 1ULL << 7,   // shield proficiency or better use
    FEAT_HEAVY_ARMOR_TRAIN  = 1ULL << 8,   // heavy armor proficiency
    FEAT_RESISTANT          = 1ULL << 9,   // generic damage type resistance

    // --- Mobility ---
    FEAT_FLEETFOOT          = 1ULL << 10,  // increased speed
    FEAT_ACROBAT            = 1ULL << 11,  // better jumps / dex checks / climbing
    FEAT_MOBILE_COMBATANT   = 1ULL << 12,  // hit-and-run / avoid opportunity attacks

    // --- Magic ---
    FEAT_SPELL_FOCUS        = 1ULL << 13,  // increased spell accuracy or DC
    FEAT_RITUAL_CASTER      = 1ULL << 14,  // perform rituals / utility casting
    FEAT_CONCENTRATION      = 1ULL << 15,  // maintain spells better
    FEAT_ARCANE_ADEPT       = 1ULL << 16,  // general magical aptitude

    // --- Skills & Utility ---
    FEAT_STEALTHY           = 1ULL << 17,  // improved stealth / hiding
    FEAT_ALERT              = 1ULL << 18,  // initiative & perception bonuses
    FEAT_SKILLED            = 1ULL << 19,  // +skills / extra proficiencies
    FEAT_SURVIVALIST        = 1ULL << 20,  // tracking / foraging / nature skills
    FEAT_SILVER_TONGUE      = 1ULL << 21,  // persuasion / deception abilities

    // --- Mental & Willpower ---
    FEAT_IRON_WILL          = 1ULL << 22,  // resist fear, charm, domination
    FEAT_FOCUSED_MIND       = 1ULL << 23,  // concentration & mental saves

    // --- Special Actions ---
    FEAT_REACTION_TRAINING  = 1ULL << 24,  // extra or improved reactions
    FEAT_GRAPPLER           = 1ULL << 25,  // improved grappling ability
    FEAT_TACTICIAN          = 1ULL << 26,  // battlefield positioning help
    FEAT_ENDURANCE          = 1ULL << 27,  // exhaustion resistance / stamina

    FEAT_ATHLETICS          = 1ULL << 28,
    FEAT_LUCKY              = 1ULL << 29,
    FEAT_CRUSHER            = 1ULL << 30,
    FEAT_SLASHER            = 1ULL << 31,
    FEAT_HEALER             = 1ULL << 32,
    FEAT_SKULKER            = 1ULL << 33,
    FEAT_SENTINEL           = 1ULL << 34,
    FEAT_SNIPER             = 1ULL << 35,
    FEAT_PIERCER            = 1ULL << 36,
    FEAT_POISONER           = 1ULL << 37,
    FEAT_BRAWLER            = 1ULL << 38,
    FEAT_DURABLE            = 1ULL << 39,
} FeatFlags;
typedef uint64_t PhysQs;
typedef uint64_t MentalQs;
typedef uint64_t Feats;

typedef uint64_t Traits;

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
}Trait;

typedef struct{
  PhysQual      pq;
  Traits traits;
  FeatFlags     feats;
}phys_qualities_t;

typedef struct{
  MentalQual    pq;
  Traits traits;
  FeatFlags     feats;
}ment_qualities_t;

static const ment_qualities_t MIND[25] = {
  {MQ_SIMPLE, TRAIT_ADV_FEAR | TRAIT_ADV_CHARM},
  {MQ_OBLIVIOUS, TRAIT_ADV_CHARM},
  {MQ_ALERT,0,
    FEAT_ALERT | FEAT_DODGE
  },
  {MQ_CUNNING,0,FEAT_STEALTHY | FEAT_FOCUSED_MIND},

};

static const phys_qualities_t BODY[35] = {
  {PQ_THICK_FUR, TRAIT_SLASH_RESIST | TRAIT_COLD_RESIST},
  {PQ_THICK_HIDE, TRAIT_FIRE_RESIST | TRAIT_COLD_RESIST | TRAIT_PHYS_RESIST},
  {PQ_THICK_SCALES, TRAIT_PHYS_RESIST | TRAIT_FIRE_RESIST | TRAIT_ACID_RESIST},
  {PQ_THICK_SKIN, TRAIT_BLUNT_RESIST | TRAIT_COLD_RESIST | TRAIT_FIRE_RESIST | TRAIT_POISON_RESIST },
  {PQ_THICK_FAT, TRAIT_BLUNT_RESIST | TRAIT_COLD_RESIST },
  {PQ_HEAVY, TRAIT_FORCE_RESIST},
  {PQ_LARGE_FEET, TRAIT_FORCE_RESIST},
  {PQ_LARGE, TRAIT_FORCE_RESIST},
  {PQ_HUGE, TRAIT_POISON_RESIST},
  {PQ_GIG, TRAIT_POISON_RESIST | TRAIT_FORCE_RESIST},
  {PQ_TWIN_HEADED, TRAIT_PSYCHIC_RESIST | TRAIT_ADV_CHARM},
  {PQ_TRI_HEADED, TRAIT_ADV_FEAR | TRAIT_ADV_CHARM | TRAIT_PSYCHIC_RESIST},
  {PQ_MANY_HEADED, TRAIT_ADV_FEAR | TRAIT_ADV_CHARM | TRAIT_PSYCHIC_RESIST},
};

typedef struct {
  Traits traits;
  FeatFlags feats;
} mind_result_t;

static inline mind_result_t GetMindResult(MentalQual mask) {
  mind_result_t result = {0};

  for (int i = 0; i < 25; i++) {
    if (MIND[i].pq & mask) {         // if this mental quality is present
      result.traits |= MIND[i].traits;
      result.feats  |= MIND[i].feats;
    }
  }

  return result;
}

typedef struct {
  Traits traits;
  FeatFlags feats;
} body_result_t;

static inline body_result_t GetBodyResult(MentalQual mask) {
  body_result_t result = {0};

  for (int i = 0; i < 25; i++) {
    if (BODY[i].pq & mask) {         // if this mental quality is present
      result.traits |= BODY[i].traits;
      result.feats  |= BODY[i].feats;
    }
  }

  return result;
}


typedef struct{
  AttributeType attr;
  PhysQual      body[ASI_DONE];
  MentalQual    mind[ASI_DONE];
}attribute_quality_t;

static const attribute_quality_t ATTR_QUAL[ATTR_DONE] = {
  [ATTR_STR] = {
    .attr = ATTR_STR,
    .body = {
      [ASI_DEBIL]         = PQ_LIGHT,
      [ASI_DETRI]         = PQ_TINY,
      [ASI_CON]           = PQ_SMALL | PQ_SHORT,
      [ASI_INIT]          = PQ_DENSE_MUSCLE | PQ_LONG_LIMB,
      [ASI_LVL_EVERY]     = PQ_LARGE_HANDS,
      [ASI_LVL_COMPOSITE] = PQ_HEAVY | PQ_GIG,
      [ASI_LVL_EVEN]      = PQ_METALLIC,
      [ASI_LVL_PRIME]     = PQ_HUGE | PQ_STONE_SKIN | PQ_LONG,
      [ASI_LVL_SQUARE]    = PQ_DENSE_MUSCLE | PQ_HEAVY | PQ_LARGE,
      [ASI_LVL_CAP]       = PQ_GIG | PQ_DENSE_MUSCLE
    },
    .mind = {
      [ASI_CON]           = MQ_ANXIOUS,
      [ASI_INIT]          = MQ_AGGRESSIVE | MQ_TERRITORIAL | MQ_FEARLESS
    }
  },
  [ATTR_DEX] = {
    .attr = ATTR_DEX,
    .body = {
      [ASI_DEBIL]         = PQ_WIDE | PQ_METALLIC | PQ_STONE_SKIN,
      [ASI_DETRI]         = PQ_HEAVY | PQ_GIG | PQ_THICK_FAT,
      [ASI_CON]           = PQ_TALL | PQ_HUGE,
      [ASI_INIT]          = PQ_LIGHT | PQ_SHORT_LIMB | PQ_TALL,
      [ASI_LVL_EVERY]     = PQ_TINY | PQ_LARGE_FEET,
      [ASI_LVL_COMPOSITE] = PQ_SMALL | PQ_LIGHT,
      [ASI_LVL_EVEN]      = PQ_WINGED | PQ_TAIL | PQ_OCTPED,
      [ASI_LVL_PRIME]     = PQ_ETHEREAL | PQ_FINNED,
      [ASI_LVL_CAP]       = PQ_WINGED
    },
    .mind = {
      [ASI_CON]           = MQ_CAUTIOUS | MQ_ABSENT | MQ_OBLIVIOUS,
      [ASI_INIT]          = MQ_CUNNING,
      [ASI_LVL_EVEN]      = MQ_PERCEPTIVE,
      [ASI_LVL_SQUARE]    = MQ_PATIENT | MQ_AWARE,
      [ASI_LVL_CAP]       = MQ_CUNNING
    }
  },
  [ATTR_CON] = {
    .attr = ATTR_CON,
    .body = {
      [ASI_DEBIL]         = PQ_TINY,
      [ASI_DETRI]         = PQ_SHORT,
      [ASI_CON]           = PQ_SMALL | PQ_LIGHT,
      [ASI_INIT]          = PQ_HUGE | PQ_THICK_FAT,
      [ASI_LVL_EVERY]     = PQ_METALLIC | PQ_GIG,
      [ASI_LVL_COMPOSITE] = PQ_HUGE | PQ_STONE_SKIN,
      [ASI_LVL_EVEN]      = PQ_HEAVY | PQ_WIDE,
      [ASI_LVL_PRIME]     = PQ_LARGE,
      [ASI_LVL_SQUARE]    = PQ_DENSE_MUSCLE,
      [ASI_LVL_CAP]       = PQ_STONE_SKIN
    },
    .mind = {
      [ASI_INIT]          = MQ_RESOLUTE | MQ_TERRITORIAL,
      [ASI_LVL_PRIME]     = MQ_FEARLESS,
      [ASI_LVL_SQUARE]    = MQ_RESOLUTE | MQ_PROTECTIVE,
      [ASI_LVL_CAP]       = MQ_UNREADABLE
    }
  },
  [ATTR_INT] = {
    .attr = ATTR_INT,
    .body = {
      [ASI_DETRI]         = PQ_TINY_HEAD,
      [ASI_CON]           = PQ_SMALL_HEAD,
      [ASI_INIT]          = PQ_SMALL,     // efficient body
      [ASI_LVL_COMPOSITE] = PQ_LIGHT,
      [ASI_LVL_CAP]       = PQ_ETHEREAL
    },
    .mind = {
      [ASI_DEBIL]         = MQ_ABSENT,
      [ASI_DETRI]         = MQ_SIMPLE,
      [ASI_CON]           = MQ_HIVE_MIND,
      [ASI_INIT]          = MQ_SENTIENT,
      [ASI_LVL_EVERY]     = MQ_UNREADABLE,
      [ASI_LVL_COMPOSITE] = MQ_CAUTIOUS,
      [ASI_LVL_EVEN]      = MQ_PERCEPTIVE,
      [ASI_LVL_PRIME]     = MQ_STRATEGIC | MQ_CUNNING,
      [ASI_LVL_SQUARE]    = MQ_CALCULATING | MQ_DISCIPLINED,
      [ASI_LVL_CAP]       = MQ_TELEPATH
    }
  },
  [ATTR_WIS] = {
    .attr = ATTR_WIS,
    .body = {
      [ASI_CON]           = PQ_TINY_HEAD,
      [ASI_LVL_SQUARE]    = PQ_ETHEREAL,
      [ASI_LVL_CAP]       = PQ_SHAPELESS
    },
    .mind = {
      [ASI_DETRI]         = MQ_OBLIVIOUS | MQ_ABSENT,
      [ASI_CON]           = MQ_SIMPLE,
      [ASI_INIT]          = MQ_AWARE | MQ_ATTUNED,
      [ASI_LVL_EVERY]     = MQ_PERCEPTIVE,
      [ASI_LVL_COMPOSITE] = MQ_UNREADABLE | MQ_PATIENT,
      [ASI_LVL_EVEN]      = MQ_DISCIPLINED,
      [ASI_LVL_PRIME]     = MQ_EMPATH,
      [ASI_LVL_SQUARE]    = MQ_SENTIENT,
      [ASI_LVL_CAP]       = MQ_TELEPATH
    }
  },
  [ATTR_CHAR] = {
    .attr = ATTR_CHAR,
    .body = {
      [ASI_INIT]          = PQ_TALL | PQ_LARGE | PQ_DENSE_MUSCLE,
      [ASI_LVL_EVEN]      = PQ_WINGED,
      [ASI_LVL_PRIME]     = PQ_ETHEREAL,
      [ASI_LVL_CAP]       = PQ_METALLIC | PQ_ETHEREAL
    },
    .mind = {
      [ASI_DEBIL]         = MQ_ABSENT,
      [ASI_DETRI]         = MQ_ANXIOUS | MQ_OBEDIENT | MQ_OBLIVIOUS,
      [ASI_CON]           = MQ_HIVE_MIND, 
      [ASI_INIT]          = MQ_EMPATH,
      [ASI_LVL_EVERY]     = MQ_LEADER | MQ_RESOLUTE | MQ_FEARLESS,
      [ASI_LVL_COMPOSITE] = MQ_UNREADABLE,
      [ASI_LVL_EVEN]      = MQ_CUNNING,
      [ASI_LVL_PRIME]     = MQ_TELEPATH | MQ_STRATEGIC,
      [ASI_LVL_SQUARE]    = MQ_AWARE,
      [ASI_LVL_CAP]       = MQ_EMPATH
    }
  }
};

typedef struct{
  StatType    stat;
  PhysQual    stature[SC_DONE];
  MentalQual  psyche[SC_DONE];
}stat_quality_t;

static const stat_quality_t STAT_QUAL[STAT_ENT_DONE] = {
  [STAT_REACH] = {
    .stat = STAT_REACH,
    .stature = {
      [SC_ABOVE] = PQ_LONG | PQ_LONG_LIMB | PQ_TALL | PQ_LARGE | PQ_HUGE | PQ_GIG,
    }
  },
  [STAT_DAMAGE] = {
    .stat = STAT_DAMAGE,
    .stature = {
      [SC_MIN]    = PQ_TINY,
      [SC_INFER]  = PQ_LIGHT,
      [SC_LESSER] = PQ_SHORT_LIMB,
      [SC_BELOW]  = PQ_SMALL,
      [SC_ABOVE]  = PQ_DENSE_MUSCLE,
      [SC_GREATER]= PQ_LARGE | PQ_HEAVY | PQ_STONE_SKIN,
      [SC_SUPER]  = PQ_METALLIC | PQ_HUGE,
      [SC_MAX]    = PQ_GIG
    } 
  },
  [STAT_HEALTH] = {
    .stat = STAT_HEALTH,
    .stature = {
      [SC_INFER]   = PQ_TINY,
      [SC_LESSER]  = PQ_SMALL,
      [SC_BELOW]   = PQ_LIGHT,
      [SC_ABOVE]   = PQ_LARGE,
      [SC_GREATER] = PQ_HUGE | PQ_WIDE,
      [SC_SUPER]   = PQ_HEAVY,
      [SC_MAX]     = PQ_GIG
    }
  },
  [STAT_ARMOR]  = {
    .stat = STAT_DAMAGE,
    .stature = {
      [SC_INFER]   = PQ_TINY,
      [SC_LESSER]  = PQ_SMALL,
      [SC_BELOW]   = PQ_LIGHT,
      [SC_ABOVE]   = PQ_DENSE_MUSCLE | PQ_THICK_SKIN | PQ_THICK_FUR | PQ_THICK_HIDE,
      [SC_GREATER] = PQ_THICK_SCALES | PQ_TOUGH_HIDE | PQ_TOUGH_SCALES,
      [SC_SUPER]   = PQ_STONE_SKIN,
      [SC_MAX]     = PQ_METALLIC
    }
  },
  [STAT_AGGRO]  = {
    .stat = STAT_AGGRO,
    .psyche = {
      [SC_INFER] = MQ_OBLIVIOUS | MQ_ABSENT,
      [SC_BELOW] = MQ_PATIENT | MQ_CUNNING,
      [SC_ABOVE] = MQ_ALERT,
      [SC_SUPER] = MQ_PROTECTIVE | MQ_AGGRESSIVE,
      [SC_MAX]    = MQ_TERRITORIAL 
    }
  },
  [STAT_ACTIONS] = {
    .stat = STAT_ACTIONS,
    .stature = {
      [SC_ABOVE] = PQ_TWIN_HEADED,
      [SC_SUPER] = PQ_TRI_HEADED,
      [SC_MAX]   = PQ_MANY_HEADED
    }
  },
  [STAT_ENERGY] = {
    .stat = STAT_ENERGY,
    .stature = {
      [SC_INFER] = PQ_TINY_HEAD,
      [SC_BELOW] = PQ_SMALL_HEAD,
      [SC_ABOVE] = PQ_HUGE | PQ_TWIN_HEADED,
      [SC_SUPER] = PQ_TRI_HEADED | PQ_GIG,
      [SC_MAX]   = PQ_MANY_HEADED | PQ_ETHEREAL
    },
    .psyche = {
      [SC_INFER] = MQ_ABSENT,
      [SC_BELOW] = MQ_SIMPLE,
      [SC_ABOVE] = MQ_SENTIENT | MQ_PATIENT,
      [SC_SUPER] = MQ_CALCULATING,
      [SC_MAX]   = MQ_ATTUNED | MQ_RESOLUTE
    }
  },
  [STAT_STAMINA] = {
    .stat = STAT_STAMINA,
    .stature = {
      [SC_MIN]   = PQ_TINY | PQ_WINGED,
      [SC_INFER] = PQ_THICK_FAT | PQ_SMALL | PQ_THICK_SCALES | PQ_SHORT_LIMB,
      [SC_BELOW] = PQ_SHORT,PQ_TOUGH_SCALES,
      [SC_ABOVE] = PQ_ETHEREAL | PQ_LARGE | PQ_GILLED | PQ_DENSE_MUSCLE,
      [SC_SUPER] = PQ_TALL | PQ_HUGE | PQ_FINNED | PQ_STONE_SKIN,
      [SC_MAX]   = PQ_GIG | PQ_METALLIC
    },
    .psyche = {
      [SC_ABOVE] = MQ_CALCULATING,
      [SC_SUPER] = MQ_DETERMINED | MQ_DISCIPLINED 
    }
  },
  [STAT_STAMINA_REGEN] = {},
  [STAT_ENERGY_REGEN] = {},
  [STAT_STAMINA_REGEN_RATE] = {},
  [STAT_ENERGY_REGEN_RATE] = {},
  [STAT_RAGE] = {}
};

typedef struct{
  Trait  trait;
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
 RACE_USE_TOOLS     = BIT64(0),
 RACE_USE_WEAPS     = BIT64(1),
 RACE_USE_ARMOR     = BIT64(2),
 RACE_USE_POTIONS   = BIT64(3),
 RACE_USE_SCROLLS   = BIT64(4),

 RACE_USE_MASK       = 0xFFULL,

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
 RACE_BUILD_SIMPLE    = BIT64(57),
 RACE_BUILD_BASIC     = BIT64(58),
 RACE_BUILD_FORGE     = BIT64(59),
 RACE_BUILD_SOPH      = BIT64(60),
 RACE_BUILD_KEEP      = BIT64(61),

 RACE_BUILD_MASK      = 0xFFULL << 56,
}RaceProp;

typedef enum{
  PROF_NONE,
  PROF_SOLDIER,
  PROF_ARCHER,
  PROF_MAGICIAN,
  PROF_MYSTIC,
  PROF_HEALER,
  PROF_LABORER,
  PROF_HAULER,
  PROF_RUNNER,
  PROF_GATHERER,
  PROF_MINER,
  PROF_CHOPPER,
  PROF_TENDER,
  PROF_SCHOLAR,
  PROF_HUNTER,
  PROF_FISHER,
  PROF_BUILDER,
  PROF_COOK,
  PROF_MEDIC,
  PROF_GUARD,
  PROF_SCRIBE,
  PROF_MERCHANT,
  PROF_WOOD,
  PROF_STONE,
  PROF_METAL,
  PROF_FLETCHER,
  PROF_TANNER,
  PROF_TAILOR,
  PROF_TRAPPER,
  PROF_ENGINEER,
  PROF_RESEARCH,
  PROF_ALCHEMIST,
  PROF_ARCHITECT,
  PROF_SURGEON,
  PROF_BOOKS,
  PROF_END
}Profession;

typedef struct{
  Profession    id;
  int           social_weights[SOC_DONE];
  const char*   social_name[SOC_DONE];
  uint64_t      rules;
  float         attributes[ATTR_DONE];
  int           skills[SKILL_DONE];
}define_prof_t;

static const define_prof_t DEFINE_PROF[PROF_END]= {
  [PROF_NONE]     = {PROF_NONE, 
    {[SOC_NONE]=100,[SOC_PRIMITIVE]=5,[SOC_CIVIL]=10,[SOC_HIGH]=12}
  },       
  [PROF_SOLDIER] = { PROF_SOLDIER,
    {[SOC_PRIMITIVE]=10, [SOC_MARTIAL]=10,[SOC_CIVIL]=5, [SOC_HIGH]=4},
    .skills = {[SKILL_SURV] = 400, [SKILL_ATH]=400, [SKILL_WEAP_MART]=600,
    }
  },
  [PROF_ARCHER] = { PROF_ARCHER,
    {[SOC_PRIMITIVE]=7, [SOC_MARTIAL]=10,[SOC_CIVIL]=7, [SOC_HIGH]=7},
    .skills = {[SKILL_PERCEPT]=600, [SKILL_ACRO] = 600, [SKILL_WEAP_BOW] = 600
    },
  },
  [PROF_MAGICIAN] = { PROF_MAGICIAN,
    {[SOC_PRIMITIVE]=0, [SOC_MARTIAL]=7,[SOC_CIVIL]=7, [SOC_HIGH]=10},
    .skills = {[SKILL_ARCANA] = 1200, [SKILL_INSIGHT]=600
    },
  },
  [PROF_MYSTIC] = { PROF_MYSTIC,
    {[SOC_PRIMITIVE]=7, [SOC_MARTIAL]=5,[SOC_CIVIL]=7, [SOC_HIGH]=7},
    .skills = {[SKILL_NATURE]=600, [SKILL_HERB] = 800,[SKILL_ANIM] = 800, [SKILL_MED] = 600, [SKILL_RELIG]=400
    },
  },
  [PROF_HEALER] = { PROF_HEALER,
    {[SOC_PRIMITIVE]=4, [SOC_MARTIAL]=4,[SOC_CIVIL]=10, [SOC_HIGH]=10},
    .skills = {[SKILL_RELIG]=1000,[SKILL_MED] = 1200, [SKILL_SPELL_DIV]=600
    },
  },
  [PROF_LABORER]  = {PROF_LABORER,
    {[SOC_PRIMITIVE]=10, [SOC_MARTIAL]=10,[SOC_CIVIL]=5, [SOC_HIGH]=4},
    {[SOC_PRIMITIVE]="Worker",[SOC_MARTIAL]="Peon"},
    MOB_LOC_FOREST | MOB_LOC_CAVE | MOB_LOC_DUNGEON,
    {[ATTR_CON]=0.125,[ATTR_STR]=0.125},
    .skills =  {[SKILL_STONE] = 400,[SKILL_WOOD]= 400, [SKILL_WEAP_MACE] = 400}
  },   
  [PROF_HAULER]   = {PROF_HAULER,
    {[SOC_HIVE] = 20, [SOC_PRIMITIVE]=10, [SOC_MARTIAL]=5,[SOC_CIVIL]=5,[SOC_HIGH]=2},
    {[SOC_PRIMITIVE]="Lugger",[SOC_MARTIAL]="Packhand"},
    MOB_LOC_FOREST | MOB_LOC_CAVE | MOB_LOC_DUNGEON,
    {[ATTR_CON]=0.1,[ATTR_STR]=0.15}
  },   
  [PROF_RUNNER]   = {PROF_RUNNER,
    {[SOC_HIVE]=5,[SOC_PRIMITIVE]=10, [SOC_MARTIAL]=5,[SOC_CIVIL]=5,[SOC_HIGH]=2},
    {[SOC_PRIMITIVE]="Runner",[SOC_MARTIAL]="Envoy"},
    MOB_LOC_FOREST | MOB_LOC_CAVE | MOB_LOC_DUNGEON,
    {[ATTR_DEX]=.15}
  },   
  [PROF_GATHERER] = {PROF_GATHERER,
    {[SOC_HIVE]=20,[SOC_PRIMITIVE]=10, [SOC_MARTIAL]=5,[SOC_CIVIL]=5,[SOC_HIGH]=2},
    {[SOC_PRIMITIVE]="Scavenger",[SOC_MARTIAL]="Rummager",[SOC_CIVIL]="Gatherer",[SOC_HIGH]="Forager"}
  },
  [PROF_MINER]={PROF_MINER,
    {[SOC_HIVE]=20,[SOC_PRIMITIVE]=6, [SOC_MARTIAL]=12,[SOC_CIVIL]=8,[SOC_HIGH]=4},
    {[SOC_HIVE]="Digger",[SOC_PRIMITIVE]="Digger",[SOC_MARTIAL]="Miner"},
    MOB_LOC_CAVE,
    {[ATTR_STR]=.2,[ATTR_CON]=.2},
    .skills = {[SKILL_STONE] = 600, [SKILL_WEAP_PICK] = 500}
  },
  [PROF_CHOPPER]={PROF_CHOPPER,
    {[SOC_HIVE]=20,[SOC_PRIMITIVE]=12, [SOC_MARTIAL]=12,[SOC_CIVIL]=8,[SOC_HIGH]=4},
    {[SOC_HIVE]="Cutter",[SOC_PRIMITIVE]="Cutter",[SOC_MARTIAL]="Logger", [SOC_CIVIL]="Lumberjack"},
    MOB_LOC_FOREST,
    {[ATTR_STR]=.2},
    .skills = {[SKILL_WOOD] = 600, [SKILL_WEAP_AXE] = 500}
  },
  [PROF_TENDER]={PROF_TENDER,
    {[SOC_HIVE]=15,[SOC_PRIMITIVE]=10, [SOC_MARTIAL]=12,[SOC_CIVIL]=10,[SOC_HIGH]=8},
    {[SOC_HIVE]="Broodhand",[SOC_PRIMITIVE]="Penkeeper",[SOC_MARTIAL]="Stockkeeper",[SOC_CIVIL]="Caretaker",[SOC_HIGH]="Animal Tender"}
  },
  [PROF_SCHOLAR]={PROF_SCHOLAR,
    {[SOC_MARTIAL]=2,[SOC_CIVIL]=12,[SOC_HIGH]=15}
  },
  [PROF_HUNTER]={PROF_HUNTER,
    {[SOC_HIVE]=8,[SOC_PRIMITIVE]=12, [SOC_MARTIAL]=12,[SOC_CIVIL]=8,[SOC_HIGH]=4}
  }, 
  [PROF_FISHER]={PROF_FISHER,
    {[SOC_PRIMITIVE]=8, [SOC_MARTIAL]=12,[SOC_CIVIL]=10,[SOC_HIGH]=6}
  }, 
  [PROF_BUILDER]={PROF_BUILDER,
    {[SOC_HIVE]=12,[SOC_PRIMITIVE]=6, [SOC_MARTIAL]=10,[SOC_CIVIL]=10,[SOC_HIGH]=5}
  },
  [PROF_COOK]={PROF_COOK,
    {[SOC_PRIMITIVE]=2, [SOC_MARTIAL]=10,[SOC_CIVIL]=12,[SOC_HIGH]=10}
  },
  [PROF_MEDIC]={PROF_MEDIC,
    {[SOC_PRIMITIVE]=2, [SOC_MARTIAL]=8,[SOC_CIVIL]=12,[SOC_HIGH]=12}
  },  
  [PROF_GUARD]={PROF_GUARD,
    {[SOC_HIVE]=15,[SOC_PRIMITIVE]=10, [SOC_MARTIAL]=12,[SOC_CIVIL]=15,[SOC_HIGH]=10}
  },  
  [PROF_SCRIBE]={PROF_SCRIBE,
    {[SOC_PRIMITIVE]=2, [SOC_MARTIAL]=5,[SOC_CIVIL]=10,[SOC_HIGH]=12}
  }, 
  [PROF_MERCHANT]={PROF_MERCHANT,
    {[SOC_PRIMITIVE]=2, [SOC_MARTIAL]=2,[SOC_CIVIL]=15,[SOC_HIGH]=20}
  },
  [PROF_WOOD]={PROF_WOOD,
    {[SOC_PRIMITIVE]=8, [SOC_MARTIAL]=10,[SOC_CIVIL]=15,[SOC_HIGH]=20}
  },
  [PROF_STONE]={PROF_STONE,
    {[SOC_PRIMITIVE]=5, [SOC_MARTIAL]=8,[SOC_CIVIL]=20,[SOC_HIGH]=22}
  }, 
  [PROF_METAL]={PROF_METAL,
    {[SOC_MARTIAL]=15,[SOC_CIVIL]=25,[SOC_HIGH]=25}
  },
  [PROF_FLETCHER]={PROF_FLETCHER,
    {[SOC_MARTIAL]=15,[SOC_CIVIL]=22,[SOC_HIGH]=22}
  },
  [PROF_TANNER]={PROF_TANNER,
    {[SOC_PRIMITIVE]=8, [SOC_MARTIAL]=6,[SOC_CIVIL]=18,[SOC_HIGH]=20}
  },
  [PROF_TAILOR]={PROF_TAILOR,
    {[SOC_MARTIAL]=5,[SOC_CIVIL]=25,[SOC_HIGH]=30}
  },
  [PROF_TRAPPER]={PROF_TRAPPER,
    {[SOC_PRIMITIVE]=8, [SOC_MARTIAL]=8,[SOC_CIVIL]=8,[SOC_HIGH]=2}
  },
  [PROF_ENGINEER]={PROF_ENGINEER,
    {[SOC_MARTIAL]=4,[SOC_CIVIL]=6,[SOC_HIGH]=15}
  },
  [PROF_RESEARCH]={PROF_RESEARCH,
    {[SOC_CIVIL]=8,[SOC_HIGH]=16}
  },
  [PROF_ALCHEMIST]={PROF_ALCHEMIST,
    {[SOC_MARTIAL]=3,[SOC_CIVIL]=6,[SOC_HIGH]=15}
  },
  [PROF_ARCHITECT]={PROF_ARCHITECT, 
    {[SOC_MARTIAL]=4,[SOC_CIVIL]=8,[SOC_HIGH]=16}
  },
  [PROF_SURGEON]={PROF_SURGEON,
    {[SOC_MARTIAL]=4,[SOC_CIVIL]=10,[SOC_HIGH]=18}
  },  
  [PROF_BOOKS]={},
};

typedef struct {
  SpeciesType   race;
  const char    name[MAX_NAME_LEN];
  EntityType    base_ent;
  RaceProps     props;
  Traits        traits;
  uint64_t      body;
  uint64_t      mind;
  uint64_t      feats;
  float         base_challenge;
}race_define_t;

static inline int SpecToIndex(SpeciesType spec){
  return __builtin_ctzll(spec);
}

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
  int         weight;
  Archetype   base,sub,promo;
  const char  main[MAX_NAME_LEN], second[MAX_NAME_LEN],rank[MAX_NAME_LEN];
  int         skills[SKILL_DONE];
  int         rankups[SKILL_DONE];
}race_class_t;

typedef struct{
  Profession    id;
  int           count;
  race_class_t  classes[10];
}define_race_class_t;

typedef struct{
  unsigned int  uid;
  ItemCategory  cat;
  int           item_id;
  int           weight,rarity;
}loot_t;

typedef enum{
  PROP_NONE           = 0,
  PROP_QUAL_TRASH     = BIT64(0),
  PROP_QUAL_POOR      = BIT64(1),
  PROP_QUAL_STANDARD  = BIT64(2),
  PROP_QUAL_WELL      = BIT64(3),
  PROP_QUAL_FINE      = BIT64(4),
  PROP_QUAL_SUPER     = BIT64(5),
  PROP_QUAL_EXPERT    = BIT64(6),
  PROP_QUAL_MASTER    = BIT64(7),
  PROP_QUAL_ARTIFACT  = BIT64(8),
  
  PROP_MAT_CLOTH      = BIT64(9),
  PROP_MAT_LEATHER    = BIT64(10),
  PROP_MAT_BONE       = BIT64(11),
  PROP_MAT_WOOD       = BIT64(12),
  PROP_MAT_STONE      = BIT64(13),
  PROP_MAT_METAL      = BIT64(14),
  PROP_MAT_LIQUID     = BIT64(15), 
}ItemProp;

typedef enum{
  PROP_WEAP_NONE        = 0,
  PROP_WEAP_LIGHT       = BIT64(0),
  PROP_WEAP_HEAVY       = BIT64(1),
  PROP_WEAP_SIMP        = BIT64(2),
  PROP_WEAP_MARTIAL     = BIT64(3),
  PROP_WEAP_TWO_HANDED  = BIT64(4),
  PROP_WEAP_REACH       = BIT64(5),
  PROP_WEAP_RANGED      = BIT64(6),
  PROP_WEAP_AMMO        = BIT64(7),
  PROP_WEAP_THROW       = BIT64(8),
}WeaponProp;

typedef enum{
  PROP_ARMOR_NONE      = 0,

}ArmorProp;

typedef enum{
  PROP_CONS_NONE       = 0,
  PROP_CONS_HEAL       = BIT64(0),
}ConsumeProp;

typedef uint64_t ItemProps;
typedef uint64_t ArmorProps;
typedef uint64_t ConsumeProps;
typedef uint64_t WeaponProps;
typedef struct{
  ArmorType          type;
  int                armor_class;
  damage_reduction_t dr_base,dr_rarity;
  int                weight,cost,durability;
  AttributeType      modifier,required;
  int                mod_max, req_min;
  ItemProps          i_props;
  SkillType          skill;
}armor_def_t;

typedef struct{
  WeaponType      type;
  int             cost,weight,penn,drain,reach_bonus,durability;
  ItemProps       i_props;
  WeaponProps     w_props;
  AbilityID       ability;
  SkillType       skill;
}weapon_def_t;
WeaponType GetWeapTypeBySkill(SkillType s);

typedef struct{
  ConsumeType     type;
  int             cost,weight,quanity,amount;
  ItemProps       i_props;
  ConsumeProps    w_props;
  AbilityID       ability;
  SkillType       skill;
}consume_def_t;

extern armor_def_t ARMOR_TEMPLATES[ARMOR_DONE];
extern weapon_def_t WEAPON_TEMPLATES[WEAP_DONE];
extern consume_def_t CONSUME_TEMPLATES[CONS_DONE];


typedef struct value_affix_s value_affix_t;
typedef float (*AffixFn)(value_affix_t* self, int val);
float AffixAdd(value_affix_t* self, int val);
float AffixSub(value_affix_t* self, int val);
float AffixMul(value_affix_t* self, int val);
float AffixFract(value_affix_t* self, int val);
float AffixBase(value_affix_t* self, int val);

struct value_affix_s{
  ValueCategory modifies;
  ValueAffix    affix;
  int           val;
  AffixFn       affix_mod;
};

typedef struct{
  int            propID;
  int            num_aff;
  value_affix_t  val_change[4];
  SkillType      add_skill;
}item_prop_mod_t;

value_affix_t* InitValueAffixFromMod(value_affix_t mod);
int GetItemPropsMods(ItemProps props, uint64_t cat_props, ItemCategory cat, item_prop_mod_t **mods);
item_prop_mod_t* GetItemPropMods(ItemCategory cat, uint64_t prop);

static item_prop_mod_t PROP_MODS[ITEM_DONE][NUM_ITEM_PROPS]={
  [ITEM_NONE] = {
    {PROP_QUAL_TRASH,4,
      .val_change = {
        {VAL_ADV_HIT,AFF_SUB,2},
        {VAL_ADV_SAVE,AFF_SUB,2},
        {VAL_WORTH,AFF_FRACT,33},
        {VAL_DURI,AFF_FRACT,50}
      }
    },
    {PROP_QUAL_POOR,4,
      .val_change = {
        {VAL_DURI,AFF_FRACT,75},
        {VAL_WORTH,AFF_FRACT,66},
        {VAL_ADV_HIT,AFF_SUB,1},
        {VAL_ADV_SAVE,AFF_SUB,1}
      }
    },
    /*
       {PROP_QUAL_WELL,125, VAL_WORTH,AFF_FRACT},
       {PROP_QUAL_WELL,120, VAL_DURI,AFF_FRACT},
       {PROP_QUAL_FINE,150, VAL_WORTH,AFF_FRACT},
       {PROP_QUAL_FINE,140, VAL_DURI,AFF_FRACT},
       {PROP_QUAL_SUPER,175, VAL_WORTH,AFF_FRACT},
       {PROP_QUAL_SUPER,160, VAL_DURI,AFF_FRACT},
       {PROP_QUAL_EXPERT,200, VAL_WORTH,AFF_FRACT},
       {PROP_QUAL_EXPERT,180, VAL_DURI,AFF_FRACT},
       {PROP_QUAL_MASTER,225, VAL_WORTH,AFF_FRACT},
       {PROP_QUAL_MASTER,200, VAL_DURI,AFF_FRACT},
       {PROP_QUAL_ARTIFACT,250, VAL_WORTH,AFF_FRACT},
       {PROP_QUAL_ARTIFACT,300, VAL_DURI,AFF_FRACT},
       */
    {PROP_MAT_BONE,2,
      .val_change = {
         {VAL_DURI,AFF_FRACT,75},
         {VAL_PENN, AFF_SUB, 1},
      }
    },
    {PROP_MAT_STONE,2, 
      .val_change = {
         {VAL_DURI,AFF_FRACT,90},
         {VAL_HIT, AFF_SUB, 1},
      }
    },
    {PROP_MAT_WOOD, 2,
      .val_change = {
         {VAL_DURI,AFF_FRACT,60},
         {VAL_PENN, AFF_SUB, 2},
      }
    },
  },
  [ITEM_WEAPON] = {
    {PROP_WEAP_LIGHT, 1,
      {VAL_PENN, AFF_SUB, 1},
    },
    {PROP_WEAP_HEAVY,1,
      {VAL_PENN, AFF_ADD, 1},
    },
    {PROP_WEAP_SIMP,
      .add_skill = SKILL_WEAP_SIMP
    },
    {PROP_WEAP_MARTIAL,1,

      .val_change = (value_affix_t){ VAL_PENN, AFF_ADD, 1},
      .add_skill  = SKILL_WEAP_SIMP
    },
    {PROP_WEAP_TWO_HANDED, 1,
      .val_change = (value_affix_t){ VAL_PENN, AFF_ADD, 1},
    },
    {PROP_WEAP_REACH},
    {PROP_WEAP_RANGED},
    {PROP_WEAP_AMMO},
  },
  [ITEM_ARMOR] = {

  }
};

static int CountItemPropMods(ItemProps props, WeaponProps wprops) {
    int count = 0;

    while (props) {
        props &= props - 1;
        count++;
    }

    while (wprops) {
        wprops &= wprops - 1;
        count++;
    }

    return count;
}

typedef struct{
  ActionSlot  id;
  ActionType  allowed[ACTION_SLOTTED];
}define_slot_actions;

#endif
