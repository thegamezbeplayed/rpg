#include "game_types.h"
#include "game_tools.h"
#include "game_math.h"
#include "game_process.h"

MAKE_ADAPTER(StepState, ent_t*);

ent_t* InitEnt(ObjectInstance data){
  ent_t* e = malloc(sizeof(ent_t));
  *e = (ent_t){0};  // zero initialize if needed
  e->type = data.id;

  e->map = WorldGetMap();
  e->pos = data.pos;
  e->facing = CELL_EMPTY;
  e->sprite = InitSpriteByID(data.id,&tiledata);
  e->sprite->owner = e;

  e->events = InitEvents();

  e->control = InitController();
  MobCategory cat = GetEntityCategory(e->type);

  e->attack = InitBasicAttack();

  category_stats_t base = CATEGORY_STATS[cat];
  for (int i = 0; i < ATTR_DONE; i++)
    e->attribs[i] = InitAttribute(i, base.attr[i]);

  for (int i = 0; i < STAT_DONE;i++){
    if (base.stats[i] ==0)
      continue;


    e->stats[i] = InitStatOnMax(i,base.stats[i]);
  }

  e->stats[STAT_HEALTH]->on_stat_empty = EntKill;

  InitActions(e->actions);

  if(e->type != ENT_PERSON){
    e->control->ranges[RANGE_NEAR] = e->stats[STAT_AGGRO]->current;
    e->control->ranges[RANGE_LOITER] = e->stats[STAT_AGGRO]->current+2;
    for (int i = STATE_SPAWN; i < STATE_END; i++){
      if(data.behaviors[i] == BEHAVIOR_NONE)
        continue;
      e->control->bt[i] = InitBehaviorTree(data.behaviors[i]);
    }

    e->actions[ACTION_MOVE] = InitAction(ACTION_MOVE,ActionTraverseGrid,NULL);
    e->actions[ACTION_ATTACK] = InitAction(ACTION_ATTACK,ActionAttack,NULL);
  }

  SetState(e,STATE_SPAWN,NULL);
  return e;
}

ent_t* InitEntStatic(TileInstance data,Vector2 pos){
  ent_t* e = malloc(sizeof(ent_t));
  *e = (ent_t){0};  // zero initialize if needed
  //e->type = ENT_TILE;

  e->sprite = InitSpriteByIndex(data.id,&tiledata);
  e->sprite->owner = e;
  //e->pos = pos;// = Vector2Add(Vector2Scale(e->sprite->slice->center,SPRITE_SCALE),pos);

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

bool EntKill(ent_t* e){
  return SetState(e, STATE_DIE,NULL);
}

void EntInitOnce(ent_t* e){
  EntSync(e);

  cooldown_t* spawner = InitCooldown(3,EVENT_SPAWN,StepState_Adapter,e);
  AddEvent(e->events, spawner);
}

void EntDestroy(ent_t* e){
  if(!e || !SetState(e, STATE_END,NULL))
    return;

  MapRemoveOccupant(e->map,e->pos);

  if(e->sprite!=NULL){
    e->sprite->owner = NULL;
    e->sprite->is_visible = false;
  }

  e->control = NULL;
}

bool EntHit(ent_t* e, attack_t* a){
  int damage = -1 * a->stats[STAT_DAMAGE]->current;
  if(StatChangeValue(e,e->stats[STAT_HEALTH], damage)){
   TraceLog(LOG_INFO,"Ent %i hit health now %0.0f/%0.0f",e->uid, e->stats[STAT_HEALTH]->current,e->stats[STAT_HEALTH]->max);

    return true;
  }

  return false;
}

bool EntAttack(ent_t* e, attack_t* a, ent_t* target){
  TraceLog(LOG_INFO,"Ent %i swings at ent %i",e->uid,target->uid);
  return EntHit(target, a);
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
attack_t* InitBasicAttack(void){
  attack_t* a = malloc(sizeof(attack_t));

  a->stats[STAT_REACH] = InitStat(STAT_REACH,1,1,1);
  a->stats[STAT_DAMAGE] = InitStat(STAT_DAMAGE,1,1,1);

  return a;
}

attack_t* EntGetCurrentAttack(ent_t* e){
  return e->attack;
}

void EntSync(ent_t* e){
  if(e->control)  
    EntControlStep(e);

  if(e->events)
    StepEvents(e->events);

  if(!e->sprite)
    return;

  e->sprite->pos = CellToVector2(e->pos,CELL_WIDTH);// + abs(ent->sprite->offset.y);
}

TileStatus EntGridStep(ent_t *e, Cell step){
  Cell newPos = CellInc(e->pos,step);
  TileStatus status = MapChangeOccupant(e->map,e,e->pos,newPos);

  if(status < TILE_ISSUES){

    e->pos = newPos;
    e->facing = CellInc(e->pos,step);
  }
  return status;
}

void EntSetCell(ent_t *e, Cell pos){
  e->pos = pos;
}

void EntControlStep(ent_t *e){
  if(!e->control || !e->control->bt || !e->control->bt[e->state])
    return;

  behavior_tree_node_t* current = e->control->bt[e->state];

  current->tick(current, e);
}

void EntToggleTooltip(ent_t* e){
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
    case STATE_NONE:
      return false;
      break;
    default:
      return true;
      break;
  }

  return true;
} 

void OnStateChange(ent_t *e, EntityState old, EntityState s){
  switch(old){
    case STATE_SPAWN:
      if(e->sprite)
        e->sprite->is_visible = true;
      break;
    default:
      break;
  }

  switch(s){
    case STATE_DIE:
      EntDestroy(e);
      break;
    default:
      break;
  }
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

MobCategory GetEntityCategory(EntityType t){
 if (t >= 0 && t < ENT_DONE)
        return ENTITY_CATEGORY_MAP[t];
    return MOB_HUMANOID; // fallback default
}
