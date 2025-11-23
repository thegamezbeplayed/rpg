#include "game_process.h"
#include "game_helpers.h"

#define map_set(f, x,y) MapBuilderSetFlags((f), (x),(y))

static map_build_t builder;
static path_node_t nodes[GRID_WIDTH][GRID_HEIGHT];

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

void MapBuilderSetFlags(TileFlags flags, int x, int y){
  builder.enviroment[x][y] = flags;
}

void MapRoomGen(map_grid_t* m){
  int cx = (int)m->width/2;
  int cy = (int)m->height/2;

  int bt = 4;
  int roomArea = m->width * m->height;

  int px = cx;
  int py = cy;

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

}

void MapRoomBuild(map_grid_t* m){
  for (int y = 0; y < m->height; y++){
    for (int x = 0; x < m->width; x++){
      if(builder.enviroment[x][y] == TILEFLAG_EMPTY)
        continue;

      if(builder.enviroment[x][y] == TILEFLAG_SPAWN){
        MapSpawnMob(x,y);
        map_set(TILEFLAG_GRASS,x,y);
      }
      if(builder.enviroment[x][y] == TILEFLAG_NONE)
        map_set(TILEFLAG_BORDER, x,y);

      MapSpawn(builder.enviroment[x][y],x,y);

    }
  }
}

void MapSpawnMob(int x, int y){
  int total = 0;

  int count = 4;
  for (int i = 0; i < count; i++){
    if(dark_forest[i].mob == ENT_DONE)
      break;
    total+=dark_forest[i].weight;
  }

  int r = rand() % total;

  for (int i = 0; i < count; i++){
    if(dark_forest[i].mob == ENT_DONE)
      break;

    if(r<dark_forest[i].weight){
      ObjectInstance mob = GetEntityData(dark_forest[i].mob);
      int mobs = 0;
      for (int j = 0; j < mob.max; j++){
        if(mobs>=mob.max)
          break;

        if(rand()%(mobs+1)==0){
          RegisterEnt(InitMob(dark_forest[i].mob,(Cell){x,y}));
          mobs++;
          break;
        }
      }
      break;
    }

    r -= dark_forest[i].weight;
  }
}

void MapSpawn(TileFlags flags, int x, int y){

  EnvTile t = GetTileByFlags(flags);

  Cell pos = {x,y};
  RegisterEnv(InitEnv(t,pos));

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
