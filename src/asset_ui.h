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
  {ELEMENT_EMPTY},
  { UI_SELECTOR_CHOSEN,8, 8, 0, 0, 16, 16, BLUE, 0, 0, 0, 0 },
  { UI_SELECTOR_EMPTY, 8, 8, 16, 0, 16, 16, YELLOW, 0, 0, 0, 0 },
  { UI_SELECTOR_VALID, 8, 8, 32, 0, 16, 16, WHITE, 0, 0, 0, 0 },
  { UI_GRID_CELL,      8, 8, 48, 0, 16, 16, LIGHTGRAY },

};
