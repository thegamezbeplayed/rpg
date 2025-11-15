#ifndef __GAME_COMMON__
#define __GAME_COMMON__

#include "raylib.h"
#define SHAPE_COUNT 41

static inline bool LESS_THAN(int a, int b){
  return a<b;
}

static inline bool GREATER_THAN(int a, int b){
  return a>b;
}

static inline bool EQUAL_TO(int a, int b){
  return a==b;
}

typedef enum{
  ENT_NONE,
  ENT_SHAPE,
  ENT_TILE,
  ENT_DONE
}EntityType;

typedef enum {
  BACKTILE_01,
  BACKTILE_02,
  BACKTILE_03,
  BACKTILE_04,
  BACKTILE_05,
  BACKTILE_06,
  BACKTILE_07,
  BACKTILE_08,
  BACKTILE_09,
  BACKTILE_10,
  BACKTILE_11,
  BACKTILE_12,
  BACKTILE_13,
  BACKTILE_14,
  BACKTILE_15,
  BACKTILE_16,
  BACKTILE_17,
  BACKTILE_18,
  BACKTILE_DONE
} BackTileID;

typedef enum{
  BEHAVIOR_NONE,
  BEHAVIOR_CHANGE_STATE,
  BEHAVIOR_CHANGE_OWNER_STATE,
  BEHAVIOR_CHANGE_CHILD_STATE,
  BEHAVIOR_CLEAR_MATCH_STATE,
  BEHAVIOR_SELECT_SHAPE,
  BEHAVIOR_HELPFUL_SHAPE,
  BEHAVIOR_INIT_CHILD,
  BEHAVIOR_ACQUIRE_CHILD,
  BEHAVIOR_SOLVABLE_CHILD,
  BEHAVIOR_CHECK_OTHERS_STATE,
  BEHAVIOR_CHECK_CHILD_STATE,
  BEHAVIOR_CHECK_OWNERS_STATE,
  BEHAVIOR_PROGRESS_WORLD_STATE,
  BEHAVIOR_CHECK_WORLD_STATE,
  BEHAVIOR_SHAPE_PLACED,
  BEHAVIOR_START_CALC,
  BEHAVIOR_MATCH_NEIGHBORS,
  BEHAVIOR_STANDBY,
  BEHAVIOR_DEPOSIT_POINTS,
  BEHAVIOR_MATCH_GRID,
  BEHAVIOR_CALC_TILE,
  BEHAVIOR_CALCULATE,
  BEHAVIOR_SCORE_TURN,
  BEHAVIOR_MATCH_CHILDREN,
  BEHAVIOR_END_TURN,
  BEHAVIOR_IDLE_TILE,
  BEHAVIOR_CHECK_TURN,
  BEHAVIOR_NEXT_TURN,
  BEHAVIOR_COUNT
}BehaviorID;

typedef struct sub_texture_s {
    int tag;
    int originX, originY;
    int positionX, positionY;
    int sourceWidth, sourceHeight;
    int padding;
    bool trimmed;
    int trimRecX, trimRecY, trimRecWidth, trimRecHeight;
    int colliderType;
    int colliderPosX, colliderPosY, colliderSizeX, colliderSizeY;
} sub_texture_t;

typedef enum {
    // Colors (high nibble)
    SHAPE_COLOR_NONE   = 0x00,
    SHAPE_COLOR_GREEN  = 0x10,
    SHAPE_COLOR_GRAY   = 0x20,
    SHAPE_COLOR_BLUE   = 0x30,
    SHAPE_COLOR_RED    = 0x40,
    // ... add more if needed

    SHAPE_COLOR_MAX    =0x40,
    SHAPE_COLOR_MASK   = 0xF0,

    // Shapes (low nibble)
    SHAPE_TYPE_NONE     = 0x00,
    SHAPE_TYPE_SQUARE   = 0x01,
    SHAPE_TYPE_DIAMOND  = 0x02,
    SHAPE_TYPE_PENTAGON = 0x03,
    SHAPE_TYPE_STUD     = 0x04,
    SHAPE_TYPE_TRIANGLE = 0x05,
    SHAPE_TYPE_COMPASS  = 0x06,
    SHAPE_TYPE_STAR     = 0x07,
    SHAPE_TYPE_GEM      = 0x08,
    SHAPE_TYPE_CIRCLE   = 0x09,
    SHAPE_TYPE_HEART    = 0x0A,

    SHAPE_TYPE_MAX     = 0x0B,
    SHAPE_TYPE_MASK     = 0x0F,
} ShapeFlags;

typedef enum{
  SHAPE_NONE = SHAPE_COLOR_NONE| SHAPE_TYPE_NONE,
  SHAPE_GREEN_SQUARE = SHAPE_COLOR_GREEN | SHAPE_TYPE_SQUARE,
  SHAPE_GREEN_DIAMOND = SHAPE_COLOR_GREEN | SHAPE_TYPE_DIAMOND,
  SHAPE_GREEN_PENTAGON = SHAPE_COLOR_GREEN | SHAPE_TYPE_PENTAGON,
  SHAPE_GREEN_STUD = SHAPE_COLOR_GREEN | SHAPE_TYPE_STUD,
  SHAPE_GREEN_TRIANGLE = SHAPE_COLOR_GREEN | SHAPE_TYPE_TRIANGLE,
  SHAPE_GREEN_COMPASS = SHAPE_COLOR_GREEN | SHAPE_TYPE_COMPASS,
  SHAPE_GREEN_STAR = SHAPE_COLOR_GREEN | SHAPE_TYPE_STAR,
  SHAPE_GREEN_GEM = SHAPE_COLOR_GREEN | SHAPE_TYPE_GEM,
  SHAPE_GREEN_CIRCLE = SHAPE_COLOR_GREEN | SHAPE_TYPE_CIRCLE,
  SHAPE_GREEN_HEART = SHAPE_COLOR_GREEN | SHAPE_TYPE_HEART,
  SHAPE_GRAY_SQUARE = SHAPE_COLOR_GRAY | SHAPE_TYPE_SQUARE,
  SHAPE_GRAY_DIAMOND = SHAPE_COLOR_GRAY | SHAPE_TYPE_DIAMOND,
  SHAPE_GRAY_PENTAGON = SHAPE_COLOR_GRAY | SHAPE_TYPE_PENTAGON,
  SHAPE_GRAY_STUD = SHAPE_COLOR_GRAY | SHAPE_TYPE_STUD,
  SHAPE_GRAY_TRIANGLE = SHAPE_COLOR_GRAY | SHAPE_TYPE_TRIANGLE,
  SHAPE_GRAY_COMPASS = SHAPE_COLOR_GRAY | SHAPE_TYPE_COMPASS,
  SHAPE_GRAY_STAR = SHAPE_COLOR_GRAY | SHAPE_TYPE_STAR,
  SHAPE_GRAY_GEM = SHAPE_COLOR_GRAY | SHAPE_TYPE_GEM,
  SHAPE_GRAY_CIRCLE = SHAPE_COLOR_GRAY | SHAPE_TYPE_CIRCLE,
  SHAPE_GRAY_HEART = SHAPE_COLOR_GRAY | SHAPE_TYPE_HEART,
  SHAPE_BLUE_SQUARE = SHAPE_COLOR_BLUE | SHAPE_TYPE_SQUARE,
  SHAPE_BLUE_DIAMOND = SHAPE_COLOR_BLUE | SHAPE_TYPE_DIAMOND,
  SHAPE_BLUE_PENTAGON = SHAPE_COLOR_BLUE | SHAPE_TYPE_PENTAGON,
  SHAPE_BLUE_STUD = SHAPE_COLOR_BLUE | SHAPE_TYPE_STUD,
  SHAPE_BLUE_TRIANGLE = SHAPE_COLOR_BLUE | SHAPE_TYPE_TRIANGLE,
  SHAPE_BLUE_COMPASS = SHAPE_COLOR_BLUE | SHAPE_TYPE_COMPASS,
  SHAPE_BLUE_STAR = SHAPE_COLOR_BLUE | SHAPE_TYPE_STAR,
  SHAPE_BLUE_GEM = SHAPE_COLOR_BLUE | SHAPE_TYPE_GEM,
  SHAPE_BLUE_CIRCLE = SHAPE_COLOR_BLUE | SHAPE_TYPE_CIRCLE,
  SHAPE_BLUE_HEART = SHAPE_COLOR_BLUE | SHAPE_TYPE_HEART,
  SHAPE_RED_SQUARE = SHAPE_COLOR_RED | SHAPE_TYPE_SQUARE,
  SHAPE_RED_DIAMOND = SHAPE_COLOR_RED | SHAPE_TYPE_DIAMOND,
  SHAPE_RED_PENTAGON = SHAPE_COLOR_RED | SHAPE_TYPE_PENTAGON,
  SHAPE_RED_STUD = SHAPE_COLOR_RED | SHAPE_TYPE_STUD,
  SHAPE_RED_TRIANGLE = SHAPE_COLOR_RED | SHAPE_TYPE_TRIANGLE,
  SHAPE_RED_COMPASS = SHAPE_COLOR_RED | SHAPE_TYPE_COMPASS,
  SHAPE_RED_STAR = SHAPE_COLOR_RED | SHAPE_TYPE_STAR,
  SHAPE_RED_GEM = SHAPE_COLOR_RED | SHAPE_TYPE_GEM,
  SHAPE_RED_CIRCLE = SHAPE_COLOR_RED | SHAPE_TYPE_CIRCLE,
  SHAPE_RED_HEART = SHAPE_COLOR_RED | SHAPE_TYPE_HEART,
  SHAPE_DONE = 0xFF
}ShapeID;

typedef struct{
  const char* text;
  Vector2     pos;
  int         size;
  Color       color;
  int         duration;
}render_text_t;

void AddFloatingText(render_text_t *rt);

struct ent_s;   // forward declaration

//====EVENTS===>
typedef enum{
  EVENT_GAME_PROCESS,
  EVENT_INTERACTION,
  EVENT_PLAY_SFX,
  EVENT_SONG_END,
  EVENT_SONG_FADE_IN,
  EVENT_SONG_FADE_OUT,
  EVENT_WAIT,
  EVENT_FINISH,
  EVENT_NONE,
  MAX_EVENTS
} EventType;

typedef void (*CooldownCallback)(void* params);

typedef struct{
  EventType         type;
  int               duration;
  int               elapsed;
  bool              is_complete;
  bool              is_recycled;
  void*             on_end_params;
  void*             on_step_params;
  CooldownCallback  on_end;
  CooldownCallback  on_step;
}cooldown_t;
cooldown_t* InitCooldown(int dur, EventType type, CooldownCallback on_end_callback, void* params);
void UnloadCooldown(cooldown_t* cd);

typedef struct{
  cooldown_t  cooldowns[MAX_EVENTS];
  bool        cooldown_used[MAX_EVENTS];
}events_t;

events_t* InitEvents();
void UnloadEvents(events_t* ev);
int AddEvent(events_t* pool, cooldown_t* cd);
void StepEvents(events_t* pool);
void StartEvent(events_t* pool, EventType type);
void ResetEvent(events_t* pool, EventType type);
bool CheckEvent(events_t* pool, EventType type);
typedef enum{
  STAT_POINTS,
  STAT_TURNS,
  STAT_ENT,
  STAT_MAX_COLOR,
  STAT_MAX_TYPE,
  STAT_COLOR_MUL,
  STAT_TYPE_MUL,
  STAT_COMBO_MUL,
  STAT_BLANK
}StatType;

struct stat_s;
typedef bool (*StatOwnerCallback)(struct ent_s* owner);
typedef void (*StatCallback)(struct ent_s* owner, float old, float cur);
typedef float (*StatGetter)(struct stat_s* self);
typedef struct stat_s{
  StatType  attribute;
  float     min;
  float     max;
  float     current;
  float     increment;
  StatGetter ratio;
  StatCallback on_stat_change;
  StatOwnerCallback on_stat_empty;
} stat_t;

stat_t* InitStatOnMin(StatType attr, float min, float max);
stat_t* InitStatOnMax(StatType attr, float val);
stat_t* InitStatEmpty(void);
stat_t* InitStat(StatType attr,float min, float max, float amount);
bool StatIncrementValue(stat_t* attr,bool increase);
bool StatChangeValue(struct ent_s* owner, stat_t* attr, float val);
void StatMaxOut(stat_t* s);
void StatEmpty(stat_t* s);
float StatGetRatio(stat_t *self);
//<====STATS

typedef enum{
  STATE_NONE,//if ent_t is properly initalized to {0} this is already set
  STATE_SPAWN,//Should only be set after NONE
  STATE_EMPTY,
  STATE_IDLE, //should be able to move freely between these ==>
  STATE_SELECTED,
  STATE_HOVER,
  STATE_PLACED,
  STATE_CALCULATING,
  STATE_SCORE,
  STATE_DIE,//<===== In MOST cases. Should not be able to go down from DIE
  STATE_END//sentinel entity state should never be this or greater
}EntityState;

typedef enum{
  TURN_START,
  TURN_INPUT,
  TURN_CALC,
  TURN_SCORE,
  TURN_END,
  TURN_STANDBY,
  TURN_COUNT
}TurnState;

typedef struct{
  const char* name;
  TurnState  state;
}TurnStateName;

typedef bool (*StateComparator)(int a, int b);

typedef struct{
  int             state;
  StateComparator can;
  int             required;
}state_change_requirement_t;

static state_change_requirement_t turn_reqs[TURN_COUNT] = {
  {TURN_START,EQUAL_TO,TURN_STANDBY},
  {TURN_INPUT,LESS_THAN, TURN_CALC},
  {TURN_CALC, EQUAL_TO, TURN_INPUT},
  {TURN_SCORE, EQUAL_TO, TURN_CALC},
  {TURN_END, EQUAL_TO, TURN_SCORE},
  {TURN_STANDBY, EQUAL_TO, TURN_END},
};

static TurnStateName turn_name[TURN_COUNT] = {
  {"Start", TURN_START},
  {"Input", TURN_INPUT},
  {"Calc", TURN_CALC},
  {"Score", TURN_SCORE},
  {"End", TURN_END},
  {"Standby", TURN_STANDBY},
};

typedef struct {
  ShapeID     id;
  int         points;
  BehaviorID  behaviors[STATE_END];
} ObjectInstance;

typedef struct {
  BackTileID  id;
  BehaviorID  behaviors[STATE_END];
} TileInstance;

typedef enum{
  ELEMENT_EMPTY,
  ELEMENT_PANEL_GRAY,
  ELEMENT_PANEL_GRAY_WIDE,
  ELEMENT_BUTTON_GRAY_ACTIVE,
  ELEMENT_BUTTON_GRAY,
  ELEMENT_BOX_GRAY,
  ELEMENT_ERROR_WIDE,
  ELEMENT_COUNT
}ElementID;

#endif
