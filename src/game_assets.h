#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

//#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include "game_common.h"

#define MAX_SONGS 4
#define MAX_SPRITES 64

struct ent_s;
void InitResources();

typedef enum{
  ANIM_NONE,
  ANIM_IDLE,
  ANIM_BOUNCE,
  ANIM_RETURN,
  ANIM_DONE,
  ANIM_COUNT
}AnimState;

typedef struct{
  int duration;
  int elapsed;

}anim_t;

typedef enum{
  TILES,
  SHAPES,
}SheetID;
//====SHADERS===>
typedef struct{
  bool              has_chain[SHAPE_NONE];
  RenderTexture2D   chain1[SHAPE_NONE];
  RenderTexture2D   chain2[SHAPE_NONE];
}texture_chain_t;

typedef enum{
  SHADER_NONE,
  SHADER_BASE,
  SHADER_BLOOM,
  SHADER_BLUR,
  SHADER_INVERT,
  SHADER_OUTLINE,
  SHADER_DONE
}ShaderType;

typedef struct{
  const char* name;
  ShaderType  type;
}ShaderTypeAlias;

typedef enum {
  UNIFORM_TEXSIZE,
  UNIFORM_OUTLINESIZE,
  UNIFORM_OUTLINECOLOR,
  UNIFORM_COUNT,
  UNIFORM_NONE
} ShaderUniform;

typedef enum{
  STANDARD_INT,
  STANDARD_FLOAT,
  STANDARD_UINT,
  STANDARD_BOOL,
  STANDARD_VEC2,
  STANDARD_VEC3,
  STANDARD_VEC4,
}UniformTypes;

typedef struct{
  ShaderUniform uniform;
  UniformTypes  type;
  void*         val;
}shader_uniform_t;

static ShaderTypeAlias shader_alias[SHADER_DONE] = {
  {"base", SHADER_BASE},
  {"bloom", SHADER_BLOOM},
  {"blur", SHADER_BLUR},
  {"invert", SHADER_INVERT},
  {"outline", SHADER_OUTLINE},
};

ShaderType ShaderTypeLookup(const char* name);

typedef struct{
  ShaderType        stype;
  int               version;
  Shader            shader;
  const char*       vs_path;
  const char*       fs_path;
  shader_uniform_t  uniforms[UNIFORM_NONE];
}gl_shader_t;
extern gl_shader_t shaders[SHADER_DONE];

void InitShaderChainCache(ShapeID,int maxWidth, int maxHeight);
void InitShaders();
void LoadShaders();
void ShaderSetUniforms(gl_shader_t *s, Texture2D texture);
//<=====SHADERS====

typedef struct{
  int     duration;
  int     elapsed;
  float   vol;
  bool    increase;
}audio_fade_t;

typedef struct{
  Music        music;
  events_t     *events;
  audio_fade_t *fade;
}music_track_t;

typedef struct{
  unsigned int  index;
  int           num_songs;
  unsigned int  current_index;
  float         volume;
  music_track_t track[MAX_SONGS];
}music_group_t;

typedef enum{
  SFX_ALL,
  SFX_UI,
  SFX_ACTION,
  SFX_IMPORTANT,
  SFX_DONE
}SfxGroup;

typedef enum {
  ACTION_HOVER,
  ACTION_PLACE,
  END_SFX
}SfxType;

typedef struct{
  SfxType     type;
  SfxGroup    group;
  int         num_sounds;
  const char* paths[5];
}sfx_info_t;

typedef struct{
  SfxType   type;
  int       num_sounds;
  Sound     *sounds;
}sfx_sound_t;

static sfx_info_t sfx_catalog[]={
  {ACTION_PLACE,SFX_ACTION,4,{"switch_001.ogg","switch_002.ogg","switch_003.ogg","switch_004.ogg"}},
  {ACTION_HOVER,SFX_ACTION,5,{"click_001.ogg","click_002.ogg","click_003.ogg","click_004.ogg","click_005.ogg"}},
};

typedef struct{
  int         num_types;
  float       volume;
  sfx_sound_t items[END_SFX];
}sfx_group_t;

typedef struct{
  music_group_t   tracks[6];
  float           volume;
  int             current_track;
  int             concurrent_track;
  int             num_tracks;
  sfx_group_t     sfx[SFX_DONE];
  events_t        *timers[SFX_DONE];
}audio_manager_t;

void InitAudio();
int AudioBuildMusicTracks(const char* subdir);
void AudioStep();
void AudioPlaySfx(SfxGroup group, SfxType type, int index);
void AudioPlayRandomSfx(SfxGroup group, SfxType type);
void AudioPlayMusic(int index);
bool AudioPlayNext(music_group_t* t);
void AudioMusicFade(music_track_t* t);

typedef enum{
  LAYER_ROOT = -1,
  LAYER_BG,
  LAYER_MAIN,
  LAYER_TOP,
  LAYER_DONE
}RenderLayer;

typedef struct {
  int       id;
  int       sequence_index;
  Vector2   center;
  Rectangle bounds;
  Vector2   offset;
  float     scale;
} sprite_slice_t;

typedef enum {
  SLICE_NONE =-1,
  SLICE_TOP_LEFT,
  SLICE_TOP_MID,
  SLICE_TOP_RIGHT,
  SLICE_EDGE_LEFT,
  SLICE_CENTER,
  SLICE_EDGE_RIGHT,
  SLICE_BOT_LEFT,
  SLICE_BOT_MID,
  SLICE_BOT_RIGHT,
  SLICE_ALL
}SliceParts;

typedef enum{
  SCALE_NONE,
  SCALE_STRETCH_W,
  SCALE_STRETCH_H,
  SCALE_NORMAL
}ScalingType;

typedef struct {
  ScalingType      scaling[SLICE_ALL];
  sprite_slice_t   *slices[SLICE_ALL];
  float             scale;
} scaling_slice_t;

typedef struct{
  scaling_slice_t *sprites[ELEMENT_COUNT];
  Texture2D       *sprite_sheet;
}scaling_sprite_data_t;

typedef struct{
  SliceParts  type;
  ScalingType rules;
}scaling_rules_t;

static scaling_rules_t scaling_rules[SLICE_ALL] = {
  {SLICE_TOP_LEFT, SCALE_NONE},
  {SLICE_TOP_MID, SCALE_STRETCH_W},
  {SLICE_TOP_RIGHT, SCALE_NONE},
  {SLICE_EDGE_LEFT, SCALE_STRETCH_H},
  {SLICE_CENTER, SCALE_NORMAL},
  {SLICE_EDGE_RIGHT,SCALE_STRETCH_H},
  {SLICE_BOT_LEFT, SCALE_NONE},
  {SLICE_BOT_MID,  SCALE_STRETCH_W},
  {SLICE_BOT_RIGHT, SCALE_NONE}
};

typedef struct{
  int             num_sprites;
  sprite_slice_t  *sprites[128];
  Texture2D       *sprite_sheet;
}sprite_sheet_data_t;

extern sprite_sheet_data_t shapedata;
extern sprite_sheet_data_t tiledata;
void SpriteLoadSubTextures(sprite_sheet_data_t *out, int sheet_id);
void SpriteLoadSlicedTextures();
//SPRITE_T===>
typedef struct {
  int             suid;
  AnimState       state;
  anim_t          *anim;
  Texture2D       *sheet;
  sprite_slice_t* slice;
  gl_shader_t*    gls[SHADER_DONE];
  bool            is_visible;
  float           rot;
  Vector2         offset;
  Vector2         pos;
  RenderLayer     layer;
  struct ent_s    *owner;
} sprite_t;

void DrawSlice(sprite_t *spr, Vector2 position,float rot);
void DrawNineSlice(scaling_slice_t *spr, Rectangle dst);
sprite_t* InitSpriteByID(int id, sprite_sheet_data_t* data);
sprite_t* InitSpriteByIndex(int index, sprite_sheet_data_t* spritesheet);
scaling_slice_t* InitScalingElement(ElementID id);
bool FreeSprite(sprite_t* s);
void DrawSprite(sprite_t* s);
void DrawSpriteAtPos(sprite_t*s , Vector2 pos);
void SpriteSync(sprite_t *spr);
bool SpriteSetAnimState(sprite_t* spr, AnimState s);
bool SpriteCanChangeState(AnimState old, AnimState s);
void SpriteAnimate(sprite_t *spr);
void SpritePreprocessImg(Image *img, Texture2D* out);
//====SPRITE_T>>
#endif
