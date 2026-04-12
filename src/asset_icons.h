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

#define ICONS_IMAGE_PATH      "asset_icons.png"
#define ATLAS_ASSET_ICONS_SPRITE_COUNT    5

// Atlas sprites array
static sub_texture_t ICON_SPRITES[ICON_ALL] = {
  { ICON_HELM_COM,  16, 16, 0, 0, 32, 32},
  { ICON_POT_FULL,  16, 16, 32, 0, 32, 32},
  { ICON_MACE_COM,  16, 16, 64, 0, 32, 32},
  { ICON_BELT,      16, 16, 96, 0, 32, 32},
  { ICON_SCROLL,    16, 16, 128, 0, 32, 32, BEIGE},
  { ICON_TOME,      16, 16, 160, 0, 32, 32, RAYWHITE},
  { ICON_SWORD,     16, 16, 192, 0, 32, 32},
  { ICON_HATCHET,   16, 16, 224, 0, 32, 32},
  { ICON_CURAISS,   16, 16, 0, 32,  32, 32},
  { ICON_SHIRT,     16, 16, 32, 32, 32, 32},
  { ICON_TUNIC_COM, 16, 16, 64, 32, 32, 32},
  { ICON_DAGGER,    16, 16, 96, 32, 32, 32},
  { ICON_KNIFE,     16, 16, 128, 32, 32, 32, RAYWHITE},
  { ICON_ROCKS,     16, 16, 160, 32, 32, 32, LIGHTGRAY},
  { ICON_SHARD,     16, 16, 192,  32, 32, 32, DARKGRAY},
  { ICON_PICK,      16, 16, 224,  32, 32, 32, LIGHTGRAY},
  { ICON_WOOD,      16, 16, 0,  64, 32, 32, BROWN},
  { ICON_INGOT,     16, 16, 32, 64, 32, 32, DARKGRAY},
  { ICON_BUCKET,    16, 16, 32, 64, 32, 32, LIGHTGRAY}
};
