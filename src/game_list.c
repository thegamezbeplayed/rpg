#include "game_process.h"

ability_t ABILITY_LIST[SCHOOL_DONE][50] = {
  [SCHOOL_ABJ] = {
    {ABILITY_RESISTANCE,
      .skills = SKILL_SPELL_ABJ, .image_id = SPELL_SHIELD
    },
    {ABILITY_CURE_WOUNDS, AT_HEAL, ACTION_MAGIC, DMG_TRUE,
      STAT_ENERGY, DES_SEL_TAR, 15, STAT_HEALTH, ATTR_NONE, ATTR_WIS,
      .vals = {
        [VAL_HIT]     = 50,
        [VAL_DMG]     = 6,
        [VAL_DMG_DIE] = 2,
        [VAL_SCORE]   = 15,
        [VAL_REACH]   = 1,
        [VAL_DRAIN]   = 6,
      },
      .image_id = SPELL_PIERCE,
      .skills = SKILL_SPELL_ABJ
    },
  },

  [SCHOOL_CONJ] = {
    {ABILITY_CALL_LIGHTNING, AT_DMG, ACTION_MAGIC, DMG_LIGHTNING, 
      STAT_ENERGY, DES_AREA, 10,
      STAT_HEALTH, ATTR_DEX, ATTR_WIS,
      .vals = {
        [VAL_HIT]     = 17,
        [VAL_DMG]     = 4,
        [VAL_DMG_DIE] = 4,
        [VAL_SCORE]   = 10,
        [VAL_REACH]   = 4,
        [VAL_SIZE]    = 1,
        [VAL_DRAIN]   = 14,
      },
      .image_id = SPELL_STORM
    },

  },

  [SCHOOL_DIV] = {

  },

  [SCHOOL_ENCH] = {
    {ABILITY_DISSONANT_WHISPERS, AT_DMG, ACTION_MAGIC, DMG_PSYCHIC,
      STAT_ENERGY, DES_SEL_TAR, 15,
      STAT_HEALTH, ATTR_WIS, ATTR_CHAR,
      .skills = SKILL_SPELL_ENCH, .image_id = SPELL_EYE,
      .vals = {
        [VAL_HIT]     = 20,
        [VAL_DMG]     = 6,
        [VAL_DMG_DIE] = 3,
        [VAL_SCORE]   = 15,
        [VAL_REACH]   = 2,
        [VAL_DRAIN]   = 8,
      }
    },
    
  },

  [SCHOOL_EVO] = {
    {ABILITY_ACID_SPLASH, AT_DMG, ACTION_MAGIC, DMG_ACID, STAT_ENERGY,
    DES_AREA, 11, STAT_HEALTH, ATTR_DEX, ATTR_WIS,
    .skills = SKILL_SPELL_EVO, .image_id = SPELL_BUBBLES,
     .vals = {
        [VAL_HIT]       = 19,
        [VAL_DMG]       = 6,
        [VAL_DMG_DIE]   = 1,
        [VAL_DMG_BONUS] = 1,
        [VAL_SCORE]     = 11,
        [VAL_REACH]     = 2,
        [VAL_SIZE]      = 1,
        [VAL_DRAIN]     = 12,
     },
    },
    {ABILITY_BLAST_WAVE, AT_DMG, ACTION_MAGIC, DMG_FIRE, STAT_ENERGY,
      DES_ORIGIN, 12, STAT_HEALTH, ATTR_DEX, ATTR_INT,
      .skills = SKILL_SPELL_EVO, .image_id = SPELL_WISP,
      .vals = {
        [VAL_HIT]       = 16,
        [VAL_DMG]       = 8,
        [VAL_DMG_DIE]   = 1,
        [VAL_SCORE]     = 11,
        [VAL_REACH]     = 2,
        [VAL_SIZE]      = 2,
        [VAL_DRAIN]     = 14,
      },
    },
    {ABILITY_MAGIC_MISSLE, AT_DMG, ACTION_MAGIC, DMG_FORCE, STAT_ENERGY, DES_MULTI_TAR, 20,4,99,1,4,1,3,STAT_HEALTH, ATTR_NONE, ATTR_NONE, .chain_id = ABILITY_NONE, .skills = SKILL_SPELL_EVO,
      .image_id = SPELL_MISSILE,
      .vals = {
        [VAL_HIT]       = 99,
        [VAL_DMG]       = 4,
        [VAL_DMG_DIE]   = 1,
        [VAL_DMG_BONUS] = 1,
        [VAL_SCORE]     = 25,
        [VAL_REACH]     = 3,
        [VAL_DRAIN]     = 4,
        [VAL_QUANT]     = 3
      }
    },
    {ABILITY_ELDRITCH_BLAST, AT_DMG,ACTION_MAGIC, DMG_FORCE,
      STAT_ENERGY, DES_CONE, 30, 
      STAT_HEALTH, ATTR_NONE, ATTR_CHAR,
      .skills = SKILL_SPELL_EVO, .image_id = SPELL_FLARE,
      .vals = {
        [VAL_HIT]       = 12,
        [VAL_DMG]       = 4,
        [VAL_DMG_DIE]   = 2,
        [VAL_DMG_BONUS] = 3,
        [VAL_SCORE]     = 35,
        [VAL_REACH]     = 2,
        [VAL_SIZE]      = 2,
        [VAL_DRAIN]     = 14,
      }
    },
    {ABILITY_GUIDING_BOLT, AT_DMG, ACTION_MAGIC, DMG_RADIANT, 
      STAT_ENERGY, DES_SEL_TAR, 20,
      STAT_HEALTH, ATTR_NONE, ATTR_WIS,
      .skills = SKILL_SPELL_EVO, .image_id = SPELL_MISSILE,
      .vals = {
        [VAL_HIT]       = 21,
        [VAL_DMG]       = 6,
        [VAL_DMG_DIE]   = 1,
        [VAL_SCORE]     = 25,
        [VAL_REACH]     = 2,
        [VAL_DRAIN]     = 4,
      }
    },
    {ABILITY_SACRED_FLAME, AT_DMG,ACTION_MAGIC, DMG_RADIANT,
      STAT_ENERGY, DES_SEL_TAR, 25,
      STAT_HEALTH, ATTR_DEX, ATTR_WIS,
      .skills = SKILL_SPELL_EVO, .image_id = SPELL_FLARE,
      .vals = {
        [VAL_HIT]       = 20,
        [VAL_DMG]       = 8,
        [VAL_DMG_DIE]   = 1,
        [VAL_SCORE]     = 25,
        [VAL_REACH]     = 3,
        [VAL_DRAIN]     = 8,
      }
    },
    {ABILITY_STARRY_WISP, AT_DMG,ACTION_MAGIC,DMG_RADIANT,
      STAT_ENERGY, DES_SEL_TAR, 25,
      STAT_HEALTH, ATTR_NONE, ATTR_WIS,
      .skills = SKILL_SPELL_EVO, .image_id = SPELL_WISP,
      .vals = {
        [VAL_HIT]       = 17,
        [VAL_DMG]       = 8,
        [VAL_DMG_DIE]   = 1,
        [VAL_SCORE]     = 25,
        [VAL_REACH]     = 5,
        [VAL_DRAIN]     = 6,
      }
    },
    {ABILITY_FIRE_BOLT, AT_DMG, ACTION_MAGIC, DMG_FIRE,
      STAT_ENERGY, DES_SEL_TAR, 20, STAT_HEALTH,
      ATTR_NONE, ATTR_INT,
      .skills = SKILL_SPELL_EVO, .image_id = SPELL_MISSILE,
      .vals = {
        [VAL_HIT]       = 14,
        [VAL_DMG]       = 12,
        [VAL_DMG_DIE]   = 1,
        [VAL_DMG_BONUS] = 1,
        [VAL_SCORE]     = 25,
        [VAL_REACH]     = 4,
        [VAL_DRAIN]     = 8,
      }
    },
    {ABILITY_FIRE_BLAST, AT_DMG, ACTION_MAGIC, DMG_FIRE,
      STAT_ENERGY, DES_CONE, 7, STAT_HEALTH, ATTR_DEX, ATTR_INT, 
      .skills = SKILL_SPELL_EVO, .image_id = SPELL_SLASH,
      .vals = {
        [VAL_HIT]     = 18,
        [VAL_DMG]     = 18,
        [VAL_DMG_DIE] = 1,
        [VAL_SCORE]   = 7,
        [VAL_REACH]   = 3,
        [VAL_SIZE]    = 1,
        [VAL_DRAIN]   = 12,
      },
    },
    {ABILITY_FLAME_STRIKE, AT_DMG, ACTION_MAGIC, DMG_FIRE,
      STAT_ENERGY, DES_AREA, 10, STAT_HEALTH, ATTR_DEX, ATTR_INT, 
      .skills = SKILL_SPELL_EVO, .image_id = SPELL_PYRE,
      .vals = {
        [VAL_HIT]     = 17,
        [VAL_DMG]     = 16,
        [VAL_DMG_DIE] = 1,
        [VAL_SCORE]   = 10,
        [VAL_REACH]   = 3,
        [VAL_SIZE]    = 2,
        [VAL_DRAIN]   = 20,
      },
    },
    {ABILITY_FROST_BITE, AT_DMG, ACTION_MAGIC, DMG_COLD,
      STAT_ENERGY, DES_SEL_TAR, 12, 
      STAT_HEALTH, ATTR_CON, ATTR_INT,
      .skills = SKILL_SPELL_EVO, .image_id = SPELL_GLIMMER,
      .vals = {
        [VAL_HIT]     = 20,
        [VAL_DMG]     = 6,
        [VAL_DMG_DIE] = 1,
        [VAL_SCORE]   = 12,
        [VAL_REACH]   = 3,
        [VAL_DRAIN]   = 8,
      },
      .affects = AFFECT_CHILL

    },
    {ABILITY_THUNDER_WAVE, AT_DMG, ACTION_MAGIC, DMG_THUNDER, STAT_ENERGY,
      DES_ORIGIN, 15, STAT_HEALTH, ATTR_CON, ATTR_CHAR,
      .skills = SKILL_SPELL_EVO, .image_id = SPELL_PYRE,
        .vals = {
          [VAL_HIT]     = 21,
          [VAL_DMG]     = 8,
          [VAL_DMG_DIE] = 1,
          [VAL_SCORE]   = 15,
          [VAL_REACH]   = 2,
          [VAL_SIZE]    = 1,
          [VAL_DRAIN]   = 15,
        },
        .affects = AFFECT_BOOM
    },
  },
  [SCHOOL_ILL] = {
  },
  [SCHOOL_NECRO] = {
    {ABILITY_CHILL_TOUCH, AT_DMG, ACTION_MAGIC, DMG_NECROTIC,
      STAT_ENERGY, DES_IMBUE, 24, 
      STAT_HEALTH, ATTR_NONE, ATTR_NONE,
      .vals = {
        [VAL_DMG]     = 8,
        [VAL_DMG_DIE] = 1,
        [VAL_SCORE]   = 25,
        [VAL_DRAIN]   = 6,
      },
      .skills = SKILL_SPELL_NECRO, .image_id = SPELL_BONE,
      .affects = AFFECT_NECRO | AFFECT_CHILL
    },
    {ABILITY_POISON_SPRAY, AT_DMG,ACTION_MAGIC, DMG_POISON,
      STAT_ENERGY, DES_SEL_TAR, 25, 
      STAT_HEALTH, ATTR_CON, ATTR_NONE,
      .vals = {
        [VAL_HIT]     = 17,
        [VAL_DMG]     = 12,
        [VAL_DMG_DIE] = 1,
        [VAL_SCORE]   = 25,
        [VAL_REACH]   = 2,
        [VAL_DRAIN]   = 8,
      },
      .skills = SKILL_SPELL_NECRO, .image_id = SPELL_BUBBLE},
  },
  [SCHOOL_TRANS] = {
  {ABILITY_MAGIC_STONE,.skills = SKILL_SPELL_TRANS, .image_id = SPELL_ROCK},
  },
  [SCHOOL_ARMOR] = {
    {ABILITY_ARMOR_SAVE, AT_SAVE, ACTION_SAVE, .mod = ATTR_DEX,
      .chain_id = ABILITY_ARMOR_DR, .save_fn = EntAbilitySave},
    {ABILITY_ARMOR_DR, AT_DR, ACTION_SAVE,.save_fn = EntAbilityReduce},
  },
  [SCHOOL_WEAP] = {
{ABILITY_WEAP_BLUDGEON, AT_DMG,ACTION_WEAPON,
    DMG_BLUNT, STAT_STAMINA, DES_NONE, 25,
    STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_CHOP,AT_DMG,ACTION_WEAPON,
    DMG_SLASH, STAT_STAMINA, DES_NONE, 25,
    STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_STAB,AT_DMG,ACTION_WEAPON,
    DMG_PIERCE, STAT_STAMINA, DES_NONE, 25,
    STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_SLASH,AT_DMG,ACTION_WEAPON,
    DMG_SLASH, STAT_STAMINA, DES_NONE, 25,
    STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_PIERCE,AT_DMG,ACTION_WEAPON,
    DMG_PIERCE, STAT_STAMINA, DES_NONE, 25, 
    STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_RANGE_PIERCE,AT_DMG,ACTION_WEAPON,
    DMG_PIERCE, STAT_STAMINA, DES_NONE, 27,
      STAT_HEALTH,ATTR_NONE, ATTR_DEX},
  },
  [SCHOOL_NATURAL] = {
    {ABILITY_PUNCH, AT_DMG, ACTION_ATTACK, DMG_BLUNT,
      STAT_STAMINA, DES_NONE, 25,
      STAT_HEALTH, ATTR_NONE, ATTR_STR,
      .skills = SKILL_WEAP_NONE,
      .vals = {
        [VAL_DRAIN]   = 3,
        [VAL_HIT]     = 19,
        [VAL_DMG]     = 4,
        [VAL_DMG_DIE] = 1,
        [VAL_REACH]   = 1,
        [VAL_SCORE]   = 10
      }
    },
    {ABILITY_RAM,AT_DMG,ACTION_ATTACK, DMG_BLUNT,
      STAT_STAMINA, DES_NONE, 25, 
      STAT_HEALTH, ATTR_NONE, ATTR_STR,
      .vals = {
        [VAL_DRAIN]     = 2,
        [VAL_HIT]       = 16,
        [VAL_DMG]       = 4,
        [VAL_DMG_DIE]   = 2,
        [VAL_DMG_BONUS] = 1,
        [VAL_REACH]     = 1,
        [VAL_SCORE]     = 25
      },
      .skills[0] = SKILL_WEAP_NONE
    },
    {ABILITY_BITE, AT_DMG,ACTION_ATTACK, DMG_PIERCE,
      STAT_STAMINA, DES_NONE, 20,
      STAT_HEALTH, ATTR_NONE, ATTR_STR,
      .vals = {
        [VAL_DRAIN]     = 3,
        [VAL_HIT]       = 14,
        [VAL_DMG]       = 4,
        [VAL_DMG_DIE]   = 1,
        [VAL_DMG_BONUS] = 4,
        [VAL_REACH]     = 1,
        [VAL_SCORE]     = 25
      },
      .skills = SKILL_WEAP_NONE,
    },
    {ABILITY_CHEW, AT_DMG,ACTION_ATTACK, DMG_PIERCE,
      STAT_STAMINA, DES_NONE, 25, 
      STAT_ARMOR, ATTR_NONE, ATTR_NONE, ABILITY_GNAW,
      .vals = {
        [VAL_DRAIN]     = 1,
        [VAL_HIT]       = 8,
        [VAL_DMG]       = 2,
        [VAL_DMG_DIE]   = 1,
        [VAL_DMG_BONUS] = 1,
        [VAL_REACH]     = 1,
        [VAL_SCORE]     = 25
      },
      .skills = SKILL_WEAP_NONE
    },
    {ABILITY_GNAW, AT_DMG, ACTION_ATTACK, DMG_PIERCE,
      STAT_STAMINA, DES_NONE, 25,
      STAT_HEALTH, ATTR_NONE, ATTR_STR,
      .vals = {
        [VAL_DRAIN]     = 1,
        [VAL_HIT]       = 21,
        [VAL_DMG]       = 2,
        [VAL_DMG_DIE]   = 1,
        [VAL_REACH]     = 1,
        [VAL_SCORE]     = 25
      },
      .skills = SKILL_WEAP_NONE
    },
    {ABILITY_CLAW,AT_DMG,ACTION_ATTACK, DMG_SLASH,
      STAT_STAMINA, DES_NONE, 50,
      STAT_HEALTH, ATTR_NONE, ATTR_STR,
      .vals = {
        [VAL_DRAIN]     = 3,
        [VAL_HIT]       = 19,
        [VAL_DMG]       = 2,
        [VAL_DMG_DIE]   = 3,
        [VAL_DMG_BONUS] = 1,
        [VAL_REACH]     = 1,
        [VAL_SCORE]     = 50
      },
      .skills = SKILL_WEAP_NONE},
    {ABILITY_SWIPE, AT_DMG,ACTION_ATTACK, DMG_SLASH,
      STAT_STAMINA, DES_ORIGIN, 50,
      STAT_HEALTH, ATTR_STR,
      .vals = {
        [VAL_DRAIN]     = 6,
        [VAL_HIT]       = 17,
        [VAL_DMG]       = 6,
        [VAL_DMG_DIE]   = 2,
        [VAL_DMG_BONUS] = 3,
        [VAL_REACH]     = 1,
        [VAL_SIZE]      = 1,
        [VAL_SCORE]     = 50
      },
      .skills = SKILL_WEAP_NONE},
    {ABILITY_BITE_POISON, AT_DMG, ACTION_ATTACK, DMG_PIERCE,
      STAT_STAMINA, DES_NONE,
      STAT_HEALTH, ATTR_NONE, ATTR_STR, ABILITY_POISON, 
      .vals = {
        [VAL_DRAIN]     = 3,
        [VAL_HIT]       = 14,
        [VAL_DMG]       = 4,
        [VAL_DMG_DIE]   = 1,
        [VAL_DMG_BONUS] = 1,
        [VAL_REACH]     = 1,
        [VAL_SCORE]     = 25
      },

      .skills = SKILL_WEAP_NONE,
    },
    {ABILITY_THROW_ROCK, AT_DMG, ACTION_ATTACK, DMG_BLUNT,
      STAT_STAMINA, DES_SEL_TAR, 20,
      STAT_HEALTH, ATTR_DEX, ATTR_STR,
      .skills = SKILL_WEAP_NONE, .image_id = SPELL_ROCK,
      .vals = {
        [VAL_HIT] = 12, [VAL_DMG] = 4, [VAL_DMG_DIE] = 1,
        [VAL_REACH] = 2, [VAL_DRAIN] = 3
      }
    },
  },
  [SCHOOL_TOOL] = {
    {ABILITY_TOOL_SKIN, AT_SKILL, ACTION_INTERACT,
      .targeting = DES_REQ, ATTR_NONE, ATTR_DEX,
      .skills = SKILL_SKIN, .use_fn = AbilityInteract,
      .params = {[PARAM_RESOURCE] = RES_HIDE | RES_FUR | RES_MEAT},
      .req = CTX_METHOD_EXTRACT | CTX_HAS_RESOURCE | CTX_TAR_ENV,
      .vals = {
        [VAL_DMG] = 6, [VAL_DMG_DIE] = 2, [VAL_HIT] = 12,
        [VAL_DRAIN] = 1, [VAL_REACH] = 1, [VAL_DMG_BONUS] = 1
      }
    },
    {ABILITY_TOOL_MINE, AT_SKILL, ACTION_INTERACT,
      .targeting = DES_REQ, ATTR_NONE, ATTR_STR,
      .skills = SKILL_STONE, .use_fn = AbilityInteract,
      .req = CTX_METHOD_EXTRACT | CTX_HAS_RESOURCE | CTX_TAR_ENV,
      .params = {[PARAM_RESOURCE] = RES_STONE},
      .vals = {
        [VAL_DMG] = 6, [VAL_DMG_DIE] = 4, [VAL_HIT] = 12,
        [VAL_DRAIN] = 2, [VAL_REACH] = 1, [VAL_DMG_BONUS] = 2
      }
    },
    {ABILITY_TOOL_SMELT, AT_SKILL, ACTION_INTERACT,
      .targeting = DES_INTERFACE, ATTR_NONE, ATTR_WIS,
      .skills = SKILL_SMELT, .use_fn = AbilityProcess,
      .req = CTX_METHOD_PROCESS | CTX_HAS_RESOURCE,
      .params = {[PARAM_RESOURCE] = RES_STONE},
      .vals = {
        [VAL_DRAIN] = 2,
      }
    },
    {ABILITY_TOOL_FELL, AT_SKILL, ACTION_INTERACT,
      .targeting = DES_REQ, ATTR_NONE, ATTR_STR,
      .skills = SKILL_WOOD, .use_fn = AbilityInteract,
      .req = CTX_METHOD_EXTRACT | CTX_HAS_RESOURCE | CTX_TAR_ENV,
      .params = {[PARAM_RESOURCE] = RES_WOOD},
      .vals = {
        [VAL_DMG] = 4, [VAL_DMG_DIE] = 2, [VAL_HIT] = 14,
        [VAL_DRAIN] = 4, [VAL_REACH] = 1, [VAL_DMG_BONUS] = 1
      }
    }
  },
  [SCHOOL_ITEM] = {
    {ABILITY_ITEM_HEAL, AT_HEAL, ACTION_ITEM,
      DMG_RADIANT, STAT_NONE, DES_SELF, 10,
      STAT_HEALTH, ATTR_NONE, ATTR_NONE,
      .skills=SKILL_ALCH, .use_fn = AbilityConsume,
      .vals = {
        [VAL_SCORE] = 10, [VAL_DMG] = 4, [VAL_DMG_DIE] = 2,
        [VAL_DMG_BONUS] = 2,
      }
    },
    {ABILITY_ITEM_RESTORE, AT_HEAL, ACTION_ITEM,
      DMG_RADIANT, STAT_NONE, DES_SELF, 10,
      STAT_ENERGY, ATTR_NONE, ATTR_NONE,
      .skills=SKILL_ALCH, .use_fn = AbilityConsume,
      .vals = {
        [VAL_SCORE] = 10, [VAL_DMG] = 20, [VAL_DMG_DIE] = 1,
      }
    },
    {ABILITY_ITEM_SKILL, AT_KNOWLEDGE, ACTION_ITEM,
      .skills=SKILL_LING, .use_fn = AbilityGrantExp
    },
    {ABILITY_ITEM_TOME, AT_KNOWLEDGE, ACTION_ITEM,
      .skills=SKILL_ARCANA, .use_fn = AbilityLearn 
    }, 
    {ABILITY_ITEM_SCROLL, AT_KNOWLEDGE, ACTION_ITEM,
    },
  },
  [SCHOOL_STATUS] = {
    {ABILITY_BLEED, AT_DMG, ACTION_NONE,
      DMG_BLEED, STAT_STAMINA, DES_NONE, 0,
      STAT_HEALTH, ATTR_CON,ATTR_STR,
      .vals = {
        [VAL_DMG] = 6, [VAL_DMG_DIE] = 1, [VAL_DMG_BONUS] = 2,
        [VAL_HIT] = 4,
      }
    },
    {ABILITY_POISON, AT_DMG, ACTION_NONE, DMG_POISON,
      STAT_NONE, DES_NONE, 25,
      STAT_HEALTH, ATTR_CON, ATTR_NONE,
      .skills = SKILL_POISON, .image_id = SPELL_BUBBLE,
      .vals = {
        [VAL_HIT] = 9, [VAL_DMG] = 4, [VAL_DMG_DIE] = 1,
        [VAL_DMG_BONUS] = 1
      }
    },
  },
  [SCHOOL_TECHNIQ] = {
    {ABILITY_REND, AT_DMG, ACTION_ATTACK, DMG_SLASH,
      STAT_STAMINA, DES_NONE, 25,
      STAT_HEALTH, ATTR_NONE,ATTR_STR,ABILITY_BLEED,
      .vals = {
        [VAL_HIT] = 12, [VAL_DMG] = 10, [VAL_DMG_DIE] = 1,
        [VAL_DMG_BONUS] = 1, [VAL_DRAIN] = 8, [VAL_REACH] = 1
      },
      .image_id = SPELL_SLASH
    },
    {ABILITY_HAMSTRING},
    {ABILITY_RAGE, AT_DMG,ACTION_ATTACK,DMG_TRUE,
      STAT_RAGE, DES_IMBUE, 10, 
      STAT_HEALTH, ATTR_NONE, ATTR_CON,
      .vals = {
        [VAL_HIT] = 20, [VAL_DMG] = 2, [VAL_DMG_DIE] = 1,
        [VAL_DRAIN] = 2, [VAL_REACH] = 1, [VAL_DMG_BONUS] = 1
      } 
    },
    {ABILITY_DASH, AT_MOVE, ACTION_MOVE, 0,
      STAT_STAMINA, DES_SEL_LOC, 5, 
      STAT_NONE, ATTR_NONE, ATTR_DEX,
      .skills = SKILL_ACRO, .image_id = SPELL_DASH,
      .vals = {
        [VAL_DRAIN] = 5, [VAL_REACH] = 4, [VAL_DMG] = 2,
        [VAL_DMG_DIE] = 1
      },
      .cat = ACT_BONUS},
  }
};


