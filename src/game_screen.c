#include "raylib.h"
#include "game_tools.h"
#include "game_process.h"
#if defined(PLATFORM_ANDROID)
#include <jni.h>
#endif
mouse_controller_t mousectrl;
play_area_t play_area;

void InitPlayArea(void){
  float sx = GetScreenWidth()/DESIGN_WIDTH;
  float sy = GetScreenHeight()/DESIGN_HEIGHT;
  float scale = GetApproxDPIScale();
  SPRITE_SCALE = scale*GetApproxDPIScale();;
  TraceLog(LOG_INFO,"Render Sprite Scaling %0.2f",SPRITE_SCALE);
  UI_SCALE = scale;
  play_area.sizes[SIZE_SCALE] = SPRITE_SCALE;
  play_area.sizes[SIZE_GRID] = GRID_WIDTH * scale;
  play_area.sizes[SIZE_CELL] = CELL_WIDTH * scale;

  play_area.area[AREA_PLAY] = Rect(0,0,ROOM_WIDTH,ROOM_HEIGHT);
  play_area.area[AREA_UI] = Rect(0,0,DESIGN_WIDTH*scale,DESIGN_HEIGHT * scale);
}

float ScreenSized(PlaySizes s){
  return play_area.sizes[s];
}

Vector2 ScreenAreaStart(ScreenArea t){
  ScreenCalcAreas();

  return RectXY(play_area.area[AREA_PLAY]);
}

void ScreenCalcAreas(void){
  Vector2 ps = RectStart(play_area.area[AREA_UI],play_area.area[AREA_PLAY]);
  play_area.area[AREA_PLAY] = RectPos(ps,play_area.area[AREA_PLAY]);
}

void InitScreenInteractive(void){
  mousectrl = (mouse_controller_t) {
    .is_dragging = false,
      .pos = GetMousePosition(),
      .offset = VECTOR2_ZERO,
      .target = NULL
  };
}

void ClearMouse(void){
  mousectrl.target = NULL;
  mousectrl.offset = VECTOR2_ZERO;
  mousectrl.is_dragging = false;
}

void ScreenSyncMouse(void){

  mousectrl.pos = GetMousePosition();
  if(!mousectrl.target && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){

    mousectrl.target = ScreenEntMouseCollision();
    if(mousectrl.target){
      mousectrl.offset = Vector2Subtract(mousectrl.pos, mousectrl.target->pos);
      mousectrl.is_dragging = true;
    }
    else
      mousectrl.is_dragging =false;
  }
  else if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)){
    if(mousectrl.target){

      SetState(mousectrl.target,STATE_PLACED,SetViableTile);
      EntSetPos(mousectrl.target, mousectrl.target->owner->pos);
    }
    ClearMouse(); 
  }
  else if (!mousectrl.is_dragging){
  /*
    ent_t* hover = ScreenEntMouseHover();
    if(mousectrl.hover && mousectrl.hover != hover){
      SetState(mousectrl.hover,STATE_IDLE,NULL);
    }

    mousectrl.hover = hover;
    if(mousectrl.hover)
      SetState(mousectrl.hover,STATE_HOVER,NULL);
  */
   }

  if(mousectrl.is_dragging){
    SetState(mousectrl.target,STATE_SELECTED,NULL);
    EntSetPos(mousectrl.target, Vector2Add(mousectrl.pos,mousectrl.offset));
  }
}

ent_t* ScreenEntMouseHover(void){
  ent_t* shape_pool[GRID_WIDTH * GRID_HEIGHT];

  int num_shapes = WorldGetEnts(shape_pool,FilterEntShape, NULL);
  if(num_shapes <= 0)
    return NULL;

  for(int i = 0; i < num_shapes; i++){
    if(CheckEntPosition(shape_pool[i],mousectrl.pos))
      return shape_pool[i];
  }

  return NULL;
}
ent_t* ScreenEntMouseCollision(void){
  ent_t* shape_pool[GRID_WIDTH * GRID_HEIGHT];

  int num_shapes = WorldGetEnts(shape_pool,FilterEntShape, NULL);
  if(num_shapes <= 0)
    return NULL;

  for(int i = 0; i < num_shapes; i++){
    if(shape_pool[i]->control->moves<1)
      continue;
    if(CheckEntPosition(shape_pool[i],mousectrl.pos))
      return shape_pool[i];
  }

  return NULL;
}

float GetApproxDPIScale(void)
{
#if defined (PLATFORM_ANDROID)
  TraceLog(LOG_INFO," Render Width %0.4f", (float)GetRenderWidth());
    return (float)(float)GetRenderWidth()/DESIGN_HEIGHT;
#else
    return 1.0f;
#endif
    //return (float)(float)GetRenderWidth()/DESIGN_WIDTH;
}
