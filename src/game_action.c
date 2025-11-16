#include "game_types.h"
#include "game_process.h"

void InitActions(action_turn_t* actions[ACTION_DONE]){
  for (int i = 0; i < ACTION_DONE; i++)
    actions[i] = InitAction(ACTION_NONE,NULL,NULL);
}

action_turn_t* InitAction(ActionType t, TakeActionCallback fn, OnActionCallback cb){
  action_turn_t* a = malloc(sizeof(action_turn_t));

  *a = (action_turn_t){0};

  a->action = t;
  a->fn = fn;
  a->cb = cb;
  return a;
}

bool ActionMove(ent_t* e, ActionType a, KeyboardKey k){
  if(!EntCanTakeAction(e,a))
    return false;

 Cell dir;

switch(k){
  case KEY_A:
  case KEY_LEFT:
    dir = CELL_LEFT;
    break;
  case KEY_D:
  case KEY_RIGHT:
    dir = CELL_RIGHT;
    break;
  case KEY_W:
  case KEY_UP:
    dir = CELL_UP;
    break;
  case KEY_S:
  case KEY_DOWN:
    dir = CELL_DOWN;
    break;
  default:
    break;
} 
 
  EntGridStep(e,dir);

  return true;
}

bool EntCanTakeAction(ent_t* e, ActionType a){
  if(e->state == STATE_ACTION)
    return false;

  return true;
}

void ActionSync(ent_t* e){
  if(e->state == STATE_ACTION){
    SetState(e,STATE_STANDBY,NULL);

    return;
  }

  if(e->state == STATE_STANDBY){
    if(!SetState(e, e->previous,NULL))
      SetState(e,STATE_IDLE,NULL);

    return;
  }
  for (int i = 0; i < ACTION_DONE; i++){
    if(!e->actions[i]->context)
      continue;

  
    if(!EntCanTakeAction(e,i))
      continue;

    if(TakeAction(e,e->actions[i]))
      return;
  }
}

bool ActionInput(void){
  for(int i = 0; i < ACTION_DONE; i++){
    ActionType a = action_keys[i].action;
    ActionKeyCallback fn = action_keys[i].fn;
    for(int j = 0; j<action_keys[i].num_keys; j++){
      KeyboardKey k = action_keys[i].keys[j];
      if(!IsKeyDown(k))
        continue;

      if(fn(player,a,k)){
        SetState(player,STATE_ACTION,NULL);
        return true;
      }
    }
  }

  return false;
    
}

bool TakeAction(ent_t* e, action_turn_t* action){
  if(!action->fn(e,action->action,action->cb))
    return false;

  if(action->cb)
    action->cb(e,action->action);

  return ActionTaken(e, action->action);
}

bool ActionTaken(ent_t* e, ActionType a){
  return SetState(e, STATE_ACTION,NULL);
}

bool SetAction(ent_t* e, ActionType a, void *context){
  if(e->actions[a]->action == ACTION_NONE)
    return false;

  e->actions[a]->context = context;

}

bool ActionTraverseGrid(ent_t* e,  ActionType a, OnActionCallback cb){
  action_turn_t* inst = e->actions[a];
  if(!inst || !inst->context)
    return false;
  Cell* dest = (Cell*)inst->context;

  int x = e->pos.x;
  int y = e->pos.y;
  int tx = dest->x;
  int ty = dest->y;

  int dx = tx - x;
  int dy = ty - y;

  // choose the dominant axis (prioritize horizontal or vertical)
  int stepX = 0;
  int stepY = 0;

  if (abs(dx) > abs(dy)) {
    stepX = (dx > 0) ? 1 : -1;
  } else if (dy != 0) {
    stepY = (dy > 0) ? 1 : -1;
  }

  Cell step = { stepX, stepY };

  EntGridStep(e,step);
  e->actions[a]->context = NULL;
  return true;
}
