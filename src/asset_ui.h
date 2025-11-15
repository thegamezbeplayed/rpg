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
  {ELEMENT_PANEL_GRAY, 64, 64, 768, 0, 128, 128, 0, false, 0, 0, 128, 128, 0, 10, 10, 108, 108 },
  {ELEMENT_PANEL_GRAY_WIDE, 192, 64, 0, 0, 384, 128, 0, false, 0, 0, 384, 128, 0, 10, 10, 364, 108 },
  {ELEMENT_BUTTON_GRAY_ACTIVE, 192, 64, 0, 128, 384, 128, 0, false, 0, 0, 384, 128, 2, 778, -118, 107, 108 },
  { ELEMENT_BUTTON_GRAY, 192, 64, 384, 0, 384, 128, 0, false, 0, 0, 384, 128, 0, 10, 10, 364, 108 },
  { ELEMENT_BOX_GRAY, 64, 64, 896, 0, 128, 128, 0, false, 0, 0, 128, 128, 2, -883, 139, 360, 104 },
  { ELEMENT_ERROR_WIDE, 192, 64, 384, 128, 384, 128, 0, false, 0, 0, 384, 128, 0, 16, 16, 352, 96 },

};
