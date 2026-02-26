#ifndef __GAME_UI__
#define __GAME_UI__
#include <stdint.h>
#include <stdbool.h>
#include "raylib.h"
#include "game_assets.h"
#include "game_utils.h"

#define MAX_LINE_ITEMS 12
#define MAX_LINE_VAL 4
#define MAX_LINE_LEN 256
#define MAX_SUB_ELE 20
#define MAX_ELEMENTS 64

#define ELE_COUNT 21
#if defined(PLATFORM_ANDROID)
#define DEFAULT_MENU_SIZE (Vector2){GetScreenWidth()/2, GetScreenHeight()/2}
#define DEFAULT_MENU_THIN_SIZE (Vector2){GetScreenWidth(), 64*UI_SCALE}
#define DEFAULT_BUTTON_SIZE (Vector2){160*UI_SCALE, 96*UI_SCALE}
#define XS_PANEL_SIZE (Vector2){264*UI_SCALE, 112*UI_SCALE}
#define XS_PANEL_THIN_SIZE (Vector2){264*UI_SCALE, 80*UI_SCALE}
#else
#define DEFAULT_MENU_SIZE (Vector2){GetScreenWidth()/2, GetScreenHeight()*.75f}
#define FIXED_MENU_FULL (Vector2){1600, 1080}
#define DEFAULT_MENU_SIZE (Vector2){GetScreenWidth()/2, GetScreenHeight()*.75f}
#define DEFAULT_MENU_THIN_SIZE (Vector2){GetScreenWidth()/2, 64*UI_SCALE}
#define DEFAULT_BUTTON_SIZE (Vector2){120*UI_SCALE, 48*UI_SCALE}
#define XS_PANEL_SIZE (Vector2){108*UI_SCALE, 64*UI_SCALE}
#define XS_PANEL_THIN_SIZE (Vector2){108*UI_SCALE, 32*UI_SCALE}
#endif
#define LARGE_BUTTON_SIZE     (Vector2){164*UI_SCALE, 32*UI_SCALE}
#define SQUARE_PANEL          (Vector2){96,96}
#define DEFAULT_BUTTON_WIDE   (Vector2){172*UI_SCALE, 48*UI_SCALE}
#define DEFAULT_PANEL_SIZE    (Vector2){GetScreenWidth()*UI_SCALE, 64*UI_SCALE}
#define FIXED_PANEL_HOR       (Vector2){910, 64}
#define FIXED_PANEL_VER       (Vector2){96, 540}
#define STAT_SHEET_PANEL_VER  (Vector2){128, 540}
#define DEFAULT_PANEL_THIN_SIZE (Vector2){224*UI_SCALE, 32*UI_SCALE}
#define LARGE_PANEL_THIN_SIZE (Vector2){GetScreenWidth()*UI_SCALE, 32*UI_SCALE}
#define SMALL_PANEL_SIZE (Vector2){192*UI_SCALE, 64*UI_SCALE}
#define SMALL_PANEL_THIN_SIZE (Vector2){184*UI_SCALE, 32*UI_SCALE}
#define DEFAULT_LINE_SIZE (Vector2){2 *UI_SCALE, 64*UI_SCALE}

#define FIXED_BUTTON_SIZE     (Vector2){128, 24}
#define FIXED_LABEL_SIZE      (Vector2){96, 24}
#define FIXED_BOX_SIZE        (Vector2){36, 36}
#define FIXED_TOOL_TIP        (Vector2){96, 24}

#define UI_PANEL_RIGHT (Vector2){1472, 0}
#define UI_PANEL_BOT (Vector2){48, 640}

#define UI_LOG_HOR (Vector2){480, 128}
#define LABEL_LOG (Vector2){464, 12}

#define LIST_LEFT_HAND_PAD 20
#define LIST_RIGHT_HAND_PAD 8

typedef struct element_value_s element_value_t;
typedef struct local_context_s local_context_t;
typedef struct interaction_s interaction_t;

typedef enum{
  MENU_INACTIVE,
  MENU_READY,
  MENU_CLOSED,
  MENU_ACTIVE,
  MENU_OPENED,
  MENU_FOCUSED,
  MENU_CLOSE,
  MENU_END
}MenuState;

typedef enum{
  ELEMENT_NONE,
  ELEMENT_LOAD,
  ELEMENT_HIDDEN,
  ELEMENT_IDLE,
  ELEMENT_SHOW,
  ELEMENT_FOCUSED,
  ELEMENT_ACTIVATE,
  ELEMENT_TOGGLE,
  ELEMENT_ACTIVATED,
  ELEMENT_DONE
}ElementState;

typedef enum{
  UI_MASK,
  UI_BUTTON,
  UI_LABEL,
  UI_STATUSBAR,
  UI_PROGRESSBAR,
  UI_PANEL,
  UI_TAB_PANEL,
  UI_BOX,
  UI_ICON,
  UI_LINE,
  UI_TEXT,
  UI_GAME,
  UI_TOOL_TIP,
  UI_BLANK
}ElementType;

typedef enum{
  LAYOUT_FREE,
  LAYOUT_VERTICAL,
  LAYOUT_HORIZONTAL,
  LAYOUT_STACK,
  LAYOUT_GRID,
}UILayout;

typedef enum{
  ALIGN_NONE = 0,
  ALIGN_CENTER  = 0x01,
  ALIGN_LEFT    = 0x02,
  ALIGN_RIGHT   = 0x04,
  ALIGN_TOP     = 0x10,
  ALIGN_MID     = 0x20,
  ALIGN_BOT     = 0x40,
  ALIGN_OVER    = 0x100,
}UIAlignment;

typedef enum{
  UI_PADDING,
  UI_PADDING_TOP,
  UI_PADDING_BOT,
  UI_PADDING_LEFT,
  UI_PADDING_RIGHT,
  UI_MARGIN,
  UI_MARGIN_TOP,
  UI_MARGIN_LEFT,
  UI_MARGIN_RIGHT,
  UI_POSITIONING
}UIPosition;

typedef enum{
  MENU_NONE,
  MENU_MAIN,
  MENU_OPTIONS,
  MENU_PAUSE,
  MENU_RECAP,
  MENU_HUD,
  MENU_PLAY_AREA,
  MENU_EXIT,
  MENU_DEBUG,
  MENU_DONE
}MenuId;

typedef enum{
  FETCH_NONE,
  FETCH_UPDATE,
  FETCH_EVENT,
  FETCH_TURN,
  FETCH_ONCE,
  FETCH_ACTIVE,
  FETCH_DONE,
}FetchRate;


typedef struct{
  int               num_val, r_len, r_wid, r_hei, padd_r, padd_l, des_len;
  element_value_t   *values[4];
  const char*       text_format;
}line_item_t;

typedef struct{
  int           lines;
  line_item_t*  ln[MAX_LINE_ITEMS];
}stat_sheet_t;

line_item_t* InitLineItem(element_value_t **val, int num_val, const char* format);
const char* PrintLine(line_item_t* ln);
char *TextFormatLineItem(line_item_t *item);
void PrintMobDetail(ent_t* e);
int EntGetStatPretty(element_value_t **fill, stat_t* stat);
int EntGetNamePretty(element_value_t **fill, ent_t* e );
int CtxGetString(element_value_t **fill, local_ctx_t*, GameObjectParam);
element_value_t* InventoryGetItem(element_value_t* self, void* context);
element_value_t* StatGetPretty(element_value_t* self, void* context);
element_value_t* SkillGetPretty(element_value_t* self, void* context);
void PrintSyncLine(line_item_t* ln, FetchRate poll);
int SetCtxParams(local_ctx_t* , line_item_t**, const char f[PARAM_ALL][MAX_NAME_LEN], int pad[UI_POSITIONING], bool);
element_value_t* SetCtxItems(void*, GameObjectParam params[4], int);
int SetActivityLines(element_value_t*, int pad[UI_POSITIONING]);
int SetCtxDetails(local_ctx_t* , line_item_t**, const char f[PARAM_ALL][MAX_NAME_LEN], int pad[UI_POSITIONING], bool);
int SetCtxDescription(void* , line_item_t**, GameObjectParam, int pad[UI_POSITIONING]);
char* PrintElementValue(element_value_t* ev, int spacing[UI_POSITIONING], char* out);
typedef struct ui_element_s ui_element_t;
typedef bool (*ElementCallback)( ui_element_t* self);
typedef enum {
  VAL_ICO,
  VAL_INT,
  VAL_FLOAT,
  VAL_CHAR,
  VAL_LN,
} ValueType;

typedef element_value_t* (*ElementFetchValue)(element_value_t* e, void* context);
typedef element_value_t* (*ElementSetValue)(ui_element_t* e, void* context);

struct element_value_s{
  FetchRate   rate;
  ValueType type;
  union {
    int           *i;
    float         *f;
    char          *c;
    line_item_t   *l[MAX_LINE_ITEMS];
    sprite_t      *s;
  };
  size_t            char_len, num_ln, text_len, text_hei;
  void*             context;
  int               index;
  bool              reverse;
  ElementFetchValue get_val;
};

typedef void (*ElementValueSync)(ui_element_t* e, FetchRate poll);

typedef void* (*ElementDataContext)(void*);
void* ElementGetOwnerContext(void*);
void* ElementMatchTab(void*);
void* ElementOwnerItemContext(void*);
void* ElementOwnerChildren(void*);
void* ElementNiblings(void *);
void* ElementPresetContext(void*);
void* ElementGetScreenSelection(void* p);
void* ElementIndexContext(void* p);
bool ElementScreenContext(ui_element_t* e);
bool ElementActivityContext(ui_element_t* e);

struct ui_element_s{
  uint32_t            hash;
  char                name[MAX_NAME_LEN];
  int                 index;
  struct ui_menu_s    *menu;
  struct ui_element_s *owner;
  ElementType         type;
  ElementState        state, prior;
  ElementCallback     cb[ELEMENT_DONE];
  Rectangle           bounds;
  scaling_slice_t     *texture;
  float               width,height;
  UILayout            layout;
  UIAlignment         align;
  int                 spacing[UI_POSITIONING];
  char*               text;
  ElementSetValue     set_val;
  ElementValueSync    sync_val;
  element_value_t        *value;
  int                 num_children, num_params;
  ui_element_t*       children[MAX_SUB_ELE];
  ElementDataContext  get_ctx;
  GameObjectParam     params[4];
  void*               ctx;
};

typedef struct{
  const char          identifier[MAX_NAME_LEN];
  Vector2             pos, size;
  ElementType         type;
  ElementState        state;
  UILayout            layout;
  UIAlignment         align;
  ElementSetValue     set;
  ElementDataContext  context;
  ElementCallback     cb[ELEMENT_DONE];
  int                 spacing[UI_POSITIONING];
  //ElementValueSync  sync;
  int                 num_children;
  const char          kids[MAX_SUB_ELE][MAX_NAME_LEN];
  GameObjectParam     params[MAX_SUB_ELE][PARAM_ALL];
  UiType              texture;
}ui_element_d;
extern ui_element_d ELEM_DATA[ELE_COUNT];

ui_element_t* InitElement(const char* name, ElementType type, Vector2 pos, Vector2 size, UIAlignment align,UILayout layout);
ui_element_t* GetElement(const char* name);
void ElementStepState(ui_element_t* e, ElementState s);
bool ElementSetState(ui_element_t* e, ElementState s);
void ElementAddChild(ui_element_t *o, ui_element_t* c);
void ElementAddGameElement( ent_t* e);
void UISyncElement(ui_element_t* e, FetchRate poll);
bool UICloseOwner(ui_element_t* e);        
bool UIFreeElement(ui_element_t* e);
bool UIHideElement(ui_element_t* e);
bool ElementActivateChildren(ui_element_t*);
bool ElementLoadChildren(ui_element_t*);
bool ElementShow(ui_element_t* e);
bool ElementShowIcon(ui_element_t* e);
bool ElementShowChildren(ui_element_t*);
bool ElementShowPrimary(ui_element_t*);
bool ElementTabToggle(ui_element_t* e);
bool ElementSetContext(ui_element_t* e);
bool ElementHideSiblings(ui_element_t* e);
bool ElementSyncContext(ui_element_t* e);
bool ElementShowContext(ui_element_t* e);
bool ElementSyncOwnerContext(ui_element_t* e);
bool ElementToggleTooltip(ui_element_t* e);
bool ElementToggle(ui_element_t* e);
bool ElementToggleChildren(ui_element_t* e);
bool ElementDynamicChildren(ui_element_t* e);
bool ElementShowTooltip(ui_element_t* e);
bool ElementSetTooltip(ui_element_t* e);
bool ElementSetActiveTab(ui_element_t* e);
struct ui_menu_s;
typedef bool (*MenuCallback)(struct ui_menu_s* self);

element_value_t* GetContextParams(ui_element_t* e, void* context);
element_value_t* GetContextName(ui_element_t* e, void* context);
element_value_t* GetContextItem(ui_element_t* e, void* context);
element_value_t* GetElementName(ui_element_t* e, void* context);
element_value_t* GetActivityEntry(ui_element_t* e, void* context);
element_value_t* GetContextStat(ui_element_t* e, void* context);
element_value_t* GetContextDetails(ui_element_t* e, void* context);
element_value_t* GetContextDescription(ui_element_t* e, void* context);

void UIEventLogEntry(EventType event, void* data, void* user);
static void UIEventActivate(EventType event, void* data, void* user){
  local_ctx_t* ctx = data;
  ui_element_t* e = user;
  e->ctx = ctx;
  ElementSetState(e, ELEMENT_IDLE);
}
void UILogEvent(EventType event, void* data, void* user);

typedef struct ui_menu_s{
  ui_element_t  *element;
  MenuCallback  cb[MENU_END];
  MenuState     state;
  bool          is_modal;
}ui_menu_t;

typedef struct{
  MenuId        id;
  char          element[MAX_NAME_LEN];
  MenuState     state;
  bool          is_modal;
  MenuCallback  cb[MENU_END];
}ui_menu_d;
extern ui_menu_d MENU_DATA[MENU_DONE];

ui_menu_t InitMenu(MenuId id,Vector2 pos, Vector2 size, UIAlignment align,UILayout layout, bool modal);
bool UIGetPlayerAttributeName(ui_element_t* e);
bool UIClearElements(ui_menu_t* m);
void UISyncMenu(ui_menu_t* m, FetchRate poll);
bool UICloseMenu(ui_menu_t* m);
bool UISelectOption(ui_element_t* e);
bool UITransitionScreen(ui_element_t* e);
void DrawMenu(ui_menu_t* m);
bool MenuCanChangeState(MenuState old, MenuState s);
bool MenuSetState(ui_menu_t* m, MenuState s);
void MenuOnStateChanged(ui_menu_t* m, MenuState old, MenuState s);
static bool MenuInert(ui_menu_t* self){
  return false;
}

bool MenuActivateChildren(ui_menu_t*);
typedef struct{
  Font         font;
  float        text_size, text_spacing;
  //MenuId      open_menu;
  KeyboardKey  menu_key[MENU_DONE];
  ui_menu_t    menus[MENU_DONE];
  int          num_elements;
  ui_element_t *elements[MAX_ELEMENTS];
  local_ctx_t*  contexts[SCREEN_CTX_ALL];
}ui_manager_t;

extern ui_manager_t ui;

void InitUI(void);

void UISync(FetchRate poll);
void UIRender(void);
bool TogglePause(ui_menu_t* m);
static inline bool UI_BOOL_DO_NOTHING(ui_element_t* self){return false;}


static state_change_requirement_t ELEM_STATE_REQ[ELEMENT_DONE] = {
  {ELEMENT_NONE, NEVER, ELEMENT_NONE},
  {ELEMENT_LOAD, EQUAL_TO, ELEMENT_NONE},
  {ELEMENT_HIDDEN, ALWAYS, ELEMENT_NONE},
  {ELEMENT_IDLE, ALWAYS, ELEMENT_HIDDEN},
  {ELEMENT_SHOW, ALWAYS, ELEMENT_NONE},
  {ELEMENT_FOCUSED, NOT_EQUAL_TO, ELEMENT_HIDDEN},
  {ELEMENT_ACTIVATE, ALWAYS, ELEMENT_NONE},
  {ELEMENT_TOGGLE, GREATER_THAN, ELEMENT_IDLE},
  {ELEMENT_ACTIVATED, NOT_EQUAL_TO, ELEMENT_HIDDEN}
};

typedef enum{
  NARRATE_FIRST,
  NARRATE_SECOND,
  NARRATE_THIRD,
  NARRATE_ALL,
}Narrator;

typedef enum{
  TENSE_PAST,
  TENSE_PRESENT,
//  TENSE_FUTURE,
  TENSE_ALL
}NarrativeTense;

typedef enum{
  TOKE_SLAIN,
  TOKE_MISS,
  TOKE_OWNER,
  TOKE_WHO,
  TOKE_REST,
  TOKE_PARAM,
  TOKE_ID,
  TOKE_DMG,
  TOKE_TAR,
  TOKE_AGG,
  TOKE_ENV,
  TOKE_RES_SUFF,
  TOKE_ATK,
  TOKE_ACT,
  TOKE_SCHOOL,
  TOKE_AMNT,
  TOKE_STAT,
  TOKE_ALL,
}ParseToken;

typedef struct {
    const char* name;
    ParseToken  token;
} token_lookup_t;

extern token_lookup_t TOKEN_TABLE[TOKE_ALL];
const char* ParseEntityToken(param_t);
const char* ParseResult(param_t p, InteractResult);

#endif
