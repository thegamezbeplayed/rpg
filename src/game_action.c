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

bool ActionPlayerAttack(ent_t* e, ActionType a, KeyboardKey k){
 if(!EntCanTakeAction(e))
    return false;

  if(cell_compare(e->facing,CELL_EMPTY))
    return false;

  TileStatus* status =malloc(sizeof(TileStatus));
  ent_t* target = MapGetOccupant(e->map, e->facing, status);
  if(target)
    return EntUseAbility(e, e->abilities[0], target);

  return true;
}

bool ActionMove(ent_t* e, ActionType a, KeyboardKey k){
  if(!EntCanTakeAction(e))
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

  if(EntGridStep(e,dir)<TILE_ISSUES)
    return true;

  return false;
}

bool EntCanTakeAction(ent_t* e){
  if(e->state == STATE_STANDBY)
    return false;

  if(e->stats[STAT_ACTIONS]->current < 1)
    return false;

  return WorldGetTurnState();
}

void ActionSync(ent_t* e){

  StatMaxOut(e->stats[STAT_ACTIONS] );

  if(e->state == STATE_STANDBY){
    //if(!SetState(e, e->previous,NULL))
      SetState(e,STATE_IDLE,NULL);
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
        SetState(player,STATE_STANDBY,NULL);
        return true;
      }
    }
  }

  return false;
    
}

bool TakeAction(ent_t* e, action_turn_t* action){
  if(!action->fn(e,action->action,action->cb)){
    SetState(e, STATE_STANDBY,NULL);
    return false;
  }

  if(action->cb)
    action->cb(e,action->action);

  return ActionTaken(e, action->action);
}

ActionType ActionGetEntNext(ent_t* e){
  ActionType next = ACTION_NONE;

  for(int i = 0; i < ACTION_DONE; i++){
    if(!e->actions[i]->on_deck)
      continue;

    if(e->actions[i]->context)
      return i;
  }

  return next;
}

bool ActionTaken(ent_t* e, ActionType a){
  e->actions[a]->on_deck = false;
  StatIncrementValue(e->stats[STAT_ACTIONS],false);

  return SetState(e, STATE_STANDBY,NULL);
}

bool SetAction(ent_t* e, ActionType a, void *context){
  if(e->actions[a]->action == ACTION_NONE)
    return false;

  for (int i = 0; i < ACTION_DONE;i++){
    if(e->actions[i]->on_deck && i!=a)
      return false;

  }

  e->actions[a]->context = context;
  e->actions[a]->on_deck = true;
  
  return true;
}

bool ActionTraverseGrid(ent_t* e,  ActionType a, OnActionCallback cb){
  action_turn_t* inst = e->actions[a];
  if(!inst || !inst->context)
    return false;
  Cell* dest = (Cell*)inst->context;

  EntGridStep(e,*dest);
  e->actions[a]->context = NULL;
  return true;
}

bool ActionAttack(ent_t* e, ActionType a, OnActionCallback cb){
  action_turn_t* inst = e->actions[a];
  if(!inst || !inst->context)
    return false;

  ability_t* ab = (ability_t*)inst->context;

  if(!e->control || !e->control->target)
    return false;
 
  ent_t* target = e->control->target;

  if(target)
    return EntUseAbility(e, ab, target);

  return false;

}
