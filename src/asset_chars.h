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
#ifndef __ASSET_CHAR__
#define __ASSET_CHAR__

#define CHARS_IMAGE_PATH      "asset_chars.png"
#define ATLAS_ASSET_CHARS_SPRITE_COUNT    5

// Atlas sprite properties
// Atlas sprites array
static sub_texture_t CHAR_SPRITES[CHAR_ALL] = {
    { LETTER_O, 8, 8, 0, 0, 16, 16,   BLUE, .name = "O"},
    { LETTER_N, 8, 8, 16, 0, 16, 16,  BLUE, .name = "N"},
    { LETTER_P, 8, 8, 32, 0, 16, 16,  BLUE, .name = "P"},
    { LETTER_G, 8, 8, 48, 0, 16, 16,  BLUE, .name = "G"},
    { LETTER_A, 8, 8, 64, 0, 16, 16,  BLUE, .name = "A"},
};

#endif
