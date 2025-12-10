#ifndef __GAME_HELPER_H
#define __GAME_HELPER_H

#include "game_common.h"


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

static cell_bounds_t RoomBounds(room_t* r,Cell c){

  RoomFlags size = r->flags & ROOM_SIZE_MASK;
  RoomFlags purpose = r->flags & ROOM_PURPOSE_MASK;
  RoomFlags layout = r->flags & ROOM_LAYOUT_MASK;
  RoomFlags orient = r->flags & ROOM_ORIENT_MASK;
  RoomFlags shape = r->flags & ROOM_SHAPE_MASK;

  int isize = size >> 12;
  int bstep = 0;
  int lstep = 0;

  int special = 0;

  switch(layout){
    case ROOM_LAYOUT_HALL:
      isize = size>>11;
      bstep = size>>15;
      lstep = imax(-1+size>>11,2);
     break;
    case ROOM_LAYOUT_OPEN:
     lstep = bstep = CLAMP(size>>12,1,3);
    default:
     break;
  }

  switch(purpose){
    case ROOM_PURPOSE_CONNECT:
      bstep=0;
      lstep-=1;
      break;
    default:
      break;
  }

  if(size == ROOM_SIZE_SMALL)
    special = 1;

  int minx = c.x-isize;
  int miny = c.y-isize;
  int maxx = c.x+isize+special;
  int maxy = c.y+isize+special;


  switch(orient){
    case ROOM_ORIENT_HOR:
      if(size>ROOM_SIZE_SMALL)
        miny+=lstep;

      maxy-=lstep;

      minx-=bstep;
      maxx+=bstep;

      break;
    case ROOM_ORIENT_VER:
      if(size>ROOM_SIZE_SMALL)
        minx+=lstep;
      if(purpose == ROOM_PURPOSE_CONNECT)
        lstep = 0;

      maxx-=lstep;
      miny-=bstep;
      maxy+=bstep;
      break;
    default:
      break;
  }

  cell_bounds_t output= {CELL_NEW(minx,miny),CELL_NEW(maxx,maxy)};
  return output;
}

static Cell RoomSize(room_t* r){
  Cell output = CELL_EMPTY;
  cell_bounds_t bounds = RoomBounds(r,CELL_EMPTY);

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


static int SizeToRadius(RoomFlags size, RoomFlags layout) {
  int output = 0;
  switch(layout){
    case ROOM_LAYOUT_HALL:
      output = imax(1,size >> 13);
      break;
    case ROOM_LAYOUT_OPEN:
      output = size >> 11;
      break;
    default:
      output = size >> 12;
      break;
  }
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

static inline RoomFlags PurposeByWeight(RoomFlags max, int budget){
  int r = RandRange(0,budget);

  int category_size = (max>>12)-1;
  for (int i = category_size; i > 1; i--){
    if(room_purpose_weights[i] < r)
      return i<<12;
  }

  return 1<<12;
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

static inline RoomFlags MobCountBySize(RoomFlags size){
  return (RoomFlags)(size<<12);
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
static inline RoomFlags RandomLayout(void) {
    int count = (ROOM_LAYOUT_MASK) >> ROOM_LAYOUT_SHIFT; // = 4 layouts
    int pick  = RandRange(0, count - 1);                   // 0..3
    return (RoomFlags)((pick + 1) << 8);
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
        case ROOM_SIZE_SMALL:   *w = 4; *h = 3; break;
        case ROOM_SIZE_MEDIUM:  *w = 6; *h = 4; break;
        case ROOM_SIZE_LARGE:   *w = 8; *h = 6; break;
        case ROOM_SIZE_XL:      *w = 10; *h = 8; break;
        case ROOM_SIZE_HUGE:    *w = 12; *h = 9; break;
        case ROOM_SIZE_MASSIVE: *w = 14; *h = 12; break;
        default:                *w = 6; *h = 4; break;
    }
}

static char TileToChar(RoomFlags flags) {
    if (flags & TILEFLAG_FLOOR) return 'X';
    return 'O';
}
#endif
