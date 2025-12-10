#ifndef __GAME_TOOLS__
#define __GAME_TOOLS__
#include <float.h>
#include "game_math.h"
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define CLAMPV2(v,a,b) ((v)<(a)?(a):((v)>(b)?(b):(v)))
#define VEC_UNSET (Vector2){FLT_MAX, FLT_MAX}
#define EPS 1e-6f
#define VECTOR2_CENTER_SCREEN   (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2}
#define VECTOR2_SCREEN   (Vector2){ GetScreenWidth(), GetScreenHeight()}
#define VECTOR2_ZERO   (Vector2){ 0.0f, 0.0f}
#define VECTOR2_ONE    (Vector2){ 1.0f, 1.0f }
#define VECTOR2_UP     (Vector2){ 0.0f, -1.0f }
#define VECTOR2_DOWN   (Vector2){ 0.0f,1.0f }
#define VECTOR2_LEFT   (Vector2){-1.0f, 0.0f }
#define VECTOR2_RIGHT  (Vector2){ 1.0f, 0.0f }
#define Vector2X(x) ((Vector2){ (x), 0.0f })
#define Vector2XY(x,y) ((Vector2){ (x), (y) })
#define Vector2Y(y) ((Vector2){ 0.0f, (y) })
#define Vector2Inc(v,xi,yi) ((Vector2){ (v.x+xi), (v.y+yi) })
#define RectStart(r1,r2) ((Vector2){(r1.width / 2 - r2.width/2),(r1.height/2 - r2.height/2)})
#define RectPos(v,r) ((Rectangle){(v.x),(v.y),(r.width),(r.height)})
#define RectSize(r) ((Vector2){(r.width),(r.height)})
#define RectXY(r) ((Vector2){(r.x),(r.y)})
#define Rect(px,py,sx,sy) ((Rectangle){ (px),(py), (sx), (sy) })
#define RECT_ZERO   (Rectangle){ 0.0f, 0.0f,0.0f,0.0f}
#define RectArea(r) (int){(r.width)*(r.height)}
#define RectInner(r,i) (Rectangle){(r.x+i),(r.y+i),(r.width-2*i),(r.height-2*i)}
#define RectInc(r,xi,yi) ((Rectangle){ (r.x+xi), (r.y+yi),(r.width),(r.height) })
#define RectScale(r,s) ((Rectangle){ (r.x), (r.y),(r.width * s),(r.height * s) })
#define CELL_EMPTY (Cell){0,0}
#define CELL_UNSET (Cell){-1,-1}
#define CELL_ONE    (Cell){ 1, 1 }
#define CELL_UP     (Cell){ 0, -1 }
#define CELL_DOWN   (Cell){ 0,1 }
#define CELL_LEFT   (Cell){-1, 0 }
#define CELL_RIGHT  (Cell){ 1, 0 }
#define CellScale(c,s) (Cell){ (c.x*s),(c.y*s)}
#define CellInc(c1,c2) ((Cell){ (c1.x+c2.x), (c1.y+c2.y) })
#define CELL_NEW(x,y) ((Cell){(x),(y)})
#define CellMul(c1,c2) (Cell){(c1.x * c2.x),(c1.y * c2.y)}
#define CellSub(c1,c2) (Cell){(c1.x - c2.x),(c1.y - c2.y)}
#define CellFlip(c) (Cell){(c.y),(c.x)}

bool SaveCharGrid( int width, int height, char grid[][width],const char *filename);
static void shuffle_array(void *base, size_t n, size_t size) {
    char *arr = base;
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        char tmp[size];
        memcpy(tmp, arr + i * size, size);
        memcpy(arr + i * size, arr + j * size, size);
        memcpy(arr + j * size, tmp, size);
    }
}

static uint32_t hash_str(const char *str) {
    uint32_t hash = 5381; // djb2 starting seed
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + (uint32_t)c; // hash * 33 + c
    return hash;
}

typedef struct {
  int x,y;
} Cell;

static inline int CellDistGrid(Cell c1,Cell c2){
  return abs( c2.x-c1.x) + abs( c2.y-c1.y);


}

static inline Cell cell_dist(Cell c1, Cell c2){
  return CELL_NEW(abs(c1.x)+abs(c2.x),abs(c1.y)+abs(c2.y));
}

static inline Cell random_direction(void){
  switch(rand()%4){
    case 0: return CELL_UP;
    case 1: return CELL_DOWN;
    case 2: return CELL_LEFT;
    default: return CELL_RIGHT;
  }
}

static inline Cell cell_point_along(Cell c, int min){
  if(c.x>c.y)
    c.x = RandRange(min,c.x-1);
  else
    c.y = RandRange(min,c.y-1);

  return c;
}

static inline bool cell_in_bounds(Cell c, Cell bounds){
  if (c.x >= bounds.x || c.x < 0)
    return false;

  return (c.y <= bounds.y || c.x > 0);
}

static inline Cell cell_random_range(int min, int max){
  return CELL_NEW(RandRange(min,max),RandRange(min,max));
}
static inline bool cell_compare(Cell c1,Cell c2){
  return (c1.x==c2.x && c1.y==c2.y);
}

static bool cells_linear(Cell a, Cell b) {
    return (a.x == b.x) || (a.y == b.y);
}

static inline int CellClusterAround(Cell c, int amnt, int space, int dist, Cell* output){

  int num = 0;
  for(int i = 0; i < amnt; i++){
    Cell pt = random_direction();
    int rdist = RandRange(1,dist);
    Cell npt = CellInc(c,CellScale(pt,rdist));
    for(int j = 0; j < i; j++)
      if (cell_compare(output[j],npt)){
        i--;
        continue;
      }

    num++;
    output[i]=npt;
  }

  return num;
}
static inline Cell cell_inc_rect(Cell c, Rectangle r){
  Cell start = CellInc(CELL_NEW(r.x,r.y),CELL_NEW(r.width,r.height));

  return CellInc(start, c);
}

static inline Cell cell_dir(Cell start, Cell end){
  int tx = end.x;
  int ty = end.y;
  int sx = start.x;
  int sy = start.y;

  int ax = abs(tx - sx);
  int ay = abs(ty - sy);

  int dx =0, dy = 0;
  if (ax > ay) {
    dx = (tx > sx) - (tx < sx);
    dy = 0;
  } else {
    dx = 0;
    dy = (ty > sy) - (ty < sy);
  }

  return CELL_NEW(dx,dy);
}

static bool is_adjacent(Cell c1, Cell c2)
{
  int x1 = c1.x;
  int x2 = c2.x;
  int y1 = c1.y;
  int y2 = c2.y;
  int dx = abs(x1 - x2);
  int dy = abs(y1 - y2);

  // Adjacent if exactly 1 step away horizontally or vertically
  return (dx + dy == 1);
}

static inline float distance(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    return sqrtf(dx*dx + dy*dy);
}

static inline float frand() {
    return (float)rand() / (float)RAND_MAX;
}

static inline Vector2 Vector2Avg(Vector2 a, Vector2 b){
 return (Vector2){ (a.x + b.x) * 0.5f,
                      (a.y + b.y) * 0.5f };
}
static inline Cell vec_to_cell(Vector2 v,float scale){
  return CELL_NEW(v.x/scale,v.y/scale);
}

static inline Vector2 CellToVector2(Cell c, float scale){
  Vector2 result = Vector2FromXY(c.x,c.y);

  return Vector2Scale(result,scale);
}
static inline bool v2_compare(Vector2 v1,Vector2 v2){
  return (v1.x==v2.x && v1.y==v2.y);
}
static inline float v2_ang_deg(Vector2 v){ return atan2(v.y,v.x)*180.0 /M_PI;}
static inline float v2_ang(Vector2 v){ return atan2(v.y,v.x);}
static inline float v2_len(Vector2 v){ return sqrtf(v.x*v.x + v.y*v.y); }
static inline Vector2 v2_add(Vector2 a, Vector2 b){ return (Vector2){a.x+b.x,a.y+b.y}; }
static inline Vector2 v2_sub(Vector2 a, Vector2 b){ return (Vector2){a.x-b.x,a.y-b.y}; }
static inline Vector2 v2_scale(Vector2 a, float s){ return (Vector2){a.x*s,a.y*s}; }
static inline Vector2 v2_norm_safe(Vector2 v){
  float L = v2_len(v);
  return (L > 1e-6f) ? v2_scale(v, 1.0f/L) : (Vector2){1,0};
}

static inline bool cell_in_rect(Cell p, Rectangle r){
  return (p.x >= r.x && p.x <= r.x + r.width &&
      p.y >= r.y && p.y <= r.y + r.height);
}


static inline bool point_in_rect(Vector2 p, Rectangle r){
  return (p.x >= r.x && p.x <= r.x + r.width &&
      p.y >= r.y && p.y <= r.y + r.height);
}

static inline Vector2 clamp_point_to_rect(Vector2 p, Rectangle r){
  return (Vector2){
    CLAMPV2(p.x, r.x, r.x + r.width),
      CLAMPV2(p.y, r.y, r.y + r.height)
  };
}

static inline Cell clamp_cell_to_bounds(Cell c, Rectangle r){
  return (Cell){
    CLAMPV2(c.x, r.x, r.x + r.width),
      CLAMPV2(c.y, r.y, r.y + r.height)
  };

}
// Random unit vector (use your RNG if needed)
static inline Vector2 rand_unit(){
  float a = ((float)rand() / (float)RAND_MAX) * 6.28318530718f;
  return (Vector2){cosf(a), sinf(a)};
}
static inline Rectangle clamp_rect_to_bounds(Rectangle r, Rectangle b){
  Cell pos = clamp_cell_to_bounds(CELL_NEW(r.x,r.y),b);
  Cell end = CellInc(pos,CELL_NEW(r.width,r.height));
  Cell size = cell_dist(clamp_cell_to_bounds(end, b),pos);
  return Rect(pos.x,pos.y,size.x,size.y);
}
#endif
