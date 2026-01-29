#ifndef __GAME_DEFINITION__
#define __GAME_DEFINITION__
#include "game_enum.h"

#define BASE_ATTR_VAL 4
#define MAX_NAME_LEN 64
#define NUM_CLASS 15

#define NUM_ITEM_PROPS 18
#define NUM_WEAP_PROPS 8
#define NUM_ARMOR_PROPS 8
// Natural weapons (bits 0–7)
#define PQ_WEAPON_SHIFT   0
#define PQ_WEAPON_BITS    16

// Size (bits 16–23)
#define PQ_SIZE_SHIFT     0
#define PQ_SIZE_BITS      8

// Shape / proportions (bits 24–31)
#define PQ_SHAPE_SHIFT    (PQ_SIZE_SHIFT + PQ_SIZE_BITS)
#define PQ_SHAPE_BITS     8

// Weight / density (bits 32–39)
#define PQ_WEIGHT_SHIFT   (PQ_SHAPE_SHIFT + PQ_SHAPE_BITS)
#define PQ_WEIGHT_BITS    8

// Locomotion (bits 40–47)
#define PQ_LOCO_SHIFT     (PQ_WEIGHT_SHIFT + PQ_WEIGHT_BITS)
#define PQ_LOCO_BITS      8

// Special biology (bits 48–63)
#define PQ_SPECIAL_SHIFT  (PQ_LOCO_SHIFT + PQ_LOCO_BITS)
#define PQ_SPECIAL_BITS   16

#define SOC_W(w)        { (w), NULL }
#define SOC_N(w, n)     { (w), (n) }
#define NUM_RES_DEF 20
#define MAX_SEN_DIST 21
#define MAX_PATHS 3
#define MAX_RANKS 4
#define FACTION_NAME(id)  Faction_Name(id)
#define SPEC_RELATE_NEGATIVE ( \
    SPEC_FEAR    | \
    SPEC_AVOID   | \
    SPEC_HOSTILE | \
    SPEC_HUNTS   )

#define SPEC_RELATE_POSITIVE ( \
    SPEC_FRIEND  | \
    SPEC_REVERED | \
    SPEC_KIN     )

#define SPEC_RELATE_NEUTRAL ( \
    SPEC_RELATE_NONE | \
    SPEC_INDIF       )

#define SPEC_DESIRE ( \
    SPEC_HUNTS | \
    SPEC_WANTS       )

#define PQ_SIZE_MASK (\
    PQ_TINY | PQ_SMALL | PQ_LARGE | PQ_HUGE | PQ_GIG)
 
#define PQ_WEIGHT_MASK (\
    PQ_LIGHT | PQ_HEAVY | PQ_ETHEREAL)
   
#define PQ_VOL_MASK (\
    PQ_NO_BONES | PQ_SHORT | PQ_TALL | PQ_LONG | PQ_TAIL |\
    PQ_LONG_LIMB | PQ_SHORT_LIMB | PQ_LARGE_HANDS | PQ_LARGE_HEAD |\
    PQ_LARGE_FEET | PQ_TINY_HEAD | PQ_SMALL_HEAD | PQ_WIDE |\
    PQ_DENSE_MUSCLE)
  
#define MAX_FACTIONS 20
//__builtin_ctzll
#define BCTZL(uint) (int){__builtin_ctzll(uint)}  

extern int NUM_FACTIONS;

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
  SPEC_SKELETAL   = BIT64(10),
  SPEC_VAMPIRIC   = BIT64(11),
  SPEC_CANIFORM   = BIT64(12),
  SPEC_RODENT     = BIT64(13),
  SPEC_RUMINANT   = BIT64(14),
  SPEC_SULKING    = BIT64(15),
  SPEC_DONE       = BIT64(16),
}SpeciesType;

typedef enum{
  SPEC_RELATE_NONE = 0,
  SPEC_FEAR        = BIT64(0),
  SPEC_AVOID       = BIT64(1),
  SPEC_INDIF       = BIT64(2),
  SPEC_HOSTILE     = BIT64(3),
  SPEC_CAUT        = BIT64(4),
  SPEC_HUNTS       = BIT64(5),
  SPEC_FRIEND      = BIT64(6),
  SPEC_REVERED     = BIT64(7),
  SPEC_KIN         = BIT64(8),
  SPEC_WANTS       = BIT64(9),
  SPEC_RELATE_DONE  = BIT64(10),
}SpeciesRelate;

typedef struct {
  SpeciesRelate rel;
  float         val[TREAT_DONE];
}species_treatment_t;

static species_treatment_t TREATMENT[10] = {
  {SPEC_RELATE_NONE}, 
  {SPEC_FEAR, 
    {
      [TREAT_KILL]      = 0,
      [TREAT_DEFEND]    = 0.75,
      [TREAT_EAT]       = 0,
      [TREAT_HELP]      = 0,
      [TREAT_FLEE]      = 1.0,
    }
  },
  {SPEC_AVOID,
    {
      [TREAT_KILL]      = 0.125,
      [TREAT_DEFEND]    = 0.875,
      [TREAT_EAT]       = 0,
      [TREAT_HELP]      = 0,
      [TREAT_FLEE]      = 0.25,
    }
  },
  {SPEC_INDIF,
    {
      [TREAT_KILL]      = 0.125,
      [TREAT_DEFEND]    = 1,
      [TREAT_EAT]       = 0,
      [TREAT_HELP]      = 0.075,
      [TREAT_FLEE]      = 0.125,
    }
  },  
  {SPEC_HOSTILE,
    {
      [TREAT_KILL]      = 0.875,
      [TREAT_DEFEND]    = 1,
      [TREAT_EAT]       = 0,
      [TREAT_HELP]      = 0,
      [TREAT_FLEE]      = 0.125,
    }
  },
  {SPEC_CAUT,
    {
      [TREAT_KILL]      = 0.125,
      [TREAT_DEFEND]    = 0.875,
      [TREAT_EAT]       = 0,
      [TREAT_HELP]      = 0.125,
      [TREAT_FLEE]      = 0.5,
    }
  }, 
  {SPEC_HUNTS,
    {
      [TREAT_KILL]      = 0.875,
      [TREAT_DEFEND]    = 1,
      [TREAT_EAT]       = 1,
      [TREAT_HELP]      = 0,
      [TREAT_FLEE]      = 0.125,
    }
  },
  {SPEC_FRIEND,
    {
      [TREAT_KILL]      = 0,
      [TREAT_DEFEND]    = 0.375,
      [TREAT_EAT]       = 0,
      [TREAT_HELP]      = 0.75,
      [TREAT_FLEE]      = 0,
    }
  },
  {SPEC_REVERED,
    {
      [TREAT_KILL]      = 0,
      [TREAT_DEFEND]    = 0.125,
      [TREAT_EAT]       = 0,
      [TREAT_HELP]      = 1.0,
      [TREAT_FLEE]      = 0,
    }
  },
  {SPEC_KIN,
    {
      [TREAT_KILL]      = 0,
      [TREAT_DEFEND]    = 0.25,
      [TREAT_EAT]       = 0,
      [TREAT_HELP]      = 1.0,
      [TREAT_FLEE]      = 0,
    }
  }
};

typedef struct{
  SpeciesType     spec;
  SpeciesType     relation[__builtin_ctzll(SPEC_RELATE_DONE)];
}species_relation_t;
extern species_relation_t SPEC_ALIGN[__builtin_ctzll(SPEC_DONE)];

typedef uint32_t Faction;
typedef struct{
  const char  *name;
  SpeciesType species;
  int         bio_pref[BIO_DONE];
  int         member_ratio[ENT_DONE];
  Faction     id;
  MobRules    rules;
}faction_t;

extern faction_t* FACTIONS[MAX_FACTIONS];

faction_t* InitFaction(const char* name);
Faction RegisterFaction(const char* name);
Faction RegisterFactionByType(ent_t* e);
const char* Faction_Name(Faction id);
static inline faction_t* GetFactionBySpec(SpeciesType spec){
  for (int i = 0; i < NUM_FACTIONS; i++){
    if(FACTIONS[i]->species == spec)
      return FACTIONS[i];
  }

  return NULL;
  
}

static inline faction_t* GetFactionByID(Faction id){
  for (int i = 0; i < NUM_FACTIONS; i++){
    if(FACTIONS[i]->id == id)
      return FACTIONS[i];
  }

  return NULL;
}
typedef struct{
  MobRule     party;
  SpeciesType species;
  int         prof_ratio[PROF_END];
}mob_group_t;

typedef struct{
  Faction        faction;
  mob_group_t    groups[8];
  choice_pool_t* choices;
}faction_groups_t;
faction_groups_t* InitFactionGroups(Faction id, int desired);

void InitMobGroup(faction_groups_t** f, MobRule size, int index);
typedef enum{
  RES_NONE = -1,
  RES_VEG  = BIT64(0),
  RES_MEAT = BIT64(1),
  RES_BONE = BIT64(2),
  RES_STONE = BIT64(3),
  RES_METAL = BIT64(4),
  RES_WOOD  = BIT64(5),
  RES_WATER = BIT64(6),
  RES_BLOOD = BIT64(7),
  RES_DONE = 8
}Resource;

typedef struct{
  const char* name;
  Resource    type;
  Resource    attached;
  int         smell;
  uint64_t    amount;
}resource_t;

typedef struct{
  const char*     name;
  Resource        type;
  ObjectCategory  cat;
  uint64_t        cat_flags;
  int             quantity, smell;      
  Resource        attached;
}define_resource_t;
extern define_resource_t DEF_RES[20];
static inline define_resource_t* GetResourceDef(Resource type){
  for(int i = 0; i < NUM_RES_DEF; i++){
    define_resource_t *res = &DEF_RES[i];
    if(res->type != type)
      continue;

    return res;
  }

  return NULL;
}
static inline define_resource_t* GetResourceByCatFlags(Resource type, ObjectCategory cat, uint64_t flags){
  for(int i = 0; i < NUM_RES_DEF; i++){
    define_resource_t *res = &DEF_RES[i];
    if(res->type != type)
      continue;

    if(res->cat != cat)
      continue;

    if((res->cat_flags & flags) > 0)
      return res;
  }

  return NULL;
}

typedef enum{
  NREQ_NONE   = 0,
  NREQ_MIN    = 0x0100,
  NREQ_LOW    = 0x0200,
  NREQ_AVG    = 0x0300,
  NREQ_HIGH   = 0x0400,
  NREQ_GREAT  = 0x0500,
  NREQ_SUPER  = 0x0700,
  NREQ_MAX    = 0x0A00,
}NeedRequirements;

typedef struct{
  Needs         id;
  ent_t*        owner;
  uint64_t      resource;
  local_ctx_t*  goal;
  StatType      stat_rel;
  NeedStatus    status;
  bool          activity;
  int           vals[NEED_DONE];
  int           prio, val, meter;
}need_t;

need_t* InitNeed(Needs id, ent_t* owner);
void NeedStep(need_t* n);
void NeedIncrement(need_t*, ent_t* owner, int amount);
Needs NeedGetGreatest(need_t* list[N_DONE]);
void NeedFulfill(need_t* n, int amount);
void OnNeedStatus(EventType event, void* data, void* user);

typedef struct{
  Needs             type;
  uint64_t          body;
  uint64_t          mind;
  uint64_t          req;
}define_need_req_t;

extern define_need_req_t NEEDS_REQ[N_DONE][8];

typedef struct{
  uint64_t    body;
  uint64_t    mind;
  uint64_t    weaps;
  uint64_t    covering;
}mob_flags_t;

typedef struct{
  EntityType  id;
  char        name[MAX_NAME_LEN];
  MobRules    rules;
  SpeciesType race;
  int         weight[MAP_DONE];
  int         cost;
  float       diff;
  SocietyType civ;
  uint64_t    has;
  uint64_t    eats;
  mob_flags_t flags;
}mob_define_t;

typedef struct{
  MobType     type;
  MobRules    m_rules;
  SpeciesType s_rules;
}define_mobtype_t;
extern define_mobtype_t MOB_THEME[MT_DONE];

static MobType MobTypeByFlags(MobRules m, SpeciesType s){
  for (int i = 0; i < MT_DONE; i++){
    if((MOB_THEME[i].m_rules & m) == 0)
      continue;


    if(MOB_THEME[i].s_rules > 0 && (MOB_THEME[i].s_rules & s) == 0)
      continue;

    return i;
  }

  return MT_NONE;
}

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
  Needs      need;
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
  SkillType         id;
  SkillType         counter;
  int               base;
  AttributeType     attr;
  ProficiencyCheck  type;
}skill_proficiency_bonus_t;

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
  Magnitude     mag;
  SkillType     skill;
  bool          skills[SKILL_DONE];
}skill_relation_t;

typedef struct{
  SkillRank     rank;
  int           skill_thresh, penalty;
  ModifierType  proficiency;
}define_skill_rank_t;


SkillRate SkillRateLookup(SkillType);
typedef enum {
  PQ_NONE = 0,
  /* Size (16–23) */
  PQ_TINY         = 1ULL << (PQ_SIZE_SHIFT + 0),
  PQ_SMALL        = 1ULL << (PQ_SIZE_SHIFT + 1),
  PQ_LARGE        = 1ULL << (PQ_SIZE_SHIFT + 2),
  PQ_HUGE         = 1ULL << (PQ_SIZE_SHIFT + 3),
  PQ_GIG          = 1ULL << (PQ_SIZE_SHIFT + 4),

  /* Shape / proportions (24–31) */
  PQ_SHORT        = 1ULL << (PQ_SHAPE_SHIFT + 0),
  PQ_TALL         = 1ULL << (PQ_SHAPE_SHIFT + 1),
  PQ_LONG         = 1ULL << (PQ_SHAPE_SHIFT + 2),
  PQ_WIDE         = 1ULL << (PQ_SHAPE_SHIFT + 3),
  PQ_LONG_LIMB    = 1ULL << (PQ_SHAPE_SHIFT + 4),
  PQ_SHORT_LIMB   = 1ULL << (PQ_SHAPE_SHIFT + 5),

  /* Weight / density (32–39) */
  PQ_LIGHT        = 1ULL << (PQ_WEIGHT_SHIFT + 0),
  PQ_HEAVY        = 1ULL << (PQ_WEIGHT_SHIFT + 1),
  PQ_DENSE_MUSCLE = 1ULL << (PQ_WEIGHT_SHIFT + 2),

  /* Locomotion (40–47) */
  PQ_BIPED        = 1ULL << (PQ_LOCO_SHIFT + 0),
  PQ_QUADPED      = 1ULL << (PQ_LOCO_SHIFT + 1),
  PQ_OCTPED       = 1ULL << (PQ_LOCO_SHIFT + 2),
  PQ_WINGED       = 1ULL << (PQ_LOCO_SHIFT + 3),
  PQ_FINNED       = 1ULL << (PQ_LOCO_SHIFT + 4),
  PQ_GILLED       = 1ULL << (PQ_LOCO_SHIFT + 5),
  PQ_TAIL         = 1ULL << (PQ_LOCO_SHIFT + 6),

  /* Special biology (48–63) */
  PQ_ETHEREAL     = 1ULL << (PQ_SPECIAL_SHIFT + 0),
  PQ_SHAPELESS    = 1ULL << (PQ_SPECIAL_SHIFT + 1),
  PQ_SMALL_HEAD   = 1ULL << (PQ_SPECIAL_SHIFT + 2),
  PQ_TINY_HEAD    = 1ULL << (PQ_SPECIAL_SHIFT + 3),
  PQ_TWIN_HEADED  = 1ULL << (PQ_SPECIAL_SHIFT + 4),
  PQ_TRI_HEADED   = 1ULL << (PQ_SPECIAL_SHIFT + 5),
  PQ_MANY_HEADED  = 1ULL << (PQ_SPECIAL_SHIFT + 6),
  PQ_LARGE_HEAD   = 1ULL << (PQ_SPECIAL_SHIFT + 7),
  PQ_NO_HEAD      = 1ULL << (PQ_SPECIAL_SHIFT + 8),
  PQ_LARGE_FEET   = 1ULL << (PQ_SPECIAL_SHIFT + 9),
  PQ_LARGE_HANDS  = 1ULL << (PQ_SPECIAL_SHIFT + 10),
  PQ_LARGE_EARS   = 1ULL << (PQ_SPECIAL_SHIFT + 11),
  PQ_LARGE_EYES   = 1ULL << (PQ_SPECIAL_SHIFT + 12),
  PQ_MANY_EYES    = 1ULL << (PQ_SPECIAL_SHIFT + 13),
  PQ_ONE_EYE      = 1ULL << (PQ_SPECIAL_SHIFT + 14),
  PQ_NO_EYES      = 1ULL << (PQ_SPECIAL_SHIFT + 15),
  PQ_LARGE_NOSE     = 1ULL << (PQ_SPECIAL_SHIFT + 16),
  PQ_SENSITIVE_NOSE = 1ULL << (PQ_SPECIAL_SHIFT + 17),
  PQ_NO_BONES       = 1ULL << (PQ_SPECIAL_SHIFT + 18),

} PhysQual;

typedef enum {
  PW_NONE = 0,

  /* Claws */
  PQ_CLAWS        = 1ULL << 0,
  PQ_SHARP_CLAWS  = 1ULL << 1,
  PQ_TOUGH_CLAWS  = 1ULL << 2,
  /* Horns */
  PQ_HORNED       = 1ULL << 3,
  PQ_SHARP_HORNS  = 1ULL << 4,
  PQ_TOUGH_HORNS  = 1ULL << 5,

  /* Teeth / mouth */
  PQ_TEETH        = 1ULL << 6,
  PQ_SHARP_TEETH  = 1ULL << 7,
  PQ_TOUGH_TEETH  = 1ULL << 8,
  PQ_FANGS        = 1ULL << 9,
  PQ_POISON_FANGS = 1ULL << 10,
  /* Future-safe */
  // PQ_MANDIBLES  = 1ULL << 9,
  // PQ_TENTACLES  = 1ULL << 10,
  // PQ_SPIKED_TAIL= 1ULL << 11,

} PhysWeapon;

typedef enum {
  PB_NONE = 0,

  /* Soft coverings */
  PQ_FUR           = 1ULL << 0,
  PQ_THICK_FUR     = 1ULL << 1,
  PQ_THICK_SKIN    = 1ULL << 2,
  PQ_THICK_FAT     = 1ULL << 3,

  /* Hide / leather-like */
  PQ_HIDE          = 1ULL << 4,
  PQ_THICK_HIDE    = 1ULL << 5,
  PQ_TOUGH_HIDE    = 1ULL << 6,

  /* Scales / plates */
  PQ_SCALES        = 1ULL << 7,
  PQ_THICK_SCALES  = 1ULL << 8,
  PQ_TOUGH_SCALES  = 1ULL << 9,

  /* Exotic */
  PQ_STONE_SKIN    = 1ULL << 10,
  PQ_METALLIC     = 1ULL << 11,

  /* Future-safe */
  // PQ_CHITINOUS   = 1ULL << 9,
  // PQ_CRYSTALLINE = 1ULL << 10,

} PhysBody;

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
  /* -------------------------------------------------- */
  /* Capability flags (0–15) */
  /* -------------------------------------------------- */
  TRAIT_CAN_MELEE     = BIT64(0),
  TRAIT_CAN_CAST      = BIT64(1),
  TRAIT_CAN_HEAL      = BIT64(2),
  TRAIT_CAN_SHOOT     = BIT64(3),
  TRAIT_CAN_STEALTH   = BIT64(4),
  TRAIT_CAN_DEFEND    = BIT64(5),

  TRAIT_PREF_MELEE    = BIT64(6),
  TRAIT_PREF_CAST     = BIT64(7),
  TRAIT_PREF_HEAL     = BIT64(8),
  TRAIT_PREF_SHOOT    = BIT64(9),
  TRAIT_PREF_COVER    = BIT64(10),
  TRAIT_PREF_SNEAK    = BIT64(11),

  TRAIT_CAP_MASK      = (BIT64(16) - 1),   /* bits 0–15 */


  /* -------------------------------------------------- */
  /* Resistance category tags (16–23) */
  /* -------------------------------------------------- */
  TRAIT_PHYS_RESIST   = BIT64(16),
  TRAIT_ELE_RESIST    = BIT64(17),
  TRAIT_MAGIC_RESIST  = BIT64(18),

  TRAIT_RESIST_TAG_MASK = (
      BIT64(16) |
      BIT64(17) |
      BIT64(18)
      ),


  /* -------------------------------------------------- */
  /* Damage-type resistances (24–39) */
  /* -------------------------------------------------- */
  TRAIT_BLUNT_RESIST    = BIT64(24),
  TRAIT_PIERCE_RESIST   = BIT64(25),
  TRAIT_SLASH_RESIST    = BIT64(26),
  TRAIT_FIRE_RESIST     = BIT64(27),
  TRAIT_COLD_RESIST     = BIT64(28),
  TRAIT_ACID_RESIST     = BIT64(29),
  TRAIT_POISON_RESIST   = BIT64(30),
  TRAIT_PSYCHIC_RESIST  = BIT64(31),
  TRAIT_RADIANT_RESIST  = BIT64(32),
  TRAIT_NECROTIC_RESIST = BIT64(33),
  TRAIT_FORCE_RESIST    = BIT64(34),

  TRAIT_RESIST_SCHOOL_MASK = (
      BIT64(24) | BIT64(25) | BIT64(26) |
      BIT64(27) | BIT64(28) | BIT64(29) |
      BIT64(30) | BIT64(31) | BIT64(32) |
      BIT64(33) | BIT64(34)
      ),


  /* -------------------------------------------------- */
  /* Weapon expertise (40–47) */
  /* -------------------------------------------------- */
  TRAIT_EXP_AXE        = BIT64(40),
  TRAIT_EXP_BOW        = BIT64(41),
  TRAIT_EXP_DAGGER     = BIT64(42),
  TRAIT_EXP_MACE       = BIT64(43),
  TRAIT_EXP_SWORD      = BIT64(44),

  TRAIT_EXP_MASK = (
      BIT64(40) |
      BIT64(41) |
      BIT64(42) |
      BIT64(43) |
      BIT64(44)
      ),


  /* -------------------------------------------------- */
  /* Vision traits (48–55) */
  /* -------------------------------------------------- */
  TRAIT_VISION_DARK    = BIT64(48),

  TRAIT_VISION_MASK   = BIT64(48),


  /* -------------------------------------------------- */
  /* Advantage traits (56–63) */
  /* -------------------------------------------------- */
  TRAIT_ADV_FEAR      = BIT64(56),
  TRAIT_ADV_CHARM     = BIT64(57),

  TRAIT_ADV_MASK = (
      BIT64(56) |
      BIT64(57)
      )
}Trait;

typedef struct{
  PhysQual      pq;
  Traits        traits;
  FeatFlags     feats;
  AbilityID     abilities[4];
}phys_qualities_t;

typedef struct{
  PhysWeapon    pq;
  Traits        traits;
  FeatFlags     feats;
  int           num_abilities;
  AbilityID     abilities[4];
  int           skillup[SKILL_DONE];
}natural_weapons_t;

typedef struct{
  PhysBody      pq;
  Traits        traits;
  FeatFlags     feats;
  AbilityID     abilities[4];
  int           skillup[SKILL_DONE];
}body_covering_t;

typedef struct{
  MentalQual    mq;
  Traits        traits;
  FeatFlags     feats;
  int           num_abilities;
  AbilityID     abilities[4];
  int           skillup[SKILL_DONE];
}ment_qualities_t;

static const ment_qualities_t MIND[25] = {
  {MQ_SIMPLE, TRAIT_ADV_FEAR | TRAIT_ADV_CHARM},
  {MQ_OBLIVIOUS, TRAIT_ADV_CHARM},
  {MQ_ALERT,0,
    FEAT_ALERT | FEAT_DODGE
  },
  {MQ_CUNNING,0,FEAT_STEALTHY | FEAT_FOCUSED_MIND},
  {MQ_AGGRESSIVE, 0, 0, 1, ABILITY_DASH},
};

static const natural_weapons_t NAT_WEAPS[17] = {
  {PQ_TEETH, .num_abilities = 1, .abilities = ABILITY_BITE,
    .skillup = {
      [SKILL_WEAP_NONE] = 350
    }
  },
  {PQ_SHARP_TEETH, .num_abilities = 1, .abilities = ABILITY_BITE,
    .skillup = {
      [SKILL_WEAP_NONE] = 800
    }
  },
  
  {PQ_TOUGH_TEETH, .num_abilities = 0, .abilities = ABILITY_CHEW,
   .skillup = {
      [SKILL_WEAP_NONE] = 800
    }
  },
  {PQ_HORNED, .num_abilities = 1, .abilities = ABILITY_RAM,
    .skillup = {[SKILL_WEAP_NONE] = 350}
  },
  {PQ_CLAWS, .skillup = {[SKILL_WEAP_NONE] = 350}},
  {PQ_TOUGH_CLAWS, .num_abilities = 1, .abilities = ABILITY_SWIPE,
    .skillup = {[SKILL_WEAP_NONE] = 400}},
  {PQ_SHARP_CLAWS, .skillup = {[SKILL_WEAP_NONE]=800}},
  {PQ_FANGS, .skillup = {[SKILL_WEAP_NONE] = 350}},
  {PQ_POISON_FANGS, .num_abilities = 1, .abilities = ABILITY_BITE_POISON,
    .skillup = {[SKILL_WEAP_NONE] = 350}},

};

static const body_covering_t COVERINGS[35] = {
  {PQ_FUR, .skillup = {[SKILL_ARMOR_NATURAL] = 800}},
  {PQ_THICK_FUR, TRAIT_SLASH_RESIST | TRAIT_COLD_RESIST,
    .abilities = ABILITY_ARMOR_DR
  },
  {PQ_HIDE, .skillup = {[SKILL_ARMOR_NATURAL] = 800}},
  {PQ_THICK_HIDE, TRAIT_FIRE_RESIST | TRAIT_COLD_RESIST | TRAIT_PHYS_RESIST,
    .abilities = ABILITY_ARMOR_DR,
    .skillup = {[SKILL_ARMOR_NATURAL] = 800}
  },
  {PQ_SCALES, .skillup = {[SKILL_ARMOR_NATURAL] = 800}},
  {PQ_THICK_SCALES, TRAIT_PHYS_RESIST | TRAIT_FIRE_RESIST | TRAIT_ACID_RESIST,
    .abilities = ABILITY_ARMOR_DR,
    .skillup = {[SKILL_ARMOR_NATURAL] = 800}
  },
  {PQ_THICK_SKIN, TRAIT_BLUNT_RESIST | TRAIT_COLD_RESIST | TRAIT_FIRE_RESIST | TRAIT_POISON_RESIST,
    .abilities = ABILITY_ARMOR_DR
  },
  {PQ_THICK_FAT, TRAIT_BLUNT_RESIST | TRAIT_COLD_RESIST,
    .abilities = ABILITY_ARMOR_DR
  },
};

static const phys_qualities_t BODY[35] = {
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
    if (MIND[i].mq & mask) {         // if this mental quality is present
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
  PhysWeapon    weap[ASI_DONE];
  PhysBody      covering[ASI_DONE];
  MentalQual    mind[ASI_DONE];
}attribute_quality_t;

typedef struct{
  Senses        type;
  PhysQual      body[SC_DONE];
  MentalQual    mind[SC_DONE];
  int           base[SC_DONE];
}sense_quality_t;
extern sense_quality_t SENSE_QUAL[SEN_DONE];

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
      [ASI_LVL_PRIME]     = PQ_HUGE | PQ_LONG,
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
      [ASI_LVL_EVERY]     = PQ_GIG,
      [ASI_LVL_COMPOSITE] = PQ_HUGE | PQ_STONE_SKIN,
      [ASI_LVL_EVEN]      = PQ_HEAVY | PQ_WIDE,
      [ASI_LVL_PRIME]     = PQ_LARGE,
      [ASI_LVL_SQUARE]    = PQ_DENSE_MUSCLE,
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
      [ASI_LVL_CAP]       = PQ_ETHEREAL
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
      [SC_ABOVE] = PQ_LONG | PQ_TALL | PQ_LARGE | PQ_HUGE | PQ_GIG,
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
      [SC_GREATER]= PQ_LARGE | PQ_HEAVY,
      [SC_SUPER]  = PQ_HUGE,
      [SC_MAX]    = PQ_GIG
    } 
  },
  [STAT_HEALTH] = {
    .stat = STAT_HEALTH,
    .stature = {
      [SC_INFER]   = PQ_TINY,
      [SC_LESSER]  = PQ_SMALL,
      [SC_BELOW]   = PQ_LIGHT,
      [SC_ABOVE]   = PQ_LARGE | PQ_TALL,
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
      [SC_ABOVE]   = PQ_DENSE_MUSCLE,
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
      [SC_INFER] = PQ_SMALL | PQ_SHORT_LIMB,
      [SC_BELOW] = PQ_SHORT,
      [SC_ABOVE] = PQ_ETHEREAL | PQ_LARGE | PQ_GILLED | PQ_DENSE_MUSCLE,
      [SC_SUPER] = PQ_TALL | PQ_HUGE | PQ_FINNED,
      [SC_MAX]   = PQ_GIG
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
  RACE_USE_CLASSES   = BIT64(5),
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

  RACE_MOD_CLASS     = BIT64(24),
  RACE_MOD_ENLARGE   = BIT64(25),
  RACE_MOD_ALPHA     = BIT64(26),
  RACE_MOD_MASK      = 0xFFULL << 24,

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

typedef struct{
  MobMod      id;
  Color       col;
  int         lvl_boost;
  const char  name[MAX_NAME_LEN];
  RaceProps   props;
  uint64_t    body, mind, weaps, covering;
  SkillType   skillups[5];
  EntityType  alt_sprite;
}mob_modification_t;

typedef struct{
  EntityType          base;
  mob_modification_t  mods[MM_DONE];
}mob_variants_t;

typedef struct{
  int         weight;
  const char* name;
}define_social_t;

typedef struct{
  Profession      id;
  define_social_t soc[SOC_DONE];
  uint64_t        rules;
  float           attributes[ATTR_DONE];
  int             skills[SKILL_DONE];
  Archetype       combat_rel[CLASS_BASE_DONE];
}define_prof_t;

static const define_prof_t DEFINE_PROF[PROF_END]= {
  [PROF_NONE] = {
    PROF_NONE,
    {
      [SOC_NONE]      = SOC_W(100),
      [SOC_PRIMITIVE] = SOC_W(5),
      [SOC_CIVIL]     = SOC_W(10),
      [SOC_HIGH]      = SOC_W(12),
    }
  },

  [PROF_SOLDIER] = {
    PROF_SOLDIER,
    {
      [SOC_PRIMITIVE] = SOC_N(24,"Soldier"),
      [SOC_MARTIAL]   = SOC_N(40,"Soldier"),
      [SOC_CIVIL]     = SOC_N(15,"Soldier"),
      [SOC_HIGH]      = SOC_N(14,"Soldier"),
    },
    MOB_LOC_FOREST | MOB_LOC_CAVE | MOB_LOC_DUNGEON,
    {
      [ATTR_CON] = 0.075f,
      [ATTR_STR] = 0.15f,
      [ATTR_DEX] = 0.15f
    },
    {
      [SKILL_SURV]      = 4,
      [SKILL_ATH]       = 5,
      [SKILL_WEAP_MART] = 3,
      [SKILL_WEAP_SIMP] = 8,
    }
  },

  [PROF_ARCHER] = {
    PROF_ARCHER,
    {
      [SOC_PRIMITIVE] = SOC_N(17,"Archer"),
      [SOC_MARTIAL]   = SOC_N(27,"Archer"),
      [SOC_CIVIL]     = SOC_N(17,"Archer"),
      [SOC_HIGH]      = SOC_N(20,"Archer"),
    },
    MOB_LOC_FOREST | MOB_LOC_CAVE | MOB_LOC_DUNGEON,
    {
      [ATTR_CON] = 0.075f,
      [ATTR_STR] = 0.075f,
      [ATTR_DEX] = 0.225f
    },
    {
      [SKILL_PERCEPT]   = 4,
      [SKILL_ACRO]      = 2,
      [SKILL_WEAP_SHOT] = 7,
      [SKILL_SURV]      = 2,
    }
  },

  [PROF_MAGICIAN] = {
    PROF_MAGICIAN,
    {
      [SOC_PRIMITIVE] = SOC_W(0),
      [SOC_MARTIAL]   = SOC_N(17,"Magician"),
      [SOC_CIVIL]     = SOC_N(17,"Magician"),
      [SOC_HIGH]      = SOC_N(20,"Magician"),
    },
    MOB_LOC_FOREST | MOB_LOC_CAVE | MOB_LOC_DUNGEON,
    {
      [ATTR_CHAR] = 0.075f,
      [ATTR_WIS] =  0.15f,
      [ATTR_INT] = 0.15f,
    },
    {
      [SKILL_ARCANA]  = 10,
      [SKILL_INSIGHT] = 4,
      [SKILL_CALL] = 2,
    }
  },

  [PROF_MYSTIC] = {
    PROF_MYSTIC,
    {
      [SOC_PRIMITIVE] = SOC_N(20,"Mystic"),
      [SOC_MARTIAL]   = SOC_N(14,"Mystic"),
      [SOC_CIVIL]     = SOC_N(17,"Mystic"),
      [SOC_HIGH]      = SOC_N(17,"Mystic"),
    },
    MOB_LOC_FOREST | MOB_LOC_CAVE | MOB_LOC_DUNGEON,
    {
      [ATTR_CON] = 0.075f,
      [ATTR_CHAR] = 0.075f,
      [ATTR_WIS] = 0.225f
    },
    {
      [SKILL_NATURE] = 8,
      [SKILL_HERB]   = 3,
      [SKILL_ANIM]   = 6,
      [SKILL_MED]    = 6,
      [SKILL_RELIG]  = 4,
    }
  },

  [PROF_HEALER] = {
    PROF_HEALER,
    {
      [SOC_PRIMITIVE] = SOC_N(9,"Healer"),
      [SOC_MARTIAL]   = SOC_N(12,"Healer"),
      [SOC_CIVIL]     = SOC_N(20,"Healer"),
      [SOC_HIGH]      = SOC_N(21,"Healer"),
    },
    MOB_LOC_FOREST | MOB_LOC_CAVE | MOB_LOC_DUNGEON,
    {
      [ATTR_WIS] = 0.225f,
      [ATTR_INT] = 0.075f,
      [ATTR_CHAR] = 0.075f
    },
    {
      [SKILL_RELIG]     = 10,
      [SKILL_MED]       = 10,
      [SKILL_SPELL_DIV] = 2,
    }
  },

  [PROF_LABORER] = {
    PROF_LABORER,
    {
      [SOC_PRIMITIVE] = SOC_N(10,"Worker"),
      [SOC_MARTIAL]   = SOC_N(10,"Peon"),
      [SOC_CIVIL]     = SOC_W(5),
      [SOC_HIGH]      = SOC_W(4),
    },
    MOB_LOC_FOREST | MOB_LOC_CAVE | MOB_LOC_DUNGEON,
    {
      [ATTR_CON] = 0.125f,
      [ATTR_STR] = 0.125f,
    },
    {
      [SKILL_STONE]      = 4,
      [SKILL_WOOD]       = 4,
      [SKILL_TINK]       = 3,
      [SKILL_WOOD]       = 2,
      [SKILL_STONE]      = 2,
      [SKILL_MASON]      = 3,
      [SKILL_CARP]       = 3,
      [SKILL_WRESTLE]    = 5,
    },
    .combat_rel = CLASS_BASE_FIGHTER
  },

  [PROF_HAULER] = {
    PROF_HAULER,
    {
      [SOC_PRIMITIVE]  = SOC_N(10,"Lugger"),
      [SOC_MARTIAL]    = SOC_N(5,"Packhand"),
      [SOC_CIVIL]      = SOC_W(5),
      [SOC_HIGH]       = SOC_W(2),
    },
    MOB_LOC_FOREST | MOB_LOC_CAVE | MOB_LOC_DUNGEON,
    {
      [ATTR_CON] = 0.10f,
      [ATTR_STR] = 0.15f,
    },
    {
      [SKILL_ATH] = 10,
      [SKILL_WRESTLE] = 5,
    },
    .combat_rel = CLASS_BASE_FIGHTER
  },

  [PROF_RUNNER] = {
    PROF_RUNNER,
    {
      [SOC_INSTINCTIVE] = SOC_N(25,"Creeper"),
      [SOC_HIVE]        = SOC_N(5,"Creeper"),
      [SOC_PRIMITIVE]   = SOC_N(10,"Runner"),
      [SOC_FERAL]       = SOC_N(10,"Prowler"),
      [SOC_FAMILY]      = SOC_N(10,"Scout"),
      [SOC_MARTIAL]     = SOC_N(5,"Envoy"),
      [SOC_CIVIL]       = SOC_W(5),
      [SOC_HIGH]        = SOC_W(2),
    },
    MOB_LOC_FOREST | MOB_LOC_CAVE | MOB_LOC_DUNGEON,
    {
      [ATTR_DEX] = 0.15f,
    },
    {
      [SKILL_SURV]    = 8,
      [SKILL_STEALTH] = 2,
      [SKILL_ACRO]    = 4,
    },
    .combat_rel = {CLASS_BASE_ROGUE, CLASS_BASE_RANGER}
  },

  [PROF_GATHERER] = {
    PROF_GATHERER,
    {
      [SOC_INSTINCTIVE] = SOC_N(50,"Scavenger"),
      [SOC_FAMILY]      = SOC_N(20,"Scavenger"),
      [SOC_HIVE]        = SOC_N(20,"Scavenger"),
      [SOC_PRIMITIVE]   = SOC_N(10,"Scavenger"),
      [SOC_MARTIAL]     = SOC_N(5,"Rummager"),
      [SOC_CIVIL]       = SOC_N(5,"Gatherer"),
      [SOC_HIGH]        = SOC_N(2,"Forager"),
    },
    MOB_LOC_FOREST,
    {},
    {
      [SKILL_NATURE] = 7,
      [SKILL_SURV]   = 2,
      [SKILL_HERB]   = 10,
    },
    .combat_rel = {CLASS_BASE_DRUID, CLASS_BASE_RANGER},
  },
[PROF_MINER] = {
    PROF_MINER,
    {
        [SOC_PRIMITIVE]  = SOC_N(6,  "Digger"),
        [SOC_MARTIAL]    = SOC_N(12, "Miner"),
        [SOC_CIVIL]      = SOC_W(8),
        [SOC_HIGH]       = SOC_W(4),
    },
    MOB_LOC_CAVE,
    {
      [ATTR_STR] = 0.2f,
      [ATTR_CON] = 0.2f,
    },
    {
      [SKILL_STONE]     = 10,
      [SKILL_WEAP_PICK] = 3,
    },
    .combat_rel = CLASS_BASE_FIGHTER,
},
  [PROF_CHOPPER] = {
    PROF_CHOPPER,
    {
      [SOC_PRIMITIVE]  = SOC_N(12, "Cutter"),
      [SOC_MARTIAL]    = SOC_N(12, "Logger"),
      [SOC_CIVIL]      = SOC_N(8,  "Lumberjack"),
      [SOC_HIGH]       = SOC_W(4),
    },
    MOB_LOC_FOREST,
    {
      [ATTR_STR] = 0.2f,
    },
    {
      [SKILL_WOOD]     = 10,
      [SKILL_WEAP_AXE] = 3,
    },
    .combat_rel = CLASS_BASE_FIGHTER
  },
  [PROF_TENDER] = {
    PROF_TENDER,
    {
      [SOC_PRIMITIVE]  = SOC_N(10, "Penkeeper"),
      [SOC_MARTIAL]    = SOC_N(12, "Stockkeeper"),
      [SOC_CIVIL]      = SOC_N(10, "Caretaker"),
      [SOC_HIGH]       = SOC_N(8,  "Animal Tender"),
    },
    .skills = {
      [SKILL_ANIM] = 10,
      [SKILL_NATURE] = 5,
      [SKILL_MED] = 5,
    },
    .combat_rel = {CLASS_BASE_DRUID, CLASS_BASE_RANGER}

  },
  [PROF_SCHOLAR] = {
    PROF_SCHOLAR,
    {
      [SOC_MARTIAL] = SOC_W(2),
      [SOC_CIVIL]   = SOC_W(12),
      [SOC_HIGH]    = SOC_W(15),
    },
    MOB_LOC_MASK,
    {
      [ATTR_INT] = 0.25f,
      [ATTR_WIS] = 0.125f,
    },
    .skills = {
      [SKILL_ARCANA] = 4,
      [SKILL_CALL] = 8,
      [SKILL_HIST] = 15,
      [SKILL_INSIGHT] = 8,
    },
    .combat_rel = CLASS_BASE_WIZ
  },
  [PROF_HUNTER] = {
    PROF_HUNTER,
    {
      [SOC_FERAL]      = SOC_N(15, "Stalker"),
      [SOC_PRIMITIVE]  = SOC_N(12, "Scamp"),
      [SOC_MARTIAL]    = SOC_N(12, "Stalker"),
      [SOC_CIVIL]      = SOC_N(8, "Huntsman"),
      [SOC_HIGH]       = SOC_N(4, "Huntsman"),
    },
    MOB_LOC_FOREST,
    {
      [ATTR_DEX] = 0.125f
    },
    {
      [SKILL_SURV] = 5,
      [SKILL_NATURE] = 5,
      [SKILL_ANIM] = 10,
      [SKILL_WEAP_BOW] = 4,
      [SKILL_WEAP_DAGGER] = 2
    },
    .combat_rel = CLASS_BASE_RANGER
  },
  [PROF_FISHER] = {
    PROF_FISHER,
    {
      [SOC_FAMILY]     = SOC_W(20),
      [SOC_PRIMITIVE]  = SOC_W(8),
      [SOC_MARTIAL]    = SOC_W(12),
      [SOC_CIVIL]      = SOC_W(10),
      [SOC_HIGH]       = SOC_W(6),
    }
  },
  [PROF_BUILDER] = {
    PROF_BUILDER,
    {
      [SOC_MARTIAL]    = SOC_N(10, "Builder"),
      [SOC_CIVIL]      = SOC_N(10, "Constructor"),
      [SOC_HIGH]       = SOC_N(5, "Constructor"),
    },
    MOB_LOC_MASK,
    {
      [ATTR_CON] = 0.125f,
      [ATTR_STR] = 0.075f,
    },
    {
      [SKILL_CARP] = 5,
      [SKILL_MASON] = 5,
      [SKILL_STONE] = 3,
      [SKILL_WOOD] = 3,
      [SKILL_TINK] = 7,
      [SKILL_WEAP_MACE] = 4,
    },
    .combat_rel = CLASS_BASE_FIGHTER
  },
  [PROF_COOK] = {
    PROF_COOK,
    {
      [SOC_MARTIAL]   = SOC_N(10, "Cook"),
      [SOC_CIVIL]     = SOC_N(12, "Cook"),
      [SOC_HIGH]      = SOC_N(10, "Cook"),
    }
  },
  [PROF_MEDIC] = {
    PROF_MEDIC,
    {
      [SOC_PRIMITIVE] = SOC_N(2, "Croaker"),
      [SOC_MARTIAL]   = SOC_N(8, "Medic"),
      [SOC_CIVIL]     = SOC_N(12, "Medic"),
      [SOC_HIGH]      = SOC_N(12, "Physician"),
    },
    MOB_LOC_MASK,
    {
      [ATTR_WIS] = 0.125f,
    },
    {
      [SKILL_MED] = 10,
      [SKILL_HERB] = 6,
    },
    .combat_rel = CLASS_BASE_CLERIC
  },
  [PROF_GUARD] = {
    PROF_GUARD,
    {
      [SOC_FAMILY]     = SOC_N(15, "Protector"),
      [SOC_FERAL]      = SOC_N(15, "Protector"),
      [SOC_INSTINCTIVE]= SOC_N(15, "Protector"),
      [SOC_PRIMITIVE]  = SOC_N(0, "Watcher"),
      [SOC_MARTIAL]    = SOC_N(0, "Legionary"),
      [SOC_CIVIL]      = SOC_N(0, "Guard"),
      [SOC_HIGH]       = SOC_W(0),
    },
    MOB_LOC_MASK,
    {
      [ATTR_CON] = 0.125f,
      [ATTR_STR] = 0.125f,
      [ATTR_DEX] = 0.05f
    },
    {
      [SKILL_SURV]      = 8,
      [SKILL_ATH]       = 6,
      [SKILL_WEAP_NONE] = 5,
    },
    .combat_rel = CLASS_BASE_FIGHTER
  },
  /*
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
  */
};

typedef struct {
  SpeciesType   race;
  const char    name[MAX_NAME_LEN];
  EntityType    base_ent;
  RaceProps     props;
  Traits        traits;
  uint64_t      body;
  uint64_t      mind;
  uint64_t      weaps;
  uint64_t      covering;
  uint64_t      feats;
  float         base_challenge;
}race_define_t;

static inline int SpecToIndex(SpeciesType spec){
  return __builtin_ctzll(spec);
}

typedef struct{
  Archetype     archtype;
  int           hitdie;
  AttributeType spell,prof,save;
  float         ASI[ATTR_DONE];
  int           pref_ability[AT_DONE];
  int           pref_act[ACTION_SLOTTED];
  Traits        traits;
}define_archetype_t;

typedef struct{
  AbilityID     id;
  bool          tome,scroll;
  Archetype     base,has;
  int           lvl,ranks,priority;
  SpeciesType   racial;
}define_ability_class_t;

typedef struct{
  int         weight;
  Archetype   base;
  const char  name[MAX_NAME_LEN];
  int         skills[SKILL_DONE];
  int         beefups[SKILL_DONE];
}race_class_t;

typedef struct{
  EntityType    race;
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

typedef struct{
  int           ranks;
  Archetype     ladder[MAX_RANKS];
}define_rankup_t;

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

typedef enum{
  PROP_CONT_NONE  = 0,
  PROP_CONT_PHIAL = BIT64(0),
}ContainerProp;

typedef uint64_t ItemProps;
typedef uint64_t ArmorProps;
typedef uint64_t ConsumeProps;
typedef uint64_t ContainerProps;
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
  StorageMethod      primary;
  int                prio[STORE_DONE];
  uint16_t           size;
}armor_def_t;

typedef struct{
  ItemSlot        slot;
  int             weight, cost,slots;
  ItemProps       i_props;
  ContainerProps  c_props;
  uint16_t        size,slot_size;
}container_def_t;

typedef struct{
  WeaponType      type;
  char            name[MAX_NAME_LEN];
  int             cost,weight,penn,drain,reach_bonus,durability;
  ItemProps       i_props;
  WeaponProps     w_props;
  AbilityID       ability;
  SkillType       skill;
  StorageMethod   primary;
  int             prio[STORE_DONE];
  uint16_t           size;
}weapon_def_t;

WeaponType GetWeapTypeBySkill(SkillType s);
ArmorType GetArmorTypeBySkill(SkillType s);

typedef struct{
  ConsumeType     type;
  int             cost,weight,quanity,amount, exp;
  ItemProps       i_props;
  ConsumeProps    w_props;
  AbilityID       ability;
  SkillType       skill;
  StorageMethod   primary;
  int             prio[STORE_DONE];
  uint16_t        size;
}consume_def_t;

extern armor_def_t ARMOR_TEMPLATES[ARMOR_DONE];
extern weapon_def_t WEAPON_TEMPLATES[WEAP_DONE];
extern consume_def_t CONSUME_TEMPLATES[CONS_DONE];
extern container_def_t CONTAINER_TEMPLATES[INV_DONE];


typedef struct value_affix_s value_affix_t;
typedef float (*AffixFn)(value_affix_t* self, int val);
float AffixAdd(value_affix_t* self, int val);
float AffixSub(value_affix_t* self, int val);
float AffixMul(value_affix_t* self, int val);
float AffixFract(value_affix_t* self, int val);
float AffixBase(value_affix_t* self, int val);

typedef struct {
  ValueCategory   val;
  StatType        stat_rel, stat_aff;
  AttributeType   att_rel, att_aff;
}value_relate_t;

static value_relate_t VALUE_RELATES[VAL_ALL]={
  [VAL_REACH] = {VAL_REACH, STAT_REACH},
  [VAL_SAVE] = {VAL_SAVE, STAT_ARMOR},
};
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
  StatType    resource;
}define_slot_actions;

typedef struct{
  ItemSlot      id;
  int           limits[ITEM_DONE];
  int           cap;
  uint16_t      base_size;
}define_inventory_t;

typedef struct{
  uint64_t  size;
  int       str_mul;
}define_burden_t;

typedef struct{
  ActionType    action;
  ModifierType  modified_by[ATTR_DONE];

}define_initiative_relate_t;
#endif
