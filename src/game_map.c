#include "game_process.h"
#include "game_helpers.h"

#define map_set(f, x,y) MapBuilderSetFlags((f), (x),(y))

static map_build_t builder;

map_grid_t* InitMapGrid(void){
  map_grid_t* m = malloc(sizeof(map_grid_t));

  *m = (map_grid_t){0};

  m->step_size = CELL_WIDTH;

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
            
    float n = perlin2d(px, py, .10f, 4);
    if(n>.25f)
      map_set(TILEFLAG_SPAWN,px,py);
    else if(n>0.1f)
      map_set( TILEFLAG_GRASS, px,py);
    else
      map_set(TILEFLAG_EMPTY,px,py);

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
        //CREATE SPAWNS
        map_set(TILEFLAG_GRASS,x,y);
      }
      if(builder.enviroment[x][y] == TILEFLAG_NONE)
        map_set(TILEFLAG_BORDER, x,y);

      MapSpawn(builder.enviroment[x][y],x,y);

    }
  }
}

void MapSpawn(TileFlags flags, int x, int y){

  EnvTile t = GetTileByFlags(flags);

  Cell pos = {x,y};
  RegisterEnv(InitEnv(t,pos));

}
