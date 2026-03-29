//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
// rTexpacker v5.0 Atlas Descriptor Code exporter v5.0                          //
//                                                                              //
// more info and bugs-report:  github.com/raylibtech/rtools                     //
// feedback and support:       ray[at]raylibtech.com                            //
//                                                                              //
// Copyright (c) 2019-2025 raylib technologies (@raylibtech)                    //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////

#define ATLAS_ASSET_ENV_IMAGE_PATH      "asset_env.png"
#define ATLAS_ASSET_ENV_SPRITE_COUNT    40

// Atlas sprite properties
typedef struct rtpAtlasSprite {
    const char *nameId;
    const char *tag;
    int originX, originY;
    int positionX, positionY;
    int sourceWidth, sourceHeight;
    int padding;
    bool trimmed;
    int trimRecX, trimRecY, trimRecWidth, trimRecHeight;
    int colliderType;
    int colliderPosX, colliderPosY, colliderSizeX, colliderSizeY;
} rtpAtlasSprite;

// Atlas sprites array
static rtpAtlasSprite rtpDescAssetEnv[40] = {
    { "BONES_BEAST", "ENV_BONES_BEAST", 8, 8, 0, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "BOULDER", "ENV_BOULDER", 8, 8, 16, 0, 16, 16, 0, false, 1, 3, 14, 11, 0, 0, 0, 0, 0 },
    { "COBBLE", "ENV_COBBLE", 8, 8, 32, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "COBBLE_WORN", "ENV_COBBLE_WORN", 8, 8, 48, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "FLOWERS", "ENV_FLOWERS", 8, 8, 64, 0, 16, 16, 0, false, 2, 2, 12, 12, 0, 0, 0, 0, 0 },
    { "FLOWERS_THIN", "ENV_FLOWERS_THIN", 8, 8, 80, 0, 16, 16, 0, false, 3, 2, 10, 12, 0, 0, 0, 0, 0 },
    { "FOREST_FIR", "ENV_FOREST_FIR", 8, 8, 96, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "GRASS", "ENV_GRASS", 8, 8, 112, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "GRASS_SPARSE", "ENV_GRASS_SPARSE", 8, 8, 0, 16, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "GRASS_WILD", "ENV_GRASS_WILD", 8, 8, 16, 16, 16, 16, 0, false, 2, 3, 12, 12, 0, 0, 0, 0, 0 },
    { "LEAVES", "ENV_LEAVES", 8, 8, 32, 16, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "MAPLE", "ENV_TREE_MAPLE", 8, 8, 48, 16, 16, 16, 0, false, 2, 1, 12, 14, 0, 0, 0, 0, 0 },
    { "MEADOW", "ENV_MEADOW", 8, 8, 64, 16, 16, 16, 0, false, 1, 2, 14, 12, 0, 0, 0, 0, 0 },
    { "OLDGROWTH", "ENV_TREE_OLDGROWTH", 8, 8, 80, 16, 16, 16, 0, false, 3, 1, 10, 14, 0, 0, 0, 0, 0 },
    { "PINE", "ENV_TREE_PINE", 8, 8, 96, 16, 16, 16, 0, false, 3, 1, 10, 14, 0, 0, 0, 0, 0 },
    { "ROAD", "ENV_ROAD", 8, 8, 112, 16, 16, 16, 0, false, 2, 0, 13, 16, 0, 0, 0, 0, 0 },
    { "ROAD_CROSS", "ENV_ROAD_CROSS", 8, 8, 0, 32, 16, 16, 0, false, 0, 0, 16, 16, 0, 0, 0, 0, 0 },
    { "ROAD_FORK", "ENV_ROAD_FORK", 8, 8, 16, 32, 16, 16, 0, false, 2, 0, 14, 16, 0, 0, 0, 0, 0 },
    { "ROAD_TURN", "ENV_ROAD_TURN", 8, 8, 32, 32, 16, 16, 0, false, 2, 2, 14, 14, 0, 0, 0, 0, 0 },
    { "TREE_BIGLEAF", "ENV_TREE_BIGLEAF", 8, 8, 48, 32, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "TREE_CEDAR", "ENV_TREE_CEDAR", 8, 8, 64, 32, 16, 16, 0, false, 4, 1, 8, 14, 0, 0, 0, 0, 0 },
    { "TREE_DEAD", "ENV_TREE_DEAD", 8, 8, 80, 32, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "TREE_DYING", "ENV_TREE_DYING", 8, 8, 96, 32, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "TREE_FELLED", "ENV_TREE_FELLED", 8, 8, 112, 32, 16, 16, 0, false, 2, 1, 12, 14, 0, 0, 0, 0, 0 },
    { "TREE_FIR", "ENV_TREE_FIR", 8, 8, 0, 48, 16, 16, 0, false, 3, 1, 10, 14, 0, 0, 0, 0, 0 },
    { "TREES_DENSE", "ENV_FOREST", 8, 8, 16, 48, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "WEB", "ENV_WEB", 8, 8, 32, 48, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "DIRT", "", 8, 8, 48, 48, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "DIRT_PATCH", "", 8, 8, 64, 48, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "camp", "ENV_CAMP", 8, 8, 80, 48, 16, 16, 0, false, 2, 1, 12, 14, 0, 0, 0, 0, 0 },
    { "FLOOR_DUNGEON", "ENV_FLOOR_DUNGEON", 8, 8, 96, 48, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "WALL_DUNGEON", "ENV_STONE_WALL", 8, 8, 112, 48, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "WALL_DUNGEON_ALT", "ENV_WALL_DUNGEON", 8, 8, 0, 64, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "WALL_RUIN", "ENV_WALL_RUIN", 8, 8, 16, 64, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "DOOR_DUNGEON", "ENV_DOOR_DUNGEON", 8, 8, 32, 64, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "DOOR_HEAVY", "ENV_DOOR_DUNGEON", 8, 8, 48, 64, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "DOOR_JAILT", "ENV_DOOR_VAULT", 8, 8, 64, 64, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "BORDER_DUNGEON", "", 0, 0, 80, 64, 16, 16, 0, false, 0, 0, 16, 16, 0, 0, 0, 0, 0 },
    { "ENV_CHAIR", "", 0, 0, 96, 64, 16, 16, 0, false, 3, 5, 10, 10, 0, 0, 0, 0, 0 },
    { "ENV_EXIT", "", 0, 0, 112, 64, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
};
