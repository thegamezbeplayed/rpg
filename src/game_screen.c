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
  float scale = GetApproxDPIScale();
  SPRITE_SCALE = scale*GetApproxDPIScale();;
  TraceLog(LOG_INFO,"Render Sprite Scaling %0.2f",SPRITE_SCALE);
  UI_SCALE = scale;
  play_area.sizes[SIZE_SCALE] = SPRITE_SCALE;
  play_area.sizes[SIZE_GRID] = GRID_WIDTH * scale;
  play_area.sizes[SIZE_CELL] = CELL_WIDTH * scale;

  play_area.area[AREA_PLAY] = Rect(ROOM_WIDTH/2,ROOM_HEIGHT/2, ROOM_WIDTH, ROOM_HEIGHT);
  play_area.area[AREA_SCREEN] = Rect(0,0, DESIGN_WIDTH, DESIGN_HEIGHT);
  play_area.area[AREA_UI] = Rect(0,0,DESIGN_WIDTH*scale,DESIGN_HEIGHT * scale);
  for(int i = 0; i < ELEMENT_COUNT; i++)
    play_area.screen_icons[i] = InitSpriteByID(i,SHEET_UI);
}

void InitCamera(float zoom, float rot, Vector2 offset, Vector2 target){
  cam = GameCalloc("InitCamera", 1,sizeof(camera_t));
  Camera2D* raycam = GameCalloc("InitCamera", 1,sizeof(Camera2D));

  raycam->offset = offset;
  raycam->offset.y*=2;
  raycam->rotation = rot;
  raycam->zoom = zoom;

  cam->size = CellScale(offset,1/16);
  raycam->target = target;

  float hei_disp = 0;
  cam->bounds = Rect(offset.x, hei_disp, ROOM_WIDTH, ROOM_HEIGHT);
  cam->view = Rect(0, 0, ROOM_WIDTH, -ROOM_HEIGHT);
  cam->render = LoadRenderTexture(cam->bounds.width, cam->bounds.height);
  cam->target = CELL_UNSET;
  cam->camera = raycam;
}

bool ScreenCameraSetView(Cell v){
  /*
  int vtop = v.y - cam->size.x/2;
  int vleft = v.x - cam->size.y/2;
  cam->view = Rect(vleft,vtop,cam->size.x,cam->size.y);
  cam->view = clamp_rect_to_bounds(cam->view,cam->bounds);
  */
  return true;
}

void ScreenCameraSetBounds(Cell b){
  //cam->bounds = Rect(0,0,b.x,b.y);
}

Rectangle ScreenGetCameraView(void){
  return cam->view;
}

void ScreenCameraToggle(void){
  cam->mode = !cam->mode;

  if(cam->mode){
    BeginTextureMode(cam->render);
    ClearBackground(BLACK); 
    BeginMode2D(*cam->camera);

  }
  else{
    EndMode2D();
    EndTextureMode();
    DrawTexturePro(cam->render.texture, cam->view, cam->bounds, VECTOR2_ZERO, 0, WHITE);
  }
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

void ScreenApplyContext(local_ctx_t* ctx[SCREEN_CTX_ALL]){
    for(int i = 0; i < SCREEN_CTX_ALL; i++)
      ctx[i] = mousectrl.ctx[i];
}

void* ScreenSelectContext(void*){
  return mousectrl.ctx[SCREEN_CTX_TAR];
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
  for(int i = 0; i < SCREEN_CTX_ALL; i++)
    mousectrl.ctx[i] = NULL;
  
  mousectrl.offset = VECTOR2_ZERO;
  mousectrl.is_dragging = false;
}

/*
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
    SetHoverContext(e);
  }
  return e;
}
*/

bool SetHoverContext(local_ctx_t* ctx){
  if(ctx == NULL)
    return false;

  if(ctx == mousectrl.ctx[SCREEN_CTX_HOVER])
    return false;

  if(ctx->gouid == player->gouid)
    return false;

  mousectrl.ctx[SCREEN_CTX_HOVER] = ctx;

  WorldEvent(SCREEN_EVENT_SELECT, ctx, ctx->gouid);
  return true;
}

bool ScreenMouseFindContext(void){
  Cell c = vec_to_cell(mousectrl.pos,CELL_WIDTH); 

  Vector2 pos = GetScreenToWorld2D(mousectrl.pos,*cam->camera);

  Cell tile = vec_to_cell(pos, CELL_WIDTH);

  map_cell_t* mc = MapGetTile(WorldGetMap(), tile);
  if(!mc || mc->vis < VIS_HAS)
    return false;

  local_ctx_t* tar = {0};
  if(mc->occupant)
    tar = WorldGetContext(DATA_ENTITY, mc->occupant->gouid); 
  /*
  else if (mc->tile)
    tar = WorldGetContext(DATA_ENV, mc->tile->gouid);
*/
 return SetHoverContext(tar); 

}

void ScreenSyncMouse(void){

  mousectrl.pos = GetMousePosition();
  ScreenMouseFindContext();
  /*
  if(!mousectrl.target && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){

  //  mousectrl.target = ScreenEntMouseCollision();
    if(mousectrl.target){
      //mousectrl.offset = Vector2Subtract(mousectrl.pos, mousectrl.target->sprite->pos);
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
  */
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

local_ctx_t* ScreenPlayerContext(void*){
  for(int i = 0; i < SCREEN_CTX_ALL; i++){
    if(mousectrl.ctx[i])
      return mousectrl.ctx[i];

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
