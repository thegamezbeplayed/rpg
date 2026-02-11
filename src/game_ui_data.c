#include "game_ui.h"
#include "game_process.h"

ui_menu_d MENU_DATA[MENU_DONE] = {
  [MENU_HUD] = {MENU_HUD, "HUD_MENU_DOM", ELEMENT_NONE, false,
    {[MENU_ACTIVE] = MenuActivateChildren}
  }
};

ui_element_d ELEM_DATA[MAX_SUB_ELE] = {
  {"HUD_MENU_DOM", VECTOR2_ZERO, FIXED_MENU_FULL, UI_PANEL, ELEMENT_NONE, 
    LAYOUT_FREE, ALIGN_LEFT,
    .spacing = {[UI_MARGIN_TOP] = 48, [UI_MARGIN_LEFT] = 16 },
    .cb = {
      [ELEMENT_IDLE] = ElementActivateChildren,
      [ELEMENT_SHOW] = ElementShowChildren,
    },
    .num_children = 2, .kids= {"PLAYER_PANEL_L", "CONTEXT_PANEL_R"}
  },
  {"PLAYER_PANEL_L", VECTOR2_ZERO, STAT_SHEET_PANEL_VER, UI_PANEL,
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_LEFT, NULL, WorldPlayerContext,
    {
      [ELEMENT_IDLE] = ElementSetContext,
      [ELEMENT_SHOW] = ElementActivateChildren,
    },
   .spacing = {[UI_PADDING] = 1},
   .num_children = 6, .kids={
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
      [ELEMENT_IDLE] = ElementSetContext,
      [ELEMENT_FOCUSED] = ElementShowTooltip,
      [ELEMENT_TOGGLE] = ElementToggle
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
      [ELEMENT_IDLE] = ElementSetContext,
      [ELEMENT_FOCUSED] = ElementShowTooltip,
      [ELEMENT_TOGGLE] = ElementToggle

    },
    .num_children = 1, .kids ={"STAT_TOOL_TIP"},
    .spacing = {[UI_MARGIN_RIGHT] = 8}
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
    ELEMENT_NONE, LAYOUT_HORIZONTAL,  ALIGN_LEFT | ALIGN_TOP, NULL,
    NULL, 
    {

    },
    .num_children = 1, .kids = {
      "COMBAT_LOG",
    },
  },
  {"COMBAT_LOG", VECTOR2_ZERO, UI_LOG_HOR, UI_LABEL, ELEMENT_NONE,
    LAYOUT_VERTICAL, ALIGN_LEFT | ALIGN_BOT, GetActivityEntry, NULL,
    .cb = {

    }
  }  
};
