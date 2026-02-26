#include <assert.h>
#include "game_process.h"
#include "game_helpers.h"
#include <limits.h>


static int APPLIED = 0;
static int ATTEMPTS = 0;
static int FIX_ATTEMPTS = 0;
static int HALL_SCORE = 55;
static int CAP = 0;
static int NUM_ANCHORS = 0;
static int NUM_MOBS = 0;
static sprite_t* mark_spr[16];
static int markers;
static int PAUSE = 1;
static node_option_pool_t NODE_OPTIONS;
static map_context_t world_map;
static int GEN_SCALE = 16;

static map_room_t* best_lair;
static map_room_t* best_challenge;
static map_room_t* best_room;
static map_room_t* highest_room;

static Color col[MAX_ANCHOR_NODES]={
  BLUE,
  PURPLE,
  GREEN,
  PINK,
  YELLOW,
  MAGENTA,
  BLACK,
  BROWN
};

Cell PLAYER_SPAWN(void){
  return world_map.player_start;
}

map_grid_t* InitMapGrid(void){
  map_grid_t* m = GameMalloc("InitMapGrid", sizeof(map_grid_t));

  *m = (map_grid_t){0};

  m->step_size = CELL_WIDTH;

  m->x = 0;
  m->y = 0;
  m->width = world_map.width;
  m->height = world_map.height;
  m->floor = DARKBROWN;
  GEN_SCALE = 4;
  //MapApplyContext(m);
  return m;
}

void MapCellTurnStep(map_cell_t* m){
  ent_t* occ = m->occupant;
  env_t* env = m->tile;

  if(m->tile && m->tile->has_resources == 0)
    EnvSetStatus(m->tile, ENV_STATUS_DEAD);

  uint64_t occid = occ?occ->gouid:0;
  uint64_t envid = env?env->gouid:0;

  uint64_t swap = 0;
  uint64_t present = occid | envid;

  uint64_t next[SATUR_MAX];
  for(int i = 0; i < SATUR_MAX; i++){
    uint64_t bits = m->props->scents[i];

    uint64_t still_present = bits & present;
    uint64_t missing       = bits & ~present;

    if (still_present) {
      int up = (i + 1 < SATUR_MAX) ? i + 1 : i;
      next[up] |= still_present;
    }

    // Decay: move down
    if (missing && i > 0)
      next[i - 1] |= missing;

  }

   uint64_t new_bits = present;
  for (int i = 0; i < SATUR_MAX; i++)
    new_bits &= ~m->props->scents[i];

  if (new_bits)
    next[0] |= new_bits;

  // Commit
  memcpy(m->props->scents, next, sizeof(next));

}

void MapTurnStep(map_grid_t* m){
  if(!m)
    return;

  for(int x = 0; x < m->width; x++)
    for(int y = 0; y < m->height; y++){
      map_cell_t* mc = &m->tiles[x][y];
      if(!mc)
        continue;
      if(!mc->in_ctx)
        continue;

      if(mc->vis == VIS_NEVER)
        continue;

      MapCellTurnStep(mc);
    }
}

void MapSync(map_grid_t* m){
  if(!m || !m->updates)
    return;

  for(int i = 0; i < m->num_changes; i++){
    m->changes[i]->updates = false;
  }

  m->num_changes = 0;
}

void MapCellRender(map_cell_t* mc){
  if(mc->vis > VIS_HAS){
    if(mc->tile)
      EnvRender(mc->tile);

    if(mc->vis == VIS_FULL && mc->occupant)
      EntRender(mc->occupant);
  }

  if(mc->vis < VIS_FULL){
    mask_t m = AssMan.masks[mc->vis];
    DrawScreenOverlay(m, mc->coords);
  } 

}

void MapRender(map_grid_t* m){
  for(int x = 0; x < m->width; x++){
    for(int y = 0; y < m->height; y++){
      map_cell_t* mc = &m->tiles[x][y];

      MapCellRender(mc);
    }
 }
}


bool MapContextSetTile(Cell c, RoomFlags f){
  world_map.tiles[c.x][c.y] = f;

  return true;

}

GenStatus MapGetStatus(void){
  return world_map.status;
}

bool InitMap(void){
  map_context_t* ctx =&world_map;
  memset(ctx, 0, sizeof(*ctx));
  ctx->map_rules = GameMalloc("InitMap", sizeof(map_gen_t));
  *ctx->map_rules = MAPS[DARK_FOREST];

  mark_spr[markers++] = InitSpriteByID(ENV_WALL_DUNGEON, SHEET_ENV);
  mark_spr[markers++] = InitSpriteByID(ENV_DOOR_DUNGEON, SHEET_ENV);
  mark_spr[markers++] = InitSpriteByID(ENV_FLOOR_DUNGEON, SHEET_ENV);
  mark_spr[markers++] = InitSpriteByID(ENV_BORDER_DUNGEON, SHEET_ENV);
  ctx->level = BoundsFromRec(Rect(99,99,-99,-99));
  ctx->player_start = CELL_UNSET;
  ctx->decor_density = 15;  
  bool gen = MapGenerate(ctx);
  
  if(gen){
    TraceLog(LOG_INFO,"===Generate Complete!====");
    ctx->status = GEN_DONE;
    /* char grid[ctx->height][ctx->width+1];
       MapToCharGrid(ctx, grid);
       SaveCharGrid( ctx->width, ctx->height, grid, "map.txt");
       */
  }
  
  return gen;
}

map_cell_t* MapRoomPlacement(map_room_t* r){
  if(!r)
    return NULL;

  map_grid_t* m = r->map;
  if(!m)
    return NULL;

  Rectangle bounds = RecFromBounds(&r->bounds);
  int area = bounds.width * bounds.height;
  bool running = false;
  choice_pool_t* picker = StartChoice(&r->placements, area,ChooseByWeight, &running);

  if(!running){
    for (int x = r->bounds.min.x; x < r->bounds.max.x; x++){
      for (int y = r->bounds.min.y; y < r->bounds.max.y; y++){
        map_cell_t* c = &m->tiles[x][y];
        if(c->status!= TILE_EMPTY)
          continue;

        int dist = 1 + cell_distance(r->center, c->coords);

        AddChoice(picker, x*1000+y, area/dist, c, DiscardChoice);
      }
    }
  }

  choice_t* sel = picker->choose(picker);
  if(!sel || sel->context == NULL)
    return NULL;

  return sel->context;
}

void MapRoomSpawn(map_grid_t* m, EntityType data, int room){
  map_room_t* r = m->rooms[room];
  if(!r)
    return;

  ent_t* e = InitEntByRace(MONSTER_MASH[data]);

  map_cell_t* c = MapRoomPlacement(r);
  e->pos = c->coords;
  if(RegisterEnt(e)){
    e->map = m;
    EntPrepare(e);
    SetState(e,STATE_SPAWN,NULL);
  }

}

void MapRoomRunSpawner(map_room_t* r, spawner_t* s){
  Rectangle bounds = RecFromBounds(&r->bounds);
  int area = bounds.width * bounds.height;
  choice_pool_t* picker = InitChoicePool(area,ChooseByWeight);


  for (int i = 0; i < s->num_mobs; i++){
    mob_t* mob = &s->pool[i];
    map_cell_t* c = MapRoomPlacement(r);

    Cell pos = c->coords;
    TraceLog(LOG_INFO,
        "===== LEVEL SPAWN ROOM  %i MOB COUNT %i=====\n", r->id, r->num_mobs);

    if(r->num_mobs < MOB_ROOM_MAX)
    r->mobs[r->num_mobs++] = InitMob(mob, pos);
    else
      DO_NOTHING();
  }

  for (int i = 0; i < r->num_mobs; i++){
    ent_t* e = r->mobs[i];
    if(!RegisterEnt(e))
      continue;
    e->map = r->map;
    EntPrepare(e);
    if(SetState(e,STATE_SPAWN,NULL))
      TraceLog(LOG_INFO,
          "===== LEVEL SPAWN ROOM  %i at (%i, %i) =====\n %i: %s",
          r->id, e->pos.x, e->pos.y, i, e->name);

  }
}

void OnTurnMapRoom(EventType event, void* data, void* user){
  map_room_t* r = user;

  if(r->respawn_factor == 0)
    return;

  int roll = RandRange(0, r->respawn_factor);

  if(roll <= r->map->num_mobs)
    return;

  DO_NOTHING();
}

void OnRoomMobEvent(EventType event, void* data, void* user){
  map_room_t* r = user;


  switch(event){
    case EVENT_ENT_DEATH:
      ent_t* e = data;
      int index = -1;
      for (int i = 0; i < r->num_mobs; i++)
      {
        if(e->gouid == r->mobs[i]->gouid){
          index = i;
          break;
        }
      }

      if(index == -1)
        return;

      if(index == r->num_mobs-1)
        r->mobs[index] = NULL;
      else
        r->mobs[index] = r->mobs[r->num_mobs-1];

      r->num_mobs--;
      r->map->num_mobs--;
      mob_define_t def = MONSTER_MASH[e->type];
      MobRule respawn = def.rules&MOB_RESPAWN_MASK;
      switch(respawn){
        case MOB_RESPAWN_LOW:
          r->respawn_factor += r->num_mobs;
          break;
        case MOB_RESPAWN_AVG:
          r->respawn_factor += 1.5 * r->num_mobs;
          break;
        case MOB_RESPAWN_HIGH:
          r->respawn_factor += 2 * r->num_mobs;
          break;
      }
      if(r->num_mobs > 0)
        return;

      WorldSubscribe(EVENT_TURN_END, OnTurnMapRoom, r);

      break;
    case EVENT_LEVEL_SPAWNER_READY:
      spawner_t* s = data;
      MapRoomRunSpawner(r, s);
      break;
  }

}



map_room_t* InitMapRoom(map_context_t* ctx, room_t* r){
  map_room_t *mr = GameCalloc("InitMapRoom", 1,sizeof(map_room_t));  

  mr->center = r->center;
  mr->bounds = r->bounds; 
  mr->purpose = r->flags & ROOM_PURPOSE_MASK;
  mr->flags = r->flags; 
  if(r->num_mobs > 0){
    qsort(r->mobs, r->num_mobs, sizeof(ent_t*), CompareEntByCR);
    for(int i = 0; i < r->num_mobs; i++){
      mr->mobs[mr->num_mobs++] = r->mobs[i];
      WorldTargetSubscribe(EVENT_ENT_DEATH, OnRoomMobEvent, mr, r->mobs[i]->gouid);
    }
  }
  else{
    uint64_t mask = ROOM_PURPOSE_LEVEL_EVENT_MASK;
    if((mr->purpose & mask)>0 && (mr->purpose & ROOM_PURPOSE_START) == 0){
      WorldEvent(EVENT_ROOM_READY, mr, mr->id);
    WorldTargetSubscribe(EVENT_LEVEL_SPAWNER_READY, OnRoomMobEvent, mr, mr->id);
  }
  }
  
  return mr;
}

void WorldMapLoaded(map_grid_t* m){
  for(int i = 0; i < world_map.num_rooms; i++){
    map_room_t* mr = InitMapRoom(&world_map,world_map.rooms[i]);
  
    mr->id = GameObjectMakeUID("MAP_ROOM", i, WorldGetTime());
    m->rooms[m->num_rooms++] = mr;
    mr->map = m;
    m->num_mobs += mr->num_mobs;
  }
}


bool MapUpdateBiome(map_context_t* ctx, int count, mob_define_t* e){
  ctx->mob_pool->filtered = 0;
  return true;
  MobRules theme = e->rules & MOB_THEME_MASK;
  SpeciesType spec = e->race;

  MobType mt  = MobTypeByFlags(theme, spec);

  if(ctx->mob_pool->total <= ctx->mob_pool->count){

  }
  ctx->eco->current[mt]+= count;
  ctx->eco->sum+=count;
  choice_t* c = ChoiceById(ctx->mob_pool, e->id);
  if(c && c->context){
    int update = (int)((c->score + 
        c->score * BiomeDemand(ctx->eco, mt))/2);


    int diff = c->score - update;
    ctx->mob_pool->total-=diff;
    c->score = update;

  }
  return ctx->eco->current[mt] < ctx->eco->desired[mt];
}

MapNodeResult MapBuildBiome(map_context_t* ctx, map_node_t *node){
  map_gen_t* gen = ctx->map_rules;
  ctx->eco = &BIOME[gen->biome];
  MobRules mob_rules = GetMobRulesByMask(ctx->map_rules->mobs.rules,MOB_LOC_MASK);
  mob_define_t mob_pool[MOB_MAP_MAX];
  int mobs = GetMobsByRules(mob_rules,mob_pool);

  bool running = false;
  ctx->mob_pool = StartChoice(&ctx->mob_pool, mobs, ChooseByWeightInBudget, &running);

  ctx->eco->total = gen->min_mobs;
  for (int i = 0; i < MT_DONE; i++){
    int count = imax(1, ctx->eco->ratios[i] * ctx->num_rooms);
    ctx->eco->desired[i] = count;
  }

  if(!running){
    for(int i = 0; i < mobs; i++){
      mob_define_t* e = &mob_pool[i];
      MobRules theme = e->rules & MOB_THEME_MASK;
      SpeciesType spec = e->race;

      MobType mt  = MobTypeByFlags(theme, spec);
      int desired = ctx->eco->desired[mt];
      int score =  mob_pool[i].weight[ctx->map_rules->id];
      for(int j = 0; j < desired; j++)
        AddPurchase(ctx->mob_pool, mob_pool[i].id, score, mob_pool[i].cost, &mob_pool[i], DiscardChoice);
    }
  }

  return MAP_NODE_SUCCESS;
}

MapNodeResult MapGraphNodes(map_context_t *ctx, map_node_t *node){
  TraceLog(LOG_INFO,"===Generate New Node====");
  bool success =  MapGenNodeScan(ctx,NULL)==MAP_NODE_SUCCESS;
  RefreshNodeOptions();
  if(success){
    APPLIED++;
    TraceLog(LOG_INFO, "====New Node added nodes count %i",APPLIED);
    return APPLIED>CAP?MAP_NODE_SUCCESS:MAP_NODE_RUNNING;
  }

  if( CAP > APPLIED )
    return MAP_NODE_RUNNING;
  TraceLog(LOG_WARNING,"======= MAP GEN FIND OPTIONS ATTEMPTS ====\n =====  %i =====",ATTEMPTS);
  if (ATTEMPTS > MAX_ATTEMPTS)
    return MAP_NODE_SUCCESS;
}

room_t* ConvertNodeToRoom(map_context_t* ctx, room_node_t* node){
  if(node->applied)
    return NULL;

  room_t* r = GameCalloc("ConvertNodeToRoom", 1,sizeof(room_t));

  r->flags = node->flags;
  r->bounds = node->bounds;
  r->center = node->center;
  r->ref = node;
  r->is_root = node->is_root;
  r->is_reachable = node->is_reachable;
  r->depth = node->depth;

  node->applied = true;

  node->col.a*=.33;

  for(int i = 0; i < node->max_children; i++){
    r->openings[i] = GameCalloc("ConvertNodeToRoom", 1,sizeof(room_opening_t));
    r->openings[i]->dir = node->children[i]->dir;
    r->openings[i]->pos = node->children[i]->pos;
    cell_bounds_t range = node->children[i]->range;

    r->openings[i]->range = range;//,r->openings[i]->dir);

    r->openings[i]->entrance = node->children[i]->enter;

    r->num_children++;
    room_t* to = NULL;
    if(node->children[i]->room)
     to = ConvertNodeToRoom(ctx,node->children[i]->room); 
  
    if(to)
      ctx->rooms[ctx->num_rooms++]= to;

    ctx->connections[ctx->num_conn] = GameCalloc("ConvertNodeToRoom", 1,sizeof(room_connection_t));

    room_connection_t* r_conn = ctx->connections[ctx->num_conn++];

    cell_bounds_t o = r->openings[i]->range;
    r_conn->grid_iid = IntGridIndex(o.center.x, o.center.y);
    r_conn->to = to;
    r_conn->from = r;
    r_conn->opening = r->openings[i];

  }
  return r;
}

MapNodeResult MapNodesToGrid(map_context_t *ctx, map_node_t *node){
  room_node_t* root = ctx->anchors[0];


  ctx->rooms[ctx->num_rooms++] = ConvertNodeToRoom(ctx,root);

  return MAP_NODE_SUCCESS;
}

void DrawNode(room_node_t* node, Color col){
  Rectangle out = RectScale(RecFromBounds(&node->bounds),GEN_SCALE);
  out.x*=GEN_SCALE;
  out.y *=GEN_SCALE;
  bool write = false;
  char* mark ="R";
  switch(GetRoomPurpose(node->flags)){
    case ROOM_PURPOSE_LAIR:
      write = true;
      mark = "L";
      break;
    case ROOM_PURPOSE_CHALLENGE:
      write = true;
      mark ="C";
      break;
    case ROOM_PURPOSE_CONNECT:
      write = true;
      mark = "H";
      break;
    case ROOM_PURPOSE_SECRET:
      write = true;
      mark = "S";
      break;
    case ROOM_PURPOSE_TREASURE:
      write = true;
      mark = "T";
      break;
  }

  DrawRectangleRec(out, col);
  if(!node->applied){
  if(node->is_root){
    Rectangle sect = RectScale(RecFromBounds(&node->section),GEN_SCALE);
    sect.x*=GEN_SCALE;
    sect.y*=GEN_SCALE;
    DrawRectangleLinesEx(sect,2,col);
  }
  if(node->entrance_index > -1){
    Rectangle d_pos = RectScale(RecFromBounds(&node->children[node->entrance_index]->range),GEN_SCALE);
    d_pos.x*=GEN_SCALE;
    d_pos.y*=GEN_SCALE;
    DrawRectangleLinesEx(d_pos,2,WHITE);
  }
  DrawRectangleLinesEx(out,1, BLACK);
  }
  /*if(write)
    DrawText(mark, node->center.x*GEN_SCALE -3,node->center.y*GEN_SCALE -3 , 14, BLACK);
*/
  for(int i = 0; i < node->max_children; i++){
    if(!node->children[i]->used)
      continue;
   
    if(node->children[i]->room)
      DrawNode(node->children[i]->room,col);
  }
}

void MapGenRender(void){
  Rectangle sect = RectScale(RecFromBounds(&world_map.level),GEN_SCALE);
  sect.x*=GEN_SCALE;
  sect.y*=GEN_SCALE;

  DrawRectangleLinesEx(sect,4,BLACK);
  if(world_map.status == GEN_DONE){
    for(int x = 0; x < world_map.width; x++){
      for(int y = 0; y < world_map.height; y++){
        if(world_map.tiles[x][y] <1)
          continue;

        int tile = GetTileByFlags(world_map.tiles[x][y] |world_map.map_rules->map_flag);
        Cell pos = CELL_NEW(x,y);
        Vector2 vpos = CellToVector2(pos, GEN_SCALE);

        int tag = -1;

        for(int i = 0; i < markers; i++){
          if (mark_spr[i]->tag == tile)
            tag = i;
        }

        if(tag > -1)
          DrawSpriteAtPos(mark_spr[tag],vpos);
      }
    }
  }
  
  for(int i = 0; i < NUM_ANCHORS; i++){
    if(world_map.anchors[i])
      DrawNode(world_map.anchors[i],world_map.anchors[i]->col);
  }

  for(int i = 0; i < world_map.num_conn; i++){
    room_connection_t* issue = world_map.connections[i];
    Rectangle problem = RectScale(RecFromBounds(&issue->opening->range),GEN_SCALE);
    problem.x*=GEN_SCALE;
    problem.y*=GEN_SCALE;

    float p_scale = 0.25;
    Color c = RED;
    switch(issue->status){
      case RS_WHERE_IS_THE_DOOR:
        c = RED;
        break;
      case RS_DOOR_INSIDE:
        c = ORANGE;
        break;
      case RS_NO_PATH:
        c = PINK;
        break;
      case RS_CARVE_DOOR:
        c = BLUE;
        break;
      case RS_PATH_FOUND:
        c = GREEN;
        break;
      case RS_GTG:
        continue;
        break;
      case RS_ISSUE:
        c = BLACK;
        break;
      default:
        if(issue->opening->sub ==NULL && !issue->opening->entrance)
          p_scale = 0.125;
        c = WHITE;
        break;
    }
    Vector2 arrow[3];
    MakeTriangleFromRect(problem,issue->opening->dir, arrow); 
    DrawTriangleLines(arrow[0],arrow[1],arrow[2],c);

//    DrawRectangleLinesEx(problem,GEN_SCALE*p_scale,c);

  }

}

map_node_t* MapBuildNodeRules(MapNodeType id){
  if(room_nodes[id].id !=id)
    return NULL;

  map_node_data_t data = room_nodes[id];

  if(data.type == MAP_NODE_LEAF)
    return data.fn(id);

  map_node_t **kids = GameMalloc("MapBuildNodeRules", sizeof(kids)*data.num_children);
  for(int i = 0; i < data.num_children; ++i)
    kids[i] = MapBuildNodeRules(data.children[i]);

  return MapCreateSequence( id, kids, data.num_children);
}

map_node_t* MapCreateLeafNode(MapNodeFn fn, MapNodeID id){  
  map_node_t* node = GameMalloc("MapCreateLeafNode",sizeof(map_node_t));
  node->run = fn;
  node->id = id;
  node->type = MAP_NODE_LEAF;

  return node;
}

map_node_t* MapCreateSequence( MapNodeID id, map_node_t **children, int count){  
  map_node_t* node = GameMalloc("MapCreateSequence", sizeof(map_node_t));
  node->num_children = count;
  node->children = children;
  node->run = MapNodeRunSequence;
  node->id = id;
  node->type = MAP_NODE_SEQUENCE;


  return node;
}

Cell MapApplyContext(map_grid_t* m){
  m->id = world_map.map_rules->id;
  m->tiles = GameMalloc("MapApplyContext", world_map.width * sizeof(map_cell_t*));
  m->width = world_map.width;
  m->height = world_map.height;
  Cell out = CELL_UNSET;

  for(int x = 0; x < world_map.width; x++){
    m->tiles[x] = GameCalloc("MapApplyContext", m->height, sizeof(map_cell_t));
    for(int y = 0; y < world_map.height; y++){
      map_cell_t* mc = RegisterMapCell(x,y);

      m->tiles[x][y] = *mc;
      env_t* e = MapSpawn(world_map.tiles[x][y],x,y);
      if(e)
        RegisterEnv(e);
    }
  }
  for(int r = 0; r < world_map.num_rooms; r++)
    RoomSpawnMob(m, world_map.rooms[r]);

  if(!cell_compare(world_map.player_start,CELL_UNSET))
    out = world_map.player_start;

  return out;
}

TileStatus MapChangeOccupant(map_grid_t* map,ent_t* e, Cell old, Cell c){
  Cell bounds = CELL_NEW(map->width,map->height);
  if(!cell_in_bounds(c,bounds))
    return TILE_OUT_OF_BOUNDS;

  if(map->tiles[c.x][c.y].occupant)
    return TILE_OCCUPIED;

  if(map->tiles[old.x][old.y].occupant)
    MapRemoveOccupant(map,old);

  return MapSetOccupant(map,e,c);
}

TileStatus MapSetTile(map_grid_t* m, env_t* e, Cell c){
  Cell bounds = CELL_NEW(m->width,m->height);
  if(!cell_in_bounds(c,bounds))
    return TILE_OUT_OF_BOUNDS;

  map_cell_t* mc = &m->tiles[c.x][c.y];

  mc->tile = e;
  e->map_cell = mc;
  if(TileHasFlag(e->type, TILEFLAG_SOLID))
    mc->status = TILE_COLLISION;
  else if(TileHasFlag(e->type, TILEFLAG_BORDER))
    mc->status = TILE_BORDER;
  else
    mc->status = TILE_EMPTY;

  mc->updates = true;
  mc->props->resources |= e->has_resources;
  mc->props->scents[0]|=e->gouid;
  if(m->num_changes < 128)
    m->changes[m->num_changes++] = &m->tiles[c.x][c.y];
  if(mc->status < TILE_REACHABLE)
    m->reachable++;
  return TILE_SUCCESS;
}

TileStatus MapSetOccupant(map_grid_t* m, ent_t* e, Cell c){
  Cell bounds = CELL_NEW(m->width,m->height);
  if(!cell_in_bounds(c,bounds))
    return TILE_OUT_OF_BOUNDS;

  map_cell_t* mc = &m->tiles[c.x][c.y];
  if(mc->status > TILE_ISSUES)
    return TILE_OCCUPIED;

  MapRemoveOccupant(m,e->pos);
  mc->occupant =e;
  mc->status = TILE_OCCUPIED;

  mc->props->resources |= EntGetScents(e);
  mc->props->scents[0]|=e->gouid;

  WorldDebugCell(c, GREEN);
  e->map = m;
  return TILE_SUCCESS;
}

map_cell_t* MapGetTile(map_grid_t* map,Cell tile){
  Cell bounds = CELL_NEW(map->width,map->height);

  if(!cell_in_bounds(tile,bounds))
    return NULL;

  return &map->tiles[tile.x][tile.y];

}

TileStatus MapTileAvailable(map_grid_t* m, Cell c){
  Cell bounds = CELL_NEW(m->width,m->height);

  if(!cell_in_bounds(c,bounds))
    return TILE_OUT_OF_BOUNDS;
  map_cell_t* mc = &m->tiles[c.x][c.y];

  return mc->status;
}

TileStatus MapRemoveOccupant(map_grid_t* m, Cell c){
  Cell bounds = CELL_NEW(m->width,m->height);
  if(!cell_in_bounds(c,bounds))
    return TILE_OUT_OF_BOUNDS;

  m->tiles[c.x][c.y].occupant = NULL;

  m->tiles[c.x][c.y].status = TILE_EMPTY;

  WorldDebugCell(c, YELLOW);
  return TILE_SUCCESS;
}

ent_t* MapGetOccupant(map_grid_t* m, Cell c, TileStatus* status){
  map_cell_t tile = m->tiles[c.x][c.y];

  *status = tile.status;

  return tile.occupant;
}

bool CheckPath(map_context_t* m, Cell from, Cell to, Cell *block)
{
    if (cell_compare(from, to))
        return true;

    int W = imin(m->width,  GRID_WIDTH);
    int H = imin(m->height, GRID_HEIGHT);

    // Init grid
    for (int y = 0; y < H; y++)
      for (int x = 0; x < W; x++)
        nodes[x][y] = (path_node_t){
            .x = x, .y = y,
            .gCost = 999999,
            .hCost = 0,
            .fCost = 999999,
            .open = false,
            .closed = false,
            .parentX = -1,
            .parentY = -1
        };

    int sx = from.x, sy = from.y;
    int tx = to.x,   ty = to.y;

    path_node_t *start = &nodes[sx][sy];
    path_node_t *goal  = &nodes[tx][ty];

    start->gCost = 0;
    start->hCost = Heuristic(sx, sy, tx, ty);
    start->fCost = start->hCost;
    start->open = true;

    while (true) {
        path_node_t *current = NULL;

        // Find lowest-fCost open node
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++) {
                path_node_t *n = &nodes[x][y];
                if (n->open && !n->closed) {
                    if (!current || n->fCost < current->fCost)
                        current = n;
                }
            }

        if (!current)
            return false; // no path exists

        if (current == goal)
            break; // reached target

        current->open = false;
        current->closed = true;

        static const int dirs[4][2] = {
            { 1, 0 }, {-1, 0},
            { 0, 1 }, { 0,-1}
        };

        for (int i = 0; i < 4; i++) {
            int nx = current->x + dirs[i][0];
            int ny = current->y + dirs[i][1];

            // BOUND CHECK
            if (nx < 0 || nx >= W || ny < 0 || ny >= H)
              continue;
            // BOUND CHECK FIRST
            
            if (nx < 0 || nx >= m->width || ny < 0 || ny >= m->height)
              continue;

            // THEN tile check
            if (TileFlagBlocksMovement(m->tiles[nx][ny]))
              continue;

            path_node_t *neighbor = &nodes[nx][ny];
            if (neighbor->closed)
                continue;

            int newCost = current->gCost + 1;

            if (!neighbor->open || newCost < neighbor->gCost) {
                neighbor->gCost = newCost;
                neighbor->hCost = Heuristic(nx, ny, tx, ty);
                neighbor->fCost = neighbor->gCost + neighbor->hCost;
                neighbor->parentX = current->x;
                neighbor->parentY = current->y;
                neighbor->open = true;
            }
        }
    }

    // Reconstruct — if at least one parent exists, path is valid
    path_node_t *n = goal;
    while (!(n->x == from.x && n->y == from.y)) {
        if (n->parentX < 0 || n->parentY < 0)
            return false; // broken path
        n = &nodes[n->parentX][n->parentY];
    }

    return true;
}

int RoomFindClosestOpening(map_context_t* ctx, room_t* r){

  Cell closest =  CELL_NEW(999,999);
  int nearest = 99999;
  int cindex = -1;
  for(int i = 0; i < ctx->num_conn; i++){
    room_connection_t* conn = ctx->connections[i];
    if(conn->from->ref->sector == r->ref->sector)
      continue;
    
    if(conn->to && conn->to!=r)
      continue;

    int clen = CELL_LEN(cell_dist(conn->opening->range.center, r->center));

    if(clen > nearest)
      continue;

    nearest = clen;
    closest = conn->opening->range.center;
    cindex = i;
  }

  return cindex;
}

void RoomSpawnMob(map_grid_t* m, room_t* r){
  Rectangle bounds = RecFromBounds(&r->bounds);
  int area = bounds.width * bounds.height;
  choice_pool_t* picker = InitChoicePool(area,ChooseByWeight);

  for (int x = r->bounds.min.x; x < r->bounds.max.x; x++){
    for (int y = r->bounds.min.y; y < r->bounds.max.y; y++){
      map_cell_t* c = &m->tiles[x][y];
      if(c->status!= TILE_EMPTY)
        continue;

      int dist = 1 + cell_distance(r->center, c->coords);

      AddChoice(picker, x*1000+y, area/dist, c, DiscardChoice);
    }
  }

  for(int i = 0; i < r->num_mobs; i++){
    choice_t* sel = picker->choose(picker);
    map_cell_t* c = sel->context;
    r->mobs[i]->pos = c->coords;
    if(RegisterEnt(r->mobs[i])){
      r->mobs[i]->map = m; 
      SetState(r->mobs[i],STATE_SPAWN,NULL);
    }
  }
}


env_t* MapSpawn(TileFlags flags, int x, int y){

  if(flags == TILEFLAG_EMPTY)
    return NULL;

  EnvTile t = GetTileByFlags(flags|world_map.map_rules->map_flag);

  Cell pos = {x,y};
 
  if(flags & TILEFLAG_BORDER)
    return NULL;

  uint32_t tflags = EnvTileFlags[t];
  uint32_t size = GetEnvSize(tflags);
  env_t* env = InitEnv(t,pos);
  if(size==0 && ((flags * TILEFLAG_BORDER) == 0))
    return env;
  
  for (int i = 0; i < RES_DONE; i++){
    define_resource_t* temp = GetResourceByCatFlags(BIT64(i), OBJ_ENV, tflags);
    resource_t *res = GameCalloc("MapSpawn", 1,sizeof(resource_t));
    env->resources[i] = res;
    if(!temp)
      continue;

    res->name = temp->name;
    res->type = temp->type;
    env->has_resources |= temp->type;
    uint64_t amnt = size*temp->quantity;


    *res = (resource_t){
      .type = temp->type,
        .amount = amnt,
        .attached = temp->attached
    };

    env->smell += temp->smell;
    env->resources[i]->amount+=amnt;
  }

  return env;
}

MapNodeResult MapNodeRunSequence(map_context_t *ctx, map_node_t *node){
  int i = 0;
  while (i < node->num_children) {
    MapNodeResult r = node->children[i]->run(ctx, node->children[i]);
    switch(r){
      case MAP_NODE_RUNNING:
        break;
      case MAP_NODE_FAILURE:
        return r;
        break;
      case MAP_NODE_SUCCESS:
        i++;
        break;
    }
  }
  return MAP_NODE_SUCCESS;
}

bool MapGenerate(map_context_t *ctx){
  map_node_t *root = MapBuildNodeRules(ctx->map_rules->node_rules);
  bool success = root->run(ctx, root) == MAP_NODE_SUCCESS;

  return success;
}

bool RoomCheckPaths(map_context_t *ctx, room_connection_t* conn){
  if(!conn->from || !conn->to){
    if(conn->opening->entrance || conn->opening->sub){
      conn->status = RS_ISSUE;
      return true;

    }
    else{
      conn->status = RS_GTG;
      return false;
    }
  }

  bool f_closed = room_is_enclosed(ctx, conn->from->bounds);
  bool t_closed = room_is_enclosed(ctx, conn->to->bounds);

  Cell d_pos = conn->opening->pos;
  if(t_closed || f_closed){
    conn->status = RS_WHERE_IS_THE_DOOR;
    return true;
  }

  Cell tblocker = CELL_UNSET;
  Cell fblocker = CELL_UNSET;
  bool t_path = CheckPath(ctx,conn->to->bounds.center,d_pos, &tblocker);
  bool f_path = CheckPath(ctx,conn->from->bounds.center,d_pos, &fblocker);


  if(f_path || t_path){
    if(f_path != t_path)
      DO_NOTHING();

    Rectangle f_rec = RecFromBounds(&conn->from->bounds);
    Rectangle t_rec = RecFromBounds(&conn->to->bounds);
    bool f_door_inside = cell_in_rect(d_pos, f_rec);
    bool t_door_inside = cell_in_rect(d_pos, t_rec);
    if(f_door_inside || t_door_inside){
      conn->status = RS_DOOR_INSIDE;
      return true;
    }

    conn->status = RS_GTG; 
    return false;
  }
  else{
    conn->status = RS_NO_PATH;
    return true;
  }
}

bool RoomFindDoor(map_context_t *ctx, room_connection_t* conn){
  bool f_closed = room_is_enclosed(ctx, conn->from->bounds);
  bool t_closed = room_is_enclosed(ctx, conn->to->bounds);

  cell_bounds_t d_pos = conn->opening->range;
  if(!t_closed  && !f_closed){
    conn->status = RS_CHECK;
    return false;
  }

  Rectangle f_rec = RecFromBounds(&conn->from->bounds);
  Rectangle t_rec = RecFromBounds(&conn->to->bounds);
  bool f_door_inside = cell_in_rect(d_pos.center, f_rec);
  bool t_door_inside = cell_in_rect(d_pos.center, t_rec);

  if(f_door_inside || t_door_inside){
    conn->status = RS_DOOR_INSIDE;
    return true;
  }

  Cell f_center = conn->from->bounds.center;
  Cell t_center = conn->to->bounds.center;
  Rectangle d_rec = RecFromBounds(&conn->opening->range);
  bool rooms_align = cells_linear(f_center,t_center);
  bool door_from = cells_linear(f_center,conn->opening->range.center);
  bool door_to = cells_linear(t_center,conn->opening->range.center);

  if(rooms_align && door_from && door_to){
    conn->status = RS_RESOLVED;
    return false;
  }

  return true;
}

bool RoomFindPath(map_context_t *ctx, room_connection_t* conn){
  cell_bounds_t d_pos = conn->opening->range;
  Cell tblocker = CELL_UNSET;
  Cell fblocker = CELL_UNSET;
  bool t_path = CheckPath(ctx,conn->to->bounds.center,d_pos.center, &tblocker);
  bool f_path = CheckPath(ctx,conn->from->bounds.center,d_pos.center, &fblocker);

  if(f_path && t_path){
    conn->status = RS_GTG;
    return false;
  }

  return true;
}

bool RoomReplaceDoor(map_context_t *ctx, room_connection_t* conn){
  room_opening_t *best_to, *best_from;

  if (FindBestOpeningPair(conn->to, conn->from, &best_to, &best_from)) {
    conn->in = best_to;
    conn->out = best_from;
    conn->status = RS_CARVE_DOOR;
    return false;
  }
 
  bool f_closed = room_is_enclosed(ctx, conn->from->bounds);
  bool t_closed = room_is_enclosed(ctx, conn->to->bounds);

  Cell d_pos = conn->opening->pos;
    
  Cell blocker = CELL_UNSET;
  bool path = CheckPath(ctx,conn->to->bounds.center,conn->from->bounds.center, &blocker);

  if(path){
    conn->status = RS_PATH_FOUND;
    return false;
  }

  Rectangle f_rec = RecFromBounds(&conn->from->bounds);
  Rectangle t_rec = RecFromBounds(&conn->to->bounds);
  bool f_door_inside = cell_in_rect(d_pos, f_rec);
  bool t_door_inside = cell_in_rect(d_pos, t_rec);

 if(f_door_inside || t_door_inside){
    conn->status = RS_DOOR_INSIDE;
    return true;
  }

  Cell f_center = conn->from->bounds.center;
  Cell t_center = conn->to->bounds.center;
  Rectangle d_rec = RecFromBounds(&conn->opening->range);
  bool rooms_align = cells_linear(f_center,t_center);
  bool door_from = cells_linear(f_center,conn->opening->range.center);
  bool door_to = cells_linear(t_center,conn->opening->range.center);

  if(rooms_align && door_from && door_to){
    conn->status = RS_RESOLVED;
    return false;
  }

  return true;
}

bool RoomMakePath(map_context_t *ctx, room_connection_t* conn){
  Cell f_center = conn->from->bounds.center;
  Cell t_center = conn->to->bounds.center;
  Rectangle d_rec = RecFromBounds(&conn->opening->range);
  bool rooms_align = cells_linear(f_center,t_center);
  bool door_from = cells_linear(f_center,conn->opening->range.center);
  bool door_to = cells_linear(t_center,conn->opening->range.center);

  if(rooms_align && door_from && door_to){
    conn->status = RS_RESOLVED;
    return false;
  }

  return true;
}

bool RoomOpeningCheckPairs(map_context_t *ctx, room_connection_t* conn){

  room_connection_t* results[8];
  int count = ConnectionsByIID(conn,results);

  //TODO REMOVE
  return true;

  for(int i = 0; i < count; i++){
    if(results[i]->status > RS_NO_ISSUE){
      conn->status = RS_CORRECTED;
      return false;
    }
  }
  count = ConnectionGetNeighbors(conn,results, 3, 8);

  for(int i = 0; i < count; i++){
    if(!cells_linear(conn->opening->dir,results[i]->opening->dir))
      continue;

    if(cell_compare(conn->opening->dir,results[i]->opening->dir))
      continue;

    if(results[i]->status > RS_NO_ISSUE){
      conn->status = RS_CORRECTED;
      return false;
    }

  }

  return true;
}

bool RoomCarveDoor(map_context_t *ctx, room_connection_t* conn){

  Cell in_pos = conn->opening->pos;
  Cell out_pos = conn->opening->pos;
  
  if(conn->in)
    in_pos = conn->in->pos;
  if(conn->out)
    out_pos = conn->out->pos;

  bool status = false;
  if(TileCellBlocksMovement(ctx, in_pos))
    status = MapContextSetTile(in_pos, ctx->map_rules->opening_flag);
  else if(TileCellHasFlag(ctx,in_pos, ctx->map_rules->opening_flag))
    status = true;

  if(!cell_compare(in_pos,out_pos)){
    RoomFlags out_flag = status?TILEFLAG_EMPTY:ctx->map_rules->opening_flag;
    if(TileCellBlocksMovement(ctx, out_pos))
      status = status || MapContextSetTile(out_pos, out_flag);
    else if(TileCellHasFlag(ctx,out_pos, ctx->map_rules->opening_flag))
      status = true;
  }
  
  if(!status){
    Cell access;
    if(room_has_access(ctx, conn->from->bounds,&access))
      status = true;
  }

  if(status){
    if(conn->status == RS_CORRECTED)
      conn->status = RS_CARVED;
    else
      conn->status = RS_CORRECTED;
  }
  else
    conn->status = RS_CHECK;

  return status;
}

MapNodeResult MapApplyFixes(map_context_t *ctx, map_node_t *node){
 MapNodeResult result = MAP_NODE_SUCCESS;
 bool issue = false; 
 for (int i = 0; i < ctx->num_conn; i++){
    room_connection_t* conn = ctx->connections[i];
    if(conn->status > RS_FIXED){
      conn->status = RS_GTG;
      continue;
    }
    usleep(1*PAUSE);

    issue = true;
    switch(conn->status){
      case RS_CARVE_DOOR:
      case RS_ISSUE:
      case RS_CORRECTED:
      case RS_NO_PATH:
        issue != RoomCarveDoor(ctx,conn);
        break;
      case RS_DOOR_INSIDE:
        issue = RoomOpeningCheckPairs(ctx,conn);
        if(issue)
        issue != RoomCarveDoor(ctx,conn);
        break;
      default:
        if(!RoomCheckPaths(ctx,conn)){
          conn->status = RS_GTG;
          issue = false;
        }
        break;
    }
    if(issue)
      FIX_ATTEMPTS++;
  }
  if(issue && FIX_ATTEMPTS < MAX_ATTEMPTS)
    result = MAP_NODE_RUNNING;
  
  return result;
}

MapNodeResult MapFixIssues(map_context_t *ctx, map_node_t *node){
  MapNodeResult result = MAP_NODE_SUCCESS;
  for (int i = 0; i < ctx->num_conn; i++){
    room_connection_t* conn = ctx->connections[i];
    if(conn->status > RS_NO_ISSUE)
     continue; 
    usleep(1*PAUSE);
    bool issue = RoomOpeningCheckPairs(ctx,conn);
    if(!issue)
      continue;

    switch(conn->status){
      case RS_CHECK:
        issue = RoomCheckPaths(ctx, conn);
        break;
      case RS_WHERE_IS_THE_DOOR:
        issue = RoomFindDoor(ctx,conn);
        break;
      case RS_NO_PATH:
        issue = RoomMakePath(ctx,conn);
        break;
      case RS_DOOR_INSIDE:
        issue = RoomReplaceDoor(ctx,conn);
        break;
      default:
        DO_NOTHING();
        break;
    }
    if(issue){
      FIX_ATTEMPTS++;
      result = MAP_NODE_RUNNING;
    }
  }

  if(FIX_ATTEMPTS > MAX_ATTEMPTS){
    result = MAP_NODE_SUCCESS;
    FIX_ATTEMPTS = 0;
  }
  return result;//MAP_NODE_SUCCESS;
}

MapNodeResult MapCheckPaths(map_context_t *ctx, map_node_t *node){
  MapNodeResult result = MAP_NODE_SUCCESS;
  for (int i = 0; i < ctx->num_conn; i++){
    room_connection_t* conn = ctx->connections[i];
    if(conn->status > RS_NO_ISSUE)
      continue;

    RoomCheckPaths(ctx, conn);
  } 
  return MAP_NODE_SUCCESS;
}

void AlignNodeConnections(map_context_t *ctx, room_node_t* rn){
  for(int i = 0; i < rn->max_children; i++){
    if(!rn->children[i]->used || rn->children[i]->room == NULL)
      continue;
    node_connector_t* conn = rn->children[i];
    cell_bounds_t o = conn->range;
    Rectangle r_bounds = RecFromBounds(&rn->bounds);

    Cell gap = cell_along_rect(o.center, r_bounds);
    room_node_t* child = rn->children[i]->room;
    Rectangle c_bounds = RecFromBounds(&child->bounds);
  }
}

MapNodeResult MapAlignNodes(map_context_t *ctx, map_node_t *node){
  for(int i = 0; i < NUM_ANCHORS; i++){
    room_node_t* r = ctx->anchors[i];

    AlignNodeConnections(ctx, r);
  }

  return MAP_NODE_SUCCESS;
}

node_option_t RoomConnectOptions(node_connector_t* conn, room_node_t* dest){
  node_option_t out = {0};

  RoomFlags flags = GetRoomPlacing(dest->flags) | RandomOrient() |
    ROOM_LAYOUT_HALL | ROOM_PURPOSE_NONE | SizeByWeight(ROOM_SIZE_MAX,RandRange(20,100));

  cell_bounds_t bounds = RoomBounds(flags,CELL_EMPTY);
  int h_area = (bounds.max.x - bounds.min.x) * (bounds.max.y - bounds.min.y);
  out.score = CELL_LEN(dest->size)-h_area;

  Cell dest_dir = RoomFlagsGetPlacing(flags);

  Cell orient = GetRoomOrient(flags)== ROOM_ORIENT_HOR?CELL_RIGHT: CELL_DOWN;

  int dist = CELL_LEN(cell_dist(conn->range.center,dest->center));
  if(cell_compare(dest_dir, conn->dir))
    out.score+=50;
  else
    out.score-=dist;

  if(cells_linear(conn->dir,orient))
    out.score+=10;

  out.flags = flags;
  out.connect = conn;
  return out;
}

bool ConnectAnchorOrphan(room_node_t* anchor, room_node_t* orphan){
  for(int i = 0; i < anchor->max_children; i++){
    if(anchor->children[i]->used)
      continue;

    node_option_t opt = RoomConnectOptions(anchor->children[i],orphan);
    if(opt.score > 0){
      MapAddNodeOption(opt);
    }
  }

  node_option_t* best = PickBestNodeOption();

  if(best)
    return MapAddNodeToConnector(anchor->children[best->connect->index],orphan);

  return false;
}

bool MapOptionsConnectAnchors(map_context_t *ctx, room_node_t* root, room_node_t* dest){
  while(NODE_OPTIONS.count < root->max_children*1.5){
    for(int i = 0; i < root->max_children; i++){
      if(root->children[i]->used)
        continue;

      node_connector_t *conn = root->children[i];
      //conn.room = dest;
      node_option_t opt = RoomConnectOptions(conn, dest);

      opt.type = OPT_CONN;

      if(opt.score > 0){
        opt.context = root;
        MapAddNodeOption(opt);
      }
    }
  }
  
  node_option_t* best = PickBestNodeOption();
  if(!best || !ApplyNode(best)){
    if(best)
      best->score-=100;
    return MapOptionsConnectAnchors(ctx, root, dest);
  }
  else{
    RefreshNodeOptions();
    if(root->children[best->connect->index]->room)
      ConnectAnchorOrphan(root->children[best->connect->index]->room, dest);
  }

  return true;
}

bool MapAnchorLayoutOptions(map_context_t* ctx, room_node_t* root){
  int num_orphans = root->num_orphans;
  bool recursive_result = true;

  for(int i = 0; i < root->num_orphans; i++){
    if(!MapOptionsConnectAnchors(ctx, root, root->orphans[i]))
      continue;

    num_orphans--;
    if(root->orphans[i]->num_orphans > 0)
      if(!MapAnchorLayoutOptions(ctx, root->orphans[i]))
        recursive_result = false;
      else
        root->orphans[i] = NULL;
  }

  root->num_orphans = num_orphans;

  return recursive_result && num_orphans == 0;
}


MapNodeResult MapGridLayout(map_context_t *ctx, map_node_t *node){

  if(NUM_ANCHORS==0)
    return MAP_NODE_FAILURE;

  bool result = MapAnchorLayoutOptions(ctx,ctx->anchors[0]);

  if(result)
    return MAP_NODE_SUCCESS;
  
  return MAP_NODE_FAILURE;
}

bool AdjustChildFromConn(room_node_t* root, room_node_t* child, node_connector_t* conn, int c_index){
  node_connector_t* c_conn = child->children[c_index];
  Cell disp = CellInc(c_conn->dir,CellSub(conn->pos, c_conn->pos));

  Cell og_center = child->center;
  child->center = CellInc(og_center,disp);

  cell_bounds_t og_bounds = child->bounds;
  child->bounds = RoomBounds(child->flags,child->center);

  Vector2 overlap;
  Rectangle p_rec = RecFromBounds(&root->bounds);
  if(GetRectOverlap(RecFromBounds(&child->bounds),p_rec,&overlap)){
    while(GetRectOverlap(RecFromBounds(&child->bounds),p_rec,&overlap)){
      Cell shift = CellMul(conn->dir,overlap);
      child->center = CellInc(child->center,shift);
      child->bounds = RoomBounds(child->flags,child->center);
    }
  }
  else{

    if(overlap.x < 0 || overlap.y < 0)
      DO_NOTHING();
  }
  if(MapCheckOverlap(RecFromBounds(&child->bounds),&overlap)){
    child->bounds = og_bounds;
    child->center = og_center;
    return false;
  }

  disp = CellSub(child->center,og_center);
  for(int i = 0; i < child->max_children; i++){
    child->children[i]->range = ShiftCellBounds(child->children[i]->range,disp);
    child->children[i]->pos = child->children[i]->range.center;

  }

  return true;
}

bool AdjustChildNodePosition(room_node_t* root, room_node_t* child, node_connector_t* conn, Cell dir){

  Cell req_dir = CellMul(CELL_UNSET,conn->dir);
   for(int j = 0; j < child->max_children; j++){
   // child->children[j]->range =  AdjustCellBounds(&child->children[j]->range,trans);
  //  child->children[j]->pos = CellInc(child->children[j]->pos,trans);

    if(child->children[j]->room)
      continue;

    if(!cell_compare(req_dir,child->children[j]->dir))
      continue;
    
    if(!AdjustChildFromConn(root,child,conn,j))
     continue; 
    
    child->entrance_index = j; 
    child->children[j]->used = true;
    child->children[j]->enter = true;
    break;
  }

   if(child->entrance_index < 0)
     return false;

  world_map.room_bounds[world_map.num_bounds++] = RecFromBounds(&child->bounds);

  return true;
}

int GetRootNode(node_connector_t* conn, room_node_t* out, int* count){
  (*count)++;
  if(conn->owner->is_root && conn->owner->anchor)
    return GetRootNode(conn->owner->anchor, out, count);
  else
    return *count;
}

void CalcLevelSize(room_node_t* root){
  if(root->section.min.x < world_map.level.min.x)
    world_map.level.min.x = root->section.min.x;
  if(root->section.min.y < world_map.level.min.y)
    world_map.level.min.y = root->section.min.y;

  if(root->section.max.x > world_map.level.max.x)
    world_map.level.max.x = root->section.max.x;
  if(root->section.max.y > world_map.level.max.y)
    world_map.level.max.y = root->section.max.y;

}

void CalcSectionSize(room_node_t* root, room_node_t* new){
  if(new->bounds.min.x < root->section.min.x)
    root->section.min.x = new->bounds.min.x;
  if(new->bounds.min.y < root->section.min.y)
    root->section.min.y = new->bounds.min.y;

  if(new->bounds.max.x > root->section.max.x)
    root->section.max.x = new->bounds.max.x;
  if(new->bounds.max.y > root->section.max.y)
    root->section.max.y = new->bounds.max.y;

  CalcLevelSize(root);
}

void NodeConnectorIncreaseDepth(node_connector_t* conn, int amnt){
  conn->depth+=amnt;
  if(conn->owner && conn->owner->anchor)
      NodeConnectorIncreaseDepth(conn->owner->anchor, amnt);
}

void RootNodeIncreaseTotal(room_node_t* node, int amnt, room_node_t* new){
  if(!node->is_root && node->anchor->owner)
    RootNodeIncreaseTotal(node->anchor->owner, amnt, new);
  else{
    CalcSectionSize(node,new);
    node->total+=amnt;
  }
}

bool RoomCheckNeighborSections(room_node_t* node, node_connector_t* conn){
  Rectangle r_rec = RecFromBounds(&world_map.anchors[node->sector]->section);

  for(int i = 0; i < NUM_ANCHORS; i++){
    if(i == node->sector)
      continue;

    Rectangle n_rec = RecFromBounds(&world_map.anchors[i]->section);
    Vector2 v_pen = VEC_UNSET;
    bool overlap = GetRectOverlap(n_rec, r_rec, &v_pen);
    if(!overlap)
      continue;

    node_option_t opt = {
      .type = OPT_CONN,
      .connect = conn,
      .context = node,
    };
    NODE_OPTIONS.items[NODE_OPTIONS.count++] = opt;
  } 
}

bool MapAddNodeToConnector(node_connector_t* conn, room_node_t* node){

  if(conn->room || conn->used || !conn->owner)
    return false;

  if(!AdjustChildNodePosition(conn->owner, node, conn, conn->dir))
    return false;

  node->sector = conn->owner->sector;
  conn->used = true;
  node->anchor = conn;
  conn->room = node;
  conn->depth++;
  conn->owner->num_children++; 
  node->depth = GetRootNode(conn,NULL, &node->depth);
  if(conn->owner && conn->owner->anchor)
    NodeConnectorIncreaseDepth(conn->owner->anchor,1);

  RootNodeIncreaseTotal(conn->owner,1, node);
  return true;
}

bool MapAddNode(room_node_t* root, room_node_t* child){

  for(int i = 0; i < root->max_children; i++){
    if(root->children[i]->used)
      continue;

    Cell dir = RoomFacingFromFlag(child->flags);

    if(!cell_compare(dir,root->children[i]->dir))
      continue;
   
    node_connector_t* conn = root->children[i]; 

    if(!AdjustChildNodePosition(root, child, conn,CELL_EMPTY))//conn->dir))
      continue;


    child->anchor = root->children[i];
    root->children[i]->depth++;
    root->children[i]->used = true; 
    root->children[i]->room = child;
    root->num_children++;

    return true;
  }
  return false;
}

bool MapAddSubNode(room_node_t* root, room_node_t* child){
  for(int i = 0; i < root->max_children; i++){
    if(!root->children[i]->used)
      continue;
   
    if(root->children[i]->depth >= MAX_NODE_DEPTH)
     continue;

    if(MapAddNode(root->children[i]->room, child)){
      root->children[i]->depth++;
      return true;
    }
  }

  return false;
}

room_node_t* MapBuildNode(RoomFlags flags, Cell pos){
  room_node_t* node = GameCalloc("MapBuildNode", 1,sizeof(room_node_t));

  *node = (room_node_t){
    .center = pos,
    .flags = flags,
    .children = GameMalloc("MapBuildNode", sizeof(room_node_t)),
    .entrance_index = -1
  };

  if(GetRoomPurpose(flags) == ROOM_PURPOSE_CONNECT)
    node->is_root = true;

  node->bounds = RoomBounds(flags,pos);

  node->size = CellSub(node->bounds.max,node->bounds.min); 
  node->max_children = RoomConnectionsFromFlags(node, node->children); 
  node->cap = node->max_children - 3;

  return node;
}

int GetRootDepth(node_connector_t* conn){
  if(conn->owner && conn->owner->anchor){
    if(conn->owner->anchor->depth > conn->depth)
      return GetRootDepth(conn->owner->anchor);
  }
      
  return conn->depth;
}

node_option_t TryNodeOption(room_node_t* root, node_connector_t* conn, RoomFlags pflags, RoomFlags test){
  node_option_t opt = {0};

  opt.connect = conn;

  opt.score = 75;

  int depth = GetRootDepth(conn);
  if(root->is_root)
    opt.score += 50;
  else if(!root->is_root && depth >= MAX_NODE_DEPTH)
    opt.score -= depth*root->cap;
  else
    opt.score -= depth*root->total;
  
  RoomFlags purpose = test & ROOM_PURPOSE_MASK;
  RoomFlags size = test & ROOM_SIZE_MASK;
  RoomFlags layout = test & ROOM_LAYOUT_MASK;
  RoomFlags orient = test & ROOM_ORIENT_MASK;
  RoomFlags shape = test & ROOM_SHAPE_MASK;

  if(layout == ROOM_LAYOUT_HALL){
    if(conn->dir.y != 0)
      orient = ROOM_ORIENT_VER;
    else
      orient = ROOM_ORIENT_HOR;
  }

  opt.flags = purpose | size | layout | orient | shape;

  int p_area = root->size.x * root->size.y;
  switch(GetRoomLayout(pflags)){
    case ROOM_LAYOUT_SUB:
      opt.score -= conn->depth*5 + 10;
      break;
    case ROOM_LAYOUT_HALL:
      if(layout != ROOM_LAYOUT_HALL)
        opt.score += ((1+root->num_children) * 5); 
      else
        opt.score -= (1+depth+root->num_children)*4;
      break;
    case ROOM_LAYOUT_OPEN:
      int lscore = 5;
      switch(layout){
        case ROOM_LAYOUT_SUB:
          lscore+=10;
          break;
        case ROOM_LAYOUT_HALL:
          lscore+=15;
          break;
        case ROOM_LAYOUT_OPEN:
          lscore -=25;
          break;
      }
      opt.score += lscore; 
        break;
    case ROOM_LAYOUT_ROOM:
      break;

  }
  switch(GetRoomPurpose(pflags)){
    case ROOM_PURPOSE_SIMPLE:
      opt.score -= p_area;
      switch(size){
        case ROOM_SIZE_MAX:
          opt.score -=20;
          break;
        case ROOM_SIZE_MASSIVE:
          opt.score -=15;
        case ROOM_SIZE_HUGE:
        case ROOM_SIZE_XL:
          opt.score -=10;
          break;
        case ROOM_SIZE_LARGE:
          opt.score -=5;
          break;
        default:
          opt.score -=2;
      }
     break;
    case ROOM_PURPOSE_START:
    case ROOM_PURPOSE_SECRET:
     opt.score = 0;
     break;
    case ROOM_PURPOSE_TREASURE:
    case ROOM_PURPOSE_CHALLENGE:
     opt.score -=25;
     break;
    case ROOM_PURPOSE_TRAPPED:
    case ROOM_PURPOSE_LAIR:
     opt.score -= 15;
     break;
    case ROOM_PURPOSE_CONNECT:
     opt.score += imax(20,50 - (root->num_children * 5));
     break;
  }

  opt.score += imax(50,p_area);

  switch(GetRoomShape(pflags)){
    case ROOM_SHAPE_SQUARE:
    case ROOM_SHAPE_RECT:
      opt.score+=20;
      break;
    case ROOM_SHAPE_FORKED:
    case ROOM_SHAPE_CROSS:
      opt.score+=15;
      break;
    case ROOM_SHAPE_CIRCLE:
    case ROOM_SHAPE_ANGLED:
      opt.score-=5;
      break;
  }

  switch(shape){
    case ROOM_SHAPE_SQUARE:
    case ROOM_SHAPE_RECT:
      opt.score+=15;
      break;
    case ROOM_SHAPE_FORKED:
    case ROOM_SHAPE_CROSS:
      opt.score+=10;
      break;
    case ROOM_SHAPE_CIRCLE:
    case ROOM_SHAPE_ANGLED:
      opt.score-=5;
      break;
  }
  
  switch(purpose){
    case ROOM_PURPOSE_START:
    opt.score = 0;
    break;
    case ROOM_PURPOSE_SECRET:
    case ROOM_PURPOSE_TREASURE:
    if(GetRoomPurpose(pflags) > ROOM_PURPOSE_TREASURE)
      opt.score += depth + p_area;
    else
      opt.score -= root->num_children;
    break;
    case ROOM_PURPOSE_CHALLENGE:
    case ROOM_PURPOSE_TRAPPED:
    opt.score+= depth*5;
    if(layout == ROOM_LAYOUT_HALL)
      opt.score = 0;
    break;
    case ROOM_PURPOSE_LAIR:
    if(conn->depth > 5)
     opt.score+= depth*4;
    else
     opt.score-=10;
     if(layout == ROOM_LAYOUT_HALL)
      opt.score = 0;
    break;
    case ROOM_PURPOSE_CONNECT:
    if(GetRoomPurpose(pflags)==ROOM_PURPOSE_CONNECT)
      opt.score-=25;
    else{
      if(layout == ROOM_LAYOUT_HALL)
        opt.score+=20-(depth + root->num_children);
      else
        opt.score+=5 - depth;
    }
    break;
    default:
    opt.score-= depth + root->num_children; 
    break;
  }
 
  switch(layout){
    case ROOM_LAYOUT_HALL:
      if(root->is_root && root->num_children < 3)
        opt.score += 44/(root->num_children+1);
      else
        opt.score -= (2 + depth + root->num_children) * 5;
      break;
    case ROOM_LAYOUT_OPEN:
    case ROOM_LAYOUT_MAZE:
      opt.score += 30 - ((depth + root->num_children)*5);
      break;
    case ROOM_LAYOUT_SUB:
      if(root->is_root)
        opt.score-=30;
      else
        opt.score+=depth + root->num_children;
      break;
  }

  switch(size){
    case ROOM_SIZE_SMALL:
      opt.score += (1+depth+root->num_children)*10;
      break;
    case ROOM_SIZE_MEDIUM:
      opt.score += (1+depth+root->num_children)*7;
      break;
    case ROOM_SIZE_MASSIVE:
    case ROOM_SIZE_XL:
    case ROOM_SIZE_HUGE:
      opt.score -= (1+depth+root->num_children)*5;
      break;
    default:
      opt.score+=5;
      break;
  }
  
  return opt;
}

bool NodeFindOptions(room_node_t* node, bool ignore_depth){
  for(int i = 0; i < node->max_children; i++){
    if(node->total < node->cap){
    RoomFlags test_flags = LayoutByWeight(ROOM_LAYOUT_MAX,99) | RandomShape() | SizeByWeight(ROOM_SIZE_MAX,67) | PurposeByWeight(ROOM_PURPOSE_CAMP,67);

     node_option_t opt = TryNodeOption(node, node->children[i], node->flags, test_flags);

     if(opt.score > 50)
       MapAddNodeOption(opt);
    }

    if(node->children[i]->used && node->children[i]->room)
      NodeFindOptions(node->children[i]->room,false);
  }

  return true;
}

MapNodeResult MapGenNodeScan(map_context_t *ctx, map_node_t *node){
  for(int i = 0; i < NUM_ANCHORS; i++)  
    NodeFindOptions(ctx->anchors[i],true);

  bool applied = false;
  int attempts = 0;
    usleep(9*PAUSE);
  while(!applied && attempts < 20){
    attempts++;
    node_option_t* best = PickBestNodeOption();

    if(!best)
      return MAP_NODE_FAILURE;

    if(!ApplyNode(best)){
      ATTEMPTS++;  
      best->score = 0;
    }
    else{
     char* purtext = GetPurposeName(best->flags);

     switch(GetRoomLayout(best->flags)){
       case ROOM_LAYOUT_HALL:
         TraceLog(LOG_INFO,"%s Hallway placed",purtext);
         break;
       case ROOM_LAYOUT_ROOM:
         TraceLog(LOG_INFO,"%s Room placed",purtext);
         break;
       case ROOM_LAYOUT_SUB:
         TraceLog(LOG_INFO,"%s Subroom placed",purtext);
         break;
       case ROOM_LAYOUT_OPEN:
         TraceLog(LOG_INFO,"%s Open Area placed", purtext);
         break;
       case ROOM_LAYOUT_MAZE:
         TraceLog(LOG_INFO,"%s Maze placed",purtext);
         break;
       default:
         TraceLog(LOG_WARNING,"Not sure why this was placed -%i",GetRoomLayout(best->flags));
         break;

     }
     if(GetRoomLayout(best->flags)==ROOM_LAYOUT_HALL)
       HALL_SCORE *= .8;

     return MAP_NODE_SUCCESS;
    }

  }

  return APPLIED<ctx->map_rules->min_rooms?MAP_NODE_FAILURE: MAP_NODE_SUCCESS; 
}

void NodeGetUsedConnections(room_node_t* node, node_connector_t **conn_pool, int *count){
  for(int i = 0; i < node->max_children; i++){
    node_connector_t *conn = node->children[i];
    if(!conn)
      continue;
    
    if(conn->enter || conn->used || conn->room){
      conn_pool[*count] = GameCalloc("NodeGetUsedConnections", 1,sizeof(node_connector_t));
      conn_pool[(*count)++] = conn;
    }

    if(conn->room)
       NodeGetUsedConnections(conn->room, conn_pool, count);
  }

}

void RoomSetExit(map_context_t* ctx, room_node_t* r){

  Cell dir = RoomFacingFromFlag(r->flags);

  int count = 0;
  int indexes[4];
  for(int i = 0; i < r->max_children; i++){
    node_connector_t* conn = r->children[i];

    if(!cell_compare(conn->dir, dir))
      continue;

    indexes[count++] = i;

    conn->used = true;
    conn->exit = true;

  }

  int best_mid = 0;
  if (count > 2)
    best_mid = count /2;

  if(r->children[indexes[best_mid]]){
    map_exit_t *exit = GameCalloc("RoomSetExit", 1,sizeof(map_exit_t));
    exit->tile = TILEFLAG_EXIT;
    exit->room = r;
    exit->conn = r->children[indexes[best_mid]];  
    ctx->exits[ctx->num_exits++]= exit;
  }

}

room_node_t* BuildNodeFromData(map_context_t *ctx, room_gen_t* data, Cell pos){
  RoomFlags flags = data->flags;
  room_node_t* root = MapBuildNode(flags, pos);
  root->is_root = true;
  root->sector = NUM_ANCHORS;
  root->cap = data->cap;
  root->section = root->bounds;
  //root->is_reachable = true;

  CAP+=root->cap;
  ctx->room_bounds[ctx->num_bounds++] = RecFromBounds(&root->bounds);

  int index = NUM_ANCHORS;
  root->col=col[index];
  ctx->anchors[NUM_ANCHORS++]= root;

  if(GetRoomPurpose(flags) == ROOM_PURPOSE_EXIT)
    RoomSetExit(ctx, root);

  for(int i = 0; i < data->num_children; i++){
    room_node_t* orphan = BuildNodeFromData(ctx,data->children[i],CELL_UNSET);
   
    root->orphans[root->num_orphans++] = orphan;
  }
  return root; 
}

MapNodeResult MapGenerateRooms(map_context_t *ctx, map_node_t *node){
  map_gen_t *gen = ctx->map_rules;
  Cell pos = CELL_NEW(35,30);
  room_gen_t* rgen = &gen->root;
  room_node_t* anchor = BuildNodeFromData(ctx,rgen, pos);

  return NUM_ANCHORS>0 ? MAP_NODE_SUCCESS : MAP_NODE_FAILURE;
}

void RoomCarveTiles(map_context_t* ctx, room_t* r){
  for (int y = r->bounds.min.y; y <= r->bounds.max.y; y++) {
    for (int x = r->bounds.min.x; x <= r->bounds.max.x; x++) {
      bool border = (x == r->bounds.min.x ||
          x == r->bounds.max.x ||
          y == r->bounds.min.y ||
          y == r->bounds.max.y);
      int r = RandRange(0,100);

      TileFlags floor = TILEFLAG_EMPTY;
      if(r<ctx->decor_density)
        floor = TILEFLAG_FLOOR;
      else if (r < ctx->decor_density *2)
        floor =  TILEFLAG_FLOOR|TILEFLAG_SIZE_XS;
      if(ctx->tiles[x][y])
      ctx->tiles[x][y] = border ? TILEFLAG_WALL : floor;
    }
  }
}

void RoomCarveOpenings(map_context_t* ctx, room_t* r){
  for(int i = 0; i < r->num_children; i++){
    if(!r->openings[i]->sub && !r->openings[i]->entrance)
      continue;
    Cell pos = r->openings[i]->pos;
    TileFlags opening = TILEFLAG_FLOOR;
    if(r->openings[i]->entrance)
      opening = ctx->map_rules->opening_flag;;


    bool opened = false;
    if(ctx->tiles[pos.x][pos.y]==TILEFLAG_WALL){
      opened = true;
      ctx->tiles[pos.x][pos.y] = opening;
    }
    else{
      pos = CellInc(r->openings[i]->pos, r->openings[i]->dir);
        if(ctx->tiles[pos.x][pos.y]==TILEFLAG_WALL){
          opened = true;
          ctx->tiles[pos.x][pos.y] = opening;
        }
    }

    RoomStatus s = opened?RS_CARVED:RS_WHERE_IS_THE_DOOR;
    ConnectionSetStatus(r->openings[i]->range.center,s);
    
    if(r->openings[i]->sub)
      RoomCarveOpenings(ctx,r->openings[i]->sub);
  }

}

MapNodeResult MapCarveTiles(map_context_t *ctx, map_node_t *node) {
  // clear
  for (int y = 0; y < ctx->height; y++)
    for (int x = 0; x < ctx->width; x++)
      ctx->tiles[x][y] = TILEFLAG_BORDER;

  // rooms
  for (int r = 0; r < ctx->num_rooms; r++)
    RoomCarveTiles(ctx,ctx->rooms[r]);

  for( int o = 0; o < ctx->num_rooms; o++){
    ctx->rooms[o]->ref->col.a*=0.5;
    RoomCarveOpenings(ctx, ctx->rooms[o]);
  }
  // halls already carved as floor in NodeConnectHalls
  return MAP_NODE_SUCCESS;
}

bool RoomPlaceSpawns(map_context_t *ctx, room_t *r){
  if(r->num_mobs > 0)
    return true;

  RoomFlags purpose = r->flags & ROOM_PURPOSE_MASK;

  float diff = ctx->map_rules->diff; 
  int num_spawns = 0; 
  if(purpose > ROOM_PURPOSE_CONNECT)
    return true;

  RoomFlags size = r->flags & ROOM_SIZE_MASK;
  RoomFlags layout = r->flags & ROOM_LAYOUT_MASK;
  RoomFlags orient = r->flags & ROOM_ORIENT_MASK;
  RoomFlags shape = r->flags & ROOM_SHAPE_MASK;

  MobRules mob_rules = GetMobRulesByMask(ctx->map_rules->mobs.rules,MOB_LOC_MASK);

 int importance = size>>10; 

  int budget =( size>>12); 

  switch(purpose){
    case ROOM_PURPOSE_CHALLENGE:
      mob_rules |= MOB_SPAWN_CHALLENGE;
      importance +=40+(r->depth*10);
      budget+=4+(r->depth*5);
      diff+=.375*r->depth;
      break;
    case ROOM_PURPOSE_LAIR:
      mob_rules |= MOB_SPAWN_LAIR;
      importance +=50;
      diff+=.5*r->depth;
      for(int d = 5; d<r->depth; d++){
        importance+=10;
        budget+=2;
      }
      break;
    case ROOM_PURPOSE_CONNECT:
      mob_rules |= MOB_SPAWN_PATROL;
      budget+=3;
      break;
    case ROOM_PURPOSE_CAMP:
      mob_rules |= MOB_SPAWN_CAMP;
      budget+=4;
      importance+=50;
      break;
    default:
      break;
  }

  switch(layout){
    case ROOM_LAYOUT_OPEN:
      mob_rules |= ( MOB_MOD_ENLARGE | MOB_SPAWN_CAMP );
      num_spawns++;
      importance+=20;
      budget+=2;
      diff+=.05*r->depth;
      break;
    case ROOM_LAYOUT_HALL:
      num_spawns--;
      budget-=1;
      mob_rules &= ~MOB_SPAWN_LAIR;
      break;
    case ROOM_LAYOUT_MAZE:
      num_spawns++;
      importance+=10;
      budget+=3;
      mob_rules |= MOB_SPAWN_PATROL;
      diff+=.05*r->depth;
      break;
    case ROOM_LAYOUT_ROOM:
      budget+=1;
      break;
    default:
      break;
  }

    
  MobRules grouping = GetMobRulesByMask(ctx->map_rules->mobs.rules, MOB_GROUPING_MASK);
  num_spawns+=size>>11;

  if(num_spawns < 9)
    grouping &= ~MOB_GROUPING_WARBAND;

  if(num_spawns < 7)
    grouping &= ~MOB_GROUPING_SQUAD;

  if(num_spawns < 5)
    grouping &= ~MOB_GROUPING_CREW;
  /*
  if(num_spawns < 4)
    grouping &= ~MOB_GROUPING_PARTY;

  if(num_spawns < 2)
    grouping &= ~MOB_GROUPING_TROOP;
*/
  int filtered = 0;

  int mobs = ctx->mob_pool->count;
  mob_define_t filtered_pool[ENT_DONE]; 
  filtered = GetMobsByDiff(diff, filtered_pool);
  filtered = FilterMobsByRules(grouping,filtered_pool,filtered,filtered_pool);

 for(int i = 0; i < filtered; i++)
    AddFilter(ctx->mob_pool, filtered_pool[i].id,  &filtered_pool[i]);
 
  mob_rules |= GetMobRulesByMask(ctx->map_rules->mobs.rules,MOB_MOD_MASK);
  mob_rules |= grouping;
  int built = 0;

  if(r->depth>4)
    mob_rules |= MOB_MOD_BEEF;

  budget+=r->depth;

  importance+=filtered+num_spawns;
  importance-=(NUM_MOBS - MOB_MAP_MAX);
  int rcheck = RandRange(num_spawns,100);
  if(rcheck>importance)
    return false;

  num_spawns = CLAMP(num_spawns,0,MOB_ROOM_MAX);
  
  ctx->mob_pool->budget = budget;//diff*10;
  int attempts = 0;
  while(built < num_spawns && attempts < 3){
    choice_t* sel = ctx->mob_pool->choose(ctx->mob_pool);
    if(sel == NULL)
      continue;

    mob_define_t def = MONSTER_MASH[sel->id];

    built += EntBuild(def,mob_rules,r->mobs);
    attempts++;
    if(built>0){
      MapUpdateBiome(ctx, built, &def);
      break;
    }
    else
      DO_NOTHING();
  }
  r->num_mobs+=built;
  NUM_MOBS+=built;//r->num_mobs;
  
  if(built>0){

    TraceLog(LOG_INFO,"NUM MOBS %i",NUM_MOBS);
  }
  return (built>0);
}

MapNodeResult MapPlaceSpawns(map_context_t *ctx, map_node_t *node) {
  int attempts = 0;
  while(NUM_MOBS<ctx->map_rules->min_mobs && attempts < MAX_ATTEMPTS){
    for (int r = 0; r < ctx->num_rooms; r++){
      if(!RoomPlaceSpawns(ctx, ctx->rooms[r]))
        attempts++;
    }
    TraceLog(LOG_INFO, "====== Spawn Attempts %i =====\n", attempts);
    //EndChoice(ctx->mob_pool,true);
  }

  return MAP_NODE_SUCCESS;
}

MapNodeResult MapEnhance(map_context_t *ctx, map_node_t *node) {
  return MAP_NODE_SUCCESS;

  int count = 0;
  room_t *enhance[ctx->num_rooms-2];
  for(int i = 0; i < ctx->num_rooms; i++){
    room_t* r = ctx->rooms[i];
    RoomFlags purpose = r->flags & ROOM_PURPOSE_MASK;

    if(purpose < ROOM_PURPOSE_TRAPPED)
      continue;

    enhance[count++] = r;

  }

  int traps = 0, loots = 0, chals = 0, lairs = 0, shh=0;
  room_t* trap[count];
  room_t* chest[count];
  room_t* chall[count];
  room_t* lair[count];
  room_t* secret[count];
  
  for(int i = 0; i < count; i++){
    room_t* r = ctx->rooms[i];
    RoomFlags purpose = r->flags & ROOM_PURPOSE_MASK;
    
    switch(purpose){
      case ROOM_PURPOSE_TRAPPED:
        trap[traps++] = r;
        break;
      case ROOM_PURPOSE_SECRET:
        secret[shh++] = r;
        break;
      case ROOM_PURPOSE_TREASURE:
        chest[loots++] = r;
        break;
      case ROOM_PURPOSE_CHALLENGE:
        chall[chals++] = r;
        break;
      case ROOM_PURPOSE_LAIR:
        lair[lairs++] = r;
        break;
      default:
        continue;
        break;

    };

  }

  MapAssignTreasures(ctx, chest, loots);
  MapAssignTraps(ctx, trap, traps);
  MapAssignSecrets(ctx, secret, shh);
  MapAssignChallenges(ctx, chall, chals);
  MapAssignLairs(ctx, lair, lairs);

  return MAP_NODE_SUCCESS;
}

int RoomScore(room_t* r){
  int w,h;
  RoomDimensionsFromFlags(r->flags, &w, &h);
  return (r->depth+1) * w*h;
};

void RoomAddTreasure(room_t* r, int score){

}

void MapAssignTreasures(map_context_t *ctx, room_t** rooms, int count){
  int cap = ctx->num_rooms * BIOME[ctx->map_rules->biome].r_treasure;

  if(cap < 1)
    return;

  choice_pool_t* picker = InitChoicePool(count, ChooseBest);

  for(int i = 0; i < count; i++){
    int score = RoomScore(rooms[i]);
    AddChoice(picker, i, score, rooms[i], DiscardChoice);
  }

  for(int i = 0; i < cap; i++){
    choice_t* c = picker->choose(picker);
    if(c==NULL)
      continue;
    room_t* r = c->context;
    if(r == NULL)
      continue;

    RoomAddTreasure(r,c->score);
  }
}

int RoomScoreByMobs(room_t* r, MobRules spawn, MobRules rules, RaceProps props){

  float spawn_mod = 0.5;
  float r_score = 0;
  float p_score = 0;
  ent_t* e = r->mobs[0];
  
  RaceProps m_props = DEFINE_RACE[SpecToIndex(e->props->race)].props;
  MobRules  m_rules = MONSTER_MASH[e->type].rules;

  if((m_rules & spawn)>0)
    spawn_mod = 1;
  while(rules){
    uint64_t rule = rules & -rules;
    rules &= rules - 1;

    if((m_rules&rule)>0)
      r_score+spawn_mod;
  }

  while(props){
    uint64_t prop = props & -props;
    props &= props - 1;

    if((m_props&prop)>0)
      p_score+=spawn_mod;
  }

  return r->num_mobs * (p_score+r_score) * e->props->base_diff;

}

void RoomAddMobs(map_context_t* ctx, room_t* r, int score, MobRules rule){
  int beef = r->depth+1;
  int max_cr = score + (75*beef) * ctx->map_rules->diff;
  int min_cr = score + ((75*beef) * ctx->map_rules->diff);

  choice_pool_t* picker = InitChoicePool(ENT_DONE, ChooseByWeight);

  MobRules loc = ctx->map_rules->mobs.rules&MOB_LOC_MASK;

  int count = 0;
  for(int i = 0; i < ENT_DONE; i++){
    mob_define_t def = MONSTER_MASH[i];

    if((def.rules&rule)==0)
      continue;

    if((def.rules&loc)==0)
      continue;

    MobRules theme = def.rules & MOB_THEME_MASK;

    if(theme < MOB_THEME_MARTIAL)
      continue;

    if(AddChoice(picker, i, def.weight[ctx->map_rules->biome], &MONSTER_MASH[i], NULL))
      count++;
  }

  if(count < 1)
    return;

  choice_t* sel = picker->choose(picker);
  if(sel==NULL)
    return;

  MobRules grouping = MOB_GROUPING_PAIRS;
  switch(r->flags&ROOM_SIZE_MASK){
    case ROOM_SIZE_MEDIUM:
      grouping = MOB_GROUPING_TROOP;
      break;
    case ROOM_SIZE_LARGE:
      grouping = MOB_GROUPING_PARTY;
      break;
    case ROOM_SIZE_XL:
      grouping = MOB_GROUPING_CREW;
      break;
    case ROOM_SIZE_HUGE:
      grouping = MOB_GROUPING_SQUAD;
      break;
    case ROOM_SIZE_MASSIVE:
      grouping = MOB_GROUPING_WARBAND;
      break;
  }

  MobRules rules = loc | rule | MOB_MOD_WEAPON | MOB_MOD_ARMOR | grouping;
  mob_define_t *def = sel->context;
  int built = EntBuild(*def, rules, r->mobs);

  r->num_mobs = built;
}

void RoomEnhanceMobs(map_context_t* ctx, room_t* r, int score, MobRules rule){
  ent_t* e = r->mobs[0];

  RaceProps m_props = DEFINE_RACE[SpecToIndex(e->props->race)].props;
  MobRules  m_rules = MONSTER_MASH[e->type].rules;

  if((m_rules & rule)==0){
    //RoomAddMobs(ctx, r, score, rule);
    return;
  }

  int beef = r->depth+1;
  int max_cr = score + ((75*beef) * ctx->map_rules->max_diff);
  int min_cr = score + ((75*beef) * ctx->map_rules->diff);

  int total_cr = 0, cr_cap = max_cr*r->num_mobs, cr_min = min_cr*r->num_mobs;

  for(int i = 0; i < r->num_mobs; i++)
    total_cr+=r->mobs[i]->props->cr;

  if(total_cr > min_cr)
    return;

  while(total_cr < cr_cap){
    total_cr = EnhanceEnts(r->mobs, rule, r->num_mobs);
  }
}


void MapAssignChallenges(map_context_t *ctx, room_t** rooms, int count){
  int cap = ctx->num_rooms * BIOME[ctx->map_rules->biome].r_chall;

  if(cap < 1)
    return;

  if(count < cap)
    cap = count;

  choice_pool_t* picker = InitChoicePool(count, ChooseBest);

  for(int i = 0; i < count; i++){
    room_t* r = rooms[i];
    int score = 0;
    if(r->num_mobs == 0)
      score = RoomScore(r);
    else{
      score = RoomScoreByMobs(r,MOB_SPAWN_CHALLENGE, MOB_SPAWN_CAMP | 
          MOB_SPAWN_PATROL | MOB_MOD_MASK | MOB_THEME_MARTIAL | MOB_THEME_PRIMITIVE,
          RACE_USE_WEAPS | RACE_USE_ARMOR | RACE_USE_POTIONS | RACE_DIFF_LVL | RACE_DIFF_SKILL | RACE_DIFF_SPELLS | RACE_DIFF_PETS);
      if(score<5)
        score+=RoomScore(r);
    }
    TraceLog(LOG_INFO,"====CHALLENGE====\nscore %i",score);
    AddChoice(picker, i, score, r, DiscardChoice);
  }

  for(int i = 0; i < cap; i++){
    choice_t* c = picker->choose(picker);
    if(c==NULL)
      continue;
    room_t* r = c->context;
    if(r == NULL)
      continue;

    if(r->num_mobs == 0)
      RoomAddMobs(ctx, r,c->score, MOB_SPAWN_CHALLENGE);
    else
      RoomEnhanceMobs(ctx, r, c->score, MOB_SPAWN_CHALLENGE);
  }
}

void MapAssignTraps(map_context_t *ctx, room_t** rooms, int count){

}

void MapAssignSecrets(map_context_t *ctx, room_t** rooms, int count){

}

void MapAssignLairs(map_context_t *ctx, room_t** rooms, int count){
  int cap = ctx->num_rooms * BIOME[ctx->map_rules->biome].r_lairs;

  if(cap < 1)
    return;

  if(count < cap)
    cap = count;

  choice_pool_t* picker = InitChoicePool(count, ChooseBest);

  for(int i = 0; i < count; i++){
    room_t* r = rooms[i];
    int score = 0;
    if(r->num_mobs == 0)
      score = RoomScore(r);
    else{ 
      score = RoomScoreByMobs(r,MOB_SPAWN_LAIR,  MOB_MOD_ENLARGE | MOB_THEME_GAME | MOB_THEME_PRED | MOB_THEME_PRIMITIVE,
          RACE_DIFF_LVL | RACE_DIFF_SKILL | RACE_DIFF_ALPHA );
      if(score<5)
        score+=RoomScore(r);
    }
    TraceLog(LOG_INFO,"===LAIR===\nscore %i",score);
    AddChoice(picker, i, score, r, DiscardChoice);
  }     
      
  for(int i = 0; i < cap; i++){
    choice_t* c = picker->choose(picker);
    if(c==NULL)
      continue;
    room_t* r = c->context;
    if(r == NULL)
      continue;
        
    if(r->num_mobs == 0)
      RoomAddMobs(ctx, r,c->score, MOB_SPAWN_LAIR);
    else
      RoomEnhanceMobs(ctx, r, c->score, MOB_SPAWN_LAIR);
  }     
}

MapNodeResult NodeDecorate(map_context_t *ctx, map_node_t *node) {

  for (int r = 0; r < ctx->num_rooms; r++) {
    room_t *room = ctx->rooms[r];
    TileFlags special = RoomSpecialDecor(room->flags & ROOM_PURPOSE_MASK);

    for (int y = room->bounds.min.y; y <= room->bounds.max.y; y++) {
      for (int x = room->bounds.min.x; x <= room->bounds.max.x; x++) {

        TileFlags *tile = &ctx->tiles[x][y];

        // Skip uncarved / invalid tiles
        if (!TileHasFlag(*tile, TILEFLAG_FLOOR))
          continue;

        // Skip tiles with reserved flags
        if (TileHasFlag(*tile, TILEFLAG_SPAWN | TILEFLAG_START |TILEFLAG_BORDER))
          continue;

        // Try special room decor first
        if (special && (rand() % 6 == 0)) {
          *tile |= special;
        }

        // Try global decor rules
        for (int i = 0; i < dungeon_decor_count; i++) {
          DecorRule *rule = &dungeon_decor[i];

          if (!TileHasFlag(*tile, rule->required_floor))
            continue;

          if (TileHasFlag(*tile, rule->forbidden))
            continue;

          if (rand() % rule->chance == 0) {
            *tile |= rule->deco_flag;
            break;
          }
        }
      }
    }
  }

  return MAP_NODE_SUCCESS;
}


MapNodeResult MapShapeRoom(room_t* r) {
  r->bounds = RoomBounds(r->flags,r->center);

  return MAP_NODE_SUCCESS;

}

MapNodeResult MapApplyRoomShapes(map_context_t *ctx, map_node_t *node) {

  for (int i = 0; i < ctx->num_rooms; i++) {
    room_t *room = ctx->rooms[i];
    MapShapeRoom(room);

    for(int k = 0; k < room->num_children; k++){
      if(room->openings[k]->sub == NULL)
        continue;

      MapShapeRoom(room->openings[k]->sub);
    }
  }

  return MAP_NODE_SUCCESS;

}

Cell RoomGetBestOpening(room_t* root, room_t* sub, int child){
  Cell r_dir = RoomFlagsGetPlacing(root->flags);
  RoomFlags s_dir = sub->flags&ROOM_PLACING_MASK;
  Cell size = RoomSize(sub);

  Cell pos = CellMul(root->openings[child]->dir,size);

  pos = CellInc(CellMul(r_dir,size),pos);

  return pos;
}


void RoomAdjustPosition(room_t *r, Cell disp, bool children){

  r->center = CellInc(r->center,disp);

  r->ref->center = r->center;
  r->bounds = RoomBounds(r->flags, r->center);

  r->ref->bounds = r->bounds;

  for(int i = 0; i < r->num_children; i++){
    r->openings[i]->pos = CellInc(r->openings[i]->pos,disp);
    r->openings[i]->range = ShiftCellBounds(r->openings[i]->range,disp);
    r->openings[i]->rebound = true;

    if(!children)
      continue;

    if(!r->openings[i]->sub)
      continue;

    RoomAdjustPosition(r->openings[i]->sub, disp,true);
  }
}

MapNodeResult MapPlaceSubrooms(map_context_t *ctx, map_node_t *node) {
  return MAP_NODE_SUCCESS;
}

MapNodeResult MapComputeBounds(map_context_t *ctx, map_node_t *node)
{
  if (ctx->num_rooms <= 0)
    return MAP_NODE_FAILURE;

  int minx = INT_MAX;
  int miny = INT_MAX;
  int maxx = INT_MIN;
  int maxy = INT_MIN;

  // --- 1. Compute global min/max of all room bounds ---
  for (int i = 0; i < ctx->num_rooms; i++) {
     room_t *r = ctx->rooms[i];
     if (r->bounds.min.x < minx) minx = r->bounds.min.x;
      if (r->bounds.min.y < miny) miny = r->bounds.min.y;
      if (r->bounds.max.x > maxx) maxx = r->bounds.max.x;
      if (r->bounds.max.y > maxy) maxy = r->bounds.max.y;
  }

  // Expand slightly if you want padding around the dungeon:
  int padding = 8;//ctx->map_rules->border;
  minx -= padding;
  miny -= padding;
  maxx += padding;
  maxy += padding;

  ctx->level.min.x = minx;
  ctx->level.min.y = miny;
  ctx->level.max.x = maxx;
  ctx->level.max.y = maxy;
  // --- 2. Compute width + height of final map ---
  ctx->width  = abs(maxx) + abs(minx);// + ctx->map_rules->border;
  ctx->height = abs(maxy) + abs(miny);// + ctx->map_rules->border;;


  TraceLog(LOG_INFO,"\n=====MAP SIZE ===\n===== WIDTH %i====\n====HEIGHT %i ====\n",ctx->width,ctx->height);

  Cell offset = CELL_NEW(-minx,-miny);

  for (int i = 0; i < ctx->num_rooms; i++) {

    usleep(4*PAUSE);

    RoomAdjustPosition(ctx->rooms[i],offset,false);

  }

  for(int i = 0; i < ctx->num_conn; i++){
    ConnectionSetIID(ctx->connections[i]);
  }

  return MAP_NODE_SUCCESS;
}

MapNodeResult MapAllocateTiles(map_context_t *ctx, map_node_t *node)
{
    if (ctx->width <= 0 || ctx->height <= 0)
        return MAP_NODE_FAILURE;

    // --- Free old tile memory if previously allocated ---
    if (ctx->tiles && ctx->alloc_w > 0 && ctx->alloc_h > 0) {
        for (int x = 0; x < ctx->alloc_w; x++)
            free(ctx->tiles[x]);

        free(ctx->tiles);
    }

    // --- Allocate new tile grid ---
    ctx->tiles = GameMalloc("MapAllocateTiles", ctx->width * sizeof(TileFlags*));
    if (!ctx->tiles)
        return MAP_NODE_FAILURE;

    for (int x = 0; x < ctx->width; x++) {
        ctx->tiles[x] = GameCalloc("MapAllocateTiles",
            ctx->height, sizeof(TileFlags));
        if (!ctx->tiles[x])
            return MAP_NODE_FAILURE;
    }

    ctx->alloc_w = ctx->width;
    ctx->alloc_h = ctx->height;

    return MAP_NODE_SUCCESS;
}

MapNodeResult MapPlayerSpawn(map_context_t *ctx, map_node_t *node){
  for(int i = 0; i < ctx->num_rooms; i++){
    RoomFlags purpose = ctx->rooms[i]->flags&ROOM_PURPOSE_MASK;
    if(purpose!= ROOM_PURPOSE_START)
      continue;

    ctx->player_start = ctx->rooms[i]->center;
    return MAP_NODE_SUCCESS;
  }

  return MAP_NODE_FAILURE;
}

MapNodeResult MapFillWalls(map_context_t *ctx, map_node_t *node){
  for(int x = 0; x<ctx->width; x++){
    for (int y = 0; y < ctx->height; y++){
      TileFlags *tile = &ctx->tiles[x][y];

      if (!TileFlagHas(*tile, TILEFLAG_EMPTY| TILEFLAG_FLOOR))
        continue;

      for(int nx = x-1; nx < x+1; nx++){
        for(int ny = y-1; ny < y+1; ny++){
          if(nx<0 || nx > ctx->width || ny < 0 || ny > ctx->width)
            continue;
          
          if(!is_adjacent(CELL_NEW(x,y),CELL_NEW(nx,ny)))
            continue;

          if(ctx->tiles[nx][ny] < TILEFLAG_EMPTY)
            ctx->tiles[nx][ny] = TILEFLAG_WALL;
        }
      }
    }
  }
    
  return MAP_NODE_SUCCESS;
}

TileFlags RoomSpecialDecor(RoomFlags p) {
  switch(p) {
    case ROOM_PURPOSE_CHALLENGE: return TILEFLAG_DEBRIS;
    case ROOM_PURPOSE_TREASURE:  return TILEFLAG_DECOR;
    case ROOM_PURPOSE_SECRET:      return TILEFLAG_DEBRIS | TILEFLAG_DECOR;
    default:             return 0;
  }
}

void MapAddNodeOption(node_option_t opt){ 
  if (NODE_OPTIONS.count < MAX_OPTIONS) {
    NODE_OPTIONS.items[NODE_OPTIONS.count++] = opt;
  }
}
int GetNeighborFlags(map_context_t* ctx, Cell c, RoomFlags f, Cell *filter){
  int count = 0;

  for (int dy = -1; dy <= 1; dy++) {
    int ny = c.y+dy;
    if (ny < 0 || ny >= ctx->height)  continue;
    if (ctx->tiles[c.x][ny] & f)
      filter[count++] = CELL_NEW(c.x,ny);
  }

  for (int dx = -1; dx <= 1; dx++) {

    if (dx == 0) continue;

    int nx = c.x+dx;

    if (nx < 0 || nx >= ctx->width) continue;

    if (ctx->tiles[nx][c.y] & f)
      filter[count++] = CELL_NEW(nx,c.y);

  }

  return count;
}

bool HasFloorNeighbor(map_context_t* ctx, int x, int y){
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {

      if (dx == 0 && dy == 0) continue;

      int nx = x + dx;
      int ny = y + dy;

      if (nx < 0 || nx >= ctx->width)  continue;
      if (ny < 0 || ny >= ctx->height) continue;

      if (ctx->tiles[ny][nx] & TILEFLAG_FLOOR)
        return true;
    }
  }
  return false;
}

bool ApplyNode(node_option_t *opt){
  bool result = false;
  switch(opt->type){
    case OPT_ROOM:
      result = MapAddNodeToConnector(opt->connect,MapBuildNode(opt->flags, CELL_EMPTY));
      break;
    case OPT_CONN:
      result = MapAddNodeToConnector(opt->connect,MapBuildNode(opt->flags, CELL_EMPTY));
      break;
  }

  return result;
}

void RefreshNodeOptions(void){
  NODE_OPTIONS.count = 0;
}

node_option_t *PickBestNodeOption(void){
  if (NODE_OPTIONS.count == 0) return NULL;

  int best_index = 0;

  for (int i = 1; i < NODE_OPTIONS.count; i++) {
    if (NODE_OPTIONS.items[i].score > NODE_OPTIONS.items[best_index].score) {
      best_index = i;
    }
  }

  return &NODE_OPTIONS.items[best_index];
}


bool MapCheckOverlap(Rectangle bounds, Vector2 *overlap){
  for (int i = 0; i < world_map.num_bounds; i++){
    if(!GetRectOverlap(bounds, world_map.room_bounds[i], overlap))
      continue;
    if(Vector2Length(*overlap) < world_map.map_rules->margin_error)
      continue;

    return true;
  }

  return false;
}

bool room_is_enclosed(map_context_t* ctx, cell_bounds_t room){
    int left   = room.min.x;
    int right  = room.max.x;
    int top    = room.min.y;
    int bottom = room.max.y;

    int map_h = ctx->height;
    int map_w = ctx->width;
    // Loop over room perimeter
    for (int x = left; x <= right; x++) {
        for (int y = top; y <= bottom; y++) {

            bool is_edge = (x == left || x == right || y == top || y == bottom);
            if (!is_edge) continue; // skip interior tiles

            // If touching outside of map, it counts as enclosed
            if (x < 0 || x >= map_w || y < 0 || y >= map_h)
                continue;

            if (!TileFlagBlocksMovement(ctx->tiles[x][y]))
              return false;

        }
    }

    return true;
}

int ConnectionsByIID(room_connection_t* conn, room_connection_t** pairs){
  int count = 0;
  Cell pos = conn->opening->range.center;

  int index = IntGridIndex(pos.x,pos.y);

  for(int i = 0; i < world_map.num_conn; i++){
    if(world_map.connections[i]->grid_iid!= index)
      continue;

    pairs[count++] = world_map.connections[i];
  }

  return count;
}

void ConnectionSetIID(room_connection_t* conn){
  Cell pos = conn->opening->range.center;
  
  conn->grid_iid = IntGridIndex(pos.x,pos.y);
}

int ConnectionGetNeighbors(room_connection_t* conn, room_connection_t** pairs, int range, int cap){

  int count = 0;
  Cell pos = conn->opening->range.center;
  Rectangle rec = RecFromBounds(&conn->opening->range);    
  int index = IntGridIndex(pos.x,pos.y);
  
  for(int i = 0; i < world_map.num_conn; i++){
    if(count>=cap)
      break;
    room_connection_t* nei = world_map.connections[i];
    if(nei->grid_iid==index)
      continue;

    if(cell_compare(nei->opening->pos,pos)){
      pairs[count++] = world_map.connections[i];
      continue;
    }
    
    int c_dist = CELL_LEN(CellSub(pos,nei->opening->range.center));
    if(c_dist > range)
      continue;

    Vector2 dist;
    Rectangle n_rec = RecFromBounds(&nei->opening->range);

    bool overlap = GetRectOverlap(rec,n_rec,&dist);

    if(overlap){
      pairs[count++] = world_map.connections[i];
      continue;
    }

    float n_dist = Vector2Length(dist);

    if(n_dist <= range){
pairs[count++] = world_map.connections[i];
      continue;
    }
  }

  return count;
}

void ConnectionSetStatus(Cell pos, RoomStatus status){
  int index = IntGridIndex(pos.x,pos.y);

  for(int i = 0; i < world_map.num_conn; i++){
    if(world_map.connections[i]->grid_iid!= index)
      continue;

    world_map.connections[i]->status = status;
  }
}

void MapVisEvent(EventType event, void* data, void* user){
  map_cell_t* mc = user;
  ent_t* e = data;

  int range = e->senses[SEN_SEE]->range;

  int dist = cell_distance(e->pos, mc->coords);
  switch(mc->vis){
    case VIS_UNSEEN:
    case VIS_EXPLORED:
      if(dist < range)
        mc->vis = VIS_FULL;
      if(dist == range)
        mc->vis = VIS_EXPLORED;
      break;
    case VIS_FULL:
      if(dist > range)
        mc->vis = VIS_EXPLORED;
      break;
  }
}
