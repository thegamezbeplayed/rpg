#ifndef __GAME_GEN__ 
#define __GAME_GEN__ 


#define ABSTRACT_MAP_MIN   0
#define ABSTRACT_MAP_MAX   128
#define SECTION_SIZE 16
#define MAX_ROOM_SIZE 8
#define MAX_ROOMS  12
#define MAX_EDGES   128
#define MAX_ROOM_WIDTH 16
#define MAX_ROOM_HEIGHT 16
#define HAS_ANY_IN_CATEGORY(value, mask) ((value) & (mask))
#define IS_TRAIT(value, mask, trait) (((value) & (mask)) == (trait))
#define GET_FLAG(flag,mask) (flag&mask)
#define MOB_ROOM_MAX 10
#define MOB_MAP_MAX 64
#define QUAD_SIZE 16
#define MAX_OPTIONS 512  
#define ROOM_MOBS_SHIFT 20
#define ROOM_LAYOUT_SHIFT 8

typedef struct ent_s ent_t;

typedef enum{
  TILEFLAG_NONE        = 0,
  TILEFLAG_BORDER      = 1 << 0,
  TILEFLAG_EMPTY       = 1 << 1,   // blocks movement
  TILEFLAG_SOLID       = 1 << 2,   // blocks movement
  TILEFLAG_NATURAL     = 1 << 3,
  TILEFLAG_TREE        = 1 << 4,
  TILEFLAG_ROAD        = 1 << 5,
  TILEFLAG_FOREST      = 1 << 6,
  TILEFLAG_DEBRIS      = 1 << 7,
  TILEFLAG_DECOR       = 1 << 8,
  TILEFLAG_OBSTRUCT    = 1 << 9,
  TILEFLAG_SPAWN       = 1 << 10,
  TILEFLAG_FLOOR       = 1 << 11,
  TILEFLAG_WALL        = 1 << 12,
  TILEFLAG_DOOR        = 1 << 13,
  TILEFLAG_START       = 1 << 14,
  TILEFLAG_INTERACT    = 1 << 15,
  MAPFLAG_DUNGEON      = 1 << 16,
  MAPFLAG_FOREST       = 1 << 17,
}TileFlags;

static const uint32_t EnvTileFlags[ENV_DONE] = {
  [ENV_BONES_BEAST]    = TILEFLAG_DEBRIS | TILEFLAG_DECOR | MAPFLAG_FOREST | MAPFLAG_DUNGEON,
  [ENV_BOULDER]        = TILEFLAG_SOLID | TILEFLAG_DEBRIS | TILEFLAG_NATURAL,
  [ENV_COBBLE]         = TILEFLAG_ROAD,
  [ENV_COBBLE_WORN]    = TILEFLAG_ROAD,
  [ENV_FLOWERS]        = TILEFLAG_DEBRIS | TILEFLAG_DECOR | TILEFLAG_NATURAL,
  [ENV_FLOWERS_THIN]   = TILEFLAG_DEBRIS | TILEFLAG_DECOR | TILEFLAG_NATURAL,
  [ENV_FOREST_FIR]     = TILEFLAG_SOLID | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_GRASS]          = TILEFLAG_FLOOR | TILEFLAG_NATURAL,
  [ENV_GRASS_SPARSE]   = TILEFLAG_FLOOR | TILEFLAG_NATURAL,
  [ENV_GRASS_WILD]     = TILEFLAG_OBSTRUCT | TILEFLAG_NATURAL,
  [ENV_LEAVES]         = TILEFLAG_DECOR | TILEFLAG_NATURAL,
  [ENV_TREE_MAPLE]     = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_MEADOW]         = TILEFLAG_OBSTRUCT | TILEFLAG_NATURAL,
  [ENV_TREE_OLDGROWTH] = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_TREE_PINE]      = TILEFLAG_SOLID | TILEFLAG_WALL | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_ROAD]           = TILEFLAG_ROAD,
  [ENV_ROAD_CROSS]     = TILEFLAG_ROAD,
  [ENV_ROAD_FORK]      = TILEFLAG_ROAD,
  [ENV_ROAD_TURN]      = TILEFLAG_ROAD,
  [ENV_TREE_BIGLEAF]   = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_TREE_CEDAR]     = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_TREE_DEAD]      = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST,
  [ENV_TREE_DYING]     = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST,
  [ENV_TREE_FELLED]    = TILEFLAG_SOLID | TILEFLAG_DEBRIS | TILEFLAG_FOREST | TILEFLAG_NATURAL,  // updated
  [ENV_TREE_FIR]       = TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_FOREST]         = TILEFLAG_BORDER | TILEFLAG_SOLID| TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_WEB]            = TILEFLAG_DECOR|MAPFLAG_FOREST|MAPFLAG_DUNGEON,
  [ENV_DIRT]            = TILEFLAG_FLOOR,
  [ENV_DIRT_PATCH]            = TILEFLAG_FLOOR,
  [ENV_CAMP]          = TILEFLAG_SPAWN,
  [ENV_FLOOR_DUNGEON]  = MAPFLAG_DUNGEON | TILEFLAG_FLOOR, 
  [ENV_STONE_WALL]    = MAPFLAG_DUNGEON | TILEFLAG_WALL | TILEFLAG_SOLID,
  [ENV_WALL_DUNGEON]  = MAPFLAG_DUNGEON | TILEFLAG_WALL | TILEFLAG_SOLID,
  [ENV_WALL_RUIN]     = MAPFLAG_DUNGEON | TILEFLAG_WALL | TILEFLAG_SOLID,
  [ENV_DOOR_DUNGEON]  = MAPFLAG_DUNGEON | TILEFLAG_DOOR | TILEFLAG_OBSTRUCT | TILEFLAG_INTERACT,
  [ENV_DOOR_HEAVY]    = MAPFLAG_DUNGEON | TILEFLAG_DOOR | TILEFLAG_OBSTRUCT | TILEFLAG_INTERACT,
  [ENV_DOOR_VAULT]    = MAPFLAG_DUNGEON | TILEFLAG_DOOR | TILEFLAG_OBSTRUCT | TILEFLAG_INTERACT,
  [ENV_BORDER_DUNGEON]  = MAPFLAG_DUNGEON | TILEFLAG_BORDER | TILEFLAG_SOLID,
  [ENV_FURNITURE_CHAIR]  = MAPFLAG_DUNGEON | TILEFLAG_SPAWN | TILEFLAG_DECOR,
};

typedef enum {

    // ----- Size (bits 12–15) -----
    ROOM_SIZE_SMALL   = 0x1000,
    ROOM_SIZE_MEDIUM  = 0x2000,
    ROOM_SIZE_LARGE   = 0x3000,
    ROOM_SIZE_XL      = 0x4000,
    ROOM_SIZE_HUGE    = 0x5000,
    ROOM_SIZE_MASSIVE = 0x6000,
    ROOM_SIZE_MAX     = 0x7000,
    ROOM_SIZE_MASK    = 0xF000,

    // ----- Layout type (bits 8–11) -----
    ROOM_LAYOUT_NONE  = 0x0000,
    ROOM_LAYOUT_ROOM  = 0x0100,
    ROOM_LAYOUT_HALL  = 0x0200,
    ROOM_LAYOUT_OPEN  = 0x0300,
    ROOM_LAYOUT_MAZE  = 0x0400,
    ROOM_LAYOUT_MASK  = 0x0F00,
    
    // ----- Purpose (bits 4–7) -----
    ROOM_PURPOSE_NONE            = 0x0000,
    ROOM_PURPOSE_SIMPLE          = 0x0010,
    ROOM_PURPOSE_TRAPPED         = 0x0020,
    ROOM_PURPOSE_SECRET          = 0x0030,
    ROOM_PURPOSE_TREASURE        = 0x0040,
    ROOM_PURPOSE_TREASURE_FALSE  = 0x0050,
    ROOM_PURPOSE_CHALLENGE = 0x0060,
    ROOM_PURPOSE_LAIR      = 0x0070,
    ROOM_PURPOSE_START     = 0x0080,
    ROOM_PURPOSE_CONNECT   = 0x0090,
    ROOM_PURPOSE_MAX      = 0x00A0,
    ROOM_PURPOSE_MASK      = 0x00F0,

    // ----- Shape (bits 0–3) -----
    ROOM_SHAPE_SQUARE   = 0x0001,
    ROOM_SHAPE_CIRCLE   = 0x0002,
    ROOM_SHAPE_FORKED   = 0x0003,
    ROOM_SHAPE_CROSS    = 0x0004,
    ROOM_SHAPE_RECT     = 0x0005,
    ROOM_SHAPE_ANGLED   = 0x0006,
    ROOM_SHAPE_MAX      = 0x0007,
    ROOM_SHAPE_MASK     = 0x000F,

    // --- Orientation (bits 16-19) ---
    ROOM_ORIENT_HOR   = 0x00010000,
    ROOM_ORIENT_VER   = 0x00020000,
    ROOM_ORIENT_MASK   = 0x00030000,

    ROOM_PLACING_C = 0x00000000,
    ROOM_PLACING_N = 0x00100000,
    ROOM_PLACING_S = 0x00200000,
    ROOM_PLACING_E = 0x00300000,
    ROOM_PLACING_W = 0x00400000,
    ROOM_PLACING_NW = 0x00500000,
    ROOM_PLACING_NE = 0x00600000,
    ROOM_PLACING_SE = 0x00700000,
    ROOM_PLACING_SW = 0x00800000,
    ROOM_PLACING_MAX = 0x00900000,
    ROOM_PLACING_MASK = 0x00F00000,

    ROOM_SPAWN_SOLO = 0x01000000,
    ROOM_SPAWN_PAIR = 0x02000000,
    ROOM_SPAWN_TRIO = 0x03000000,
    ROOM_SPAWN_GROUP = 0x04000000,
    ROOM_SPAWN_PACK = 0x05000000,
    ROOM_SPAWN_CAMP = 0x06000000,
    ROOM_SPAWN_MAX = 0x07000000,
    ROOM_SPAWN_MASK = 0x0F000000,

} RoomFlags;

static int room_size_weights[7] = {0,9,16,21,67,82,96};
static int room_purpose_weights[8] = {5,20,23,29,32,34,63,99};
static int room_shape_weights[7] = {50,20,10,10,5,20,15};

typedef uint64_t MobRules;

typedef enum{
  MOB_SPAWN_TRAP        = BIT64(0),
  MOB_SPAWN_LAIR        = BIT64(1),
  MOB_SPAWN_CHALLENGE   = BIT64(2),
  MOB_SPAWN_SECRET      = BIT64(3),
  MOB_SPAWN_CAMP        = BIT64(4),
  MOB_SPAWN_PATROL      = BIT64(5),
  MOB_SPAWN_MASK        = 0xFFULL,

  MOB_MOD_ENLARGE       = BIT64(8),
  MOB_MOD_WEAPON        = BIT64(9),
  MOB_MOD_ARMOR         = BIT64(10),
  MOB_MOD_MASK          = 0xFFULL << 8,

  MOB_LOC_DUNGEON       = BIT64(16),
  MOB_LOC_CAVE          = BIT64(17),
  MOB_LOC_FOREST        = BIT64(18),
  MOB_LOC_MASK          = 0xFFULL << 16,

  MOB_THEME_CRITTER     = BIT64(24),
  MOB_THEME_PRIMITIVE   = BIT64(25),
  MOB_THEME_MARTIAL     = BIT64(26),
  MOB_THEME_MASK        = 0xFFULL << 24,

  MOB_FREQ_COMMON      = BIT64(32),
  MOB_FREQ_UNCOMMON    = BIT64(33),
  MOB_FREQ_RARE        = BIT64(34),
  MOB_FREQ_ELUSIVE     = BIT64(35),
  MOB_FREQ_LIMITED     = BIT64(36),
  MOB_FREQ_MASK        = 0xFFULL << 32,

  MOB_GROUPING_SOLO   = BIT64(40),
  MOB_GROUPING_PAIRS  = BIT64(41),
  MOB_GROUPING_TROOP  = BIT64(42),
  MOB_GROUPING_PARTY  = BIT64(43),
  MOB_GROUPING_CREW   = BIT64(44),
  MOB_GROUPING_SQUAD  = BIT64(45),
  MOB_GROUPING_WARBAND= BIT64(46),
  MOB_GROUPING_SWARM  = BIT64(47),
  MOB_GROUPING_MASK   = 0xFFULL << 40
}MobRule;

typedef struct{
  MobRules    rules;
  int         max_mobs,max_room_mobs;
}spawn_rules_t;

typedef struct {
    TileFlags required_floor;   // must be standing on: TILEFLAG_FLOOR, FOREST, etc
    TileFlags forbidden;        // cannot overlap
    TileFlags deco_flag;        // TILEFLAG_DECOR, TILEFLAG_DEBRIS, etc
    int chance;                 // 1 = always, 5 = 1/5, 10 = 1/10
} DecorRule;

static DecorRule dungeon_decor[] = {
    { TILEFLAG_FLOOR, TILEFLAG_SPAWN | TILEFLAG_START, TILEFLAG_DEBRIS, 12 },
    { TILEFLAG_FLOOR, TILEFLAG_SPAWN,                  TILEFLAG_DECOR,  15 },
    { TILEFLAG_FLOOR, TILEFLAG_WALL,                  TILEFLAG_DECOR,  20 }
};
static const int dungeon_decor_count = sizeof(dungeon_decor)/sizeof(dungeon_decor[0]);

typedef struct {
    Cell min, max;
} cell_bounds_t;
typedef struct room_s room_t;

typedef struct{
  bool          entrance;
  struct room_s *sub;
  Cell          dir, pos;
}room_opening_t;

struct room_s{
  cell_bounds_t   bounds;
  int             num_children;
  RoomFlags       flags;
  room_opening_t  openings[16];
  Cell            center,dir;
  int             num_mobs;
  ent_t           *mobs[MOB_ROOM_MAX];
};

typedef enum {
    MAP_NODE_SUCCESS,
    MAP_NODE_FAILURE
} MapNodeResult;

typedef enum {
  MAP_NODE_ROOT,
  MAP_NODE_SEQUENCE,
  MAP_NODE_LEAF,
} MapNodeType;

typedef enum {
  MN_GRID,
  MN_SELECT_ROOMS,
  MN_FILL_MISSING_ROOMS,
  MN_GENERATE_ROOMS,
  MN_GRID_GEN,
  MN_MAP_OUT_GRID,
  MN_APPLY_SHAPES,
  MN_PLACE_ROOMS,
  MN_COMPUTE_BOUNDS,
  MN_ALLOCATE_TILES,
  MN_CARVE_TILES,
  MN_GRAPH_ROOTS,
  MN_BUILD,
  MN_CONNECT_SUB,
  MN_DETAIL,
  MN_ENCASE_FLOORS,
  MN_PLACE_POI,
  MN_SET_PLAYER,
  MN_PLACE_SPAWNS,
  MN_OPT,
  MN_OPT_INIT,
  MN_SCAN,
  MAP_NODE_PLACE_ROOMS,
  MAP_NODE_BUILD_GRAPH,
  MAP_NODE_CONNECT_HALLS,
  MAP_NODE_PLACE_SPAWNS,
  MAP_NODE_DECORATE,
  MAP_NODE_DONE
} MapNodeID;

struct room_gen_s;
typedef struct room_gen_s{
  int                col,row;
  RoomFlags          flags;
}room_gen_t;

typedef struct {
Cell  top_left;
int   wid,hei, num_rooms;
Cell  rooms[4];
}room_quad_t;

bool QuadHasRoomAt(room_quad_t* q, int x, int y);
typedef struct{
  MapID           id;
  TileFlags       map_flag;
  int             quad_col,quad_row,density,num_rooms;
  room_quad_t     *quads[32][32];
  spawn_rules_t   mobs;
  room_gen_t      rooms[12];
  MapNodeID       node_rules;
}map_gen_t;

typedef struct {
  map_gen_t   *map_rules;
  int         width, height, num_rooms;
  room_t      rooms[MAX_ROOMS];
  Cell        player_start;
  TileFlags **tiles;
  int alloc_w, alloc_h;
  // random seed etc if you want
  unsigned int seed;
  int         decor_density;
} map_context_t;

typedef enum {
    OPT_ROOM,
    OPT_CORRIDOR,
    OPT_FEATURE,
} OptionType;

// forward declare
struct option_s;

typedef void (*ApplyFn)(struct option_s *opt, map_gen_t *ctx);

typedef struct option_s {
    OptionType type;
    int        score;
    RoomFlags  flags;
    // placement information
    Cell       pos;
    int        row,col,w, h;   // room size or corridor length

    ApplyFn apply;
} option_t;

typedef struct {
    option_t items[MAX_OPTIONS];
    int count;
} option_pool_t;

option_t *PickBestOption(map_context_t *context);
void MapAddOption(option_t opt);
void ApplyRoom(option_t *opt, map_gen_t *map);
option_t TryRoomOption(map_gen_t *map, RoomFlags flags, int row, int col, int x, int y);
option_t TryCorridorOption(map_context_t *map, int x, int y);
bool HasFloorNeighbor(map_context_t* ctx, int x, int y);
struct map_node_s;
typedef struct map_node_s map_node_t;

typedef MapNodeResult (*MapNodeFn)(map_context_t *ctx, map_node_t *node);

void RefreshOptionPool(map_context_t *context, map_node_t* node);

MapNodeResult MapGenInit(map_context_t *context, map_node_t *node);
MapNodeResult MapGenScan(map_context_t *context, map_node_t *node);
MapNodeResult MapGenRun(map_context_t *context, map_node_t *node);

MapNodeResult MapFillMissing(map_context_t *ctx, map_node_t *node);
MapNodeResult MapPlaceSpawns(map_context_t *ctx, map_node_t *node);
MapNodeResult MapPlayerSpawn(map_context_t *ctx, map_node_t *node);
MapNodeResult MapFillWalls(map_context_t *ctx, map_node_t *node);
MapNodeResult MapGraphRooms(map_context_t *ctx, map_node_t *node);
MapNodeResult MapApplyRoomShapes(map_context_t *ctx, map_node_t *node);
MapNodeResult MapPlaceSubrooms(map_context_t *ctx, map_node_t *node);
MapNodeResult MapCarveTiles(map_context_t *ctx, map_node_t *node);
MapNodeResult MapAllocateTiles(map_context_t *ctx, map_node_t *node);
MapNodeResult MapAssignPositions(map_context_t *ctx, map_node_t *node);
MapNodeResult MapGridLayout(map_context_t *ctx, map_node_t *node);
MapNodeResult MapGenerateRooms(map_context_t *ctx, map_node_t *node);
MapNodeResult MapComputeBounds(map_context_t *ctx, map_node_t *node);
MapNodeResult MapConnectSubrooms(map_context_t *ctx, map_node_t *node);

struct map_node_s {
    MapNodeType type;
    MapNodeID   id;
    MapNodeFn   run;
    int         num_children;
    map_node_t  **children;
};

typedef struct{
  MapNodeID     id;
  MapNodeType   type;
  map_node_t*   (*fn)(MapNodeID id );
  int           num_children;
  MapNodeType   children[6];
}map_node_data_t;

map_node_t* MapCreateLeafNode(MapNodeFn fn, MapNodeID id);
map_node_t* MapCreateSequence( MapNodeID id, map_node_t **children, int count);

static inline map_node_t* LeafMapFillMissing(MapNodeID id)  { return MapCreateLeafNode(MapFillMissing,id); }
static inline map_node_t* LeafMapGenRun(MapNodeID id)  { return MapCreateLeafNode(MapGenRun,id); }
static inline map_node_t* LeafMapGenInit(MapNodeID id)  { return MapCreateLeafNode(MapGenInit,id); }
static inline map_node_t* LeafMapPlaceSpawns(MapNodeID id)  { return MapCreateLeafNode(MapPlaceSpawns,id); }
static inline map_node_t* LeafMapFillWalls(MapNodeID id)  { return MapCreateLeafNode(MapFillWalls,id); }
static inline map_node_t* LeafMapPlayerSpawn(MapNodeID id)  { return MapCreateLeafNode(MapPlayerSpawn,id); }
static inline map_node_t* LeafMapConnectSubrooms(MapNodeID id)  { return MapCreateLeafNode(MapConnectSubrooms,id); }
static inline map_node_t* LeafMapGraphRooms(MapNodeID id)  { return MapCreateLeafNode(MapGraphRooms,id); }
static inline map_node_t* LeafMapApplyRoomShapes(MapNodeID id)  { return MapCreateLeafNode(MapApplyRoomShapes,id); }
static inline map_node_t* LeafMapPlaceSubrooms(MapNodeID id)  { return MapCreateLeafNode(MapPlaceSubrooms,id); }
static inline map_node_t* LeafMapAllocateTiles(MapNodeID id)  { return MapCreateLeafNode(MapAllocateTiles,id); }
static inline map_node_t* LeafMapCarveTiles(MapNodeID id)  { return MapCreateLeafNode(MapCarveTiles,id); }
static inline map_node_t* LeafMapComputeBounds(MapNodeID id)  { return MapCreateLeafNode(MapComputeBounds,id); }
static inline map_node_t* LeafMapAssignPositions(MapNodeID id)  { return MapCreateLeafNode(MapAssignPositions,id); }
static inline map_node_t* LeafMapGridLayout(MapNodeID id)  { return MapCreateLeafNode(MapGridLayout,id); }
static inline map_node_t* LeafMapGenerateRooms(MapNodeID id)  { return MapCreateLeafNode(MapGenerateRooms,id); }

map_node_t* MapBuildNodeRules(MapNodeType id);
MapNodeResult MapNodeRunSequence(map_context_t *ctx, map_node_t *node);
map_node_t* MapBuildRootPipeline(void);
bool MapGenerate(map_context_t *ctx);
MapNodeResult NodeAssignPositions(map_context_t *ctx, map_node_t *node);
MapNodeResult NodeBuildRoomGraph(map_context_t *ctx, map_node_t *node);
void CarveHallBetween(map_context_t *ctx, Cell a, Cell b);
MapNodeResult NodeAllocateTiles(map_context_t *ctx, map_node_t *node);
MapNodeResult NodeCarveToTiles(map_context_t *ctx, map_node_t *node);
MapNodeResult NodeConnectHalls(map_context_t *ctx, map_node_t *node);
MapNodeResult NodeDecorate(map_context_t *ctx, map_node_t *node);
TileFlags RoomSpecialDecor(RoomFlags p);

bool TooClose(Cell a, Cell b, int min_dist);
int QuadNeighbors(map_gen_t* map, int row, int col);
static int RoomsOverlap(room_t *a, room_t *b, int spacing) {

    int dist = CellDistGrid(a->center, b->center);

    RoomFlags asize = a->flags & ROOM_SIZE_MASK;
    RoomFlags bsize = b->flags & ROOM_SIZE_MASK;
    int ar = (asize >> 12) + spacing;
    int br = (bsize >> 12) + spacing;

    return imax(ar-dist,br-dist);
}

#endif
