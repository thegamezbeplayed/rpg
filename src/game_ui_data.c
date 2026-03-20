#include "game_ui.h"
#include "game_process.h"

ui_menu_d MENU_DATA[MENU_DONE] = {
  [MENU_MAIN] = {MENU_MAIN, "TITLE_MENU_DOM", ELEMENT_NONE, false,
    {[MENU_ACTIVE] = MenuActivateChildren}
  },
  [MENU_HUD] = {MENU_HUD, "HUD_MENU_DOM", ELEMENT_NONE, false,
    {[MENU_LOAD] = MenuActivateChildren,
      [MENU_READY] = MenuProcessReady
    }
  }
};

ui_element_d ELEM_DATA[ELE_COUNT] = {
  {"TITLE_MENU_DOM", VECTOR2_ZERO, FIXED_MENU_FULL, UI_CONTAINER,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_MID | ALIGN_CENTER,
    .cb = {
      [ELEMENT_IDLE] = ElementActivateChildren,
      [ELEMENT_SHOW] = ElementShowChildren,
    },
    {
      [UI_MARGIN_TOP] = 8, [UI_MARGIN_LEFT] = 6,
      [UI_PADDING_TOP] = 4, [UI_PADDING_LEFT] = 8 
    },
    2, {
      "TITLE_PANEL",
      "PLAY_BTN"
    }

  },
  {"TITLE_PANEL", VECTOR2_ZERO, VECTOR2_ZERO, UI_BOX,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_CENTER | ALIGN_MID ,
    .cb = {
      [ELEMENT_IDLE] = ElementActivateChildren,
      [ELEMENT_SHOW] = ElementShowChildren,
    },
    {
      [UI_MARGIN_TOP] = 32, [UI_MARGIN_LEFT] = 6,
      [UI_PADDING_TOP] = 12, [UI_PADDING_LEFT] = 8
    },
    0, {"HEADER_CHAR_SPR"},
    .text = "PARAGON"
  },
  {"PLAY_BTN", VECTOR2_ZERO, FIXED_BUTTON_SIZE, UI_BUTTON,
    ELEMENT_NONE, 0, ALIGN_CENTER | ALIGN_MID,
    .cb = {
      [ELEMENT_ACTIVATE] = UITransitionScreen
    },
    .text = "PLAY"
  },
  {"HEADER_CHAR_SPR", VECTOR2_ZERO, FIXED_TITLE_CHAR, UI_CHAR_SPR,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_LEFT | ALIGN_TOP,
    GetTextSprite, ElementOwnerTextAt,
    .cb = {
      [ELEMENT_LOAD] = ElementSetContext,
      [ELEMENT_SHOW] = ElementShowIcon

    },
  },
  {"HUD_MENU_DOM", VECTOR2_ZERO, VECTOR2_ZERO, UI_CONTAINER, ELEMENT_NONE, 
    LAYOUT_FREE, ALIGN_LEFT,
    .spacing = {
      [UI_MARGIN_TOP] = 12, [UI_MARGIN_LEFT] = 6,
      [UI_PADDING_TOP] = 12, [UI_PADDING_LEFT] = 8
    },
    .cb = {
      [ELEMENT_IDLE] = ElementActivateChildren,
      [ELEMENT_SHOW] = ElementShowChildren,
    },
    .num_children = 2, .kids= {"PLAYER_PANEL_L","ACTIVITY_PANEL", "CONTEXT_PANEL_R"},
    .params = { {PARAM_NONE}, {PARAM_NONE}, {PARAM_NAME }}
  },
  {"PLAYER_PANEL_L", VECTOR2_ZERO, VECTOR2_ZERO, UI_CONTAINER,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_LEFT, NULL, WorldPlayerContext,
    {
      [ELEMENT_IDLE] = ElementActivateChildren,
      [ELEMENT_SHOW] = ElementShowChildren,
    },
    .spacing = {[UI_MARGIN_TOP] = 24, [UI_MARGIN_LEFT] = 8},
    .num_children = 2, .kids={
      "PLAYER_PANEL_HEADERS",
      "PANEL_GROUP",
      "ACTIVITY_PANEL"
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
  {"PANEL_GROUP", VECTOR2_ZERO, VECTOR2_ZERO, UI_GROUP,
    ELEMENT_NONE, LAYOUT_STACK, ALIGN_LEFT, NULL, WorldPlayerContext,
    .cb = {
      [ELEMENT_LOAD] = ElementSetContext,
      [ELEMENT_IDLE] = ElementActivateChildren,
      [ELEMENT_SHOW] = ElementShowPrimary,
    },
    .num_children = 2, .kids = {
      "CHARACTER_SHEET",
      "INVENTORY"
    },
    .spacing ={[UI_PADDING_RIGHT] = 24, [UI_PADDING_LEFT] = 8}
  },
  {"TAB_BUTTON", VECTOR2_ZERO, FIXED_BUTTON_SIZE, UI_BUTTON,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_CENTER | ALIGN_MID, GetElementName, ElementPresetContext,
    {
      [ELEMENT_LOAD] = ElementSetContext,
      //[ELEMENT_IDLE] = ElementSetContext,
      [ELEMENT_ACTIVATE] = ElementTabToggle,
    },
    .delimiter = ' '
  },
  {"INVENTORY", VECTOR2_ZERO, VECTOR2_ZERO, UI_CONTAINER,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_LEFT, NULL, ElementGetOwnerContext,
    {
      [ELEMENT_LOAD]      = ElementSetContext,
      [ELEMENT_IDLE]      = ElementActivateChildren,
      [ELEMENT_ACTIVATE]  = ElementHideSiblings,
      [ELEMENT_ACTIVATED] = ElementShow,
      [ELEMENT_SHOW]      = ElementShowChildren
    },
    .num_children = 5, .kids = {
      "ITEM_PANEL",
    },
    .params = {
      {PARAM_INV_HELD},
      {PARAM_INV_WORN},
      {PARAM_INV_BELT},
      {PARAM_INV_SLING},
      {PARAM_INV_PACK}
    },
    .spacing = {[UI_MARGIN_TOP] = 8, [UI_MARGIN_LEFT] = 4, [UI_PADDING_TOP] = 16},
    .text = "Inventory"
  },
  {"ITEM_PANEL", VECTOR2_ZERO, FIXED_GRID_PANEL, UI_PANEL, 
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_LEFT, GetContextParams,
    ElementGetOwnerContext,
    {
      [ELEMENT_LOAD] = ElementSetContextParams,
      [ELEMENT_IDLE] = ElementActivateChildren,
      [ELEMENT_SHOW] = ElementShowChildren
    },
    .num_children = 2, .kids = {
      "HEADER_BAR_VAL",
      "ITEM_GRID"
    },
    .spacing = {[UI_PADDING_BOT] = 12, [UI_PADDING_TOP] = -8}
  },
  {"ITEM_GRID", VECTOR2_ZERO, VECTOR2_ZERO, UI_CONTAINER,
    ELEMENT_NONE, LAYOUT_GRID, ALIGN_LEFT, NULL,
    ElementGetOwnerContextParams, 
    {
      [ELEMENT_LOAD] = ElementSetContext,
      [ELEMENT_IDLE] = ElementInventoryContext,
      [ELEMENT_SHOW] = ElementDynamicChildren,
      //[ELEMENT_SHOW] = ElementShowChildren
    },
    .spacing = { [UI_MARGIN_LEFT] = 2},
    .kids = {"ITEM_BOX"},
    .spacing = {[UI_PADDING_RIGHT] = 4, [UI_PADDING_BOT] = 2}
  },
  {"ITEM_BOX", VECTOR2_ZERO, FIXED_BOX_SIZE, UI_ICON,
    ELEMENT_NONE, LAYOUT_STACK, ALIGN_MID, GetContextItem, NULL,
    {
      [ELEMENT_LOAD]      = ElementSetContext,
      [ELEMENT_FOCUSED]   = ElementShowTooltip,
      [ELEMENT_TOGGLE]    = ElementToggleChildren,
      [ELEMENT_SHOW]      = ElementShowIcon,
      [ELEMENT_ACTIVATE]  = ElementItemUse,
    },
    .spacing = {[UI_MARGIN_TOP] = 0, [UI_PADDING_LEFT] = 16, [UI_PADDING_TOP] =  16},
    .texture = UI_GRID_CELL,
    .num_children = 1, .kids = {"ITEM_TOOL_TIP"},
    .params = {PARAM_ITEM}
  }, 
  {"CHARACTER_SHEET", VECTOR2_ZERO, VECTOR2_ZERO, UI_CONTAINER,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_LEFT, GetContextName, ElementGetOwnerContext,
    {
      [ELEMENT_LOAD] = ElementSetContext,
      [ELEMENT_IDLE] = ElementActivateChildren,
      [ELEMENT_SHOW] = ElementShowChildren,
      //[ELEMENT_ACTIVATE] = ElementToggle,
      [ELEMENT_ACTIVATE] = ElementHideSiblings,
    },
    .num_children = 3, .kids = {
      "HEADER_BAR_CTX",
      "CHARACTER_STATS",
      "CHARACTER_ATTR"
    },
    .params = {PARAM_NAME},
    .spacing = {[UI_MARGIN_TOP] = 0, [UI_MARGIN_LEFT]=0, [UI_PADDING_RIGHT] = 8}
  },
  {"CHARACTER_STATS", VECTOR2_ZERO, VECTOR2_ZERO, UI_PANEL,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_TOP | ALIGN_LEFT, NULL, ElementGetOwnerContext,
    {
      [ELEMENT_LOAD] = ElementSetContext,
      [ELEMENT_SHOW] = ElementShowChildren,
      [ELEMENT_IDLE] = ElementActivateChildren,
      //[ELEMENT_ACTIVATE] = ElementToggle,
    },
    .num_children = 6, .kids ={
      "HEADER_BAR",
      "CHARACTER_LINE_NAME_VAL",
    },
    .params = {
      {PARAM_NONE}, 
      {PARAM_SKILL_LVL},
      {PARAM_STAT_HEALTH},
      {PARAM_STAT_ARMOR},
      {PARAM_STAT_STAMINA},
      {PARAM_STAT_ENERGY},
    },
    .text = "Stats",
    .spacing = {[UI_MARGIN_TOP] = 4, [UI_PADDING_TOP] = 0, [UI_PADDING_BOT] = 4},
  },
  {"HEADER_BAR", VECTOR2_ZERO, FIXED_HEADER_SIZE, UI_HEADER,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_LEFT | ALIGN_TOP,
    GetOwnerText, ElementGetOwnerContext, 
    {
      [ELEMENT_IDLE] = ElementSetContext,
    },
  },
  {"VALUE_LABEL", VECTOR2_ZERO, FIXED_VAL_LABEL_SIZE, UI_LABEL,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_MID | ALIGN_RIGHT,
    GetContextVal, ElementGetOwnerContext,
    {
      [ELEMENT_IDLE] = ElementSetContext,
      [ELEMENT_FOCUSED] = ElementShowTooltip,
      [ELEMENT_TOGGLE] = ElementToggleChildren
    },
    .num_children = 1, .kids ={"STAT_TOOL_TIP"},
    .spacing = {[UI_MARGIN_LEFT] = -32, [UI_PADDING_LEFT] = -16}
  },
  {"CHARACTER_ATTR", VECTOR2_ZERO, VECTOR2_ZERO, UI_PANEL,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_LEFT | ALIGN_TOP, NULL, ElementGetOwnerContext,
    {
      [ELEMENT_LOAD] = ElementSetContext,
      [ELEMENT_IDLE] = ElementActivateChildren,
      [ELEMENT_SHOW] = ElementShowChildren,
      //[ELEMENT_ACTIVATE] = ElementToggle,
    },
    .num_children = 7, .kids ={
      "HEADER_BAR",
      "CHARACTER_LINE_NAME_VAL",
    },
    .params = {
      {PARAM_NONE},
      {PARAM_ATTR_CON},
      {PARAM_ATTR_STR},
      {PARAM_ATTR_DEX},
      {PARAM_ATTR_INT},
      {PARAM_ATTR_WIS},
      {PARAM_ATTR_CHAR},
    },
    .spacing = {[UI_PADDING_BOT] = 4},
    .text = "Attributes"
  },
  {"HEADER_BAR_VAL", VECTOR2_ZERO, FIXED_HEADER_SIZE, UI_HEADER,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_LEFT, GetOwnerValue, ElementGetOwnerContext,
    {
      [ELEMENT_IDLE] = ElementSetContext,
    },
  }, 
  {"HEADER_BAR_CTX", VECTOR2_ZERO, FIXED_HEADER_SIZE, UI_HEADER,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_LEFT, GetContextName, ElementGetOwnerContext,
    {
      [ELEMENT_IDLE] = ElementSetContext,
    },
  },
  {"VALUE_NAME_LABEL", VECTOR2_ZERO, FIXED_LABEL_SIZE, UI_LABEL,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_MID | ALIGN_LEFT, GetContextValueName, ElementGetOwnerContext,
    {
      [ELEMENT_IDLE] = ElementSetContext,
    },
  },
  {"CHARACTER_LINE_NAME_VAL", VECTOR2_ZERO, VECTOR2_ZERO, UI_CONTAINER,
    ELEMENT_NONE, LAYOUT_HORIZONTAL, ALIGN_LEFT, 
    NULL, ElementGetOwnerContext,
    {
      [ELEMENT_LOAD] = ElementSetContext,
      //[ELEMENT_IDLE] = ElementAssignValues,
      [ELEMENT_SHOW] = ElementActivateChildren,
    },
    .num_children = 2, .kids = {
      "VALUE_NAME_LABEL",
      "VALUE_LABEL",
    },
    .spacing = {[UI_MARGIN_BOT] = -20,[UI_PADDING_LEFT] = 0,[UI_PADDING_RIGHT] = -72, [UI_MARGIN_LEFT] = 8},
  },
  {"STAT_TOOL_TIP", VECTOR2_ZERO, FIXED_TOOL_TIP, UI_TOOL_TIP,
    ELEMENT_HIDDEN, LAYOUT_VERTICAL, ALIGN_CENTER | ALIGN_TOP | ALIGN_OVER,
    GetContextDetails, ElementGetOwnerContextParams,
    {
      [ELEMENT_SHOW] = ElementSetContext,
      [ELEMENT_FOCUSED] = ElementSetTooltip,
      [ELEMENT_TOGGLE] = UIHideElement
    },
    .spacing = {[UI_MARGIN_LEFT] = -16, [UI_MARGIN_TOP] = -8}
  },
  {"ITEM_TOOL_TIP", VECTOR2_ZERO, FIXED_TOOL_TIP, UI_TOOL_TIP,
    ELEMENT_HIDDEN, LAYOUT_VERTICAL, ALIGN_LEFT | ALIGN_TOP | ALIGN_OVER,
    GetContextDescription, ElementOwnerItemContext,
    {
      [ELEMENT_SHOW] = ElementSetContext,
      [ELEMENT_FOCUSED] = ElementSetTooltip,
      [ELEMENT_TOGGLE] = UIHideElement
    },
    .spacing = {[UI_MARGIN_LEFT] = 16, [UI_MARGIN_TOP] = -8, [UI_PADDING_LEFT] = 4}
  },

  {"CONTEXT_PANEL_R", UI_PANEL_RIGHT, STAT_SHEET_PANEL_VER, UI_PANEL,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_RIGHT, NULL, ScreenSelectContext,
    {
      [ELEMENT_IDLE] = ElementLoadChildren,
    },
    .num_children = 1, .kids = {"CONTEXT_SHEET"},
  },
  {"CONTEXT_SHEET", VECTOR2_ZERO, STAT_SHEET_PANEL_VER, UI_CONTAINER,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_CENTER, NULL,
    ElementGetScreenSelection, 
    {
      [ELEMENT_LOAD] = ElementScreenContext,
      [ELEMENT_IDLE] = ElementSyncContext,
      [ELEMENT_SHOW] = ElementShowChildren,
    },
    .num_children = 3, .kids = {
      "NAME_LABEL",
      "CHARACTER_LINE_NAME_VAL",
      "CHARACTER_LINE_NAME_VAL"
    },
    .params = {
      {PARAM_NAME},
      {PARAM_STAT_HEALTH},
      {PARAM_STAT_ARMOR},

    },
  },
  {"ACTIVITY_PANEL", UI_PANEL_BOT, VECTOR2_ZERO, UI_PANEL,
    ELEMENT_NONE, LAYOUT_STACK,  0,
    NULL, NULL, 
    {
      [ELEMENT_IDLE] = ElementLoadChildren,
      [ELEMENT_SHOW] = ElementShowChildren,
    },
    .num_children = 2, .kids = {
      "PLAYER_PANEL_HEADERS",
      "COMBAT_LOG",
    },
    .text = "Activity Log",
    .spacing = {[UI_PADDING_BOT] = 20, [UI_MARGIN_TOP] = 0}
  },
  {"COMBAT_LOG", VECTOR2_ZERO, UI_LOG_HOR, UI_CONTAINER, ELEMENT_NONE,
    LAYOUT_VERTICAL, ALIGN_LEFT | ALIGN_BOT, NULL, NULL,
    .cb = {
      [ELEMENT_LOAD] = ElementLoadChildren,
    },
    .num_children = MAX_SUB_ELE, .kids = {
      "COMBAT_TEXT",
    },
    .spacing = {
      [UI_MARGIN_TOP] = 20, [UI_MARGIN_LEFT] = 4
    }
  },
  {"COMBAT_TEXT", VECTOR2_ZERO, LABEL_LOG, UI_TEXT,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_TOP | ALIGN_LEFT,
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
    "{AGG} {SLAIN} {TAR}"},
  {ACT_STAT_RESTORE, NARRATE_FIRST, TENSE_PRESENT,
    "{WHO} {RESTORE} {AMNT} {STAT}"},
  {ACT_LEARN, NARRATE_FIRST, TENSE_PRESENT,
    "{WHO} {LEARN} {ABILITY} rank {QUAL}"}
};

token_lookup_t TOKEN_TABLE[TOKE_ALL] = {
  {"ID",     TOKE_ID},
  {"AGG",    TOKE_AGG},
  {"TAR",    TOKE_TAR},
  {"OWNER",  TOKE_OWNER},
  {"DMG",    TOKE_DMG},
  {"WHO",    TOKE_WHO},
  {"ENV",    TOKE_ENV},
  {"RESOURCE",  TOKE_RESOURCE},
  {"RES",    TOKE_RES_SUFF},
  {"ATK",    TOKE_ATK},
  {"ACT",    TOKE_ACT},
  {"ABILITY", TOKE_ABILITY},
  {"SCHOOL", TOKE_SCHOOL},
  {"MISS",   TOKE_MISS},
  {"LEARN",  TOKE_LEARN},
  {"RESTORE",TOKE_REST},
  {"SLAIN",  TOKE_SLAIN},
  {"AMNT",   TOKE_AMNT},
  {"STAT",   TOKE_STAT},
  {"DOES",   TOKE_DOES},
  {"USES",   TOKE_USES},
  {"QUAL",   TOKE_QUAL},
  {"DESC",   TOKE_DESC}
};
