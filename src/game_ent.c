#include "game_types.h"
#include "game_tools.h"
#include "game_math.h"
#include "game_process.h"
#include "game_helpers.h"

MAKE_ADAPTER(StepState, ent_t*);

ent_t* InitEnt(ObjectInstance data){
  ent_t* e = malloc(sizeof(ent_t));
  *e = (ent_t){0};  // zero initialize if needed
  e->type = ENT_SHAPE;

  e->shape = data.id;
  e->sprite = InitSpriteByID(data.id,&shapedata);
  e->sprite->owner = e;

  e->stats[STAT_POINTS] = InitStat(STAT_POINTS,1,10,(int)WorldGetPossibleShape()-2);
  e->events = InitEvents();

  e->control = InitController();
  for (int i = STATE_SPAWN; i < STATE_END; i++){
    if(BASE_SHAPE.behaviors[i] == BEHAVIOR_NONE)
      continue;
    e->control->bt[i] = InitBehaviorTree(BASE_SHAPE.behaviors[i]);
  }
  e->control->moves = 2;
  SetState(e,STATE_SPAWN,NULL);
  return e;
}

ent_t* InitEntStatic(TileInstance data,Vector2 pos){
  ent_t* e = malloc(sizeof(ent_t));
  *e = (ent_t){0};  // zero initialize if needed
  e->type = ENT_TILE;

  e->sprite = InitSpriteByIndex(data.id,&tiledata);
  e->sprite->owner = e;
  e->pos = pos;// = Vector2Add(Vector2Scale(e->sprite->slice->center,SPRITE_SCALE),pos);

  e->control = InitController();
  for (int i = STATE_SPAWN; i < STATE_END; i++){
    if(data.behaviors[i] == BEHAVIOR_NONE)
      continue;
    e->control->bt[i] = InitBehaviorTree(data.behaviors[i]);
  }
  e->events = InitEvents();

  SetState(e,STATE_SPAWN,NULL);
  return e;
}

void EntAddPoints(ent_t *e,EntityState old, EntityState s){
  float mul = WorldGetGridCombo(e->intgrid_pos);
  AddPoints(mul, e->stats[STAT_POINTS]->current,e->pos);
}

void EntDestroy(ent_t* e){
  if(!e || !SetState(e, STATE_END,NULL))
    return;

  if(e->sprite!=NULL){
    e->sprite->owner = NULL;
    e->sprite->is_visible = false;
  }

  e->owner->child = NULL;
  e->control = NULL;
}

bool FreeEnt(ent_t* e){
  if(!e)
    return false;

  free(e);
  return true;
}

controller_t* InitController(){
  controller_t* ctrl = malloc(sizeof(controller_t));
  *ctrl = (controller_t){0};

  return ctrl;
}

void EntSync(ent_t* e){
  if(e->control)  
    EntControlStep(e);

  if(e->events)
    StepEvents(e->events);

  if(e->type == ENT_TILE && !e->child)
    SetState(e,STATE_EMPTY,NULL);
  if(!e->sprite)
    return;

  e->sprite->pos = e->pos;// + abs(ent->sprite->offset.y);
}

void EntSetPos(ent_t *e, Vector2 pos){
  e->sprite->pos = e->pos = pos;
}

void EntControlStep(ent_t *e){
  if(!e->control || !e->control->bt || !e->control->bt[e->state])
    return;

  behavior_tree_node_t* current = e->control->bt[e->state];

  current->tick(current, e);
}

void SetViableTile(ent_t* e, EntityState old, EntityState s){
  ent_t* neighbors[4];

  int num_neighbors = WorldGetEnts(neighbors, FilterEntNeighbor, e->owner);

  for (int i = 0; i < num_neighbors; i++){
    if(!CheckEntPosition(neighbors[i],e->pos))
      continue;

    EntChangeOccupant(e, neighbors[i]);

    AudioPlayRandomSfx(SFX_ACTION,ACTION_PLACE);
    return;
  }

  SetState(e,STATE_IDLE,NULL);
}

void EntToggleTooltip(ent_t* e){
  TraceLog(LOG_INFO,"Owner at %i | %i", e->owner->intgrid_pos.x, e->owner->intgrid_pos.y);
  TraceLog(LOG_INFO,"Ent %i at %i | %i",SHAPE_TYPE(e->shape) ,e->intgrid_pos.x, e->intgrid_pos.y);
}

bool SetState(ent_t *e, EntityState s,StateChangeCallback callback){
  if(CanChangeState(e->state,s)){
    EntityState old = e->state;
    e->state = s;

    if(callback!=NULL)
      callback(e,old,s);

    OnStateChange(e,old,s);
    return true;
  }

  return false;
}

void StepState(ent_t *e){
  SetState(e, e->state+1,NULL);
}

bool CanChangeState(EntityState old, EntityState s){
  if(old == s || old > STATE_END)
    return false;

  switch(s){
    case STATE_SCORE:
      if(old < STATE_CALCULATING)
        return false;
      break;
    case STATE_CALCULATING:
      if(old>STATE_PLACED || old < STATE_IDLE)
        return false;
    default:
      return true;
  }

  return true;
} 

void OnStateChange(ent_t *e, EntityState old, EntityState s){
  switch(old){
    case STATE_HOVER:
      EntToggleTooltip(e);
      break;
    case STATE_SPAWN:
      if(e->sprite)
        e->sprite->is_visible = true;
      break;
    default:
      break;
  }

  switch(s){
    case STATE_HOVER:
      AudioPlayRandomSfx(SFX_ACTION,ACTION_HOVER);
      EntToggleTooltip(e);
      break;
    case STATE_DIE:
      TraceLog(LOG_INFO,"destroy shape %i",e->uid);
      EntDestroy(e);
      break;
    case STATE_SCORE:
      break;
    case STATE_PLACED:
      e->control->moved = true;
      break;
    default:
      break;
  }
}

void ReduceMoveCount(ent_t *e, ent_t* old, ent_t* owner){
  e->control->moves--;
  StatIncrementValue(e->stats[STAT_POINTS],false);
}

void EntChangeOccupant(ent_t* e, ent_t* owner){
  ent_t* tenant = owner->child;
  ent_t* old = e->owner;

  if(!EntSetOwner(e,owner,true,ReduceMoveCount))
    return;

  if(!tenant)
    return;

  if(!EntSetOwner(tenant,old,false,NULL)){
    EntSetOwner(e,old,true,NULL);
    return;
  }
}

void EntOnOwnerChange(ent_t *e, ent_t* old, ent_t* owner){
  if(e->uid <= 0){
    RegisterEnt(e);
    SetState(e,STATE_IDLE,NULL);
  }

  e->pos = owner->pos;
  e->intgrid_pos = owner->intgrid_pos;
  owner->shape = e->shape;
}

bool EntSetOwner(ent_t* e, ent_t* owner, bool evict, OwnerChangeCallback cb){
  if (e->owner == owner)
    return false;

  if(!evict && owner->child)
    return false;

  ent_t* old = e->owner;
  e->owner = owner;
  owner->child = e;
  if(old && old->child && old->child == e)
    old->child = NULL;

  EntOnOwnerChange(e,old, e->owner);
  if(cb)
    cb(e,old,e->owner);

  return true;
}

bool CheckEntPosition(ent_t* e, Vector2 pos){
  if(!e)
    return false;

  Vector2 topCorner = Vector2Subtract(e->sprite->pos,e->sprite->slice->center);

  Rectangle bounds = RectPos(topCorner, e->sprite->slice->bounds);
  return point_in_rect(pos, bounds);
}

bool CheckEntAvailable(ent_t* e){
  if(!e)
    return false;

  return (e->state < STATE_DIE);
}
