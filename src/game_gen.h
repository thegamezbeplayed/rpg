#ifndef __GAME_GEN__ 
#define __GAME_GEN__ 
#include "game_tools.h"
#include "game_enum.h"

#define MAX_CACHED_PATHS 128
#define MAX_PATH_LEN    128

#define MAX_OPTIONS 256
#define MAX_ROOMS  128
#define HAS_ANY_IN_CATEGORY(value, mask) ((value) & (mask))
#define IS_TRAIT(value, mask, trait) (((value) & (mask)) == (trait))
#define GET_FLAG(flag,mask) (flag&mask)
#define MOB_ROOM_MAX 10
#define MOB_MAP_MAX 64
#define ROOM_MOBS_SHIFT 20
#define ROOM_LAYOUT_SHIFT 8
#define ROOM_PURPOSE_SHIFT 4
#define MAX_NODE_DEPTH 5
#define MAX_ANCHOR_NODES 10
#define MAX_ATTEMPTS 500

#define GRID_WIDTH 128
#define GRID_HEIGHT 128

#define TILE_SIZE_MASK (\
    TILEFLAG_SIZE_XS  |\
    TILEFLAG_SIZE_SM  |\
    TILEFLAG_SIZE_MED |\
    TILEFLAG_SIZE_L   |\
    TILEFLAG_SIZE_XL  |\
    TILEFLAG_SIZE_MAX )

#define ROOM_PURPOSE_LEVEL_EVENT_MASK (\
    ROOM_PURPOSE_SECRET | ROOM_PURPOSE_TREASURE |\
    ROOM_PURPOSE_CHALLENGE | ROOM_PURPOSE_LAIR |\
    ROOM_PURPOSE_CAMP)

#define GROUPING_SHIFT (__builtin_ctzll(MOB_GROUPING_MASK))
typedef struct map_grid_s map_grid_t;
typedef struct choice_pool_s choice_pool_t;
typedef struct local_ctx_s local_ctx_t;
typedef struct ent_s ent_t;
typedef struct env_s env_t;

//#include "game_utils.h"
typedef enum{
  GEN_NONE,
  GEN_DONE,
  GEN_RUNNING,
  GEN_ISSUES,
}GenStatus;
GenStatus MapGetStatus(void);

typedef enum{
  RS_CHECK,
  RS_ORPHANED,
  RS_WHERE_IS_THE_DOOR,
  RS_DOOR_INSIDE,
  RS_NO_PATH,
  RS_RESOLVE,
  RS_RESOLVED,
  RS_ISSUE,
  RS_NO_VIABLE_PATH,
  RS_NO_VIABLE_DOOR,
  RS_NO_ISSUE,
  RS_CARVE_DOOR,
  RS_PATH_FOUND,
  RS_FIXED,
  RS_GTG,
  RS_CARVED,
  RS_CORRECTED
}RoomStatus;

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
  TILEFLAG_BONE        = 1 << 8,
  TILEFLAG_STONE       = 1 << 9,
  TILEFLAG_DECOR       = 1 << 10,
  TILEFLAG_OBSTRUCT    = 1 << 11,
  TILEFLAG_SPAWN       = 1 << 12,
  TILEFLAG_FLOOR       = 1 << 13,
  TILEFLAG_WALL        = 1 << 14,
  TILEFLAG_DOOR        = 1 << 15,
  TILEFLAG_START       = 1 << 16,
  TILEFLAG_INTERACT    = 1 << 17,
  TILEFLAG_EXIT        = 1 << 18,
  TILEFLAG_SIZE_XS     = 1 << 19,
  TILEFLAG_SIZE_SM     = 1 << 20,
  TILEFLAG_SIZE_MED    = 1 << 21,
  TILEFLAG_SIZE_L      = 1 << 22,
  TILEFLAG_SIZE_XL     = 1 << 23,
  TILEFLAG_SIZE_MAX    = 1 << 24,
  MAPFLAG_DUNGEON      = 1 << 25,
  MAPFLAG_FOREST       = 1 << 26,
}TileFlags;

static const uint32_t EnvTileFlags[ENV_DONE] = {
  [ENV_BONES_BEAST]    = TILEFLAG_SIZE_MED | TILEFLAG_BONE | TILEFLAG_DEBRIS | TILEFLAG_DECOR | MAPFLAG_FOREST | MAPFLAG_DUNGEON,
  [ENV_BOULDER]        = TILEFLAG_SIZE_MED | TILEFLAG_SOLID | TILEFLAG_DEBRIS | TILEFLAG_STONE,
  [ENV_COBBLE]         = TILEFLAG_ROAD,
  [ENV_COBBLE_WORN]    = TILEFLAG_ROAD,
  [ENV_FLOWERS]        = TILEFLAG_SIZE_SM | TILEFLAG_DEBRIS | TILEFLAG_DECOR | TILEFLAG_NATURAL,
  [ENV_FLOWERS_THIN]   = TILEFLAG_SIZE_XS | TILEFLAG_DEBRIS | TILEFLAG_DECOR | TILEFLAG_NATURAL,
  [ENV_FOREST_FIR]     = TILEFLAG_SIZE_XL | TILEFLAG_SOLID | TILEFLAG_FOREST | TILEFLAG_NATURAL,
  [ENV_GRASS]          = TILEFLAG_SIZE_SM |MAPFLAG_FOREST | TILEFLAG_FLOOR | TILEFLAG_NATURAL,
  [ENV_GRASS_SPARSE]   = TILEFLAG_SIZE_XS | MAPFLAG_FOREST | TILEFLAG_EMPTY | TILEFLAG_FLOOR | TILEFLAG_NATURAL,
  [ENV_GRASS_WILD]     = TILEFLAG_SIZE_MED | MAPFLAG_FOREST | TILEFLAG_OBSTRUCT | TILEFLAG_NATURAL,
  [ENV_LEAVES]         = TILEFLAG_SIZE_XS | MAPFLAG_FOREST | TILEFLAG_DECOR | TILEFLAG_NATURAL,
  [ENV_TREE_MAPLE]     = TILEFLAG_SIZE_XL | TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_NATURAL,
  [ENV_MEADOW]         = TILEFLAG_SIZE_MAX | TILEFLAG_OBSTRUCT | TILEFLAG_NATURAL,
  [ENV_TREE_OLDGROWTH] = TILEFLAG_SIZE_MAX | MAPFLAG_FOREST |TILEFLAG_SOLID | TILEFLAG_TREE |  TILEFLAG_NATURAL,
  [ENV_TREE_PINE]      = TILEFLAG_SIZE_L | MAPFLAG_FOREST | TILEFLAG_SOLID | TILEFLAG_WALL | TILEFLAG_TREE | TILEFLAG_NATURAL,
  [ENV_ROAD]           = MAPFLAG_FOREST | TILEFLAG_ROAD,
  [ENV_ROAD_CROSS]     = MAPFLAG_FOREST | TILEFLAG_ROAD,
  [ENV_ROAD_FORK]      = MAPFLAG_FOREST | TILEFLAG_ROAD,
  [ENV_ROAD_TURN]      = MAPFLAG_FOREST | TILEFLAG_ROAD,
  [ENV_TREE_BIGLEAF]   = TILEFLAG_SIZE_XL | TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_NATURAL,
  [ENV_TREE_CEDAR]     = TILEFLAG_SIZE_L | TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_NATURAL,
  [ENV_TREE_DEAD]      = TILEFLAG_SIZE_SM | TILEFLAG_SOLID | TILEFLAG_TREE| TILEFLAG_DEBRIS,
  [ENV_TREE_DYING]     = TILEFLAG_SIZE_MED | TILEFLAG_SOLID | TILEFLAG_TREE,
  [ENV_TREE_FELLED]    = TILEFLAG_SIZE_MED | TILEFLAG_SOLID | TILEFLAG_DEBRIS | TILEFLAG_NATURAL | TILEFLAG_DECOR,  // updated
  [ENV_TREE_FIR]       = TILEFLAG_SIZE_L | TILEFLAG_SOLID | TILEFLAG_TREE | TILEFLAG_NATURAL,
  [ENV_FOREST]         = TILEFLAG_SIZE_MAX | MAPFLAG_FOREST | TILEFLAG_BORDER | TILEFLAG_SOLID| TILEFLAG_FOREST | TILEFLAG_NATURAL,
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
  [ENV_EXIT]  = MAPFLAG_DUNGEON | MAPFLAG_FOREST |  TILEFLAG_EXIT,
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
    ROOM_LAYOUT_SUB   = 0x0400,
    ROOM_LAYOUT_MAZE  = 0x0500,
    ROOM_LAYOUT_MAX  = 0x0600,
    ROOM_LAYOUT_MASK  = 0x0F00,
    
    // ----- Purpose (bits 4–7) -----
    ROOM_PURPOSE_NONE            = 0x0000,
    ROOM_PURPOSE_SIMPLE          = 0x0010,
    ROOM_PURPOSE_TRAPPED         = 0x0020,
    ROOM_PURPOSE_SECRET          = 0x0030,
    ROOM_PURPOSE_TREASURE        = 0x0040,
    ROOM_PURPOSE_CHALLENGE = 0x0050,
    ROOM_PURPOSE_LAIR      = 0x0060,
    ROOM_PURPOSE_CONNECT   = 0x0070,
    ROOM_PURPOSE_CAMP      = 0x0080,
    ROOM_PURPOSE_START     = 0x0090,
    ROOM_PURPOSE_STAIRS    = 0x00A0,
    ROOM_PURPOSE_SPAWN     = 0x00B0,
    ROOM_PURPOSE_EXIT      = 0x00C0,
    ROOM_PURPOSE_MAX       = 0x00D0,
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
    ROOM_ORIENT_MASK  = 0x00030000,

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
static int room_layout_weights[6] = {0,25,70,75,80,98};
static int room_purpose_weights[8] = {5,52,55,57,59,64,69,99};
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
  MOB_MOD_BEEF          = BIT64(11),
  MOB_MOD_MASK          = 0xFFULL << 8,

  MOB_LOC_DUNGEON       = BIT64(16),
  MOB_LOC_CAVE          = BIT64(17),
  MOB_LOC_FOREST        = BIT64(18),
  MOB_LOC_MASK          = 0xFFULL << 16,

  MOB_THEME_CRITTER     = BIT64(24),
  MOB_THEME_GAME        = BIT64(25),
  MOB_THEME_PRED        = BIT64(26),
  MOB_THEME_MONSTER     = BIT64(27),
  MOB_THEME_PRIMITIVE   = BIT64(28),
  MOB_THEME_MARTIAL     = BIT64(29),
  MOB_THEME_CIVIL       = BIT64(30),
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
  MOB_GROUPING_MASK   = 0xFFULL << 40,

  MOB_RESPAWN_NEVER   = BIT64(48),
  MOB_RESPAWN_LOW     = BIT64(49),
  MOB_RESPAWN_AVG     = BIT64(50),
  MOB_RESPAWN_HIGH    = BIT64(51),
  MOB_RESPAWN_MASK    = 0xFFULL << 48,
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
    Cell min, center, max;
} cell_bounds_t;

typedef struct {
  Biome     id;
  float     r_lairs, r_chall, r_secrets, r_traps, r_treasure;
  float     ratios[MT_DONE];
  int       desired[MT_DONE];
  int       current[MT_DONE];
  int       total, sum;
}biome_t;
extern biome_t BIOME[BIO_DONE];

static inline float BiomeDemand(biome_t* b, MobType mt){
    if (b->desired[mt] <= 0)
        return 0.0f;

    float remaining = (float)(b->desired[mt] - b->current[mt]);
    float ratio = remaining / (float)b->desired[mt];

    return CLAMP(ratio, 0.0f, 1.0f);
}

typedef struct room_s room_t;
typedef struct room_node_s room_node_t;

typedef struct{
  bool          entrance,rebound;
  struct room_s *sub;
  Cell          dir, pos;
  cell_bounds_t range;
}room_opening_t;

struct room_s{
  room_node_t*    ref;
  cell_bounds_t   bounds;
  int             num_mobs,depth,num_children;
  RoomFlags       flags;
  room_opening_t  *openings[16];
  Cell            center,dir;
  bool            is_root,is_reachable;
  ent_t           *mobs[MOB_ROOM_MAX];
};

typedef enum {
    MAP_NODE_SUCCESS,
    MAP_NODE_RUNNING,
    MAP_NODE_FAILURE
} MapNodeResult;

typedef enum {
  MAP_NODE_ROOT,
  MAP_NODE_SEQUENCE,
  MAP_NODE_LEAF,
} MapNodeType;

typedef enum {
  MN_GRID,
  MN_ANCHORS,
  MN_GENERATE_ROOMS,
  MN_GRID_GEN,
  MN_LAYOUT,
  MN_APPLY_SHAPES,
  MN_PLACE_ROOMS,
  MN_COMPUTE_BOUNDS,
  MN_ALLOCATE_TILES,
  MN_CARVE_TILES,
  MN_GRAPH_ROOTS,
  MN_BUILD,
  MN_DETAIL,
  MN_ENCASE_FLOORS,
  MN_PLACE_POI,
  MN_SET_PLAYER,
  MN_PLACE_SPAWNS,
  MN_ALIGN_ROOMS,
  MN_CLEANUP,
  MN_APPLY_NODES,
  MN_PATHS,
  MN_CHECK_MAP,
  MN_ISSUES,
  MN_GRAPH,
  MN_FIX,
  MN_ECO,
  MN_ENH,
  MAP_NODE_DONE
} MapNodeID;

typedef struct room_gen_s room_gen_t;
struct room_gen_s{
  int                cap,num_children;
  RoomFlags          flags;
  struct room_gen_s* children[4];
};

typedef struct{
  int           index;
  room_node_t*  owner;
  bool          used,enter, exit;
  int           depth;
  cell_bounds_t range;
  Cell          dir,pos;
  room_node_t   *room;
}node_connector_t;

struct room_node_s{
  node_connector_t  *anchor;
  Cell              center, size;
  RoomFlags         flags;
  cell_bounds_t     bounds, section;
  int               sector,num_children, max_children, total, cap, depth, num_orphans;
  int               entrance_index;
  bool              is_root,applied, is_reachable;
  Color             col;
  node_connector_t  **children;
  room_node_t       *orphans[MAX_NODE_DEPTH];
};

typedef struct{
  MapID           id;
  Biome           biome;
  float           diff,max_diff;
  TileFlags       map_flag;
  int             density,min_rooms,min_mobs;
  TileFlags       opening_flag;
  spawn_rules_t   mobs;
  int             margin_error;
  room_gen_t      root;
  MapNodeID       node_rules;
}map_gen_t;

typedef struct{
  int       grid_iid;
  room_t*   from, *to;
  room_opening_t  *opening,*in,*out;
  RoomStatus    status;
}room_connection_t;

typedef struct{
  TileFlags        tile;
  room_node_t      *room;
  node_connector_t *conn;
}map_exit_t;

typedef struct {
  GenStatus   status;
  map_gen_t   *map_rules;
  biome_t     *eco;
  int         width, height, num_rooms;
  room_node_t *anchors[MAX_ANCHOR_NODES];
  int         num_bounds;
  Rectangle   room_bounds[MAX_ROOMS];
  room_t      *rooms[MAX_ROOMS];
  Cell        player_start;
  map_exit_t  *exits[4];
  int          num_exits, num_conn;
  room_connection_t* connections[MAX_ROOMS*3];
  int         alloc_w, alloc_h;
  // random seed etc if you want
  cell_bounds_t level;
  unsigned int seed;
  int         decor_density;
  TileFlags   **tiles;
  choice_pool_t* mob_pool;
} map_context_t;

Cell PLAYER_SPAWN(void);

typedef enum {
  OPT_CONN,
  OPT_ROOM,
  OPT_CORRIDOR,
  OPT_FEATURE,
} OptionType;

typedef struct node_option_s{
  OptionType        type;
  int               score;
  RoomFlags         flags;
  void*             context;
  node_connector_t* connect;
} node_option_t;

typedef struct {
  node_option_t  items[MAX_OPTIONS];
  int count; 
}node_option_pool_t;

bool HasFloorNeighbor(map_context_t* ctx, int x, int y);
struct map_node_s;
typedef struct map_node_s map_node_t;

typedef MapNodeResult (*MapNodeFn)(map_context_t *ctx, map_node_t *node);

void RefreshOptionPool(map_context_t *context, map_node_t* node);

MapNodeResult MapGenScan(map_context_t *context, map_node_t *node);

MapNodeResult MapGraphNodes(map_context_t *ctx, map_node_t *node);
MapNodeResult MapApplyFixes(map_context_t *ctx, map_node_t *node);
MapNodeResult MapFixIssues(map_context_t *ctx, map_node_t *node);
MapNodeResult MapCheckPaths(map_context_t *ctx, map_node_t *node);
MapNodeResult MapNodesToGrid(map_context_t *ctx, map_node_t *node);
MapNodeResult MapAlignNodes(map_context_t *ctx, map_node_t *node);
MapNodeResult MapPlaceSpawns(map_context_t *ctx, map_node_t *node);
MapNodeResult MapBuildBiome(map_context_t *ctx, map_node_t *node);
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
MapNodeResult MapEnhance(map_context_t *ctx, map_node_t *node);

void MapAssignTreasures(map_context_t *ctx, room_t** rooms, int count);
void MapAssignTraps(map_context_t *ctx, room_t** rooms, int count);
void MapAssignSecrets(map_context_t *ctx, room_t** rooms, int count);
void MapAssignChallenges(map_context_t *ctx, room_t** rooms, int count);
void MapAssignLairs(map_context_t *ctx, room_t** rooms, int count);

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

static inline map_node_t* LeafMapPlaceSpawns(MapNodeID id)  { return MapCreateLeafNode(MapPlaceSpawns,id); }
static inline map_node_t* LeafMapEnhance(MapNodeID id)  { return MapCreateLeafNode(MapEnhance,id); }
static inline map_node_t* LeafMapBuildBiome(MapNodeID id)  { return MapCreateLeafNode(MapBuildBiome,id); }
static inline map_node_t* LeafMapFillWalls(MapNodeID id)  { return MapCreateLeafNode(MapFillWalls,id); }
static inline map_node_t* LeafMapPlayerSpawn(MapNodeID id)  { return MapCreateLeafNode(MapPlayerSpawn,id); }
static inline map_node_t* LeafMapGraphNodes(MapNodeID id)  { return MapCreateLeafNode(MapGraphNodes,id); }
static inline map_node_t* LeafMapApplyFixes(MapNodeID id)  { return MapCreateLeafNode(MapApplyFixes,id); }
static inline map_node_t* LeafMapFixIssues(MapNodeID id)  { return MapCreateLeafNode(MapFixIssues,id); }
static inline map_node_t* LeafMapCheckPaths(MapNodeID id)  { return MapCreateLeafNode(MapCheckPaths,id); }
static inline map_node_t* LeafMapNodesToGrid(MapNodeID id)  { return MapCreateLeafNode(MapNodesToGrid,id); }
static inline map_node_t* LeafMapAlignNodes(MapNodeID id)  { return MapCreateLeafNode(MapAlignNodes,id); }
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
void MapGenRender(void);

bool TooClose(Cell a, Cell b, int min_dist);
static int RoomsOverlap(room_t *a, room_t *b, int spacing) {
    int dist = CellDistGrid(a->center, b->center);

    RoomFlags asize = a->flags & ROOM_SIZE_MASK;
    RoomFlags bsize = b->flags & ROOM_SIZE_MASK;
    int ar = (asize >> 12) + spacing;
    int br = (bsize >> 12) + spacing;

    return imax(ar-dist,br-dist);
}

bool MapAddNodeToConnector(node_connector_t* conn, room_node_t* node);
void RoomAdjustPosition(room_t *r, Cell disp, bool all);
bool room_is_enclosed(map_context_t* ctx, cell_bounds_t room);
bool CheckPath(map_context_t* m, Cell start, Cell end, Cell *block);
bool MapCheckOverlap(Rectangle bounds, Vector2 *overlap);
void RefreshNodeOptions(void);
bool MapRoomGraphNodes(void);
MapNodeResult MapGenNodeScan(map_context_t *ctx, map_node_t *node);
bool ApplyNode(node_option_t *opt);
node_option_t *PickBestNodeOption(void);
void MapAddNodeOption(node_option_t opt);
bool NodeFindOptions(room_node_t* node, bool ignore_depth);

int ConnectionsByIID(room_connection_t* conn, room_connection_t** pairs);
void ConnectionSetIID(room_connection_t* conn);
void ConnectionSetStatus(Cell pos, RoomStatus status);
int ConnectionGetNeighbors(room_connection_t* conn, room_connection_t** pairs, int range, int cap);
int GetNeighborFlags(map_context_t* ctx, Cell c, RoomFlags f, Cell *filter);

typedef struct{
  uint64_t  resources;
  uint64_t  scents[SATUR_MAX];
}site_properties_t;

typedef struct{
  game_object_uid_i   gouid;
  int                 index;
  Cell                coords;
  TileStatus          status;
  Visibility          vis;
  TileFlags           flags;
  env_t*              tile;
  ent_t*              occupant;
  Color               fow;
  site_properties_t*  props;
  bool                in_ctx, updates, explored;
}map_cell_t;

typedef struct{
  map_grid_t*     map;
  uint64_t        id;
  Cell            center;
  cell_bounds_t   bounds;
  RoomFlags       purpose, flags;
  int             respawn_factor;
  int             num_mobs, total_cr, avg_cr, best_cr;
  choice_pool_t*  placements;
  ent_t           *mobs[MOB_ROOM_MAX];
}map_room_t;

map_room_t* InitMapRoom(map_context_t* ctx, room_t* r);

struct map_grid_s{
  MapID        id;
  int          num_rooms;
  map_room_t   *rooms[MAX_ROOMS];
  map_cell_t   **tiles;
  int          num_changes, reachable;
  map_cell_t   *changes[128];
  int          x,y,width,height;
  int          step_size;
  int          num_mobs;
  Color        floor;
  bool         updates;
};

bool InitMap(void);
void WorldMapLoaded(map_grid_t* m);
map_grid_t* InitMapGrid(void);
void MapRoomSpawn(map_grid_t* m, EntityType data, int room);
void MapRender(map_grid_t* m);
void MapSync(map_grid_t* m);
void MapTurnStep(map_grid_t* m);
TileStatus MapChangeOccupant(map_grid_t* m, ent_t* e, Cell old, Cell c);
TileStatus MapSetOccupant(map_grid_t* m, ent_t* e, Cell c);
ent_t* MapGetOccupant(map_grid_t* m, Cell c, TileStatus* status);
map_cell_t* MapGetTile(map_grid_t* map,Cell tile);
TileStatus MapTileAvailable(map_grid_t* m, Cell c);
TileStatus MapRemoveOccupant(map_grid_t* m, Cell c);
TileStatus MapSetTile(map_grid_t* m, env_t* e, Cell c);
void MapBuilderSetFlags(TileFlags flags, int x, int y,bool safe);
env_t* MapSpawn(TileFlags flags, int x, int y);
void MapSpawnMob(map_grid_t* m, int x, int y);
void RoomSpawnMob(map_grid_t* m, room_t* r);
Cell MapApplyContext(map_grid_t* m);
void MapVisEvent(EventType event, void* data, void* user);
static int MapGetNeighborsByStatus(map_grid_t* m, Cell pos, map_cell_t* nei[8], TileStatus status){

  int count = 0;
  for (int x = pos.x -1; x < pos.x+1; x++)
    for (int y = pos.y -1; y < pos.y+1; y++){
      map_cell_t* n = &m->tiles[x][y];

      if(n->status != status)
        continue;

      nei[count++] = n;
    }

  return count;
}
static uint32_t g_navRevision = 1;

static void NavMarkDirty(void) {
    g_navRevision++;
}

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

typedef uint64_t path_cache_uid_i;
typedef struct {
  path_cache_uid_i  guid;
  path_cache_uid_i  puid;
  bool              valid;
  int               sx, sy, tx, ty, cost, length;
  Cell              next;
  Cell              path[MAX_PATH_LEN];
  uint32_t          navRevision;
} path_cache_entry_t;

typedef struct{
  int                 count, cap;
  path_cache_entry_t  *entries;
} path_pool_t;

path_pool_t* InitPathPool(int cap);

typedef struct {
  bool  found;
  int   cost;
  int   length;
  Cell  path[MAX_PATH_LEN];
} path_result_t;

static inline bool InBounds(map_grid_t *m, int x, int y) {
    return (x >= 0 && x < m->width && y >= 0 && y < m->height);
}

static inline int Heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

static path_node_t nodes[GRID_WIDTH][GRID_HEIGHT];
static path_cache_entry_t pathCacheTmp[MAX_CACHED_PATHS];
path_cache_entry_t* PathCacheFind(int sx, int sy, int tx, int ty);

path_cache_entry_t* PathCacheStore(path_result_t* res, Cell sc, Cell tc, game_object_uid_i start, game_object_uid_i end);
path_cache_entry_t* PathCacheStoreTemp(path_result_t* res, int sx, int sy,int tx, int ty);
path_result_t* FindPath(map_grid_t *m, int sx, int sy, int tx, int ty, Cell *outNextStep, int depth);
static path_result_t* FindPathCell(map_grid_t *m, Cell sc, Cell tc, Cell *out, int depth){
  return FindPath(m, sc.x, sc.y, tc.x, tc.y, out, depth);
}
path_cache_entry_t* PathCacheFindRoute(ent_t* e, local_ctx_t* dest);
path_cache_entry_t* StartRoute(ent_t* e, local_ctx_t* dest, int depth, bool* result);
int RouteScore(ent_t* e, path_cache_entry_t* route);
Cell RouteGetNext(ent_t* e, path_cache_entry_t* route);
bool HasLOS(map_grid_t* m, Cell c0, Cell c1);
void CastLight(map_grid_t *m, Cell pos, int row, float start, float end, int radius,int xx, int xy, int yx, int yy);
int ScorePath(map_grid_t *m, int sx, int sy, int tx, int ty, int depth);
static int ScorePathCell(map_grid_t *m, Cell sc, Cell tc, int depth){
  return ScorePath(m, sc.x, sc.y, tc.x, tc.y, depth);
}

bool room_has_access(map_context_t* ctx, cell_bounds_t room,Cell *access);
bool IsDiagBlocked(map_grid_t* m, map_cell_t* cc, map_cell_t* nc, TileBlock fn);

static int PathCost(int tx, int ty) {
    return nodes[tx][ty].gCost;
}
static bool TileFlagHas(TileFlags f, TileFlags has){
  return ((f & has) > 0);
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

bool TileBlocksMovement(map_cell_t *c);

bool TileBlocksSight(map_cell_t *c);
    
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

#endif
