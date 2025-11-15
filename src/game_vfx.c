#include "game_tools.h"
#include "game_assets.h"
#include "game_utils.h"

gl_shader_t shaders[SHADER_DONE];

void InitShaders(){
  for (int i = 0; i < SHADER_DONE; i++)
    shaders[i] = (gl_shader_t){0};
  
  FilePathList shaderfiles = LoadDirectoryFiles("resources/shaders/glsl100");
  for (int i = 0; i < shaderfiles.count; i++){
    const char* filename = GetFileName(shaderfiles.paths[i]);
    char* shadername = GetFileStem(filename);
    const char* extStr = GetFileExtension(filename);

    ShaderType type = ShaderTypeLookup(shadername);
    gl_shader_t *s = &shaders[type];
    if(shaders[type].stype == SHADER_NONE){
      s->stype = type;
      s->version = 100;
    }
    
    if(strcmp(extStr,".vs") == 0)
      s->vs_path = strdup(shaderfiles.paths[i]);
    else if (strcmp(extStr,".fs") == 0)
      s->fs_path = strdup(shaderfiles.paths[i]);
  
    free(shadername);
  }

  UnloadDirectoryFiles(shaderfiles);
}

void LoadShaders(){
  for(int i = 0; i < SHADER_DONE;i++){
    if(shaders[i].stype == SHADER_NONE)
      continue;

    shaders[i].shader = LoadShader(shaders[i].vs_path, shaders[i].fs_path);
  }
}

void ShaderSetUniforms(gl_shader_t *s, Texture2D texture){
  if(s->stype != SHADER_OUTLINE)
    return;
  float textureSize[2] = { (float)texture.width, (float)texture.height };

  // Get shader locations
  int outlineSizeLoc = GetShaderLocation(s->shader, "outlineSize");
  int outlineColorLoc = GetShaderLocation(s->shader, "outlineColor");
  int textureSizeLoc = GetShaderLocation(s->shader, "textureSize");

  SetShaderValue(s->shader, outlineSizeLoc, s->uniforms[UNIFORM_OUTLINESIZE].val, SHADER_UNIFORM_FLOAT);
  SetShaderValue(s->shader, outlineColorLoc, s->uniforms[UNIFORM_OUTLINECOLOR].val, SHADER_UNIFORM_VEC4);
  SetShaderValue(s->shader, textureSizeLoc, textureSize, SHADER_UNIFORM_VEC2);

}

ShaderType ShaderTypeLookup(const char* name){
  for (int i = 0; i < SHADER_DONE; i++){
    if(strcmp(name, shader_alias[i].name) == 0)
      return shader_alias[i].type;
  }

  TraceLog(LOG_WARNING,"<====GAME_DATA====>\n====== State Definition for %s not implemented!",name);
  return 0;

}
