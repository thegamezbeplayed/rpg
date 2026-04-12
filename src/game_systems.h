#ifndef __GAME_SYS__
#define __GAME_SYS__

#include "game_ui.h"

//INTERACTIONS_T===>
typedef struct interaction_s interaction_t;
typedef uint64_t interaction_uid_i;
typedef interaction_uid_i (*InteractionCB)(interaction_t* self, void* ctx, param_t payload);

struct interaction_s {
  interaction_uid_i uid;
  EventType         event;
  InteractResult    result;
  void*             ctx;
  int               last_update_turn;
  cooldown_t*       timer;
  bool              refresh_on_update;
  InteractionCB     on_update, on_add;
};

typedef struct{
  Activity         kind;
  InteractResult   result;
  param_t          tokens[TOKE_ALL];
  size_t           str_len;
}activity_t;

typedef struct{
  int           count, cap;
  size_t        head;
  activity_t    entries[64];
}activity_tracker_t;

typedef struct{
  Activity        kind;
  Narrator        perspective;
  NarrativeTense  tense;
  const char*     fmt;
}activity_format_t;
extern activity_format_t ACT_LOG_FMT[ACT_ALL];

void InitActivities(int cap);
int ActivitiesAssignValues(element_value_t** fill, int pos);
element_value_t* ActivitiesFetch(element_value_t*, param_t);
char* ParseActivity(activity_t* act, char* buffer, size_t buf_size);

static inline interaction_uid_i InteractionMakeUID(EventType type, uint16_t context_id,
    uint64_t source_id,
    uint64_t target_id){
  uint64_t who = hash_combine_64(source_id, target_id);

  return hash_combine_64(who, context_id);
}

interaction_uid_i RegisterSkillEvent(interaction_t* self, void* ctx, param_t payload);
interaction_uid_i UpdateSkillEvent(interaction_t* self, void* ctx, param_t payload);
typedef uint64_t event_uid_i;
typedef uint64_t event_fuid_i;
static inline event_fuid_i EventMakeFlexID(uint64_t user_id, flex_id_t fid ){

  event_uid_i euid = -1;
  switch(fid.type){
    case DATA_INT:
      euid = hash_combine_64(user_id, fid.id);
      break;
    case DATA_UINT64:
      euid = hash_combine_64(user_id, fid.uid);
      break;
  }

  return euid;
}

typedef void (*EventCallback)(
    EventType  event,
    void*      event_data,
    void*      user_data
);

typedef struct {
    EventType      event;
    EventCallback  cb;
    uint64_t       uid;
    void*          user_data;
} event_sub_t;

typedef struct {
    event_sub_t* subs;
    int count, cap;
} event_bus_t;

typedef struct{
  EventType   type;
  EventStatus status;
  int         max, calls;
  void*       data;
  uint64_t    iuid;
}event_t;

event_bus_t* InitEventBus(int cap);
event_sub_t* EventSubscribe(event_bus_t* bus, EventType event, EventCallback cb, void* u_data);
void EventEmit(event_bus_t* bus, event_t*);
void EventRemove(event_bus_t* bus, uint64_t id);
static inline event_uid_i EventMakeUID(EventType type, uint64_t data_id){
  event_uid_i euid = hash_combine_64(type, data_id);

  return euid;
}
//EVENTS==>

int InitInteractions();
interaction_t* RegisterInteraction(uint64_t source, uint64_t target, EventType event, int duration, void* ctx, uint16_t ctx_id);
interaction_t* StartInteraction(uint64_t source, uint64_t target, EventType event,
    int duration, void* ctx, uint16_t ctx_id, param_t data, InteractionCB add, InteractionCB update, bool refresh);
int InteractionExists(interaction_uid_i uid);
interaction_t* GetInteractionByUID(interaction_uid_i uid);
bool AddInteraction(interaction_t* inter);
bool CanInteract(int source, int target);
int GetInteractions(int source);
void FreeInteraction(interaction_t* item);
void FreeInteractionByIndex(int i);
void FreeInteractions();
void InteractionStep();
//==INTERACTIONS_T==>
typedef struct{
  DamageType      type;
  int             dmg_init, dmg_final;
  InteractResult  result;
}damage_t;

typedef uint64_t combat_exchange_i;
typedef struct{
  InteractParams   type;
  param_t          ctx[IP_DONE];
}combat_context_t;

typedef struct{
  combat_exchange_i exid;
  combat_context_t  *cctx[IM_DONE];
  InteractResult    phase[COM_END];
  InteractResult    step[BAT_DONE];
  Activity          active;
  BattleStep        current;
  InteractResult    result;
}combat_t;

typedef struct{
  int           count, cap;
  interaction_t *entries;
  hash_map_t    map;
}combat_system_t;
static combat_exchange_i CombatMakeExID(ent_t* source, uint64_t tar, uint64_t w_id, uint64_t time){
  uint64_t h = hash_string_64(source->name);

  h = hash_combine_64(h, tar);
  h = hash_combine_64(h, w_id);
  h = hash_combine_64(h, time);

  return h;

}
void InitCombatSystem(int cap);
interaction_t* StartCombat(ent_t* agg, ent_t* tar, ability_t* a);
combat_context_t* CombatContext(ent_t*, InteractMember, ability_t*);
InteractResult CombatStep(interaction_t* i, InteractResult);
#endif
