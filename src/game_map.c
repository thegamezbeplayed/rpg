#include "game_process.h"
#include "game_helpers.h"
#include <limits.h>
static map_context_t world_map;
static path_node_t nodes[GRID_WIDTH][GRID_HEIGHT];

bool InitMap(void){
  map_context_t* ctx =&world_map;
  memset(ctx, 0, sizeof(*ctx));
  ctx->map_rules = malloc(sizeof(map_gen_t));
  *ctx->map_rules = MAPS[DANK_DUNGEON];

  ctx->player_start = CELL_UNSET;
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

Cell MapApplyContext(map_grid_t* m){
  m->id = world_map.map_rules->id;
  m->tiles = malloc(world_map.width * sizeof(map_cell_t*));
  int sect_depth = world_map.height/SECTION_SIZE;
  int sect_ind = world_map.width/SECTION_SIZE;
  m->sections = malloc(sect_ind * sizeof(map_section_t));
  Cell out = CELL_UNSET;
  /*
  for(int sx = 0; sx < section_ind; sx++){
    m->sections[sx] = malloc(sect_depth * sizeof(map_section_t));
    for(int sy = 0; sy < section_depth; sy++){

    }
  }
  */
  for(int x = 0; x < world_map.width; x++){
    m->tiles[x] = calloc(m->height, sizeof(map_cell_t));
    for(int y = 0; y < world_map.height; y++){
      m->tiles[x][y].coords = CELL_NEW(x,y); 
      m->tiles[x][y].fow = BLACK; 
     
      /*
       * if(world_map.tiles[x][y] == TILEFLAG_SPAWN)
        MapSpawnMob(m,x,y);
      else
      */
        MapSpawn(world_map.tiles[x][y],x,y);
    }
  }
  
  for(int r = 0; r < world_map.num_rooms; r++){
    RoomSpawnMob(m, &world_map.rooms[r]);
    for(int o = 0; o < world_map.rooms[r].num_children; o++){
      if(world_map.rooms[r].openings[o].sub)
        RoomSpawnMob(m, world_map.rooms[r].openings[o].sub);
    }
  }
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
    if(RegisterEnt(r->mobs[i]))
       SetState(r->mobs[i],STATE_SPAWN,NULL);


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

  /*n_connect.type = MAP_NODE_CONNECT_HALLS;
  n_connect.run  = NodeConnectHalls;
  n_carve.type = MAP_NODE_CARVE_TO_TILES;
  n_carve.run  = NodeCarveToTiles;
*/
/*
 * n_spawns.type = MAP_NODE_PLACE_SPAWNS;
  n_spawns.run  = NodePlaceSpawns;
*/
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
    if(i > temp_id)
      ctx->map_rules->num_rooms++;

    if((ctx->map_rules->rooms[i]&ROOM_LAYOUT_MASK)!=ROOM_LAYOUT_NONE)
      continue;

    RoomFlags flags = ctx->map_rules->rooms[i]
      | ROOM_LAYOUT_ROOM 
      | RandomPurpose()
      | RandomShape()
      | SizeByWeight(ROOM_SIZE_MAX,69);

    ctx->map_rules->rooms[i] = flags;
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

    Cell size = RoomSize(room);
    room->dir = RoomFlagsGetPlacing(room->flags);
    room->center = CellInc(CellMul(room->dir,CellInc(CellScale(size,1),prev_dis)),prev_pos);

    Cell dir = (i>0)?cell_dir(room->center,prev_pos):room->dir;
    Cell door_pos = CellInc(CellMul(dir,size),room->center);
    prev_dis = CellInc(size,CellScale(spacing,0.5));
    if(purpose!=ROOM_PURPOSE_START)
    room->openings[room->num_children++] = (room_opening_t){
      .entrance = true,
      .pos = door_pos,
      .dir = room->dir
    };

    prev_pos = room->center;
    
    if(i>0)
      door_pos = CELL_UNSET;


    for(int j = i+1; j < ctx->num_rooms; j++){
      room->openings[room->num_children++] = (room_opening_t)
      {
        .dir = RoomFlagsGetPlacing(ctx->rooms[j].flags),
          .pos = door_pos,
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
  }

  return ctx->num_rooms > 0 ? MAP_NODE_SUCCESS : MAP_NODE_FAILURE;
}

MapNodeResult MapGraphRooms(map_context_t *ctx, map_node_t *node) {
  /*
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
*/
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

MapNodeResult MapConnectSubrooms(map_context_t *ctx, map_node_t *node) {
  for (int i = 0; i < ctx->num_rooms; i++) {
    room_t *root = &ctx->rooms[i];
    for (int k = 0; k < root->num_children; k++){
      if(root->openings[k].sub == NULL)
        continue;

      room_t *sub = root->openings[k].sub;

      Cell r_ent = root->openings[k].pos;
      Cell s_ent = sub->openings[0].pos;
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
    Cell pos = r->openings[i].pos;
    TileFlags opening = TILEFLAG_EMPTY;
    if(r->openings[i].entrance)
      opening = TILEFLAG_DOOR;

    ctx->tiles[pos.x][pos.y] = opening;
    if(r->openings[i].sub)
      RoomCarveTiles(ctx,r->openings[i].sub);
  }

}

MapNodeResult MapCarveTiles(map_context_t *ctx, map_node_t *node) {
  // clear
  for (int y = 0; y < ctx->height; y++)
    for (int x = 0; x < ctx->width; x++)
      ctx->tiles[x][y] = TILEFLAG_BORDER;

  // rooms
  for (int r = 0; r < ctx->num_rooms; r++)
    RoomCarveTiles(ctx,&ctx->rooms[r]);


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
  if(r->num_mobs >0){
    DO_NOTHING();
  }

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
    RoomPlaceSpawns(ctx, &ctx->rooms[r]);
    for (int i = 0; i < ctx->rooms[r].num_children; i++){
      if(ctx->rooms[r].openings[i].sub == NULL)
        continue;

       RoomPlaceSpawns(ctx, ctx->rooms[r].openings[i].sub);

    }
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

Cell RoomGetBestOpening(room_t* root, room_t* sub, int child){
  Cell r_dir = RoomFlagsGetPlacing(root->flags);
  RoomFlags s_dir = sub->flags&ROOM_PLACING_MASK;
  Cell size = RoomSize(sub);

  Cell pos = CellMul(root->openings[child].dir,size);

  pos = CellInc(CellMul(r_dir,size),pos);

  return pos;
}


void RoomAdjustPosition(room_t *r, Cell disp){

  r->center = CellInc(r->center,disp);

  r->bounds = RoomBounds(r, r->center);

  for(int i = 0; i < r->num_children; i++){
    r->openings[i].pos = CellInc(r->openings[i].pos,disp);


    if(!r->openings[i].sub)
      continue;

    if((r->openings[i].sub->flags&ROOM_PURPOSE_MASK) == ROOM_PURPOSE_CONNECT)
      continue;
       RoomAdjustPosition(r->openings[i].sub, disp);
  }

}

MapNodeResult MapPlaceSubrooms(map_context_t *ctx, map_node_t *node) {

  for (int i = 0; i < ctx->num_rooms; i++){
    room_t *root = &ctx->rooms[i];
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


      Cell opening = CellScale(o_dir,ctx->map_rules->spacing);
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
  int width  = abs(maxx) + abs(minx);// + ctx->map_rules->border;
  int height = abs(maxy) + abs(miny);// + ctx->map_rules->border;;

  ctx->width = clamp_to_interval(width,SECTION_SIZE);
  ctx->height = clamp_to_interval(height,SECTION_SIZE);

  TraceLog(LOG_INFO,"\n=====MAP SIZE ===\n===== WIDTH %i====\n====HEIGHT %i ====\n",ctx->width,ctx->height);

  int offsetx = clamp_to_interval(-minx,SECTION_SIZE);
  int offsety = clamp_to_interval(-miny,SECTION_SIZE);
  Cell offset = CELL_NEW(offsetx,offsety);

  for (int i = 0; i < ctx->num_rooms; i++) {
    RoomAdjustPosition(&ctx->rooms[i],offset);

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
    RoomFlags purpose = ctx->rooms[i].flags&ROOM_PURPOSE_MASK;
    if(purpose!= ROOM_PURPOSE_START)
      continue;

    ctx->player_start = ctx->rooms[i].center;
      return MAP_NODE_SUCCESS;

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
