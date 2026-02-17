#ifndef SCREENS_H
#define SCREENS_H
#include "game_types.h"
/*
#if defined(PLATFORM_ANDROID)
#define DESIGN_WIDTH 1080.0f
#define DESIGN_HEIGHT 1920.0f
#else
*/
#define DESIGN_WIDTH 1920.0f
#define DESIGN_HEIGHT 1080.0f
//#endif

static float SPRITE_SCALE = 1.0f;
static float UI_SCALE = 1.0f;

typedef struct{
  Camera2D        *camera;
  Cell            size,pos,target;
  Rectangle       view,bounds;
  bool            mode;
  RenderTexture2D render;
}camera_t;

void InitCamera(float zoom, float rot, Vector2 offset, Vector2 target);
void ScreenCameraSync(Cell target);
void ScreenCameraToggle(void);
bool ScreenCameraSetView(Cell v);
void ScreenCameraSetBounds(Cell b);
Rectangle ScreenGetCameraView(void);

local_ctx_t* ScreenPlayerContext(void*);
void* ScreenSelectContext(void*);

typedef struct{
  bool    is_dragging;
  Vector2 offset,pos;
  local_ctx_t   *ctx[SCREEN_CTX_ALL];
}mouse_controller_t;

typedef void (*SelectionCallback)(ent_t* e, ActionType a, local_ctx_t* selection);

typedef struct{
  bool              active;
  Cell              pos;
  map_cell_t*       selections[5];
  int               desired,selected;
  bool              occupied;
  SelectionCallback on_select, on_complete;
}key_controller_t;

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
  AREA_SCREEN,
  AREA_RENDERGL,
  AREA_ALL
}ScreenArea;

typedef float (*PlaySizeSync)(PlaySizes);
typedef struct{
  Rectangle       area[AREA_ALL];
  float           sizes[SIZE_ALL];
  PlaySizeSync    get_size;
  sprite_t*       screen_icons[ELEMENT_COUNT];
  RenderTexture2D view;
}play_area_t;

void ScreenApplyContext(local_ctx_t* ctx[SCREEN_CTX_ALL]);
float GetApproxDPIScale(void);
void InitPlayArea(void);
void ScreenCalcAreas(void);
Vector2 ScreenAreaStart(ScreenArea t);
float ScreenSized(PlaySizes s);
void ScreenActivateSelector(Cell pos, int num, bool occupied, SelectionCallback on_select);
bool ScreenSelectorInput(void);
key_controller_t* ScreenGetSelection(void);
bool ScreenMoveSelector(struct ent_s* e, ActionType a, KeyboardKey k, ActionSlot slot);
bool ScreenMakeSelection(struct ent_s* e, ActionType a, KeyboardKey k, ActionSlot slot);
void ScreenRender(void);
void InitScreenInteractive(void);
void ScreenSyncMouse(void);
void ScreenSyncKey(void);
Vector2 CaptureInput();
ent_t* ScreenEntMouseCollision(void);
ent_t* ScreenEntMouseHover(void);
static action_key_t selector_keys[ACTION_DONE] = {
  {ACTION_NONE},
/*
  {ACTION_MOVE,8,{KEY_D,KEY_A,KEY_W,KEY_S,KEY_LEFT, KEY_RIGHT,KEY_UP,KEY_DOWN},ScreenMoveSelector,SLOT_NONE},
  {ACTION_SELECT,1,{KEY_ENTER},ScreenMakeSelection},
*/
  };
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
