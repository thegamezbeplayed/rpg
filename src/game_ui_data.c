#include "game_ui.h"
#include "game_process.h"

ui_menu_d MENU_DATA[MENU_DONE] = {
  [MENU_HUD] = {MENU_HUD, "HUD_MENU_DOM", ELEMENT_NONE, false,
    {[MENU_ACTIVE] = MenuActivateChildren}
  }
};

ui_element_d ELEM_DATA[MAX_SUB_ELE] = {
  {"HUD_MENU_DOM", VECTOR2_ZERO, FIXED_MENU_FULL, UI_PANEL, ELEMENT_NONE, 
    LAYOUT_HORIZONTAL, ALIGN_CENTER,
    .spacing = {[UI_MARGIN_TOP] = 64 },
    .cb = {[ELEMENT_IDLE] = ElementActivateChildren },
    .num_children = 1, .kids= {"PLAYER_PANEL_L"}
  },
  {"PLAYER_PANEL_L", VECTOR2_ZERO, FIXED_PANEL_VER, UI_PANEL, 
    ELEMENT_NONE, LAYOUT_VERTICAL, ALIGN_LEFT, GetStatSheet, WorldPlayerContext,
    {[ELEMENT_IDLE] = ElementSetContext}
  }
};
