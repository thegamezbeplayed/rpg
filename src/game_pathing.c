#include "game_gen.h"
#include "game_types.h"

bool TileBlocksMovement(map_cell_t *c) {
  if(c->occupant)
    return true;

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

path_cache_entry_t* PathCacheFindByUID(game_object_uid_i start, game_object_uid_i end){

   path_cache_uid_i guid = hash_combine_64(start, end);

   assert(guid != UID_INVALID );

   for (int i = 0; i < MAX_CACHED_PATHS; i++){
      path_cache_entry_t* e = &pathCache[i];

      if(e->guid == guid)
        return e;
   }

   return NULL;
}

path_cache_entry_t* PathCacheFind(int sx, int sy, int tx, int ty){
    for (int i = 0; i < MAX_CACHED_PATHS; i++) {
        path_cache_entry_t* e = &pathCacheTmp[i];

        if (!e->valid) continue;
        if (e->navRevision != g_navRevision) continue;

        if (e->sx == sx && e->sy == sy &&
            e->tx == tx && e->ty == ty)
            return e;
    }
    return NULL;
}

path_cache_entry_t* StartRoute(ent_t* e, local_ctx_t* dest, Cell goal, int depth, bool* result){
  game_object_uid_i start = e->gouid;
  game_object_uid_i end = dest->gouid;
 
  path_cache_entry_t* p = PathCacheFindByUID(start, end);

  if(p)
    *result = true;
  else{
    Cell next;
    path_result_t* pres = FindPathCell(e->map, e->pos, goal, &next, depth); 
    if(pres){
      *result = true;
      p = PathCacheStore(pres, e->pos, goal, start, end); 
    }
  }

  return p;
}

Cell* PathCachedNext(path_cache_entry_t* entry, int sx, int sy){
  int cur = entry->length;

  Cell cc = CELL_NEW(sx,sy);
  for(int i = entry->length-1; i > -1; i--){
    if(cell_compare(cc, entry->path[i])){
      cur = i;
      break;
    }
  }

  if(cur > 0)
    return &entry->path[cur-1];

  return NULL;
}

Cell RouteGetNext(ent_t* e, path_cache_entry_t* route){

  Cell c = e->pos;  
  Cell* out = PathCachedNext(route, c.x,c.y);

  return (out)?*out:CELL_UNSET;
}

path_cache_entry_t* PathCacheStore(path_result_t* res, Cell sc, Cell tc, game_object_uid_i start, game_object_uid_i end){
  static int next = 0;
  path_cache_entry_t* tmp = PathCacheStoreTemp(res, sc.x, sc.y, tc.x, tc.y);


  path_cache_uid_i guid = hash_combine_64(start, end);

  assert(guid != UID_INVALID );

  path_cache_entry_t* dst = &pathCache[next++ % MAX_CACHED_PATHS];
  *dst = *tmp;
  dst->guid = guid;
  tmp->valid = false;
  return dst;
}

path_cache_entry_t* PathCacheStoreTemp(path_result_t* res, int sx, int sy,int tx, int ty){
    // simple round-robin eviction
    static int next_tmp = 0;

    path_cache_entry_t* e =
        &pathCacheTmp[next_tmp++ % MAX_CACHED_PATHS];

    e->valid = true;
    e->sx = sx; e->sy = sy;
    e->tx = tx; e->ty = ty;

    e->cost = res->cost;
    e->length = res->length;
    memcpy(e->path, res->path,
           sizeof(Cell) * res->length);

    e->navRevision = g_navRevision;

    return e;
}



bool room_has_access(map_context_t* ctx, cell_bounds_t room,Cell *access){
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

            if (TileFlagHasAccess(ctx->tiles[x][y])){
              access->x = x;
              access->y = y;
              return true;
            }
        }
    }

    return false;
}


void CastLight(map_grid_t *m, Cell pos, int row, float start, float end, int radius,int xx, int xy, int yx, int yy)
{
  int cx = pos.x;
  int cy = pos.y;

  if (start < end) return;

  float new_start = start;
  for (int i = row; i <= radius; i++) {
    bool blocked = false;
    for (int dx = -i, dy = -i; dx <= 0; dx++) {
      float l_slope = (dx - 0.5f) / (dy + 0.5f);
      float r_slope = (dx + 0.5f) / (dy - 0.5f);

      if (r_slope > start) continue;
      if (l_slope < end) break;

      int x = cx + dx * xx + dy * xy;
      int y = cy + dx * yx + dy * yy;

      if (!InBounds(m, x, y)) continue;

      float dist = sqrtf(dx*dx + dy*dy);
      if (dist <= radius)
        m->tiles[x][y].fow.a = 0;

      if (blocked) {
        if (TileBlocksSight(&m->tiles[x][y])) {
          new_start = r_slope;
          continue;
        } else {
          blocked = false;
          start = new_start;
        }
      } else {
        if (TileBlocksSight(&m->tiles[x][y]) && i < radius) {
          blocked = true;
          CastLight(m, pos, i+1, start, l_slope,
                    radius, xx, xy, yx, yy);
          new_start = r_slope;
        }
      }
    }
    if (blocked) break;
  }
}

bool IsDiagBlocked(map_grid_t* m, map_cell_t* cc, map_cell_t* nc, TileBlock fn){

  int dx = cc->coords.x - nc->coords.x;
  int dy = cc->coords.y - nc->coords.y;

  if (abs(dx) == 1 && abs(dy) == 1) {
    map_cell_t* cv = &m->tiles[cc->coords.x][cc->coords.y + dy];
    map_cell_t* ch = &m->tiles[cc->coords.x + dx][cc->coords.y];
    if(fn(ch) && fn(cv))
      return true;
  }

  return false;
}

int ScorePath(map_grid_t *m, int sx, int sy, int tx, int ty, int depth){
  int score = -1;
  Cell sc = CELL_NEW(sx,sy);
    // Early out: same tile
    if (sx == tx && sy == ty)
        return score;

    depth = CLAMP(depth,1,80);
    // Init nodes
    for (int y = 0; y < m->height; y++)
    for (int x = 0; x < m->width; x++) {
      map_cell_t mc = m->tiles[x][y];
      if(mc.status > TILE_REACHABLE)
        continue;

      if(cell_distance(mc.coords, sc)>depth)
        continue;

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

    int passes = 0;
    while (depth > passes)
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
            return -1; // no path
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

            map_cell_t* curr_cell = &m->tiles[current->x][current->y];
            map_cell_t* next_cell = &m->tiles[nx][ny];

            if (!InBounds(m, nx, ny)) continue;
            if (!(nx == tx && ny == ty) &&TileBlocksMovement(next_cell)) continue;

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

        passes++;
    }

    if(nodes[tx][ty].open)
      score = PathCost(tx,ty);

    return score;

}


path_result_t* FindPath(map_grid_t *m, int sx, int sy, int tx, int ty, Cell *outNextStep, int depth){

  path_result_t *res = calloc(1,sizeof(path_result_t));
  path_cache_entry_t* cached =
    PathCacheFind(sx, sy, tx, ty);

  if(cached == NULL){
    if(ScorePath(m, sx, sy, tx, ty, depth) < 0){
      free(res);
      return false;
    }

    path_node_t *goal  = &nodes[tx][ty];

    // Step 3: Backtrack from goal to start to find next step
    path_node_t *node = goal;

    int len = 0;
    Cell path[MAX_PATH_LEN] = {0};
    
    
    while (!(node->x == sx && node->y == sy)) {
      if (node->parentX < 0 || node->parentY < 0){
        free(res);
        return NULL;
      }
      path[len++] = CELL_NEW(node->x, node->y);
      node = &nodes[node->parentX][node->parentY];

      if (len >= MAX_PATH_LEN)
        break;
    }
    
    *res = (path_result_t){
      .found = true, goal->gCost,
        .length = len
    };
    memcpy(res->path, path,
        sizeof(Cell) * len);
    // Write next step
    outNextStep->x = node->x;
    outNextStep->y = node->y;

    PathCacheStoreTemp(res , sx, sy, tx, ty);
  }
  else{
    outNextStep = PathCachedNext(cached, sx, sy);
 
    *res = (path_result_t){
     .found = cached->valid,
      .cost = cached->cost,
     .length = cached->length
    };
    memcpy(res->path, cached->path,
        sizeof(Cell) * cached->length);

  }

  return res;
}

bool HasLOS(map_grid_t* m, Cell c0, Cell c1){
  int x0 = c0.x;
  int y0 = c0.y;
  int x1 = c1.x;
  int y1 = c1.y;


  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  int x = x0;
  int y = y0;

  while (!(x == x1 && y == y1))
  {
    if (TileBlocksSight(&m->tiles[x][y]))
      return false;

    int e2 = err * 2;

    int nx = x;
    int ny = y;

    if (e2 > -dy) { err -= dy; nx += sx; }
    if (e2 <  dx) { err += dx; ny += sy; }

    /* Corner blocking */
    if (nx != x && ny != y) {
      if (TileBlocksSight(&m->tiles[nx][y]) &&
          TileBlocksSight(&m->tiles[x][ny]))
        return false;
    }

    x = nx;
    y = ny;
  }

  return true;
}
