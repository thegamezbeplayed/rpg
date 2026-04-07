#include "game_ui.h"
#include "screens.h"
#include <pthread.h>
#include "game_process.h"

#define RAYGUI_IMPLEMENTATION
void* GenerateMap(void* arg){
 GameProcess pid = GP_MAP_GEN;
 if( InitMap())
   WorldEvent(EVENT_PROCESS_READY, &pid, pid); 
}

// Options Screen Initialization logic
void InitOptionsScreen(void)
{
  pthread_t t;
  //MenuSetState(&ui.menus[MENU_OPTIONS],MENU_ACTIVE);
  InitLevel();
  pthread_create(&t, NULL, GenerateMap, NULL);
  
    //InitMapGrid();
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
  ClearBackground(BROWN);

  MapGenRender();

DrawFPS(5,5);
  //UISync();
  EndDrawing();
}

// Options Screen Unload logic
void UnloadOptionsScreen(void)
{
    // TODO: Unload TITLE screen variables here!
  MenuSetState(&ui.menus[MENU_OPTIONS],MENU_INACTIVE);
}
