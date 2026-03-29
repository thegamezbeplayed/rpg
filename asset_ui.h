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

#define ATLAS_ASSET_UI_IMAGE_PATH      "asset_ui.png"
#define ATLAS_ASSET_UI_SPRITE_COUNT    4

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
static rtpAtlasSprite rtpDescAssetUi[4] = {
    { "UI_SELECTOR_CHOSEN", "", 8, 8, 0, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "UI_SELECTOR_EMPTY", "", 8, 8, 16, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "UI_SELECTOR_VALID", "", 8, 8, 32, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
    { "UI_GRID_CELL", "", 8, 8, 48, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
};
