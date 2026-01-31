#include "raylib.h"
#include "game_process.h"
#include "game_ui.h"

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Gameplay Screen Initialization logic
void InitGameplayScreen(void){
  MenuSetState(&ui.menus[MENU_HUD],MENU_ACTIVE);
  //camera.target = player.position;
  InitCamera(2.0f,0.0f,VECTOR2_CENTER_SCREEN,VECTOR2_CENTER_SCREEN);
  
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
  WORLD_TICK++;
  if(player)
    ScreenCameraSync( WorldPlayer()->pos );
  
  ScreenSyncMouse();
  ScreenSyncKey();
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
  //  if(game_process.state == GAME_LOADING)
  //  return;
  BeginDrawing();

  ScreenCameraToggle();

  WorldRender();

  ScreenCameraToggle();
  DrawTextEx(ui.font,GetWorldTime(),Vector2XY(VECTOR2_CENTER_SCREEN.x, 32) ,28,1,BLUE);

  MapGenRender();
  UISync();
  DrawFPS(5,5);

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
