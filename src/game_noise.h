#ifndef __GAME_NOISE__
#define __GAME_NOISE__

typedef struct {
    uint64_t state;
} rng_t;

static void InitRng(rng_t** rng, uint64_t seed) {
  *rng = GameCalloc("InitRng", 1, sizeof(rng_t));
  (*rng)->state = seed;
}

static uint64_t RngNext(rng_t* rng) {
    uint64_t z = (rng->state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static int RngRange(rng_t* rng, int min, int max) {
    return min + (RngNext(rng) % (max - min + 1));
}

static uint64_t RngRollUID(rng_t* rng, int start, int end){

  int index = RngRange(rng, start, end);

  return BIT64(index);
}

static int RngRoll(rng_t* rng, int start, int end){
  return RngRange(rng, start, end);

}
#endif
