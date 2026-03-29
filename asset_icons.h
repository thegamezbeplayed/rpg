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

#define ATLAS_ASSET_ICONS_IMAGE_PATH      "asset_icons.png"
#define ATLAS_ASSET_ICONS_SPRITE_COUNT    12

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
static rtpAtlasSprite rtpDescAssetIcons[12] = {
    { "HELM_COMt", "", 0, 0, 0, 0, 32, 32, 0, false, 4, 2, 24, 29, 0, 0, 0, 0, 0 },
    { "POtt", "", 0, 0, 32, 0, 32, 32, 0, false, 4, 2, 24, 29, 0, 0, 0, 0, 0 },
    { "MACE_COM", "", 0, 0, 64, 0, 32, 32, 0, false, 4, 5, 23, 23, 0, 0, 0, 0, 0 },
    { "BELT", "", 0, 0, 96, 0, 32, 32, 0, false, 2, 8, 28, 15, 0, 0, 0, 0, 0 },
    { "SCROLL", "", 0, 0, 128, 0, 32, 32, 0, false, 2, 2, 28, 28, 0, 0, 0, 0, 0 },
    { "TOME", "", 0, 0, 160, 0, 32, 32, 0, false, 2, 1, 28, 29, 0, 0, 0, 0, 0 },
    { "SWORD", "", 0, 0, 192, 0, 32, 32, 0, false, 6, 6, 20, 20, 0, 0, 0, 0, 0 },
    { "HATCHET", "", 0, 0, 224, 0, 32, 32, 0, false, 5, 8, 25, 19, 0, 0, 0, 0, 0 },
    { "CURAISS", "", 0, 0, 0, 32, 32, 32, 0, false, 4, 4, 24, 24, 0, 0, 0, 0, 0 },
    { "SHIRT", "", 0, 0, 32, 32, 32, 32, 0, false, 8, 8, 16, 17, 0, 0, 0, 0, 0 },
    { "LEATHER_ARMOR", "", 0, 0, 64, 32, 32, 32, 0, false, 4, 6, 25, 23, 0, 0, 0, 0, 0 },
    { "DAGGER", "", 0, 0, 96, 32, 32, 32, 0, false, 6, 2, 24, 24, 0, 0, 0, 0, 0 },
};
