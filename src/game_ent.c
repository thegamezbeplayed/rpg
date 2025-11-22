#include "game_types.h"
#include "game_tools.h"
#include "game_math.h"
#include "game_process.h"

MAKE_ADAPTER(StepState, ent_t*);

ent_t* InitEnt(ObjectInstance data,Cell pos){
  ent_t* e = malloc(sizeof(ent_t));
  *e = (ent_t){0};  // zero initialize if needed
  e->type = data.id;

  e->size = data.size;
  e->map = WorldGetMap();
  e->pos = pos;
  e->facing = CELL_EMPTY;
  e->sprite = InitSpriteByID(data.id,SHEET_ENT);
  e->sprite->owner = e;

  strcpy(e->name,data.name);
  e->events = InitEvents();

  e->control = InitController();

  EntCalcStats(e);
  e->stats[STAT_HEALTH]->on_stat_empty = EntKill;

  InitActions(e->actions);

  for(int i = 0; i < GEAR_DONE; i++){
    if(data.items[i]==GEAR_NONE)
      break;

    item_def_t* item = GetItemDefByID(i);
    EntAddItem(e, InitItem(item), true);
  }

  if(e->type != ENT_PERSON){
    e->sprite->color = DARKGREEN;
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
  else
    e->sprite->color = GOLD;

  SetState(e,STATE_SPAWN,NULL);
  return e;
}

env_t* InitEnv(EnvTile t,Cell pos){
  env_t* e = malloc(sizeof(ent_t));
  *e = (env_t){0};  // zero initialize if needed
  e->type = t;

  e->vpos =CellToVector2(pos,CELL_WIDTH);
  e->pos = pos;
  env_t* batch =WorldGetEnvById(e->type);
  if(!batch){
    e->sprite = InitSpriteByID(t,SHEET_ENV);
    e->sprite->color = GREEN;
  }
  else
    e->sprite = batch->sprite;
//  e->sprite->owner = e;
  //e->pos = pos;// = Vector2Add(Vector2Scale(e->sprite->slice->center,SPRITE_SCALE),pos);

  return e;
}

void EntCalcStats(ent_t* e){
  MobCategory cat = GetEntityCategory(e->type);

  SpeciesType species = GetEntitySpecies(e->type);

  category_stats_t base = CATEGORY_STATS[cat];
  species_stats_t racial = RACIALS[species];
  size_category_t size = MOB_SIZE[0];
  
  for (int i = 0; i < ATTR_DONE; i++){
    int val = base.attr[i] + racial.attr[i] + size.attr[e->size][i];
    e->attribs[i] = InitAttribute(i,val);
  }
  for (int i = 0; i < STAT_DONE;i++){
    int val = base.stats[i] + racial.stats[i] + size.stats[e->size][i];
      e->stats[i] = InitStatOnMax(i,val);
    
    e->stats[i]->owner = e;
    e->stats[i]->start(e->stats[i]);
  }


}

item_t* InitItem(item_def_t* def){
  item_t* item = malloc(sizeof(item_t));

  *item = (item_t){
    .def = def
  };

  if(item_funcs[def->category].cat != ITEM_NONE){
    if(item_funcs[def->category].on_equip)
      item->on_equip = item_funcs[def->category].on_equip;
  }

  return item;
}

item_pool_t* InitItemPool(void) {
    item_pool_t* ip = calloc(1, sizeof(item_pool_t));
    ip->size = 0;
    return ip;
}

item_def_t* DefineItem(ItemInstance data){
  item_def_t* item = malloc(sizeof(item_def_t));
  item->id = data.id;

  item->category = data.cat;
  for (int i = 0; i < STAT_DONE; i++){
    if(data.stats[i] == 0){
      item->stats[i] = 0;
      continue;
    }

    item->stats[i] = InitStat(i, 1, data.stats[i], data.stats[i]);
  }
  
  return item;
}

bool EntKill(ent_t* e){
  return SetState(e, STATE_DIE,NULL);
}

void EntInitOnce(ent_t* e){
  EntSync(e);

  EntPollInventory(e);
  //if(e->items[0]
  if(e->attack==NULL)
    e->attack = InitBasicAttack(e);
 

  for(int i = 0; i < STAT_DONE; i++){
    if(!e->stats[i])
      continue;

    StatMaxOut(e->stats[i]);
  }

  cooldown_t* spawner = InitCooldown(3,EVENT_SPAWN,StepState_Adapter,e);
  AddEvent(e->events, spawner);
}

void EntPollInventory(ent_t* e){
  for (int i = 0; i < CARRY_SIZE; i++){
    if(!e->gear[i])
      break;
    if(!e->gear[i]->equipped)
      continue;

    if(e->gear[i]->on_equip)
      e->gear[i]->on_equip(e,e->gear[i]);
  
  }
}

bool ItemApplyStats(struct ent_s* owner, item_t* item){
  for(int i = 0; i < STAT_DONE; i++){
    if(item->def->stats[i])
      StatExpand(owner->stats[i],item->def->stats[i]->current,true);
  }
}

item_t* EntGetItem(ent_t* e, ItemCategory cat, bool equipped){
  for (int i = 0; i < CARRY_SIZE; i++){
    if(!e->gear[i])
      break;

    if(e->gear[i]->equipped && e->gear[i]->def->category == cat)
      return e->gear[i];
  }

  return NULL;
}

bool EntAddItem(ent_t* e, item_t* item, bool equip){
  for(int i = 0; i < CARRY_SIZE; i++){
    if(e->gear[i])
      continue;

    item->equipped = equip;
    e->gear[i] = item;

    item->owner = e;

    return true;
  }

  return false;
}

void EntDestroy(ent_t* e){
  if(!e || !SetState(e, STATE_END,NULL))
    return;

  MapRemoveOccupant(e->map,e->pos);

  if(e->sprite!=NULL){
    e->sprite->owner = NULL;
    SpriteSetAnimState(e->sprite,ANIM_KILL);
  }

  e->control = NULL;
}

bool EntHit(ent_t* e, attack_t* a){

  int base_dmg = a->dc->roll(a->dc);

  int damage = -1 * (base_dmg + a->stats[STAT_DAMAGE]->current);

  if(StatChangeValue(e,e->stats[STAT_HEALTH], damage)){
   TraceLog(LOG_INFO,"%s hit health now %0.0f/%0.0f",e->name, e->stats[STAT_HEALTH]->current,e->stats[STAT_HEALTH]->max);

    return true;
  }

  return false;
}

bool EntAttack(ent_t* e, attack_t* a, ent_t* target){
  TraceLog(LOG_INFO,"%s swings at %s",e->name,target->name);

  int hit = a->hit->roll(a->hit);
  if (hit < target->stats[STAT_ARMOR]->current){

    TraceLog(LOG_INFO,"%s misses",e->name);
    return false;
  }

  a->stats[STAT_DAMAGE]->start(a->stats[STAT_DAMAGE]);

  StatMaxOut(a->stats[STAT_DAMAGE]);
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

bool ItemAddAttack(struct ent_s* owner, item_t* item){
  attack_t* a = malloc(sizeof(attack_t));

  const item_def_t* def = item->def;

  a->dc = Die(1,def->stats[STAT_DAMAGE]->current);
  a->hit = Die(20,1);

  memset(a->stats, 0, sizeof(a->stats));

  a->stats[STAT_REACH] = def->stats[STAT_REACH];
  a->stats[STAT_DAMAGE] = InitStatOnMax(STAT_DAMAGE, 0);

  for (int i = 0; i < STAT_DONE; i++){
    if(!a->stats[i])
      continue;
    a->stats[i]->owner = owner;
  }

  owner->attack = a;
}

attack_t* InitWeaponAttack(ent_t* owner, item_t* w){
  attack_t* a = malloc(sizeof(attack_t));

  const item_def_t* def = w->def;
  
  a->dc = Die(1,def->stats[STAT_DAMAGE]->current);
  a->hit = Die(20,1);
  
  memset(a->stats, 0, sizeof(a->stats));

  a->stats[STAT_REACH] = def->stats[STAT_REACH];
  a->stats[STAT_DAMAGE] = InitStatOnMax(STAT_DAMAGE, 0);

  for (int i = 0; i < STAT_DONE; i++){
    if(!a->stats[i])
      continue;
    a->stats[i]->owner = owner;
  }
  return a;
}

attack_t* InitBasicAttack(ent_t* owner){
  attack_t* a = malloc(sizeof(attack_t));

  memset(a->stats, 0, sizeof(a->stats));

  a->hit = Die(20,1);
  a->dc = Die(1,1);
  a->stats[STAT_REACH] = InitStat(STAT_REACH,1,1,1);
  a->stats[STAT_DAMAGE] = InitStatOnMax(STAT_DAMAGE,1);

  for (int i = 0; i < STAT_DONE; i++){
    if(!a->stats[i])
      continue;
    a->stats[i]->owner = owner;
  }

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
  TileStatus status = MapSetOccupant(e->map,e,newPos);

  if(status < TILE_ISSUES){

    e->pos = newPos;
    e->facing = CellInc(e->pos,step);
  }
  else
    e->facing = newPos;

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

SpeciesType GetEntitySpecies(EntityType t){
  if (t >= 0 && t < ENT_DONE)
        return RACE_MAP[t];
    return SPEC_HUMAN; // fallback default

}

