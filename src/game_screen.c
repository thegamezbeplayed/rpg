#include "raylib.h"
#include "game_tools.h"
#include "game_process.h"
#include "game_ui.h"
#if defined(PLATFORM_ANDROID)
#include <jni.h>
#endif
mouse_controller_t mousectrl;
play_area_t play_area;
camera_t* cam;

key_controller_t keyctrl;

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
  for(int i = 0; i < ELEMENT_COUNT; i++)
    play_area.screen_icons[i] = InitSpriteByID(i,SHEET_UI);
}

void InitCamera(float zoom, float rot, Vector2 offset, Vector2 target){
  cam = calloc(1,sizeof(camera_t));
  Camera2D* raycam = calloc(1,sizeof(Camera2D));

  raycam->offset = offset;
  raycam->rotation = rot;
  raycam->zoom = zoom;

  cam->size = CellScale(offset,1/16);
  raycam->target = target;

  cam->target = CELL_UNSET;
  cam->camera = raycam;
}

bool ScreenCameraSetView(Cell v){
  int vtop = v.y - cam->size.x/2;
  int vleft = v.x - cam->size.y/2;
  cam->view = Rect(vleft,vtop,cam->size.x,cam->size.y);
  cam->view = clamp_rect_to_bounds(cam->view,cam->bounds);
  return true;
}

void ScreenCameraSetBounds(Cell b){
  cam->bounds = Rect(0,0,b.x,b.y);
}

Rectangle ScreenGetCameraView(void){
  return cam->view;
}

void ScreenCameraToggle(void){
  cam->mode = !cam->mode;

  if(cam->mode)
    BeginMode2D(*cam->camera);
  else
    EndMode2D();
}

bool ScreenCameraSyncView(Cell target){

  ScreenCameraSetView(target);
  return true;
}

void ScreenCameraSync(Cell target){
  Vector2 vpos =  CellToVector2(target,CELL_WIDTH);

  if(Vector2Distance(cam->camera->target,vpos) < 64){
    cam->camera->target = vpos;
    return;
  }
  
  if(!ScreenCameraSyncView(target))
    return;

  cam->camera->target = Vector2Lerp(cam->camera->target,vpos,0.2);

  cam->target = target;


}

void ScreenRender(void){
  
  if(!keyctrl.active)
    return;

  play_area.screen_icons[UI_SELECTOR_EMPTY]->is_visible = true;
  DrawSpriteAtPos(play_area.screen_icons[UI_SELECTOR_EMPTY],CellToVector2(keyctrl.pos,CELL_WIDTH));
  for (int i = 0; i < keyctrl.selected; i++){
    if(!keyctrl.selections[i])
      continue;

    play_area.screen_icons[UI_SELECTOR_CHOSEN]->is_visible = true;
    Vector2 pos = CellToVector2(keyctrl.selections[i]->coords,CELL_WIDTH);
    DrawSpriteAtPos(play_area.screen_icons[UI_SELECTOR_CHOSEN],pos);

  }
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

  keyctrl = (key_controller_t) {0};
}

void ScreenActivateSelector(Cell pos, int num, bool occupied, SelectionCallback on_select){
  keyctrl.active = true;
  keyctrl.pos = pos;
  keyctrl.desired = num;
  keyctrl.occupied = occupied;
  keyctrl.selected = 0;
  keyctrl.on_select = on_select;
}

bool ScreenSelectorInput(void){
  if(!keyctrl.active)
    return false;

  for(int i = 0; i < ACTION_DONE; i++){
    ActionType a = selector_keys[i].action;
    ActionKeyCallback fn = selector_keys[i].fn;
    for(int j = 0; j<selector_keys[i].num_keys; j++){
      KeyboardKey k = selector_keys[i].keys[j];
      if(!IsKeyPressed(k))
        continue;

      //return fn(player,a,k,SLOT_NONE);
    }
  }
}

key_controller_t* ScreenGetSelection(void){
  return &keyctrl;
}

bool ScreenMakeSelection(struct ent_s* e, ActionType a, KeyboardKey k, int binding){
  map_cell_t* sel = WorldGetTile(keyctrl.pos);

  if(keyctrl.occupied && sel->occupant==NULL)
    return false;

  keyctrl.selections[keyctrl.selected++] = sel;
  if(keyctrl.on_select){
    local_ctx_t* ctx = WorldGetContext(DATA_MAP_CELL, sel->gouid);
    keyctrl.on_select(player, a, ctx);
  }
  return true;
}

bool ScreenMoveSelector(struct ent_s* e, ActionType a, KeyboardKey k, int binding){

  Cell dir;

  switch(k){
    case KEY_A:
    case KEY_LEFT:
      dir = CELL_LEFT;
      break;
    case KEY_D:
    case KEY_RIGHT:
      dir = CELL_RIGHT;
      break;
    case KEY_W:
    case KEY_UP:
      dir = CELL_UP;
      break;
    case KEY_S:
    case KEY_DOWN:
      dir = CELL_DOWN;
      break;
    default:
      break;
  }

  keyctrl.pos = CellInc(keyctrl.pos,dir);

  return true;

}

void ClearMouse(void){
  mousectrl.target = NULL;
  mousectrl.offset = VECTOR2_ZERO;
  mousectrl.is_dragging = false;
}

void ScreenSyncMouse(void){

  mousectrl.pos = GetMousePosition();
  ScreenEntMouseHover();
  if(!mousectrl.target && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){

  //  mousectrl.target = ScreenEntMouseCollision();
    if(mousectrl.target){
      mousectrl.offset = Vector2Subtract(mousectrl.pos, mousectrl.target->sprite->pos);
      mousectrl.is_dragging = true;
    }
    else
      mousectrl.is_dragging =false;
  }
  else if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)){
    if(mousectrl.target){

    }
    ClearMouse(); 
  }
  else if (!mousectrl.is_dragging){
   }

  if(mousectrl.is_dragging){
  }
}

void ScreenSyncKey(void){
  if(ScreenSelectorInput())
    if(keyctrl.selected >= keyctrl.desired){
      keyctrl.active = false;
      //ActionType next = ActionGetEntNext(player);
      //action_turn_t* action = player->actions[next];
      //TakeAction(player,action);
    }
}

Vector2 CaptureInput(){
  Vector2 input = {0.0f,0.0f};

  if (IsKeyDown(KEY_D)) input.x += 1.0f;
  if (IsKeyDown(KEY_A)) input.x -= 1.0f;
  if (IsKeyDown(KEY_W)) input.y -= 1.0f;
  if (IsKeyDown(KEY_S)) input.y += 1.0f;

  return input;
}

ent_t* ScreenEntMouseHover(void){
  Cell c = vec_to_cell(mousectrl.pos,CELL_WIDTH); 

  Vector2 pos = GetScreenToWorld2D(mousectrl.pos,*cam->camera);
  sprite_t* ent_sprites[MAX_ENTS];
  int num = WorldGetEntSprites(ent_sprites);

  ent_t *e = {0};
  for (int i = 0; i < num; i++){
    if(Vector2Distance(pos,ent_sprites[i]->pos)<CELL_WIDTH){
      e = ent_sprites[i]->owner;
    }
  }
  
  if(e && e!=mousectrl.hover){
    mousectrl.hover = e;
    //PrintMobDetail(e);
  }
  return e;
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
