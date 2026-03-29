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

#define ATLAS_ASSET_ENT_IMAGE_PATH      "asset_ent.png"
#define ATLAS_ASSET_ENT_SPRITE_COUNT    21

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
static rtpAtlasSprite rtpDescAssetEnt[21] = {
    { "BERSERKER", "ENT_BERSERKER", 8, 8, 0, 0, 16, 16, 0, false, 2, 1, 13, 14, 0, 0, 0, 0, 0 },
    { "ENT_PERSON", "ENT_PERSON", 8, 8, 16, 0, 16, 16, 0, false, 2, 1, 12, 14, 0, 0, 0, 0, 0 },
    { "GOBLIN", "ENT_GOBLIN", 8, 8, 32, 0, 16, 16, 0, false, 2, 1, 12, 14, 0, 0, 0, 0, 0 },
    { "HOBGOB", "ENT_HOBGOBLIN", 8, 8, 48, 0, 16, 16, 0, false, 2, 1, 12, 14, 0, 0, 0, 0, 0 },
    { "OGRE", "ENT_OGRE", 8, 8, 64, 0, 16, 16, 0, false, 1, 0, 14, 16, 0, 0, 0, 0, 0 },
    { "ORC", "ENT_ORC", 8, 8, 80, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "ORCFIGHTER", "ENT_ORC_FIGHTER", 8, 8, 96, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "OROG", "ENT_OROG", 8, 8, 112, 0, 16, 16, 0, false, 1, 0, 14, 15, 0, 0, 0, 0, 0 },
    { "SCORPION", "ENT_SCORPION", 8, 8, 0, 16, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "SPIDER", "ENT_SPIDER", 8, 8, 16, 16, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "TROLL", "ENT_TROLL", 8, 8, 32, 16, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "TROLL_CAVE", "ENT_TROLL_CAVE", 8, 8, 48, 16, 16, 16, 0, false, 1, 0, 14, 16, 0, 0, 0, 0, 0 },
    { "BEAR", "ENT_BEAR", 8, 8, 64, 16, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "wolf", "ENT_WOLF", 8, 8, 80, 16, 16, 16, 0, false, 0, 0, 16, 16, 0, 0, 0, 0, 0 },
    { "ratf", "ENT_RAT", 8, 8, 96, 16, 16, 16, 0, false, 1, 2, 14, 12, 0, 0, 0, 0, 0 },
    { "ENT_SKELETON", "", 0, 0, 112, 16, 16, 16, 0, false, 2, 1, 12, 14, 0, 0, 0, 0, 0 },
    { "ENT_DEER", "", 0, 0, 0, 32, 16, 16, 0, false, 0, 0, 16, 14, 0, 0, 0, 0, 0 },
    { "ENT_KOBOLD", "", 0, 0, 16, 32, 16, 16, 0, false, 2, 3, 13, 12, 0, 0, 0, 0, 0 },
    { "ENT_BUGBEAR", "", 0, 0, 32, 32, 16, 16, 0, false, 1, 1, 14, 15, 0, 0, 0, 0, 0 },
    { "ENT_KNIGHT", "", 0, 0, 48, 32, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "ENT_BOAR", "", 0, 0, 64, 32, 16, 16, 0, false, 0, 4, 16, 10, 0, 0, 0, 0, 0 },
};
