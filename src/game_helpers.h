#ifndef __GAME_HELPER_H
#define __GAME_HELPER_H

#include "game_common.h"

typedef bool (*TileBlock)(map_cell_t *c);

static const int mult[8][4] = {
  { 1, 0, 0, 1 }, { 0, 1, 1, 0 },
  { 0, -1, 1, 0 }, { -1, 0, 0, 1 },
  { -1, 0, 0, -1 }, { 0, -1, -1, 0 },
  { 0, 1, -1, 0 }, { 1, 0, 0, -1 }
};

typedef struct {
    int x, y;

    int gCost;          // cost from start
    int hCost;          // heuristic
    int fCost;          // g + h

    int parentX;
    int parentY;

    bool open;
    bool closed;
} path_node_t;
static path_node_t nodes[GRID_WIDTH][GRID_HEIGHT];

static inline int Heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

static inline bool InBounds(map_grid_t *m, int x, int y) {
    return (x >= 0 && x < m->width && y >= 0 && y < m->height);
}

static int permutation[256] = {
151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,
103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
35,11,32,57,177,33,88,237,149,56,87,174,
20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,
92,41,55,46,245,40,244,102,143,54, 65,25,63,161, 1,216,
80,73,209,76,132,187,208,89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186,
 3,64,52,217,226,250,124,123, 5,202,38,147,118,126,255,
82,85,212,207,206,59,227,47,16,58,17,182,
189,28,42,223,183,170,213,119,248,152, 2,44,
154,163, 70,221,153,101,155,167,
 43,172,9,129,22,39,253,19,98,108,
110,79,113,224,232,178,185,112,
104,218,246,97,228,251,34,242,
193,238,210,144,12,191,179,162,
241, 81,51,145,235,249,14,239,
107,49,192,214, 31,181,199,106,
157,184, 84,204,176,115,121,50,
45,127, 4,150,254,138,236,205,
93,222,114, 67,29,24,72,243,
141,128,195,78,66,215,61,156,
180
};

static int p[512];

static float fade(float t) { return t*t*t*(t*(t*6-15)+10); }
static float lerp(float t, float a, float b) { return a + t*(b-a); }
static float grad(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

static float perlin2d(float x, float y, float freq, int depth){
  int i, xi, yi;
  float xf, yf, result = 0.0f, amplitude = 1.0f;
  float totalAmplitude = 0.0f;

  // initialize permutation table (once)
  static int initialized = 0;
  if (!initialized) {
    for (i = 0; i < 256; i++) {
      p[256 + i] = p[i] = permutation[i];
    }
    initialized = 1;
  }

  for (i = 0; i < depth; i++) {
    xi = (int)(x * freq) & 255;
    yi = (int)(y * freq) & 255;

    xf = (x * freq) - (int)(x * freq);
    yf = (y * freq) - (int)(y * freq);

    float u = fade(xf);
    float v = fade(yf);

    int aa = p[p[xi] + yi];
    int ab = p[p[xi] + yi + 1];
    int ba = p[p[xi + 1] + yi];
    int bb = p[p[xi + 1] + yi + 1];

    float x1 = lerp(u, grad(aa, xf, yf, 0), grad(ba, xf - 1, yf, 0));
    float x2 = lerp(u, grad(ab, xf, yf - 1, 0), grad(bb, xf - 1, yf - 1, 0));
    float value = lerp(v, x1, x2);

    result += value * amplitude;
    totalAmplitude += amplitude;

    amplitude *= 0.5f;
    freq *= 2.0f;
  }

  return result / totalAmplitude; 


}

static float rand2i(int x, int y) {
    unsigned int n = x * 374761393 + y * 668265263; 
    n = (n ^ (n >> 13)) * 1274126177;
    return (float)(n & 0x7fffffff) / (float)0x7fffffff;
}

static float Noise2D(float x, float y) {
    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float sx = x - x0;
    float sy = y - y0;

    float n0 = rand2i(x0, y0);
    float n1 = rand2i(x1, y0);
    float ix0 = lerp(n0, n1, sx);

    n0 = rand2i(x0, y1);
    n1 = rand2i(x1, y1);
    float ix1 = lerp(n0, n1, sx);

    return lerp(ix0, ix1, sy);
}

static inline cell_bounds_t BoundsFromRec(Rectangle r){
  cell_bounds_t out = {
    .min = CELL_NEW(r.x-r.width/2,r.y-r.height/2),
    .center = CELL_NEW(r.x, r.y),
    .max = CELL_NEW(r.x+r.width/2,r.y+r.height/2)
  };

  return out;
}

static inline cell_bounds_t ShiftCellBounds(cell_bounds_t b, Cell shift){
  cell_bounds_t out = b;

  out.min = CellInc(b.min,shift);
  out.center = CellInc(b.center,shift);
  out.max = CellInc(b.max,shift);

  return out;
}

static inline Rectangle RecFromBounds(cell_bounds_t* bounds){
  Cell dist = CellSub(bounds->max,bounds->min);

  return (Rectangle){bounds->min.x,bounds->min.y, dist.x,dist.y};
}

static int SplitBoundsFromRec(Rectangle base, int sect_size,
                              node_connector_t **fill, int max_sec)
{
    int count = 0;

    // Compute how many chunks along each axis (min 1)
    int hchunks = imax(1, (int)base.width  / sect_size);
    int vchunks = imax(1, (int)base.height / sect_size);

    // EXCLUDE CORNERS → usable span is width-2, height-2
    int inner_w = (int)base.width  - 2;
    int inner_h = (int)base.height - 2;
    if (inner_w < 1) inner_w = 1;
    if (inner_h < 1) inner_h = 1;

    int chunk_w = inner_w / hchunks;
    int chunk_h = inner_h / vchunks;
    if (chunk_w < 1) chunk_w = 1;
    if (chunk_h < 1) chunk_h = 1;

    // Starting positions skipping corners
    int start_x = (int)base.x + 1;
    int start_y = (int)base.y + 1;

    // ---- TOP edge (1 row above) ----
    for (int i = 0; i < hchunks && count < max_sec; i++) {
        Rectangle r = {
            start_x + i * chunk_w,
            base.y - 1,    // 1 CELL OUTSIDE
            chunk_w,
            1
        };
        node_connector_t *c = calloc(1, sizeof(*c));
        c->range = BoundsFromRec(r);
        c->dir = CELL_UP;
        fill[count++] = c;
    }

    // ---- BOTTOM edge (1 row below) ----
    for (int i = 0; i < hchunks && count < max_sec; i++) {
        Rectangle r = {
            start_x + i * chunk_w,
            base.y + base.height,   // 1 CELL OUTSIDE
            chunk_w,
            1
        };
        node_connector_t *c = calloc(1, sizeof(*c));
        c->range = BoundsFromRec(r);
        c->dir = CELL_DOWN;
        fill[count++] = c;
    }

    // ---- LEFT edge (1 column to the left) ----
    for (int i = 0; i < vchunks && count < max_sec; i++) {
        Rectangle r = {
            base.x - 1,   // 1 CELL OUTSIDE
            start_y + i * chunk_h,
            1,
            chunk_h
        };
        node_connector_t *c = calloc(1, sizeof(*c));
        c->range = BoundsFromRec(r);
        c->dir = CELL_LEFT;
        fill[count++] = c;
    }

    // ---- RIGHT edge (1 column to the right) ----
    for (int i = 0; i < vchunks && count < max_sec; i++) {
        Rectangle r = {
            base.x + base.width,   // 1 CELL OUTSIDE
            start_y + i * chunk_h,
            1,
            chunk_h
        };
        node_connector_t *c = calloc(1, sizeof(*c));
        c->range = BoundsFromRec(r);
        c->dir = CELL_RIGHT;
        fill[count++] = c;
    }

    return count;
}

static cell_bounds_t RoomBounds(RoomFlags flags,Cell c){

  RoomFlags size = flags & ROOM_SIZE_MASK;
  RoomFlags purpose = flags & ROOM_PURPOSE_MASK;
  RoomFlags layout = flags & ROOM_LAYOUT_MASK;
  RoomFlags orient = flags & ROOM_ORIENT_MASK;
  RoomFlags shape = flags & ROOM_SHAPE_MASK;


  int area = 25;
  switch(size){
    case ROOM_SIZE_MEDIUM:
      area = 36;
      break;
    case ROOM_SIZE_LARGE:
      area =49;
      break;
    case ROOM_SIZE_XL:
      area = 64;
      break;
    case ROOM_SIZE_HUGE:
      area = 81;
      break;
    case ROOM_SIZE_MASSIVE:
      area = 100;
      break;
    case ROOM_SIZE_MAX:
      area = 111;
      break;
  }

  int w = isqrt(area);
  int h = isqrt(area);

  switch(layout){
    case ROOM_LAYOUT_HALL:
      h = imin(3,(w/2)-w%2);
      w*=2;
      break;
    case ROOM_LAYOUT_OPEN:
     w++;
     h++;
     break;
    case ROOM_LAYOUT_SUB:
     w--;
     h--;
     break;
    default:
     break;
  }

  switch(purpose){
    case ROOM_PURPOSE_CONNECT:
    case ROOM_PURPOSE_LAIR:
      w++;
      h++;
      break;
    default:
      break;
  }

  switch(shape){
    case ROOM_SHAPE_RECT:
      h--;
      w++;
      break;
    default:
      break;
  }

  h = imax(3,h);
  w = imax(3,w);

  Rectangle r = Rect(c.x,c.y,w,h);
  switch(orient){
    case ROOM_ORIENT_HOR:
      r = Rect(c.x,c.y,w,h);
      break;
    case ROOM_ORIENT_VER:
      r = Rect(c.x,c.y,h,w);
      break;
    default:
      break;
  }

  cell_bounds_t output = BoundsFromRec(r);
  return output;
}

static Cell RoomSize(room_t* r){
  Cell output = CELL_EMPTY;
  cell_bounds_t bounds = RoomBounds(r->flags,CELL_EMPTY);

  output = CellScale(cell_dist(bounds.min,bounds.max),0.5);
  /*
  RoomFlags size = f & ROOM_SIZE_MASK;
  RoomFlags purpose = f & ROOM_PURPOSE_MASK;
  RoomFlags layout = f & ROOM_LAYOUT_MASK;
  
  switch(layout){
    case ROOM_LAYOUT_HALL:
      if(purpose == ROOM_PURPOSE_CONNECT){
        output = CELL_NEW(size>>10,imax(size>>13,1));
      }
      else
        output = CELL_NEW(size>>11,imax(size>>13,1));
      break;
    default:
      output = CELL_NEW(size>>12,size>>12);
      break;
  }
*/
  return output;
}

static inline Cell RoomFlagsGetPlacing(RoomFlags f){
  RoomFlags placing = f & ROOM_PLACING_MASK;

  Cell dir = CELL_UNSET;
  switch(placing){
    case  ROOM_PLACING_N:
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
    case ROOM_PLACING_NW:
      dir = CellInc(CELL_UP,CELL_LEFT);
      break;
    case ROOM_PLACING_NE:
      dir = CellInc(CELL_UP,CELL_RIGHT);
      break;
    case ROOM_PLACING_SE:
      dir = CellInc(CELL_DOWN,CELL_RIGHT);
      break;
    case ROOM_PLACING_SW:
      dir = CellInc(CELL_DOWN,CELL_LEFT);
      break;
    default:
      dir = CELL_EMPTY;
      break;
  }

  return dir;
}
 
static inline RoomFlags SizeByWeight(RoomFlags max, int budget){
  int r = RandRange(0,budget);
  
  int category_size = (max>>12)-1;
  for (int i = category_size; i > 1; i--){
    if(room_size_weights[i] < r)
      return i<<12;
  }

  return 1<<12;
}

static inline RoomFlags LayoutByWeight(RoomFlags max, int budget){
  int r = RandRange(0,budget);

  int category_size = (max>>ROOM_LAYOUT_SHIFT)-1;
  for (int i = category_size; i > 1; i--){
    if(room_layout_weights[i] < r)
      return i<<ROOM_LAYOUT_SHIFT;
  }

  return 1<<ROOM_LAYOUT_SHIFT;
}



static inline RoomFlags PurposeByWeight(RoomFlags max, int budget){
  int r = RandRange(0,budget);

  int category_size = (max>>ROOM_PURPOSE_SHIFT)-1;
  for (int i = category_size; i > 1; i--){
    if(room_purpose_weights[i] < r)
      return i<<ROOM_PURPOSE_SHIFT;
  }

  return 1<ROOM_PURPOSE_SHIFT;
}


static inline int SpawnPoints(RoomFlags f){
  int shift = ROOM_MOBS_SHIFT;

  int count = 0;
  RoomFlags size = f & ROOM_SIZE_MASK;
  RoomFlags purpose = f & ROOM_PURPOSE_MASK;
  RoomFlags layout = f & ROOM_LAYOUT_MASK;
  RoomFlags orient = f & ROOM_ORIENT_MASK;
  RoomFlags shape = f & ROOM_SHAPE_MASK;

  switch(purpose){
    case ROOM_PURPOSE_START:
      return count;
      break;
    case ROOM_PURPOSE_CHALLENGE:
      shift-=1;
      break;
    case ROOM_PURPOSE_LAIR:
      shift+=1;
      break;
    case ROOM_PURPOSE_CONNECT:
      shift+=1;
    default:
      break;
  }
  
  count += (size<<8)>>shift;

  return count;
}

static inline RoomFlags RandomSize(void) {
    int count = (ROOM_SIZE_MAX - ROOM_SIZE_SMALL) >> 12;  // 7 - 1 = 6 valid sizes
    int pick  = RandRange(0, count - 1);                  // 0..5
    return (RoomFlags)((pick + 1) << 12);
}
static inline RoomFlags RandomShape(void) {
    int count = (ROOM_SHAPE_MAX - ROOM_SHAPE_SQUARE); // = 8
    int pick  = RandRange(0, count - 1);              // 0..7
    return (RoomFlags)(ROOM_SHAPE_SQUARE + pick);
}
static inline RoomFlags RandomPurpose(RoomFlags pool) {
  RoomFlags options[16];  
  int count = 0;
  for (int bit = ROOM_PURPOSE_NONE; bit <= ROOM_PURPOSE_START; bit += (1 << 4)) {
    if (pool & bit)
      continue;
  
    options[count++] = bit;
  }

  if(count == 0)
    return ROOM_PURPOSE_NONE;

  int pick = RandRange(0, count - 1);
  return options[pick];
}
static inline RoomFlags RandomOrient(void){
  return RandRange(0,5)&2==0?ROOM_ORIENT_HOR:ROOM_ORIENT_VER;
}

static inline RoomFlags RandomLayout(void) {
    int count = (ROOM_LAYOUT_MAX) >> ROOM_LAYOUT_SHIFT; // = 4 layouts
    int pick  = RandRange(1, count - 1);                   // 0..3
    return (RoomFlags)((pick) << 8);
}

static int weighted_choice(const int* weights, int count){
  int total = 0;

    // Sum weights
    for (int i = 0; i < count; i++)
        total += weights[i];

    if (total <= 0)
        return -1; // No valid choices

    int r = rand() % total;

    // Find selected index
    for (int i = 0; i < count; i++) {
        if (r < weights[i])
            return i;
        r -= weights[i];
    }

    return -1; // Should not happen

}
static inline RoomFlags GetRoomSize(RoomFlags f) {
    return (f & ROOM_SIZE_MASK);
}

static inline RoomFlags GetRoomLayout(RoomFlags f) {
    return (f & ROOM_LAYOUT_MASK);
}

static inline RoomFlags GetRoomPurpose(RoomFlags f) {
    return (f & ROOM_PURPOSE_MASK);
}

static inline RoomFlags GetRoomShape(RoomFlags f) {
    return (f & ROOM_SHAPE_MASK);
}

static inline RoomFlags GetRoomOrient(RoomFlags f) {
    return (f & ROOM_ORIENT_MASK);
}

static inline RoomFlags GetRoomPlacing(RoomFlags f) {
    return (f & ROOM_PLACING_MASK);
}

static inline RoomFlags GetRoomSpawn(RoomFlags f) {
    return (f & ROOM_SPAWN_MASK);
}

static void RoomDimensionsFromFlags(RoomFlags f, int *w, int *h) {
    switch (GetRoomSize(f) ) {
        case ROOM_SIZE_SMALL:   *w = 3; *h = 3; break;
        case ROOM_SIZE_MEDIUM:  *w = 5; *h = 5; break;
        case ROOM_SIZE_LARGE:   *w = 7; *h = 7; break;
        case ROOM_SIZE_XL:      *w = 9; *h = 9; break;
        case ROOM_SIZE_HUGE:    *w = 11; *h = 11; break;
        case ROOM_SIZE_MASSIVE: *w = 13; *h = 13; break;
        default:                *w = 4; *h = 4; break;
    }
}
static cell_bounds_t AdjustCellBounds(cell_bounds_t * bounds, Cell adj){

  cell_bounds_t out = {
    .center = CellInc(adj,bounds->center),
    .min = CellInc(adj,bounds->min),
    .max = CellInc(adj,bounds->max),
  };
  
  return out;
}

static int RoomConnectionsFromFlags(room_node_t *r, node_connector_t ** con){

  Rectangle bounds = RecFromBounds(&r->bounds);
  Rectangle inner = bounds;
  int sections = SplitBoundsFromRec(inner, 4, r->children, 16);
  for(int i = 0; i < sections; i++){
    r->children[i]->index = i;
    r->children[i]->owner = r;
    
    r->children[i]->pos = r->children[i]->range.center;
  }

  return sections;
}

static inline Cell RoomFacingFromFlag(RoomFlags f){
  Cell dir = CELL_EMPTY;
  switch(GetRoomPlacing(f)){
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

  return dir;
}

static char TileToChar(RoomFlags flags) {
    if (flags & TILEFLAG_FLOOR) return 'X';
    return 'O';
}

static char* GetPurposeName(RoomFlags f){
  switch(GetRoomPurpose(f)){
    case ROOM_PURPOSE_SECRET:
      return "SECRET";
      break;
    case ROOM_PURPOSE_TREASURE:
      return "TREASURE";
      break;
    case ROOM_PURPOSE_CHALLENGE:
      return "CHALLENGE";
      break;
    case ROOM_PURPOSE_LAIR:
      return "LAIR";
    default:
      return "PLAIN";
      break;
  }
}

static bool openings_face_each_other(room_opening_t *A, room_opening_t *B)
{
    // Example: A.dir = {1,0}, B.dir = {-1,0}
    return (A->dir.x == -B->dir.x) && (A->dir.y == -B->dir.y);
}
static int ClosestPointsBetweenBounds(
    cell_bounds_t *A,
    cell_bounds_t *B,
    Cell *outA,
    Cell *outB)
{
    // All candidate points in A
    Cell a_pts[3] = { A->min, A->center, A->max };
    Cell b_pts[3] = { B->min, B->center, B->max };

    int bestDist = 999;
    Cell bestA = {0};
    Cell bestB = {0};

    // Check all 9 combinations
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {

            int dist = cell_distance(a_pts[i], b_pts[j]);
            if (dist < bestDist) {
                bestDist = dist;
                bestA = a_pts[i];
                bestB = b_pts[j];
            }
        }
    }

    // Write results
    if (outA) *outA = bestA;
    if (outB) *outB = bestB;

    return bestDist;
}
static int overlap_value(cell_bounds_t a, cell_bounds_t b)
{
    int x0 = (a.min.x > b.min.x) ? a.min.x : b.min.x;
    int x1 = (a.max.x < b.max.x) ? a.max.x : b.max.x;

    int y0 = (a.min.y > b.min.y) ? a.min.y : b.min.y;
    int y1 = (a.max.y < b.max.y) ? a.max.y : b.max.y;

    int w = x1 - x0 + 1;
    int h = y1 - y0 + 1;

    if (w <= 0 || h <= 0)
        return -1;

    return w * h; // area of overlap
}

static bool TileFlagHasAccess(TileFlags f) {
              
  return (f & TILEFLAG_DOOR) || (f & TILEFLAG_FLOOR) || (f & TILEFLAG_EMPTY);
}             


static bool TileFlagBlocksMovement(TileFlags f)
{
    // Treat ANY of these as blocking
    return
        (f & TILEFLAG_SOLID)   ||
        (f & TILEFLAG_BORDER)  ||
        (f & TILEFLAG_WALL);
        // Add more if needed  
} 

static bool TileCellBlocksMovement(map_context_t* ctx, Cell c){
  RoomFlags f = ctx->tiles[c.x][c.y];

  return TileFlagBlocksMovement(f);
}
static bool TileBlocksMovement(map_cell_t *c) {
  if(c->occupant)
    return true;

  if(c->tile==NULL)
    return false;

  uint32_t f = EnvTileFlags[c->tile->type];
  return (f & TILEFLAG_SOLID) || (f & TILEFLAG_BORDER);
} 
    
static bool TileBlocksSight(map_cell_t *c) {
  if(c->tile==NULL)
    return false;

  uint32_t f = EnvTileFlags[c->tile->type];
  return (f & TILEFLAG_SOLID) || (f & TILEFLAG_OBSTRUCT);
}

   
static bool FindBestOpeningPair(room_t *A, room_t *B,room_opening_t **outA,room_opening_t **outB){
  int bestScore = -1; 
  room_opening_t *bestA = NULL;
  room_opening_t *bestB = NULL;  
    
  for (int i = 0; i < A->num_children; i++) {
    room_opening_t *oa = A->openings[i];
    if (!oa) continue;

    for (int j = 0; j < B->num_children; j++) {
      room_opening_t *ob = B->openings[j];
      if (!ob) continue;
  
      // Must point directly toward each other
      if (!openings_face_each_other(oa, ob))
        continue;

      // Score by overlapping bounds
      int score = overlap_value(oa->range, ob->range);
      if (score > bestScore) {
        bestScore = score;
        bestA = oa;
        bestB = ob;
      }
    }
  }

  if (!bestA || !bestB)
    return false;

  *outA = bestA;
  *outB = bestB;
  return true;
}

static inline bool TileHasFlag(EnvTile t, uint32_t flag) {
    return (EnvTileFlags[t] & flag) != 0;
} 

static inline bool TileHasAllFlags(EnvTile t, uint32_t flags) {
    return ( (EnvTileFlags[t] & flags) == flags );
} 
  
static inline bool TileHasAnyFlags(EnvTile t, uint32_t flags) {
    return (EnvTileFlags[t] & flags) != 0;
} 

static inline EnvTile GetTileByFlags(uint32_t flags) {
    for (int i = 0; i < ENV_DONE; i++) {
        if (TileHasAllFlags(i, flags))
            return (EnvTile)i;
    }
    return (EnvTile)-1; // NONE
}
static bool TileCellHasFlag(map_context_t* ctx, Cell c, RoomFlags f){
  EnvTile t = ctx->tiles[c.x][c.y];

  return TileHasFlag(t,f); 
}   

static bool room_has_access(map_context_t* ctx, cell_bounds_t room,Cell *access){
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

static void CastLight(map_grid_t *m, Cell pos,
               int row, float start, float end,
               int radius,
               int xx, int xy, int yx, int yy)
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

static bool FindPath(map_grid_t *m, int sx, int sy, int tx, int ty, Cell *outNextStep, int depth, TileBlock fn){
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

    depth = CLAMP(depth,20,80);
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
            if (!(nx == tx && ny == ty) &&fn(&m->tiles[nx][ny])) continue;

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
static inline define_race_class_t* GetRaceClassForSpec(EntityType spec, Profession prof)
{

    define_race_class_t *entry = &RACE_CLASS_DEFINE[spec][prof];

    // Check if this SPEC actually has an entry for this CLASS
    /*if (entry->race_class == 0)
        return NULL;
*/
    return entry;
}

static inline asi_bonus_t* GetAsiBonus(AttributeType attr,PhysQual mob_pq,
    MentalQual mob_mq,
        AsiEvent event)
{
    asi_bonus_t *out = calloc(1,sizeof(asi_bonus_t));

    const attribute_quality_t *aq = &ATTR_QUAL[attr];

    // Only give the bonus if the mob actually has the required qualities
    PhysQual required_pq = aq->body[event];
    MentalQual required_mq = aq->mind[event];

    if (required_pq & mob_pq)
        out->pq = true;

    if (required_mq & mob_mq)
        out->mq = true;

    if(out->mq||out->pq)
      return out;
    else
      return NULL;
}

static inline StatClassif GetStatClassif(
        StatType stat,
        PhysQual mob_pq,
        MentalQual mob_mq)
{
    const stat_quality_t *sq = &STAT_QUAL[stat];
    StatClassif best = SC_MIN;

    bool matched = false;
    for (int sc = SC_MIN; sc < SC_DONE; sc++)
    {
        bool matches_pq = (sq->stature[sc] & mob_pq) != 0;
        bool matches_mq = (sq->psyche[sc]  & mob_mq) != 0;

        if (matches_pq || matches_mq)
          return sc;
    }

    if(!matched)
      best = SC_AVERAGE;

    return best;
}

static choice_pool_t* GetRaceClassPool(EntityType type, int size, ChoiceFn fn){
  choice_pool_t* class_choice = InitChoicePool(size,ChooseByWeight);

  int count = 0;
  for(int i = 0; i < PROF_LABORER; i++){
    define_race_class_t* drc = GetRaceClassForSpec(type, i);
    if(drc)
    for(int j = 0; j<drc->count; j++){
      race_class_t c_class = drc->classes[j];
      int id = c_class.base; 
      AddChoice(class_choice, id, c_class.weight,&drc->classes[j], ChoiceReduceScore);
      count++;
    }
  }

  return class_choice;
}

static race_class_t* GetRaceClass(EntityType e, Archetype id){

  for(int i = 0; i < PROF_LABORER; i++){
    define_race_class_t drc = RACE_CLASS_DEFINE[e][i];

    for(int j = 0; j < drc.count; j++){
      if(drc.classes[j].base == id)
        return &RACE_CLASS_DEFINE[e][i].classes[j];
    }
  }

  return NULL;

}
#endif
