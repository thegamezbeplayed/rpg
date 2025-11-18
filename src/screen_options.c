#include "game_ui.h"
#include "screens.h"
#define RAYGUI_IMPLEMENTATION

// Options Screen Initialization logic
void InitOptionsScreen(void)
{
  MenuSetState(&ui.menus[MENU_OPTIONS],MENU_ACTIVE);
}

// Options Screen Update logic
void UpdateOptionsScreen(void)
{
    // TODO: Update TITLE screen variables here!
}

// Options Screen Draw logic
void DrawOptionsScreen(void)
{
  BeginDrawing();
  ClearBackground(RAYWHITE);

  UISync();
  EndDrawing();
}

// Options Screen Unload logic
void UnloadOptionsScreen(void)
{
    // TODO: Unload TITLE screen variables here!
  MenuSetState(&ui.menus[MENU_OPTIONS],MENU_INACTIVE);
}
