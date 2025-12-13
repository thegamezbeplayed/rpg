#include "game_process.h"
#include "game_helpers.h"
#include <limits.h>
static map_context_t world_map;
static path_node_t nodes[GRID_WIDTH][GRID_HEIGHT];
static option_pool_t map_pool;
static node_option_pool_t NODE_OPTIONS;
static int APPLIED = 0;
static int ATTEMPTS = 0;
static int FIX_ATTEMPTS = 0;
static int HALL_SCORE = 55;
static int CAP = 0;
static int NUM_ANCHORS = 0;
static sprite_t* mark_spr[16];
static int markers;

bool InitMap(void){
  map_context_t* ctx =&world_map;
  memset(ctx, 0, sizeof(*ctx));
  ctx->map_rules = malloc(sizeof(map_gen_t));
  *ctx->map_rules = MAPS[DANK_DUNGEON];

  ctx->level = BoundsFromRec(Rect(99,99,-99,-99));
  ctx->player_start = CELL_UNSET;
  ctx->decor_density = 12;  
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

MapNodeResult MapGraphNodes(map_context_t *ctx, map_node_t *node){
  TraceLog(LOG_INFO,"===Generate New Node====");
  if(MapRoomNodeScan(ctx,NULL)==MAP_NODE_SUCCESS){
    RefreshNodeOptions();
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
  room_t* r = calloc(1,sizeof(room_t));

  r->flags = node->flags;
  r->bounds = node->bounds;
  r->center = node->center;
  r->ref = node;

  node->applied = true;

  node->col.a*=.1;
  if(node->max_children > 15)
    DO_NOTHING();

  for(int i = 0; i < node->max_children; i++){
    r->openings[i] = calloc(1,sizeof(room_opening_t));
    r->openings[i]->dir = node->children[i]->dir;
    r->openings[i]->pos = CellInc(node->children[i]->range.center,node->children[i]->dir);
    cell_bounds_t range = node->children[i]->range;

    r->openings[i]->range = ShiftCellBounds(range,r->openings[i]->dir);

    r->openings[i]->entrance = node->children[i]->enter;

    if(r->openings[i]->entrance)
      DO_NOTHING();
 
    r->num_children++;
    room_t* to = NULL;
    if(node->children[i]->room)
     to = ConvertNodeToRoom(ctx,node->children[i]->room); 
  
    if(to)
      ctx->rooms[ctx->num_rooms++]= to;

    ctx->connections[ctx->num_conn] = calloc(1,sizeof(room_connection_t));

    room_connection_t* r_conn = ctx->connections[ctx->num_conn++];

    r_conn->to = to;
    r_conn->from = r;
    r_conn->range = &r->openings[i]->range;

  }
  return r;
}

MapNodeResult MapNodesToGrid(map_context_t *ctx, map_node_t *node){
  int num_roots = ctx->num_rooms;
  
  for(int i = 0; i < num_roots; i++){

    room_node_t* root = ctx->anchors[i];


    ctx->rooms[i] = ConvertNodeToRoom(ctx,root);
  }

  return MAP_NODE_SUCCESS;
}

void DrawNode(room_node_t* node, Color col){
  Rectangle out = RectScale(RecFromBounds(&node->bounds),16);
  out.x*=16;
  out.y *=16;
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
    Rectangle sect = RectScale(RecFromBounds(&node->section),16);
    sect.x*=16;
    sect.y*=16;
    DrawRectangleLinesEx(sect,2,col);
  }
  if(node->entrance_index > -1){
    Rectangle d_pos = RectScale(RecFromBounds(&node->children[node->entrance_index]->range),16);
    d_pos.x*=16;
    d_pos.y*=16;
    DrawRectangleRec(d_pos,WHITE);
  }
  DrawRectangleLinesEx(out,1, BLACK);
  }
  if(write)
    DrawText(mark, node->center.x*16 -3,node->center.y*16 -3 , 14, BLACK);

  for(int i = 0; i < node->max_children; i++){
    if(!node->children[i]->used)
      continue;
   
    if(node->children[i]->room)
      DrawNode(node->children[i]->room,col);
  }
}

void MapRender(void){
  Rectangle sect = RectScale(RecFromBounds(&world_map.level),16);
  sect.x*=16;
  sect.y*=16;

  DrawRectangleLinesEx(sect,4,BLACK);
  if(world_map.status == GEN_DONE){
    for(int x = 0; x < world_map.width; x++){
      for(int y = 0; y < world_map.height; y++){
        if(world_map.tiles[x][y] <1)
          continue;

        int tile = GetTileByFlags(world_map.tiles[x][y] |world_map.map_rules->map_flag);
        Cell pos = CELL_NEW(x,y);
        Vector2 vpos = CellToVector2(pos, 16);

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

  for(int i = 0; i < world_map.num_issues; i++){
    room_connection_t* issue = world_map.issues[i];
    if(issue->status == RS_GTG)
      continue;

    Rectangle problem = RectScale(RecFromBounds(issue->range),16);
    problem.x*=16;
    problem.y*=16;

    DrawRectangleLinesEx(problem,4,BLACK);
  }
}

map_grid_t* InitMapGrid(void){
  map_grid_t* m = malloc(sizeof(map_grid_t));

  *m = (map_grid_t){0};

  m->step_size = CELL_WIDTH;

  m->x = 0;
  m->y = 0;
  m->width = world_map.width;
  m->height = world_map.height;
  m->floor = DARKBROWN;
  MapApplyContext(m);
  return m;
}

map_node_t* MapBuildNodeRules(MapNodeType id){
  if(room_nodes[id].id !=id)
    return NULL;

  map_node_data_t data = room_nodes[id];

  if(data.type == MAP_NODE_LEAF)
    return data.fn(id);

  map_node_t **kids = malloc(sizeof(kids)*data.num_children);
  for(int i = 0; i < data.num_children; ++i)
    kids[i] = MapBuildNodeRules(data.children[i]);

  return MapCreateSequence( id, kids, data.num_children);
}

map_node_t* MapCreateLeafNode(MapNodeFn fn, MapNodeID id){  
  map_node_t* node = malloc(sizeof(map_node_t));
  node->run = fn;
  node->id = id;
  node->type = MAP_NODE_LEAF;

  return node;
}

map_node_t* MapCreateSequence( MapNodeID id, map_node_t **children, int count){  
  map_node_t* node = malloc(sizeof(map_node_t));
  node->num_children = count;
  node->children = children;
  node->run = MapNodeRunSequence;
  node->id = id;
  node->type = MAP_NODE_SEQUENCE;

  return node;
}

Cell MapApplyContext(map_grid_t* m){
  m->id = world_map.map_rules->id;
  m->tiles = malloc(world_map.width * sizeof(map_cell_t*));
  m->width = world_map.width;
  m->height = world_map.height;
  Cell out = CELL_UNSET;
  
  for(int x = 0; x < world_map.width; x++){
    m->tiles[x] = calloc(m->height, sizeof(map_cell_t));
    for(int y = 0; y < world_map.height; y++){
      m->tiles[x][y].coords = CELL_NEW(x,y); 
      m->tiles[x][y].fow = BLACK;  
      m->tiles[x][y].occupant = NULL;  
      MapSpawn(world_map.tiles[x][y],x,y);
    }
  }
  /*
  for(int r = 0; r < world_map.num_rooms; r++){
    //RoomSpawnMob(m, world_map.rooms[r]);
    for(int o = 0; o < world_map.rooms[r]->num_children; o++){
      if(world_map.rooms[r]->openings[o]->sub)
        RoomSpawnMob(m, world_map.rooms[r]->openings[o]->sub);
    }
  }
  */
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

  m->tiles[c.x][c.y].tile = e;
  if(TileHasFlag(e->type, TILEFLAG_SOLID))
    m->tiles[c.x][c.y].status = TILE_COLLISION;
  else if(TileHasFlag(e->type, TILEFLAG_BORDER))
    m->tiles[c.x][c.y].status = TILE_BORDER;
  else
    m->tiles[c.x][c.y].status = TILE_EMPTY;

  return TILE_SUCCESS;
}

TileStatus MapSetOccupant(map_grid_t* m, ent_t* e, Cell c){
  Cell bounds = CELL_NEW(m->width,m->height);
  if(!cell_in_bounds(c,bounds))
    return TILE_OUT_OF_BOUNDS;

  if(m->tiles[c.x][c.y].status > TILE_ISSUES)
    return TILE_OCCUPIED;

  MapRemoveOccupant(m,e->pos);
  m->tiles[c.x][c.y].occupant =e;
  m->tiles[c.x][c.y].status = TILE_OCCUPIED;

  return TILE_SUCCESS;
}

map_cell_t* MapGetTile(map_grid_t* map,Cell tile){
  Cell bounds = CELL_NEW(map->width,map->height);

  if(!cell_in_bounds(tile,bounds))
    return NULL;

  return &map->tiles[tile.x][tile.y];

}

TileStatus MapRemoveOccupant(map_grid_t* m, Cell c){
  Cell bounds = CELL_NEW(m->width,m->height);
  if(!cell_in_bounds(c,bounds))
    return TILE_OUT_OF_BOUNDS;

  m->tiles[c.x][c.y].occupant = NULL;

  m->tiles[c.x][c.y].status = TILE_EMPTY;

  return TILE_SUCCESS;
}

ent_t* MapGetOccupant(map_grid_t* m, Cell c, TileStatus* status){
  map_cell_t tile = m->tiles[c.x][c.y];

  *status = tile.status;

  return tile.occupant;
}

bool TileFlagHasAccess(TileFlags f) {

  return (f & TILEFLAG_DOOR) || (f & TILEFLAG_FLOOR) || (f & TILEFLAG_EMPTY);
}

bool TileFlagBlocksMovement(TileFlags f) {

  return (f & TILEFLAG_SOLID) || (f & TILEFLAG_BORDER);
}

bool TileBlocksMovement(map_cell_t *c) {
  if(c->tile==NULL)
    return false;

  uint32_t f = EnvTileFlags[c->tile->type];
  return (f & TILEFLAG_SOLID) || (f & TILEFLAG_BORDER);
}

bool TileBlocksSight(map_cell_t *c) {
  if(c->tile==NULL)
    return false;

  uint32_t f = EnvTileFlags[c->tile->type];
  return (f & TILEFLAG_SOLID) || (f & TILEFLAG_OBSTRUCT);
}

bool CheckPath(map_context_t* m, Cell from, Cell to, Cell *block){
  if (!cell_compare(from,to))
    return false;

  int sx = from.x;
  int sy = from.y;
  
  int tx = to.x;
  int ty = to.y;
  
  // Init nodes
  for (int y = 0; y < m->height; y++)
    for (int x = 0; x < m->width; x++) {
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
    }

  path_node_t *start = &nodes[sx][sy];
  path_node_t *goal  = &nodes[tx][ty];

  start->gCost = 0;
  start->hCost = Heuristic(sx, sy, tx, ty);
  start->fCost = start->hCost;
  start->open = true;

      while (1)
    {
        // Step 1: Find lowest fCost open node
        path_node_t *current = NULL;

        for (int y = 0; y < m->height; y++)
        for (int x = 0; x < m->width; x++) {
            path_node_t *n = &nodes[x][y];
            if (n->open && !n->closed) {
                if (!current || n->fCost < current->fCost)
                    current = n;
            }
        }

        if (!current) {
            return false; // no path
        }

        // Reached target
        if (current == goal)
            break;

        current->open = false;
        current->closed = true;

        // Step 2: Explore neighbors
        const int dirs[4][2] = {
            { 1, 0 }, {-1, 0},
            { 0, 1 }, { 0,-1}
        };

        for (int i = 0; i < 4; i++)
        {
            int nx = current->x + dirs[i][0];
            int ny = current->y + dirs[i][1];

            //if (!InBounds(m, nx, ny)) continue;
            if (TileFlagBlocksMovement(m->tiles[nx][ny])) continue;

            path_node_t *neighbor = &nodes[nx][ny];
            if (neighbor->closed) continue;

            int cost = current->gCost + 1;

            if (!neighbor->open || cost < neighbor->gCost) {
                neighbor->gCost = cost;
                neighbor->hCost = Heuristic(nx, ny, tx, ty);
                neighbor->fCost = neighbor->gCost + neighbor->hCost;

                neighbor->parentX = current->x;
                neighbor->parentY = current->y;

                neighbor->open = true;
            }
        }
    }

      path_node_t *node = goal;

      while (node->parentX != sx || node->parentY != sy) {
        if (node->parentX < 0 || node->parentY < 0)
          return false;
        node = &nodes[node->parentX][node->parentY];
      }

      // Write next step
      //outNextStep->x = node->x;
      //outNextStep->y = node->y;
      
      return true;
}

bool FindPath(map_grid_t *m, int sx, int sy, int tx, int ty, Cell *outNextStep){
    // Early out: same tile
    if (sx == tx && sy == ty)
        return false;

    // Init nodes
    for (int y = 0; y < m->height; y++)
    for (int x = 0; x < m->width; x++) {
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
    }

    // Pointer to nodes
    path_node_t *start = &nodes[sx][sy];
    path_node_t *goal  = &nodes[tx][ty];

    start->gCost = 0;
    start->hCost = Heuristic(sx, sy, tx, ty);
    start->fCost = start->hCost;
    start->open = true;

    while (1)
    {
        // Step 1: Find lowest fCost open node
        path_node_t *current = NULL;

        for (int y = 0; y < m->height; y++)
        for (int x = 0; x < m->width; x++) {
            path_node_t *n = &nodes[x][y];
            if (n->open && !n->closed) {
                if (!current || n->fCost < current->fCost)
                    current = n;
            }
        }

        if (!current) {
            return false; // no path
        }

        // Reached target
        if (current == goal)
            break;

        current->open = false;
        current->closed = true;

        // Step 2: Explore neighbors
        const int dirs[4][2] = {
            { 1, 0 }, {-1, 0},
            { 0, 1 }, { 0,-1}
        };

        for (int i = 0; i < 4; i++)
        {
            int nx = current->x + dirs[i][0];
            int ny = current->y + dirs[i][1];

            if (!InBounds(m, nx, ny)) continue;
            if (TileBlocksMovement(&m->tiles[nx][ny])) continue;

            path_node_t *neighbor = &nodes[nx][ny];
            if (neighbor->closed) continue;

            int cost = current->gCost + 1;

            if (!neighbor->open || cost < neighbor->gCost) {
                neighbor->gCost = cost;
                neighbor->hCost = Heuristic(nx, ny, tx, ty);
                neighbor->fCost = neighbor->gCost + neighbor->hCost;

                neighbor->parentX = current->x;
                neighbor->parentY = current->y;

                neighbor->open = true;
            }
        }
    }

    // Step 3: Backtrack from goal to start to find next step
    path_node_t *node = goal;

    while (node->parentX != sx || node->parentY != sy) {
        if (node->parentX < 0 || node->parentY < 0)
            return false;
        node = &nodes[node->parentX][node->parentY];
    }

    // Write next step
    outNextStep->x = node->x;
    outNextStep->y = node->y;

    return true;
}

void RoomSpawnMob(map_grid_t* m, room_t* r){
  Cell pos = r->center;
  
  for(int i = 0; i < r->num_mobs; i++){
    r->mobs[i]->pos = pos;
    if(RegisterEnt(r->mobs[i])){
      r->mobs[i]->map = m; 
      SetState(r->mobs[i],STATE_SPAWN,NULL);
    }

    if(i%2==0)
      pos.x++;
    else
      pos.y++;
  }
}

void MapSpawnMob(map_grid_t* m, int x, int y){
/*
  int total = 0;
  int count = MAPS[m->id].num_mobs;
  spawn_rules_t* spawn = MAPS[m->id].mobs;
  Cell pos = CELL_NEW(x,y);
  for (int i = 0; i < count; i++){
    if(spawn[i].mob == ENT_DONE)
      break;
    total+=spawn[i].weight;
  }

  int r = rand() % total;

  for (int i = 0; i < count; i++){
    if(spawn[i].mob == ENT_DONE)
      break;

    if(r<spawn[i].weight){
      ObjectInstance mob = GetEntityData(spawn[i].mob);
      RegisterEnt(InitMob(mob.id,pos));
      break;
    }

    r -= spawn[i].weight;
  }
  */
}

void MapSpawn(TileFlags flags, int x, int y){

  if(flags == TILEFLAG_EMPTY)
    return;

  EnvTile t = GetTileByFlags(flags|world_map.map_rules->map_flag);

  Cell pos = {x,y};
  for (int i = 0; i < markers; i++){
    if(mark_spr[i]->tag == t)
      return;
  }

  mark_spr[markers++] = InitSpriteByID(t, SHEET_ENV);
  mark_spr[markers-1]->is_visible = true;
  //RegisterEnv(InitEnv(t,pos));

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

map_node_t* MapBuildRootPipeline(void){
}

MapNodeResult MapGenInit(map_context_t *ctx, map_node_t *node){
  map_pool.count = 0;
  ctx->height = 100;
  ctx->width = 100;

  map_gen_t* gen = ctx->map_rules;

  return MAP_NODE_SUCCESS;
}

MapNodeResult MapGenScan(map_context_t *ctx, map_node_t *node){
  RoomFlags variations[] = {
    ROOM_LAYOUT_ROOM | ROOM_SIZE_SMALL  | ROOM_PURPOSE_SIMPLE | ROOM_SHAPE_SQUARE,
    ROOM_LAYOUT_ROOM | ROOM_SIZE_MEDIUM | ROOM_PURPOSE_SIMPLE   | ROOM_SHAPE_RECT,
    ROOM_LAYOUT_ROOM | ROOM_SIZE_MEDIUM | ROOM_PURPOSE_CHALLENGE | ROOM_SHAPE_SQUARE,
    ROOM_LAYOUT_ROOM | ROOM_SIZE_LARGE  | ROOM_PURPOSE_CHALLENGE | ROOM_SHAPE_CIRCLE,
    ROOM_LAYOUT_ROOM | ROOM_SIZE_SMALL  | ROOM_PURPOSE_SIMPLE | ROOM_SHAPE_CIRCLE
  };

  map_gen_t* gen = ctx->map_rules; 
/*
  for(int y = 0; y < gen->height; y++){
    for(int x = 0; x; gen->width; x++){
      int i = RandRange(0,4);
      option_t opt = TryRoomOption(gen, variations[i], x, y);
      if (opt.score > 51+APPLIED)
        MapAddOption(opt);

      continue;
      // Example: corridor option
      option_t hall = TryCorridorOption(ctx, x, y);
      if (hall.score > 0) MapAddOption(hall);
    }
  }
  // More options later...
*/
  
  return MAP_NODE_SUCCESS;
}

MapNodeResult MapGenRun(map_context_t *ctx, map_node_t *node){
  MapGenScan(ctx, node);
  for (int iter = 0; iter < 1000; iter++) {

    option_t *best = PickBestOption(ctx);
    if (!best) continue;

    // Apply top-scoring candidate
    best->apply(best, ctx->map_rules);
    APPLIED++;
    // Rebuild candidate pool
    RefreshOptionPool(ctx, node);
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
    conn->status = RS_GTG;
    return false;  
  }

  bool f_closed = room_is_enclosed(ctx, conn->from->bounds);
  bool t_closed = room_is_enclosed(ctx, conn->to->bounds);

  cell_bounds_t d_pos = *conn->range;
  if(t_closed || f_closed){
    conn->status = RS_WHERE_IS_THE_DOOR;
    return true;
  }

  Cell tblocker = CELL_UNSET;
  Cell fblocker = CELL_UNSET;
  bool t_path = CheckPath(ctx,conn->to->bounds.center,d_pos.center, &tblocker);
  bool f_path = CheckPath(ctx,conn->from->bounds.center,d_pos.center, &fblocker);


  if(f_path && t_path){
    Rectangle f_rec = RecFromBounds(&conn->from->bounds);
    Rectangle t_rec = RecFromBounds(&conn->to->bounds);
    bool f_door_inside = cell_in_rect(d_pos.center, f_rec);
    bool t_door_inside = cell_in_rect(d_pos.center, t_rec);
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

  cell_bounds_t d_pos = *conn->range;
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
  Rectangle d_rec = RecFromBounds(conn->range);
  bool rooms_align = cells_linear(f_center,t_center);
  bool door_from = cells_linear(f_center,conn->range->center);
  bool door_to = cells_linear(t_center,conn->range->center);

  if(rooms_align && door_from && door_to){
    conn->status = RS_RESOLVED;
    return false;
  }

  return true;
}

bool RoomFindPath(map_context_t *ctx, room_connection_t* conn){
  cell_bounds_t d_pos = *conn->range;
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
  bool f_closed = room_is_enclosed(ctx, conn->from->bounds);
  bool t_closed = room_is_enclosed(ctx, conn->to->bounds);

  cell_bounds_t d_pos = *conn->range;
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
  Rectangle d_rec = RecFromBounds(conn->range);
  bool rooms_align = cells_linear(f_center,t_center);
  bool door_from = cells_linear(f_center,conn->range->center);
  bool door_to = cells_linear(t_center,conn->range->center);

  if(rooms_align && door_from && door_to){
    conn->status = RS_RESOLVED;
    return false;
  }


  return true;

}

MapNodeResult MapFixIssues(map_context_t *ctx, map_node_t *node){
  MapNodeResult result = MAP_NODE_SUCCESS;
  int num_issues = 0;  
  for (int i = 0; i < ctx->num_issues; i++){
    room_connection_t* conn = ctx->issues[i];
    if(conn == NULL)
      continue;

    bool issue = false;
    switch(conn->status){
      case RS_GTG:
        conn = NULL;
        break;
      case RS_CHECK:
        issue = RoomCheckPaths(ctx, conn);
        break;
      case RS_WHERE_IS_THE_DOOR:
        issue = RoomFindDoor(ctx,conn);
        break;
      case RS_NO_PATH:
        issue = RoomFindPath(ctx,conn);
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
      ctx->issues[num_issues++] = conn;
    }
  }

  ctx->num_issues = num_issues;

  if(FIX_ATTEMPTS > MAX_ATTEMPTS)
    result = MAP_NODE_SUCCESS;
  return result;//MAP_NODE_SUCCESS;
}

MapNodeResult MapCheckPaths(map_context_t *ctx, map_node_t *node){
  MapNodeResult result = MAP_NODE_SUCCESS;
  int num_connections = 0;  
  for (int i = 0; i < ctx->num_conn; i++){
    room_connection_t* conn = ctx->connections[i];
    if(conn == NULL)
      continue;

    bool issue = false;
    switch(conn->status){
      case RS_GTG:
        conn = NULL;
        break;
      case RS_CHECK:
        issue = RoomCheckPaths(ctx, conn);
        break;
      case RS_WHERE_IS_THE_DOOR:
        issue = RoomFindDoor(ctx,conn);
        break;
      case RS_NO_PATH:
        issue = RoomFindPath(ctx,conn);
        break;
      default:
        DO_NOTHING();
        break;
    }
    if(issue){
      result = MAP_NODE_RUNNING;
      ctx->issues[ctx->num_issues++] = conn;
      ctx->connections[i] = NULL;
    }
    else{
      ctx->connections[num_connections++] = conn;
    }
  
  }

  ctx->num_conn = num_connections;

  return result;//MAP_NODE_SUCCESS;
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

    /*bool touching = GetRectOverlap(r_bounds, c_bounds, &overlap);

    int space = Vector2Length(overlap);

    if(space==0)
      continue;
    
    Rectangle o_rec = GetIntersectionRec(r_bounds,c_bounds);  
*/
  }
}

MapNodeResult MapAlignNodes(map_context_t *ctx, map_node_t *node){
  for(int i = 0; i < ctx->num_rooms; i++){
    room_node_t* r = ctx->anchors[i];

    AlignNodeConnections(ctx, r);
   
  }

  return MAP_NODE_SUCCESS;
}

MapNodeResult MapGridLayout(map_context_t *ctx, map_node_t *node){
  Cell prev_pos = CELL_EMPTY;
  Cell border =CELL_NEW(4,4);//ctx->map_rules->border,ctx->map_rules->border);
  Cell spacing =CELL_NEW(1,1);//ctx->map_rules->spacing,ctx->map_rules->spacing);
  Cell prev_dis = border;
  int num_root_rooms = 0;
  for (int i = 0 ; i < ctx->num_rooms; i++){
    room_t *room = ctx->rooms[i];
    RoomFlags purpose = room->flags & ROOM_PURPOSE_MASK;

    if(purpose!=ROOM_PURPOSE_CONNECT && purpose!=ROOM_PURPOSE_START)
      continue;

    Cell size = RoomSize(room);
    room->dir = RoomFlagsGetPlacing(room->flags);
    room->center = CellInc(CellMul(room->dir,CellInc(CellScale(size,1),prev_dis)),prev_pos);

    Cell dir = (i>0)?cell_dir(room->center,prev_pos):room->dir;
    Cell door_pos = CellInc(CellMul(dir,size),room->center);
    prev_dis = CellInc(size,CellScale(spacing,0.5));
    /*
    if(purpose!=ROOM_PURPOSE_START)
    room->openings[room->num_children++] = (room_opening_t){
      .entrance = true,
      .pos = door_pos,
      .dir = room->dir
    };
*/
    prev_pos = room->center;
    
    if(i>0)
      door_pos = CELL_UNSET;

/*
    for(int j = i+1; j < ctx->num_rooms; j++){
      room->openings[room->num_children++] = (room_opening_t)
      {
        .dir = RoomFlagsGetPlacing(ctx->rooms[j]->flags),
          .pos = door_pos,
          .sub = ctx->rooms[j],
      };

      if((ctx->rooms[j]->flags&ROOM_PURPOSE_MASK) == ROOM_PURPOSE_CONNECT)
        break;
    }
*/
    ctx->rooms[num_root_rooms++] = room;

    sleep(2);
  }

  if(num_root_rooms > 0){
    ctx->num_rooms = num_root_rooms;
    return MAP_NODE_SUCCESS;
  }

  return MAP_NODE_FAILURE;
}

bool AdjustChildNodePosition(room_node_t* root, room_node_t* child, node_connector_t* conn, Cell dir){
  Cell c_size = CellScale(child->size,0.5);

  Cell disp = CellMul(dir,c_size);
  Cell og_center = child->center;
  child->center = conn->range.center;

  cell_bounds_t og_bounds = child->bounds;
  child->bounds = RoomBounds(child->flags,child->center);

  Vector2 overlap;
  Rectangle p_rec = RecFromBounds(&root->bounds);
  while(GetRectOverlap(RecFromBounds(&child->bounds),p_rec,&overlap)){
    Cell shift = CellMul(conn->dir,overlap);
    child->center = CellInc(child->center,shift);
    child->bounds = RoomBounds(child->flags,child->center);
  } 

  Cell trans = CellSub(child->center, og_center);
   if(MapCheckOverlap(RecFromBounds(&child->bounds),&overlap)){
    child->bounds = og_bounds;
    child->center = og_center;
    return false;
  }

   int best = -1;
  int margin = world_map.map_rules->margin_error;
  for(int j = 0; j < child->max_children; j++){
    child->children[j]->range =  AdjustCellBounds(&child->children[j]->range,trans);
    if(!cell_compare(CellInc(conn->dir,child->children[j]->dir),CELL_EMPTY))
      continue;

    Cell o_dist = cell_along_rect(child->children[j]->range.center,p_rec);
    if(CELL_LEN(o_dist) < margin){
      margin = CELL_LEN(o_dist);
      best = j;
    }
  }
  if(best>-1){
   child->entrance_index = best; 
    child->children[best]->used = true;
    child->children[best]->enter = true;
  }
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

bool MapAddNodeToConnector(node_connector_t* conn, room_node_t* node){

  if(conn->room || conn->used || !conn->owner)
    return false;

  if(!AdjustChildNodePosition(conn->owner, node, conn,conn->dir))
    return false;

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
  //root->children[root->num_children] = calloc(1,sizeof(room_node_t));

  for(int i = 0; i < root->max_children; i++){
    if(root->children[i]->used)
      continue;

    Cell dir = RoomFacingFromFlag(child->flags);

    if(!cell_compare(dir,root->children[i]->dir))
      continue;
   
    node_connector_t* conn = root->children[i]; 

    if(!AdjustChildNodePosition(root, child, conn,conn->dir))
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
  room_node_t* node = calloc(1,sizeof(room_node_t));

  *node = (room_node_t){
    .center = pos,
    .flags = flags,
    .children = malloc(sizeof(room_node_t)),
    .entrance_index = -1
  };

  if(GetRoomPurpose(flags) == ROOM_PURPOSE_CONNECT)
    node->is_root = true;

  node->bounds = RoomBounds(flags,pos);

  node->size = CellSub(node->bounds.max,node->bounds.min); 
  node->max_children = RoomConnectionsFromFlags(node, node->children); 
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
    case ROOM_PURPOSE_TREASURE_FALSE:
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
    case ROOM_PURPOSE_TREASURE_FALSE:
    if(GetRoomPurpose(pflags) > ROOM_PURPOSE_TREASURE_FALSE)
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
    if(node->is_root && (node->total >= node->cap))
      return false;

    RoomFlags test_flags = LayoutByWeight(ROOM_LAYOUT_MAX,99) | RandomShape() | SizeByWeight(ROOM_SIZE_MAX,67) | PurposeByWeight(ROOM_PURPOSE_START,67);

     node_option_t opt = TryNodeOption(node, node->children[i], node->flags, test_flags);

     if(opt.score > 50)
       MapAddNodeOption(opt);

     if(node->children[i]->used && node->children[i]->room)
       NodeFindOptions(node->children[i]->room,false);
  }

  return true;
}

MapNodeResult MapRoomNodeScan(map_context_t *ctx, map_node_t *node){
  for(int i = 0; i < ctx->num_rooms; i++)  
    NodeFindOptions(ctx->anchors[i],true);

  bool applied = false;
  int attempts = 0;
    usleep(90000);
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

MapNodeResult MapGenerateRooms(map_context_t *ctx, map_node_t *node){
  map_gen_t *gen = ctx->map_rules;

  Color col[MAX_ANCHOR_NODES]={
    BLUE,
    PURPLE,
    GREEN,
    PINK,
    YELLOW,
    MAGENTA,
    BLACK,
    BROWN
  };

  for (int i = 0; i < gen->num_rooms; i++){
    room_gen_t* rgen = &gen->rooms[i];
    RoomFlags flags = rgen->flags;
    Cell pos = CELL_NEW(rgen->col,rgen->row);
    room_node_t* root = MapBuildNode(flags, pos);
    root->is_root = true;
    root->col = col[i];
    root->cap = rgen->cap;
    root->section = root->bounds;
    CAP+=root->cap;
    ctx->room_bounds[world_map.num_bounds++] = RecFromBounds(&root->bounds);
    NUM_ANCHORS++;
    ctx->anchors[ctx->num_rooms++] = root;
  }

  return ctx->num_rooms>0 ? MAP_NODE_SUCCESS : MAP_NODE_FAILURE;
}

MapNodeResult MapGraphRooms(map_context_t *ctx, map_node_t *node) {  
  return MAP_NODE_SUCCESS;
}

void CarveHallBetween(map_context_t *ctx, Cell a, Cell b) {
  Cell cur = a;

  while (!(cur.x == b.x && cur.y == b.y)) {
    int dx = b.x - cur.x;
    int dy = b.y - cur.y;

    Cell step = { 0, 0 };
    if (abs(dx) > abs(dy))
      step.x = (dx > 0) ? 1 : -1;
    else
      step.y = (dy > 0) ? 1 : -1;

    cur.x += step.x;
    cur.y += step.y;

    if (cur.x < 0 || cur.y < 0 ||
        cur.x >= ctx->width || cur.y >= ctx->height)
      break;

    ctx->tiles[cur.x][cur.y] = RandRange(0,100)<ctx->decor_density?TILEFLAG_FLOOR:TILEFLAG_EMPTY;
  }
}

MapNodeResult MapGenConnectRoots(map_context_t *ctx, map_node_t *node) {
  for (int i = 1; i < ctx->map_rules->num_rooms; i++) {
/*
    Cell dir = CELL_EMPTY;
    switch(GetRoomPlacing(flags)){
      case ROOM_PLACING_N:
        dir = CELL_UP;
        break;
      case ROOM_PLACING_S:
        dir = CELL_DOWN;
        break;
      case ROOM_PLACING_E:
        dir = CELL_RIGHT;
        break;
      case ROOM_PLACING_W:
        dir = CELL_LEFT;
        break;
    }



    Cell dist = cell_dist(root->top_left,next->top_left);
*/

  }

  return MAP_NODE_SUCCESS;
}


MapNodeResult MapConnectSubrooms(map_context_t *ctx, map_node_t *node) {
  for (int i = 0; i < ctx->num_rooms; i++) {
    room_t *root = ctx->rooms[i];
    for (int k = 0; k < root->num_children; k++){
      if(root->openings[k]->sub == NULL)
        continue;

      room_t *sub = root->openings[k]->sub;

      Cell r_ent = root->openings[k]->pos;
      Cell s_ent = sub->openings[0]->pos;
      CarveHallBetween(ctx, s_ent, r_ent);
    }
  }
  return MAP_NODE_SUCCESS;
}

void RoomCarveTiles(map_context_t* ctx, room_t* r){
  for (int y = r->bounds.min.y; y <= r->bounds.max.y; y++) {
    for (int x = r->bounds.min.x; x <= r->bounds.max.x; x++) {
      bool border = (x == r->bounds.min.x ||
          x == r->bounds.max.x ||
          y == r->bounds.min.y ||
          y == r->bounds.max.y);
      TileFlags floor = RandRange(0,100)<ctx->decor_density?TILEFLAG_FLOOR:TILEFLAG_EMPTY;
      ctx->tiles[x][y] = border ? TILEFLAG_WALL : floor;
    }
  }
  for(int i = 0; i < r->num_children; i++){
    Cell pos = r->openings[i]->pos;
    TileFlags opening = TILEFLAG_EMPTY;
    if(r->openings[i]->entrance)
      opening = TILEFLAG_DOOR;

    ctx->tiles[pos.x][pos.y] = opening;
    if(r->openings[i]->sub)
      RoomCarveTiles(ctx,r->openings[i]->sub);
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


  // halls already carved as floor in NodeConnectHalls
  return MAP_NODE_SUCCESS;
}

bool RoomPlaceSpawns(map_context_t *ctx, room_t *r){
  if(r->num_mobs > 0)
    return true;

  RoomFlags purpose = r->flags & ROOM_PURPOSE_MASK;
 
  int num_spawns = 0; 
  if(purpose == ROOM_PURPOSE_START)
    return true;

  RoomFlags size = r->flags & ROOM_SIZE_MASK;
  RoomFlags layout = r->flags & ROOM_LAYOUT_MASK;
  RoomFlags orient = r->flags & ROOM_ORIENT_MASK;
  RoomFlags shape = r->flags & ROOM_SHAPE_MASK;

  MobRules mob_rules = GetMobRulesByMask(ctx->map_rules->mobs.rules,MOB_LOC_MASK);
  mob_define_t mob_pool[MOB_MAP_MAX];
  int mobs = FilterMobsByRules(mob_rules,mob_pool);
 

  MobRules theme = GetMobRulesByMask(ctx->map_rules->mobs.rules, MOB_THEME_MASK);

  mob_rules |= theme;
  
  switch(purpose){
    case ROOM_PURPOSE_CHALLENGE:
      mob_rules |= MOB_SPAWN_CHALLENGE;
      break;
    case ROOM_PURPOSE_LAIR:
      mob_rules |= MOB_SPAWN_LAIR;
      break;
    case ROOM_PURPOSE_CONNECT:
      mob_rules |= MOB_SPAWN_PATROL;
    default:
      break;
  }

  switch(layout){
    case ROOM_LAYOUT_OPEN:
      mob_rules |= ( MOB_MOD_ENLARGE | MOB_SPAWN_CAMP );
      num_spawns++;
      break;
    case ROOM_LAYOUT_HALL:
      num_spawns--;
      mob_rules &= ~MOB_SPAWN_LAIR;
      break;
    case ROOM_LAYOUT_MAZE:
      num_spawns++;
      mob_rules |= MOB_SPAWN_PATROL;
    default:
      break;
  }

    
  MobRules grouping = GetMobRulesByMask(ctx->map_rules->mobs.rules, MOB_GROUPING_MASK);
  num_spawns+=size>>12;

  if(num_spawns < 9)
    grouping &= ~MOB_GROUPING_WARBAND;

  if(num_spawns < 7)
    grouping &= ~MOB_GROUPING_SQUAD;

  if(num_spawns < 5)
    grouping &= ~MOB_GROUPING_CREW;
  
  if(num_spawns < 3)
    grouping &= ~MOB_GROUPING_PARTY;

  if(num_spawns <= 2)
    grouping &= ~MOB_GROUPING_TROOP;


  
  int filtered = 0;
 
  for (int i = 0; i < mobs; i++){
    if(!MobHasAnyRules(mob_pool[i].id,grouping))
      continue;

    if(!MobHasAnyRules(mob_pool[i].id,theme))
      continue;

    mob_pool[filtered++] = mob_pool[i];
  }
  
  mob_rules |= GetMobRulesByMask(ctx->map_rules->mobs.rules, MOB_FREQ_MASK);
  mob_rules |= GetMobRulesByMask(ctx->map_rules->mobs.rules,MOB_MOD_MASK);
  mob_rules |= grouping;
  int built = 0;

  for(int i = 0; i < filtered; i++){
   if(mob_pool[i].weight > num_spawns)
    continue;

    built = EntBuild(mob_pool[i],mob_rules,r->mobs);
    r->num_mobs+=built;
    break;
  }

  return (built>0);
}

MapNodeResult MapPlaceSpawns(map_context_t *ctx, map_node_t *node) {
  for (int r = 0; r < ctx->num_rooms; r++){
    RoomPlaceSpawns(ctx, ctx->rooms[r]);
    for (int i = 0; i < ctx->rooms[r]->num_children; i++){
      if(ctx->rooms[r]->openings[i]->sub == NULL)
        continue;

       RoomPlaceSpawns(ctx, ctx->rooms[r]->openings[i]->sub);

    }
  }
    return MAP_NODE_SUCCESS;
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


void RoomAdjustPosition(room_t *r, Cell disp){

  r->center = CellInc(r->center,disp);

  r->ref->center = r->center;
  r->bounds = RoomBounds(r->flags, r->center);

  r->ref->bounds = r->bounds;

  for(int i = 0; i < r->num_children; i++){
    r->openings[i]->pos = CellInc(r->openings[i]->pos,disp);
  }
/*
    if(!r->openings[i]->sub)
      continue;

    if((r->openings[i]->sub->flags&ROOM_PURPOSE_MASK) == ROOM_PURPOSE_CONNECT)
      continue;
       RoomAdjustPosition(r->openings[i]->sub, disp);
  }
*/
}

MapNodeResult MapPlaceSubrooms(map_context_t *ctx, map_node_t *node) {
/*
  for (int i = 0; i < ctx->num_rooms; i++){
    room_t *root = ctx->rooms[i];
    cell_bounds_t bounds = root->bounds;
    Cell prev_n = CELL_NEW(bounds.min.x,bounds.min.y);
    Cell prev_s = CELL_NEW(bounds.min.x,bounds.max.y);
    Cell prev_e = CELL_NEW(bounds.max.x,bounds.min.y);
    Cell prev_w = CELL_NEW(bounds.min.x,bounds.min.y);

    for(int k = 0; k < root->num_children; k++){
      if(!root->openings[k].sub)
        continue;

      room_t* sub = root->openings[k].sub;

      if((sub->flags&ROOM_PURPOSE_MASK) == ROOM_PURPOSE_CONNECT)
        continue;

      RoomFlags s_dir = sub->flags&ROOM_PLACING_MASK;

      Cell o_dir = root->openings[k].dir;

      Cell subDim = RoomSize(sub);
      
      if(cell_compare(root->openings[k].dir,CELL_UNSET))
        root->openings[k].dir = CellFlip(root->dir);

      Cell pos = root->openings[k].pos;
      if(cell_compare(root->openings[k].pos,CELL_UNSET)){
        pos = RoomGetBestOpening(root,sub, k);

        switch(s_dir){
          case ROOM_PLACING_N:
            pos.x += prev_n.x+subDim.x;
            pos.y=prev_n.y;
            prev_n.x = pos.x + subDim.x;
            break;
          case ROOM_PLACING_S:
            pos.x+=subDim.x + prev_s.x;
            pos.y=prev_s.y;
            prev_s.x  = pos.x + subDim.x;
            break;
          case ROOM_PLACING_E:
            pos.x= prev_e.x;
            prev_e = pos;
            break;
          case ROOM_PLACING_W:
            pos.y+=subDim.y;
            prev_w = pos;
            break;
          case ROOM_PLACING_NW:

            break;
          case ROOM_PLACING_NE:

            break;
          case ROOM_PLACING_SE:

            break; 
          case ROOM_PLACING_SW:

            break;
          default:
            break;
        }
        root->openings[k].pos = pos;
      }

      Cell opening = CellScale(o_dir,2);//ctx->map_rules->spacing);
      Cell disp = CellInc(CellMul(subDim,o_dir),opening);
      opening = CellInc(opening,pos);
      sub->center = CELL_EMPTY;
      RoomAdjustPosition(sub,CellInc(root->openings[k].pos,disp));
      sub->openings[sub->num_children++] = (room_opening_t){
          .entrance = true,
          .pos = opening,
          .dir = cell_dir(sub->center, opening)
      };
    }
  }
*/
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

  // --- 2. Compute width + height of final map ---
  ctx->width  = abs(maxx) + abs(minx);// + ctx->map_rules->border;
  ctx->height = abs(maxy) + abs(miny);// + ctx->map_rules->border;;


  TraceLog(LOG_INFO,"\n=====MAP SIZE ===\n===== WIDTH %i====\n====HEIGHT %i ====\n",ctx->width,ctx->height);

  Cell offset = CELL_NEW(-minx,-miny);

  for (int i = 0; i < ctx->num_rooms; i++) {

    usleep(400000);

    RoomAdjustPosition(ctx->rooms[i],offset);

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
    ctx->tiles = malloc(ctx->width * sizeof(TileFlags*));
    if (!ctx->tiles)
        return MAP_NODE_FAILURE;

    for (int x = 0; x < ctx->width; x++) {
        ctx->tiles[x] = calloc(ctx->height, sizeof(TileFlags));
        if (!ctx->tiles[x])
            return MAP_NODE_FAILURE;
    }

    ctx->alloc_w = ctx->width;
    ctx->alloc_h = ctx->height;

    return MAP_NODE_SUCCESS;
}

MapNodeResult MapPlayerSpawn(map_context_t *ctx, map_node_t *node){
  for(int i = 0; i < ctx->num_rooms; i++){
    /*
    RoomFlags purpose = ctx->rooms[i].flags&ROOM_PURPOSE_MASK;
    if(purpose!= ROOM_PURPOSE_START)
      continue;

    //ctx->player_start = ctx->rooms[i]->center;
    return MAP_NODE_SUCCESS;
*/
  }

  return MAP_NODE_FAILURE;
}

MapNodeResult MapFillWalls(map_context_t *ctx, map_node_t *node){
  for(int x = 0; x<ctx->width; x++){
    for (int y = 0; y < ctx->height; y++){
      TileFlags *tile = &ctx->tiles[x][y];

      if (!TileHasFlag(*tile, TILEFLAG_EMPTY) || !TileHasFlag(*tile,TILEFLAG_FLOOR))
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

option_t TryRoomOption(map_gen_t *map, RoomFlags flags, int x, int y){
  option_t opt = {0};
  opt.type = OPT_ROOM;
  opt.flags = flags;
  // room size


  RoomDimensionsFromFlags(flags, &opt.w, &opt.h);
  opt.pos = CELL_NEW(x,y);

  // HEURISTIC SCORE
  opt.score = 10;

  switch (GetRoomShape(flags)) {
    case ROOM_SHAPE_CIRCLE: opt.score += 6; break;
    case ROOM_SHAPE_SQUARE:  opt.score += 9; break;
    case ROOM_SHAPE_RECT: opt.score += 12; break;
    default: opt.score+=5;  break;
  }
  // purpose-specific
  switch (GetRoomPurpose(flags)) {
    case ROOM_PURPOSE_SIMPLE:
      opt.score += 18; // valuable
      break;
    case ROOM_PURPOSE_CHALLENGE:
      opt.score += 14; // leads to big fights
      break;
    case ROOM_PURPOSE_LAIR:
      opt.score -= 5;
      break;
    case ROOM_PURPOSE_SECRET:
      //if (!HasFloorNeighbor(map, x, y)) opt.score += 30;
      break;
  }
  
  opt.apply = ApplyRoom;

  return opt;
}

void MapAddOption(option_t opt){ 
  if (map_pool.count < MAX_OPTIONS) {
    map_pool.items[map_pool.count++] = opt;
  }
}

void MapAddNodeOption(node_option_t opt){ 
  if (NODE_OPTIONS.count < MAX_OPTIONS) {
    NODE_OPTIONS.items[NODE_OPTIONS.count++] = opt;
  }
}

option_t TryCorridorOption(map_context_t *map, int x, int y){

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

void ApplyRoomCircle(option_t *opt, map_context_t *map){
  int cx = opt->pos.x + opt->w / 2;
  int cy = opt->pos.y + opt->h / 2;

  int radius = (opt->w < opt->h ? opt->w : opt->h) / 2;
  int r2 = radius * radius;

  for (int y = 0; y < opt->h; y++) {
    for (int x = 0; x < opt->w; x++) {

      int gx = opt->pos.x + x;
      int gy = opt->pos.y + y;

      int dx = gx - cx;
      int dy = gy - cy;

      if (dx*dx + dy*dy <= r2) {
        map->tiles[gy][gx] = TILEFLAG_FLOOR;
      }
    }
  }
}
  
void ApplyRoomRect(option_t *opt, map_context_t *map){
  for (int y = 0; y < opt->h; y++) {
    for (int x = 0; x < opt->w; x++) {
      map->tiles[opt->pos.y + y][opt->pos.x + x] = TILEFLAG_FLOOR;
    }
  }
}

void ApplyRoom(option_t *opt, map_gen_t *map){
  /*
  switch(GetRoomShape(opt->flags)){
    case ROOM_SHAPE_RECT:
    case ROOM_SHAPE_SQUARE:
      ApplyRoomRect(opt, map);
      break;
    case ROOM_SHAPE_CIRCLE:
      ApplyRoomCircle(opt, map);
      break;
    default:
      ApplyRoomRect(opt, map);
      break;
  }
  */
}

bool ApplyNode(node_option_t *opt){
  return  MapAddNodeToConnector(opt->connect,MapBuildNode(opt->flags, CELL_EMPTY));
}

void RefreshNodeOptions(void){
  NODE_OPTIONS.count = 0;
}


void RefreshOptionPool(map_context_t *context, map_node_t* node){
  map_pool.count = 0;
  MapGenScan(context, node);
}

option_t *PickBestOption(map_context_t *context){
  if (map_pool.count == 0) return NULL;

  int best_index = 0;

  for (int i = 1; i < map_pool.count; i++) {
    if (map_pool.items[i].score > map_pool.items[best_index].score) {
      best_index = i;
    }
  }

  return &map_pool.items[best_index];
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

bool room_has_access(map_context_t* ctx, cell_bounds_t room){
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

            if (TileFlagHasAccess(ctx->tiles[x][y]))
              return true;

        }
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

