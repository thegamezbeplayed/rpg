#include <raylib.h>
#include "game_tools.h"
#include "game_process.h"
#include "game_gen.h"
#include <stdio.h>

#include "game_helpers.h"
ent_t* ParamReadEnt(const param_t* o) {
    assert(o->type_id == DATA_ENTITY);
    return (ent_t*)o->data;
}
env_t* ParamReadEnv(const param_t* o){
    assert(o->type_id == DATA_ENV);
    return (env_t*)o->data;
}

map_cell_t* ParamReadMapCell(const param_t* o){
    assert(o->type_id == DATA_MAP_CELL);
    return (map_cell_t*)o->data;
}

local_ctx_t* ParamReadCtx(const param_t* o){
    assert(o->type_id == DATA_LOCAL_CTX);
    return (local_ctx_t*)o->data;
}

faction_t* FACTIONS[MAX_FACTIONS];
int NUM_FACTIONS = 0;

define_need_req_t NEEDS_REQ[N_DONE][8]= {
  [N_HUNGER] = {
    {N_HUNGER,
      PQ_ETHEREAL | PQ_NO_HEAD,
      MQ_ABSENT,
      NREQ_NONE
    },
    {N_HUNGER,
      PQ_TINY | PQ_LIGHT,
      MQ_HIVE_MIND,
      NREQ_MIN,
    },
     {N_HUNGER,
      PQ_SMALL,
      MQ_SIMPLE,
      NREQ_LOW,
    },
    {N_HUNGER,
      PQ_GILLED | PQ_SHORT | PQ_SHORT_LIMB,
      MQ_SENTIENT,
      NREQ_AVG,
    },
     {N_HUNGER,
      PQ_TALL | PQ_WIDE |  PQ_LONG_LIMB| PQ_LARGE | PQ_DENSE_MUSCLE | PQ_TWIN_HEADED,
      MQ_AGGRESSIVE | MQ_PROTECTIVE,
      NREQ_HIGH,
    },
     {N_HUNGER,
      PQ_HUGE | PQ_HEAVY | PQ_SHAPELESS | PQ_TRI_HEADED,
      MQ_TELEPATH,
      NREQ_GREAT
     },
     {N_HUNGER,
       PQ_MANY_HEADED,
       MQ_NONE,
       NREQ_SUPER,
     },
     {N_HUNGER,
      PQ_GIG,
      MQ_NONE,
      NREQ_MAX,
    },
  },
};


define_resource_t DEF_RES[20] = {
  {"Bone", RES_BONE,
    OBJ_ENV,
    TILEFLAG_BONE, 2, 1
  },
  {"Wood", RES_WOOD,
    OBJ_ENV,
    TILEFLAG_TREE | TILEFLAG_FOREST,
    3, 3
  },
  {"Vegetation", RES_VEG,
    OBJ_ENV,
    TILEFLAG_NATURAL,
    2, 5,
    RES_WATER
  },
  {"Water", RES_WATER,
    OBJ_ENV,
    TILEFLAG_NATURAL,
    1, 2
  },
  {"Stone", RES_STONE,
    OBJ_ENV,
    TILEFLAG_STONE,
    2, 1
  },
  {"Flesh", RES_MEAT,
    OBJ_ENT,
    TILEFLAG_NONE,
    .smell = 5
  },
  {"Blood", RES_BLOOD,
    OBJ_ENT,
    TILEFLAG_NONE,
    .smell = 5
  },
};

species_relation_t SPEC_ALIGN[__builtin_ctzll(SPEC_DONE)] = {
  {SPEC_NONE},      
  {SPEC_HUMAN,
    {
      [SPEC_HOSTILE] = SPEC_SULKING | SPEC_ORC | SPEC_GOBLINOID | SPEC_CANIFORM
    }
  },    
  {SPEC_ELF,

  },      
  {SPEC_ARCHAIN,

  },   
  {SPEC_GOBLINOID,
    {
      [SPEC_HOSTILE]  = SPEC_HUMAN | SPEC_ELF,
      [SPEC_INDIF]     = SPEC_RUMINANT ,
      [SPEC_KIN]       = SPEC_ORC,
      [SPEC_FRIEND]   = SPEC_SULKING,
      [SPEC_CAUT]      = SPEC_ARCHAIN | SPEC_SKELETAL | SPEC_ROTTING,
      [SPEC_AVOID]     = SPEC_CANIFORM | SPEC_GIANT,
      [SPEC_REVERED]   = SPEC_ARTHROPOD | SPEC_RODENT
    }
  }, 
  {SPEC_ORC,
    {
      [SPEC_HOSTILE]   = SPEC_HUMAN | SPEC_ELF,
      [SPEC_INDIF]     = SPEC_RUMINANT | SPEC_ARTHROPOD | SPEC_RODENT ,
      [SPEC_KIN]       = SPEC_GOBLINOID,
      [SPEC_FRIEND]    = SPEC_SULKING,
      [SPEC_CAUT]      = SPEC_ARCHAIN | SPEC_SKELETAL | SPEC_ROTTING,
      [SPEC_AVOID]     = SPEC_GIANT,
      [SPEC_REVERED]   = SPEC_CANIFORM 
    }
  },      
  {SPEC_GIANT,

  },     
  {SPEC_ARTHROPOD,
    {
      [SPEC_FEAR]       = SPEC_RODENT | SPEC_ELF | SPEC_HUMAN,
      [SPEC_AVOID]      = SPEC_CANIFORM | SPEC_RUMINANT,
      [SPEC_REVERED]    = SPEC_GOBLINOID | SPEC_SULKING,
      [SPEC_INDIF]      = SPEC_ARCHAIN | SPEC_GIANT,
      [SPEC_FRIEND]     = SPEC_SKELETAL | SPEC_ROTTING,
      [SPEC_HUNTS]      = SPEC_ARTHROPOD
    }
  }, 
  {SPEC_ETHEREAL,

  },  
  {SPEC_ROTTING,

  },   
  {SPEC_SKELETAL,

  },  
  {SPEC_VAMPIRIC,

  }, 
  {SPEC_CANIFORM,
    {
      [SPEC_FEAR]        = SPEC_ROTTING,
      [SPEC_AVOID]       = SPEC_HUMAN | SPEC_ELF | SPEC_SKELETAL,
      [SPEC_INDIF]       = SPEC_SULKING | SPEC_ARCHAIN,
      [SPEC_HOSTILE]     = SPEC_ARTHROPOD,
      [SPEC_CAUT]        = SPEC_CANIFORM | SPEC_ETHEREAL | SPEC_VAMPIRIC,
      [SPEC_HUNTS]       = SPEC_RUMINANT | SPEC_RODENT,
      [SPEC_FRIEND]      = SPEC_ORC | SPEC_GOBLINOID,
      [SPEC_REVERED]     = SPEC_GIANT,
    }
  },  
  {SPEC_RODENT,
    {
      [SPEC_FEAR]        = SPEC_CANIFORM,
      [SPEC_AVOID]       = SPEC_HUMAN | SPEC_ELF | SPEC_SKELETAL,
      [SPEC_INDIF]       = SPEC_ARCHAIN | SPEC_ROTTING,
      [SPEC_CAUT]        = SPEC_RUMINANT | SPEC_GIANT | SPEC_ETHEREAL | SPEC_VAMPIRIC,
      [SPEC_HUNTS]       = SPEC_ARTHROPOD,
      [SPEC_FRIEND]      = SPEC_ORC | SPEC_GOBLINOID,
      [SPEC_REVERED]     = SPEC_SULKING,
    }
  },    
  {SPEC_RUMINANT,
    {
      [SPEC_FEAR]        = SPEC_CANIFORM | SPEC_VAMPIRIC | SPEC_SULKING | SPEC_ORC | SPEC_GOBLINOID,
      [SPEC_AVOID]       = SPEC_HUMAN | SPEC_SKELETAL | SPEC_ARTHROPOD,
        [SPEC_INDIF]       = SPEC_ARCHAIN | SPEC_ROTTING,
        [SPEC_CAUT]        = SPEC_RODENT | SPEC_GIANT | SPEC_ETHEREAL,
        [SPEC_FRIEND]      = SPEC_ELF,
      }
    },  
    {SPEC_SULKING,
      {
        [SPEC_FEAR]       = SPEC_GIANT | SPEC_ROTTING | SPEC_VAMPIRIC,
        [SPEC_AVOID]      = SPEC_CANIFORM,
        [SPEC_HOSTILE]    = SPEC_HUMAN | SPEC_ELF,    
        [SPEC_CAUT]       = SPEC_ARCHAIN,     
        [SPEC_HUNTS]      = SPEC_SKELETAL | SPEC_RUMINANT | SPEC_ARTHROPOD,       [SPEC_FRIEND]     = SPEC_ORC | SPEC_GOBLINOID, 
        [SPEC_REVERED]  = SPEC_RODENT,
      }
    }   
};

ability_t ABILITIES[ABILITY_DONE]={
  {ABILITY_NONE},
  {ABILITY_PUNCH,AT_DMG,ACTION_ATTACK, DMG_BLUNT,STAT_STAMINA, DES_NONE, 25,2, 0, 1, 3,0, 1, STAT_HEALTH, ATTR_NONE, ATTR_STR, 
    .skills[0] = SKILL_WEAP_NONE},
  {ABILITY_RAM,AT_DMG,ACTION_ATTACK, DMG_BLUNT,STAT_STAMINA, DES_NONE, 25,2, 0, 1, 3,2, 1, STAT_HEALTH, ATTR_NONE, ATTR_STR, 
    .skills[0] = SKILL_WEAP_NONE},
  {ABILITY_BITE, AT_DMG,ACTION_ATTACK, DMG_PIERCE,STAT_STAMINA, DES_NONE, 20,1, 0, 1, 4,4, 1, STAT_HEALTH, ATTR_NONE, ATTR_STR,
    .skills = SKILL_WEAP_NONE,
    .rankup = {[VAL_PENN] = 1, [VAL_DMG_BONUS] = 2}
  },
  {ABILITY_CHEW, AT_DMG,ACTION_ATTACK, DMG_PIERCE,STAT_STAMINA, DES_NONE, 25,2, 8, 1, 2,0, 1, STAT_ARMOR, ATTR_NONE, ATTR_NONE, ABILITY_GNAW, 
    .skills = SKILL_WEAP_NONE},
  {ABILITY_GNAW,AT_DMG,ACTION_ATTACK, DMG_PIERCE,STAT_STAMINA, DES_NONE, 25,1, 0, 1, 2,0, 1, STAT_HEALTH, ATTR_NONE, ATTR_STR, 
    .skills = SKILL_WEAP_NONE},
  {ABILITY_CLAW,AT_DMG,ACTION_ATTACK, DMG_SLASH, STAT_STAMINA, DES_NONE, 50,2, 0, 2, 3, 1, 1, STAT_HEALTH, ATTR_NONE, ATTR_STR, 
    .skills = SKILL_WEAP_NONE},
  {ABILITY_SWIPE, AT_DMG,ACTION_ATTACK, DMG_SLASH,STAT_STAMINA, DES_NONE, 50,2,0, 2, 6, 3,1, STAT_HEALTH, ATTR_STR,
    .skills = SKILL_WEAP_NONE},

  {ABILITY_BITE_POISON, AT_DMG,ACTION_ATTACK, DMG_PIERCE, STAT_STAMINA, DES_NONE, 25,1, 0, 1, 2, 0,1, STAT_HEALTH, ATTR_NONE, ATTR_STR,ABILITY_POISON, .skills = SKILL_WEAP_NONE,
  },
  {ABILITY_POISON, AT_DMG, ACTION_NONE, DMG_POISON, STAT_NONE, DES_NONE, 25,1, 9, 1, 3,0,1,STAT_HEALTH, ATTR_CON, ATTR_NONE,
    .skills = SKILL_POISON},
 {ABILITY_MAGIC_MISSLE, AT_DMG, ACTION_MAGIC, DMG_FORCE, STAT_ENERGY, DES_SELECT_TARGET, 20,4,99,1,4,1,3,STAT_HEALTH, ATTR_NONE, ATTR_NONE, .chain_id = ABILITY_NONE, .num_skills =1, .skills = SKILL_SPELL_EVO},
  {ABILITY_ELDRITCH_BLAST,AT_DMG,ACTION_MAGIC, DMG_FORCE, STAT_ENERGY, DES_MULTI_TARGET, 30, 8, 14, 1, 10,3, STAT_HEALTH, ATTR_NONE, ATTR_CHAR,
   .skills = SKILL_SPELL_EVO },
  {ABILITY_RESISTANCE,
    .skills = SKILL_SPELL_ABJ
  },
  {ABILITY_GUIDING_BOLT, AT_DMG, ACTION_MAGIC, DMG_RADIANT, STAT_ENERGY, DES_SELECT_TARGET, 20, 4, 0, 1, 6, 0, STAT_HEALTH, ATTR_NONE, ATTR_WIS,
    .skills = SKILL_SPELL_EVO },
  {ABILITY_SACRED_FLAME, AT_DMG,ACTION_MAGIC, DMG_RADIANT, STAT_ENERGY, DES_SELECT_TARGET, 25, 8, 4, 1, 8, 0, 5, STAT_HEALTH, ATTR_DEX, ATTR_WIS,
    .skills = SKILL_SPELL_EVO},
  {ABILITY_STARRY_WISP, AT_DMG,ACTION_MAGIC,DMG_RADIANT, STAT_ENERGY, DES_SELECT_TARGET, 25, 8, 14 , 1, 8, 0, 5, STAT_HEALTH, ATTR_NONE, ATTR_WIS,
  .skills = SKILL_SPELL_EVO},
  {ABILITY_MAGIC_STONE, AT_DMG,ACTION_MAGIC,DMG_BLUNT, STAT_ENERGY, DES_MULTI_TARGET, 25, 3, 15, 1, 6, 0, 5, STAT_HEALTH, ATTR_NONE, ATTR_INT,
  .skills = SKILL_SPELL_TRANS},
  {ABILITY_POISON_SPRAY, AT_DMG,ACTION_MAGIC,DMG_POISON, STAT_ENERGY, DES_SELECT_TARGET, 25, 8, 14, 1,10,0,3, STAT_HEALTH, ATTR_CON, ATTR_NONE,
  .skills = SKILL_SPELL_NECRO},
  {ABILITY_FIRE_BOLT, AT_DMG, ACTION_MAGIC,DMG_FIRE, STAT_ENERGY, DES_SELECT_TARGET, 20, 8, 2, 1,10,0,4, STAT_HEALTH, ATTR_NONE, ATTR_INT,
  .skills = SKILL_SPELL_EVO},
  {ABILITY_REND, AT_DMG,ACTION_ATTACK,DMG_SLASH, STAT_STAMINA, DES_NONE, 25, 8, 4, 1, 4,1,1,STAT_HEALTH, ATTR_NONE,ATTR_STR,ABILITY_BLEED},
  {ABILITY_CURE_WOUNDS, AT_HEAL, ACTION_MAGIC, DMG_TRUE, STAT_ENERGY, DES_SELECT_TARGET, 15, 8, 0, 2,4,1,1, STAT_HEALTH, ATTR_NONE, ATTR_WIS,
    .skills = SKILL_SPELL_ABJ},
  {ABILITY_HAMSTRING},
  {ABILITY_RAGE, AT_DMG,ACTION_ATTACK,DMG_TRUE, STAT_RAGE, DES_NONE, 10, 1, 0,1, 2, 1,1, STAT_HEALTH, ATTR_NONE, ATTR_NONE },
  {ABILITY_WEAP},
  {ABILITY_THROW_ROCK, AT_DMG, ACTION_ATTACK, DMG_BLUNT, STAT_STAMINA, DES_SELECT_TARGET, 20, 1, 10, 1, 4, 0, 2, STAT_HEALTH, ATTR_DEX, ATTR_STR,
    .skills = SKILL_WEAP_NONE
  },
  {ABILITY_WEAP_BLUDGEON, AT_DMG,ACTION_WEAPON,
    DMG_BLUNT, STAT_STAMINA, DES_NONE, 25,1,17,3,2,0,1,STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_CHOP,AT_DMG,ACTION_WEAPON,
    DMG_SLASH, STAT_STAMINA, DES_NONE, 25,1,17,2,3,0,1,STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_STAB,AT_DMG,ACTION_WEAPON,
    DMG_PIERCE, STAT_STAMINA, DES_NONE, 25,1,17,2,3,0,1,STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_SLASH,AT_DMG,ACTION_WEAPON,
    DMG_SLASH, STAT_STAMINA, DES_NONE, 25,1,15,1,6,0,1,STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_PIERCE,AT_DMG,ACTION_WEAPON,
    DMG_PIERCE, STAT_STAMINA, DES_NONE, 25,1,17,2,2,0,1,STAT_HEALTH,ATTR_NONE, ATTR_STR,
  },
  {ABILITY_WEAP_RANGE_PIERCE,AT_DMG,ACTION_WEAPON,
    DMG_PIERCE, STAT_STAMINA, DES_NONE, 27,1,15,1,4,0,3,STAT_HEALTH,ATTR_NONE, ATTR_DEX},
  {ABILITY_STATUS},
  {ABILITY_BLEED, AT_DMG, ACTION_NONE,
    DMG_BLEED, STAT_STAMINA, DES_NONE, 0, 0, 4, 1, 6, 2,1, STAT_HEALTH, ATTR_CON,ATTR_STR},
  {ABILITY_ARMOR},
  {ABILITY_ARMOR_SAVE, AT_SAVE, ACTION_SLOTTED,
    .chain_id = ABILITY_ARMOR_DR, .save_fn = EntAbilitySave},
  {ABILITY_ARMOR_DR, AT_DR, ACTION_SLOTTED,.save_fn = EntAbilityReduce},
  {ABILITY_ITEM},
  {ABILITY_ITEM_HEAL, AT_HEAL, ACTION_ITEM,
    DMG_RADIANT, STAT_NONE, DES_SELF, 10, 1, 1, 2,4,2,0, STAT_HEALTH, ATTR_NONE, ATTR_NONE,.skills=SKILL_ALCH, .use_fn = AbilityConsume
  }
};

item_fn_t item_funcs[ITEM_DONE] = {
  {ITEM_NONE},
  {ITEM_WEAPON,.num_equip = 1, .on_equip= ItemAddAbility},
  {ITEM_ARMOR,.num_equip = 2, .on_equip=
    {
      ItemAddAbility, ItemApplyStats
    }
  },
  {ITEM_CONSUMABLE, .num_equip = 1, .num_use = 1, .on_equip = ItemAddAbility, .on_use =ItemSkillup},
  {ITEM_CONTAINER},
  {ITEM_DONE}
};

weapon_def_t WEAPON_TEMPLATES[WEAP_DONE]= {
  {WEAP_NONE},
  {WEAP_MACE, "Mace",
    550,1250,0,1,0, 50,
    .ability = ABILITY_WEAP_BLUDGEON,
    .skill = SKILL_WEAP_MACE,
    STORE_HELD,
    {[STORE_CARRY] = 2,[STORE_WORN]=1},
    0x0200
  },
  {WEAP_SWORD, "Sword",
    900,1000,0,1,0, 30,
    .ability = ABILITY_WEAP_SLASH,
    .skill = SKILL_WEAP_SWORD,
     STORE_HELD,
    {[STORE_CARRY] = 2,[STORE_WORN]=1},
    0x0400
  },
  {WEAP_AXE,"Axe",
    650,1250,1,0,1, 50,
    .ability = ABILITY_WEAP_CHOP, 
    .skill = SKILL_WEAP_AXE,
    STORE_HELD,
    {[STORE_CARRY] = 2,[STORE_WORN]=1},
    0x0300
  },
  {WEAP_DAGGER, "Dagger",
    200, 500,0, 1,0, 40,
    .ability = ABILITY_WEAP_STAB,
    .skill = SKILL_WEAP_DAGGER,
    STORE_HELD,
    {[STORE_CARRY] = 2,[STORE_WORN]=1},
    0x0100
  },
  {WEAP_JAVELIN, "Javelin",
    5, 850, 0, 1, 2, 20,
    .ability = ABILITY_WEAP_PIERCE,
    .skill = SKILL_WEAP_SPEAR,
    STORE_HELD,
    {[STORE_CARRY] = 2,[STORE_WORN]=1},
    0x0300
  },
  {WEAP_BOW, "Bow",
    2500,1250,0,2,0, 50,
    .w_props = PROP_WEAP_AMMO | PROP_WEAP_TWO_HANDED,
    .ability = ABILITY_WEAP_RANGE_PIERCE,
    .skill = SKILL_WEAP_BOW,
    STORE_HELD,
    {[STORE_CARRY] = 2,[STORE_WORN]=1},
    0x0600
  },
};

WeaponType GetWeapTypeBySkill(SkillType s){
  for(int i = 0; i < WEAP_DONE; i++){
    if(WEAPON_TEMPLATES[i].skill == s)
      return WEAPON_TEMPLATES[i].type;
  }
}

armor_def_t ARMOR_TEMPLATES[ARMOR_DONE]={
  {ARMOR_NONE},
  {ARMOR_NATURAL, 4,
    .skill = SKILL_ARMOR_NATURAL,
  },
  {ARMOR_CLOTH, 6,
    {},
    {},
    1, 5, 50, ATTR_NONE, ATTR_NONE, 0, 0,
    .skill = SKILL_ARMOR_CLOTH,
    STORE_WORN,
    {[STORE_CARRY] = 3, [STORE_CONTAINER]=1},
    0x0700 
  },
  {ARMOR_PADDED, 8,
    {},
    {},
    1, 5, 100, ATTR_DEX,ATTR_NONE,10,0,
   .skill = SKILL_ARMOR_PADDED,
    STORE_WORN,
    {[STORE_CARRY] = 3, [STORE_CONTAINER]=1},
    0x1000
  },
  {ARMOR_LEATHER, 10, 
    {{[DMG_SLASH]=1,[DMG_BLUNT]=1},{}},
    {{[DMG_SLASH]=1,[DMG_PIERCE]=1},{}},
    4,10,200,ATTR_DEX,ATTR_NONE,10,0,
    .skill = SKILL_ARMOR_LEATHER,
    STORE_WORN,
    {[STORE_CARRY] = 3, [STORE_CONTAINER]=1},
    0x1000
  },
  {ARMOR_CHAIN, 12, 
    {{[DMG_SLASH]=2,[DMG_PIERCE]=1},{}},
    {{[DMG_SLASH]=1,[DMG_PIERCE]=1},{}},
    8,25,400,ATTR_DEX,ATTR_STR,2,9,
    .skill = SKILL_ARMOR_CHAIN,
    STORE_WORN,
    {[STORE_CARRY] = 3, [STORE_CONTAINER]=1},
    0x0800
  },
  {ARMOR_PLATE, 16,
    {{[DMG_SLASH]=2,[DMG_PIERCE]=2,[DMG_BLUNT]=1},{}},
    {{[DMG_SLASH]=1,[DMG_PIERCE]=1}},
    20,100,800,ATTR_NONE,ATTR_STR,0,12,
    .skill = SKILL_ARMOR_PLATE,
    STORE_WORN,
    {[STORE_CARRY] = 3, [STORE_CONTAINER]=1},
    0x1000
  },
};

ArmorType GetArmorTypeBySkill(SkillType s){
  for(int i = 0; i < ARMOR_DONE; i++){
    if(ARMOR_TEMPLATES[i].skill == s)
      return ARMOR_TEMPLATES[i].type;
  }
}
consume_def_t CONSUME_TEMPLATES[CONS_DONE] = {
  {CONS_POT, 50, 400, 1, 2, 5,
    PROP_MAT_LIQUID, PROP_CONS_HEAL,
    ABILITY_ITEM_HEAL, SKILL_ALCH,
    STORE_SPECIAL,
    {[STORE_HELD] = 5, [STORE_CONTAINER] = 1},
    0x0020
  }
};

container_def_t CONTAINER_TEMPLATES[INV_DONE]={
  {INV_HELD},
  {INV_WORN},
  {INV_BACK},
  {INV_BELT},
  {INV_SLING, 1250, 25, 4, .size = 0x0100, 0x0020}
};

define_mobtype_t MOB_THEME[MT_DONE] = {
  {MT_PREY,       MOB_THEME_GAME},
  {MT_PRED,       MOB_THEME_PRED},
  {MT_CRITTER,    MOB_THEME_CRITTER, SPEC_RODENT},
  {MT_BUG,        MOB_THEME_CRITTER, SPEC_ARTHROPOD},
  {MT_FACTION,    MOB_THEME_MARTIAL | MOB_THEME_CIVIL},
  {MT_MONSTER,    MOB_THEME_MONSTER},
  {MT_LOCALS,     MOB_THEME_PRIMITIVE},
};

biome_t BIOME[BIO_DONE] = {
  {BIO_CAVE,
  
  },
  {BIO_DUNGEON},
  {BIO_FOREST,
    0.1f, 0.15f, 0.05f, 0.1f, 0.05f,
    .ratios = {
      [MT_PREY] = .40f, [MT_PRED] = .08f, [MT_CRITTER] = .20f, [MT_BUG] = .12f, [MT_FACTION] = .0f, [MT_MONSTER] = 0.07f, [MT_LOCALS] = .13f},
  },
};

sense_quality_t SENSE_QUAL[SEN_DONE] = {
  [SEN_HEAR] = { SEN_HEAR,
    {
      [SC_MIN]     = PQ_NO_HEAD,
      [SC_LESSER]  = PQ_TINY_HEAD,
      [SC_BELOW]   = PQ_SMALL_HEAD,
      [SC_ABOVE]   = PQ_TWIN_HEADED, PQ_TRI_HEADED,
      [SC_GREATER] = PQ_MANY_HEADED,
      [SC_SUPER]   = PQ_LARGE_EARS
    },
    {
      [SC_ABOVE]   = MQ_ANXIOUS | MQ_CAUTIOUS,
      [SC_GREATER] = MQ_ALERT,
      [SC_SUPER]   = MQ_ATTUNED
    },
    {
      [SC_MIN]      = 0,
      [SC_LESSER]   = 1,
      [SC_BELOW]    = 3,
      [SC_AVERAGE]  = 5,
      [SC_ABOVE]    = 7,
      [SC_GREATER]  = 9,
      [SC_SUPER]    = 11,
      [SC_MAX]      = 13,
    }
  },
  [SEN_SEE] = { SEN_SEE,
    {
      [SC_MIN]      = PQ_NO_HEAD | PQ_NO_EYES,
      [SC_INFER]    = PQ_ONE_EYE | PQ_SHORT,
      [SC_LESSER]   = PQ_TINY_HEAD,
      [SC_BELOW]    = PQ_SMALL_HEAD,
      [SC_ABOVE]    = PQ_TWIN_HEADED | PQ_TRI_HEADED | PQ_MANY_HEADED,
      [SC_GREATER]  = PQ_LARGE_HEAD | PQ_TALL,
      [SC_SUPER]    = PQ_HUGE | PQ_GIG
    },
    {
      [SC_ABOVE]    = MQ_PERCEPTIVE | MQ_AWARE
    },
    {
      [SC_MIN]      = 0,
      [SC_INFER]    = 4,
      [SC_LESSER]   = 6,
      [SC_BELOW]    = 8,
      [SC_AVERAGE]  = 10,
      [SC_ABOVE]    = 12,
      [SC_GREATER]  = 14,
      [SC_SUPER]    = 16,
      [SC_MAX]      = 20,
    }
  },
  [SEN_SMELL] = {
    SEN_SMELL,
    {
      [SC_MIN]      = PQ_NO_HEAD,
      [SC_ABOVE]    = PQ_TWIN_HEADED | PQ_TRI_HEADED | PQ_MANY_HEADED,
      [SC_GREATER]  = PQ_LARGE_HEAD | PQ_TALL,
      [SC_SUPER]    = PQ_LARGE_NOSE | PQ_SENSITIVE_NOSE,
    },
    {
      [SC_ABOVE]    = MQ_HIVE_MIND,
      [SC_GREATER]  = MQ_PERCEPTIVE,
      [SC_SUPER]    = MQ_TERRITORIAL,
    },
    {
      [SC_MIN]      = 0,
      [SC_INFER]    = 1,
      [SC_LESSER]   = 2,
      [SC_BELOW]    = 3,
      [SC_AVERAGE]  = 4,
      [SC_ABOVE]    = 6,
      [SC_GREATER]  = 8,
      [SC_SUPER]    = 10,
      [SC_MAX]      = 11
    }
  },
};

  
static sense_t SENSE_BASE[SEN_DONE] = {
  {SEN_HEAR, SKILL_PERCEPT, ATTR_WIS, MOD_SQRT},
  {SEN_SEE, SKILL_PERCEPT, ATTR_WIS, MOD_CBRT},
  {SEN_SMELL, SKILL_PERCEPT, ATTR_WIS, MOD_SQRT}
};

Faction FactionMakeHashID(int count, param_t* params[count]){

  char name[256];
  int  len = 0;
  
  len += snprintf(name + len, sizeof(name) - len, "RUNTIME");
  for(int i = 0; i < count; i++){
    switch(params[i]->type_id){
      case DATA_INT:
      case DATA_FLOAT:
      case DATA_BOOL:
      case DATA_PTR:
      case DATA_ENTITY:
      case DATA_CELL:
      break;
      case DATA_STRING:
      len += snprintf(name + len, sizeof(name) - len,
          "_%s", ParamReadString(params[i]));
      break;

    }

  }

  return RegisterFaction(name);
}

faction_t* GetFactionByType(EntityType type){
  for(int i = 0; i < NUM_FACTIONS; i++){

    if(FACTIONS[i]->member_ratio[type]>0)
      return FACTIONS[i];
  }

  return NULL;
}

Faction FindFaction(ent_t* e){
  faction_t* exists = GetFactionBySpec(e->props->race);

  if(exists)
    return exists->id;

  exists = GetFactionByType(e->type);
  if(exists)
    return exists->id;

  return 0;
}

Faction RegisterFactionByType(ent_t* e){
  Faction exists = FindFaction(e);
  
  if(exists > 0)
    return exists;

  param_t name = {
    .type_id = DATA_STRING,
    .data = (void*)&e->props->race_name,
    .size = strlen(e->props->race_name)+1
  };

  race_define_t race = DEFINE_RACE[SpecToIndex(e->props->race)];
  param_t spec = {
    .type_id = DATA_STRING,
    .data = (void*)&race.name,
    .size = strlen(race.name)+1
  };

  param_t* params[2];
  
  params[0] = &name;
  params[1] = &spec;

  Faction fid = FactionMakeHashID(2, params);
  faction_t* f = GetFactionByID(fid);
  f->species = e->props->race;
  f->member_ratio[e->type]++;
}

Faction RegisterFaction(const char* name){
  Faction f = hash_str_32(name);

  for(int i = 0; i < NUM_FACTIONS; i++){
    if(FACTIONS[i]->id == f)
      return f;

    if(strcmp(name,FACTIONS[i]->name) == 0){
      FACTIONS[i]->id = f;
      return f;
    }
  
  }


  return InitFaction(name)->id;
}

faction_t* InitFaction(const char* name){
  faction_t* f = calloc(1, sizeof(faction_t));

  Faction id = hash_str_32(name);

  f->id = id;
  f->name = strdup(name);

  FACTIONS[NUM_FACTIONS++] = f;
  return f;
}

const char* Faction_Name(Faction id){

}



dice_roll_t* Die(int side, int num){
  dice_roll_t* die = malloc(sizeof(dice_roll_t));

  *die = (dice_roll_t){
    .sides = side,
    .num_die = num,
    .roll = RollDie
  };

  return die;
}

dice_roll_t* InitDie(int side, int num, int adv, DiceRollFunction fn){
  dice_roll_t* die = malloc(sizeof(dice_roll_t));

  DiceCompareCallback cb = NULL;
  if(adv > 0)
    cb = GREATER_THAN;
  else if (adv < 0)
    cb = LESS_THAN;

  *die = (dice_roll_t){
    .sides      = side,
    .num_die    = num,
    .advantage  = adv,
    .roll       = fn,
    .cb         = cb
  };

  return die;
}
int DieMax(dice_roll_t* d){

  return d->sides * d->num_die;
}
int RollDieAdvantage(dice_roll_t* d, int* results){
  int choice = RollDie(d,results);

  for (int i = 0; i < d->advantage; i++){
     choice = d->cb(choice, RollDie(d,results));
  }

  return choice;
}

int RollDie(dice_roll_t* d, int* results){
  int sum = 0;

  for(int i = 0; i < d->num_die; i++){
    int roll = (rand()% d->sides) + 1;
    sum+=roll;
    results[i]=roll;
  }

  return sum;
}

attribute_t* InitAttribute(AttributeType type, int val){
  attribute_t* t = calloc(1,sizeof(attribute_t));

  t->type = type;
  t->max = t->min = t->val = val;
  t->asi = 0;
  t->rollover = 0;
  t->cap = 20;
  return t;
}

bool AttributeScoreEvent(attribute_t* a){
  if(a->max == a->cap)
    return false;
  a->max++;
  a->val++;
  return true;
}
bool AttributeScoreIncrease(attribute_t* a){
  if(a->asi==0)
    return false;

  a->rollover+=a->asi;
  if((a->asi+a->rollover+a->max) > a->cap)
    return false;

  float new = a->max+a->rollover;
  a->rollover = new - (int)new;
  a->max = a->val = (int)new;
  TraceLog(LOG_INFO,"===== ATTRIBUTE %s ====\n increased to %i",attributes[a->type].name, a->val);
  return true;
}
// Allocates a copy of the filename without extension
char* GetFileStem(const char* filename) {
    const char* dot = strrchr(filename, '.');
    size_t len = dot ? (size_t)(dot - filename) : strlen(filename);

    char* stem = malloc(len + 1);
    if (!stem) return NULL;
    memcpy(stem, filename, len);
    stem[len] = '\0';
    return stem;
}

value_t* InitValue(ValueCategory cat, int base){
  value_t* v = calloc(1,sizeof(value_t));
  value_relate_t rel = VALUE_RELATES[cat];
  *v = (value_t){
    .cat = cat,
    .base = base,
    .stat_aff_by = rel.stat_aff,
    .stat_relates_to = rel.stat_rel,
    .attr_aff_by = rel.att_aff,
    .attr_relates_to = rel.att_rel,
  };

  return v;
}

float AffixAdd(value_affix_t* self, int val){
  return val + self->val;
}

float AffixSub(value_affix_t* self, int val){
  return val - self->val;
}

float AffixMul(value_affix_t* self, int val){
  return val * self->val/100;
}

float AffixFract(value_affix_t* self, int val){
  return val * self->val/100;
}

float AffixBase(value_affix_t* self, int val){
  return self->val;
}

int ValueRebase(value_t* self){
  for(int i = 0; i < AFF_DONE; i++){
    if(self->base_app[i] == NULL)
      continue;

    if(self->base_app[i]->val == 0)
      continue;

    self->base = ValueApplyModsToVal(self->base, self->base_app[i]);
  }

  return self->base;

}

int ValueApplyModsToVal(int val, value_affix_t* aff){
  int result = aff->affix_mod(aff,val);

  return result;
}

void ValueAddBaseMod(value_t* self, value_affix_t mod){
  ValueAffix aff = mod.affix;

  if(self->base_app[aff]){
    int val = self->base_app[aff]->val;
    switch(aff){
      case AFF_ADD:
      case AFF_SUB:
        self->base_app[aff]->val+=mod.val;
        break;
      case AFF_FRACT:
        float fval = val/100 * mod.val/100;
        self->base_app[aff]->val = CLAMP(fval*100,1,99);
        break;
      case AFF_BASE:
        self->base_app[aff]->val = val>mod.val?val:mod.val;
        break;
      case AFF_MUL:
        self->base_app[aff]->val = imax(100,mod.val+val-100);
        break;
    }
  }
  else{
    self->base_app[aff] = InitValueAffixFromMod(mod);
  }
}

value_affix_t* InitValueAffixFromMod(value_affix_t mod){
  value_affix_t* va = calloc(1,sizeof(value_affix_t));

  *va = mod;
  va->affix_mod = AffixBase;
  switch(va->affix){
    case AFF_ADD:
      va->affix_mod = AffixAdd;
      break;
    case AFF_SUB:
      va->affix_mod = AffixSub;
      break;
    case AFF_MUL:
      va->affix_mod = AffixMul;
      break;
    case AFF_FRACT:
      va->affix_mod = AffixFract;
      break;
  }

  return va;
}

stat_t* InitStatOnMin(StatType attr, float min, float max){
 stat_t* s = calloc(1,sizeof(stat_t));
 *s =(stat_t){
    .type = attr,
      .min = min,
      .max = max,
      .current = min,
      .ratio = StatGetRatio,
      .increment = 1.0f
  };

 return s;
}

stat_t* InitStatOnMax(StatType attr, float val, AttributeType modified_by){

  return InitStat(attr, 0, val, val);
}

bool StatIsEmpty(stat_t* s){
  return s->current <= s->min;
}

stat_t* InitStatEmpty(void){}
stat_t* InitStat(StatType attr,float min, float max, float amount){
 stat_t* s = malloc(sizeof(stat_t));
 stat_attribute_relation_t relate = stat_modifiers[attr];

 stat_relate_t tandem = STAT_RELATION[attr];

 *s =(stat_t){
    .type = attr,
      .related   = tandem.related,
      .min       = min,
      .max       = max,
      .base      = amount,
      .current   = amount,
      .ratio     = StatGetRatio,
      .increment = 1.0f,
      .die       = Die(amount,1),
      .start     = relate.init,
      .lvl       = relate.lvl,
      .reverse   = relate.reverse,
      .need      = tandem.need
 }; 
 for(int i = 0; i < ATTR_DONE; i++)
   s->modified_by[i] = relate.modifier[i];

 return s;

}

bool StatExpand(stat_t* s, int val, bool fill){
  s->max+= val;
  if(fill)
    StatMaxOut(s);

  return true;
}
void StatIncreaseValue(stat_t* self, float old, float cur){
  StatChangeValue(self->owner, self, self->increment);
}

bool StatIncrementValue(stat_t* attr,bool increase){
  float inc = attr->increment;
  if(!increase)
    inc*=-1;

  float old = attr->current;
  if(old+inc<attr->min)
    return false;

  attr->current+=inc;
  attr->current = CLAMPF(attr->current,attr->min, attr->max);
  float cur = attr->current;

  if(attr->current == attr->max && old != attr->max)
    if(attr->on_stat_full)
      attr->on_stat_full(attr,old,cur);

  if(StatIsEmpty(attr)&& attr->on_stat_empty)
    attr->on_stat_empty(attr,old,cur);

  if(attr->current != old)
    if(attr->on_stat_change != NULL)
      attr->on_stat_change(attr,old, cur);

  return true;
}

bool StatChangeValue(struct ent_s* owner, stat_t* s, float val){
  float old = s->current;
  s->current+=val;
  s->current = CLAMPF(s->current,s->min, s->max);
  float cur = s->current;
  if(s->current == old) 
    return false;

  if(s->need>N_NONE)
    NeedIncrement(s->need, s->owner, val);

  if(s->on_stat_change != NULL)
    s->on_stat_change(s,old, cur);
  
  if(s->current == s->min && s->on_stat_empty!=NULL)
    s->on_stat_empty(s,old,cur);
  
  if(s->current == s->max && old != s->max)
    if(s->on_stat_full)
      s->on_stat_full(s,old,cur);

  return true;
}

void StatRestart(struct stat_s* self, float old, float cur){
  self->current = self->min;
}

void StatReverse(struct stat_s* self, float old, float cur){
  self->increment *= -1;
}

void StatMaxOut(stat_t* s){
  if(s->reverse)
    s->current = s->min;
  else
    s->current = s->max;

}

void StatEmpty(stat_t* s){
  if(s->reverse)
    s->current = s->max;
  else
    s->current = s->min;
}

float StatGetRatio(stat_t *self){
  return self->current / self->max;
}

void FormulaAddAttr(stat_t* self){
  int modifier = 0;
      
  for(int i = 0; i < ATTR_DONE;i++){
    if(self->modified_by[i]){
      ModifierType mod = self->modified_by[i];
      switch(mod){
        case MOD_SQRT:  
          modifier += isqrt(self->owner->attribs[i]->val); 
          break;
        case MOD_NEG_SQRT:
          modifier -= isqrt(self->owner->attribs[i]->val); 
          break;
        case MOD_ADD:
          modifier += self->owner->attribs[i]->val;
          break;
        default:
          break;
      }
    }
  }

  self->max = self->base+modifier;
  self->current = imin(self->max, self->current);
}

void FormulaDie(stat_t* self){}
void FormulaDieAttr(stat_t* self){}
void FormulaBaseAttr(stat_t* self){}
void FormulaBaseDie(stat_t* self){}

void FormulaDieAddAttr(stat_t* self){
  int modifier = 0;
      
  for(int i = 0; i < ATTR_DONE;i++){
    if(self->modified_by[i]){
      ModifierType mod = self->modified_by[i];
      switch(mod){
        case MOD_SQRT:  
          modifier += isqrt(self->owner->attribs[i]->val); 
          break;
        case MOD_NEG_SQRT:
          modifier -= isqrt(self->owner->attribs[i]->val); 
          break;
        case MOD_ADD:
          modifier += self->owner->attribs[i]->val;
          break;
        default:
          break;
      }
    }
  }

  int rolls[self->die->num_die];
  int roll = self->die->roll(self->die, rolls);
  self->base+= roll;
  self->max+=roll+modifier;
  self->current = imin(self->max, self->current);
}

void SenseAddAttr(sense_t* self){
  int mod = 0;

  ModifierType modif = self->attr_mod;
  AttributeType t = self->attr;
  switch(modif){
    case MOD_CBRT:
      mod += cbrt(self->owner->attribs[t]->val);
      break;
    case MOD_SQRT:
      mod += isqrt(self->owner->attribs[t]->val);
      break;
    case MOD_NEG_SQRT:
      mod -= isqrt(self->owner->attribs[t]->val);
      break;
    case MOD_ADD:
      mod += self->owner->attribs[t]->val;
      break;
    default:
      break;
  }

  self->range = self->base+mod;
}

sense_t* InitSense(ent_t* e, Senses type, int val){
  sense_t* s = calloc(1,sizeof(sense_t));

  sense_t base = SENSE_BASE[type];

  *s = (sense_t){
    .type     = type,
    .owner    = e,
    .skill    = base.skill,
    .attr     = base.attr,
    .attr_mod = base.attr_mod,
    .base     = val,
  };

  SenseAddAttr(s);
}

skill_event_t* InitSkillEvent(skill_t* s, int cr){
  skill_event_t* skev = calloc(1,sizeof(skill_event_t));

  skill_decay_t* decay = SkillEventDecay(s->id, cr);
  *skev = (skill_event_t){
    .skill       = s->id,
      .challenge = cr,
      .decay     = decay
  };
}

skill_check_t* InitSkillCheck(skill_t* skill){
  skill_check_t* sc = calloc(1,sizeof(skill_check_t));

  skill_proficiency_bonus_t spb = GRANTS_PB[skill->id];

  AttributeType bonus =spb.attr;
  *sc = (skill_check_t){
    .owner = skill->owner,
      .skill = skill,
      .id  = skill->id,
      .counter = spb.counter,
      .type = spb.type,
      .die = Die(spb.base,1)
  };


  sc->bonus[bonus] = MOD_CBRT;
}

bool SkillUseSecondary(skill_t* self, int gain, InteractResult result){
  
}
 
bool SkillUse(skill_t* self, int source, int target, int gain, InteractResult result){

  define_skill_rank_t r = SKILL_RANKS[SkillRankGet(self)];
  gain-=r.penalty;
  
  int* cr = malloc(sizeof(int));
  *cr = gain;
  
  param_t data = {
    .type_id  = DATA_INT,
    .data     = cr,
    .size = sizeof(int)
  };
 
  interaction_t* iter = StartInteraction(source,target,EVENT_SKILL, 9, self, self->id, data, RegisterSkillEvent, UpdateSkillEvent, true);

  if (gain < 1)
    return false;


  skill_event_t* skev = iter->ctx;
  skill_decay_t* sk_decay = skev->decay;

  float amnt = sk_decay->weights[result];
  float decay = sk_decay->diminish[result];
  if(decay == 0)
   decay = 50; 

  if(amnt == 0)
    amnt = 75;

  decay*=0.01f;
  amnt*=0.01f;
  int increase = gain * amnt;


  for(int i = 0; i < skev->uses; i++)
    increase*=decay;

  if(increase > 0.5f)
    SkillIncrease(self,increase);

  skev->uses++;
  return true;
}

interaction_uid_i RegisterSkillEvent(interaction_t* self, void* ctx, param_t payload){
  skill_t* skill = ctx;
  int cr = 0;
  if(payload.type_id == DATA_INT){
    cr = ParamReadInt(&payload);
    ParamFree(&payload); 
  }

  skill_event_t* skev = InitSkillEvent(skill, cr);

  skev->interact_uid = self->uid;
  self->ctx = skev;

  return self->uid; 
}

interaction_uid_i UpdateSkillEvent(interaction_t* self, void* ctx, param_t payload){
  skill_event_t* skev = ctx;

  self->timer->elapsed = 0;
  skev->uses++;

  return self->uid;

}

skill_decay_t* SkillEventDecay(SkillType skill, int difficulty){

  SkillRate rating = SkillRateLookup(skill);

  if(rating == RATE_NONE)
    return NULL;

  define_skill_rate_t skr = SKILL_RATES[rating];

  skill_decay_t* decay = calloc(1,sizeof(skill_decay_t));

  *decay = (skill_decay_t){
    .skill = skill,
    .rate  = rating,
    .falloff = skr.duration
  };

  for(int i = 0; i < IR_MAX; i++){
    decay->weights[i] =skr.weights[i];
    decay->diminish[i] =skr.dr[i];

  }

  return decay;
}

skill_t* InitSkill(SkillType id, struct ent_s* owner, int min, int max){
  skill_t* s = calloc(1,sizeof(skill_t));

  *s = (skill_t){
    .id = id,
    .val = min,
    .min = min,
    .max = max,
    .point = 0,
    .threshold = 350,
    .owner = owner,
    .on_skill_up = SkillupRelated
  };

  s->checks = InitSkillCheck(s); 
  return s;

}

int SkillCheckGetVal(skill_t* s, ValueCategory val){
  if(s->ovrd)
    return s->ovrd->vals[val];

  return s->checks->vals[val];
}

InteractResult SkillCheck(skill_t* s, skill_t* against){
  skill_check_t* checks = s->checks;
  if(s->ovrd)
    checks = s->ovrd;

  if(checks == NULL)
    return IR_CRITICAL_FAIL;

  if(against->checks == NULL)
    return IR_TOTAL_SUCC;

  int hits[1];

  dice_roll_t* dr = checks->die;
  if(checks->vals[VAL_ADV_HIT]!=0){
    dr->cb = GREATER_THAN;
    dr->roll = RollDieAdvantage;
    dr->advantage = checks->vals[VAL_ADV_HIT];
  }
  int hit = dr->roll(dr, hits);

  int saves[1];
  int save = against->checks->die->roll(s->checks->die, saves);

  s->ovrd = NULL;

  return (hit > save)?IR_SUCCESS:IR_FAIL;
}

bool SkillIncreaseUncapped(struct skill_s* s, int amnt){
  int rounds = amnt / MAX_SKILL_GAIN;
  int left = amnt % MAX_SKILL_GAIN;

  for(int i = 0; i < rounds; i++)
    SkillIncrease(s, MAX_SKILL_GAIN);

  SkillIncrease(s,left);
}

bool SkillIncrease(struct skill_s* s, int amnt){
  s->point+=imin(MAX_SKILL_GAIN,amnt);

  if(s->point < s->threshold){
    //TraceLog(LOG_INFO,"%s %i experience in %s now (%0f / %i)",s->owner->name, s->owner->uid, SKILL_NAMES[s->id], s->point, s->threshold);
    return false;
  }

  s->point = s->point - s->threshold;


  int old = s->val;
  s->val++;

  s->threshold+= 100/s->val;
  
  TraceLog(LOG_INFO,"%s has reached %s rank %i",s->owner->name, SKILL_NAMES[s->id], s->val);
  if(s->on_skill_up)
    s->on_skill_up(s,old,s->val);

  return true;
}

void SkillupRelated(skill_t* self, float old, float cur){

  for (int i = 0; i < MAG_DONE; i++){
    if(!SKILLUP_RELATION[i].skills[self->id])
      continue;
    skill_relation_t related = SKILLUP_RELATION[i];

    int mag = i+2;
    int inc = (mag*mag-i)*cur;
    SkillType rel = related.skill;
    if (rel == SKILL_NONE)
      continue;
    SkillIncrease(self->owner->skills[rel], inc);
    if(self->owner->type == ENT_PERSON)
      TraceLog(LOG_INFO, "%s %0f exp till level %i",self->owner->name,
          self->threshold - self->point, self->val+1);
  }
}

SkillRate SkillRateLookup(SkillType s){
  for(int i = 0; i < RATE_DONE; i++){
   if(SKILLRATE_LOOKUP[i].skills[s])
    return i; 
  }

  return RATE_LINEAR;
}

int ResistDmgLookup(uint64_t trait){
  for (int i = 0; i < 15; i++){
    if(RESIST_LOOKUP[i].trait == trait)
      return RESIST_LOOKUP[i].school;
  }
}

EntityType MobGetByRules(MobRules rules){

}

int GetItemPropsMods(ItemProps props, uint64_t cat_props, ItemCategory cat, item_prop_mod_t **mods){
  int idx = 0;

  while(props){
    uint64_t prop = props & -props;
    props &= props - 1;
    mods[idx++] = GetItemPropMods(ITEM_NONE,prop);
  }

  while (cat_props) {
    uint64_t bit = cat_props & -cat_props;
    cat_props &= cat_props - 1;

    mods[idx++] = GetItemPropMods(cat,bit);
  }

  return idx;
}

item_prop_mod_t* GetItemPropMods(ItemCategory cat, uint64_t prop){
  for(int i = 0; i < NUM_WEAP_PROPS; i++){
    if(PROP_MODS[cat][i].propID != prop)
      continue;

    return &PROP_MODS[cat][i];
  }
  
  return NULL;
}

need_t* InitNeed(Needs id, ent_t* owner){

  uint64_t req = GetNeedReq(id, owner->props->body, owner->props->mind);
  uint64_t res = EntGetResourceByNeed(owner, id);

  need_t* n = calloc(1,sizeof(need_t));

  *n = (need_t){
    .id = id,
      .resource = res,
      .owner = owner,
      .status = NEED_OK
  };

  if(req > 0){
    for(int i = 0; i < NEED_DONE; i++){
      int val = NeedThreshold(req, i);
      n->vals[i] = val;
      if(i == NEED_OK)
        n->val = val;
    }
  }

  return n;
}

void NeedSyncMeter(need_t* n, int amount){
  if(n->val < 0)
    amount = 128;

  n->meter+=amount;

  if(n->meter < (int)NREQ_AVG)
    return;

  n->meter = 0;

  n->val += (1+n->status)*(NREQ_MIN/4);

  if(n->val < n->vals[n->status])
    return;

  if(n->status < NEED_CRITICAL)
    n->status++;
}

void NeedIncrement(Needs id, ent_t* owner, int amount){
  need_t* n = owner->control->needs[id];

  NeedSyncMeter(n, amount);
  
}
void NeedReset(need_t* n){
  n->activity = false;

  for(int i = NEED_CRITICAL; i > -1; i--){
    if (n->val > n->vals[i])
      break;
    n->prio*=0.5;
    n->status = i;
  }

  if(n->status > NEED_MET)
    return;

  n->prio = 0;
  n->goal = NULL;
}

void NeedStep(need_t* n){
  n->prio += n->id + n->status;

  if(n->activity){
    NeedReset(n);
    return;
  }

  int r = RandRange(0, (int)NREQ_MIN);

  if(r > (int)n->vals[NEED_LOW])
    return;

  r = CLAMP(r,4,64);
  NeedSyncMeter(n, r);
}

initiative_t* InitInit(ActionType action, ent_t* e){
  initiative_t* i  = calloc(1, sizeof(initiative_t));

  *i = (initiative_t){
    .owner = e,
      .action = action,
      .base   = 10,
  };

  i->modified_by[ATTR_DEX] = MOD_ADD;

  i->die = Die(i->base + e->attribs[ATTR_DEX]->max, 1);

  return i;
}

int RollInitiative(initiative_t* i){

  int res[1];
  return i->die->roll(i->die, res);
}
