#ifndef __GAME_MATH__
#define __GAME_MATH__
#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>
#include "reasings.h"

#define CLAMP(x, low, high) (((x) < (low)) ? (low) : (((x) > (high)) ? (high) : (x)))
#define CLAMPF(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))


static inline Vector2 Vector2FromXY(float x, float y){
  Vector2 result = {
    x,y
  };

  return result;
}

static inline Vector2 Vector2FromAngle(float a, float len){
  return (Vector2){
    .x = cosf(a) * len,
    .y = sinf(a) * len
  };
}

static inline Vector2 Vector2Bisect(Vector2 a, Vector2 b, float dist){
  Vector2 dir = Vector2Normalize(Vector2Subtract(b,a));

  return Vector2Add(a,Vector2Scale(dir,dist));
}

static inline Rectangle RectangleCrop(Rectangle rec, float ex_wid,float ex_hei){
  return (Rectangle){
    .x = rec.x + ex_wid,
    .y = rec.y + ex_hei,
    .width = rec.width - ex_wid,
    .height = rec.height - ex_hei
  };
}

static inline Rectangle RecFromCoords(int x, int y, int width, int height){

  Rectangle result = {
    x,y,width,height
  };

  return result;
}

static inline Vector2 VectorDistanceBetween(Vector2 a, Vector2 b) {
  return Vector2Subtract(b,a);
}

static inline Vector2 VectorDirectionBetween(Vector2 a, Vector2 b) {
    return Vector2Normalize((Vector2){ b.x - a.x, b.y - a.y });
}

static inline Rectangle RectangleScale(Rectangle r, float scale){
  r.width*=scale;
  r.height*=scale;

  return r;
}

static inline Vector2 GetRecCenter(Rectangle rec){
  Vector2 result = {
    rec.x+rec.width/2.0f,
    rec.y+rec.height/2.0f
  };

  return result;
}

static inline Rectangle GetIntersectionRec(Rectangle r1, Rectangle r2){
  float x = fmaxf(r1.x, r2.x);
  float y = fmaxf(r1.y, r2.y);
  float w = fminf(r1.x + r1.width, r2.x + r2.width) - x;
  float h = fminf(r1.y + r1.height, r2.y + r2.height) - y;

  if (w <= 0 || h <= 0) return (Rectangle){0}; // no overlap
  return (Rectangle){ x, y, w, h };
}

static int isqrt(int n) {
    int x = n;
    int y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}
static int RandRange(int min, int max) {
    return min + rand() % (max - min + 1);
}

static inline int imin(int a, int b) {
    return (a < b) ? a : b;
}

static inline int imax(int a, int b) {
    return (a > b) ? a : b;
}

static int clamp_to_interval(int val, int ntrvl){
  if(ntrvl <=0)
    return val;
  return((val + ntrvl -1)/ntrvl)*ntrvl;
}

static inline bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;

    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    }
    return true;
}

static inline bool is_composite(int n) {
    return (n > 1 && !is_prime(n));
}

static inline bool is_perfect_square(int n) {
    if (n < 1) return false;
    int r = (int)sqrt((double)n);
    return r * r == n;
}
#endif
