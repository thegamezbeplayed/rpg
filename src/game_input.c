#include "game_types.h"
#include "game_process.h"

static game_input_t player_input;


BehaviorStatus InputActionMove(ent_t* e, action_key_t akey, KeyboardKey k){
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
    return BEHAVIOR_FAILURE;

  Cell coords = CellInc(dir, e->pos);
  if(MapTileAvailable(e->map, coords) >= TILE_ISSUES){
    e->facing = coords;
    return BEHAVIOR_FAILURE;
  }

  param_t p = ParamMake(DATA_CELL, sizeof(Cell), &dir);

  player_input.decisions[ACTION_MOVE]->params[ACT_PARAM_STEP] = p;
  action_t* a = InitActionByDecision(player_input.decisions[ACTION_MOVE], ACTION_MOVE);

  return ActionExecute(ACTION_MOVE, a);
}

BehaviorStatus InputActionAttack(ent_t* e, action_key_t a, KeyboardKey k){

  AbilityID aid = ABILITY_WEAP_BLUDGEON;
  player_input.decisions[ACTION_ATTACK]->params[ACT_PARAM_ABILITY] = ParamMake(DATA_INT, sizeof(int), &aid);

  local_ctx_t* tar = EntGetTarget(e, aid);

  param_t p = ParamMake(DATA_LOCAL_CTX, sizeof(local_ctx_t), tar);
  player_input.decisions[ACTION_ATTACK]->params[ACT_PARAM_TAR] = p;

  action_t* act = InitActionByDecision(player_input.decisions[ACTION_ATTACK], ACTION_ATTACK);
;

  return ActionExecute(ACTION_ATTACK, act);
}

BehaviorStatus InputActionItem(ent_t* e, action_key_t a,  KeyboardKey k){

}

BehaviorStatus InputActionMagic(ent_t* e, action_key_t a, KeyboardKey k){

}


void InitInput(ent_t* player){
  player_input.owner = player;

  player_input.key_event = BEHAVIOR_FAILURE;
  player->control->decider[STATE_STANDBY] = InitDecisionPool(ACTION_DONE, player, STATE_STANDBY);

  for(int i = 0; i < ACTION_PASSIVE; i++){
    player_input.decisions[i] = InitDecision(player->control->decider[STATE_STANDBY], i);
    player_input.decisions[i]->score = 100;
    player_input.decisions[i]->cost = 1;
    player_input.decisions[i]->decision = i;
  }
  player_input.actions[ACTION_ATTACK] = (action_key_t){
    ACTION_WEAPON,1,{KEY_F},InputActionAttack,SLOT_NONE};


  player_input.actions[ACTION_MOVE] = (action_key_t){
    ACTION_MOVE,8,{KEY_D,KEY_A,KEY_W,KEY_S,KEY_LEFT, KEY_RIGHT,KEY_UP,KEY_DOWN},InputActionMove,SLOT_ATTACK};

  }

void InputSync(TurnPhase phase, int turn){
  if(phase == player_input.phase && turn == player_input.turn)
    return;

  player_input.phase = phase;
  player_input.turn  = turn;

  player_input.key_event = BEHAVIOR_FAILURE;
}

bool InputCheck(TurnPhase phase, int turn){
  if(phase != TURN_INIT)
    return false;

  if(IsKeyDown(KEY_SPACE))
    moncontrol(1);

  if(player_input.key_event == BEHAVIOR_SUCCESS)
    return false;

  for(int i = 0; i < ACTION_DONE; i++){
    action_key_t akey = player_input.actions[i];

    for (int j = 0; j< akey.num_keys; j++){
      KeyboardKey k = akey.keys[j];


      if(!IsKeyDown(k))
        continue;


      player_input.key_event = akey.fn(player, akey, k);
      
      return (player_input.key_event == BEHAVIOR_SUCCESS);
    }
  }

  return (player_input.key_event == BEHAVIOR_SUCCESS);
}
