#include "game_types.h"

static game_input_t player_input;

void InitInput(ent_t* player){
  player_input.owner = player;

  player_input.actions[ACTION_MOVE] = (action_key_t){
    ACTION_MOVE,8,{KEY_D,KEY_A,KEY_W,KEY_S,KEY_LEFT, KEY_RIGHT,KEY_UP,KEY_DOWN},ActionTurnMove,SLOT_NONE};

  }

bool InputToggle(void){
  player_input.key_event = !player_input.key_event;
}

action_t* InputActionMove(action_key_t akey, KeyboardKey k){
  Cell dir = CELL_UNSET;


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

  if(cell_compare(dir, CELL_UNSET))
    return NULL;

  Cell dest = CellInc(player_input.owner->pos, dir);

  return InitActionMove(player_input.owner, ACT_MAIN, dest, 50); 
}

action_t* InputActionAttack(action_key_t akey){

}

action_t* InputActionItem(action_key_t akey){

}

action_t* InputActionMagic(action_key_t akey){

}

bool InputQueueAction(action_key_t akey, KeyboardKey k){
  action_t* a = NULL;

  switch(akey.action){
    case ACTION_MOVE:
      a = InputActionMove(akey, k);
      break;
    case ACTION_ATTACK:
    case ACTION_WEAPON:
      a = InputActionAttack(akey);
      break;
    case ACTION_ITEM:
      a = InputActionItem(akey);
      break;
    case ACTION_MAGIC:
      a = InputActionMagic(akey);
      break;
    default:
      break;
  }

  if(a == NULL)
    return false;

  ActionStatus res = QueueAction(player_input.owner->control->actions, a);

  return res == ACT_STATUS_QUEUED;
}

void InputSync(TurnPhase phase, int turn){
  if(phase == player_input.phase && turn == player_input.turn)
    return;

  player_input.phase = phase;
  player_input.turn  = turn;

  player_input.key_event = false;
}

bool InputCheck(TurnPhase phase, int turn){
  if(phase != TURN_INIT)
    return false;

  if(IsKeyDown(KEY_SPACE))
    DO_NOTHING();

  if(player_input.key_event)
    return false;

  for(int i = 0; i < ACTION_DONE; i++){
    action_key_t akey = player_input.actions[i];

    for (int j = 0; j< akey.num_keys; j++){
      KeyboardKey k = akey.keys[j];


      if(!IsKeyDown(k))
        continue;


      player_input.key_event = InputQueueAction(akey, k);
      
      return player_input.key_event;
    }
  }

  return player_input.key_event;
}
