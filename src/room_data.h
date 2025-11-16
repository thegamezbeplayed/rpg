#ifndef ROOM_DATA_H
#define ROOM_DATA_H

#define ROOM_WIDTH 774
#define ROOM_HEIGHT 774

#define ROOM_LEVEL_COUNT 3

static const ObjectInstance room_instances[ENT_DONE] = {
  {ENT_NONE},
  {ENT_PERSON,(Cell){12,20},{}},
  {ENT_GOBLIN,(Cell){22,22},{[STATE_IDLE]= BEHAVIOR_SEEK,[STATE_WANDER]= BEHAVIOR_WANDER}},
//  {ENT_TREE,(Cell){23,22},{}},
  {ENT_DONE}
};

static const TileInstance BASE_TILE = {};

typedef struct {
  BehaviorID           id;
  bool                 is_root;
  BehaviorTreeType     bt_type;
  behavior_tree_node_t *(*func)(behavior_params_t *);
  bool          param_overide;
  EntityState   state;
  int           num_children;
  BehaviorID   children[5];
} BehaviorData;

static const BehaviorData room_behaviors[ BEHAVIOR_COUNT] = {
  { BEHAVIOR_NONE},
  { BEHAVIOR_CHANGE_STATE ,false,BT_LEAF,LeafChangeState,false, STATE_IDLE,0,{}},
  { BEHAVIOR_GET_TARGET ,false,BT_LEAF,LeafAcquireTarget,false, STATE_IDLE,0,{}},
  { BEHAVIOR_GET_DEST,false,BT_LEAF,LeafAcquireDestination,false, STATE_NONE,0,{}},
  { BEHAVIOR_MOVE_TO_TARGET ,false,BT_LEAF,LeafMoveToTarget,false, STATE_IDLE,0,{}},
  { BEHAVIOR_MOVE_TO_DEST,false,BT_LEAF,LeafMoveToDestination,false, STATE_NONE,0,{}},
  {BEHAVIOR_CAN_ATTACK,false,BT_LEAF,LeafCanAttackTarget,false, STATE_NONE,0,{}},
  {BEHAVIOR_ATTACK,false,BT_LEAF,LeafAttackTarget,false, STATE_NONE,0,{}},
  { BEHAVIOR_MOVE,false,BT_SEQUENCE, NULL,false, STATE_NONE,2,{ BEHAVIOR_GET_DEST, BEHAVIOR_MOVE_TO_DEST}},
  { BEHAVIOR_CHECK_AGGRO,false,BT_SEQUENCE, NULL,true, STATE_IDLE,2,{ BEHAVIOR_GET_TARGET, BEHAVIOR_CHANGE_STATE}},
  { BEHAVIOR_ACQUIRE,false, BT_SEQUENCE, NULL,true, STATE_AGGRO,2,{ BEHAVIOR_GET_TARGET, BEHAVIOR_CHANGE_STATE}},
  {BEHAVIOR_TRY_ATTACK, false, BT_SEQUENCE,NULL,true, STATE_ATTACK,2,{BEHAVIOR_GET_TARGET,BEHAVIOR_CHANGE_STATE}},
  {BEHAVIOR_APPROACH , false,BT_SEQUENCE, NULL,false, STATE_NONE,2,{BEHAVIOR_CAN_ATTACK,BEHAVIOR_ATTACK}},
  { BEHAVIOR_WANDER,true,BT_SELECTOR, NULL,false, STATE_NONE,2,{ BEHAVIOR_CHECK_AGGRO, BEHAVIOR_MOVE}},
  { BEHAVIOR_SEEK, true,BT_SELECTOR,NULL,true,STATE_WANDER,2,{ BEHAVIOR_ACQUIRE,  BEHAVIOR_CHANGE_STATE}},
  { BEHAVIOR_ACTION,false,BT_SELECTOR,NULL,false,0,2,{ BEHAVIOR_TRY_ATTACK, BEHAVIOR_APPROACH}},
  //{ BEHAVIOR_NO_ACTION},
  //{ BEHAVIOR_MOB_AGGRO ,true,BT_SELECTOR,NULL,false, 0,2,{ BEHAVIOR_ACTION, BEHAVIOR_NO_ACTION}},
};
#endif
