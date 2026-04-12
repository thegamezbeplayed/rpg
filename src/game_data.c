#include <raylib.h>
#include "game_tools.h"
#include "game_process.h"
#include "game_gen.h"
#include <stdio.h>

#include "game_helpers.h"
#include "game_strings.h"

ent_t* ParamReadEnt(const param_t* o) {
  if(o && o->type_id == DATA_ENTITY)
    return (ent_t*)o->data;

    return NULL;
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

affect_t AFFECTS[NUM_AFFECTS] = {
  {AFFECT_SAP, AFF_ABI,
    {EVENT_DAMAGE_DEALT, PARAM_WEAP_TYPE, {DATA_INT, .id = WEAP_MACE}},
    {EVENT_ROLL_DMG, PARAM_ABILITY, -1},
    AffectHitRoll,
    AffectRollAdv,
    {[VAL_HIT] = 3, [VAL_ADV_DMG] = -1}
  }
};

feat_t FEATS[NUM_FEATS] = {
  {FEAT_WEAPON_MASTERY, SKILL_WEAP_MACE, 4, 1,
    {{AFF_MOD_ABI, AFFECT_SAP, 4,0}}
  }

};

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
    TILEFLAG_BONE, TILEFLAG_BORDER,
    1, 1, 5,
  },
  {"Wood", RES_WOOD,
    OBJ_ENV,
    TILEFLAG_TREE | TILEFLAG_FOREST,
    TILEFLAG_BORDER,
    .75, 3, 2
  },
  {"Vegetation", RES_VEG,
    OBJ_ENV,
    TILEFLAG_NATURAL,
    TILEFLAG_TREE | TILEFLAG_FOREST,
    1, 5, 3,
    RES_WATER
  },
  {"Water", RES_WATER,
    OBJ_ENV,
    TILEFLAG_NATURAL, TILEFLAG_BORDER,
    1, 2, 3
  },
  {"Stone", RES_STONE,
    OBJ_ENV,
    TILEFLAG_STONE, TILEFLAG_BORDER,
    0.5f, 1, 3
  },
  {"Flesh", RES_MEAT,
    OBJ_ENT,
    TILEFLAG_NONE,TILEFLAG_BORDER,
    .smell = 5,
    .base_cr = 5
  },
  {"Blood", RES_BLOOD,
    OBJ_ENT,
    TILEFLAG_NONE,TILEFLAG_BORDER,
    .smell = 5,
    .base_cr = 7
  },
  {"Hide", RES_HIDE,
    OBJ_ENT,
    TILEFLAG_NONE,TILEFLAG_BORDER,
    .smell = 4,
    .base_cr = 4
  },
  {"Fur", RES_FUR,
    OBJ_ENT,
    TILEFLAG_NONE,TILEFLAG_BORDER,
    .smell = 6,
    .base_cr = 2,
  },
  {"Scale", RES_SCALE,
    OBJ_ENT,
    TILEFLAG_NONE,TILEFLAG_BORDER,
    .smell = 1,
    .base_cr = 4
  },
  {"Ore", RES_METAL,
    OBJ_ENV,
    TILEFLAG_NONE,TILEFLAG_BORDER,
    0.125f, 2, 10
  }
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
      [__builtin_ctzll(SPEC_HOSTILE)]  = SPEC_HUMAN | SPEC_ELF,
      [__builtin_ctzll(SPEC_INDIF)]     = SPEC_RUMINANT ,
      [__builtin_ctzll(SPEC_KIN)]       = SPEC_ORC,
      [__builtin_ctzll(SPEC_FRIEND)]   = SPEC_SULKING,
      [__builtin_ctzll(SPEC_CAUT)]      = SPEC_ARCHAIN | SPEC_SKELETAL | SPEC_ROTTING,
      [__builtin_ctzll(SPEC_AVOID)]     = SPEC_CANIFORM | SPEC_GIANT,
      [__builtin_ctzll(SPEC_REVERED)]   = SPEC_ARTHROPOD | SPEC_RODENT
    }
  }, 
  {SPEC_ORC,
    {
      [__builtin_ctzll(SPEC_HOSTILE)]   = SPEC_HUMAN | SPEC_ELF,
      [__builtin_ctzll(SPEC_INDIF)]     = SPEC_RUMINANT | SPEC_ARTHROPOD | SPEC_RODENT ,
      [__builtin_ctzll(SPEC_KIN)]       = SPEC_GOBLINOID,
      [__builtin_ctzll(SPEC_FRIEND)]    = SPEC_SULKING,
      [__builtin_ctzll(SPEC_CAUT)]      = SPEC_ARCHAIN | SPEC_SKELETAL | SPEC_ROTTING,
      [__builtin_ctzll(SPEC_AVOID)]     = SPEC_GIANT,
      [__builtin_ctzll(SPEC_REVERED)]   = SPEC_CANIFORM 
    }
  },      
  {SPEC_GIANT,

  },     
  {SPEC_ARTHROPOD,
    {
      [__builtin_ctzll(SPEC_FEAR)]       = SPEC_RODENT | SPEC_ELF | SPEC_HUMAN,
      [__builtin_ctzll(SPEC_AVOID)]      = SPEC_CANIFORM | SPEC_RUMINANT,
      [__builtin_ctzll(SPEC_REVERED)]    = SPEC_GOBLINOID | SPEC_SULKING,
      [__builtin_ctzll(SPEC_INDIF)]      = SPEC_ARCHAIN | SPEC_GIANT,
      [__builtin_ctzll(SPEC_FRIEND)]     = SPEC_SKELETAL | SPEC_ROTTING,
      [__builtin_ctzll(SPEC_HUNTS)]      = SPEC_ARTHROPOD
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
      [__builtin_ctzll(SPEC_FEAR)]        = SPEC_ROTTING,
      [__builtin_ctzll(SPEC_AVOID)]       = SPEC_HUMAN | SPEC_ELF | SPEC_SKELETAL,
      [__builtin_ctzll(SPEC_INDIF)]       = SPEC_SULKING | SPEC_ARCHAIN,
      [__builtin_ctzll(SPEC_HOSTILE)]     = SPEC_ARTHROPOD,
      [__builtin_ctzll(SPEC_CAUT)]        = SPEC_CANIFORM | SPEC_ETHEREAL | SPEC_VAMPIRIC,
      [__builtin_ctzll(SPEC_HUNTS)]       = SPEC_RUMINANT | SPEC_RODENT,
      [__builtin_ctzll(SPEC_FRIEND)]      = SPEC_ORC | SPEC_GOBLINOID,
      [__builtin_ctzll(SPEC_REVERED)]     = SPEC_GIANT,
    }
  },  
  {SPEC_RODENT,
    {
      [__builtin_ctzll(SPEC_FEAR)]        = SPEC_CANIFORM,
      [__builtin_ctzll(SPEC_AVOID)]       = SPEC_HUMAN | SPEC_ELF | SPEC_SKELETAL,
      [__builtin_ctzll(SPEC_INDIF)]       = SPEC_ARCHAIN | SPEC_ROTTING,
      [__builtin_ctzll(SPEC_CAUT)]        = SPEC_RUMINANT | SPEC_GIANT | SPEC_ETHEREAL | SPEC_VAMPIRIC,
      [__builtin_ctzll(SPEC_HUNTS)]       = SPEC_ARTHROPOD,
      [__builtin_ctzll(SPEC_FRIEND)]      = SPEC_ORC | SPEC_GOBLINOID,
      [__builtin_ctzll(SPEC_REVERED)]     = SPEC_SULKING,
    }
  },    
  {SPEC_RUMINANT,
    {
      [__builtin_ctzll(SPEC_FEAR)]        = SPEC_CANIFORM | SPEC_VAMPIRIC | SPEC_SULKING | SPEC_ORC | SPEC_GOBLINOID,
      [__builtin_ctzll(SPEC_AVOID)]       = SPEC_HUMAN | SPEC_SKELETAL | SPEC_ARTHROPOD,
        [__builtin_ctzll(SPEC_INDIF)]       = SPEC_ARCHAIN | SPEC_ROTTING,
        [__builtin_ctzll(SPEC_CAUT)]        = SPEC_RODENT | SPEC_GIANT | SPEC_ETHEREAL,
        [__builtin_ctzll(SPEC_FRIEND)]      = SPEC_ELF,
      }
    },  
    {SPEC_SULKING,
      {
        [__builtin_ctzll(SPEC_FEAR)]       = SPEC_GIANT | SPEC_ROTTING | SPEC_VAMPIRIC,
        [__builtin_ctzll(SPEC_AVOID)]      = SPEC_CANIFORM,
        [__builtin_ctzll(SPEC_HOSTILE)]    = SPEC_HUMAN | SPEC_ELF,    
        [__builtin_ctzll(SPEC_CAUT)]       = SPEC_ARCHAIN,     
        [__builtin_ctzll(SPEC_HUNTS)]      = SPEC_SKELETAL | SPEC_RUMINANT | SPEC_ARTHROPOD,
        [__builtin_ctzll(SPEC_FRIEND)]     = SPEC_ORC | SPEC_GOBLINOID, 
        [__builtin_ctzll(SPEC_REVERED)]  = SPEC_RODENT,
      }
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
  [ITEM_TOOL] = {ITEM_TOOL,.num_equip = 1, .on_equip = ItemAddAbility, .on_acquire = ItemAddUse},
  {ITEM_DONE}
};

weapon_def_t WEAPON_TEMPLATES[WEAP_DONE]= {
  {WEAP_NONE},
  {WEAP_MACE, "Mace",
    PROP_MAT_BONE | PROP_MAT_WOOD | PROP_MAT_STONE | PROP_MAT_METAL,
    PROP_WEAP_NONE,
    PROP_MAT_TOOLING,
    .abilities = ABILITY_WEAP_BLUDGEON,
    .skill = SKILL_WEAP_MACE,
    STORE_HELD,
    {[STORE_CARRY] = 2,[STORE_WORN]=1},
    .vals = {
      [VAL_WORTH]   = 250,[VAL_PENN]    = 0,
      [VAL_SCORE]   = 25, [VAL_WEIGHT]  = 1250,
      [VAL_SIZE]    = 512,[VAL_STORAGE] = 512,
      [VAL_DURI]    = 64, [VAL_DRAIN]   = 2,
      [VAL_REACH]   = 1,  [VAL_HIT]     = 14,
      [VAL_DMG]     = 2,  [VAL_DMG_DIE] = 3,
    },
    .loot = LF_WEAP | LF_MAT_WOOD | LF_MAT_STONE | LF_MAT_METAL
  },
  {WEAP_SWORD, "Sword",
    PROP_MAT_BONE | PROP_MAT_WOOD | PROP_MAT_STONE | PROP_MAT_METAL,
    PROP_WEAP_NONE,
    PROP_MAT_TOOLING | PROP_MAT_BLADE,
    .abilities = {ABILITY_WEAP_SLASH},
    .skill = SKILL_WEAP_SWORD,
     STORE_HELD,
    {[STORE_CARRY] = 2,[STORE_WORN]=1},
    .vals = {
      [VAL_WORTH]   = 600, [VAL_PENN]    = 0,
      [VAL_SCORE]   = 10,  [VAL_WEIGHT]  = 1000,
      [VAL_SIZE]    = 1024,[VAL_STORAGE] = 1024,
      [VAL_DURI]    = 32,  [VAL_DRAIN]   = 1,
      [VAL_REACH]   = 1,   [VAL_HIT]     = 10,
      [VAL_DMG]     = 6,   [VAL_DMG_DIE] = 1,
    },
    .loot = LF_WEAP | LF_MAT_METAL
  },
  {WEAP_AXE,"Axe",
    PROP_MAT_BONE | PROP_MAT_WOOD | PROP_MAT_STONE | PROP_MAT_METAL,
    PROP_WEAP_NONE,
    PROP_MAT_TOOLING | PROP_MAT_BLADE,
    .abilities = {ABILITY_WEAP_CHOP, ABILITY_TOOL_FELL}, 
    .skill = SKILL_WEAP_AXE,
    STORE_HELD,
    {[STORE_CARRY] = 2,[STORE_WORN]=1},
    .vals = {
      [VAL_WORTH]   = 300, [VAL_PENN]    = 2,
      [VAL_SCORE]   = 200, [VAL_WEIGHT]  = 1250,
      [VAL_SIZE]    = 768, [VAL_STORAGE] = 768,
      [VAL_DURI]    = 48,  [VAL_DRAIN]   = 1,
      [VAL_REACH]   = 1,   [VAL_HIT]     = 12,
      [VAL_DMG]     = 4,   [VAL_DMG_DIE] = 2,
     },
    .loot = LF_WEAP | LF_MAT_STONE | LF_MAT_METAL
  },
  {WEAP_DAGGER, "Dagger",
    PROP_MAT_BONE | PROP_MAT_WOOD | PROP_MAT_STONE | PROP_MAT_METAL,
    PROP_WEAP_NONE,
    PROP_MAT_TOOLING | PROP_MAT_BLADE,
    .abilities = {ABILITY_WEAP_STAB},
    .skill = SKILL_WEAP_DAGGER,
    STORE_HELD,
    {[STORE_CARRY] = 2,[STORE_WORN]=1},
    .vals = {
      [VAL_WORTH]   = 275,
      [VAL_PENN]    = 0,
      [VAL_SCORE]   = 15,
      [VAL_WEIGHT]  = 500,
      [VAL_SIZE]    = 256,
      [VAL_STORAGE] = 256,
      [VAL_DURI]    = 32,
      [VAL_DRAIN]   = 1,
      [VAL_REACH]   = 1,
      [VAL_HIT]     = 12,
    },
    .loot = LF_WEAP | LF_MAT_STONE | LF_MAT_METAL
  },
  {WEAP_JAVELIN, "Javelin",
    PROP_MAT_BONE | PROP_MAT_WOOD | PROP_MAT_STONE | PROP_MAT_METAL,
    PROP_WEAP_NONE,
    PROP_MAT_TOOLING | PROP_MAT_BLADE,
    .abilities = {ABILITY_WEAP_PIERCE},
    .skill = SKILL_WEAP_SPEAR,
    STORE_HELD,
    {[STORE_CARRY] = 2,[STORE_WORN]=1},
    .vals = {
      [VAL_WORTH]   = 5,
      [VAL_PENN]    = 0,
      [VAL_SCORE]   = 5,
      [VAL_WEIGHT]  = 750,
      [VAL_SIZE]    = 768,
      [VAL_STORAGE] = 768,
      [VAL_DURI]    = 4,
      [VAL_DRAIN]   = 1,
      [VAL_REACH]   = 2,
    },
    .loot = LF_WEAP | LF_MAT_WOOD | LF_MAT_METAL
  },
  {WEAP_BOW, "Bow",
    PROP_MAT_WOOD,
    PROP_WEAP_AMMO | PROP_WEAP_TWO_HANDED,
    PROP_MAT_TOOLING,
    .abilities = {ABILITY_WEAP_RANGE_PIERCE},
    .skill = SKILL_WEAP_BOW,
    STORE_HELD,
    {[STORE_CARRY] = 2,[STORE_WORN]=1},
    .vals = {
      [VAL_WORTH]   = 300,
      [VAL_PENN]    = 0,
      [VAL_SCORE]   = 10,
      [VAL_WEIGHT]  = 1250,
      [VAL_SIZE]    = 1536,
      [VAL_STORAGE] = 1536,
      [VAL_DURI]    = 128,
      [VAL_DRAIN]   = 1,
      [VAL_REACH]   = 3,
    },
    .loot = LF_WEAP | LF_MAT_WOOD
  },
};

tool_def_t TOOL_TEMPLATES[TOOL_DONE] = {
  [TOOL_KNIFE] = {TOOL_KNIFE, "Carving Knife",
    PROP_MAT_STONE | PROP_MAT_METAL | PROP_MAT_BONE,
    0,
    .vals = {
      [VAL_DMG]     = 4,
      [VAL_DMG_DIE] = 1,
      [VAL_ADV_HIT] = -1,
      [VAL_ADV_DMG] = -1,
      [VAL_REACH]   = 1,
      [VAL_WEIGHT]  = 400,
      [VAL_SIZE]    = 0x0100,
      [VAL_WORTH]   = 150,
      [VAL_SCORE]   = 25,
      [VAL_DRAIN]   = 1,
      [VAL_STORAGE] = 0x0080
    },
    .skills = {SKILL_WEAP_DAGGER},
    .ability = ABILITY_WEAP_STAB,
    .use = ABILITY_TOOL_SKIN,
    .reagents = RES_BONE | RES_STONE | RES_METAL,
    .icon = ICON_KNIFE,
    .loot = LF_TOOL | LF_RES_BEAST | LF_MAT_STONE | LF_MAT_METAL
  },
  [TOOL_PICKAXE] = { TOOL_PICKAXE, "Pickaxe",
    PROP_MAT_STONE | PROP_MAT_METAL,
    0,
    .vals = {
      [VAL_DMG]     = 4,
      [VAL_DMG_DIE] = 1,
      [VAL_ADV_HIT] = -1,
      [VAL_ADV_DMG] = -1,
      [VAL_REACH]   = 1,
      [VAL_WEIGHT]  = 700,
      [VAL_SIZE]    = 0x0300,
      [VAL_WORTH]   = 250,
      [VAL_SCORE]   = 25,
      [VAL_DRAIN]   = 2,
      [VAL_STORAGE] = 0x0100
    },
    .skills = {SKILL_WEAP_PICK},
    .use = ABILITY_TOOL_MINE,
    .reagents = RES_STONE | RES_METAL,
    .icon = ICON_PICK,
    .loot = LF_TOOL | LF_RES_STONE | LF_MAT_STONE | LF_MAT_METAL
  },
  [TOOL_HATCHET] = { TOOL_HATCHET, "Hatchet",
        PROP_MAT_STONE | PROP_MAT_METAL,
    0,
    .vals = {
      [VAL_DMG]     = 4,
      [VAL_DMG_DIE] = 1,
      [VAL_ADV_HIT] = -1,
      [VAL_ADV_DMG] = -1,
      [VAL_REACH]   = 1,
      [VAL_WEIGHT]  = 700,
      [VAL_SIZE]    = 0x0300,
      [VAL_WORTH]   = 325,
      [VAL_SCORE]   = 500,
      [VAL_DRAIN]   = 2,
      [VAL_STORAGE] = 0x0100
    },
    .skills = {SKILL_WEAP_AXE},
    .ability = ABILITY_WEAP_CHOP,
    .use = ABILITY_TOOL_FELL,
    .reagents = RES_STONE | RES_METAL,
    .icon = ICON_HATCHET,
    .loot = LF_TOOL | LF_RES_WOOD | LF_MAT_STONE | LF_MAT_METAL
  },
  [TOOL_CRUCIBLE] = { TOOL_CRUCIBLE, "Crucible",
    PROP_MAT_STONE,
    .vals = {
      [VAL_SIZE]    = 1024,
      [VAL_WEIGHT]  = 768,
      [VAL_STORAGE] = 512,
      [VAL_WORTH]   = 150,
      [VAL_SCORE]   = 20,
      [VAL_DMG]     = 4,
      [VAL_DMG_DIE] = 1
    },
    .skills = {SKILL_SURV},
    .use = ABILITY_TOOL_SMELT,
    .icon = ICON_BUCKET,
  }
};

WeaponType GetWeapTypeBySkill(SkillType s){
  for(int i = 0; i < WEAP_DONE; i++){
    if(WEAPON_TEMPLATES[i].skill == s)
      return WEAPON_TEMPLATES[i].type;
  }
}

armor_def_t ARMOR_TEMPLATES[ARMOR_DONE]={
  {ARMOR_NONE},
  {ARMOR_NATURAL,"", 
    .skill = SKILL_ARMOR_NATURAL,
    .vals = {
      [VAL_SAVE] = 4,
    }
  },
  {ARMOR_CLOTH, "Shirt",
    {},
    {},
    ATTR_NONE, ATTR_NONE, 0, 0,
    .i_props = PROP_MAT_CLOTH,
    .m_props = PROP_MAT_CLOTHING,
    .skill = SKILL_ARMOR_CLOTH,
    STORE_WORN,
    {[STORE_CARRY] = 3, [STORE_CONTAINER]=1},
    .vals = {
      [VAL_SAVE]    = 6,
      [VAL_WEIGHT]  = 1,
      [VAL_WORTH]   = 5,
      [VAL_DURI]    = 50,
      [VAL_STORAGE] = 1792, 
      [VAL_SIZE]    = 1792, 
      [VAL_SCORE]   = 25,
    },
    .loot = LF_ARMOR | LF_MAT_CLOTH
  },
  {ARMOR_PADDED, "Shirt",
    {},
    {},
    ATTR_DEX,ATTR_NONE,10,0,
   .i_props = PROP_MAT_CLOTH | PROP_MAT_FUR,
    .m_props = PROP_MAT_CLOTHING,
   .skill = SKILL_ARMOR_PADDED,
    STORE_WORN,
    {[STORE_CARRY] = 3, [STORE_CONTAINER]=1},
    .vals = {
      [VAL_SAVE]    = 8,
      [VAL_WEIGHT]  = 2,
      [VAL_WORTH]   = 6,
      [VAL_DURI]    = 100,
      [VAL_STORAGE] = 4096,
      [VAL_SIZE]    = 4096,
      [VAL_SCORE]   = 20,
    },
    .loot = LF_ARMOR | LF_MAT_CLOTH
  },
  {ARMOR_LEATHER, "Vest", 
    {{[DMG_SLASH]=1,[DMG_BLUNT]=1},{}},
    {{[DMG_SLASH]=1,[DMG_PIERCE]=1},{}},
    ATTR_DEX,ATTR_NONE,10,0,
    .i_props = PROP_MAT_LEATHER | PROP_MAT_HIDE,
    .m_props = PROP_MAT_CLOTHING,
    .skill = SKILL_ARMOR_LEATHER,
    STORE_WORN,
    {[STORE_CARRY] = 3, [STORE_CONTAINER]=1},
    .vals = {
      [VAL_SAVE]    = 10,
      [VAL_WEIGHT]  = 4,
      [VAL_WORTH]   = 10,
      [VAL_DURI]    = 200,
      [VAL_STORAGE] = 4096,
      [VAL_SIZE]    = 4096,
      [VAL_SCORE]   = 15,
    },
    .loot = LF_ARMOR | LF_MAT_HIDE

  },
  {ARMOR_CHAIN, "Mail Shirt", 
    {{[DMG_SLASH]=2,[DMG_PIERCE]=1},{}},
    {{[DMG_SLASH]=1,[DMG_PIERCE]=1},{}},
    ATTR_DEX,ATTR_STR,2,9,
    .i_props = PROP_MAT_METAL,
    .m_props = PROP_MAT_CLOTHING,
    .skill = SKILL_ARMOR_CHAIN,
    STORE_WORN,
    {[STORE_CARRY] = 3, [STORE_CONTAINER]=1},
    .vals = {
      [VAL_SAVE]    = 12,
      [VAL_WEIGHT]  = 8,
      [VAL_WORTH]   = 25,
      [VAL_DURI]    = 400,
      [VAL_STORAGE] = 2048,
      [VAL_SIZE]    = 2048,
      [VAL_SCORE]   = 10,
      [VAL_DRAIN]   = 1,
    },
    .loot = LF_ARMOR | LF_MAT_METAL
  },
  {ARMOR_PLATE, "Curaiss",
    {{[DMG_SLASH]=2,[DMG_PIERCE]=2,[DMG_BLUNT]=1},{}},
    {{[DMG_SLASH]=1,[DMG_PIERCE]=1}},
    20,100,800,ATTR_NONE,ATTR_STR,0,12,
    .i_props = PROP_MAT_METAL,
    .m_props = PROP_MAT_CLOTHING,
    .skill = SKILL_ARMOR_PLATE,
    STORE_WORN,
    {[STORE_CARRY] = 3, [STORE_CONTAINER]=1},
    .vals = {
      [VAL_SAVE]    = 16,
      [VAL_WEIGHT]  = 16,
      [VAL_WORTH]   = 100,
      [VAL_DURI]    = 800,
      [VAL_STORAGE] = 4096,
      [VAL_SIZE]    = 4096,
      [VAL_SCORE]   = 5,
      [VAL_DRAIN]   = 2
    },
    .loot = LF_ARMOR | LF_MAT_METAL
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
    PROP_MAT_LIQUID, PROP_CONS_HEALTH | PROP_CONS_RESTORE,
    ABILITY_ITEM_HEAL, SKILL_ALCH,
    STORE_SPECIAL,
    {[STORE_HELD] = 5, [STORE_CONTAINER] = 1},
    0x0020,
    "Potion of ",
    .loot = LF_CONS | LF_POT
  },
  {CONS_FOOD},
  {CONS_DRINK},
  {CONS_SCROLL,
    75, 200, 1, 2, 5,
    PROP_MAT_CLOTH | PROP_QUAL_FINE, 0,
    ABILITY_ITEM_TOME, SKILL_LING,
    STORE_SPECIAL,
    {[STORE_HELD] = 5, [STORE_CONTAINER] = 3},
    0x0050,
    "Scroll of ",
    .loot = LF_CONS | LF_SCROLL
  },
  {CONS_TOME,
    250, 1250, 1, 1, 20,
    PROP_MAT_LEATHER | PROP_QUAL_FINE, 0,
    ABILITY_ITEM_TOME, SKILL_LING,
    STORE_SPECIAL,
    {[STORE_HELD] = 5, [STORE_CONTAINER] = 3},
    0X1250,
    "Tome of ",
    .loot = LF_CONS | LF_TOME
  },
  {CONS_SKILLUP,
     200, 500, 1, 25, 50,
    PROP_MAT_LEATHER | PROP_QUAL_FINE, 0,
    ABILITY_ITEM_SKILL, SKILL_LING,
    STORE_SPECIAL,
    {[STORE_HELD] = 5, [STORE_CONTAINER] = 3},
    0X1000,
    "Manual of ",
     .vals = {[VAL_DMG] = 6, [VAL_DMG_DIE] = 10},  
    .loot = LF_CONS | LF_MANUAL
  }
};

container_def_t CONTAINER_TEMPLATES[INV_DONE]={
  {INV_HELD},
  {INV_WORN},
  {INV_BACK},
  {INV_BELT},
  {"Sling", INV_SLING, 1250, 25, 4, .size = 0x0100, 0x0020}
};
define_natural_armor_t NAT_ARMOR_TEMPLATES[13] = {
  {PQ_FUR, ARMOR_NATURAL, 5,
    {{[DMG_COLD] = 1}}
  },
  {PQ_THICK_FUR, ARMOR_NATURAL, 5,
    {{[DMG_COLD] = 2, [DMG_SLASH] = 1}}
  },
  {PQ_THICK_SKIN, ARMOR_NATURAL, 4,
    {{[DMG_BLUNT] = 1, [DMG_POISON] = 1, [DMG_FIRE] = 1, [DMG_COLD] = 1}}
  },
  {PQ_THICK_FAT, ARMOR_NATURAL, 4,
      {{[DMG_BLUNT] = 1, [DMG_COLD] = 1}}
  },
  {PQ_HIDE, ARMOR_NATURAL, 7},
  {PQ_THICK_HIDE, ARMOR_NATURAL, 8,
      {{[DMG_FIRE] = 1, [DMG_COLD] = 1},
        {[DMGTAG_PHYSICAL] = 1}}

  },
  {PQ_TOUGH_HIDE, ARMOR_NATURAL, 8},
  {PQ_SCALES, ARMOR_NATURAL, 7},
  {PQ_THICK_SCALES, ARMOR_NATURAL, 8},
  {PQ_TOUGH_SCALES, ARMOR_NATURAL, 8},
  {PQ_STONE_SKIN, ARMOR_NATURAL, 11},
  {PQ_METALLIC, ARMOR_NATURAL, 14},
  {PQ_CHITIN, ARMOR_NATURAL, 4,
    {{[DMG_POISON] = 1 }}
  },

};
material_data_t MATERIAL_DATA[MAT_ALL] = {
  [MAT_WOOD_ASH]        = {MAT_WOOD_ASH, MAT_WOOD, "Ash"},
  [MAT_WOOD_BEECH]      = {MAT_WOOD_BEECH, MAT_WOOD, "Beech",
   PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
        BEIGE, ICON_WOOD,
        .vals = {
          [VAL_WEIGHT] = 750,
          [VAL_WORTH]  = 2
        }
  },
  [MAT_WOOD_BIRCH]      = {MAT_WOOD_BIRCH, MAT_WOOD, "Birch"},
  [MAT_WOOD_CEDAR]      = {MAT_WOOD_CEDAR, MAT_WOOD, "Cedar"},
  [MAT_WOOD_CHERRY]     = {MAT_WOOD_CHERRY, MAT_WOOD, "Cherry"},
  [MAT_WOOD_CYPRESS]    = {MAT_WOOD_CYPRESS, MAT_WOOD, "Cypress"},
  [MAT_WOOD_ELM]        = {MAT_WOOD_ELM, MAT_WOOD, "Elm"},
  [MAT_WOOD_FIR]        = {MAT_WOOD_FIR, MAT_WOOD, "Fir",
   PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
        BEIGE, ICON_WOOD,
        .vals = {
          [VAL_WEIGHT] = 500,
          [VAL_WORTH]  = 1
        }

  },
  [MAT_WOOD_HELVORN]    = {MAT_WOOD_HICKORY, MAT_WOOD, "Helvorn",
       PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
       FROSTGRAY,
       .vals = {
         [VAL_WEIGHT] = 1000,
         [VAL_WORTH]  = 28
       }
  },
  [MAT_WOOD_HICKORY]    = {MAT_WOOD_HICKORY, MAT_WOOD, "Hickory"},
  [MAT_WOOD_IRON]       = {MAT_WOOD_IRON, MAT_WOOD, "Ironwood",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
    DARKGRAY,
    .vals = {
      [VAL_WEIGHT] = 1337,
      [VAL_WORTH]  = 6
    }
  },
  [MAT_WOOD_MAPLE]      = {MAT_WOOD_MAPLE, MAT_WOOD, "Maple"},
  [MAT_WOOD_OAK]        = {MAT_WOOD_OAK, MAT_WOOD, "Oak",
        PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
        BROWN, ICON_WOOD,
        .vals = {
          [VAL_WEIGHT] = 700,
          [VAL_WORTH]  = 3
        }
  },
  [MAT_WOOD_PINE]       = {MAT_WOOD_PINE, MAT_WOOD, "Pine"},
  [MAT_WOOD_POPLAR]     = {MAT_WOOD_POPLAR, MAT_WOOD, "Poplar"},
  [MAT_WOOD_REDWOOD]    = {MAT_WOOD_REDWOOD, MAT_WOOD, "Redwood"},
  [MAT_WOOD_SPRUCE]     = {MAT_WOOD_SPRUCE, MAT_WOOD, "Spruce",
   PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
        DARKBROWN, ICON_WOOD,
        .vals = {
          [VAL_WEIGHT] = 425,
          [VAL_WORTH]  = 1
        }

  },
  [MAT_WOOD_WALNUT]     = {MAT_WOOD_WALNUT, MAT_WOOD, "Walnut"},
  [MAT_WOOD_WYRD]     = {MAT_WOOD_WYRD, MAT_WOOD, "Wyrdwood",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
    COLDGRAY, ICON_WOOD,
    .vals = {
      [VAL_WEIGHT] = 700,
      [VAL_WORTH]  = 24        
    }
  },

  [MAT_STONE_CASSITERITE] = {MAT_STONE_CASSITERITE, MAT_STONE,
    "Cassiterite",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
    SILVER, ICON_SHARD, ENV_OXIDE,
    .vals = {
      [VAL_WORTH] = 16,
      [VAL_WEIGHT] = 6850
    }
  },
  [MAT_STONE_CHALCOPY]   = {MAT_STONE_CHALCOPY, MAT_STONE, 
    "Chalcopyrite",
     PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
     COPPER, ICON_SHARD, ENV_OXIDE,
     .vals = {
       [VAL_WORTH] = 14,
       [VAL_WEIGHT] = 4150
     }
  },
  [MAT_STONE_CHERT]       = {MAT_STONE_CHERT, MAT_STONE, "Chert",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE |PROP_MAT_TOOLING,
    BEIGE, ICON_SHARD, ENV_OXIDE,
    .vals = {
      [VAL_WORTH] = 8,
      [VAL_WEIGHT] = 2750
    }
  },
  [MAT_STONE_DOLOMITE]  = {MAT_STONE_DOLOMITE, MAT_STONE, "Dolomite",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
    RAYWHITE, 0,
    .vals = {
      [VAL_WORTH] = 3,
      [VAL_WEIGHT] = 2850
    }
  },  
  [MAT_STONE_FELDSPAR] = {MAT_STONE_FELDSPAR, MAT_STONE, "Feldspar",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
    BRONZE, 0,
    .vals = {
      [VAL_WORTH] = 2,
      [VAL_WEIGHT] = 2600
    }
  },
  [MAT_STONE_FLINT]  = {MAT_STONE_FLINT, MAT_STONE, "Obsidian",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE |PROP_MAT_TOOLING,
    DARKGRAY, .icon = ICON_SHARD,
    .vals = {
      [VAL_WORTH] = 10,
      [VAL_WEIGHT] = 2750
    }
  },
  [MAT_STONE_GNEISS]    = {MAT_STONE_GNEISS, MAT_STONE, "Gneiss",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
    .col = LIGHTGRAY,
    .vals = {
      [VAL_WORTH] = 5
    }
  },
  [MAT_STONE_GRANITE]   = {MAT_STONE_GRANITE, MAT_STONE, "Granite",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
    .col = REDDISHBROWN,
    .vals = {
      [VAL_WORTH] = 5,
      [VAL_WEIGHT] = 2800
    }
  },
  [MAT_STONE_HEMATITE] = {MAT_STONE_HEMATITE, MAT_STONE, "Hematite",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
    RUST, ICON_SHARD, ENV_OXIDE,
    .vals = {
      [VAL_WEIGHT] = 4950,
      [VAL_WORTH] = 18
    }
  },
  [MAT_STONE_MALCHITE] = {MAT_STONE_MALCHITE, MAT_STONE, "Malachite",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
    GREEN, ICON_SHARD, ENV_OXIDE,
    .vals = {
      [VAL_WORTH] = 28,
      [VAL_WEIGHT] = 3750
    }
  },
  [MAT_STONE_MAGNETITE] = {MAT_STONE_MAGNETITE, MAT_STONE, "Magnetite",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
    REDDISHBROWN, ICON_SHARD, ENV_OXIDE,
    .vals = {
      [VAL_WEIGHT] = 5150,
      [VAL_WORTH]  = 16
    }
  },
  [MAT_STONE_OBSIDIAN]  = {MAT_STONE_OBSIDIAN, MAT_STONE, "Obsidian",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE | PROP_MAT_TOOLING,
    .col = BLACK, .icon = ICON_SHARD,
    .vals = {
      [VAL_WEIGHT] = 2250,
      [VAL_WORTH]  = 12,
    }
  },
  [MAT_STONE_LIMESTONE] = {MAT_STONE_LIMESTONE, MAT_STONE, "Limestone",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
    .col = LIMESTONE
  },
  [MAT_STONE_SANDSTONE] = {MAT_STONE_SANDSTONE, MAT_STONE, "Sandstone",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
    .col = SANDSTONE,
    .vals = {
      [VAL_WORTH] = 2,
      [VAL_WEIGHT] = 2800
    }
  },
  [MAT_STONE_WOLFRAMITE] = {MAT_STONE_WOLFRAMITE, MAT_STONE, "Wolframite",
        PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
        DARKGRAY, ICON_SHARD, ENV_OXIDE,
        .vals = {
          [VAL_WORTH] = 21,
          [VAL_WEIGHT] 7450
        }
  },
  [MAT_METAL_ADAMANT]   = {MAT_METAL_ADAMANT, MAT_METAL, "Adamant",
    PROP_MAT_TOOLING | PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE,
    .col = SKYBLUE,
    .vals = {
      [VAL_WORTH] = 250,
      [VAL_WEIGHT] = 9000
    }
  },
  [MAT_METAL_BRONZE]    = {MAT_METAL_BRONZE, MAT_METAL, "Bronze",
    PROP_MAT_TOOLING | PROP_MAT_BLADE,
   .col =  BRONZE,
   .vals = {
     [VAL_WEIGHT] = 8800,
     [VAL_WORTH]  = 400, 
   },
  },
  [MAT_METAL_COPPER]    = {MAT_METAL_COPPER, MAT_METAL, "Copper",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE | PROP_MAT_TOOLING | PROP_MAT_BLADE,
    .col = COPPER,
    .vals = {
      [VAL_WEIGHT] = 8900,
      [VAL_WORTH]  = 10
    }
  },
  [MAT_METAL_IRON]      = { MAT_METAL_IRON, MAT_METAL, "Iron",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE | PROP_MAT_TOOLING | PROP_MAT_BLADE,
    .col = GRAY,
    .vals = {
      [VAL_WEIGHT] = 7850,
      [VAL_WORTH]  = 25
    }
  },
  [MAT_METAL_MITHRIL]   = { MAT_METAL_MITHRIL, MAT_METAL, "Mithril",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE | PROP_MAT_TOOLING | PROP_MAT_BLADE,
    .col = RAYWHITE,
    .vals = {
      [VAL_WEIGHT] = 4750,
      [VAL_WORTH]  = 700,
    }
  },
  [MAT_METAL_ORICHAL]   = {MAT_METAL_ORICHAL, MAT_METAL, "Orichalcum",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE | PROP_MAT_TOOLING | PROP_MAT_BLADE,
    .col = REDDISHBROWN,
    .vals = {
      [VAL_WEIGHT] = 8800,
      [VAL_WORTH]  = 550,
    }
  },
  [MAT_METAL_STEEL]     = {MAT_METAL_STEEL, MAT_METAL, "Steel",
    PROP_MAT_TOOLING | PROP_MAT_BLADE,
    .col = LIGHTGRAY,
    .vals = {
      [VAL_WORTH] = 120,
      [VAL_WEIGHT] 7850
    }
  },
  [MAT_METAL_TIN]       = { MAT_METAL_TIN, MAT_METAL, "Tin",
    PROP_MAT_TOOLING,
    SILVER,
    .vals = {
      [VAL_WORTH] = 30,
      [VAL_WEIGHT] = 7300
    }
  },
  [MAT_METAL_WOLFRAM]   = { MAT_METAL_WOLFRAM, MAT_METAL, "Wolfram",
    PROP_MAT_RAW | PROP_MAT_HAS_RESOURCE | PROP_MAT_TOOLING, 
    .col = DARKGRAY,
    .vals = {
      [VAL_WEIGHT] = 11350,
      [VAL_WORTH]  = 700
    }
  },
  [MAT_GEM_CALCITE] = {MAT_GEM_CALCITE, MAT_GEM, "Calcite",
    .vals = {
      [VAL_WEIGHT] = 2700,
      [VAL_WORTH]  = 3
    }
  },
  [MAT_GEM_MALACHITE] = {MAT_GEM_MALACHITE, MAT_GEM, "Malachite",
    .vals = {
      [VAL_WEIGHT] = 3600,
      [VAL_WORTH]  = 30
    }
  },
  [MAT_GEM_PYRITE] = {MAT_GEM_PYRITE, MAT_GEM, "Pyrite",
    .vals = {
      [VAL_WEIGHT] = 4850,
      [VAL_WORTH] = 3
    }
  },
  [MAT_GEM_QUARTZ] = {MAT_GEM_QUARTZ, MAT_GEM, "Quartz",
    .vals = {
      [VAL_WEIGHT] = 2650,
      [VAL_WORTH] = 4
    }
  },
  [MAT_GEM_TOPAZ] = {MAT_GEM_TOPAZ, MAT_GEM, "Topaz",
  .vals = {
      [VAL_WEIGHT] = 3500,
      [VAL_WORTH] = 50
    }
  },
  [MAT_GEM_TOURMALINE] = {MAT_GEM_TOURMALINE, MAT_GEM, "Tourmaline",
    .vals = {
      [VAL_WEIGHT] = 3150,
      [VAL_WORTH] = 100
    }
  },
};

material_def_t MATERIAL_TEMPLATES[MAT_DONE] = {
  {MAT_CLOTH,
    PROP_MAT_CLOTH,
    PROP_MAT_CLOTHING,
    .vals ={
      [VAL_STORAGE] = 400,
      [VAL_WEIGHT]  = 15,
    },
    .score = 25,
    .cost = 50,
    .loot = LF_MAT | LF_MAT_CLOTH
  },
  {MAT_HIDE,
    PROP_MAT_HIDE,
    PROP_MAT_CLOTHING,
    .vals ={
      [VAL_STORAGE] = 400,
      [VAL_WEIGHT]  = 50,
    },
    .score = 50,
    .cost = 30,
    .loot = LF_MAT | LF_MAT_HIDE
  },
  {MAT_FUR,
    PROP_MAT_FUR,
    PROP_MAT_CLOTHING,
    .vals ={
      [VAL_STORAGE] = 400,
      [VAL_WEIGHT]  = 25, 
    },
    .score = 40,
    .cost = 20,
    .loot = LF_MAT | LF_MAT_HIDE
  },
  {MAT_LEATHER,
    PROP_MAT_LEATHER,
    PROP_MAT_CLOTHING,
    .vals ={
      [VAL_STORAGE] = 400,
      [VAL_WEIGHT]  = 50,
    },
    .score = 30,
    .cost = 20,
    .loot = LF_MAT | LF_MAT_HIDE
  },
  {MAT_BONE,
    PROP_MAT_BONE,
    PROP_MAT_HAS_RESOURCE,
    RES_BONE,
    .vals ={
      [VAL_STORAGE] = 200,
      [VAL_WEIGHT]  = 1500,
      [VAL_WORTH]   = 10
    },
    .score = 20,
    .cost = 25,
    .loot = LF_MAT | LF_MAT_BONE
  },
  {MAT_WOOD,
    PROP_MAT_WOOD,
    PROP_MAT_HAS_RESOURCE,
    RES_WOOD,
    .icon = ICON_WOOD,
    .vals ={
      [VAL_STORAGE] = 2000,
      [VAL_WEIGHT]  = 1500,
    },
    .score = 100,
    .cost = 10,
    .loot = LF_MAT | LF_MAT_WOOD
  },
  {MAT_STONE,
    PROP_MAT_STONE,
    PROP_MAT_HAS_RESOURCE,
    RES_STONE,
    .icon = ICON_ROCKS,
    .vals ={
      [VAL_STORAGE] = 1000,
      [VAL_WEIGHT]  = 3000,
      [VAL_WORTH]   = 1
    },
    .score = 50,
    .cost = 10,
    .loot = LF_MAT | LF_MAT_STONE
  },
  {MAT_GEM,
    PROP_MAT_GEM,

    .loot = LF_MAT | LF_MAT_GEM
  },
  {MAT_METAL,
    PROP_MAT_METAL,
    PROP_MAT_CLOTHING,
    RES_METAL,
    .icon = ICON_INGOT,
    .vals = {
      [VAL_STORAGE] = 1000,
      [VAL_WEIGHT]  = 7000,
      [VAL_WORTH]   = 5,
    },
    .score = 20,
    .cost = 50,
    .loot = LF_MAT | LF_MAT_METAL
  }
};

material_properties_t MATERIAL_COMP[MAT_ALL] = {
  [MAT_WOOD_BEECH] = {MAT_WOOD_BEECH, 1,
    {
      {MAT_WOOD_BEECH, 75, 2, 3, .method = SKILL_WOOD}
    }
  },
  [MAT_WOOD_FIR] = {MAT_WOOD_FIR, 1,
    {
      {MAT_WOOD_FIR, 75, 1, 3, .method = SKILL_WOOD}
    }
  },
  [MAT_WOOD_OAK] = {MAT_WOOD_OAK, 1,
    {
      {MAT_WOOD_OAK, 75, 3, 4, .method = SKILL_WOOD}
    }
  },
  [MAT_WOOD_SPRUCE] = {MAT_WOOD_SPRUCE, 1,
    {
      {MAT_WOOD_SPRUCE, 75, 1, 3, .method = SKILL_WOOD}
    }
  },
  [MAT_STONE_DOLOMITE] = {MAT_STONE_DOLOMITE, 1,
    {
      {MAT_STONE_DOLOMITE, 50, 0, 1, 
        .method = SKILL_STONE}
    }
  },
  [MAT_STONE_FELDSPAR] = {MAT_STONE_FELDSPAR, 1,
    {
      {MAT_STONE_FELDSPAR, 50, 0, 1, .method=SKILL_STONE}
    }
  },
  [MAT_STONE_FLINT] = {MAT_STONE_FLINT, 1,
    {
      {MAT_STONE_FLINT, 24, 1, 2, .method=SKILL_STONE}
    }
  },
  [MAT_STONE_CHERT] = {MAT_STONE_CHERT, 1,
    {
      {MAT_STONE_CHERT, 24, 1, 2, .method=SKILL_STONE}
    }
  },

  [MAT_STONE_GNEISS] = {MAT_STONE_GNEISS, 1,
    {
      {MAT_STONE_GNEISS, 54, 0, 1, .method=SKILL_STONE}
    }
  },
  [MAT_STONE_GRANITE] = {MAT_STONE_GRANITE, 1,
    {
      {MAT_STONE_GRANITE, 54, 0, 1, .method=SKILL_STONE}
    }
  },
  [MAT_STONE_OBSIDIAN] = {MAT_STONE_OBSIDIAN, 1,
    {
      {MAT_STONE_OBSIDIAN, 17, 2, 3, .method=SKILL_STONE}
    }
  },
  [MAT_STONE_LIMESTONE] = {MAT_STONE_LIMESTONE, 1,
    {
      {MAT_STONE_LIMESTONE, 54, 0, 1, .method=SKILL_STONE}
    }
  },
  [MAT_STONE_SANDSTONE] = {MAT_STONE_SANDSTONE, 1,
    {
      {MAT_STONE_SANDSTONE, 54, 0, 1, .method=SKILL_STONE}
    }
  },
  [MAT_STONE_CHALCOPY] = {MAT_STONE_CHALCOPY, 6,
    {
      {MAT_METAL_COPPER, 7, 1, 3, .method=SKILL_SMELT},
      {MAT_STONE_CHALCOPY, 14, 3, 3, .method=SKILL_STONE},
      {MAT_STONE_LIMESTONE, 33, 0, 1, .method=SKILL_STONE},
      {MAT_STONE_DOLOMITE, 17, 0, 1, .method=SKILL_STONE},
      {MAT_GEM_PYRITE, 17, 1, 3, .method=SKILL_JEWL},
      {MAT_GEM_QUARTZ, 33, 0, 2, .method=SKILL_JEWL},
    } 
  },
  [MAT_STONE_MALCHITE] =  {MAT_STONE_MALCHITE, 6,
    {
      {MAT_METAL_COPPER, 14, 1, 2, .method=SKILL_SMELT},
      {MAT_STONE_MALCHITE, 28, 2, 3, .method=SKILL_STONE},
      {MAT_STONE_DOLOMITE, 33, 0, 1, .method=SKILL_STONE},
      {MAT_GEM_QUARTZ, 17, 0, 1, .method=SKILL_JEWL},
      {MAT_GEM_MALACHITE, 33, 1, 4, .method=SKILL_JEWL},
      {MAT_GEM_CALCITE, 33, 0, 1, .method=SKILL_JEWL},
    }
  },
  [MAT_STONE_MAGNETITE] = {MAT_STONE_MAGNETITE, 4,
    {
      {MAT_METAL_IRON, 17, 2, 4, .method=SKILL_SMELT},
      {MAT_STONE_MAGNETITE, 34, 4, 3, .method=SKILL_STONE},
      {MAT_GEM_QUARTZ, 33, 0, 1, .method=SKILL_JEWL},
      {MAT_STONE_FELDSPAR, 33, 0, 1, .method=SKILL_STONE},
    }
  },
  [MAT_STONE_HEMATITE] = {MAT_STONE_HEMATITE, 5,
    {
      {MAT_METAL_IRON, 11, 2, 3, .method=SKILL_SMELT},
      {MAT_STONE_HEMATITE, 24, 4, 3, .method=SKILL_STONE},
      {MAT_GEM_QUARTZ, 17, 0, 1, .method=SKILL_JEWL},
      {MAT_STONE_FELDSPAR, 33, 0, 1, .method=SKILL_STONE},
      {MAT_GEM_CALCITE, 17, 1, 2, .method=SKILL_JEWL}
    }
  },
  [MAT_STONE_WOLFRAMITE] = {MAT_STONE_WOLFRAMITE, 5,
    {
      {MAT_METAL_WOLFRAM, 3, 7, 5,.method=SKILL_SMELT},
      {MAT_STONE_WOLFRAMITE, 7, 5, 3, .method=SKILL_STONE},
      {MAT_GEM_QUARTZ, 34, 0, 1, .method=SKILL_JEWL},
      {MAT_STONE_FELDSPAR, 33, 0, 1, .method=SKILL_STONE},
      {MAT_STONE_CASSITERITE, 17, 7, 1, .method=SKILL_STONE}
    }
  },
  [MAT_STONE_CASSITERITE] = {MAT_STONE_CASSITERITE, 6,
    {
      {MAT_METAL_TIN, 1, 5, 4, .method=SKILL_SMELT},
      {MAT_STONE_CASSITERITE, 4, 4, 3, .method=SKILL_STONE},
      {MAT_GEM_TOPAZ, 3, 5, 4, .method=SKILL_JEWL},
      {MAT_GEM_TOURMALINE, 4, 5, 4, .method=SKILL_JEWL},
      {MAT_GEM_QUARTZ, 34, 0, 1, .method=SKILL_JEWL},
      {MAT_STONE_FELDSPAR, 34, 0, 1, .method=SKILL_STONE}
    }
  }
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
    .materials = {
      [MAT_STONE_GNEISS] = 1, [MAT_STONE_CHERT] = 2,
      [MAT_STONE_GRANITE] = 5, [MAT_STONE_LIMESTONE] = 5, 
      [MAT_STONE_SANDSTONE] = 5, [MAT_STONE_FLINT] = 4,
      [MAT_WOOD_BEECH] = 25, [MAT_WOOD_SPRUCE] = 30, 
      [MAT_WOOD_OAK] = 5, [MAT_WOOD_FIR] = 15,
      [MAT_STONE_CASSITERITE] = 2, [MAT_STONE_MAGNETITE] = 2, 
      [MAT_STONE_WOLFRAMITE] = 1
    }
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

enum_map_t MAT_RES_MAP[MAT_DONE] = {
  {MAT_CLOTH},
  {MAT_HIDE,    RES_HIDE},
  {MAT_FUR,     RES_FUR},
  {MAT_LEATHER, RES_HIDE},
  {MAT_BONE,    RES_BONE},
  {MAT_WOOD,    RES_WOOD},
  {MAT_STONE,   RES_STONE},
  {MAT_METAL,   RES_METAL},
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
  faction_t* f = GameCalloc("InitFaction", 1, sizeof(faction_t));

  Faction id = hash_str_32(name);

  f->id = id;
  f->name = strdup(name);

  FACTIONS[NUM_FACTIONS++] = f;
  return f;
}

const char* Faction_Name(Faction id){

}



dice_roll_t* Die(int side, int num){
  dice_roll_t* die = GameMalloc("Die", sizeof(dice_roll_t));

  *die = (dice_roll_t){
    .sides = side,
    .num_die = num,
    .roll = RollDie
  };

  return die;
}

dice_roll_t* InitDie(int side, int num, int adv, DiceRollFunction fn){
  dice_roll_t* die = GameMalloc("InitDie", sizeof(dice_roll_t));

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
  attribute_t* t = GameCalloc("InitAttribute", 1,sizeof(attribute_t));

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
  return true;
}
// Allocates a copy of the filename without extension
char* GetFileStem(const char* filename) {
    const char* dot = strrchr(filename, '.');
    size_t len = dot ? (size_t)(dot - filename) : strlen(filename);

    char* stem = GameMalloc("GetFileStem", len + 1);
    if (!stem) return NULL;
    memcpy(stem, filename, len);
    stem[len] = '\0';
    return stem;
}

value_t* InitValue(ValueCategory cat, int base){
  value_t* v = GameCalloc("InitValue", 1,sizeof(value_t));
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

void ValueSet(value_t* self, int amnt){
  self->base = amnt;
  self->val = ValueRebase(self);
}

void ValueDecrease(value_t* self, int amnt){
  self->val -= amnt;

  if(self->on_change)
    self->on_change(self, self->context);

  if(self->val <= 0 && self->on_empty)
    self->on_empty(self, self->context);
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
  value_affix_t* va = GameCalloc("InitValueAffixFromMod", 1,sizeof(value_affix_t));

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
 stat_t* s = GameCalloc("InitStatOnMin", 1,sizeof(stat_t));
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
 stat_t* s = GameMalloc("InitStat", sizeof(stat_t));
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

bool StatIncrementValue(stat_t* s, bool increase){
  float inc = s->increment;
  if(!increase)
    inc*=-1;

  float old = s->current;
  if(old+inc<s->min)
    return false;

  return StatChangeValue(s->owner, s, inc);
}

bool StatChangeValue(struct ent_s* owner, stat_t* s, float val){
  float old = s->current;
  s->current+=val;
  s->current = CLAMPF(s->current,s->min, s->max);
  float cur = s->current;
  if(s->current == old) 
    return false;

  if(!owner)
    return true;

  if(owner->needs && s->need>N_NONE)
    NeedIncrement(owner->needs[s->need], s->owner, val);

  if(s->on_stat_change != NULL)
    s->on_stat_change(s,old, cur);
  
  if(s->on_change && s->on_change_data)
    s->on_change(owner, s->on_change_data);

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

  self->max = self->base+modifier+self->bonus;
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
  self->max+=roll+modifier+self->bonus;
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
  sense_t* s = GameCalloc("InitSense", 1,sizeof(sense_t));

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

  return s;
}

skill_event_t* InitSkillEvent(skill_t* s, int cr){
  skill_event_t* skev = GameCalloc("InitSkillEvent", 1,sizeof(skill_event_t));

  skill_decay_t* decay = SkillEventDecay(s->id, cr);
  *skev = (skill_event_t){
    .skill       = s->id,
      .challenge = cr,
      .decay     = decay
  };
}

skill_check_t* InitSkillCheck(skill_t* skill){
  skill_check_t* sc = GameCalloc("InitSkillCheck", 1,sizeof(skill_check_t));

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

  return sc;
}

bool SkillUseSecondary(skill_t* self, int gain, InteractResult result){
  
}
 
bool SkillUse(skill_t* self, uint64_t source, uint64_t target, int gain, InteractResult result){

  define_skill_rank_t r = SKILL_RANKS[SkillRankGet(self)];
  gain-=r.penalty;
  
//  int* cr = GameMalloc("SkillUse", sizeof(int));
  //*cr = gain;
  
  
  param_t data = ParamMake(DATA_INT, sizeof(int), &gain);
 
  interaction_t* iter = StartInteraction(source,target,EVENT_SKILL, 9, self, self->id, data, RegisterSkillEvent, UpdateSkillEvent, true);

  if (!iter || gain < 1)
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

  skill_decay_t* decay = GameCalloc("SkillEventDecay", 1,sizeof(skill_decay_t));

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

bool UnlockRequirementMet(skill_t* self, unlock_req_t req){
  switch(req.req){
    case REQ_SKILL_RANK:
      if(req.vals[self->rank])
        return true;
      break;
  }

  return false;
}

bool UnlockRequirementsMet(skill_t* s, unlock_t u){
  for (int i = 0; i < u.num_req; i++){
    if(!UnlockRequirementMet(s, u.req[i]))
      return false;
  }

  return true;
}

void SkillRankup(skill_t* self, float old, float cur){
  skill_unlocks_t sru = SKILL_UNLOCKS[self->id];

  for(int i = 0; i < sru.num_unlocks; i++){
    unlock_t unlock = sru.unlocks[i];

    if(!UnlockRequirementsMet(self, unlock))
      continue;

    Feats feats = unlock.feats;
    while(feats){
      FeatFlag feat = feats & -feats;
      feats &= feats-1;

      PropAddFeat(self->owner, feat, self->id);
    }
  }
}

skill_t* InitSkill(SkillType id, struct ent_s* owner, int min, int max){
  skill_t* s = GameCalloc("InitSkill", 1,sizeof(skill_t));

  *s = (skill_t){
    .id = id,
    .val = min,
    .min = min,
    .max = max,
    .point = 0,
    .threshold = 350,
    .owner = owner,
    .on_skill_up = SkillupRelated,
    .on_rank_up = SkillRankup,
  };

  s->checks = InitSkillCheck(s); 
  return s;

}

int SkillCheckGetVal(skill_t* s, ValueCategory val){
  if(s->ovrd)
    return s->ovrd->vals[val];

  return s->checks->vals[val];
}

InteractResult SkillCheckVal(skill_t* s, int cr){
  skill_check_t* checks = s->checks;
  if(s->ovrd)
    checks = s->ovrd;

  if(checks == NULL) 
    return IR_CRITICAL_FAIL;

  int hits[1];

  dice_roll_t* dr = checks->die;
  if(checks->vals[VAL_ADV_HIT]!=0){
    dr->cb = GREATER_THAN;
    dr->roll = RollDieAdvantage;
    dr->advantage = checks->vals[VAL_ADV_HIT];
  }
  int hit = dr->roll(dr, hits);


  s->ovrd = NULL;

  return (hit > cr)?IR_SUCCESS:IR_FAIL;

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

  WorldEvent(EVENT_SKILL_POINTS, s, s->id);
  if(s->point < s->threshold)
    return false;

  s->point = s->point - s->threshold;


  int old = s->val;
  s->val++;

  WorldEvent(EVENT_SKILL_LVL, s, s->id);

  s->threshold+= 100/s->val;
  
  if(s->on_skill_up)
    s->on_skill_up(s,old,s->val);

  define_skill_rank_t r = SKILL_RANKS[SkillRankGet(s)];

  if(r.rank != s->rank){
    s->rank=r.rank;
    if(s->on_rank_up)
      s->on_rank_up(s, old, s->rank);
    WorldEvent(EVENT_SKILL_RANK, s, s->id);
  }

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
void OnNeedStatus(EventType event, void* data, void* user){
  ent_t* e = user;
  need_t* n = data;

  if(e->state == STATE_NEED && n->status <= NEED_OK)
    SetState(e, STATE_IDLE, NULL);

}

bool NeedSetStatus(need_t* n, NeedStatus s){
  if(n->status = s)
    return false;

  n->status = s;
  WorldEvent(EVENT_NEED_STATUS, n, n->owner->gouid);
}

need_t* InitNeed(Needs id, ent_t* owner){

  uint64_t req = GetNeedReq(id, owner->props->body, owner->props->mind);
  uint64_t res = EntGetResourceByNeed(owner, id);

  need_t* n = GameCalloc("InitNeed", 1,sizeof(need_t));

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
    NeedSetStatus(n, n->status+1);
}

void NeedFulfill(need_t* n, int amount){
  if(amount == 0)
    return;

  n->val-= amount;
  n->meter = 0;
  n->activity = true;
}

void NeedIncrement(need_t *n, ent_t* owner, int amount){
  NeedSyncMeter(n, amount);
  
}
void NeedReset(need_t* n){
  n->activity = false;

  NeedStatus new = n->status;

  for(int i = NEED_CRITICAL; i > -1; i--){
    int diff = n->vals[i] - n->val;
    if (diff < 0)
      break;
    n->prio -= diff/(NEED_CRITICAL+i);
    new = i;
  }

  if(! NeedSetStatus(n, new))
    return;

  if(n->status > NEED_MET)
    return;

  n->goal = NULL;
}

void NeedStep(need_t* n){
  n->prio += imax(0, n->id + n->status - 2);

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
  initiative_t* i  = GameCalloc("InitInit", 1, sizeof(initiative_t));

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

loot_item_t* InitLoot(param_t params[LOOT_PARAM_END]){
  loot_item_t* li = GameCalloc("InitLoot", 1, sizeof(loot_item_t));

  for (int i = 0; i < LOOT_PARAM_END; i++)
    li->params[i] = params[i];

  li->ref = GenerateItem(li->params);
}

item_def_t* GenerateItemDef(LootParams params[LOOT_PARAM_END]){

}

item_gen_pool* InitItemGenPool(int cap){
  item_gen_pool *p = GameCalloc("InitItemGenPool", 1, sizeof(item_gen_pool));

  p->cap = cap;
  p->entries = GameCalloc("InitItemGenPool", cap, sizeof(item_type_d));

  HashInit(&p->map, next_pow2_int(cap*2));
  return p;
}

item_type_d* ItemPoolGetEntry(item_gen_pool *p, uint64_t uid){
  return HashGet(&p->map, uid);
}

item_type_d* ItemGenAdd(item_gen_pool* p, ItemCategory cat, void* def){
  item_type_d* idef = &p->entries[p->count++];

  switch(cat){
    case ITEM_WEAPON:
      idef->data.weap = *(weapon_def_t*) def;
      idef->gouid = GameObjectMakeUID(idef->data.weap.name, cat, WorldGetTime());
      break;
    case ITEM_ARMOR:
      idef->data.armor = *(armor_def_t*) def;
      idef->gouid = GameObjectMakeUID(idef->data.armor.name, cat, WorldGetTime());
      break;
    case ITEM_CONSUMABLE:
      idef->data.cons = *(consume_def_t*) def;
      idef->gouid = GameObjectMakeUID(idef->data.cons.name, cat, WorldGetTime());
      break;
    case ITEM_CONTAINER:
      idef->data.cont = *(container_def_t*) def;
      idef->gouid = GameObjectMakeUID(idef->data.cont.name, cat, WorldGetTime());
      break;
    case ITEM_MATERIAL:
      idef->data.mat = *(material_def_t*) def;
      idef->gouid = idef->data.mat.spec;
      break;
    case ITEM_TOOL:
      idef->data.tool = *(tool_def_t*)def;
      idef->gouid = GameObjectMakeUID(idef->data.tool.name, cat, WorldGetTime());
      break;
    default:
      return NULL;
  }

  idef->cat = cat;

  HashPut(&p->map, idef->gouid, idef);
  return idef;
}
consume_def_t* ConsumeGenerateKnowledge(int id, ConsumeType type){

  consume_def_t* def = GameCalloc("ConsumeGenerateKnowledge", 1, sizeof(consume_def_t));


  *def = CONSUME_TEMPLATES[type];

  def->chain_id = id;
  DataType d_type = -1;
  switch(type){
    case CONS_SCROLL:
    case CONS_TOME:
      strcat(def->name, GetAbilityName(id));
      break;
    case CONS_SKILLUP:
      d_type = DATA_SKILL;
      strcat(def->name, SKILL_NAMES[id]);
      break;
  }


  def->i_props |= PROP_MAT_LEATHER;

  return def;
}


consume_def_t* ConsumeGeneratePotion(StatType stat){
  consume_def_t* def = GameCalloc("ConsumeGenerateKnowledge", 1, sizeof(consume_def_t));

  
  *def = CONSUME_TEMPLATES[CONS_POT];

  switch(stat){
    case STAT_HEALTH:
    case STAT_ARMOR:
    case STAT_ENERGY:
    case STAT_STAMINA_REGEN_RATE:
    case STAT_ENERGY_REGEN_RATE:
    case STAT_HEALTH_REGEN_RATE:
    case STAT_RAGE:
    case STAT_STAMINA:
      return NULL;
      break;    
    case STAT_STAMINA_REGEN:
      def->ability = ABILITY_ITEM_RESTORE;
      def->weight = 21;
      def->chain_id = STAT_STAMINA;
      strcat(def->name, "Endurance");
      break;
    case STAT_ENERGY_REGEN:
      def->ability = ABILITY_ITEM_RESTORE;
      def->weight = 25;
      def->chain_id = STAT_ENERGY;
      strcat(def->name, "Mana");
      break;
    case STAT_HEALTH_REGEN:
      def->weight = 11;
      strcat(def->name, "Health");
      break;
  }

  return def;
}

tool_def_t* ToolGenerate(ToolType type, MaterialSpec spec){
  tool_def_t* def = GameCalloc("ToolGenerate", 1, sizeof(tool_def_t));

  tool_def_t base = TOOL_TEMPLATES[type];

  memcpy(def, &base, sizeof(tool_def_t));

  def->material = spec;
  material_spec_d *mspec = MaterialsGetEntry(LevelMaterials(), spec);
  if(mspec->id > MAT_NO_ID){
    material_data_t data = MATERIAL_DATA[mspec->id];
    sprintf(def->name, "%s %s", data.name, base.name);
    if(data.col.a == 255)
      def->col = data.col;
/*
    for(int i = 0; i < VAL_ALL; i++){
      if (data.vals[i] == 0)
        continue;

      def->vals[i] = data.vals[i];
    }
    */
  }
  else
    sprintf(def->name, "%s %s - %s", mspec->root, mspec->mat, base.name);

  return def;
}

material_def_t* InitMaterial(material_def_t base, material_spec_d* spec){
  material_def_t* mat = GameCalloc("InitMaterial", 1, sizeof(material_def_t));

  mat->type = spec->type;
  mat->i_props = base.i_props;
  mat->m_props = base.m_props;
  mat->resources = base.resources;
  mat->cost = base.cost;
  mat->score = base.score;
  mat->spec = spec->spec;
  mat->icon = base.icon;

  sprintf(mat->name,"%s %s",spec->root, spec->mat);
  memcpy(mat->vals, base.vals, sizeof(base.vals));

  if(spec->id > MAT_NO_ID){
    material_data_t data = MATERIAL_DATA[spec->id];
    memcpy(mat->vals, data.vals, sizeof(data.vals));
    if(data.icon > 0)
      mat->icon = data.icon;
    if(data.col.a > 0)
      mat->col = data.col;
    if(data.tile > 0)
      mat->tile = data.tile;
    mat->m_props |= data.props;
  }
  return mat;
}

weapon_def_t* WeaponGenerate(weapon_def_t* base, material_def_t* mat){
  weapon_def_t* def = GameCalloc("WeaponGenerate", 1, sizeof(weapon_def_t));

  memcpy(def, base, sizeof(weapon_def_t));

  def->mat = mat->spec;

  ItemMaterialGetName(mat->spec, ITEM_WEAPON, base->name, def->name);

  return def;
}

armor_def_t* ArmorGenerate(armor_def_t* base, material_def_t* mat){
  armor_def_t* def = GameCalloc("ArmorGenerate", 1, sizeof(armor_def_t));

  memcpy(def, base, sizeof(armor_def_t));

  def->mat = mat->spec;

  ItemMaterialGetName(mat->spec, ITEM_ARMOR, base->name, def->name);

  return def;

}
