#ifndef __GAME_LIST__
#define __GAME_LIST__
#include "game_define.h"

static faction_t FACTION_DEFS[3] = {
  {"Dark Legion",
    SPEC_GOBLINOID | SPEC_ORC | SPEC_SULKING,
    {[BIO_FOREST] = 10, [BIO_CAVE] = 40, [BIO_DUNGEON] = 50},
    {[ENT_GOBLIN] = 35, [ENT_ORC] = 24, [ENT_HOBGOBLIN] = 16,
      [ENT_OGRE] = 4, [ENT_BUGBEAR] = 15
    }
  },
  {"Forest Tribe Oak",
    SPEC_SULKING,
    {[BIO_FOREST] = 100},
    {[ENT_BUGBEAR] = 25,[ENT_TROLL] = 5}
  },
  {"Solensted",
    SPEC_HUMAN,
    {[BIO_FOREST] = 10},
    {[ENT_KNIGHT] = 10, [ENT_FOOTMAN]=25}
  }
};

static const char* DAMAGE_STRING[DMG_DONE]={
  "Blunt",
  "Piercing",
  "Slashing",
  "Bleed",
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

static stat_name_t STAT_STRING[STAT_ENT_DONE]={
  {STAT_NONE,"N/A"},
  {STAT_REACH,"Reach"},
  {STAT_DAMAGE,"Damage"},
  {STAT_HEALTH,"Health"},
  {STAT_ARMOR, "Armor"},
  {STAT_ACTIONS, "Actions"},
  {STAT_ENERGY,"Spell Energy"},
  {STAT_STAMINA, "Stamina"},
  {STAT_STAMINA_REGEN,"Stamina Regen"},
  {STAT_ENERGY_REGEN, "Spell Regen"},
  {STAT_STAMINA_REGEN_RATE,"Stamina Regen Rate"},
  {STAT_ENERGY_REGEN_RATE, "Spell Regen Rate"},
};

static stat_relate_t STAT_RELATION[STAT_ENT_DONE] = {
  [STAT_HEALTH] = {STAT_HEALTH, STAT_HEALTH_REGEN_RATE},
  [STAT_STAMINA] = {STAT_STAMINA, STAT_STAMINA_REGEN_RATE},
  [STAT_ENERGY] = {STAT_ENERGY, STAT_ENERGY_REGEN_RATE},
  [STAT_STAMINA_REGEN] = {STAT_STAMINA_REGEN, STAT_STAMINA},
  [STAT_ENERGY_REGEN] = {STAT_ENERGY_REGEN, STAT_ENERGY},
  [STAT_HEALTH_REGEN] = {STAT_HEALTH_REGEN, STAT_HEALTH},
  [STAT_STAMINA_REGEN_RATE] = {STAT_STAMINA_REGEN_RATE, STAT_STAMINA_REGEN},
  [STAT_ENERGY_REGEN_RATE] = {STAT_ENERGY_REGEN_RATE, STAT_ENERGY_REGEN},
  [STAT_HEALTH_REGEN_RATE] = {STAT_HEALTH_REGEN_RATE, STAT_HEALTH_REGEN},
};

static const int  STAT_STANDARDS[STAT_ENT_DONE][SC_DONE] = {
  [STAT_REACH] = {[SC_AVERAGE] = 1, [SC_ABOVE] = 2},
  [STAT_DAMAGE] = {
    [SC_MIN]    = 0,
    [SC_INFER]  = 0,
    [SC_LESSER] = 1,
    [SC_BELOW]  = 1,
    [SC_AVERAGE]= 2,
    [SC_ABOVE]  = 3,
    [SC_SUPER]  = 5,
    [SC_MAX]    = 8
  },
  [STAT_HEALTH] = {
    [SC_MIN]     = 1,
    [SC_INFER]   = 2,
    [SC_LESSER]  = 3,
    [SC_BELOW]   = 5,
    [SC_AVERAGE] = 8,
    [SC_ABOVE]   = 13,
    [SC_GREATER] = 21,
    [SC_SUPER]   = 34,
    [SC_MAX]     = 55
  },
  [STAT_ARMOR] = {
    [SC_MIN]     = 0,
    [SC_INFER]   = 0,
    [SC_LESSER]  = 1,
    [SC_BELOW]   = 1,
    [SC_AVERAGE] = 2,
    [SC_ABOVE]   = 2,
    [SC_GREATER] = 4,
    [SC_SUPER]   = 4,
    [SC_MAX]     = 6,
  },
  [STAT_ACTIONS] = {
    [SC_MIN]     = 1,
    [SC_AVERAGE] = 1,
    [SC_ABOVE]   = 2,
    [SC_SUPER]   = 3,
    [SC_MAX]     = 4
  },
  [STAT_ENERGY] = {
    [SC_MIN]     = 1,
    [SC_INFER]   = 2,
    [SC_BELOW]   = 3,
    [SC_AVERAGE] = 5,
    [SC_ABOVE]   = 8,
    [SC_SUPER]   = 13,
    [SC_MAX]     = 21
  },
  [STAT_STAMINA] = {
    [SC_MIN]     = 2,
    [SC_INFER]   = 3,
    [SC_BELOW]   = 5,
    [SC_AVERAGE] = 7,
    [SC_ABOVE]   = 11,
    [SC_SUPER]   = 13,
    [SC_MAX]     = 17
  },
  [STAT_STAMINA_REGEN] = {
    [SC_MIN]     = 1,
    [SC_INFER]   = 2,
    [SC_BELOW]   = 3,
    [SC_AVERAGE] = 4,
    [SC_ABOVE]   = 5,
    [SC_SUPER]   = 6,
    [SC_MAX]     = 7
  },
  [STAT_ENERGY_REGEN] = {
    [SC_MIN]     = 0,
    [SC_INFER]   = 1,
    [SC_BELOW]   = 2,
    [SC_AVERAGE] = 3,
    [SC_ABOVE]   = 4,
    [SC_SUPER]   = 5,
    [SC_MAX]     = 6
  },
  [STAT_STAMINA_REGEN_RATE] = {
    [SC_MAX]    = 4,
    [SC_SUPER]  = 6,
    [SC_ABOVE]  = 8,
    [SC_AVERAGE]= 10,
    [SC_BELOW]  = 12,
    [SC_INFER]  = 14,
    [SC_MIN]    = 16
  },
  [STAT_ENERGY_REGEN_RATE] = {
    [SC_MAX]      = 5,
    [SC_SUPER]    = 7,
    [SC_ABOVE]    = 9,
    [SC_AVERAGE]  = 11,
    [SC_BELOW]    = 13,
    [SC_INFER]    = 15,
    [SC_MIN]      = 17
  },
  [STAT_HEALTH_REGEN] = {
    [SC_MIN]      = 0,
    [SC_INFER]    = 0,
    [SC_BELOW]    = 0,
    [SC_AVERAGE]  = 1,
    [SC_ABOVE]    = 2,
    [SC_SUPER]    = 3,
    [SC_MAX]      = 4,
  },
  [STAT_HEALTH_REGEN_RATE] = {
    [SC_MAX]      = 5,
    [SC_SUPER]    = 7,
    [SC_ABOVE]    = 9,
    [SC_AVERAGE]  = 11,
    [SC_BELOW]    = 13,
    [SC_INFER]    = 15,
    [SC_MIN]      = 17
  },
};

static skill_name_t SKILL_STRING[SKILL_DONE] = {
  {SKILL_LVL, "Level"},
};

static define_skill_rate_t  SKILL_RATES[RATE_DONE]={
  {RATE_NONE},
  {RATE_LINEAR,
    {[IR_FAIL] = 100, [IR_SUCCESS] = 100},
    {[IR_FAIL] = 75,[IR_SUCCESS] = 75},
    20
  },
  {RATE_REWARD,
    {[IR_FAIL] = 75, [IR_SUCCESS] = 100},
    {[IR_FAIL] = 75, [IR_SUCCESS] = 90},
    30
  },
  {RATE_REWARD_SLOW,
    {[IR_FAIL] = 25, [IR_SUCCESS] = 75},
    {[IR_FAIL] = 25, [IR_SUCCESS] = 50},
    30
  },
  {RATE_RISK, 
    {
      [IR_FAIL] = 100,
      [IR_CRITICAL_FAIL] = 110,
      [IR_SUCCESS] = 75,
      [IR_TOTAL_SUCC] = 50
    },
    {[IR_FAIL] = 90, [IR_SUCCESS]=50},
    40
  }
};

static skill_rate_relation_t SKILLRATE_LOOKUP[RATE_DONE]={
  {RATE_NONE},
  {RATE_LINEAR},
  {RATE_REWARD,
    {
      [SKILL_WEAP_MART]=true,
    }
  },
  {RATE_REWARD_SLOW,
    {
      [SKILL_ARMOR_NATURAL] = true,
      [SKILL_ARMOR_PADDED] = true,
      [SKILL_ARMOR_LEATHER] = true,
      [SKILL_ARMOR_CHAIN] = true,
      [SKILL_ARMOR_PLATE] = true,
    }
  },
  {RATE_ALL_OR_NOTHING,
    {
      [SKILL_WEAP_SWORD] = true,
      [SKILL_WEAP_BOW]   = true,
    }
  },
  {RATE_RISK,
    {
      [SKILL_ARCANA] = true,
      [SKILL_WEAP_SIMP] = true,
    }
  },
};

static attribute_name_t attributes[ATTR_DONE]={
  {ATTR_NONE, "N/A"},
  {ATTR_CON,"CONSTITUTION"},
  {ATTR_STR,"STRENGTH"},
  {ATTR_DEX,"DEXTERITY"},
  {ATTR_INT,"INTELLIGENCE"},
  {ATTR_WIS,"WISDOM"},
  {ATTR_CHAR,"CHARISMA"},
  {ATTR_BLANK,"REROLL"}
};

static const race_define_t DEFINE_RACE[ 16 ] = {
  {SPEC_NONE},
  {SPEC_HUMAN, "Joseph", ENT_PERSON,
    RACE_USE_TOOLS | RACE_USE_WEAPS | RACE_USE_ARMOR | RACE_USE_POTIONS |RACE_USE_SCROLLS |
    RACE_ARMOR_SIMPLE | RACE_ARMS_SIMPLE,
    TRAIT_EXP_SWORD,
    PQ_BIPED,
    MQ_SENTIENT | MQ_CAUTIOUS | MQ_DETERMINED | MQ_PATIENT | MQ_STRATEGIC | MQ_LEADER | MQ_PROTECTIVE | MQ_DISCIPLINED,
    PW_NONE, PB_NONE,
    0,
    1
  },
  {SPEC_ELF},
  {SPEC_ARCHAIN},
  {SPEC_GOBLINOID, "Goblin", ENT_NONE,
    RACE_USE_TOOLS | RACE_USE_WEAPS | RACE_USE_ARMOR | RACE_USE_POTIONS |RACE_USE_SCROLLS |
      RACE_ARMOR_CRUDE | RACE_ARMOR_LIGHT| RACE_ARMS_CRUDE | RACE_ARMS_LIGHT |
      RACE_MOD_CLASS | RACE_TACTICS_CRUDE |
      RACE_DIFF_LVL | RACE_DIFF_SKILL | RACE_DIFF_SPELLS | RACE_DIFF_PETS | RACE_DIFF_ALPHA |
      RACE_BUILD_CRUDE | 
      RACE_SPECIAL_TRAPS | RACE_SPECIAL_FOCI | RACE_SPECIAL_WARDS,
    TRAIT_POISON_RESIST,
    PQ_BIPED | PQ_LARGE_EARS,
    MQ_TERRITORIAL | MQ_SENTIENT | MQ_ANXIOUS | MQ_CUNNING | MQ_CAUTIOUS | MQ_OBEDIENT,
    PW_NONE,
    PQ_THICK_SKIN,
    0,
    .75
  },
  {SPEC_ORC, "Orc", ENT_ORC,
    RACE_USE_TOOLS | RACE_USE_WEAPS | RACE_USE_ARMOR | RACE_USE_POTIONS |RACE_USE_SCROLLS |
      RACE_ARMOR_CRUDE | RACE_ARMOR_SIMPLE | RACE_ARMOR_LIGHT | RACE_ARMOR_MEDIUM | RACE_ARMOR_HEAVY | RACE_ARMOR_FORGED |
      RACE_ARMS_SIMPLE | RACE_ARMS_LIGHT | RACE_ARMS_HEAVY | RACE_ARMS_FORGED | RACE_ARMS_SKILLED |
      RACE_MOD_CLASS |
      RACE_TACTICS_MARTIAL | RACE_TACTICS_SIMPLE |
      RACE_DIFF_LVL | RACE_DIFF_SKILL | RACE_DIFF_GEAR | RACE_DIFF_SPELLS | RACE_DIFF_ALPHA |
      RACE_SPECIAL_FOCI,
      TRAIT_EXP_AXE | TRAIT_VISION_DARK,
      PQ_LONG_LIMB | PQ_SMALL_HEAD | PQ_BIPED | PQ_LARGE_HANDS | PQ_DENSE_MUSCLE,
      MQ_OBLIVIOUS | MQ_SENTIENT | MQ_AGGRESSIVE | MQ_TERRITORIAL,
      PW_NONE,
      PQ_THICK_SKIN,
      0,
      1.25
  },
  {SPEC_GIANT, "Giant", ENT_NONE,
    RACE_MOD_ALPHA | RACE_TACTICS_CRUDE |
    RACE_DIFF_LVL | RACE_DIFF_ALPHA |
    RACE_USE_TOOLS | RACE_USE_WEAPS |
    RACE_ARMS_SIMPLE | RACE_ARMS_HEAVY,
    TRAIT_VISION_DARK,
    PQ_TALL | PQ_HUGE | PQ_LONG_LIMB | PQ_LARGE_HANDS | PQ_DENSE_MUSCLE,
    MQ_AGGRESSIVE | MQ_TERRITORIAL,
    PQ_CLAWS | PQ_TEETH,
    PQ_FUR | PQ_HIDE | PQ_THICK_FUR,
    0,
    4
  },
  {SPEC_ARTHROPOD, "Crawly", ENT_NONE,
    RACE_MOD_ALPHA | RACE_MOD_ENLARGE | RACE_TACTICS_CRUDE | RACE_DIFF_LVL | RACE_DIFF_ALPHA,
    TRAIT_POISON_RESIST,
    MQ_SIMPLE | MQ_CAUTIOUS | MQ_ALERT | MQ_HIVE_MIND,
    PB_NONE,
    0,
    .25
  },
  {SPEC_ETHEREAL},
  {SPEC_ROTTING},
  {SPEC_SKELETAL},
  {SPEC_VAMPIRIC},
  {SPEC_CANIFORM, "Predator", ENT_NONE,
    RACE_MOD_ENLARGE | RACE_MOD_ALPHA | RACE_TACTICS_CRUDE | RACE_DIFF_LVL | RACE_DIFF_ALPHA,
    0,
    PQ_LARGE_NOSE | PQ_LARGE_EARS | PQ_BIPED, MQ_AGGRESSIVE | MQ_TERRITORIAL,
    PQ_CLAWS | PQ_TEETH,
    PQ_FUR,
    0,
    .5
  },
  {SPEC_RODENT, "Rat", ENT_RAT,
  RACE_MOD_ENLARGE | RACE_TACTICS_CRUDE | RACE_DIFF_LVL | RACE_DIFF_ALPHA,
  0,
  PQ_LARGE_EARS | PQ_SENSITIVE_NOSE | PQ_TINY | PQ_LIGHT | PQ_QUADPED | PQ_TAIL | PQ_TINY_HEAD,
  MQ_SIMPLE | MQ_CAUTIOUS | MQ_ALERT | MQ_HIVE_MIND,
  PQ_TOUGH_TEETH | PQ_TEETH,
  PQ_FUR,
  0,
  .25
  },
  { SPEC_RUMINANT, "Grazer", ENT_NONE,
    RACE_MOD_ENLARGE | RACE_MOD_ALPHA | RACE_TACTICS_CRUDE | RACE_DIFF_ALPHA,
    0,
    PQ_SMALL | PQ_QUADPED | PQ_LARGE_EARS,
    MQ_SIMPLE | MQ_CAUTIOUS | MQ_ALERT | MQ_PROTECTIVE,
    PQ_HORNED,
    PQ_FUR | PQ_HIDE,
    0,
    0.375
  },
  {SPEC_SULKING, "Kobold", ENT_NONE,
    RACE_MOD_CLASS | RACE_TACTICS_CRUDE | RACE_DIFF_LVL | RACE_DIFF_ALPHA,
    0,  
    PQ_SENSITIVE_NOSE | PQ_LARGE_EARS | PQ_BIPED | PQ_TAIL,
    MQ_SIMPLE | MQ_CAUTIOUS | MQ_ALERT | MQ_TERRITORIAL,
    PQ_TOUGH_TEETH | PQ_TEETH,
    PQ_FUR,
    0,
    .375
  },

};
static race_define_t GetRaceByFlag(SpeciesType f){
  int index = SpecToIndex(f);

  return DEFINE_RACE[index];
}

static Traits SKILL_TRAITS[SKILL_DONE] = {
//  [SKILL_WEAP_SIMP],
//  [SKILL_WEAP_MART],
  [SKILL_WEAP_MACE]   = TRAIT_EXP_MACE,
  [SKILL_WEAP_SWORD]  = TRAIT_EXP_SWORD, 
  [SKILL_WEAP_AXE]    = TRAIT_EXP_AXE,
  [SKILL_WEAP_DAGGER] = TRAIT_EXP_DAGGER,
  [SKILL_WEAP_BOW]    = TRAIT_EXP_BOW,
//  [SKILL_WEAP_NONE],

};

static const define_archetype_t CLASS_DATA[CLASS_DONE] = {
  {CLASS_BASE_BARD},
  {CLASS_BASE_CLERIC, 6, ATTR_WIS,ATTR_INT,ATTR_CHAR,
    {[ATTR_WIS]=1,[ATTR_INT]=0.5f,[ATTR_INT]=0.5f, [ATTR_CHAR] =.75f},
    {[AT_HEAL]=5, [AT_DMG]=1},
    {[ACTION_MAGIC]=5,[ACTION_CANTRIP]=4, [ACTION_ATTACK]=3, [ACTION_WEAPON] = 1},
    TRAIT_PREF_COVER | TRAIT_PREF_HEAL
  },
  {CLASS_BASE_DRUID, 6, ATTR_WIS,ATTR_WIS,ATTR_INT,
    {[ATTR_CON]=0.25f, [ATTR_WIS]=0.25f, [ATTR_CHAR]=0.5f, [ATTR_INT]=0.25f},
    {[AT_HEAL]=3, [AT_DMG]=2},
    {[ACTION_MAGIC]=3,[ACTION_CANTRIP]=3, [ACTION_ATTACK]=3, [ACTION_WEAPON] = 1},
    TRAIT_PREF_COVER
  },
  {CLASS_BASE_FIGHTER, 8, ATTR_NONE,ATTR_STR,ATTR_CON,
    {[ATTR_STR]=1,[ATTR_DEX]=1,[ATTR_CON]=.25f},
    {[AT_DMG] = 5},
    {[ACTION_ATTACK] = 5, [ACTION_WEAPON] = 4},
    TRAIT_PREF_MELEE | TRAIT_CAN_DEFEND
  },
  {CLASS_BASE_MONK},
  {CLASS_BASE_RANGER, 8, ATTR_WIS, ATTR_DEX, ATTR_STR,
      {[ATTR_STR]=0.5f,[ATTR_DEX]=0.5f,[ATTR_CON]=.125f,[ATTR_WIS]=0.125f},
      {[AT_DMG] = 3},
      {[ACTION_ATTACK] = 4, [ACTION_WEAPON] = 4, [ACTION_ITEM] = 2, [ACTION_CANTRIP]=2, [ACTION_MOVE]=3},
      TRAIT_PREF_COVER | TRAIT_PREF_SHOOT
  },
  {CLASS_BASE_ROGUE, 6, ATTR_NONE, ATTR_DEX, ATTR_INT,
    {[ATTR_DEX]=0.5f, [ATTR_STR]=0.125,[ATTR_INT]=0.25},
      {[AT_DMG] = 4},
      {[ACTION_ATTACK] = 4, [ACTION_WEAPON] = 4, [ACTION_ITEM] = 3,[ACTION_MOVE]=3},
      TRAIT_PREF_SNEAK | TRAIT_CAN_STEALTH
  },
  {CLASS_BASE_LOCK, 6, ATTR_WIS, ATTR_CHAR, ATTR_CHAR,
    {[ATTR_CON]=0.25f, [ATTR_WIS]=0.25f, [ATTR_CHAR]=0.75f},
    {[AT_DMG]=3},
    {[ACTION_MAGIC]=4,[ACTION_CANTRIP]=3, [ACTION_ATTACK]=2, [ACTION_WEAPON] = 1},
    TRAIT_PREF_COVER | TRAIT_PREF_CAST
  },
  {CLASS_BASE_WIZ, 4, ATTR_INT, ATTR_WIS, ATTR_INT,
    {[ATTR_WIS]=0.5f, [ATTR_CHAR]=0.25f, [ATTR_INT]=0.75f},
    {[AT_DMG]=3},
    {[ACTION_MAGIC]=4,[ACTION_CANTRIP]=3, [ACTION_ATTACK]=2, [ACTION_WEAPON] = 1},
    TRAIT_PREF_COVER | TRAIT_PREF_CAST
  },
  
  {CLASS_BASE_BERZ, 10, ATTR_NONE, ATTR_STR, ATTR_CON},
  {CLASS_SUB_CHAMP, 10, ATTR_NONE, ATTR_STR, ATTR_CON},
};

static define_race_class_t RACE_CLASS_DEFINE[ENT_DONE][PROF_LABORER] = {
  [ENT_PERSON] = {ENT_PERSON, PROF_NONE, 2,
    {
      { 25, CLASS_BASE_FIGHTER, "Warrior", 
        .skills = {
          [SKILL_ARMOR_PADDED]=2, [SKILL_WEAP_SIMP]=2,
        },
        .beefups = {
          [SKILL_ARMOR_PADDED]=1, [SKILL_WEAP_SIMP]=1,
        }
      },
      { 15, CLASS_BASE_BARD, "Upholder",
        .skills = {
          [SKILL_ARMOR_CLOTH] = 2, [SKILL_ARCANA] = 4,
          [SKILL_PERFORM] = 5, 
        },
        .beefups = {
          [SKILL_ARMOR_CLOTH] = 1, [SKILL_ARCANA] = 2
        }
      }
    }
  },
  [ENT_GOBLIN] = {
    [PROF_SOLDIER] = {ENT_GOBLIN, PROF_SOLDIER,2,
      {
        { 15, CLASS_BASE_FIGHTER, "Brawler",
          .skills = {
            [SKILL_ARMOR_LEATHER]=2,[SKILL_ARMOR_SHIELD]=1,
          [SKILL_WEAP_MACE]=2, [SKILL_WEAP_SWORD]=1
          },
          .beefups = {
            [SKILL_ARMOR_LEATHER]=3,[SKILL_ARMOR_SHIELD]=3,
          [SKILL_WEAP_MART]=3, [SKILL_WEAP_MACE]=3,
          }
        },
        { 10, CLASS_BASE_ROGUE, "Skulker",// "Infiltrator", "Saboteur",
           .skills = {
            [SKILL_ARMOR_LEATHER]=1,[SKILL_STEALTH]=2,
          [SKILL_WEAP_DAGGER]=3, [SKILL_WEAP_BOW]=1
          },
          .beefups = {
            [SKILL_ARMOR_LEATHER]=3,[SKILL_STEALTH]=3,
          [SKILL_WEAP_MART]=3, [SKILL_WEAP_DAGGER]=3,
          }
        }
      }
    },
    [PROF_ARCHER] = {ENT_GOBLIN, PROF_ARCHER,1,
      {
        {
          10, CLASS_BASE_RANGER, "Tracker",// "Stinger",
          .skills = {
            [SKILL_ARMOR_PADDED]=2,[SKILL_ANIM]=2,
            [SKILL_WEAP_BOW]=4, [SKILL_WEAP_DAGGER]=1
          },
          .beefups = {
            [SKILL_ARMOR_PADDED]=2,[SKILL_STEALTH]=2,
            [SKILL_WEAP_BOW]=4, [SKILL_WEAP_DAGGER]=3,
          }
        },
      }
    },
    [PROF_MYSTIC] = { ENT_GOBLIN, PROF_MYSTIC,1,
      {
        {
          5, CLASS_BASE_SHAMAN, "Pestcaller",//"Seer", "Mystic",
          .skills = {
            [SKILL_ARMOR_CLOTH]=1, [SKILL_WEAP_MACE]=1,
            [SKILL_SPELL_EVO] = 1,[SKILL_SPELL_ABJ] = 1, [SKILL_SPELL_TRANS]=3, [SKILL_SPELL_CONJ] = 2,
          },
            .beefups = {
              [SKILL_ARMOR_CLOTH]=1, [SKILL_WEAP_MACE]=1,
              [SKILL_SPELL_ABJ] = 2,[SKILL_SPELL_EVO] = 2, [SKILL_SPELL_TRANS]=2, [SKILL_SPELL_CONJ] = 2,
            }
        },
      }
    },
    [PROF_MAGICIAN] = { ENT_GOBLIN, PROF_MAGICIAN,1,
      {
        {
          5, CLASS_BASE_LOCK,"Filthcaller",
          .skills = {
            [SKILL_ARMOR_CLOTH] = 1,[SKILL_SPELL_EVO] = 2, [SKILL_SPELL_ABJ] = 2, [SKILL_SPELL_TRANS] = 1, [SKILL_SPELL_NECRO] = 3,
          },
          .beefups = {
             [SKILL_ARMOR_CLOTH] = 1 ,[SKILL_SPELL_EVO] = 2, [SKILL_SPELL_ABJ] = 1, [SKILL_SPELL_TRANS] = 1, [SKILL_SPELL_NECRO] = 2,
          }
        },
      }
    },
    [PROF_HEALER] = {ENT_GOBLIN, PROF_HEALER, 1,
      {
        {
          5, CLASS_BASE_CLERIC, "Giver", //"Witch-doctor", "Shadow Priest",
          .skills = {
            [SKILL_WEAP_DAGGER] = 1, [SKILL_ARMOR_CLOTH] = 1, [SKILL_SPELL_ENCH] = 2, [SKILL_SPELL_ABJ] = 3, [SKILL_SPELL_NECRO] = 2,
          },
          .beefups = {
            [SKILL_ARMOR_CLOTH] = 1 ,[SKILL_SPELL_ENCH] = 1, [SKILL_SPELL_ABJ] = 1, [SKILL_SPELL_NECRO] = 2,
          }

        }
      }
    },
  },
  [ENT_ORC] = {
    [PROF_SOLDIER] = {ENT_ORC, PROF_SOLDIER, 3,
      {
        {
          25, CLASS_BASE_FIGHTER, "Grunt",//"Champion","Warlord",
          .skills = {
            [SKILL_WEAP_AXE] = 2, [SKILL_ARMOR_CHAIN] = 2, [SKILL_ARMOR_SHIELD] = 1
          },
          .beefups = {
            [SKILL_WEAP_AXE] = 2, [SKILL_ARMOR_CHAIN] = 2, [SKILL_ARMOR_SHIELD] = 1
          }
        }, 
        {
          8, CLASS_BASE_ROGUE, "Cutthroat", //"Infiltrator", "Saboteur",
          .skills = {
            [SKILL_WEAP_DAGGER] = 3, [SKILL_ARMOR_LEATHER] = 2, 
            [SKILL_STEALTH] = 1,
          },
          .beefups = {
            [SKILL_WEAP_DAGGER] = 2, [SKILL_ARMOR_LEATHER] = 2, 
            [SKILL_STEALTH] = 1,
          }
        },
        {
          7, CLASS_BASE_BERZ, "Berserker", //"Juggernaut", "Warlord",
          .skills = {
            [SKILL_ARMOR_NATURAL] = 5, [SKILL_WEAP_AXE] = 3,
          },
          .beefups = {
            [SKILL_ARMOR_NATURAL] = 4, [SKILL_WEAP_AXE] = 4,
          }
        }
      }
    },
    [PROF_ARCHER] = { ENT_ORC, PROF_ARCHER, 1,
      {
        {
          10, CLASS_BASE_RANGER, "Stalker",// "Slayer", "Hunter",
          .skills = {
            [SKILL_WEAP_BOW] = 3, [SKILL_WEAP_AXE] = 1, [SKILL_ARMOR_LEATHER] = 2
          },
          .beefups = {
            [SKILL_WEAP_BOW] = 1, [SKILL_WEAP_AXE] = 1, [SKILL_ARMOR_LEATHER] = 1
          }
        },
      }
    },
    [PROF_MYSTIC] = { ENT_ORC, PROF_MYSTIC,1,
      {
        {
          5, CLASS_BASE_SHAMAN, "Shaman",//"Seer", "Farseer",
          .skills = {
            [SKILL_ARMOR_CLOTH]=1, [SKILL_WEAP_STAFF]=2,
            [SKILL_SPELL_ABJ] = 1, [SKILL_SPELL_TRANS]=2, [SKILL_SPELL_CONJ] = 1,
          },
          .beefups = {
            [SKILL_ARMOR_CLOTH] =1, [SKILL_WEAP_STAFF]=1,
            [SKILL_SPELL_ABJ] = 1, [SKILL_SPELL_TRANS]=1, [SKILL_SPELL_CONJ] = 1,
          }
        },
      }
    },
    [PROF_MAGICIAN] = { ENT_ORC, PROF_MAGICIAN, 1, 
      {
        {
          4, CLASS_BASE_LOCK, "Doomsayer",
          .skills = {
            [SKILL_WEAP_DAGGER] = 1, [SKILL_ARMOR_CLOTH] = 1,
            [SKILL_SPELL_EVO] = 2, [SKILL_SPELL_CONJ] = 1, [SKILL_SPELL_NECRO] = 2,
          },
          .beefups = {
            [SKILL_SPELL_EVO] = 2, [SKILL_SPELL_CONJ] = 1, [SKILL_SPELL_NECRO] = 2,
          }
        }
      }
    },
    [PROF_HEALER] = { ENT_ORC, PROF_HEALER,1,
      {
        {
          5, CLASS_BASE_CLERIC, "Priest",
          .skills = {
            [SKILL_WEAP_DAGGER] = 1, [SKILL_ARMOR_CLOTH] = 1, [SKILL_SPELL_ENCH] = 1, [SKILL_SPELL_ABJ] = 1, [SKILL_SPELL_NECRO] = 2,
          },
          .beefups = {
            [SKILL_ARMOR_CLOTH] = 1 ,[SKILL_SPELL_ENCH] = 1, [SKILL_SPELL_ABJ] = 1, [SKILL_SPELL_NECRO] = 2,
          }

        },
      }
    }
  },
    [ENT_HOBGOBLIN] = {
    [PROF_SOLDIER] = {ENT_HOBGOBLIN, PROF_SOLDIER, 2,
      {
        {
          25, CLASS_BASE_FIGHTER, "Foot-Soldier",//"Champion","Warlord",
          .skills = {
            [SKILL_WEAP_SWORD] = 3, [SKILL_ARMOR_PLATE] = 2, [SKILL_ARMOR_SHIELD] = 2
          },
          .beefups = {
            [SKILL_WEAP_SWORD] = 2, [SKILL_ARMOR_PLATE] = 2, [SKILL_ARMOR_SHIELD] = 1
          }
        },
        {
          6, CLASS_BASE_ROGUE, "Ambusher",
          .skills = {
            [SKILL_WEAP_DAGGER] = 3, [SKILL_ARMOR_LEATHER] = 2,
            [SKILL_STEALTH] = 2,
          },
          .beefups = {
            [SKILL_WEAP_DAGGER] = 2, [SKILL_ARMOR_LEATHER] = 2,
            [SKILL_STEALTH] = 1,
          }
        },
      }
    },
    [PROF_ARCHER] = { ENT_HOBGOBLIN, PROF_ARCHER, 1,
      {
        {
          14, CLASS_BASE_RANGER, "Bowman",// "Slayer", "Hunter",
          .skills = {
            [SKILL_WEAP_BOW] = 4, [SKILL_WEAP_SWORD] = 2, [SKILL_ARMOR_LEATHER] = 2
          },
          .beefups = {
            [SKILL_WEAP_BOW] = 2, [SKILL_WEAP_SWORD] = 1, [SKILL_ARMOR_LEATHER] = 1
          }
        },
      }
    },
    [PROF_MYSTIC] = { ENT_HOBGOBLIN, PROF_MYSTIC, 2,
      {
        {
          5, CLASS_BASE_SHAMAN, "Shaman",//"Seer", "Farseer",
          .skills = {
            [SKILL_ARMOR_CLOTH]=1, [SKILL_WEAP_AXE]=2,
            [SKILL_SPELL_ABJ] = 1, [SKILL_SPELL_TRANS]=2, [SKILL_SPELL_CONJ] = 1,
          },
          .beefups = {
            [SKILL_ARMOR_CLOTH] =1, [SKILL_WEAP_AXE]=1,
            [SKILL_SPELL_ABJ] = 1, [SKILL_SPELL_TRANS]=1, [SKILL_SPELL_CONJ] = 1,
          }
        },
        {
          8, CLASS_BASE_DRUID, "Monster-Tamer",
          .skills = {
            [SKILL_ARMOR_LEATHER] = 3, [SKILL_WEAP_SPEAR] = 2,
            [SKILL_ANIM] = 4, [SKILL_INTIM] = 3, [SKILL_ALCH] = 3,
            [SKILL_SURV] = 2, [SKILL_SPELL_ENCH] = 3
          },
          .beefups = {
            [SKILL_ARMOR_LEATHER] = 1, [SKILL_WEAP_SPEAR] = 1,
            [SKILL_ANIM] = 2, [SKILL_SPELL_ENCH] = 2
          }
        }
      }
    },
    [PROF_MAGICIAN] = { ENT_HOBGOBLIN, PROF_MAGICIAN, 1,
      {
        {
          4, CLASS_BASE_LOCK, "Warlock",
          .skills = {
            [SKILL_WEAP_DAGGER] = 1, [SKILL_ARMOR_CLOTH] = 1,
            [SKILL_SPELL_EVO] = 2, [SKILL_SPELL_CONJ] = 1, [SKILL_SPELL_NECRO] = 2,
          },
          .beefups = {
            [SKILL_SPELL_EVO] = 2, [SKILL_SPELL_CONJ] = 1, [SKILL_SPELL_NECRO] = 2,
          }
        },
        {
          4, CLASS_BASE_WIZ, "Sorceror",
          .skills = {
            [SKILL_WEAP_DAGGER] = 1, [SKILL_ARMOR_CLOTH] = 1,
            [SKILL_SPELL_EVO] = 2, [SKILL_SPELL_ILL] = 2,
          },
          .beefups = {
            [SKILL_SPELL_ABJ] = 2, [SKILL_SPELL_EVO] = 1,
            [SKILL_SPELL_ILL] = 1
          }
        }
      }
    },
    [PROF_HEALER] = { ENT_HOBGOBLIN, PROF_HEALER,1,
      {
        {
          5, CLASS_BASE_CLERIC, "Priest",
          .skills = {
            [SKILL_WEAP_STAFF] = 2, [SKILL_ARMOR_CLOTH] = 1, [SKILL_SPELL_ENCH] = 1, [SKILL_SPELL_ABJ] = 1, [SKILL_SPELL_NECRO] = 2,
          },
          .beefups = {
            [SKILL_ARMOR_CLOTH] = 1 ,[SKILL_SPELL_ENCH] = 1, [SKILL_SPELL_ABJ] = 1, [SKILL_SPELL_NECRO] = 2,
          }

        },
      }
    }
  },
  [ENT_BUGBEAR] = {
      [PROF_SOLDIER] = {ENT_BUGBEAR, PROF_SOLDIER, 2,
        {
          { 15, CLASS_BASE_FIGHTER, "Brawler",//"Brute","Champ",
            .skills = {
              [SKILL_ARMOR_LEATHER]=2,
              [SKILL_WEAP_MACE]=3, [SKILL_WEAP_SPEAR]=2
            },
            .beefups = {
              [SKILL_ARMOR_LEATHER]=1,
              [SKILL_WEAP_SPEAR]=2, [SKILL_WEAP_MACE]=2,
            }
          },
          { 10, CLASS_BASE_ROGUE, "Ambusher", //"Infiltrator", "Assassin",
            .skills = {
              [SKILL_ARMOR_LEATHER]=1,[SKILL_STEALTH]=4,
              [SKILL_WEAP_DAGGER]=3, [SKILL_WEAP_MACE]=2
            },
            .beefups = {
              [SKILL_ARMOR_LEATHER]=1,[SKILL_STEALTH]=2,
              [SKILL_WEAP_MART]=2, [SKILL_WEAP_DAGGER]=1,
            }
          }
        }
      },
      [PROF_ARCHER] = { ENT_BUGBEAR, PROF_ARCHER,1,
        {
          {
            10, CLASS_BASE_RANGER, "Tracker",// "Beast-Master", "Beast-Lord",
            .skills = {
              [SKILL_ARMOR_LEATHER]=3, [SKILL_STEALTH] = 2,
              [SKILL_WEAP_SPEAR]=4, [SKILL_WEAP_DAGGER]=2
            },
            .beefups = {
              [SKILL_ARMOR_LEATHER]=1, [SKILL_STEALTH]=1,
              [SKILL_WEAP_SPEAR]=6,
            }
          },
        }
      },
      [PROF_MYSTIC] = { ENT_BUGBEAR, PROF_MYSTIC,1,
        {
          {
            7, CLASS_BASE_SHAMAN, "Shaman",
            .skills = {
              [SKILL_WEAP_NONE] = 2, [SKILL_ARMOR_NATURAL] = 4,
              [SKILL_SPELL_ENCH] = 2, [SKILL_SPELL_ABJ] = 2
            },
            .beefups = {
              [SKILL_WEAP_NONE] = 1, [SKILL_ARMOR_NATURAL] = 1,
              [SKILL_SPELL_ENCH] = 1, [SKILL_SPELL_ABJ] = 1
            }
          }
        }
      }


  }
};

static race_class_t PROMOTE_GENERIC[CLASS_DONE] = {
  [CLASS_SUB_CHAMP]     = {
    .skills = {
      [SKILL_WEAP_MART] = 8, [SKILL_ATH] = 5, [SKILL_SURV] = 3,
      [SKILL_WEAP_GREAT] = 4
    },
    .beefups = {
      [SKILL_WEAP_MART] = 6, 
      [SKILL_WEAP_GREAT] = 4
    }
  },
  [CLASS_SUB_ASSASSIN]  = {
     .skills = {
      [SKILL_WEAP_MART] = 2, [SKILL_ACRO] = 2, [SKILL_STEALTH] = 4,
      [SKILL_POISON] = 4, [SKILL_DECEPT] = 4, [SKILL_DUEL] = 4
    },
    .beefups = {
      [SKILL_WEAP_MART] = 1, [SKILL_ACRO] = 1, [SKILL_STEALTH] = 2,
      [SKILL_POISON] = 2, [SKILL_DECEPT] = 2, [SKILL_DUEL] = 2
    } 
  },
  [CLASS_SUB_SHOOTER]   = {
    .skills = {
      [SKILL_WEAP_SHOT] = 12, [SKILL_ACRO] = 2, [SKILL_STEALTH] = 2,
      [SKILL_PERCEPT] = 4,
    },
    .beefups = {
      [SKILL_WEAP_SHOT] = 4, [SKILL_ACRO] = 1, [SKILL_STEALTH] = 1,
      [SKILL_PERCEPT] = 4,
    },

  },
  [CLASS_SUB_HEX]       = {},
  [CLASS_SUB_SEER]      = {},
  [CLASS_SUB_WARLORD]   = {
    .skills = {
      [SKILL_WEAP_MART] = 2, [SKILL_DUEL] = 4, [SKILL_WEAP_GREAT] = 4,
      [SKILL_INTIM] = 5, [SKILL_WEAP_SHOT] = 5
    },
    .beefups = {
      [SKILL_DUEL] = 2, [SKILL_WEAP_SHOT] = 2, [SKILL_INTIM] = 2,
      [SKILL_WEAP_GREAT] = 4
    }
  },
  [CLASS_SUB_BEAST]     = {
   .skills = {
      [SKILL_ANIM] = 12, [SKILL_SPELL_ENCH] = 5, [SKILL_SPELL_CONJ] = 5, [SKILL_NATURE] =4
    },
   .skills = {
      [SKILL_ANIM] = 4, [SKILL_STEALTH] = 2, [SKILL_SURV] = 2
    },
  },
  [CLASS_SUB_NECRO] = {
    .skills = {
      [SKILL_SPELL_NECRO] = 7, [SKILL_ARCANA] = 4, [SKILL_RELIG] = 5,
      [SKILL_MED] = 4, [SKILL_ALCH] =3
    },
    .beefups = {
      [SKILL_SPELL_NECRO] = 3, [SKILL_ARCANA] = 2, [SKILL_RELIG] = 1,
    }

  },
  [CLASS_SUB_HARBINGER] = {
    .skills = {
      [SKILL_ARMOR_PLATE] = 7, [SKILL_WEAP_MART] = 7, [SKILL_WEAP_GREAT] = 7, [SKILL_ARCANA] = 4
    },
    .beefups = {
      [SKILL_ARMOR_PLATE] = 4, [SKILL_WEAP_MART] = 4, [SKILL_WEAP_GREAT] = 4, [SKILL_ARCANA] = 1
    }
  },
  [CLASS_SUB_WARMAG] =  {
    .skills = {
      [SKILL_WEAP_MART] = 5, [SKILL_ARMOR_CHAIN] = 5,
      [SKILL_SPELL_ABJ] = 4, [SKILL_SPELL_EVO] = 4, [SKILL_SPELL_TRANS] = 4
    },
    .beefups = {
      [SKILL_DUEL] = 2, [SKILL_WEAP_SHOT] = 2, [SKILL_INTIM] = 2,
      [SKILL_WEAP_GREAT] = 4
    }

  },
  [CLASS_SUB_DEATH] = {
    .skills = {
      [SKILL_SPELL_NECRO] = 4, [SKILL_ARCANA] = 3, [SKILL_WEAP_MART] = 4,
      [SKILL_SPELL_CONJ] = 4, [SKILL_ALCH] =3, [SKILL_POISON] = 2
    },
    .beefups = {
      [SKILL_SPELL_NECRO] = 2, [SKILL_ARCANA] = 2, [SKILL_WEAP_MART] = 2,
      [SKILL_SPELL_CONJ] = 2
    }

  },
  [CLASS_SUB_DUNG] = {
    .skills = {
      [SKILL_DUEL] = 3, [SKILL_ARMOR_LEATHER] = 4, [SKILL_WEAP_WHIP] = 6,
      [SKILL_ANIM] = 3, [SKILL_SURV] = 3, [SKILL_ACRO] = 3
    },
    .beefups = {
      [SKILL_DUEL] = 2, [SKILL_ARMOR_LEATHER] = 2, [SKILL_ANIM] = 2,
      [SKILL_ACRO] = 2
    }
  }
};

static define_rankup_t CLASS_LADDER[ENT_DONE][CLASS_BASE_DONE] = {
  [ENT_GOBLIN] = {
    [CLASS_BASE_FIGHTER] = {
      2, {CLASS_SUB_CHAMP, CLASS_SUB_WARLORD}
    },
    [CLASS_BASE_ROGUE] =  {
      2, {CLASS_SUB_ASSASSIN, CLASS_SUB_CHAMP}
    },
    [CLASS_BASE_RANGER] =  {
      2, {CLASS_SUB_BEAST, CLASS_SUB_BEAST}
    },

  },
  [ENT_ORC] = {

  },
  [ENT_HOBGOBLIN] = {
    [CLASS_BASE_FIGHTER] = {
      2, {CLASS_SUB_CHAMP, CLASS_SUB_WARLORD}
    },
    [CLASS_BASE_ROGUE] =  {
      1, {CLASS_SUB_ASSASSIN}
    },
    [CLASS_BASE_RANGER] =  {
      1, {CLASS_SUB_SHOOTER}
    },
    [CLASS_BASE_SHAMAN] = {
      2, {CLASS_SUB_HEX, CLASS_SUB_WARMAG}
    },
    [CLASS_BASE_DRUID] = {
      2, {CLASS_SUB_BEAST, CLASS_SUB_DUNG}
    },
    [CLASS_BASE_LOCK] = {
      3, {CLASS_SUB_HARBINGER, CLASS_SUB_CHAMP, CLASS_SUB_WARLORD}
    },
    [CLASS_BASE_WIZ] = {
      1, {CLASS_SUB_NECRO}
    },
    [CLASS_BASE_CLERIC] = {
      2, {CLASS_SUB_WARMAG, CLASS_SUB_DEATH}
    }
  },
  [ENT_BUGBEAR] = {
    [CLASS_BASE_FIGHTER] = {
      2, {CLASS_SUB_CHAMP, CLASS_SUB_WARLORD}
    },
    [CLASS_BASE_ROGUE] =  {
      2, {CLASS_SUB_ASSASSIN, CLASS_SUB_CHAMP}
    },
    [CLASS_BASE_RANGER] =  {
      2, {CLASS_SUB_BEAST, CLASS_SUB_BEAST}
    },
  }


};
static race_class_t PROMOTE_RACE[ENT_DONE][CLASS_DONE] = {
  [ENT_HOBGOBLIN] = {
    [CLASS_SUB_CHAMP] = {0,
      CLASS_SUB_CHAMP, "Champion",
      .skills = {
        [SKILL_WEAP_SWORD] = 5, [SKILL_ARMOR_PLATE] = 5,
        [SKILL_WEAP_GREAT] = 7,
      },
      .beefups = {
        [SKILL_WEAP_SWORD] = 3, [SKILL_ARMOR_PLATE] = 3,
        [SKILL_WEAP_GREAT] = 3
      }
    },
    [CLASS_SUB_ASSASSIN] = {0,
      CLASS_SUB_ASSASSIN, "Infiltrator",
      .skills = {
        [SKILL_ARMOR_LEATHER]=5,
        [SKILL_WEAP_DAGGER]=6, [SKILL_WEAP_MACE]=5
      },
      .beefups = {
        [SKILL_ARMOR_LEATHER]=3,
        [SKILL_WEAP_MACE] = 2, [SKILL_WEAP_DAGGER]=4,
      }
    },
    [CLASS_SUB_WARLORD] = {0,
      CLASS_SUB_WARLORD, "CHIEF",
      .skills = {
        [SKILL_WEAP_MACE] = 7, [SKILL_ARMOR_LEATHER] = 7,
      },
      .beefups = {
        [SKILL_WEAP_MACE] = 3, [SKILL_ARMOR_LEATHER] = 3,
      }

    },
    [CLASS_SUB_BEAST] = {0,
      CLASS_SUB_BEAST, "Beast-Master",
      .skills = {
        [SKILL_WEAP_NET] = 4, [SKILL_WEAP_SPEAR] = 4, [SKILL_ARMOR_LEATHER] = 5,
      },
      .beefups = {
        [SKILL_WEAP_NET] = 2, [SKILL_WEAP_SPEAR] = 3, [SKILL_ARMOR_LEATHER] = 3,
      }

    },
    [CLASS_SUB_DUNG] = {0,
      CLASS_SUB_BEAST, "Dungeon-Master",
      .skills = {
        [SKILL_SPELL_ENCH] = 4, [SKILL_WEAP_WHIP] = 3, [SKILL_WEAP_NET] = 3, [SKILL_ANIM] = 3, [SKILL_ALCH] = 3
      },
      .beefups = {
        [SKILL_WEAP_WHIP] = 2, [SKILL_ANIM] = 2, [SKILL_WEAP_NET] = 2,
        [SKILL_SPELL_ENCH] = 1
      }
    },
    [CLASS_SUB_NECRO] = {0,
      CLASS_SUB_NECRO, "Necromancer",
      .skills = {
        [SKILL_ARMOR_CLOTH] = 3, [SKILL_WEAP_DAGGER] = 2, [SKILL_SPELL_CONJ] = 4,
      },
      .beefups = {
        [SKILL_SPELL_CONJ] = 2, [SKILL_WEAP_DAGGER] = 2, [SKILL_SPELL_ABJ]= 2
      }

    },
    [CLASS_SUB_WARMAG] = {0,
      CLASS_SUB_WARMAG, "Devastator",
      .skills = {
        [SKILL_WEAP_SWORD] = 5, [SKILL_WEAP_GREAT] = 5, [SKILL_INSIGHT] = 6, [SKILL_SPELL_EVO] = 4, [SKILL_SPELL_ABJ] = 4,
      },
      .beefups = {
        [SKILL_WEAP_SWORD] = 2, [SKILL_WEAP_GREAT] = 2,
        [SKILL_SPELL_EVO] = 2, [SKILL_SPELL_ABJ] = 2
      }
    },
    [CLASS_SUB_DEATH] = {0,
      CLASS_SUB_DEATH, "Death-Lord",
      .skills = {
        [SKILL_WEAP_SWORD] = 4, [SKILL_WEAP_MART] = 3, [SKILL_ATH] = 4,
        [SKILL_MED] = 3, [SKILL_SPELL_CONJ] = 3, [SKILL_SPELL_ABJ] = 3
      },
      .beefups = {
        [SKILL_WEAP_SWORD] = 2, [SKILL_WEAP_MART] = 1, [SKILL_MED] = 1,
        [SKILL_SPELL_CONJ] = 1, [SKILL_SPELL_TRANS] = 2
      }
    },
    [CLASS_SUB_SHOOTER] = {0,
      CLASS_SUB_SHOOTER, "Marskman",
      .skills = {
        [SKILL_WEAP_BOW] = 4, [SKILL_WEAP_MART] = 5, [SKILL_WEAP_GREAT] = 4, [SKILL_ATH] = 4,
      },
      .beefups = {
         [SKILL_WEAP_BOW] = 3, [SKILL_WEAP_MART] = 2, [SKILL_WEAP_GREAT] =2, [SKILL_ATH] = 1,
      }

    }
  },
    [ENT_BUGBEAR] = {
    [CLASS_SUB_CHAMP] = {0,
      CLASS_SUB_CHAMP, "Brute",
      .skills = {
        [SKILL_WEAP_MACE] = 5, [SKILL_ARMOR_LEATHER] = 5,
      },
      .beefups = {
        [SKILL_WEAP_MACE] = 3, [SKILL_ARMOR_LEATHER] = 3,
      }
    },
    [CLASS_SUB_ASSASSIN] = {0,
      CLASS_SUB_ASSASSIN, "Infiltrator",
      .skills = {
        [SKILL_ARMOR_LEATHER]=5,
        [SKILL_WEAP_DAGGER]=6, [SKILL_WEAP_MACE]=5
      },
      .beefups = {
        [SKILL_ARMOR_LEATHER]=3,
        [SKILL_WEAP_MACE] = 2, [SKILL_WEAP_DAGGER]=4,
      }
    },
    [CLASS_SUB_WARLORD] = {0,
      CLASS_SUB_WARLORD, "CHIEF",
      .skills = {
        [SKILL_WEAP_MACE] = 7, [SKILL_ARMOR_LEATHER] = 7,
      },
      .beefups = {
        [SKILL_WEAP_MACE] = 3, [SKILL_ARMOR_LEATHER] = 3,
      }

    },
    [CLASS_SUB_BEAST] = {0,
      CLASS_SUB_BEAST, "Beast-Master",
      .skills = {
        [SKILL_WEAP_MACE] = 4, [SKILL_WEAP_SPEAR] = 4, [SKILL_ARMOR_LEATHER] = 5,
      },
      .beefups = {
        [SKILL_WEAP_MACE] = 3, [SKILL_WEAP_SPEAR] = 3, [SKILL_ARMOR_LEATHER] = 3,
      }

    }  
  },
};

static mob_define_t MONSTER_MASH[ENT_DONE] = {
  {ENT_PERSON,
    .race = SPEC_HUMAN
  },
  {ENT_GOBLIN, "Goblin",
      MOB_SPAWN_CHALLENGE | MOB_SPAWN_CAMP | MOB_SPAWN_PATROL |
      MOB_MOD_ENLARGE | MOB_MOD_WEAPON | MOB_MOD_ARMOR |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_FREQ_UNCOMMON|
      MOB_THEME_PRIMITIVE |
      MOB_GROUPING_TROOP | MOB_GROUPING_CREW | MOB_GROUPING_SQUAD | MOB_GROUPING_WARBAND,
      SPEC_GOBLINOID,
      {0,0}, 4,
      0.5,
      SOC_PRIMITIVE,
      .flags = {
        PQ_SMALL | PQ_LIGHT
      } 
  },
  {ENT_ORC, "Orc",
      MOB_SPAWN_CHALLENGE | MOB_SPAWN_CAMP | MOB_SPAWN_PATROL |
      MOB_MOD_ENLARGE | MOB_MOD_WEAPON | MOB_MOD_ARMOR |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_FREQ_UNCOMMON |
      MOB_THEME_MARTIAL |
      MOB_GROUPING_PAIRS | MOB_GROUPING_TROOP | MOB_GROUPING_CREW | MOB_GROUPING_SQUAD | MOB_GROUPING_WARBAND,
      SPEC_ORC,
      {0,0}, 9,
      1.15,
      SOC_MARTIAL,
      .promotions = {[CLASS_BASE_FIGHTER] = 2, [CLASS_BASE_BERZ] = 2, [CLASS_BASE_SHAMAN] = 1}
  },
  {ENT_OGRE},
  {ENT_HOBGOBLIN, "Hobgoblin",
      MOB_SPAWN_CHALLENGE | MOB_SPAWN_CAMP | MOB_SPAWN_PATROL |
      MOB_MOD_ENLARGE | MOB_MOD_WEAPON | MOB_MOD_ARMOR |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_FREQ_RARE|
      MOB_THEME_MARTIAL |
      MOB_GROUPING_PAIRS | MOB_GROUPING_TROOP | MOB_GROUPING_CREW | MOB_GROUPING_SQUAD | MOB_GROUPING_WARBAND,
      SPEC_GOBLINOID,
      {0,0}, 12,
      1.2f,
      SOC_MARTIAL,
      .flags = {
        PQ_LONG_LIMB,
        MQ_STRATEGIC | MQ_DISCIPLINED | MQ_LEADER
      }
  },
  {ENT_SCORPION, "Scorpion",
      MOB_MOD_ENLARGE |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_THEME_CRITTER |
      MOB_FREQ_RARE |
      MOB_GROUPING_SOLO | MOB_GROUPING_SWARM,
    SPEC_ARTHROPOD,
    {75,67}, 3,
    .65,
    SOC_INSTINCTIVE,
  },
  {ENT_SPIDER, "Spider",
      MOB_MOD_ENLARGE |
      MOB_LOC_MASK |
      MOB_THEME_CRITTER |
      MOB_FREQ_COMMON |
      MOB_GROUPING_MASK,
    SPEC_ARTHROPOD,
    {80,69}, 2,
    0.25,
    SOC_INSTINCTIVE,
    .flags = {
      PQ_MANY_EYES,
      .weaps = PQ_FANGS | PQ_POISON_FANGS
    }
  },
  {ENT_TROLL, "Troll",
      MOB_SPAWN_LAIR |
      MOB_MOD_ENLARGE |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST|
      MOB_THEME_MONSTER |
      MOB_FREQ_RARE |
      MOB_GROUPING_SOLO,
      SPEC_GIANT,
      {6,5}, 5,
      2.25f,
      SOC_INSTINCTIVE
  },
  {ENT_TROLL_CAVE, "Cave Troll",
    MOB_SPAWN_LAIR |
      MOB_MOD_ENLARGE |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE |
      MOB_THEME_MONSTER |
      MOB_FREQ_RARE|
      MOB_GROUPING_SOLO,
    SPEC_GIANT,
    {1,5}, 6,
    2.75f,
    SOC_INSTINCTIVE
  },
  {ENT_BEAR, "Bear",
      MOB_SPAWN_LAIR |
      MOB_MOD_ENLARGE | MOB_FREQ_UNCOMMON |
      MOB_LOC_CAVE | MOB_LOC_FOREST | MOB_THEME_PRED |
      MOB_GROUPING_SOLO | MOB_GROUPING_PAIRS | MOB_GROUPING_SQUAD,
      SPEC_CANIFORM,
      {32,0}, 2,
      .975,
      SOC_FERAL,
      .flags = {
        PQ_LARGE | PQ_DENSE_MUSCLE | PQ_LARGE_HANDS,
      0, PQ_SHARP_CLAWS | PQ_SHARP_TEETH | PQ_TOUGH_CLAWS,
      PQ_THICK_FUR | PQ_TOUGH_HIDE
      }
  },
  {ENT_WOLF, "Wolf",
      MOB_SPAWN_LAIR | MOB_SPAWN_CHALLENGE |
      MOB_MOD_ENLARGE | MOB_FREQ_COMMON |
      MOB_LOC_CAVE | MOB_LOC_FOREST | MOB_THEME_PRED |
      MOB_GROUPING_TROOP | MOB_GROUPING_CREW | MOB_GROUPING_SQUAD,
      SPEC_CANIFORM,
      {64,0}, 1,
      0.65,
      SOC_FERAL,
      .flags = {
        PQ_SENSITIVE_NOSE
      }
  },
  {ENT_RAT, "Rat",
      MOB_MOD_ENLARGE |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_THEME_CRITTER |
      MOB_FREQ_COMMON |
      MOB_GROUPING_TROOP | MOB_GROUPING_CREW | MOB_GROUPING_SQUAD | MOB_GROUPING_SWARM,
      SPEC_RODENT,
      {24,69}, 1,
      .25,
      SOC_HIVE
  },
  {ENT_SKELETON,"Skeleton",
    MOB_THEME_MONSTER

  },
  {ENT_DEER, "Deer",
    MOB_MOD_ENLARGE | MOB_LOC_FOREST |
    MOB_THEME_GAME | MOB_FREQ_COMMON |
    MOB_GROUPING_PAIRS | MOB_GROUPING_TROOP |MOB_GROUPING_CREW |
    MOB_GROUPING_SQUAD,
    SPEC_RUMINANT,
    {60,0}, 2,
    0.375,
    SOC_FAMILY
  },
  {ENT_KOBOLD, "Kobold",
    MOB_MOD_WEAPON | MOB_LOC_FOREST | MOB_LOC_CAVE |
      MOB_THEME_CRITTER | MOB_THEME_PRIMITIVE |
      MOB_FREQ_UNCOMMON | MOB_GROUPING_PAIRS | MOB_GROUPING_TROOP |
      MOB_GROUPING_CREW | MOB_GROUPING_PARTY,
    SPEC_SULKING,
    {69,24}, 1,
    0.375,
    SOC_INSTINCTIVE,
    .flags = PQ_SMALL
  },
  {ENT_BUGBEAR, "Bugbear",
    MOB_SPAWN_CHALLENGE | MOB_SPAWN_LAIR | MOB_SPAWN_CAMP | MOB_SPAWN_PATROL |
    MOB_MOD_WEAPON | MOB_LOC_FOREST | MOB_LOC_CAVE |
      MOB_THEME_PRIMITIVE |
      MOB_FREQ_COMMON | MOB_GROUPING_SOLO | MOB_GROUPING_PAIRS |
      MOB_GROUPING_PARTY,
    SPEC_SULKING,
    {54,8}, 3,
    1.25,
    SOC_PRIMITIVE,
    .flags = {
      PQ_LARGE_NOSE | PQ_LARGE | PQ_DENSE_MUSCLE | PQ_LONG_LIMB,
      MQ_SENTIENT,
      PQ_SHARP_TEETH | PQ_SHARP_CLAWS,
      PQ_THICK_FUR
    },
    .promotions = {[CLASS_BASE_FIGHTER] = 2, [CLASS_BASE_RANGER] =2, [CLASS_BASE_ROGUE] =2}
  }

};

static mob_variants_t MOB_MODS[ENT_DONE] = {
  {ENT_PERSON},
  {ENT_GOBLIN},
  {ENT_ORC},
  {ENT_OGRE},
  {ENT_HOBGOBLIN},
  {ENT_SCORPION},
  {ENT_SPIDER,
    {
      [MM_ALPHA] = {
        MM_ALPHA, BLANK, 3, "%s Brood-Mother",
        PQ_LARGE | PQ_LONG_LIMB | PQ_DENSE_MUSCLE,
        MQ_TERRITORIAL,
        PQ_CLAWS,
        .skillups = {
          SKILL_ACRO, SKILL_ARMOR_NATURAL, SKILL_POISON, SKILL_WEAP_NONE, SKILL_SURV
        }
      }
    }
  },
  {ENT_TROLL},
  {ENT_TROLL_CAVE},
  {ENT_BEAR,
    {
      [MM_ALPHA] = {
        MM_ALPHA, BLANK, 4, "Apex %s",
        PQ_LARGE_HANDS | PQ_LARGE_FEET | PQ_TALL,
        .skillups = {
          SKILL_ATH, SKILL_ARMOR_NATURAL, SKILL_INTIM, SKILL_WEAP_NONE, SKILL_SURV

        }
      },
      [MM_BRUTE] = {
        MM_BRUTE, BLANK, 3, "Dire %s",
        PQ_HUGE,
        MQ_CUNNING,
        .skillups = { 
          SKILL_ACRO, SKILL_ARMOR_NATURAL, SKILL_ATH, SKILL_WEAP_NONE, SKILL_SURV
        }
      }
    }

  },
  {ENT_WOLF,
    {
      [MM_ALPHA] = {
        MM_ALPHA, BLANK, 3, "Alpha %s", 
        PQ_DENSE_MUSCLE | PQ_LARGE_HANDS | PQ_LARGE_FEET,
        MQ_PROTECTIVE | MQ_LEADER,
        PQ_SHARP_CLAWS | PQ_SHARP_TEETH,
          .skillups = {
            SKILL_ACRO, SKILL_ARMOR_NATURAL, SKILL_INTIM, SKILL_WEAP_NONE, SKILL_SURV
          }
      },
      [MM_BRUTE] = {
        MM_BRUTE, BLANK, 2, "Dire %s", 
        PQ_LARGE,
        MQ_CUNNING,
          .skillups = {
            SKILL_ACRO, SKILL_ARMOR_NATURAL, SKILL_ATH, SKILL_WEAP_NONE, SKILL_SURV
          }
      }

    }
  },
  {ENT_RAT},
  {ENT_SKELETON},
  {ENT_DEER,
    {
      [MM_BRUTE] = {
        MM_BRUTE, BLANK, 3, "Stag",
        PQ_TALL | PQ_LARGE,
        MQ_DETERMINED,
        PQ_TOUGH_HORNS,
        PQ_TOUGH_HIDE,
        .skillups = {
          SKILL_ACRO, SKILL_ARMOR_NATURAL, SKILL_INTIM, SKILL_WEAP_NONE, SKILL_SURV
        }
      }
    }
  },
  {ENT_KOBOLD,
  
  },
  {ENT_BUGBEAR},
  {ENT_KNIGHT},
  {ENT_FOOTMAN},

};

static define_ability_class_t CLASS_ABILITIES[ABILITY_DONE]={
  {ABILITY_NONE},
  {ABILITY_PUNCH},
  {ABILITY_BITE},
  {ABILITY_CHEW},
  {ABILITY_GNAW},
  {ABILITY_CLAW},
  {ABILITY_SWIPE},
  {ABILITY_BITE_POISON},
  {ABILITY_POISON},
  {ABILITY_MAGIC_MISSLE, true, true,
    CLASS_BASE_WIZ,
    CLASS_BASE_WIZ,
    0,9,10,
    SPEC_HUMAN | SPEC_ELF,
  },
  {ABILITY_ELDRITCH_BLAST, true,true, 
    CLASS_BASE_LOCK,
    CLASS_SUB_HEX,
    0,5,10,
    SPEC_ORC | SPEC_HUMAN
  },
  {ABILITY_RESISTANCE, true, true,
    CLASS_BASE_CLERIC | CLASS_BASE_DRUID,
    CLASS_BASE_SHAMAN,
    0,3,8,
    SPEC_HUMAN// | SPEC_ELF | SPEC_GOBLINOID | SPEC_ORC
  },
  {ABILITY_GUIDING_BOLT, true, true,
    CLASS_BASE_CLERIC,
    0, 3, 6,
  },
  {ABILITY_SACRED_FLAME, true, true,
    CLASS_BASE_CLERIC,
    CLASS_BASE_CLERIC,
    0,3,5,
    SPEC_HUMAN | SPEC_ELF
  },
  {ABILITY_STARRY_WISP, true, true,
    CLASS_BASE_BARD | CLASS_BASE_DRUID,
    CLASS_BASE_SHAMAN,
    0,3,6,
    SPEC_ELF | SPEC_GOBLINOID | SPEC_ORC
  },
  {ABILITY_MAGIC_STONE, true, true,
    CLASS_BASE_DRUID,
    CLASS_BASE_LOCK,
    0, 0, 5,
    SPEC_ELF | SPEC_GOBLINOID | SPEC_ORC
  },
  {ABILITY_POISON_SPRAY, true, true,
    CLASS_BASE_DRUID | CLASS_BASE_SHAMAN,
    CLASS_BASE_WIZ,
    0, 3, 8,
    SPEC_GOBLINOID | SPEC_ORC
  },
  {ABILITY_FIRE_BOLT, true, true,
    CLASS_BASE_LOCK,
    CLASS_BASE_WIZ,
    0,3,8,
    SPEC_HUMAN | SPEC_ARCHAIN | SPEC_GOBLINOID | SPEC_ORC
  },
  {ABILITY_CURE_WOUNDS, true, true,
    CLASS_BASE_CLERIC | CLASS_BASE_DRUID,
    CLASS_BASE_SHAMAN,
    0,8,8,
    SPEC_HUMAN | SPEC_ARCHAIN | SPEC_GOBLINOID | SPEC_ORC
  },
  {ABILITY_REND, false, false,
    CLASS_BASE_FIGHTER,
    CLASS_BASE_RANGER,
    0, 0,4,
    SPEC_GOBLINOID | SPEC_ORC | SPEC_HUMAN
  },
  {ABILITY_HAMSTRING, false, false,
    CLASS_BASE_FIGHTER | CLASS_BASE_RANGER,
    CLASS_BASE_ROGUE,
    1,0,4,
    SPEC_HUMAN //| SPEC_GOBLINOID | SPEC_ORC
  },
  {ABILITY_RAGE, false, false,
    CLASS_BASE_BERZ,
    CLASS_BASE_BERZ,
    0,5,12,
    SPEC_GOBLINOID | SPEC_ORC | SPEC_ARCHAIN
  },

};
static skill_relation_t SKILLUP_RELATION[MAG_DONE] = {
  [MAG_MINOR] = {
    .mag = MAG_MINOR,
    .skill = SKILL_LVL,
    .skills = {
      [SKILL_WEAP_NONE]        = true,
      [SKILL_WEAP_SIMP]        = true,
      [SKILL_ARMOR_NATURAL]    = true,
      [SKILL_ALCH]             = true,
      [SKILL_ANIM]           = true,
      [SKILL_ATH]           = true,
      [SKILL_CALL]           = true,
      [SKILL_CARP]           = true,
      [SKILL_CART]           = true,
      [SKILL_COBB]           = true,
      [SKILL_COOK]           = true,
      [SKILL_DECEPT]           = true,
      [SKILL_DUEL]           = true,
      [SKILL_GLASS]           = true,
      [SKILL_HIST]           = true,
      [SKILL_HERB]           = true,
      [SKILL_INSIGHT]           = true,
      [SKILL_INTIM]           = true,
      [SKILL_INVEST]           = true,
      [SKILL_JEWL]           = true,
      [SKILL_LW]           = true,
      [SKILL_MASON]           = true,
      [SKILL_MED]           = true,
      [SKILL_NATURE]           = true,
      [SKILL_PAINT]           = true,
      [SKILL_PERCEPT]           = true,
      [SKILL_PERFORM]           = true,
      [SKILL_PERSUAD]           = true,
      [SKILL_POTT]           = true,
      [SKILL_RELIG]           = true,
      [SKILL_SMITH]           = true,
      [SKILL_STONE]           = true,
      [SKILL_TINK]           = true,
      [SKILL_WEAV]           = true,
      [SKILL_WOOD]           = true,
    }
  },

  [MAG_NOMINAL] = {
    .mag = MAG_NOMINAL,
    .skill = SKILL_LVL,
    .skills = {
      [SKILL_STEALTH]          = true,
      [SKILL_SURV]             = true,
      [SKILL_THEFT]            = true,
      [SKILL_WEAP_MACE]        = true,
      [SKILL_WEAP_SWORD]       = true,
      [SKILL_WEAP_AXE]         = true,
      [SKILL_WEAP_DAGGER]      = true,
      [SKILL_WEAP_BOW]         = true,
      [SKILL_WEAP_PICK]        = true,
      [SKILL_ARMOR_SHIELD]     = true,
    }
  },

  [MAG_MODEST] = {
    .mag = MAG_MODEST,
    .skill = SKILL_LVL,
    .skills = {
      [SKILL_WEAP_MART]        = true,
      [SKILL_WEAP_SHOT]        = true,
      [SKILL_ARMOR_PADDED]     = true,
      [SKILL_ARMOR_LEATHER]    = true,
      [SKILL_ARMOR_CHAIN]      = true,
      [SKILL_ARMOR_PLATE]      = true,

      [SKILL_SPELL_ABJ]        = true,
      [SKILL_SPELL_CONJ]       = true,
      [SKILL_SPELL_DIV]        = true,
      [SKILL_SPELL_ENCH]       = true,
      [SKILL_SPELL_EVO]        = true,
      [SKILL_SPELL_ILL]        = true,
      [SKILL_SPELL_NECRO]      = true,
    }
  },
  [MAG_SIGNIF] = {
    .mag = MAG_SIGNIF,
    .skill = SKILL_LVL,
    .skills = {
      [SKILL_SLEIGHT]           = true,
      [SKILL_WRESTLE]         = true,
      [SKILL_WEAP_GREAT]      = true,
      [SKILL_WEAP_NET]        = true,
      [SKILL_WEAP_WHIP]       = true,
      [SKILL_POISON]           = true,
  }
  }

};

static define_slot_actions SLOTS_ALLOWED[SLOT_ALL] = {
  {SLOT_ATTACK,
    {
      [ACTION_ATTACK] = true, [ACTION_WEAPON] = true,
    },
    STAT_STAMINA,
  },
  {SLOT_INATE,
    {
      [ACTION_PASSIVE] = true,  
    }
  },
  {SLOT_ITEM, {[ACTION_ITEM]=true}, STAT_ACTIONS},
  {SLOT_SPELL, {[ACTION_MAGIC]=true}, STAT_ENERGY},
  {SLOT_SAVE, {[ACTION_SAVE]=true}},
};

static define_burden_t BURDEN_LIMITS[INV_SLING][6] = {
  [INV_HELD] = {
    {PQ_TINY, 1250},
    {PQ_SMALL, 2500},
    {PQ_LARGE, 12500},
    {PQ_HUGE, 25000},
    {PQ_GIG, 50000},
    {0, 10000},
  },
  [INV_WORN] = {
    {PQ_TINY, 1500},
    {PQ_SMALL, 3000},
    {PQ_LARGE, 15000},
    {PQ_HUGE, 30000},
    {PQ_GIG, 60000},
    {0, 12000},
  }
};

static define_inventory_t ITEMS_ALLOWED[INV_DONE] = {
  {INV_HELD,
    {
      [ITEM_ARMOR] = 0, [ITEM_WEAPON] = 2
    },
    2,
    0x2000
  },
  {INV_WORN,
    {[ITEM_ARMOR] = 1, [ITEM_RING] = 2, [ITEM_AMULET] = 1 },
    5,
    0x3000
  },
  {INV_BACK,
    {
      [ITEM_WEAPON] = 2, [ITEM_CONTAINER] = 1
    },
    2,
    0x1000,
  },
  {INV_BELT,
    {
      [ITEM_CONSUMABLE]=4, [ITEM_CONTAINER]=2, [ITEM_WEAPON] = 2
    },
    0,
    0x1000
  },
  {INV_SLING,
    {
      [ITEM_AMMO] = 2, [ITEM_CONSUMABLE] = 2
    },
    0,
    0x0500
  },
  {INV_QUIVER,
    {
      [ITEM_AMMO] = 10
    },
    0,
    0x0050
  },
  {INV_PACK,
  },
  {INV_POUCH},
  {INV_SCROLL_CASE},
};

static const char* SKILL_NAMES[SKILL_DONE] = {
  [SKILL_NONE]              = "None",
  [SKILL_LVL]               = "Leveling",
  [SKILL_ACRO]              = "Acrobatics",
  [SKILL_ALCH]              = "Alchemy",
  [SKILL_ANIM]              = "Animal Handling",
  [SKILL_ARCANA]            = "Arcana",

  [SKILL_ARMOR_NATURAL]     = "Natural Armor",
  [SKILL_ARMOR_PADDED]      = "Padded Armor",
  [SKILL_ARMOR_LEATHER]     = "Leather Armor",
  [SKILL_ARMOR_CHAIN]       = "Chain Armor",
  [SKILL_ARMOR_PLATE]       = "Plate Armor",
  [SKILL_ARMOR_SHIELD]      = "Shield Use",

  [SKILL_ATH]               = "Athletics",
  [SKILL_CALL]              = "Calligraphy",
  [SKILL_CARP]              = "Carpentry",
  [SKILL_CART]              = "Cartography",
  [SKILL_COBB]              = "Cobbling",
  [SKILL_COOK]              = "Cooking",

  [SKILL_DECEPT]            = "Deception",
  [SKILL_GLASS]             = "Glassworking",
  [SKILL_HIST]              = "History",
  [SKILL_HERB]              = "Herbalism",
  [SKILL_INSIGHT]           = "Insight",
  [SKILL_INTIM]             = "Intimidation",
  [SKILL_INVEST]            = "Investigation",
  [SKILL_JEWL]              = "Jeweling",
  [SKILL_LW]                = "Leatherworking",

  [SKILL_MASON]             = "Masonry",
  [SKILL_MED]               = "Medicine",
  [SKILL_NATURE]            = "Nature",
  [SKILL_PAINT]             = "Painting",
  [SKILL_PERCEPT]           = "Perception",
  [SKILL_PERFORM]           = "Performance",
  [SKILL_PERSUAD]           = "Persuasion",

  [SKILL_POISON]            = "Poisoncraft",
  [SKILL_POTT]              = "Pottery",
  [SKILL_RELIG]             = "Religion",
  [SKILL_SLEIGHT]           = "Sleight of Hand",
  [SKILL_SMITH]             = "Smithing",

  [SKILL_SPELL_ABJ]         = "Abjuration",
  [SKILL_SPELL_CONJ]        = "Conjuration",
  [SKILL_SPELL_DIV]         = "Divination",
  [SKILL_SPELL_ENCH]        = "Enchantment",
  [SKILL_SPELL_EVO]         = "Evocation",
  [SKILL_SPELL_ILL]         = "Illusion",
  [SKILL_SPELL_NECRO]       = "Necromancy",
  [SKILL_SPELL_TRANS]       = "Transmutation",

  [SKILL_STEALTH]           = "Stealth",
  [SKILL_STONE]             = "Stoneworking",
  [SKILL_SURV]              = "Survival",
  [SKILL_TINK]              = "Tinkering",
  [SKILL_THEFT]             = "Theft",
  [SKILL_WEAV]              = "Weaving",

  [SKILL_WEAP_SIMP]         = "Simple Weapons",
  [SKILL_WEAP_MART]         = "Martial Weapons",
  [SKILL_WEAP_MACE]         = "Maces",
  [SKILL_WEAP_SHOT]         = "Shooting",
  [SKILL_WEAP_SWORD]        = "Swords",
  [SKILL_WEAP_AXE]          = "Axes",
  [SKILL_WEAP_DAGGER]       = "Daggers",
  [SKILL_WEAP_BOW]          = "Bows",
  [SKILL_WEAP_PICK]         = "Picks",
  [SKILL_WEAP_STAFF]        = "Staves",
  [SKILL_WEAP_SPEAR]        = "Spears",
  [SKILL_WEAP_WHIP]         = "Whips",
  [SKILL_WEAP_NET]          = "Nets",
  [SKILL_WEAP_GREAT]        = "Two Handed Weapons",
  [SKILL_WEAP_NONE]         = "Unarmed",
  [SKILL_WRESTLE]           = "Wrestling",
  [SKILL_WOOD]              = "Woodworking",
};
static define_skill_rank_t SKILL_RANKS[SR_DONE] = {
  [SR_NONE]      = { SR_NONE,      0,  0  },
  [SR_NOVICE]    = { SR_NOVICE,    3,  5  },
  [SR_SKILLED]   = { SR_SKILLED,   6,  10, MOD_CBRT },
  [SR_PROFIC]    = { SR_PROFIC,    10, 25, MOD_SQRT },
  [SR_ADEPT]     = { SR_ADEPT,     15, 50 },
  [SR_EXPERT]    = { SR_EXPERT,    30, 75 },
  [SR_ACCOMP]    = { SR_ACCOMP,    45, 100 },
  [SR_GREAT]     = { SR_GREAT,     60, 125 },
  [SR_ARTIS]     = { SR_ARTIS,     75, 150 },
  [SR_MASTER]    = { SR_MASTER,    90, 200 },
  [SR_MASTER_H]  = { SR_MASTER_H,  95, 400 },
  [SR_MASTER_G]  = { SR_MASTER_G,  97, 600 },
  [SR_LEGEND]    = { SR_LEGEND,    99, 800 },
}; 

static skill_proficiency_bonus_t GRANTS_PB[SKILL_DONE] = {
  [SKILL_NONE]  =  {},           
  [SKILL_LVL]  =  {},            
  [SKILL_ACRO]  =  {},           
  [SKILL_ALCH]  =  {},           
  [SKILL_ANIM]  =  {},           
  [SKILL_ARCANA]  =  {},         

  [SKILL_ARMOR_NATURAL]  =  {},  
  [SKILL_ARMOR_PADDED]  =  {},   
  [SKILL_ARMOR_LEATHER]  =  {},  
  [SKILL_ARMOR_CHAIN]  =  {},    
  [SKILL_ARMOR_PLATE]  =  {},    
  [SKILL_ARMOR_SHIELD]  =  {},   

  [SKILL_ATH]  =  {},            
  [SKILL_CALL]  =  {},           
  [SKILL_CARP]  =  {},           
  [SKILL_CART]  =  {},           
  [SKILL_COBB]  =  {},           
  [SKILL_COOK]  =  {},           

  [SKILL_DECEPT]  =  {},         
  [SKILL_GLASS]  =  {},          
  [SKILL_HIST]  =  {},           
  [SKILL_HERB]  =  {},           
  [SKILL_INSIGHT]  =  {},        
  [SKILL_INTIM]  =  {},          
  [SKILL_INVEST]  =  {},         
  [SKILL_JEWL]  =  {},           
  [SKILL_LW]  =  {},             

  [SKILL_MASON]  =  {},          
  [SKILL_MED]  =  {},            
  [SKILL_NATURE]  =  {},         
  [SKILL_PAINT]  =  {},          
  [SKILL_PERCEPT]  =  {
    SKILL_PERCEPT,
    10,
    SKILL_STEALTH,
    ATTR_WIS,
    PC_HIT
  },        
  [SKILL_PERFORM]  =  {},        
  [SKILL_PERSUAD]  =  {},        

  [SKILL_POISON]  =  {},         
  [SKILL_POTT]  =  {},           
  [SKILL_RELIG]  =  {},          
  [SKILL_SLEIGHT]  =  {},        
  [SKILL_SMITH]  =  {},          

  [SKILL_SPELL_ABJ]  =  {},      
  [SKILL_SPELL_CONJ]  =  {},     
  [SKILL_SPELL_DIV]  =  {},      
  [SKILL_SPELL_ENCH]  =  {},     
  [SKILL_SPELL_EVO]  =  {},      
  [SKILL_SPELL_ILL]  =  {},      
  [SKILL_SPELL_NECRO]  =  {},    
  [SKILL_SPELL_TRANS]  =  {},    

  [SKILL_STEALTH]  =  {
    SKILL_STEALTH,
    SKILL_PERCEPT,
    4,
    ATTR_DEX,
    PC_SAVE
  },        
  [SKILL_STONE]  =  {},          
  [SKILL_SURV]  =  {},           
  [SKILL_TINK]  =  {},           
  [SKILL_THEFT]  =  {},          
  [SKILL_WEAV]  =  {},           

  [SKILL_WEAP_SIMP]  =  {},      
  [SKILL_WEAP_MART]  =  {},      
  [SKILL_WEAP_MACE]  =  {},      
  [SKILL_WEAP_SHOT]  =  {},      
  [SKILL_WEAP_SWORD]  =  {},     
  [SKILL_WEAP_AXE]  =  {},       
  [SKILL_WEAP_DAGGER]  =  {},    
  [SKILL_WEAP_BOW]  =  {},       
  [SKILL_WEAP_PICK]  =  {},      
  [SKILL_WEAP_STAFF]  =  {},     
  [SKILL_WEAP_SPEAR]  =  {},     
  [SKILL_WEAP_WHIP]  =  {},      
  [SKILL_WEAP_NET]  =  {},       
  [SKILL_WEAP_GREAT]  =  {},     
  [SKILL_WEAP_NONE]  =  {},      
  [SKILL_WRESTLE]  =  {},        
  [SKILL_WOOD]  =  {},           
};
#endif
