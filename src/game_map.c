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
  builder.num_rooms = 0;
  /*
     Cell list[1+test->spawn_max];
     list[0]=CELL_NEW(5,20);
     map_set(TILEFLAG_START,5,20);
     int poi_count = MapPOIs(m,list,test,1,1+test->spawn_max);
     MapRoomGen(m,list,poi_count);
     MapRoomBuild(m);
     */

  Cell pos = CELL_NEW(MAX_ROOM_SIZE + test->border,MAX_ROOM_SIZE + test->border); 
  for(int i =  0; i < test->num_rooms; i++){
    MapGenerateRoomAt(pos,test->rooms[i]);
    pos = CellInc(builder.rooms[i]->exit,CELL_NEW(RandRange(test->spacing+2,test->hall_length) ,0));
  }

  for (int i = 1; i < test->num_rooms; i++){
    Cell start = builder.rooms[i-1]->exit;
    Cell end = builder.rooms[i]->enter;
    MapConnectRooms(start, end,test); 
  }

  MapRoomBuild(m);
  MapPlaceSpawns(m);
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

Cell MapGetEmptyRoomTile(map_grid_t* m, Rectangle bounds, bool empty){
  Cell pool[RectArea(bounds)];
  int end_y = bounds.y+bounds.height;
  int end_x = bounds.x+bounds.width;
  int num_pos = 0;
  for(int y = bounds.y; y < end_y; y++)
    for(int x = bounds.x; x < end_x; x++){

      if(m->tiles[x][y].status > TILE_ISSUES)
        continue;

      TileStatus *status = calloc(1,sizeof(TileStatus));
      if(empty && MapGetOccupant(m, (Cell){x,y}, status) != NULL)
        continue;

      pool[num_pos++] = CELL_NEW(x,y);
    }

  if(num_pos == 0)
    return CELL_UNSET;

  int index = RandRange(0,num_pos);

  return pool[index];
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

Cell MapClosestWall(Rectangle bounds, Cell from){
  int x1 = bounds.x;
  int x2 = x1+bounds.width;
  int y1 = bounds.y;
  int y2 = y1+bounds.y;

  Cell out = from;

  // clamp X into the room bounds
  if (from.x < x1) out.x = x1;
  else if (from.x > x2) out.x = x2;

  // clamp Y into the room bounds
  if (from.y < y1) out.y = y1;
  else if (from.y > y2) out.y = y2;

  // now (out.x, out.y) lies on one of: top, bottom, left, right edges
  return out;
}



int MapCarveHorizontal(Cell c1, Cell c2, int width)
{
  int start = c1.x < c2.x ? c1.x : c2.x;
  int end   = c1.x < c2.x ? c2.x : c1.x;

  int len = 0;
  for (int x = start; x <= end; x++){
    len++;
    for (int w = -width; w <=width; w++)
      if(c1.y + w >= 0){
        map_set_safe(TILEFLAG_FLOOR,x,c1.y+w);
      }
  }
  return len;
}

int MapCarveVertical(Cell c1, Cell c2, int width)
{
  int start = c1.y < c2.y ? c1.y : c2.y;
  int end   = c1.y < c2.y ? c2.y : c1.y;

  int len = 0;
  for (int y = start; y <= end; y++){
    len++;
    for (int w = -width; w <=width; w++)
      if(c1.x+w >= 0){
        map_set_safe(TILEFLAG_FLOOR,c1.x+w,y);
      }
  }

  return len;
}

void MapCarveHallBetween(Cell a, Cell b, int width){
    Cell cur = a;

    while (!cell_compare(cur, b))
    {
        Cell dir = cell_dir(cur, b);  // {-1,0},{1,0},{0,1},{0,-1}

        int dx = abs(b.x - cur.x);
        int dy = abs(b.y - cur.y);

        Cell next = cur;

        // move in the dominant axis
        if (dx > dy)
            next.x += dir.x;
        else
            next.y += dir.y;

        // carve one segment
        if (next.x != cur.x)
            MapCarveHorizontal(cur, next, width);
        else
            MapCarveVertical(cur, next, width);

        cur = next;
    }
}

void MapConnectRooms(Cell start, Cell end,map_gen_t* rules){
  // place doors
    map_set(TILEFLAG_DOOR, start.x, start.y);
    map_set(TILEFLAG_DOOR, end.x,   end.y);

    MapCarveHallBetween(start,end, test->hall_thickness);

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

  for(int i = 0; i < 2;i++){
    for (int y = 0; y < m->height; y++){
      for (int x = 0; x < m->width; x++){
        m->tiles[x][y].coords = CELL_NEW(x,y);

        if(builder.enviroment[x][y] == TILEFLAG_EMPTY)
          continue;

        TileFlags f = builder.enviroment[x][y];
        switch(f){
          case TILEFLAG_START:
            if(i>0)
            RegisterEnt(InitEnt(room_instances[0],CELL_NEW(x,y)));
            continue;
            break;
          case TILEFLAG_NONE:
            map_set(TILEFLAG_BORDER,x,y);
            break;
          case TILEFLAG_FLOOR:
            if(i==0)
              continue;
            for(int nx = x - 2; nx < x+2; nx++){
              for(int ny = y - 2; ny < y+2;ny++){
                if(!is_adjacent(CELL_NEW(x,y),CELL_NEW(nx,ny)))
                  continue;

                if(builder.enviroment[nx][ny] < TILEFLAG_EMPTY)
                  f = TILEFLAG_WALL;
              }
            }
            if(f!=TILEFLAG_WALL && rand()%15>0)
              continue;

            map_set(f,x,y);
            break;
          default:
            break;
        }

        if(i > 0)
          MapSpawn(builder.enviroment[x][y],x,y);
      }
    }

  }

}

void MapPlaceSpawns(map_grid_t *m){
  for(int i = 0; i < builder.num_rooms;i++){
    if(builder.rooms[i]->num_spawns < 1)
      continue;

    MapSpawnMob(m, builder.rooms[i]);
  }
}

void MapSpawnMob(map_grid_t* m, room_t* room){
  
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
      pool[mobs]=mob;
      mobs++;
      break;
    }

    r -= spawn[i].weight;
  }

  if(mobs<=0)
    return;  

  for(int i = 0; i < count; i++){
    ObjectInstance mob = pool[i];
    for(int j = 0; j < mob.max; j++){
      bool place = rand()%(i+j+1)==0;
      if(j>mob.min && !place)
        continue;

      Cell pos = MapGetEmptyRoomTile(m, room->bounds,true);
      RegisterEnt(InitMob(mob.id,pos));
    }
  }
}

void MapSpawn(TileFlags flags, int x, int y){

  if(flags == TILEFLAG_EMPTY)
    return;

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

room_t* MapCarveSquareRoom(Cell start, int size)
{
  room_t* room = calloc(1,sizeof(room_t));
  int half = (size >> 12);

  Cell c = CellInc(start,CELL_NEW(half,0));

  room->center = c;
  int top = start.y-1;
  int left = start.x-1;
  int bot = c.y + half+1;
  int right = c.x + half+1;

  int entrance = RandRange(top+1,top+(half-2)*2);
  int exit = RandRange(top,(top+(half-2)*2));
  room->enter = CELL_NEW(left, entrance);
  room->exit = CELL_NEW(right, exit);
  room->bounds = Rect(left, top, half*2,half*2);
  // carve floor area
  for (int y = -half; y <= half; y++)
    for (int x = -half; x <= half; x++)
      map_set(TILEFLAG_FLOOR, c.x + x, c.y + y);

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

  return room;
}

room_t* MapCarveCircleRoom(Cell c, int size)
{
    int radius = (size >> 12); // because size is in high nibble

    for (int oy = -radius; oy <= radius; oy++)
        for (int ox = -radius; ox <= radius; ox++)
            if (ox*ox + oy*oy <= radius*radius)
                map_set_safe(TILEFLAG_FLOOR, c.x + ox, c.y + oy);
}

int MapCarveRoom(RoomDefinition r, Cell center)
{
  int room_id = builder.num_rooms;
  switch (r.shape) {

    case ROOM_SHAPE_SQUARE:
      builder.rooms[builder.num_rooms++] = MapCarveSquareRoom(center, r.size);
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
  
  return room_id;
}

void MapGenerateRoomAt(Cell center, RoomInstr* stream)
{
  int room_id = -1;
    // 2. Compile
    RoomDefinition rdef;
    if (!MapCompileRoom(stream, 6, &rdef)) {
        TraceLog(LOG_WARNING,"====MAP GEN====\n Could not compile room\n");
        return;
    }

    switch(rdef.layout){
      case ROOM_LAYOUT_ROOM:
        room_id = MapCarveRoom(rdef, center);
        break;
      default:
        break;
    }

    switch(rdef.purpose){
      case ROOM_PURPOSE_START:
        map_set(TILEFLAG_START,center.x, center.y);
        break;
      case ROOM_PURPOSE_CHALLENGE:
        if (room_id < 0)
          return;
        //todo warning
        {
          int max = (rdef.size >> 12);
          int space = (rdef.size >> 12);
          center = CellInc(center,CELL_NEW(max,max));
          Cell placements[max];
          int num_pos  = CellClusterAround(center,max, space-1 ,space+1, placements);


          int count = num_pos < max? num_pos: max; 
          for(int i = 0; i < count; i++){
            if(i >= test->num_mobs){
              if(rand()%i+1 != 0)
                continue;
            }

            Rectangle inside = RectInner(builder.rooms[room_id]->bounds,1); 
            Cell pos = clamp_cell_to_bounds(placements[i],inside);

            builder.rooms[room_id]->spawns[builder.rooms[room_id]->num_spawns++] = pos;
            map_set_safe(TILEFLAG_SPAWN,pos.x,pos.y);
          }
        }
        break;
      default:
        break;
    }
    
}
