#include "raylib.h"
#include "game_process.h"
#include "game_ui.h"

Camera2D camera = { 0 };
//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Gameplay Screen Initialization logic
void InitGameplayScreen(void){
  MenuSetState(&ui.menus[MENU_HUD],MENU_ACTIVE);
  //camera.target = player.position;
  camera.offset = VECTOR2_CENTER_SCREEN;
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  camera.target = VECTOR2_CENTER_SCREEN;
  InitGameEvents();
  InitScreenInteractive();
}

void PreUpdate(void){
  GameProcessStep();
  WorldPreUpdate();
}

void FixedUpdate(void){
  AudioStep();
  WorldFixedUpdate();
}

void PostUpdate(void){
  WorldPostUpdate();
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{
  ScreenSyncMouse();
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
  //  if(game_process.state == GAME_LOADING)
  //  return;
  BeginDrawing();
  ClearBackground(GRAY);
  BeginMode2D(camera);

  WorldRender();

  EndMode2D();
  UISync();
  EndDrawing();
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
  MenuSetState(&ui.menus[MENU_HUD],MENU_CLOSED);
  MenuSetState(&ui.menus[MENU_PLAY_AREA],MENU_INACTIVE);

  GameProcessEnd();
  // TODO: Unload GAMEPLAY screen variables here!
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
  return 0;
//  return finishScreen;
}
