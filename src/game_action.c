#include "game_types.h"
#include "game_process.h"

void InitActions(action_turn_t* actions[ACTION_DONE]){
  for (int i = 0; i < ACTION_DONE; i++)
    actions[i] = InitAction(ACTION_NONE,DES_NONE,NULL,NULL);
}

action_turn_t* InitAction(ActionType t, DesignationType targeting, TakeActionCallback fn, OnActionCallback cb){
  action_turn_t* a = calloc(1,sizeof(action_turn_t));

  a->targeting = targeting;
  a->action = t;
  a->fn = fn;
  a->cb = cb;
  return a;
}


bool ActionPlayerAttack(ent_t* e, ActionType a, KeyboardKey k){
 if(!EntCanTakeAction(e))
    return false;

  if(cell_compare(e->facing,CELL_EMPTY) || cell_compare(e->facing,CELL_UNSET))
    return false;

  TileStatus* status =malloc(sizeof(TileStatus));
  ent_t* target = MapGetOccupant(e->map, e->facing, status);
  if(target)
    return EntUseAbility(e, e->abilities[0], target);

  return false;
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

void EntActionsTaken(stat_t* self, float old, float cur){
  if(self->owner == NULL)
    return;

  if(self->owner->type == ENT_PERSON)
    WorldEndTurn();
}

bool ActionMakeSelection(Cell start, int num, bool occupied){
  ScreenActivateSelector(start,num,occupied, ActionSetTarget);
}

bool ActionInput(void){
  if(player->state == STATE_SELECTION)
    return false;

  ActionType acted = ACTION_NONE;
  for(int i = 0; i < ACTION_DONE; i++){
    if(acted>ACTION_NONE)
      break;

    ActionType a = action_keys[i].action;
    ActionKeyCallback fn = action_keys[i].fn;
    for(int j = 0; j<action_keys[i].num_keys; j++){
      KeyboardKey k = action_keys[i].keys[j];
      if(!IsKeyDown(k))
        continue;

      switch(action_keys[i].action){
        case ACTION_MOVE:
          if(fn(player,a,k))
            acted = ACTION_MOVE;
          break;
        case ACTION_ATTACK:
        case ACTION_MAGIC:
          ability_t* ability = player->abilities[action_keys[i].binding];
          switch(ability->targeting){
            case DES_SELECT_TARGET:
            case DES_MULTI_TARGET:
              ent_t* pool[8];
              int dist = ability->reach;
              Rectangle r = Rect(player->pos.x - dist, player->pos.y - dist, dist*2,2* dist);
              int num_near = WorldGetEnts(pool, FilterEntInRect,&r);
              if(num_near < 2)
                break;
              else{
                SetState(player, STATE_SELECTION,NULL);
                SetAction(player, a, ability, ability->targeting);
                ActionMakeSelection(player->facing, ability->reach ,true);
              }
              break;
            case DES_NONE:
            case DES_FACING:
            default:
              if(fn(player,a,k))
                acted = a;
              break;
          }
      }
    }
  }

  if(acted > ACTION_NONE)
    return ActionTaken(player,acted);
  
  return true;//false;
    
}

bool TakeAction(ent_t* e, action_turn_t* action){
  if(!action->fn(e,action->action,action->cb)){
    if(e->state == STATE_SELECTION)
      SetState(e,STATE_IDLE,NULL);
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

bool SetAction(ent_t* e, ActionType a, void *context, DesignationType targeting){
  if(e->actions[a]->action == ACTION_NONE)
    return false;

  for (int i = 0; i < ACTION_DONE;i++){
    if(e->actions[i]->on_deck && i!=a)
      return false;

  }

  e->actions[a]->targeting = targeting;
  e->actions[a]->num_targets = 0;
  //e->actions[a]->targets = ;
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

void ActionSetTarget(ent_t* e, ActionType a, void* target){

  map_cell_t* tile = target;
  ActionType next = ActionGetEntNext(e);
  action_turn_t* action = e->actions[next];

  action_target_t* at = calloc(1,sizeof(action_target_t));
  at->type = action->targeting;
  
  switch(action->targeting){
    case DES_SELECT_TARGET:
    case DES_MULTI_TARGET:
      at->target.mob = tile->occupant;
      break;
    default:
      break;
  }
      
  action->targets[action->num_targets++] = at;
}

bool ActionAttack(ent_t* e, ActionType a, OnActionCallback cb){
  action_turn_t* inst = e->actions[a];
  if(!inst || !inst->context)
    return false;

  ability_t* ab = (ability_t*)inst->context;

  if(!e->control || !e->control->target)
    return false;
 
  ent_t* target = e->control->target;

  if(target){
    EntUseAbility(e, ab, target);
    return true;
  }
  return false;

}

bool ActionMultiTarget(ent_t* e, ActionType a, OnActionCallback cb){
  action_turn_t* inst = e->actions[a];
  if(!inst || !inst->context)
    return false;

  ability_t* ab = (ability_t*)inst->context;

  bool success = false;

  for (int i = 0; i < inst->num_targets; i++){
    switch(ab->targeting){
      case DES_SELECT_TARGET:
      case DES_MULTI_TARGET:
        ent_t* target = inst->targets[i]->target.mob;
        success = EntUseAbility(e,ab,target)||success;
        break;
      default:
        break;
    }
  }

  if(!success)
    TraceLog(LOG_WARNING,"DEBUG");
  return success;
}
