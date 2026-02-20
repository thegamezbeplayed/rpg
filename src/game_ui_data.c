#include "game_ui.h"
#include "game_process.h"

ui_menu_d MENU_DATA[MENU_DONE] = {
  [MENU_HUD] = {MENU_HUD, "HUD_MENU_DOM", ELEMENT_NONE, false,
    {[MENU_ACTIVE] = MenuActivateChildren}
  }
};

ui_element_d ELEM_DATA[ELE_COUNT] = {
  {"HUD_MENU_DOM", VECTOR2_ZERO, FIXED_MENU_FULL, UI_PANEL, ELEMENT_NONE, 
    LAYOUT_FREE, ALIGN_LEFT,
    .spacing = {
      [UI_MARGIN_TOP] = 32, [UI_MARGIN_LEFT] = 6,
      [UI_PADDING_TOP] = 12, [UI_PADDING_LEFT] = 8
    },
    .cb = {
      [ELEMENT_IDLE] = ElementActivateChildren,
      [ELEMENT_SHOW] = ElementShowChildren,
    },
    .num_children = 3, .kids= {"PLAYER_PANEL_L", "CONTEXT_PANEL_R", "ACTIVITY_PANEL"},
    .params = { {PARAM_NONE}, {PARAM_NONE}, {PARAM_NAME }}
  },
  {"PLAYER_PANEL_L", VECTOR2_ZERO, STAT_SHEET_PANEL_VER, UI_PANEL,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_LEFT, NULL, WorldPlayerContext,
    {
      [ELEMENT_IDLE] = ElementActivateChildren,
      [ELEMENT_SHOW] = ElementShowChildren,
    },
    .spacing = {[UI_MARGIN_TOP] = 24, [UI_MARGIN_LEFT] = 20},
    .num_children = 2, .kids={
      "PLAYER_PANEL_HEADERS",
      "PANEL_GROUP",
    }
  },
  {"PLAYER_PANEL_HEADERS", VECTOR2_ZERO, VECTOR2_ZERO, UI_TAB_PANEL,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_LEFT, NULL, ElementNiblings,
    {
      [ELEMENT_LOAD] = ElementSetContext,
      [ELEMENT_IDLE] = ElementActivateChildren
      
    },
    .num_children = 2, .kids = {
      "TAB_BUTTON"
    },
    .spacing = {[UI_MARGIN_LEFT] = 36, [UI_PADDING_RIGHT] = 4}
  },
  {"PANEL_GROUP", VECTOR2_ZERO, STAT_SHEET_PANEL_VER, UI_PANEL,
    ELEMENT_NONE, LAYOUT_STACK, ALIGN_LEFT, NULL, WorldPlayerContext,
    .cb = {
      [ELEMENT_LOAD] = ElementSetContext,
      [ELEMENT_IDLE] = ElementActivateChildren,
      [ELEMENT_SHOW] = ElementShowPrimary,
    },
    .num_children = 2, .kids = {
      "PLAYER_STATS",
      "INVENTORY"
    }
  },
  {"TAB_BUTTON", VECTOR2_ZERO, FIXED_BUTTON_SIZE, UI_BUTTON,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_CENTER | ALIGN_MID, GetElementName, ElementPresetContext,
    {
      [ELEMENT_LOAD] = ElementSetContext,
      //[ELEMENT_IDLE] = ElementSetContext,
      [ELEMENT_ACTIVATE] = ElementTabToggle,
    }
  },
  {"INVENTORY", VECTOR2_ZERO, STAT_SHEET_PANEL_VER, UI_PANEL,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_LEFT, NULL, ElementGetOwnerContext,
    {
      [ELEMENT_LOAD]      = ElementSetContext,
      [ELEMENT_IDLE]      = ElementActivateChildren,
      [ELEMENT_ACTIVATE]  = ElementHideSiblings,
      [ELEMENT_ACTIVATED] = ElementShow,
      [ELEMENT_SHOW]      = ElementShowChildren
    },
    .num_children = 4, .kids = {
      "ITEM_GRID",
    },
    .params = {
      {PARAM_INV_HELD},
      {PARAM_INV_WORN},
      {PARAM_INV_BELT},
      { PARAM_INV_SLING},
    },
    .spacing = {[UI_MARGIN_TOP] = 20, [UI_MARGIN_LEFT] = 4},
  },
  {"ITEM_GRID", VECTOR2_ZERO, FIXED_BOX_SIZE, UI_PANEL,
    ELEMENT_NONE, LAYOUT_GRID, ALIGN_LEFT, GetContextParams,
    ElementGetOwnerContext, 
    {
      [ELEMENT_LOAD] = ElementSetContext,
      [ELEMENT_IDLE] = ElementDynamicChildren,
      [ELEMENT_SHOW] = ElementShowChildren
    },
    .spacing = { [UI_MARGIN_LEFT] = 8, [UI_PADDING] = 4},
    .kids = {"ITEM_BOX"}
  },
  {"ITEM_BOX", VECTOR2_ZERO, FIXED_BOX_SIZE, UI_ICON,
   ELEMENT_NONE, LAYOUT_STACK, ALIGN_MID | ALIGN_CENTER, GetContextItem, ElementPresetContext,
   {
      [ELEMENT_LOAD]      = ElementSetContext,
      [ELEMENT_SHOW]      = ElementShowIcon
   },
   .spacing = {[UI_PADDING_TOP] = 18, [UI_PADDING_LEFT] = 18},
   .texture = UI_GRID_CELL,
   .num_children = 1, .kids = {"ITEM_TOOL_TIP"}
  }, 
  {"PLAYER_STATS", VECTOR2_ZERO, STAT_SHEET_PANEL_VER, UI_PANEL,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_LEFT, NULL, ElementGetOwnerContext,
    {
      [ELEMENT_LOAD] = ElementSetContext,
      [ELEMENT_IDLE] = ElementActivateChildren,
      [ELEMENT_SHOW] = ElementShowChildren,
      //[ELEMENT_ACTIVATE] = ElementToggle,
      [ELEMENT_ACTIVATE] = ElementHideSiblings,

    },
    .num_children = 6, .kids ={
      "NAME_LABEL",
      "SKILL_LABEL_DETAILED",
      "STAT_LABEL_DETAILED",
      "STAT_LABEL_DETAILED",
      "STAT_LABEL_DETAILED",
      "STAT_LABEL_DETAILED",
    },
   .params = {
      {PARAM_NAME},
      {PARAM_SKILL_LVL},
      {PARAM_STAT_HEALTH},
      {PARAM_STAT_ARMOR},
      {PARAM_STAT_STAMINA},
      {PARAM_STAT_ENERGY},
    }
  },
  {"NAME_LABEL", VECTOR2_ZERO, FIXED_LABEL_SIZE, UI_LABEL,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_LEFT, GetContextName, ElementGetOwnerContext,
    {
      [ELEMENT_IDLE] = ElementSetContext,
      [ELEMENT_SHOW] = ElementSetContext,
    },
  },
  {"STAT_LABEL", VECTOR2_ZERO, FIXED_LABEL_SIZE, UI_LABEL,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_LEFT, 
    GetContextStat, ElementGetOwnerContext,
    {
      [ELEMENT_IDLE] = ElementSetContext,
      [ELEMENT_SHOW] = ElementSetContext,
    },
  },
  {"STAT_LABEL_DETAILED", VECTOR2_ZERO, FIXED_LABEL_SIZE, UI_LABEL,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_LEFT, 
    GetContextStat, ElementGetOwnerContext,
    {
      [ELEMENT_SHOW] = ElementSetContext,
      [ELEMENT_FOCUSED] = ElementShowTooltip,
      [ELEMENT_TOGGLE] = ElementToggleChildren
    },
    .num_children = 0, .kids ={"STAT_TOOL_TIP"},
  },
  {"STAT_TOOL_TIP", VECTOR2_ZERO, FIXED_TOOL_TIP, UI_TOOL_TIP,
    ELEMENT_HIDDEN, LAYOUT_VERTICAL, ALIGN_CENTER | ALIGN_TOP | ALIGN_OVER,
    GetContextDetails, ElementGetOwnerContext,
    {
      [ELEMENT_SHOW] = ElementSetContext,
      [ELEMENT_FOCUSED] = ElementSetTooltip,
      [ELEMENT_TOGGLE] = UIHideElement
    },
    .spacing = {[UI_MARGIN_LEFT] = 54, [UI_MARGIN_TOP] = -28}
  },
  {"SKILL_LABEL", VECTOR2_ZERO, FIXED_LABEL_SIZE, UI_LABEL,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_LEFT, 
    GetContextStat, ElementGetOwnerContext,
    {
      [ELEMENT_IDLE] = ElementSetContext,
    }
  },
  {"SKILL_LABEL_DETAILED", VECTOR2_ZERO, FIXED_LABEL_SIZE, UI_LABEL,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_LEFT, 
    GetContextStat, ElementGetOwnerContext,
    {
      [ELEMENT_SHOW] = ElementSetContext,
      [ELEMENT_FOCUSED] = ElementShowTooltip,
      [ELEMENT_TOGGLE] = ElementToggleChildren

    },
    .num_children = 1, .kids ={"STAT_TOOL_TIP"},
    .spacing = {[UI_MARGIN_RIGHT] = 8}
  },
  {"ITEM_TOOL_TIP", VECTOR2_ZERO, FIXED_TOOL_TIP, UI_TOOL_TIP,
    ELEMENT_HIDDEN, LAYOUT_VERTICAL, ALIGN_CENTER | ALIGN_TOP | ALIGN_OVER,
    GetContextDetails, ElementGetOwnerContext,
    {
      [ELEMENT_SHOW] = ElementSetContext,
      [ELEMENT_FOCUSED] = ElementSetTooltip,
      [ELEMENT_TOGGLE] = UIHideElement
    },
    .spacing = {[UI_MARGIN_LEFT] = 54, [UI_MARGIN_TOP] = -28}
  },

  
  {"CONTEXT_PANEL_R", UI_PANEL_RIGHT, STAT_SHEET_PANEL_VER, UI_PANEL,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_RIGHT, NULL, ScreenSelectContext,
    {
      [ELEMENT_IDLE] = ElementLoadChildren,
    },
    .num_children = 1, .kids = {"CONTEXT_SHEET"},
  },
  {"CONTEXT_SHEET", VECTOR2_ZERO, STAT_SHEET_PANEL_VER, UI_PANEL,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_CENTER, NULL,
    ElementGetScreenSelection, 
    {
      [ELEMENT_LOAD] = ElementScreenContext,
      [ELEMENT_IDLE] = ElementSyncContext,
      [ELEMENT_SHOW] = ElementShowChildren,
    },
    .num_children = 3, .kids = {
      "NAME_LABEL",
      "STAT_LABEL",
      "STAT_LABEL"
    },
    .params = {
      {PARAM_NAME},
      {PARAM_STAT_HEALTH},
      {PARAM_STAT_ARMOR},

    },
  },
  {"ACTIVITY_PANEL", UI_PANEL_BOT, FIXED_PANEL_HOR, UI_PANEL,
    ELEMENT_NONE, LAYOUT_HORIZONTAL,  ALIGN_LEFT | ALIGN_BOT,
    NULL, NULL, 
    {
      [ELEMENT_IDLE] = ElementLoadChildren,
      [ELEMENT_SHOW] = ElementShowChildren,
    },
    .num_children = 1, .kids = {
      "COMBAT_LOG",
    },
  },
  {"COMBAT_LOG", VECTOR2_ZERO, UI_LOG_HOR, UI_PANEL, ELEMENT_NONE,
    LAYOUT_VERTICAL, ALIGN_LEFT | ALIGN_BOT, NULL, NULL,
    .cb = {
      [ELEMENT_LOAD] = ElementLoadChildren,
    },
    .num_children = MAX_SUB_ELE, .kids = {
        "COMBAT_TEXT",
    },
    .spacing = {
      [UI_PADDING_BOT] = 8
    }
  },
  {"COMBAT_TEXT", VECTOR2_ZERO, LABEL_LOG, UI_TEXT,
   ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_LEFT,
   GetActivityEntry, ElementIndexContext,
    .cb = {
      [ELEMENT_LOAD] = ElementActivityContext,
      [ELEMENT_IDLE] = ElementShowContext,
    }
  }
};

activity_format_t ACT_LOG_FMT[ACT_ALL] = {
  {ACT_NONE},
  {ACT_ATTACK, NARRATE_FIRST, TENSE_PRESENT,
    "{AGG} {ACT} {TAR} for {DMG} {SCHOOL} damage"},
  {ACT_MISS, NARRATE_FIRST, TENSE_PRESENT,
    "{OWNER} {ATK} {MISS} {TAR}"},
  {ACT_KILL, NARRATE_FIRST, TENSE_PRESENT,
    "{AGG} {SLAIN} {TAR}"}
};

token_lookup_t TOKEN_TABLE[TOKE_ALL] = {
  {"ID",     TOKE_ID},
  {"AGG",    TOKE_AGG},
  {"TAR",    TOKE_TAR},
  {"OWNER",  TOKE_OWNER},
  {"DMG",    TOKE_DMG},
  {"WHO",    TOKE_WHO},
  {"ENV",    TOKE_ENV},
  {"RES",    TOKE_RES_SUFF},
  {"ATK",    TOKE_ATK},
  {"ACT",    TOKE_ACT},
  {"SCHOOL", TOKE_SCHOOL},
  {"MISS",   TOKE_MISS},
  {"SLAIN",  TOKE_SLAIN},
};
