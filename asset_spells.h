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

#define ATLAS_ASSET_SPELLS_IMAGE_PATH      "asset_spells.png"
#define ATLAS_ASSET_SPELLS_SPRITE_COUNT    16

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
static rtpAtlasSprite rtpDescAssetSpells[16] = {
    { "BLUDGEON", "", 0, 0, 0, 0, 32, 32, 0, false, 4, 2, 24, 28, 0, 0, 0, 0, 0 },
    { "BONE", "", 0, 0, 32, 0, 32, 32, 0, false, 4, 4, 24, 24, 0, 0, 0, 0, 0 },
    { "BUBBLE", "", 0, 0, 64, 0, 32, 32, 0, false, 2, 2, 28, 28, 0, 0, 0, 0, 0 },
    { "BUBBLES", "", 0, 0, 96, 0, 32, 32, 0, false, 2, 2, 28, 28, 0, 0, 0, 0, 0 },
    { "CRYSTAL", "", 0, 0, 128, 0, 32, 32, 0, false, 2, 2, 28, 28, 0, 0, 0, 0, 0 },
    { "EYE", "", 0, 0, 160, 0, 32, 32, 0, false, 2, 2, 28, 28, 0, 0, 0, 0, 0 },
    { "FLARE", "", 0, 0, 192, 0, 32, 32, 0, false, 8, 4, 16, 24, 0, 0, 0, 0, 0 },
    { "FLOWER", "", 0, 0, 224, 0, 32, 32, 0, false, 4, 4, 24, 24, 0, 0, 0, 0, 0 },
    { "GLIMMER", "", 0, 0, 0, 32, 32, 32, 0, false, 2, 4, 28, 24, 0, 0, 0, 0, 0 },
    { "MISSILE", "", 0, 0, 32, 32, 32, 32, 0, false, 4, 4, 24, 24, 0, 0, 0, 0, 0 },
    { "PIERCE", "", 0, 0, 64, 32, 32, 32, 0, false, 4, 4, 24, 24, 0, 0, 0, 0, 0 },
    { "PYRE", "", 0, 0, 96, 32, 32, 32, 0, false, 4, 4, 24, 24, 0, 0, 0, 0, 0 },
    { "ROCK", "", 0, 0, 128, 32, 32, 32, 0, false, 4, 4, 24, 24, 0, 0, 0, 0, 0 },
    { "SHIELD", "", 0, 0, 160, 32, 32, 32, 0, false, 4, 2, 24, 28, 0, 0, 0, 0, 0 },
    { "SLASH", "", 0, 0, 192, 32, 32, 32, 0, false, 4, 4, 24, 24, 0, 0, 0, 0, 0 },
    { "WISP", "", 0, 0, 224, 32, 32, 32, 0, false, 2, 4, 28, 24, 0, 0, 0, 0, 0 },
};
