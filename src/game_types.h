#ifndef __GAME_TYPES__
#define __GAME_TYPES__

#include "game_common.h"
#include "game_assets.h"
#include "game_utils.h"
#include "raylib.h"
#include "game_math.h"
#include "game_tools.h"

#define GRID_SIZE 128
#define GRID_STEP 32

#define MAX_ENTS 128  
typedef struct bounds_s {
  Vector2     pos,offset;
  float       radius;
  float       width,height;
  //BoundsCheckCall col_check;
} bounds_t;

typedef struct{
  struct ent_s*           target;
  int                     moves;
  bool                    moved;
  behavior_tree_node_t*   bt[STATE_END];
}controller_t;
controller_t* InitController();

//===ENT_T===>
typedef struct ent_s{
  int                   uid;
  ShapeID               shape;
  stat_t*               stats[STAT_ENT];
  EntityType            type;
  Vector2               pos;
  Cell                  intgrid_pos;
  EntityState           state;
  controller_t          *control;
  events_t              *events;
  sprite_t              *sprite;
  struct ent_s          *child;
  struct ent_s          *owner;
} ent_t;

ent_t* InitEnt(ObjectInstance data);
ent_t* InitEntStatic(TileInstance data,Vector2 pos);

void EntToggleTooltip(ent_t* e);

void EntSync(ent_t* e);
bool EntKill(ent_t* e);
void EntDestroy(ent_t* e);
bool FreeEnt(ent_t* e);
void EntAddPoints(ent_t* e,EntityState old, EntityState s);
void EntPrepStep(ent_t *e);
void EntSetPos(ent_t *e, Vector2 pos);
void EntControlStep(ent_t *e);
typedef void (*StateChangeCallback)(ent_t *e, EntityState old, EntityState s);
void SetViableTile(ent_t*, EntityState old, EntityState s);
bool CheckEntAvailable(ent_t* e);
bool CheckEntPosition(ent_t* e, Vector2 pos);
bool SetState(ent_t *e, EntityState s,StateChangeCallback callback);
void StepState(ent_t *e);
void OnStateChange(ent_t *e, EntityState old, EntityState s);
bool CanChangeState(EntityState old, EntityState s);
void ReduceMoveCount(ent_t *e, ent_t* old, ent_t* owner);

typedef void (*OwnerChangeCallback)(ent_t *e, ent_t* old, ent_t* owner);
void EntOnOwnerChange(ent_t *e, ent_t* old, ent_t* owner);
void EntChangeOccupant(ent_t* e, ent_t* owner);
bool EntSetOwner(ent_t*, ent_t* owner, bool evict,OwnerChangeCallback cb);
#endif
