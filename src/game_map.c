#include "game_process.h"
#include "game_helpers.h"
#include <limits.h>
static map_context_t world_map;

bool InitMap(void){
  map_context_t* ctx =&world_map;
  memset(ctx, 0, sizeof(*ctx));
  ctx->map_rules = malloc(sizeof(map_gen_t));
  *ctx->map_rules = MAPS[DANK_DUNGEON];

  ctx->decor_density = 12;  
  bool gen = MapGenerate(ctx);

  return gen;
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
  //MapApplyContext(m);
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

void MapApplyContext(map_grid_t* m){
  m->tiles = malloc(world_map.width * sizeof(map_cell_t*));

  for(int x = 0; x < world_map.width; x++){
    m->tiles[x] = calloc(m->height, sizeof(map_cell_t));
    for(int y = 0; y < world_map.height; y++){
     m->tiles[x][y].coords = CELL_NEW(x,y); 
      MapSpawn(world_map.tiles[x][y],x,y);
    }
  }
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

bool FindPath(map_grid_t *m, int sx, int sy, int tx, int ty, Cell *outNextStep){

}

void MapSpawn(TileFlags flags, int x, int y){

  if(flags == TILEFLAG_EMPTY)
    return;

  EnvTile t = GetTileByFlags(flags|world_map.map_rules->map_flag);

  Cell pos = {x,y};
  RegisterEnv(InitEnv(t,pos));

}

MapNodeResult MapNodeRunSequence(map_context_t *ctx, map_node_t *node){
  for (int i = 0; i < node->num_children; i++) {
    TraceLog(LOG_INFO,"Run sequence %i of type %i",node->id,node->type);
    MapNodeResult r = node->children[i]->run(ctx, node->children[i]);
    if (r != MAP_NODE_SUCCESS)
      return r;
  }
  return MAP_NODE_SUCCESS;
}

map_node_t* MapBuildRootPipeline(void){
  static map_node_t seq;

  static map_node_t n_genRooms;
  static map_node_t n_place;
  static map_node_t n_shape;
  static map_node_t n_bounds;
  static map_node_t n_allocate;
  static map_node_t n_graph;
  static map_node_t n_connect;
  static map_node_t n_carve;
  static map_node_t n_spawns;
  static map_node_t n_decor;

//  n_genRooms.type = MAP_NODE_GENERATE_ROOMS;
  //n_genRooms.run  = NodeGenerateRooms;
/*
  n_place.type = MAP_NODE_PLACE_ROOMS;
  n_place.run  = NodeAssignPositions;
  
  n_shape.type = MAP_NODE_APPLY_SHAPES;
  n_shape.run = NodeApplyShapes;

  //n_bounds.type = MAP_NODE_COMPUTE_BOUNDS;
  //n_bounds.run = NodeComputeBounds;
  n_allocate.type = MAP_NODE_ALLOCATE_TILE_GRID;
  n_allocate.run = NodeAllocateTiles;
  n_graph.type = MAP_NODE_BUILD_GRAPH;
  n_graph.run  = NodeBuildRoomGraph;
*/

  n_connect.type = MAP_NODE_CONNECT_HALLS;
  n_connect.run  = NodeConnectHalls;
/*
  n_carve.type = MAP_NODE_CARVE_TO_TILES;
  n_carve.run  = NodeCarveToTiles;
*/
  n_spawns.type = MAP_NODE_PLACE_SPAWNS;
  n_spawns.run  = NodePlaceSpawns;

  n_decor.type = MAP_NODE_DECORATE;
  n_decor.run  = NodeDecorate;

  seq.type = MAP_NODE_SEQUENCE;
  seq.run  = MapNodeRunSequence;
  seq.num_children = 10;
  seq.children[0] = &n_genRooms;
  seq.children[1] = &n_place;
  seq.children[2] = &n_shape;
  seq.children[3] = &n_bounds;
  seq.children[4] = &n_allocate;
  seq.children[5] = &n_graph;
  seq.children[6] = &n_carve;
  seq.children[7] = &n_connect;
  seq.children[8] = &n_spawns;
  seq.children[9] = &n_decor;

  return &seq;
}

bool MapGenerate(map_context_t *ctx){

  map_node_t *root = MapBuildNodeRules(ctx->map_rules->node_rules);
  return root->run(ctx, root) == MAP_NODE_SUCCESS;
}

MapNodeResult MapFillMissing(map_context_t *ctx, map_node_t *node){
 
  int temp_id = ctx->map_rules->num_rooms-1;

  for(int i = temp_id; i  < ctx->map_rules->max_rooms; i++){
    RoomFlags flags = ctx->map_rules->rooms[i] 
      | RandomPurpose()
      | RandomShape()
      | SizeByWeight(ROOM_SIZE_MAX,69);

    ctx->map_rules->rooms[i] = flags;
    if(i > temp_id)
      ctx->map_rules->num_rooms++;
  }

 return MAP_NODE_SUCCESS;
}

MapNodeResult MapGridLayout(map_context_t *ctx, map_node_t *node){
  Cell prev_pos = CELL_EMPTY;
  Cell border =CELL_NEW(ctx->map_rules->border,ctx->map_rules->border);
  Cell spacing =CELL_NEW(ctx->map_rules->spacing,ctx->map_rules->spacing);
  Cell prev_dis = border;
  int num_root_rooms = 0;
  for (int i = 0 ; i < ctx->num_rooms; i++){
    room_t *room = &ctx->rooms[i];
    RoomFlags purpose = room->flags & ROOM_PURPOSE_MASK;

    if(purpose!=ROOM_PURPOSE_CONNECT && purpose!=ROOM_PURPOSE_START)
      continue;

    Cell size = RoomSize(room->flags);
    room->dir = RoomFlagsGetPlacing(room->flags);
    room->center = CellInc(CellMul(room->dir,CellInc(CellScale(size,0.5),prev_dis)),prev_pos);

    prev_dis = CellInc(size,spacing);
    room->openings[room->num_children++] = (room_opening_t){
      .entrance = true,
      .pos = CellInc(cell_dir(prev_pos,room->center),room->center),
      .dir = cell_dir(room->center,prev_pos)
    };

    prev_pos = room->center;
    
    for(int j = i+1; j < ctx->num_rooms; j++){
      room->openings[room->num_children++] = (room_opening_t)
      {
        .dir = RoomFlagsGetPlacing(ctx->rooms[j].flags),
          .pos = CELL_UNSET,
          .sub = &ctx->rooms[j],
      };

      if((ctx->rooms[j].flags&ROOM_PURPOSE_MASK) == ROOM_PURPOSE_CONNECT)
        break;
    }

    ctx->rooms[num_root_rooms++] = *room;

  }

  if(num_root_rooms > 0){
    ctx->num_rooms = num_root_rooms;
    return MAP_NODE_SUCCESS;
  }

  return MAP_NODE_FAILURE;
}

MapNodeResult MapGenerateRooms(map_context_t *ctx, map_node_t *node){
  int attempts = 0;
  map_gen_t *gen = ctx->map_rules;

  for (int i = 0; i < gen->num_rooms; i++){
    RoomFlags flags = gen->rooms[i];
    room_t *room = &ctx->rooms[ctx->num_rooms++];

    room->flags = flags;
    room->num_spawns = 0;
  }

  return ctx->num_rooms > 0 ? MAP_NODE_SUCCESS : MAP_NODE_FAILURE;
}

MapNodeResult MapGraphRooms(map_context_t *ctx, map_node_t *node) {
  ctx->num_edges = 0;

  for (int i = 0; i < ctx->num_rooms; i++) {
    int best_j = -1;
    int best_d2 = 0;

    for (int j = 0; j < ctx->num_rooms; j++) {
      if (i == j) continue;

      int dx = ctx->rooms[i].center.x - ctx->rooms[j].center.x;
      int dy = ctx->rooms[i].center.y - ctx->rooms[j].center.y;
      int d2 = dx*dx + dy*dy;

      if (best_j < 0 || d2 < best_d2) {
        best_j = j;
        best_d2 = d2;
      }
    }

    if (best_j >= 0 && ctx->num_edges < MAX_EDGES) {
      room_edge_t *e = &ctx->edges[ctx->num_edges++];
      e->a = i;
      e->b = best_j;
      e->length = best_d2;
    }
  }

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

MapNodeResult NodeConnectHalls(map_context_t *ctx, map_node_t *node) {
  for (int i = 0; i < ctx->num_edges; i++) {
    room_t *A = &ctx->rooms[ctx->edges[i].a];
    room_t *B = &ctx->rooms[ctx->edges[i].b];

    CarveHallBetween(ctx, A->center, B->center);
  }
  return MAP_NODE_SUCCESS;
}

MapNodeResult MapCarveTiles(map_context_t *ctx, map_node_t *node) {
  // clear
  for (int y = 0; y < ctx->height; y++)
    for (int x = 0; x < ctx->width; x++)
      ctx->tiles[x][y] = TILEFLAG_BORDER;

  // rooms
  for (int r = 0; r < ctx->num_rooms; r++) {
    room_t *room = &ctx->rooms[r];
    for (int y = room->bounds.min.y; y <= room->bounds.max.y; y++) {
      for (int x = room->bounds.min.x; x <= room->bounds.max.x; x++) {
        bool border = (x == room->bounds.min.x ||
            x == room->bounds.max.x ||
            y == room->bounds.min.y ||
            y == room->bounds.max.y);
        TileFlags floor = RandRange(0,100)<ctx->decor_density?TILEFLAG_FLOOR:TILEFLAG_EMPTY;
        ctx->tiles[x][y] = border ? TILEFLAG_WALL : floor;
      }
    }
    for(int i = 0; i < room->num_children; i++){
      Cell pos = room->openings[i].pos;
      TileFlags opening = TILEFLAG_EMPTY;
      if(room->openings[i].entrance)
        opening = TILEFLAG_DOOR;

      ctx->tiles[pos.x][pos.y] = opening;
    }
  }


  // halls already carved as floor in NodeConnectHalls
  return MAP_NODE_SUCCESS;
}

MapNodeResult NodePlaceSpawns(map_context_t *ctx, map_node_t *node) {
  for (int r = 0; r < ctx->num_rooms; r++) {
    room_t *room = &ctx->rooms[r];
/*
    int max_mobs = 1 + ((room->size >> 12) % 4); // cheap scaling
    room->num_spawns = 0;

    for (int i = 0; i < max_mobs; i++) {
      int x = RandRange(room->bounds.min.x + 1, room->bounds.max.x - 1);
      int y = RandRange(room->bounds.min.y + 1, room->bounds.max.y - 1);

      room->spawns[room->num_spawns++] = (Cell){x,y};
      // optional: mark flag
      ctx->tiles[x][y] = TILEFLAG_SPAWN;
    }
  }
*/
}
  return MAP_NODE_SUCCESS;
}

MapNodeResult NodeDecorate(map_context_t *ctx, map_node_t *node) {

  for (int r = 0; r < ctx->num_rooms; r++) {
    room_t *room = &ctx->rooms[r];
    TileFlags special = RoomSpecialDecor(room->flags & ROOM_PURPOSE_MASK);

    for (int y = room->bounds.min.y; y <= room->bounds.max.y; y++) {
      for (int x = room->bounds.min.x; x <= room->bounds.max.x; x++) {

        TileFlags *tile = &ctx->tiles[x][y];

        // Skip uncarved / invalid tiles
        if (!TileHasFlag(*tile, TILEFLAG_FLOOR))
          continue;

        // Skip tiles with reserved flags
        if (TileHasFlag(*tile, TILEFLAG_SPAWN | TILEFLAG_START|TILEFLAG_BORDER))
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
  r->bounds = RoomBounds(r,r->center);

  return MAP_NODE_SUCCESS;

}

MapNodeResult MapApplyRoomShapes(map_context_t *ctx, map_node_t *node) {

  for (int i = 0; i < ctx->num_rooms; i++) {
    room_t *room = &ctx->rooms[i];
    MapShapeRoom(room);

    for(int k = 0; k < room->num_children; k++){
      if(room->openings[k].sub == NULL)
        continue;

      MapShapeRoom(room->openings[k].sub);
    }
  }

  return MAP_NODE_SUCCESS;

}

MapNodeResult MapPlaceSubrooms(map_context_t *ctx, map_node_t *node) {

  for (int i = 0; i < ctx->num_rooms; i++){
    room_t *root = &ctx->rooms[i];
    for(int k = 0; k < root->num_children; k++){
      if(!root->openings[k].sub)
        continue;

      room_t* sub = root->openings[k].sub;

      if((sub->flags&ROOM_PURPOSE_MASK) == ROOM_PURPOSE_CONNECT)
        continue;

      Cell dim = RoomSize(root->flags);

      if(!cell_compare(root->openings[k].dir,CELL_UNSET))
        root->openings[k].dir = CellFlip(root->dir);

      if(cell_compare(root->openings[k].pos,CELL_UNSET)){
        Cell edge = CellMul(root->dir,root->center);
        Cell opos = cell_point_along(edge,1);

        root->openings[k].pos = CellInc(CellSub(root->center,edge),opos);

      }

      Cell subDim = RoomSize(sub->flags);

      Cell disp = CellMul(subDim,root->openings[k].dir);
      sub->center = CellInc(root->openings[k].pos,disp);

    }

  }

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
     room_t *r = &ctx->rooms[i];
     if (r->bounds.min.x < minx) minx = r->bounds.min.x;
      if (r->bounds.min.y < miny) miny = r->bounds.min.y;
      if (r->bounds.max.x > maxx) maxx = r->bounds.max.x;
      if (r->bounds.max.y > maxy) maxy = r->bounds.max.y;

    for(int k = 0; k < ctx->rooms[i].num_children; k++){
      if(ctx->rooms[i].openings[k].sub == NULL)
        continue;

      r = ctx->rooms[i].openings[k].sub;

      if (r->bounds.min.x < minx) minx = r->bounds.min.x;
      if (r->bounds.min.y < miny) miny = r->bounds.min.y;
      if (r->bounds.max.x > maxx) maxx = r->bounds.max.x;
      if (r->bounds.max.y > maxy) maxy = r->bounds.max.y;
    }
  }

  // Expand slightly if you want padding around the dungeon:
  int padding = ctx->map_rules->border;
  minx -= padding;
  miny -= padding;
  maxx += padding;
  maxy += padding;

  // --- 2. Compute width + height of final map ---
  int width  = abs(maxx) + abs(minx) + 1;
  int height = abs(maxy) + abs(miny) + 1;

  ctx->width  = width;
  ctx->height = height;

  // --- 3. Compute offset so min becomes 0,0 ---
  int offx = -minx;
  int offy = -miny;

  // --- 4. Shift all rooms into tile coordinate space ---
  for (int i = 0; i < ctx->num_rooms; i++) {
    room_t *r = &ctx->rooms[i];

    r->center.x += offx;
    r->center.y += offy;

    r->bounds.min.x += offx;
    r->bounds.max.x += offx;
    r->bounds.min.y += offy;
    r->bounds.max.y += offy;

    // Shift spawns too (if placed early)
    for (int s = 0; s < r->num_spawns; s++) {
      r->spawns[s].x += offx;
      r->spawns[s].y += offy;
    }
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

TileFlags RoomSpecialDecor(RoomFlags p) {
  switch(p) {
    case ROOM_PURPOSE_CHALLENGE: return TILEFLAG_DEBRIS;
    case ROOM_PURPOSE_TREASURE:  return TILEFLAG_DECOR;
    case ROOM_PURPOSE_SECRET:      return TILEFLAG_DEBRIS | TILEFLAG_DECOR;
    default:             return 0;
  }
}
