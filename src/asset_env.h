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

#define ENV_IMAGE_PATH      "asset_env.png"
#define ATLAS_ASSET_ENV_SPRITE_COUNT    27

// Atlas sprites array
static sub_texture_t ENVIRO_SPRITES[ENV_DONE] = {
  { ENV_BONES_BEAST, 8, 8, 0, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
  { ENV_BOULDER, 8, 8, 16, 0, 16, 16, 0, false, 1, 3, 14, 11, 0, 0, 0, 0, 0 },
  { ENV_COBBLE, 8, 8, 32, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
  { ENV_COBBLE_WORN, 8, 8, 48, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
  { ENV_FLOWERS, 8, 8, 64, 0, 16, 16, 0, false, 2, 2, 12, 12, 0, 0, 0, 0, 0 },
  { ENV_FLOWERS_THIN, 8, 8, 80, 0, 16, 16, 0, false, 3, 2, 10, 12, 0, 0, 0, 0, 0 },
  { ENV_FOREST_FIR, 8, 8, 96, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
  { ENV_GRASS, 8, 8, 112, 0, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
  { ENV_GRASS_SPARSE, 8, 8, 0, 16, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
  { ENV_GRASS_WILD, 8, 8, 16, 16, 16, 16, 0, false, 2, 3, 12, 12, 0, 0, 0, 0, 0 },
  { ENV_LEAVES, 8, 8, 32, 16, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
  { ENV_TREE_MAPLE, 8, 8, 48, 16, 16, 16, 0, false, 2, 1, 12, 14, 0, 0, 0, 0, 0 },
  { ENV_MEADOW, 8, 8, 64, 16, 16, 16, 0, false, 1, 2, 14, 12, 0, 0, 0, 0, 0 },
  { ENV_TREE_OLDGROWTH, 8, 8, 80, 16, 16, 16, 0, false, 3, 1, 10, 14, 0, 0, 0, 0, 0 },
  { ENV_TREE_PINE, 8, 8, 96, 16, 16, 16, 0, false, 3, 1, 10, 14, 0, 0, 0, 0, 0 },
  { ENV_ROAD, 8, 8, 112, 16, 16, 16, 0, false, 2, 0, 13, 16, 0, 0, 0, 0, 0 },
  { ENV_ROAD_CROSS, 8, 8, 0, 32, 16, 16, 0, false, 0, 0, 16, 16, 0, 0, 0, 0, 0 },
  { ENV_ROAD_FORK, 8, 8, 16, 32, 16, 16, 0, false, 2, 0, 14, 16, 0, 0, 0, 0, 0 },
  { ENV_ROAD_TURN, 8, 8, 32, 32, 16, 16, 0, false, 2, 2, 14, 14, 0, 0, 0, 0, 0 },
  { ENV_TREE_BIGLEAF, 8, 8, 48, 32, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
  { ENV_TREE_CEDAR, 8, 8, 64, 32, 16, 16, 0, false, 4, 1, 8, 14, 0, 0, 0, 0, 0 },
  { ENV_TREE_DEAD, 8, 8, 80, 32, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
  { ENV_TREE_DYING, 8, 8, 96, 32, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
  { ENV_TREE_FELLED, 8, 8, 112, 32, 16, 16, 0, false, 2, 1, 12, 14, 0, 0, 0, 0, 0 },
  { ENV_TREE_FIR, 8, 8, 0, 48, 16, 16, 0, false, 3, 1, 10, 14, 0, 0, 0, 0, 0 },
  { ENV_FOREST, 8, 8, 16, 48, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },
  { ENV_WEB, 8, 8, 32, 48, 16, 16, 0, false, 1, 1, 14, 14, 0, 0, 0, 0, 0 },

};
