#include "game_types.h"
#include "game_process.h"

game_input_t player_input;

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

  if(ActionExecute(ACTION_MOVE, a) == BEHAVIOR_SUCCESS){
    WorldEvent(EVENT_PLAYER_INPUT, a, player->gouid);
    return BEHAVIOR_SUCCESS;
  }

  return BEHAVIOR_FAILURE;
}

BehaviorStatus InputMultiTarget(ent_t* e, ActionType a, param_t sel){
  map_cell_t** selections = GameCalloc("InputMultiTarget", sel.size, sizeof(map_cell_t));

  ability_t* abi = player_input.sel_abi;
  selections = sel.data;

  int num_ents = 0;
  local_ctx_t** locals = GameCalloc("InputMultiTarget",
      sel.size, sizeof(local_ctx_t*));

  for(int i = 0; i < sel.size; i++){
    map_cell_t* mc = selections[i];

    if(!mc->occupant)
      continue;

    if(mc->occupant == e && (abi->req  & CTX_TAR_SELF)==0)
      continue;

    locals[num_ents++] = WorldGetContext(DATA_ENTITY, mc->occupant->gouid);
  }

  if(num_ents == 0)
    return BEHAVIOR_FAILURE;

  param_t multi = ParamMakeArray(DATA_ARRAY, e->gouid, locals, num_ents, sizeof(local_ctx_t*));

  player_input.decisions[ACTION_ATTACK]->params[ACT_PARAM_TAR] = multi;

  player_input.decisions[ACTION_ATTACK]->params[ACT_PARAM_ABILITY] = ParamMakeObj(DATA_ABILITY, abi->id , abi);

  action_t* act = InitActionByDecision(player_input.decisions[ACTION_ATTACK], ACTION_ATTACK);

  if(ActionExecute(ACTION_ATTACK, act) == BEHAVIOR_SUCCESS){
    WorldEvent(EVENT_PLAYER_INPUT, act, player->gouid);
    return BEHAVIOR_SUCCESS;
  }
  
  return BEHAVIOR_FAILURE;
}

BehaviorStatus InputSetTarget(ent_t* e, ActionType a, param_t sel){
  if(sel.type_id == DATA_NONE || !player_input.sel_abi)
    return BEHAVIOR_FAILURE;

  local_ctx_t* tar = WorldGetContext(sel.type_id, sel.gouid); 

  ability_t* abi = player_input.sel_abi;

  player_input.decisions[ACTION_ATTACK]->params[ACT_PARAM_ABILITY] = ParamMakeObj(DATA_ABILITY, abi->id , abi);

  param_t p = ParamMake(DATA_LOCAL_CTX, sizeof(local_ctx_t), tar);
  player_input.decisions[ACTION_ATTACK]->params[ACT_PARAM_TAR] = p;

  action_t* act = InitActionByDecision(player_input.decisions[ACTION_ATTACK], ACTION_ATTACK);
 
  if(ActionExecute(ACTION_ATTACK, act) == BEHAVIOR_SUCCESS){
    WorldEvent(EVENT_PLAYER_INPUT, act, player->gouid);
   return BEHAVIOR_SUCCESS;
  }
  
  return BEHAVIOR_FAILURE;
}

BehaviorStatus SelectActionAttack(ent_t* e, action_key_t a, KeyboardKey k){
  WorldEvent(EVENT_PLAYER_INPUT, &a, e->gouid);
}

BehaviorStatus InputActionAttack(ent_t* e, action_key_t a, KeyboardKey k){

  ability_t* abi = NULL;
  switch(k){
    case KEY_M:
      abi = e->slots[SLOT_SPELL]->abilities[0];
      break;
    case KEY_F:
      abi = e->slots[SLOT_ATTACK]->abilities[0];
      break;
    case KEY_R:
      abi = e->slots[SLOT_ITEM]->abilities[0];
      break;
  }

  if(!abi)
    return BEHAVIOR_FAILURE;

  local_ctx_t* tar = NULL;
  switch(abi->targeting){
    case DES_NONE:
    case DES_FACING:
      tar = EntGetTarget(e, abi->id);
      break;
    case DES_SELF:
      tar = WorldGetContext(DATA_ENTITY, e->gouid);
      break;
    case DES_SEL_TAR:
    case DES_MULTI_TAR:
      player_input.sel_abi = abi;
      int amnt = imax(abi->vals[VAL_QUANT],1);
      ScreenActivateSelector(e->pos, amnt, true, NULL, InputSetTarget);
      return BEHAVIOR_RUNNING;
      break;
  }

  if(!tar)
    return BEHAVIOR_FAILURE;

  player_input.decisions[ACTION_ATTACK]->params[ACT_PARAM_ABILITY] = ParamMakeObj(DATA_ABILITY, abi->id , abi);


  param_t p = ParamMake(DATA_LOCAL_CTX, sizeof(local_ctx_t), tar);
  player_input.decisions[ACTION_ATTACK]->params[ACT_PARAM_TAR] = p;

  action_t* act = InitActionByDecision(player_input.decisions[ACTION_ATTACK], ACTION_ATTACK);
  ;

  if(ActionExecute(ACTION_ATTACK, act) == BEHAVIOR_SUCCESS){
    WorldEvent(EVENT_PLAYER_INPUT, act, player->gouid);
    return BEHAVIOR_SUCCESS;
  }

  return BEHAVIOR_FAILURE;

}



void InitInput(ent_t* player){
  player_input.owner = player;

  player_input.active = true;
  player->control->decider[STATE_STANDBY] = InitDecisionPool(ACTION_DONE, player, STATE_STANDBY);

  for(int i = 0; i < ACTION_PASSIVE; i++){
    player_input.decisions[i] = InitDecision(player->control->decider[STATE_STANDBY], i);
    player_input.decisions[i]->score = 100;
    player_input.decisions[i]->cost = 1;
    player_input.decisions[i]->decision = i;
  }

  player_input.actions[ACTION_MAGIC] = (action_key_t){
    ACTION_MAGIC,1,{KEY_M},InputActionAttack,SLOT_SPELL};


  player_input.actions[ACTION_ATTACK] = (action_key_t){
    ACTION_WEAPON,1,{KEY_F},InputActionAttack,SLOT_ATTACK};


  player_input.actions[ACTION_MOVE] = (action_key_t){
    ACTION_MOVE,8,{KEY_D,KEY_A,KEY_W,KEY_S,KEY_LEFT, KEY_RIGHT,KEY_UP,KEY_DOWN},InputActionMove,SLOT_NONE};

}

void InputSync(TurnPhase phase, int turn){
  if(phase == player_input.phase && turn == player_input.turn)
    return;

  player_input.phase = phase;
  player_input.turn  = turn;
}

void InputReset(void){
  player_input.active = true;
  
  for(int i = 0; i < TURN_ALL; i++)
    player->control->actions->options[i] = true;
  
  TraceLog(LOG_WARNING, "=== GAME INPUT ===\n RESET INPUT");
}

bool InputCheck(TurnPhase phase, int turn){
  if(IsKeyDown(KEY_SPACE))
    moncontrol(1);

  if(!player_input.active)
    return false;

  if(!player->control->actions->options[phase])
    return false;

  if(ActionMan.round[phase].status != ACT_STATUS_NONE)
    return false;
  for(int i = 0; i < ACTION_DONE; i++){
    action_key_t akey = player_input.actions[i];

    for (int j = 0; j< akey.num_keys; j++){
      KeyboardKey k = akey.keys[j];


      if(!IsKeyDown(k))
        continue;


      akey.fn(player, akey, k);
      TraceLog(LOG_INFO,"%i",WorldGetTime());
    }
  }
}
