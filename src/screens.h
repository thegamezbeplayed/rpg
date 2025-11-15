#ifndef SCREENS_H
#define SCREENS_H
#include "game_types.h"
/*
#if defined(PLATFORM_ANDROID)
#define DESIGN_WIDTH 1080.0f
#define DESIGN_HEIGHT 1920.0f
#else
*/
#define DESIGN_WIDTH 1280.0f
#define DESIGN_HEIGHT 960.0f
//#endif

static float SPRITE_SCALE = 1.0f;
static float UI_SCALE = 1.0f;

typedef struct{
  bool    is_dragging;
  Vector2 offset,pos;
  ent_t   *target;
  ent_t   *hover;
}mouse_controller_t;

typedef enum{
  SIZE_GRID,
  SIZE_CELL,
  SIZE_SCALE,
  SIZE_UI,
  SIZE_FONT,
  SIZE_ALL
}PlaySizes;

typedef enum{
  AREA_PLAY,
  AREA_UI,
  AREA_ALL
}ScreenArea;

typedef float (*PlaySizeSync)(PlaySizes);
typedef struct{
  Rectangle      area[AREA_ALL];
  float          sizes[SIZE_ALL];
  PlaySizeSync   get_size;
}play_area_t;
float GetApproxDPIScale(void);
void InitPlayArea(void);
void ScreenCalcAreas(void);
Vector2 ScreenAreaStart(ScreenArea t);
float ScreenSized(PlaySizes s);

void InitScreenInteractive(void);
void ScreenSyncMouse(void);
ent_t* ScreenEntMouseCollision(void);
ent_t* ScreenEntMouseHover(void);
//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum {
  SCREEN_LOGO,
  SCREEN_TITLE,
  SCREEN_OPTIONS,
  SCREEN_GAMEPLAY,
  SCREEN_ENDING,
  SCREEN_DONE
} GameScreen;

//----------------------------------------------------------------------------------
// Global Variables Declaration (shared by several modules)
//----------------------------------------------------------------------------------
extern GameScreen currentScreen;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Logo Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitLogoScreen(void);
void UpdateLogoScreen(void);
void DrawLogoScreen(void);
void UnloadLogoScreen(void);
int FinishLogoScreen(void);

//----------------------------------------------------------------------------------
// Title Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitTitleScreen(void);
void UpdateTitleScreen(void);
void DrawTitleScreen(void);
void UnloadTitleScreen(void);
int FinishTitleScreen(void);

//----------------------------------------------------------------------------------
// Options Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitOptionsScreen(void);
void UpdateOptionsScreen(void);
void DrawOptionsScreen(void);
void UnloadOptionsScreen(void);
int FinishOptionsScreen(void);

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitGameplayScreen(void);
void UpdateGameplayScreen(void);
void DrawGameplayScreen(void);
void UnloadGameplayScreen(void);
int FinishGameplayScreen(void);
void PreUpdate(void);
void FixedUpdate(void);
void PostUpdate(void);
//----------------------------------------------------------------------------------
// Ending Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitEndScreen(void);
void UpdateEndScreen(void);
void DrawEndScreen(void);
void UnloadEndScreen(void);
int FinishEndScreen(void);

#ifdef __cplusplus
}
#endif

#endif // SCREENS_H
