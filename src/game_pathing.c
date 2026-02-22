#include "game_gen.h"
#include "game_types.h"
#include "game_process.h"

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

path_pool_t* InitPathPool(int cap){
  path_pool_t* p = GameCalloc("InitPathPool", 1, sizeof(path_pool_t));

  *p = (path_pool_t){
    .cap = cap,
      .entries = GameCalloc("InitPathPool entries", cap, sizeof(path_cache_entry_t))
  };

  return p;
}

void PathsEnsureCap(path_pool_t* t){
  if (t->count < t->cap)
    return;

  size_t new_cap = t->cap + 64;

  path_cache_entry_t* new_entries =
    realloc(t->entries, new_cap * sizeof(path_cache_entry_t));

  if (!new_entries) {
    // Handle failure explicitly
    TraceLog(LOG_WARNING,"==== LOCAL CONTEXT ERROR ===\n REALLOC FAILED");
  }

  t->entries = new_entries;
  t->cap = new_cap;

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

path_cache_entry_t* PathCacheFindByUID(path_pool_t* p, game_object_uid_i start, game_object_uid_i end){

   path_cache_uid_i guid = hash_combine_64(start, end);

   assert(guid != UID_INVALID );

   for (int i = 0; i < p->count; i++){
      path_cache_entry_t* e = &p->entries[i];

      if(e->guid == guid && e->valid)
        return e;
   }

   return NULL;
}

path_cache_entry_t* PathCacheFindByAB(path_pool_t* p, Cell start, Cell end){

   int a = IntGridIndex(start.x, start.y);
   int b = IntGridIndex(end.x, end.y);
   path_cache_uid_i puid = hash_combine_64(a, b);

   assert(puid != UID_INVALID );

   for (int i = 0; i < p->count; i++){
      path_cache_entry_t* e = &p->entries[i];

      if(e->puid == puid && e->valid)
        return e;
   }

   return NULL;
}
path_cache_entry_t* PathCacheFindRoute(ent_t* e, local_ctx_t* dest){
  game_object_uid_i start = e->gouid;
  game_object_uid_i end = dest->gouid;

  path_cache_entry_t *p = NULL;
  path_pool_t* pool = WorldLevel()->paths;

  p = PathCacheFindByUID(pool, start, end);
  if(p)
    return p;

  p = PathCacheFindByAB(pool, e->pos, dest->pos);

  return p;
}

Cell PathGetLocal(local_ctx_t* dest){
  Cell goal = CELL_UNSET;
  switch(dest->other.type_id){
    case DATA_ENV:
      env_t* t = ParamReadEnv(&dest->other);
      goal = t->pos;
      break;
    case DATA_ENTITY:
      ent_t* e = ParamReadEnt(&dest->other);
      goal = e->pos;
      break;
    case DATA_MAP_CELL:
      map_cell_t* mc = ParamReadMapCell(&dest->other);
      goal = mc->coords;
      break;
  }

  return goal;
}


path_cache_entry_t* StartRoute(ent_t* e, local_ctx_t* dest, int depth, bool* result){
    path_cache_entry_t* p = PathCacheFindRoute(e, dest);

  if(p)
    *result = true;
  else{
    game_object_uid_i start = e->gouid;
    game_object_uid_i end = dest->gouid;

    Cell goal = PathGetLocal(dest);
    Cell next;
    map_grid_t* m = e->map;
    if(!m)
      m = WorldGetMap();

    path_result_t* pres = FindPathCell(m, e->pos, goal, &next, depth); 
    if(pres){
      *result = true;
      p = PathCacheStore(pres, e->pos, goal, start, end); 
    }
  }

  return p;
}

Cell* PathCachedNext(path_cache_entry_t* entry, int sx, int sy, int *index){
  *index = entry->length;

  Cell cc = CELL_NEW(sx,sy);
  for(int i = entry->length-1; i > -1; i--){
    if(cell_compare(cc, entry->path[i])){
      *index = i-1;
      break;
    }
  }

  if(*index >= 0)
    return &entry->path[*index];

  return NULL;
}
int RouteScore(ent_t* e, path_cache_entry_t* route){

  int index = -1;
  Cell *out = PathCachedNext(route, e->pos.x, e->pos.y, &index);

  if(index > -1){
    route->next = route->path[index];
    return route->cost - ((route->length-1) - index);
  }

  if(!out)
    return -1;

  return 0;
}

Cell RouteGetNext(ent_t* e, path_cache_entry_t* route){

  Cell c = e->pos;
  int index = -1;  
  Cell *out = PathCachedNext(route, c.x,c.y,&index);

  return out?*out:CELL_UNSET;
}

path_cache_entry_t* PathCacheStore(path_result_t* res, Cell sc, Cell tc, game_object_uid_i start, game_object_uid_i end){
  static int next = 0;
  path_cache_entry_t* tmp = PathCacheStoreTemp(res, sc.x, sc.y, tc.x, tc.y);

  int a = IntGridIndex(sc.x, sc.y);
  int b = IntGridIndex(tc.x, tc.y);
  path_cache_uid_i puid = hash_combine_64(a, b);

  assert(puid != UID_INVALID );

  path_cache_uid_i guid = hash_combine_64(start, end);

  assert(guid != UID_INVALID );

  path_pool_t* p = WorldLevel()->paths;
  PathsEnsureCap(p);

  path_cache_entry_t* dst = &p->entries[p->count++];
  dst->guid = guid;
  dst->length = tmp->length;
  dst->sx = tmp->sx;
  dst->sy = tmp->sy;
  dst->tx = tmp->tx;
  dst->ty = tmp->ty;
  dst->cost = tmp->cost;
  dst->valid = true;
  memcpy(dst->path, tmp->path, tmp->length * sizeof(Cell));
  dst->length = tmp->length;

  dst->puid = puid;
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
        return 0;

    // Init nodes
    for (int y = 0; y < m->height; y++)
    for (int x = 0; x < m->width; x++) {
      map_cell_t mc = m->tiles[x][y];
      if(mc.status > TILE_REACHABLE)
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
    int len = 0;
    while (depth > len)
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
              len++;
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

  path_result_t *res = GameCalloc("FindPath", 1,sizeof(path_result_t));
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
   
    path[len++] = CELL_NEW(sx,sy);

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
    int index = -1;
    outNextStep = PathCachedNext(cached, sx, sy, &index);
 
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
