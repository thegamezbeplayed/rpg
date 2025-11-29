#include "game_process.h"
#include "game_helpers.h"

#define map_set(f, x,y) MapBuilderSetFlags((f), (x),(y),false)
#define map_set_safe(f, x,y) MapBuilderSetFlags((f), (x),(y),true)

#define MAX_PLACEMENT_ATTEMPTS 32

static map_build_t builder;
static path_node_t nodes[GRID_WIDTH][GRID_HEIGHT];

static MapID MAP = DANK_DUNGEON;

static map_gen_t* test;

map_grid_t* InitMapGrid(void){
  map_grid_t* m = malloc(sizeof(map_grid_t));

  *m = (map_grid_t){0};

  m->step_size = CELL_WIDTH;

  m->spawn_rate = 8;
  m->x = 0;
  m->y = 0;
  m->width = GRID_WIDTH;
  m->height = GRID_HEIGHT;
  m->floor = DARKBROWN;
  return m;
}

void MapLoad(map_grid_t* m){
  test = malloc(sizeof(map_gen_t));

  *test = MAPS[MAP];
  /*
  Cell list[1+test->spawn_max];
  list[0]=CELL_NEW(5,20);
  map_set(TILEFLAG_START,5,20);
  int poi_count = MapPOIs(m,list,test,1,1+test->spawn_max);
   MapRoomGen(m,list,poi_count);
   MapRoomBuild(m);
   */
  MapGenerateRoomAt(CELL_NEW(20,20));

   MapRoomBuild(m);
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

void MapBuilderSetFlags(TileFlags flags, int x, int y,bool safe){
  if(safe && builder.enviroment[x][y]!=TILEFLAG_NONE)
    return;

  if(builder.enviroment[x][y] == TILEFLAG_START)
    return;

  builder.enviroment[x][y] = flags;// | test->map_flag;
}

Cell MapGetTileByFlag(map_grid_t* m, TileFlags f){
  for(int y = m->y; y < m->height; y++)
    for(int x = m->x; x < m->width; x++){
      if(builder.enviroment[x][y] == f)
        return CELL_NEW(x,y);
    }

  return CELL_UNSET;
}

static void carve_circle(Cell center, int radius) {
    for (int oy = -radius; oy <= radius; oy++) {
        for (int ox = -radius; ox <= radius; ox++) {
            if (ox*ox + oy*oy <= radius*radius) {
                map_set_safe(TILEFLAG_FLOOR, center.x + ox, center.y + oy);
            }
        }
    }
}

void MapCarveHorizontal(Cell c1, Cell c2, int width)
{
  int start = c1.x < c2.x ? c1.x : c2.x;
  int end   = c1.x < c2.x ? c2.x : c1.x;
  float radius = width/2;

  for (int x = start; x <= end; x++){
    for (int w = -width/2; w <=width/2; w++)
      if(c1.y + w >= 0){
        TileFlags f = rand()%4==0?TILEFLAG_FLOOR:TILEFLAG_EMPTY;
        map_set_safe(f,x,c1.y+w);
      }
  }
}

void MapCarveVertical(Cell c1, Cell c2, int width)
{
  int start = c1.y < c2.y ? c1.y : c2.y;
  int end   = c1.y < c2.y ? c2.y : c1.y;

  float radius = width/2;

  for (int y = start; y <= end; y++){
    for (int w = -width/2; w <=width/2; w++)
      if(c1.x+w >= 0){
        TileFlags f = rand()%4==0?TILEFLAG_FLOOR:TILEFLAG_EMPTY;

        map_set_safe(TILEFLAG_FLOOR,c1.x+w,y);
      }
  }
}

void MapConnectRooms(map_grid_t* m, Cell *rooms, map_gen_t* rules){
  for(int i = 1 ; i <m->rooms;i++){

    if(rand()%2){
      MapCarveHorizontal(rooms[i-1],rooms[i],rules->border);
      MapCarveVertical(rooms[i],rooms[i-1], rules->border);
    }
    else{
      MapCarveVertical(rooms[i-1],rooms[i],rules->border);
      MapCarveHorizontal(rooms[i],rooms[i-1],rules->border);
    }

  }
}

void MapRoomGen(map_grid_t* m, Cell *poi_list, int poi_count){
  /*
  for (int i = 0; i < poi_count; i++){
    Cell p = poi_list[i];

    RoomType shape = test->room_type;
    float radius = 5;

    for (int dy = -2; dy <= 2; dy++){
      switch (shape){
        case ROOM_ROUND:
          carve_circle((Cell){p.x, dy}, radius);
          break;
        default:

          for (int dx = -2; dx <=2; dx++){
            int nx = p.x +dx;
            int ny = p.y +dy;

            if(nx > 0 && nx < m->width &&
                ny > 0 && ny < m->height)
              map_set_safe(TILEFLAG_EMPTY,nx,ny);
          }
          break;
      }
    }
    m->rooms++;
    map_set(TILEFLAG_SPAWN,p.x,p.y);
  }

  MapConnectRooms(m,poi_list,test); 
     for (int i = 0; i < roomArea * 3; i++)
     {
    // carve
            
    float n = perlin2d(px * 0.25f, py*0.25f, .12f, 4);
    if(n>.25f){
      if(rand()%m->spawn_rate==0)
        map_set(TILEFLAG_SPAWN,px,py);
      else if (rand()%15==0)
        map_set(TILEFLAG_DEBRIS,px,py);
      else
        map_set(TILEFLAG_FLOOR,px,py);
    }
    else if(n>0.1f)
      map_set( TILEFLAG_GRASS, px,py);
    else{
      if(rand()%16==0)
        map_set(TILEFLAG_FLOOR,px,py);
      else
        map_set(TILEFLAG_EMPTY,px,py);
    }
    int dir = rand() % 4;
    if (dir == 0) px++;
    if (dir == 1) px--;
    if (dir == 2) py++;
    if (dir == 3) py--;

    // keep inside m boundaries
    if (px < bt) px = bt;
    if (px >= m->width-bt) px = m->x + m->width - bt;
    if (py < bt) py = bt;
    if (py >= m->height-bt) py = m->y + m->height - bt;
  }
*/
}

void MapRoomBuild(map_grid_t* m){
  Rectangle inner = Rect(test->border,test->border, m->width - 2*test->border,m->height- 2*test->border);

  for (int y = 0; y < m->height; y++){
    for (int x = 0; x < m->width; x++){
      m->tiles[x][y].coords = CELL_NEW(x,y);

      if(builder.enviroment[x][y] == TILEFLAG_EMPTY)
        continue;

      TileFlags f = builder.enviroment[x][y];
      switch(f){
        case TILEFLAG_SPAWN:
          MapSpawnMob(x,y);
          break;
        case TILEFLAG_START:
          RegisterEnt(InitEnt(room_instances[0],CELL_NEW(x,y)));
          continue;
          break;
        case TILEFLAG_NONE:
          map_set(TILEFLAG_BORDER,x,y);
          break;
        default:
          break;
      }
      MapSpawn(builder.enviroment[x][y],x,y);

    }
  }
}

void MapSpawnMob(int x, int y){
  int total = 0;
  int count = test->num_mobs;
  spawn_rules_t* spawn = test->mobs;
  for (int i = 0; i < count; i++){
    if(spawn[i].mob == ENT_DONE)
      break;
    total+=spawn[i].weight;
  }

  int r = rand() % total;

  int mobs = 0;
  ObjectInstance pool[count];

  for (int i = 0; i < count; i++){
    if(spawn[i].mob == ENT_DONE)
      break;

    if(r<spawn[i].weight){
      ObjectInstance mob = GetEntityData(spawn[i].mob);
      for (int j = 0; j < mob.max; j++){
        if(mobs>=mob.max)
          break;

        if(mobs<mob.min || rand()%(mobs+1)==0){
          pool[mobs]=mob;
          mobs++;
        }
      }
      break;
    }
    
    r -= spawn[i].weight;
  }

  if(mobs==0)
    return;  
  Cell *placements = CellClusterAround((Cell){x,y},mobs, 2,4);

 for(int i = 0; i < mobs; i++) 
  RegisterEnt(InitMob(pool[i].id,placements[i]));

 if(GetEntityCategory(pool[0].id)!=MOB_HUMANOID)
   map_set(TILEFLAG_FLOOR,x,y);

 free(placements);
}

void MapSpawn(TileFlags flags, int x, int y){

  EnvTile t = GetTileByFlags(flags|test->map_flag);

  Cell pos = {x,y};
  RegisterEnv(InitEnv(t,pos));

}

int MapPOIs(map_grid_t* map, Cell *list, map_gen_t* rules, int start, int count){
  int placed = start;
  int dist = rules->spacing;

  for (int i = 0; i < count; i++){
    Cell poi = CELL_EMPTY;
    if(i < start)
      continue;

    for (int attempts = 0; i < MAX_PLACEMENT_ATTEMPTS; i++){
      poi.x = rand()%map->width;
      poi.y = rand()%map->height;

      if(poi.x < rules->border || poi.x > map->width-rules->border)
        continue;

      if(poi.y < rules->border || poi.y > map->height-rules->border)
        continue;

      bool valid = true;
      for (int j = 0; j < placed; j++) {
        if (TooClose(poi, list[j], dist)) {
          valid = false;
          break;
        }
      }
      if (!valid) continue;

      list[placed++] = poi;
      break;
    }
  }

  return placed;
}

bool TooClose(Cell a, Cell b, int min_dist) {
    int dx = abs(a.x - b.x);
    int dy = abs(a.y - b.y);
    return (dx*dx + dy*dy) < (min_dist * min_dist);
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

bool FindPath(map_grid_t *m, int sx, int sy, int tx, int ty, Cell *outNextStep)
{
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

bool MapCompileRoom(const RoomInstr *stream, int count, RoomDefinition *out)
{
    bool in_block = false;
    RoomFlags flags = 0;

    for (int i = 0; i < count; i++) {
        RoomInstr instr = stream[i];

        switch (instr.op) {

            case CONF_START:
                if (in_block) return false; // nested START error
                in_block = true;
                flags = 0;
                break;

            case CONF_FLAG:
                if (!in_block) return false;
                flags |= instr.flags;  // combine mask bits
                break;

            case CONF_END:
                if (!in_block) return false;
                in_block = false;

                // Extract categories
                out->flags  = flags;
                out->size   = flags & ROOM_SIZE_MASK;
                out->layout = flags & ROOM_LAYOUT_MASK;
                out->purpose= flags & ROOM_PURPOSE_MASK;
                out->shape  = flags & ROOM_SHAPE_MASK;
                return true;

            default:
                return false;
        }
    }

    return false; // missing CONF_END
}

void MapCarveSquareRoom(Cell c, int size)
{
  int half = (size >> 12);

  // carve floor area
  for (int y = -half; y <= half; y++)
    for (int x = -half; x <= half; x++)
      map_set_safe(TILEFLAG_FLOOR, c.x + x, c.y + y);

  // carve walls around borders
  int outer = half + 1;

  // top + bottom walls
  for (int x = -outer; x <= outer; x++) {
    map_set_safe(TILEFLAG_WALL, c.x + x, c.y - outer);
    map_set_safe(TILEFLAG_WALL, c.x + x, c.y + outer);
  }

  // left + right walls
  for (int y = -outer; y <= outer; y++) {
    map_set_safe(TILEFLAG_WALL, c.x - outer, c.y + y);
    map_set_safe(TILEFLAG_WALL, c.x + outer, c.y + y);
  }
}

void MapCarveCircleRoom(Cell c, int size)
{
    int radius = (size >> 12); // because size is in high nibble

    for (int oy = -radius; oy <= radius; oy++)
        for (int ox = -radius; ox <= radius; ox++)
            if (ox*ox + oy*oy <= radius*radius)
                map_set_safe(TILEFLAG_FLOOR, c.x + ox, c.y + oy);
}

void MapCarveRoom(RoomDefinition r, Cell center)
{
    switch (r.shape) {

        case ROOM_SHAPE_SQUARE:
            MapCarveSquareRoom(center, r.size);
            break;

        case ROOM_SHAPE_CIRCLE:
            MapCarveCircleRoom(center, r.size);
            break;

        case ROOM_SHAPE_FORKED:
            //CarveForkedRoom(center, r.size);
            break;

        case ROOM_SHAPE_CROSS:
            //CarveCrossRoom(center, r.size);
            break;
    }
}

void MapGenerateRoomAt(Cell center)
{
    // 1. Define config stream
    RoomInstr stream[] = {
        {CONF_START},
        {CONF_FLAG, ROOM_SIZE_LARGE},
        {CONF_FLAG, ROOM_LAYOUT_ROOM},
        {CONF_FLAG, ROOM_PURPOSE_START},
        {CONF_FLAG, ROOM_SHAPE_SQUARE},
        {CONF_END},
    };

    // 2. Compile
    RoomDefinition rdef;
    if (!MapCompileRoom(stream, 6, &rdef)) {
        TraceLog(LOG_WARNING,"====MAP GEN====\n Could not compile room\n");
        return;
    }

    MapCarveRoom(rdef, center);

    switch(rdef.purpose){
      case ROOM_PURPOSE_START:
        map_set(TILEFLAG_START,center.x, center.y);
        break;
      default:
        break;
    }
    
    /*if (rdef.purpose == ROOM_PURPOSE_CHALLENGE)
        SpawnMiniBoss(center);

    if (rdef.purpose == ROOM_PURPOSE_TREASURE)
        PlaceTreasureChest(center);

    if (rdef.layout == ROOM_LAYOUT_HALL)
        AddHallwayConnectors(center, rdef.size);

        */
}
