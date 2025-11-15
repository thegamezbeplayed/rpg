#include "game_ui.h"
#include "game_utils.h"
#include "game_tools.h"
#include "game_process.h"
#include "screens.h"

// Title Screen Initialization logic
void InitEndScreen(void)
{
  MenuSetState(&ui.menus[MENU_RECAP],MENU_ACTIVE);
  UploadScore();
}

// Title Screen Update logic
void UpdateEndScreen(void)
{
    // TODO: Update TITLE screen variables here!
}

// Title Screen Draw logic
void DrawEndScreen(void)
{
  BeginDrawing();
  ClearBackground(RAYWHITE);

  UISync();
  EndDrawing();
}

// Title Screen Unload logic
void UnloadEndScreen(void)
{
    // TODO: Unload TITLE screen variables here!
  MenuSetState(&ui.menus[MENU_RECAP],MENU_INACTIVE);
}
