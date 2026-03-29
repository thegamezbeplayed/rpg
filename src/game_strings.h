#ifndef __GAME_STR__
#define __GAME_STR__

#include "game_ui.h"
#include "asset_chars.h"

typedef struct{
  int         type;
  int         type_id;
  int         num_lines;
  ParseToken  tokens[MAX_LINE_ITEMS][TOKE_ALL];
  const char* fmt[MAX_LINE_ITEMS];
}detail_format_t;

line_item_t* StringDetailsFormat(const char *fmt, int num_tokens, param_t params[num_tokens]);
void StringPrependPadding(char* s, size_t padding);
void StringAppendPadding(char* s, size_t padding);
void RepeatChar(char* out, size_t cap, char c, int times);

typedef struct{
  AbilityID     id;
  const char    strs[TENSE_ALL][MAX_NAME_LEN];
  const char    name[MAX_NAME_LEN];
}ability_strings_d;

typedef struct{
  ParseToken    token;
  const char    strs[TENSE_ALL][NARRATE_ALL][MAX_NAME_LEN];
}token_strings_d;
int ReadToken(const char* fmt, int start, char* out, int out_sz);
ParseToken TokenFromString(const char* s);

static char* INV_STRINGS[INV_DONE] = {
  "Held Items",
  "Equipped",
  "INV_BACK",
  "Utility Items",
  "Utility Items",
  "Quiver",
  "Pack",
  "Pouch"
};
static const char* STATE_STRING[STATE_END + 1]={
  "NONE",
  "SPAWN",
  "IDLE",
  "AGGRO",
  "NEED",
  "SELECT",
  "DIE",
  "DEAD",
  "STANDBY",
  "RETURN",
  "END"
};

static const char* DAMAGE_STRING[DMG_DONE]={
  "Blunt",
  "Piercing",
  "Slashing",
  "Bleed",
  "Fire",
  "Frost",
  "Lightning",
  "Thunder",
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
  {STAT_ENERGY,"Spell Energy"},
  {STAT_STAMINA, "Stamina"},
  {STAT_STAMINA_REGEN,"Stamina Regen"},
  {STAT_ENERGY_REGEN, "Spell Regen"},
  {STAT_STAMINA_REGEN_RATE,"Stamina Regen Rate"},
  {STAT_ENERGY_REGEN_RATE, "Spell Regen Rate"},
};

static const char* NEED_STRINGS[N_DONE] = {
  [N_SOC]     = "Socialize",
  [N_HUNGER]  = "Hungry",
  [N_SLEEP]   = "Tired",
  [N_THIRST]  = "Thirsty"
};

static const char* SKILL_CAT_NAMES[SKILL_CAT_DONE] = {
  [SKILL_CAT_ARMOR]      = "Armor",
  [SKILL_CAT_ADVENTURE]  = "Adventure",
  [SKILL_CAT_COMBAT]     = "Combat",
  [SKILL_CAT_CRAFT]      = "Crafting",
  [SKILL_CAT_EDU]        = "Education",
  [SKILL_CAT_GATHER]     = "Gathering",
  [SKILL_CAT_SOCIAL]     = "Social",
  [SKILL_CAT_SPELL]      = "Spells",
};

static const char* SKILL_NAMES[SKILL_DONE] = {
  [SKILL_NONE]              = "None",
  [SKILL_LVL]               = "Leveling",
  [SKILL_ACRO]              = "Acrobatics",
  [SKILL_ALCH]              = "Alchemy",
  [SKILL_ANIM]              = "Animal Handling",
  [SKILL_ARCANA]            = "Arcana",

  [SKILL_ARMOR_NATURAL]     = "Natural Armor",
  [SKILL_ARMOR_CLOTH]       = "Cloth Armor",
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
  [SKILL_DUEL]              = "Dueling",
  [SKILL_GLASS]             = "Glassworking",
  [SKILL_HIST]              = "History",
  [SKILL_HERB]              = "Herbalism",
  [SKILL_INSIGHT]           = "Insight",
  [SKILL_INTIM]             = "Intimidation",
  [SKILL_INVEST]            = "Investigation",
  [SKILL_JEWL]              = "Jeweling",
  [SKILL_LING]              = "Linguistics",
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

static ability_strings_d ABILITY_STRINGS[ABILITY_DONE] = {
  [ABILITY_RAM]           = {ABILITY_RAM, {"rammed", "rams"},"ram"},
  [ABILITY_BITE]          = {ABILITY_BITE, {"bit", "bites"}, "bite"},
  [ABILITY_CHEW]          = {ABILITY_CHEW, {"chewed on", "chews on"}, "bite"},
  [ABILITY_GNAW]          = {ABILITY_GNAW, {"gnawed at", "gnaws at"}, "bite"},
  [ABILITY_CLAW]          = {ABILITY_CLAW, {"scratched", "scratches"},"scratch"},
  [ABILITY_SWIPE]         = {ABILITY_SWIPE, {"swiped at", "swipes at"},"swipe"},
  [ABILITY_BITE_POISON]   = {ABILITY_BITE_POISON, {"bit", "bites"}, "bite"},
  [ABILITY_POISON]        = {ABILITY_POISON, {"envenommed", "envenoms"}, "poison"},
  [ABILITY_MAGIC_MISSLE]  = {ABILITY_MAGIC_MISSLE, {"shot a Magical Missile at", "shoot a Magical Missile at"},"Magic Missile"},
  [ABILITY_ELDRITCH_BLAST]  = {ABILITY_ELDRITCH_BLAST, {"blasted {TOKE_DES} energy at", "blasts a wave of {TOKE_DESC} energy at"},"Eldritch Blast"},
  [ABILITY_MAGIC_STONE]  = {ABILITY_MAGIC_STONE, {"shot a Magical Stone at", "shoots a Magical Stone at"},"Magic Stone"},
  [ABILITY_RESISTANCE]    = {ABILITY_RESISTANCE, .name = "RESISTANCE"},
  [ABILITY_GUIDING_BOLT]  = {ABILITY_GUIDING_BOLT, .name = "Guiding Bolt"},
  [ABILITY_SACRED_FLAME] = {ABILITY_SACRED_FLAME, .name = "Sacred Flame"},
  [ABILITY_STARRY_WISP] = {ABILITY_STARRY_WISP, .name = "Starry Wisp"},

  [ABILITY_WEAP_BLUDGEON] = {ABILITY_WEAP_BLUDGEON, "bludgeoned", "bludgeon", "blow"},
  [ABILITY_ITEM_HEAL] = {ABILITY_ITEM_HEAL, .name = "Healing"},
  [ABILITY_ACID_SPLASH] = {ABILITY_ACID_SPLASH, "sprayed Acidic fluid at", "sprays acidic fluid at", "Acid Splash"},
  [ABILITY_CHILL_TOUCH] = {ABILITY_CHILL_TOUCH, "grasped", "grasps", "Chilling Touch"},
  [ABILITY_POISON_SPRAY] = {ABILITY_POISON_SPRAY, "sprayed poison at", "sprays poison at", "Poison Spray"},
  [ABILITY_FIRE_BOLT]    = {ABILITY_FIRE_BOLT, "shot a fiery bolt at", "shoots a fiery bolt at", "Fire Bolt"},
  [ABILITY_FROST_BITE]    = {ABILITY_FROST_BITE, "chilled", "chills", "Frost Bite"},
  [ABILITY_DISSONANT_WHISPERS]  = {ABILITY_DISSONANT_WHISPERS, "did stuff", "does stuffs", "Dissonant Whispers"},
  [ABILITY_THUNDER_WAVE]        = {ABILITY_THUNDER_WAVE, "blasted a wave of thunderous energy at", "blasts a wave of thunderous energy at", "Thunder Wave"},
  [ABILITY_CURE_WOUNDS]         = {ABILITY_CURE_WOUNDS, "HEALED","HEALS", "Cure Wounds"},
  [ABILITY_WEAP_BLUDGEON]           = {ABILITY_WEAP_BLUDGEON, {"bludgeoned", "bludgeon"},"Bludgeon"},
  [ABILITY_WEAP_CHOP]           = {ABILITY_WEAP_CHOP, {"hacked", "hack at"},"Hack"},
  [ABILITY_WEAP_STAB]           = {ABILITY_WEAP_STAB, {"thrusted at", "thrust at"},"Stab"},
  [ABILITY_WEAP_SLASH]           = {ABILITY_WEAP_SLASH, {"cut", "cat at"},"Cut"},
  [ABILITY_WEAP_PIERCE]           = {ABILITY_WEAP_PIERCE, {"thrusted at", "thrust at"},"Pierce"},
  [ABILITY_WEAP_BLUDGEON]           = {ABILITY_WEAP_BLUDGEON, {"bludgeoned", "blugeon"},"Bludgeon"},

};

static token_strings_d TOKEN_STRINGS[TOKE_ALL] = {
  [TOKE_SLAIN] = {TOKE_SLAIN,
    {
      {"slayed", "slayed", "slayed"},
      {"have slain the", "have slain", "has slain"}
    }
  },
  [TOKE_MISS] = {TOKE_MISS,
    {
      {"missed", "missed", "missed"},
      {"misses", "miss", "misses"},
    }
  },
  [TOKE_OWNER] = {TOKE_OWNER,
    {
      {"my","your", "{TOKE_AGG}"},
      {"my","your", "{TOKE_AGG}"},
    }
  },
  [TOKE_WHO] = {TOKE_WHO,
    {
      {"you", "you", "{TOKE_AGG}"},
      {"i", "i", "{TOKE_AGG"}
    }
  },
  [TOKE_REST] = {TOKE_REST,
    {
      {"restore", "restored", "has restored"},
      {"have restored", "restored", "RESTORE"}
    }
  },
  [TOKE_LEARN] = {TOKE_LEARN,
    {
      {"learn", "learns", "has learned"},
      {"have learned", "learned", "learns"},
    }
  }
};

static const char* GetTokenString(ParseToken token, Narrator pers, NarrativeTense tense){

  return TOKEN_STRINGS[token].strs[tense][pers];

}

static const char* TargetingStringDesc(DesignationType type){
  switch(type){
    case DES_NONE:
    case DES_FACING:
      return "at nearest enemy";
      break;
    case DES_SEL_TAR:
      return "to targeted enemy";
      break;
    case DES_MULTI_TAR:
      return "to selected enemies";
      break;
    case DES_AREA:
      return "to enemies in targeted area";
      break;
    case DES_SELF:
      return "to self";
      break;
    case DES_DIR:
      return "to the enemy directly in front of self";
      break;
    case DES_ORIGIN:
      return "to enemies around self";
      break;
    case DES_IMBUE:
      return "to the victim of your next melee attack";
      break;
    default:
      return "to UNKNOWN";
      break;
  };
}

static const char* GetAbilityName(AbilityID id){
  if(ABILITY_STRINGS[id].name[0] == '\0')
    return "PLACEHOLDER_ABILITY";

  return ABILITY_STRINGS[id].name;
}


static const char* GetAbilityString(AbilityID id, NarrativeTense tense){

  if(ABILITY_STRINGS[id].strs[tense][0] == '\0')
    return "ASS TITTIES";

  return ABILITY_STRINGS[id].strs[tense];
}

static char* SkillGetTitle(skill_t* s){
  char* out = GameMalloc("SkillGetTitle", MAX_NAME_LEN * sizeof(char));

  out[0] = '\0'; // important: start as empty string

  strcat(out, SKILL_NAMES[s->id]);
  strcat(out, " - ");
  strcat(out, SKILL_RANKS[s->rank].name);
  return out;
}

static void replace_char(char* str, char find, char replace) {
  if (!str) return;
  for (int i = 0; str[i]; i++) {
    if (str[i] == find)
      str[i] = replace;
  }
}

typedef struct {
  ItemProp    qual;
  const char* name;
} quality_def_t;

typedef struct{
  ItemProp      mat;
  const char*   name;
  quality_def_t desc[QUAL_BIT_COUNT];
} material_def_t;

static const material_def_t ITEM_DEF_STRINGS[ITEM_DONE][MAT_BIT_COUNT] = {
  [ITEM_WEAPON] = {
    {PROP_MAT_BONE, },
    { PROP_MAT_WOOD, },
    {PROP_MAT_STONE, },
    {PROP_MAT_METAL, "Metal",
      {
        { PROP_QUAL_TRASH,    "Battered" },
        { PROP_QUAL_POOR,     "Discarded" },
        { PROP_QUAL_STANDARD, "" },
        { PROP_QUAL_WELL,     "Commoner's" },
        { PROP_QUAL_FINE,     "Fine" },
        { PROP_QUAL_SUPER,    "Superb" },
        { PROP_QUAL_EXPERT,   "Exceptional" },
        { PROP_QUAL_MASTER,   "Master" },
        { PROP_QUAL_ARTIFACT, "Grand" }
      }
    },
  },
  [ITEM_ARMOR] = {
    {PROP_MAT_CLOTH, "Cloth",
    },
    {PROP_MAT_LEATHER, },
    {PROP_MAT_BONE, },
    {PROP_MAT_WOOD, },
    {PROP_MAT_STONE, },
    {PROP_MAT_METAL, },
  },
};

static CharacterStrings StringGetCharEnum(const char* str){
  uint32_t hash = hash_str_32(str);
  for (int i = 0; i < CHAR_ALL; i++){
    if(CHAR_SPRITES[i].name[0] == '\0')
      continue;

    if(CHAR_SPRITES[i].hash = hash)
      return i;
  }

  return -1;
}

static char* StringSplit(char* str, char delimiter){
  char* token = strtok(str, &delimiter);

  return token;
}

static detail_format_t ITEM_FORMAT[ITEM_DONE][20] = {
  [ITEM_CONSUMABLE] = {
    [CONS_POT]      = {
      CONS_POT, CONS_POT, 
      2,
      {{TOKE_DOES, TOKE_DMG, TOKE_RESOURCE}, {TOKE_USES}}, 
      {
        "{DOES} {DMG} {RESOURCE} when consumed.",
        "{USES} uses."
      }
    },
    [CONS_FOOD]     = {},
    [CONS_DRINK]    = {},
    [CONS_SCROLL]   = {
      CONS_SCROLL, CONS_SCROLL,
      2,
      {{TOKE_ABILITY},{TOKE_USES}},
      {
        "Allows the use of {ABILITY}.",
        "{USES} charges."
      }
    },
    [CONS_TOME]     = {
      CONS_TOME, CONS_TOME,
      2,
      {{TOKE_ABILITY},{TOKE_USES}},
      {
        "Teaches the {ABILITY} ability.",
        "{USES} uses."
      }  
    },
    [CONS_SKILLUP]  = {
      CONS_SKILLUP, CONS_SKILLUP,
      2,
      {{TOKE_SKILL, TOKE_AMNT},{TOKE_USES}},
      {
        "Increases {SKILL} skill by {AMNT} points.",
        "{USES} uses."
      }
    },
  }
};

void StringBounds(Rectangle *b, char* buff);
void StringUIBounds(ui_bounds_t *b, char* buff);
#endif
