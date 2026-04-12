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

#define UI_IMAGE_PATH      "asset_ui.png"

// Atlas sprite properties
// Atlas sprites array
static sub_texture_t UI_SPRITES[ELEMENT_COUNT] = {
  [UI_SELECTOR_CHOSEN] = { UI_SELECTOR_CHOSEN,8, 8, 32, 0, 16, 16, BLUE},
  [UI_SELECTOR_EMPTY]  = { UI_SELECTOR_EMPTY, 8, 8, 48, 0, 16, 16, YELLOW},
  [UI_SELECTOR_VALID]  = { UI_SELECTOR_VALID, 8, 8, 64, 0, 16, 16, WHITE},
  [UI_GRID_CELL]       = { UI_GRID_CELL, 8, 8, 16, 0, 16, 16, LIGHTGRAY },
  [UI_SELECTOR_AREA]   = {UI_SELECTOR_AREA, 8, 8, 0, 0, 16, 16, BLUE},
};
