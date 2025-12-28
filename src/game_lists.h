#ifndef __GAME_LIST__
#define __GAME_LIST__
#include "game_define.h"


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
  {STAT_REACH,"Reach"},
  {STAT_DAMAGE,"Damage"},
  {STAT_HEALTH,"Health"},
  {STAT_ARMOR, "Armor"},
  {STAT_AGGRO, "Sight"},
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
  [STAT_AGGRO] = {
    [SC_MIN]     = 0,
    [SC_INFER]   = 2,
    [SC_BELOW]   = 4,
    [SC_AVERAGE] = 6,
    [SC_ABOVE]   = 8,
    [SC_SUPER]   = 10,
    [SC_MAX]     = 12
  },
  /*
  [STAT_STEALTH] = {
    [SC_MIN]     = 0,
    [SC_INFER]   = 1,
    [SC_BELOW]   = 3,
    [SC_AVERAGE] = 4,
    [SC_ABOVE]   = 5,
    [SC_SUPER]   = 7,
    [SC_MAX]     = 8
  },
  */
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
  {ATTR_CON,"CONSTITUTION"},
  {ATTR_STR,"STRENGTH"},
  {ATTR_DEX,"DEXTERITY"},
  {ATTR_INT,"INTELLIGENCE"},
  {ATTR_WIS,"WISDOM"},
  {ATTR_CHAR,"CHARISMA"},
  {ATTR_BLANK,"REROLL"}
};

static const race_define_t DEFINE_RACE[ 13 ] = {
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
  {SPEC_GOBLINOID, "Goblin", ENT_GOBLIN,
    RACE_USE_TOOLS | RACE_USE_WEAPS | RACE_USE_ARMOR | RACE_USE_POTIONS |RACE_USE_SCROLLS |
      RACE_ARMOR_CRUDE | RACE_ARMOR_LIGHT| RACE_ARMS_CRUDE | RACE_ARMS_LIGHT |
      RACE_SIZE_SMALL | RACE_TACTICS_CRUDE |
      RACE_DIFF_LVL | RACE_DIFF_SKILL | RACE_DIFF_SPELLS | RACE_DIFF_PETS | RACE_DIFF_ALPHA |
      RACE_BUILD_CRUDE | 
      RACE_SPECIAL_TRAPS | RACE_SPECIAL_FOCI | RACE_SPECIAL_WARDS,
    TRAIT_POISON_RESIST | TRAIT_EXP_DAGGER | TRAIT_EXP_BOW,
    PQ_SMALL | PQ_LIGHT | PQ_BIPED,
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
      RACE_SIZE_BIG |
      RACE_TACTICS_MARTIAL | RACE_TACTICS_SIMPLE |
      RACE_DIFF_LVL | RACE_DIFF_SKILL | RACE_DIFF_GEAR | RACE_DIFF_SPELLS | RACE_DIFF_ALPHA |
      RACE_SPECIAL_FOCI,
      TRAIT_VISION_DARK,
      PQ_LONG_LIMB | PQ_SMALL_HEAD | PQ_BIPED | PQ_LARGE_HANDS | PQ_DENSE_MUSCLE,
      MQ_OBLIVIOUS | MQ_SENTIENT | MQ_AGGRESSIVE | MQ_TERRITORIAL,
      PW_NONE,
      PQ_THICK_SKIN,
      0,
      1.25
  },
  {SPEC_GIANT},
  {SPEC_ARTHROPOD},
  {SPEC_ETHEREAL},
  {SPEC_ROTTING},
  {SPEC_VAMPIRIC},
  {SPEC_CANIFORM},
  {SPEC_RODENT, "Rat", ENT_RAT,
  RACE_SIZE_SMALL | RACE_TACTICS_CRUDE | RACE_DIFF_LVL | RACE_DIFF_ALPHA,
  0, PQ_TINY | PQ_LIGHT | PQ_QUADPED | PQ_TAIL | PQ_TINY_HEAD,
  MQ_SIMPLE | MQ_CAUTIOUS | MQ_ALERT | MQ_HIVE_MIND,
  PQ_TOUGH_TEETH | PQ_TEETH,
  PB_NONE,
  0,
  .25
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

static const define_archetype_t CLASS_DATA[11] = {
  {CLASS_BASE_BARD},
  {CLASS_BASE_CLERIC, 6, ATTR_WIS,ATTR_INT,ATTR_CHAR,
    {[ATTR_WIS]=1,[ATTR_INT]=0.5f,[ATTR_INT]=0.5f, [ATTR_CHAR] =.75f},
    {[AT_HEAL]=5, [AT_DMG]=1},
    {[ACTION_MAGIC]=5,[ACTION_CANTRIP]=4, [ACTION_ATTACK]=3, [ACTION_WEAPON] = 1}
  },
  {CLASS_BASE_DRUID, 6, ATTR_WIS,ATTR_WIS,ATTR_INT,
    {[ATTR_CON]=0.25f, [ATTR_WIS]=0.25f, [ATTR_CHAR]=0.5f, [ATTR_INT]=0.25f},
    {[AT_HEAL]=3, [AT_DMG]=2},
    {[ACTION_MAGIC]=3,[ACTION_CANTRIP]=3, [ACTION_ATTACK]=3, [ACTION_WEAPON] = 1}
  },
  {CLASS_BASE_FIGHTER, 8, ATTR_NONE,ATTR_STR,ATTR_CON,
    {[ATTR_STR]=1,[ATTR_DEX]=1,[ATTR_CON]=.25f},
    {[AT_DMG] = 5},
    {[ACTION_ATTACK] = 5, [ACTION_WEAPON] = 4}
  },
  {CLASS_BASE_MONK},
  {CLASS_BASE_RANGER, 8, ATTR_WIS, ATTR_DEX, ATTR_STR,
      {[ATTR_STR]=0.5f,[ATTR_DEX]=0.5f,[ATTR_CON]=.125f,[ATTR_WIS]=0.125f},
      {[AT_DMG] = 3},
      {[ACTION_ATTACK] = 4, [ACTION_WEAPON] = 4, [ACTION_ITEM] = 2, [ACTION_CANTRIP]=2, [ACTION_MOVE]=3}
  },
  {CLASS_BASE_ROGUE, 6, ATTR_NONE, ATTR_DEX, ATTR_INT,
    {[ATTR_DEX]=0.5f, [ATTR_STR]=0.125,[ATTR_INT]=0.25},
      {[AT_DMG] = 4},
      {[ACTION_ATTACK] = 4, [ACTION_WEAPON] = 4, [ACTION_ITEM] = 3,[ACTION_MOVE]=3}
  },
  {CLASS_BASE_LOCK, 6, ATTR_WIS, ATTR_CHAR, ATTR_CHAR,
    {[ATTR_CON]=0.25f, [ATTR_WIS]=0.25f, [ATTR_CHAR]=0.75f}

  },
  {CLASS_BASE_WIZ, 4, ATTR_INT, ATTR_WIS, ATTR_INT,
    {[ATTR_WIS]=0.5f, [ATTR_CHAR]=0.25f, [ATTR_INT]=0.75f}
  },
  {CLASS_SUB_BERZ, 10, ATTR_NONE, ATTR_STR, ATTR_CON},
  {CLASS_SUB_CHAMP, 10, ATTR_NONE, ATTR_STR, ATTR_CON},
};

static define_race_class_t RACE_CLASS_DEFINE[12][PROF_LABORER] = {
  [__builtin_ctzll(SPEC_GOBLINOID)] = {
    [PROF_SOLDIER] = {PROF_SOLDIER,2,
      {
        { 15, CLASS_BASE_FIGHTER, CLASS_BASE_ROGUE, CLASS_SUB_CHAMP,
          "Brawler","Saboteur","Boss",
          .skills = {
            [SKILL_ARMOR_LEATHER]=600,[SKILL_ARMOR_SHIELD]=400,
          [SKILL_WEAP_MACE]=600, [SKILL_WEAP_SWORD]=400
          },
          .rankups = {
            [SKILL_ARMOR_LEATHER]=400,[SKILL_ARMOR_SHIELD]=400,
          [SKILL_WEAP_MART]=600, [SKILL_WEAP_SWORD]=500,
          }
        },
        { 10, CLASS_BASE_ROGUE, CLASS_SUB_ASSASSIN, CLASS_SUB_CHAMP,
          "Skulker", "Infiltrator", "Saboteur",
           .skills = {
            [SKILL_ARMOR_LEATHER]=400,[SKILL_STEALTH]=600,
          [SKILL_WEAP_DAGGER]=600, [SKILL_WEAP_BOW]=400
          },
          .rankups = {
            [SKILL_ARMOR_LEATHER]=400,[SKILL_STEALTH]=400,
          [SKILL_WEAP_MART]=600, [SKILL_WEAP_DAGGER]=400,
          }
        }
      }
    },
    [PROF_ARCHER] = { PROF_ARCHER,1,
      {
        {
          10, CLASS_BASE_RANGER , CLASS_BASE_ROGUE , CLASS_SUB_SHOOTER,
          "Tracker", "Prowler", "Stinger",
          .skills = {
            [SKILL_ARMOR_PADDED]=600,[SKILL_ANIM]=600,
            [SKILL_WEAP_BOW]=600, [SKILL_WEAP_SWORD]=400
          },
          .rankups = {
            [SKILL_ARMOR_PADDED]=400,[SKILL_STEALTH]=300,
            [SKILL_WEAP_BOW]=400, [SKILL_WEAP_SWORD]=400,
          }
        },
      }
    },
    [PROF_MYSTIC] = { PROF_MYSTIC,1,
      {
        {
          5, CLASS_BASE_DRUID, CLASS_SUB_SHAMAN, CLASS_SUB_CHAMP,
          "Pestcaller","Seer", "Mystic",
          .skills = {
            [SKILL_ARMOR_CLOTH]=400, [SKILL_WEAP_STAFF]=300,
            [SKILL_SPELL_ABJ] = 400, [SKILL_SPELL_TRANS]=600, [SKILL_SPELL_CONJ] = 400,
          },
            .rankups = {
              [SKILL_ARMOR_NATURAL]=200, [SKILL_WEAP_STAFF]=200,
              [SKILL_SPELL_ABJ] = 300, [SKILL_SPELL_TRANS]=400, [SKILL_SPELL_CONJ] = 300,
            }
        },
      }
    },
    [PROF_MAGICIAN] = { PROF_MAGICIAN,1,
      {
        {
          5, CLASS_BASE_LOCK, -1,-1,
          "Filthcaller","\0","\0",
          .skills = {
            [SKILL_ARMOR_CLOTH] = 400,[SKILL_SPELL_EVO] = 600, [SKILL_SPELL_ABJ] = 400, [SKILL_SPELL_TRANS] = 300, [SKILL_SPELL_NECRO] = 600,
          },
          .rankups = {
             [SKILL_ARMOR_CLOTH] = 200 ,[SKILL_SPELL_EVO] = 500, [SKILL_SPELL_ABJ] = 100, [SKILL_SPELL_TRANS] = 100, [SKILL_SPELL_NECRO] = 600,
          }
        },
      }
    },
    [PROF_HEALER] = {PROF_HEALER, 1,
      {
        {
          5, CLASS_BASE_CLERIC, CLASS_BASE_LOCK, CLASS_SUB_HEX,
          "Giver", "Witch-doctor", "Shadow Priest",
          .skills = {
            [SKILL_ARMOR_CLOTH] = 400, [SKILL_SPELL_ENCH] = 400, [SKILL_SPELL_ABJ] = 600, [SKILL_SPELL_NECRO] = 800,
          },
          .rankups = {
            [SKILL_ARMOR_CLOTH] = 200 ,[SKILL_SPELL_ENCH] = 300, [SKILL_SPELL_ABJ] = 400, [SKILL_SPELL_NECRO] = 600,
          }

        }
      }
    },
  },
  /*
       },
       [__builtin_ctzll( SPEC_ORC )] = {
       [PROF_FIGHTER] = {
       PROF_FIGHTER, 25, CLASS_BASE_FIGHTER,CLASS_SUB_CHAMP, CLASS_SUB_CHAMP,
       "Grunt","Champion","Warlord"
       },
       [PROF_ROGUE] = {
       PROF_ROGUE, 10, CLASS_BASE_ROGUE, CLASS_SUB_ASSASSIN, CLASS_SUB_CHAMP,
       "Cutthroat", "Infiltrator", "Saboteur"},
       [PROF_RANGER] = {
       PROF_RANGER, 10, CLASS_BASE_RANGER , CLASS_BASE_FIGHTER , CLASS_SUB_SHOOTER,
       "Stalker", "Slayer", "Hunter"},
       [PROF_WARLOCK] = {
       PROF_WARLOCK,1, CLASS_BASE_LOCK, -1,-1,
       "Doomsayer","\0","\0"
       },
       [PROF_CLERIC] = {
       PROF_CLERIC,5, CLASS_BASE_CLERIC, CLASS_SUB_SHAMAN, CLASS_SUB_HEX,
       "Priest", "Shaman", "Shadow Priest"
       },
       [PROF_SPEC_MELEE] = {
       PROF_SPEC_MELEE,12, CLASS_BASE_FIGHTER, CLASS_SUB_BERZ, CLASS_SUB_CHAMP,
       "Rager", "Berserker", "Bloodrager"
       }
       },
       */
};

static const mob_define_t MONSTER_MASH[ENT_DONE] = {
  {ENT_PERSON,
    .race = SPEC_HUMAN
  },
  {ENT_GOBLIN,
      MOB_SPAWN_CHALLENGE | MOB_SPAWN_CAMP | MOB_SPAWN_PATROL |
      MOB_MOD_ENLARGE | MOB_MOD_WEAPON | MOB_MOD_ARMOR |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_FREQ_COMMON|
      MOB_THEME_PRIMITIVE |
      MOB_GROUPING_TROOP | MOB_GROUPING_CREW | MOB_GROUPING_SQUAD | MOB_GROUPING_WARBAND,
      SPEC_GOBLINOID,
      10,
      1,
      SOC_PRIMITIVE
  },
  {ENT_ORC,
      MOB_SPAWN_CHALLENGE | MOB_SPAWN_CAMP | MOB_SPAWN_PATROL |
      MOB_MOD_ENLARGE | MOB_MOD_WEAPON | MOB_MOD_ARMOR |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_FREQ_UNCOMMON |
      MOB_THEME_MARTIAL |
      MOB_GROUPING_PAIRS | MOB_GROUPING_TROOP | MOB_GROUPING_CREW | MOB_GROUPING_SQUAD | MOB_GROUPING_WARBAND,
      SPEC_ORC,
      30,
      2,
      SOC_MARTIAL
  },
  {ENT_OGRE},
  {ENT_ORC_FIGHTER},
  {ENT_BERSERKER},
  {ENT_HOBGOBLIN,
      MOB_SPAWN_CHALLENGE | MOB_SPAWN_CAMP | MOB_SPAWN_PATROL |
      MOB_MOD_ENLARGE | MOB_MOD_WEAPON | MOB_MOD_ARMOR |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_FREQ_RARE|
      MOB_THEME_MARTIAL |
      MOB_GROUPING_PAIRS | MOB_GROUPING_TROOP | MOB_GROUPING_CREW | MOB_GROUPING_SQUAD | MOB_GROUPING_WARBAND,
      SPEC_GOBLINOID,
      540,
      2,
      SOC_MARTIAL
  },
  {ENT_OROG},
  {ENT_SCORPION,
    MOB_SPAWN_LAIR |
      MOB_MOD_ENLARGE |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_THEME_CRITTER |
      MOB_FREQ_RARE |
      MOB_GROUPING_SOLO | MOB_GROUPING_SWARM,
    SPEC_ARTHROPOD,
    9,
    .75
  },
  {ENT_SPIDER,
    MOB_SPAWN_LAIR |
      MOB_MOD_ENLARGE |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_THEME_CRITTER |
      MOB_FREQ_COMMON |
      MOB_GROUPING_SOLO | MOB_GROUPING_PAIRS | MOB_GROUPING_SWARM,
    SPEC_ARTHROPOD,
    5,
    .5
  },
  {ENT_TROLL,
      MOB_SPAWN_LAIR |
      MOB_MOD_ENLARGE |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST|
      MOB_THEME_PRIMITIVE |
      MOB_GROUPING_SOLO | MOB_GROUPING_PAIRS,
      SPEC_CANIFORM,
      500,
      3,
      SOC_INSTINCTIVE
  },
  {ENT_TROLL_CAVE,
    MOB_SPAWN_LAIR |
      MOB_MOD_ENLARGE |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE |
      MOB_THEME_PRIMITIVE |
      MOB_FREQ_RARE|
      MOB_GROUPING_SOLO,
      SPEC_CANIFORM,
    600,
    3.33,
    SOC_INSTINCTIVE
  },
  {ENT_BEAR,
      MOB_SPAWN_LAIR |
      MOB_MOD_ENLARGE |
      MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_GROUPING_SOLO | MOB_GROUPING_PAIRS | MOB_GROUPING_SQUAD,
      SPEC_CANIFORM,
      75,
      2.22,
      SOC_FAMILY
  },
  {ENT_WOLF,
      MOB_SPAWN_LAIR |
      MOB_MOD_ENLARGE |
      MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_GROUPING_SOLO | MOB_GROUPING_SQUAD,
      SPEC_CANIFORM,
      40,
      1,
      SOC_FERAL
  },
  {ENT_RAT,
      MOB_MOD_ENLARGE |
      MOB_LOC_DUNGEON | MOB_LOC_CAVE | MOB_LOC_FOREST |
      MOB_THEME_CRITTER |
      MOB_FREQ_COMMON |
      MOB_GROUPING_SOLO | MOB_GROUPING_PAIRS | MOB_GROUPING_TROOP | MOB_GROUPING_CREW | MOB_GROUPING_SQUAD | MOB_GROUPING_SWARM,
      SPEC_RODENT,
      8,
      .25,
      SOC_HIVE
  },

};

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

static define_ability_class_t CLASS_ABILITIES[ABILITY_DONE]={
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
    CLASS_SUB_SHAMAN,
    0,3,8,
    SPEC_HUMAN// | SPEC_ELF | SPEC_GOBLINOID | SPEC_ORC
  },
  {ABILITY_SACRED_FLAME, true, true,
    CLASS_BASE_CLERIC,
    CLASS_BASE_CLERIC,
    0,3,5,
    SPEC_HUMAN | SPEC_ELF
  },
  {ABILITY_STARRY_WISP, true, true,
    CLASS_BASE_BARD | CLASS_BASE_DRUID,
    CLASS_SUB_SHAMAN,
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
    CLASS_BASE_DRUID | CLASS_SUB_SHAMAN,
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
    CLASS_SUB_BERZ,
    CLASS_SUB_BERZ,
    0,5,12,
    SPEC_GOBLINOID | SPEC_ORC | SPEC_ARCHAIN
  },

};

static skill_relation_t SKILLUP_RELATION[SKILL_DONE] = {
  [SKILL_WEAP_NONE] = {SKILL_WEAP_NONE,
    {
      [MAG_MINOR] = SKILL_LVL
    }
  },
  [SKILL_WEAP_SIMP] = {SKILL_WEAP_SIMP,
    {
      [MAG_MINOR] = SKILL_LVL
    }
  },
  [SKILL_WEAP_MACE] = {SKILL_WEAP_MACE,
    {
      [MAG_NOMINAL] = SKILL_LVL
    }
  },
  [SKILL_WEAP_SWORD] = {SKILL_WEAP_SWORD,
    {
      [MAG_NOMINAL] = SKILL_LVL
    }
  },
  [SKILL_WEAP_AXE] = {SKILL_WEAP_AXE,
    {
      [MAG_NOMINAL] = SKILL_LVL
    }
  },
  [SKILL_WEAP_DAGGER] = {SKILL_WEAP_DAGGER,
    {
      [MAG_NOMINAL] = SKILL_LVL
    }
  },
  [SKILL_WEAP_BOW] = {SKILL_WEAP_BOW,
    {
      [MAG_NOMINAL] = SKILL_LVL
    }
  },
  [SKILL_WEAP_PICK] = {SKILL_WEAP_PICK,
    {
      [MAG_NOMINAL] = SKILL_LVL
    }
  },
  [SKILL_WEAP_MART] = {SKILL_WEAP_MART,
    {
      [MAG_MODEST] = SKILL_LVL
    }
  },
  [SKILL_ARMOR_NATURAL] =  {SKILL_ARMOR_NATURAL,
    {
      [MAG_MINOR] = SKILL_LVL
    }
  },
  [SKILL_ARMOR_PADDED] = { SKILL_ARMOR_PADDED,
    {
      [MAG_MODEST] = SKILL_LVL
    }
  },
  [SKILL_ARMOR_LEATHER] = { SKILL_ARMOR_LEATHER,
    {
      [MAG_MODEST] = SKILL_LVL
    }
  },
  [SKILL_ARMOR_CHAIN] = { SKILL_ARMOR_CHAIN,
    {
      [MAG_MODEST] = SKILL_LVL
    }
  },
  [SKILL_ARMOR_PLATE] = { SKILL_ARMOR_PLATE,
    {
      [MAG_MODEST] = SKILL_LVL
    }
  },
  [SKILL_ARMOR_SHIELD] = { SKILL_ARMOR_SHIELD,
    {
      [MAG_NOMINAL] = SKILL_LVL
    }
  },
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
  [SKILL_WEAP_SWORD]        = "Swords",
  [SKILL_WEAP_AXE]          = "Axes",
  [SKILL_WEAP_DAGGER]       = "Daggers",
  [SKILL_WEAP_BOW]          = "Bows",
  [SKILL_WEAP_PICK]         = "Picks",
  [SKILL_WEAP_STAFF]        = "Staves",
  [SKILL_WEAP_NONE]         = "Unarmed",

  [SKILL_WOOD]              = "Woodworking",
};
static define_skill_rank_t SKILL_RANKS[SR_DONE] = {
  [SR_NONE]      = { SR_NONE,      0,  0  },
  [SR_NOVICE]    = { SR_NOVICE,    3,  5 },
  [SR_SKILLED]   = { SR_SKILLED,   6,  10 },
  [SR_PROFIC]    = { SR_PROFIC,    10, 25 },
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
#endif
