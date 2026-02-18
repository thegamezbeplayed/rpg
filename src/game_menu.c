//#include "game_ui.h"
#include "game_utils.h"
#include "screens.h"
#include "game_tools.h"
#include "game_process.h"
#include "game_ui.h"
#include "game_strings.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

static ui_element_t* active_tooltip = NULL;

int GuiLabel(Rectangle bounds, const char *text)
{
    GuiState state = guiState;

    if ((state != STATE_DISABLED) && !guiLocked && !guiControlExclusiveMode)
    {
      Vector2 mousePoint = GetMousePosition();

      // Check button state
      if (CheckCollisionPointRec(mousePoint, bounds))
        state = STATE_FOCUSED;

    }
    GuiDrawText(text, GetTextBounds(LABEL, bounds), GuiGetStyle(LABEL, TEXT_ALIGNMENT), GetColor(GuiGetStyle(LABEL, TEXT + (state*3))));
    //--------------------------------------------------------------------
    return state;
}

ui_element_t* InitElementByName(const char* name, ui_menu_t* m, ui_element_t* o){
  ui_element_d d;

  for (int i = 0; i < MAX_SUB_ELE; i++){
    if(strcmp(name, ELEM_DATA[i].identifier) != 0)
      continue;

    ui_element_d d = ELEM_DATA[i];
    ui_element_t *e = calloc(1,sizeof(ui_element_t));

    *e = (ui_element_t){
      .hash     = hash_str_32(d.identifier),
      .type     = d.type,
      .state    = d.state,
      .layout   = d.layout,
      .align    = d.align,
      .set_val  = d.set,
      .get_ctx  = d.context,
      .menu     = m,
      .owner    = o,
      .bounds   = Rect(d.pos.x,d.pos.y,d.size.x,d.size.y),
      .width    = d.size.x,
      .height   = d.size.y
    };

    strcpy(e->name ,d.identifier);
    replace_char(e->name, '_', ' ');
    for (int j = 0; j < UI_POSITIONING; j++)
      e->spacing[j] = d.spacing[j];

    for (int j = 0; j < ELEMENT_DONE; j++){
      if(d.cb[j])
        e->cb[j] = d.cb[j];
    }

    for (int j = 0; j < d.num_children; j++){
      const char* c_name;
      if (d.kids[j][0] =='\0')
        c_name = d.kids[0];
      else
        c_name = d.kids[j];

      ElementAddChild(e, InitElementByName(c_name, m, e));
      ui_element_t* c = e->children[j];
      for(int k = 0; k < PARAM_ALL; k++){
        if(d.params[j][k] == PARAM_NONE)
          continue;

        c->params[c->num_params++] = d.params[j][k];
      }
    }
    return e;

  }
  return NULL;
}

void InitMenuById(MenuId id){
  ui_menu_d d = MENU_DATA[id];

  ui_menu_t m = {0};

  m.state = d.state;
  m.is_modal = d.is_modal;

  m.element = InitElementByName(d.element, &m, NULL);
  
  for(int i = 0; i < MENU_END; i++){
    if(d.cb[i])
      m.cb[i] = d.cb[i];
    else
      m.cb[i] = MenuInert;
  }

  ui.menus[id] = m;
}

void GuiDrawRectangle(Rectangle rec, int borderWidth, Color borderColor, Color baseColor) { }
ui_manager_t ui;
void InitUI(void){
  Font font = LoadFontEx("resources/fonts/kenney-pixel-square.ttf", 64, 0, 0);
  SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);
  GuiSetFont(font);
  ui.font = GuiGetFont();
#if defined(PLATFORM_WEB)
  GuiSetStyle(DEFAULT,TEXT_SIZE,27);
#elif defined(PLATFORM_ANDROID)
  GuiSetStyle(DEFAULT,TEXT_SIZE,58);
#else
  GuiSetStyle(DEFAULT,TEXT_SIZE,20*UI_SCALE);
#endif

  ui.text_size = GuiGetStyle(DEFAULT, TEXT_SIZE);
  ui.text_spacing = GuiGetStyle(DEFAULT, TEXT_SPACING);
  SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);
  GuiSetStyle(DEFAULT,TEXT_ALIGNMENT,TEXT_ALIGN_LEFT);
  GuiSetStyle(DEFAULT,TEXT_ALIGNMENT_VERTICAL,TEXT_ALIGN_TOP);
  GuiSetStyle(LABEL,TEXT_ALIGNMENT,TEXT_ALIGN_LEFT);
  GuiSetStyle(DEFAULT,BORDER_COLOR_NORMAL,ColorToInt(WHITE));
  GuiSetStyle(TEXTBOX,TEXT_SIZE,8);

  for (int i = 0; i< MENU_DONE; i++)
    ui.menu_key[i] = KEY_NULL;


  ui.menus[MENU_MAIN] = InitMenu(MENU_MAIN,VECTOR2_ZERO,DEFAULT_MENU_SIZE,ALIGN_CENTER|ALIGN_MID,LAYOUT_VERTICAL,false);

  ui_element_t *playBtn = InitElement("PLAY_BTN",UI_BUTTON,VECTOR2_ZERO,DEFAULT_BUTTON_SIZE,ALIGN_CENTER|ALIGN_MID,0); 
  strcpy(playBtn->text, "PLAY");
  playBtn->cb[ELEMENT_ACTIVATE] = UITransitionScreen;
  ElementAddChild(ui.menus[MENU_MAIN].element,playBtn);

  ui_element_t *continueBtn = InitElement("CONTINUE_BTN",UI_BUTTON,VECTOR2_ZERO,DEFAULT_BUTTON_SIZE,ALIGN_CENTER|ALIGN_MID,0); 
  strcpy(continueBtn->text, "CONTINUE");
  continueBtn->cb[ELEMENT_ACTIVATE] = UITransitionScreen;
 
  ui.menus[MENU_OPTIONS] = InitMenu(MENU_OPTIONS, VECTOR2_ZERO,DEFAULT_MENU_SIZE,ALIGN_CENTER,LAYOUT_HORIZONTAL,false);
  
  ui.menus[MENU_HUD] = InitMenu(MENU_HUD, VECTOR2_ZERO, VECTOR2_ZERO, ALIGN_CENTER, LAYOUT_HORIZONTAL, false);


  InitMenuById(MENU_HUD);
}

ui_menu_t InitMenu(MenuId id,Vector2 pos, Vector2 size, UIAlignment align,UILayout layout, bool modal){
  ui_menu_t m = {0};

  m.is_modal = modal;
  ElementType menutype = UI_PANEL;
  if(m.is_modal){
    m.cb[MENU_CLOSE] = UICloseMenu;
    menutype = UI_MASK;
  }

  m.element = InitElement("UI_MENU_DOM",menutype,pos,size,align,layout);
  for(int i = 0; i < MENU_END; i++)
    m.cb[i] = MenuInert;

  return m;
}

ui_element_t* GetElement(const char* name){
  uint32_t hash = hash_str_32(name);

  for (int i = 0; i < MAX_ELEMENTS; i++){
    if(ui.elements[i]->hash != hash)
      continue;

    return ui.elements[i];
  }

  return NULL;
}

ui_element_t* InitElement(const char* name, ElementType type, Vector2 pos, Vector2 size, UIAlignment align,UILayout layout){
  ui_element_t* u = malloc(sizeof(ui_element_t));
  *u = (ui_element_t) {0};

  u->hash = hash_str_32(name);
  u->num_children = 0;
  u->type = type;
  u->state = ELEMENT_IDLE;
  u->sync_val = NULL;//CHAR_DO_NOTHING;
  u->bounds = Rect(pos.x,pos.y,size.x,size.y);
  u->width = size.x;
  u->height = size.y;
  //strcpy(u->text,name);
  for(int i = 0; i < ELEMENT_DONE; i++)
    u->cb[i] = UI_BOOL_DO_NOTHING;

  for (int i = 0; i < UI_POSITIONING; i++)
    u->spacing[i] = 0.0f;

  u->align = align;
  u->layout= layout;
  ui.elements[ui.num_elements++] = u;
  return u;
}

void ElementAddChild(ui_element_t *o, ui_element_t* c){
  Vector2 pos_adjustment = Vector2FromXY(c->bounds.x,c->bounds.y);
  c->index = o->num_children;
  c->menu = o->menu;
  c->owner = o;
  o->children[o->num_children++] = c;
}

float ElementGetHeightSum(ui_element_t *e){
  if(e->state < ELEMENT_LOAD)
    return 0;

  float height = e->bounds.height;// + e->spacing[UI_MARGIN] + e->spacing[UI_MARGIN_TOP];

  float cheight = 0;
  if (e->layout == LAYOUT_GRID){
      cheight = e->bounds.height * e->owner->num_children / GRID_HEIGHT;
      return height+cheight;
  }

  for(int i = 0; i < e->num_children; i++){
    if(e->children[i]->align & ALIGN_OVER)
      continue;

    if(e->layout == LAYOUT_VERTICAL)
      cheight += ElementGetHeightSum(e->children[i]);
    else
      cheight = (cheight < ElementGetHeightSum(e->children[i]))?ElementGetHeightSum(e->children[i]):cheight;
  }
  return height+cheight;
}

float ElementGetWidthSum(ui_element_t *e){
  if(e->state < ELEMENT_IDLE)
    return 0;

  float width = e->bounds.width;// + e->spacing[UI_MARGIN] + e->spacing[UI_MARGIN_LEFT];

  float cwidth = 0;
  if (e->layout == LAYOUT_GRID){
    cwidth = e->bounds.width * e->owner->num_children / GRID_WIDTH;
    return width+cwidth;
  }

  for(int i = 0; i < e->num_children; i++){
    if(e->children[i]->align & ALIGN_OVER)
      continue;

    if(e->layout == LAYOUT_HORIZONTAL)
      cwidth += ElementGetWidthSum(e->children[i]);
    else
      cwidth = (cwidth < ElementGetWidthSum(e->children[i]))?ElementGetWidthSum(e->children[i]):cwidth;
  }

  return width+cwidth;
}


void ElementResize(ui_element_t *e){
  float centerx = VECTOR2_CENTER_SCREEN.x;
  float centery = VECTOR2_CENTER_SCREEN.y;
  float xinc = e->bounds.x;
  float yinc = e->bounds.y;

  e->bounds.x = e->bounds.y = 0;
  float omarginx = e->spacing[UI_MARGIN] + e->spacing[UI_MARGIN_LEFT];
  float omarginy = e->spacing[UI_MARGIN] + e->spacing[UI_MARGIN_TOP];

  float paddingx = 0;
  float paddingy = 0;
  float owidth = omarginx;
  float oheight = omarginy; 
  float cwidths =0, cheights= 0;

  if(e->num_children == 6)
    DO_NOTHING();

  for(int i = 0; i<e->num_children; i++){
    if(e->children[i]->align & ALIGN_OVER){
      DO_NOTHING();
    }
    else{
      switch(e->layout){
        case LAYOUT_VERTICAL:
          oheight+=ElementGetHeightSum(e->children[i]);
          if(ElementGetWidthSum(e->children[i]) > cwidths)
            cwidths = ElementGetWidthSum(e->children[i]);
          break;
        case LAYOUT_HORIZONTAL:
          //owidth += ElementGetWidthSum(e->children[i]);
          if(e->children[i]->height > cheights)
            cheights = e->children[i]->height;
          break;
        case LAYOUT_GRID:
          cwidths = 0;//ElementGetWidthSum(e->children[i]);
          cheights = 0;//ElementGetHeightSum(e->children[i]);
          break;
        case LAYOUT_FREE:
          int widths = ElementGetWidthSum(e->children[i]);
          if (widths > e->bounds.width)
            cwidths = widths;
          break;
        default:
          if(e->children[i]->height > cheights)
            cheights = e->children[i]->height;
          if(e->children[i]->width > cwidths)
            cwidths = e->children[i]->width;
          break;
      }
    }
  }

  e->bounds.width = e->width + owidth+cwidths;
  e->bounds.height = e->height+ oheight+cheights;

  UIAlignment align = e->align;
  if(e->owner && e->owner->layout != LAYOUT_FREE){
    xinc = e->owner->bounds.x;
    yinc = e->owner->bounds.y;
    centerx = e->owner->bounds.x + e->owner->bounds.width/2;
    centery = e->owner->bounds.y + e->owner->bounds.height/2;
    //align = e->owner->align;
  }

  if(align & ALIGN_CENTER)
    xinc = centerx-e->bounds.width/2;
  if(align & ALIGN_MID)
    yinc = centery- e->bounds.height/2;
  if(align & ALIGN_LEFT)
  e->bounds.x = 0;
  if(align & ALIGN_RIGHT)
    e->bounds.x = e->owner->bounds.x + e->owner->bounds.width - e->bounds.width;
  if(align & ALIGN_TOP)
    DO_NOTHING();
  //xinc += omarginx;
  //yinc += omarginy;

  UILayout layout = e->layout;
  if(e->owner){
    layout = e->owner->layout;
      paddingx = e->owner->spacing[UI_PADDING] + e->owner->spacing[UI_PADDING_LEFT];
    paddingy = (e->owner->spacing[UI_PADDING] + e->owner->spacing[UI_PADDING_BOT]);
  }

  Rectangle prior = RectPos(Vector2XY(xinc/ScreenSized(SIZE_SCALE),yinc),RECT_ZERO);
  if(e->index > 0){
    prior = e->owner->children[e->index-1]->bounds;
    paddingx += e->owner->spacing[UI_PADDING] + e->owner->spacing[UI_PADDING_RIGHT];
    paddingy += e->owner->spacing[UI_PADDING] + e->owner->spacing[UI_PADDING_TOP];

  }

  switch(layout){
    case LAYOUT_VERTICAL:
      xinc += omarginx;
      yinc = omarginy+prior.y + prior.height + paddingy;
      break;
    case LAYOUT_HORIZONTAL:
      if(e->index > 0 )
        xinc = paddingx + prior.x + prior.width;
      else if(e->type == UI_TOOL_TIP)
        xinc += e->owner->width + omarginx;
      else
        xinc += omarginx;
      break;
    case LAYOUT_GRID:
      if(!e->owner)
        break;
      switch(e->index%GRID_WIDTH){
        case 1:
        case 2:
          yinc = omarginy+prior.y + prior.height;
          xinc = prior.x;
          break;  
        case 0:
          //yinc = omarginy+prior.y;//height;
          xinc = prior.x + prior.width+omarginx;
          break;
        default:
          break;
      }
      break;
    default:
      xinc += omarginx + paddingx;
      yinc += omarginy + paddingy;
      if(e->align & ALIGN_RIGHT)
        xinc *= -1;//omarginx;

      break;
  }

  e->bounds = RectInc(e->bounds,xinc,yinc);
  
  for(int i = 0; i < e->num_children; i++)
    ElementResize(e->children[i]);
}

void UISync(FetchRate poll){
  ScreenApplyContext(ui.contexts);
  for(int i = 0; i < MENU_DONE; i++){
    if(IsKeyPressed(ui.menu_key[i]))
      MenuSetState(&ui.menus[i],MENU_OPENED);

    UISyncMenu(&ui.menus[i], poll);
  }
}

void UISyncMenu(ui_menu_t* m, FetchRate poll){
  if(m->state < MENU_ACTIVE)
    return;

  if(!m->element->menu )
    m->element->menu = m; 

  UISyncElement(m->element, poll);

  if(IsKeyPressed(KEY_ESCAPE)){
    if(m->cb[MENU_CLOSE](m))
      MenuSetState(m,MENU_CLOSE);
  }
}

void UISyncElement(ui_element_t* e, FetchRate poll){
  if(!e->menu && e->owner)
    e->menu = e->owner->menu;

  if(e->state < ELEMENT_IDLE)
    return;

  int state = 0;
  int clicked = 0,toggle = 0,focused = 0;
  if(e->sync_val){
    e->sync_val(e,poll);
    if(e->value){
      switch(e->value->type){
        case VAL_CHAR:
          strcpy(e->text, e->value->c);
          break;
        case VAL_INT:
          strcpy(e->text,TextFormat("%i",*e->value->i));
          break;
        case VAL_LN:
          strcpy(e->text, PrintElementValue(e->value, e->spacing));
          break;
        case VAL_ICO:
          if(e->type == UI_ICON && e->value->s == NULL)
            return;
        default:
          break;
      }
    }
  }

  if(e->texture)
    DrawNineSlice(e->texture,e->bounds);

  switch(e->type){
    case UI_BUTTON:
      state = GuiButton(e->bounds,e->text);
      if(state!=0)
        DO_NOTHING();
      break;
    case UI_PANEL:
      state = GuiPanel(e->bounds, e->text);
      break;
    case UI_LABEL:
      state = GuiLabel(e->bounds,e->text);
      break;
    case UI_TEXT:
      state = GuiTextBox(e->bounds, e->text, 8, false);
      break;
    case UI_TOOL_TIP:
      active_tooltip = e;
      GuiSetTooltip(e->text);
      GuiTooltip(e->bounds);
      break;
    case UI_BOX:
      GuiGroupBox(e->bounds,NULL);//e->text);
    case UI_PROGRESSBAR:
      break;
    case UI_ICON:
      state = GuiPanel(e->bounds, "word");
      DrawSpriteAtPos(e->value->s, RectXY(e->bounds));

      break;
    case UI_STATUSBAR:
      GuiStatusBar(e->bounds, e->text);
      break;
    default:
      break;
  }

  switch(state){
    case STATE_FOCUSED:
      ElementSetState(e,ELEMENT_FOCUSED);
      break;
    case STATE_PRESSED:
      ElementSetState(e,ELEMENT_ACTIVATE);
      break;
    default:
      if(e->type != UI_TOOL_TIP)
      ElementSetState(e,ELEMENT_TOGGLE);
      break;
  }
  
  for(int i = 0; i<e->num_children; i++)
    UISyncElement(e->children[i], poll);
}

bool UIFreeElement(ui_element_t* e){
  if(!e)
    return false;

  free(e);

  return true;
}

bool UIHideElement(ui_element_t* e){
  return ElementSetState(e,ELEMENT_HIDDEN);
}

bool UICloseOwner(ui_element_t* e){
  if(!e->menu)
    return false;

  return UICloseMenu(e->menu);
}

bool UICloseMenu(ui_menu_t* m){
  return MenuSetState(m,MENU_CLOSE);
}

bool UIClearElements(ui_menu_t* m){
  ui_element_t* eo = m->element;
  int num_children = eo->num_children;
  for (int i = 0; i < num_children; i++){
    if(UIFreeElement(eo->children[i]))
      eo->num_children--;

  }
}

bool UIGetPlayerAttributeName(ui_element_t* e){

  strcpy(e->text,attributes[e->index+1].name);

  return true;
}

bool UISelectOption(ui_element_t* e){
}

bool UITransitionScreen(ui_element_t* e){
  GameTransitionScreen();
  return true; 
}

bool ElementSetTooltip(ui_element_t* e){
  GuiEnableTooltip();
}

bool ElementTabToggle(ui_element_t* e){
  ui_element_t* p = e->ctx;

  //return ElementSetState(p, ELEMENT_ACTIVATE);

  switch(p->state){
    case ELEMENT_HIDDEN:
    case ELEMENT_IDLE:
    case ELEMENT_TOGGLE:
      return ElementSetState(p, ELEMENT_ACTIVATE);
      break;
    default:
      return ElementSetState(p, ELEMENT_HIDDEN);
      break;
  }

}

bool ElementToggleTip(ui_element_t* e){
  return true;
}

bool ElementToggle(ui_element_t* e){
  switch(e->prior){
    case ELEMENT_SHOW:
    case ELEMENT_ACTIVATE:
    case ELEMENT_ACTIVATED:
      TraceLog(LOG_INFO,"===== ELEMENT TOGGLE ====\n hide %s",e->name);
      return ElementSetState(e, ELEMENT_HIDDEN);
      break;
    default:
      TraceLog(LOG_INFO,"===== ELEMENT TOGGLE ====\n show %s",e->name);
      return ElementSetState(e, ELEMENT_ACTIVATE);
      break;
  }
}

bool ElementHideSiblings(ui_element_t* e){
  for(int i = 0; i < e->owner->num_children; i++){
    ui_element_t* c = e->owner->children[i];
    if(c->hash == e->hash)
      continue;
    if(ElementSetState(c, ELEMENT_HIDDEN))
      TraceLog(LOG_INFO,"===== ELEMENT HIDE SIBLINGS ====\n HIDE %s",e->owner->children[i]->name);


  }
}

bool ElementToggleChildren(ui_element_t* e){
  for(int i = 0; i < e->num_children; i++)
    ElementSetState(e->children[i], ELEMENT_TOGGLE);

  return true;
}

bool ElementShowIcon(ui_element_t* e){
  if(e->value->s == NULL)
    return false;

  return e->value->s->is_visible = true;
}

bool ElementShowTooltip(ui_element_t* e){
  for(int i = 0; i < e->num_children; i++){
    ui_element_t* c = e->children[i];
    if(c->type != UI_TOOL_TIP)
      continue;

    TraceLog(LOG_INFO, "TOOLTIP TOGGLE ON");
    if(ElementSetState(c, ELEMENT_SHOW))
      ElementSetState(c, ELEMENT_FOCUSED);
  }
}

bool ElementSyncOwnerContext(ui_element_t* e){
  e->ctx = e->owner->ctx;
  e->set_val(e, e->ctx);
}

bool ElementLoadChildren(ui_element_t* e){
  for (int i = 0; i < e->num_children; i++)
    ElementSetState(e->children[i], ELEMENT_LOAD);

  ElementResize(e);
}

bool ElementActivateChildren(ui_element_t* e){
  for (int i = 0; i < e->num_children; i++){
    ElementSetState(e->children[i], ELEMENT_LOAD);
    ElementSetState(e->children[i], ELEMENT_IDLE);
  }
  ElementSetState(e, ELEMENT_SHOW);
}

bool ElementShowPrimary(ui_element_t* e){
  for (int i = 0; i < e->num_children; i++){
    if(i == 0){
      if(ElementSetState(e->children[i], ELEMENT_SHOW))
        TraceLog(LOG_INFO,"===== ELEMENT SET PRIMARY ====\n SHOW %s",e->children[i]->name);
    }
    else{
      if(ElementSetState(e->children[i], ELEMENT_HIDDEN))
        TraceLog(LOG_INFO,"===== ELEMENT SET PRIMARY ====\n HIDE %s",e->children[i]->name);
    }
  }
}

bool ElementShowChildren(ui_element_t* e){
  if(e->num_children == 6)
    DO_NOTHING();
  for (int i = 0; i < e->num_children; i++)
    ElementSetState(e->children[i], ELEMENT_SHOW);
}

bool MenuActivateChildren(ui_menu_t* m){
  if(ElementSetState(m->element, ELEMENT_IDLE))
  ElementSetState(m->element, ELEMENT_SHOW);
}

void MenuOnStateChanged(ui_menu_t*m, MenuState old, MenuState s){
  if(m->cb)
    m->cb[m->state](m);

  switch(old){
    case MENU_INACTIVE:
      ElementResize(m->element);
      break;
    default:
      break;
  }

  switch(s){
    case MENU_CLOSE:
      MenuSetState(m,MENU_CLOSED);
      break;
    case MENU_ACTIVE:
    case MENU_READY:
    case MENU_OPENED:
      GuiSetState(STATE_NORMAL);
      break;
    default:
      break;
  }
}

bool MenuCanChangeState(MenuState old, MenuState s){
  if(old == s)
    return false;

  switch(COMBO_KEY(old,s)){
    case COMBO_KEY(MENU_INACTIVE,MENU_OPENED):
      return false;
      break;
    default:
      break;
  }
  return true;
}

bool MenuSetState(ui_menu_t* m, MenuState s){
  if(!MenuCanChangeState(m->state, s))
    return false;

  MenuState old = m->state;
  m->state = s;

  MenuOnStateChanged(m,old,s);

  return true;
}

bool ElementCanChangeState(ElementState old, ElementState s){
  if(old == s)
    return false;

  state_change_requirement_t *req = &ELEM_STATE_REQ[s];
  return req->can(old, req->required);


  return true;
}

void ElementStepState(ui_element_t* e, ElementState s){
  switch(s){
    case ELEMENT_TOGGLE:
    case ELEMENT_ACTIVATE:
      ElementSetState(e,ELEMENT_ACTIVATED);
      break;
    case ELEMENT_NONE:
      ElementSetState(e,ELEMENT_IDLE);
    default:
     break;
  } 
}

bool ElementSetState(ui_element_t* e, ElementState s){
  if(!ElementCanChangeState(e->state, s))
    return false;
 
  e->prior = e->state; 
  e->state = s;
  if(e->cb[s] && e->cb[s](e))
    ElementStepState(e,s);

  return true;
}

void ElementSyncVal(ui_element_t* e, FetchRate poll){
  element_value_t* ev = e->value;

  if(!e->ctx || !ev || poll != ev->rate)
    return;

  switch(ev->type){
    case VAL_LN:
      for( int i = 0; i < ev->num_ln; i++)
        PrintSyncLine(ev->l[i], poll);
      break;
    default:
      e->value = e->set_val(e, e->ctx);
      break;
  }
}


bool ElementSyncContext(ui_element_t* e){
  if(e->get_ctx){
    e->ctx = e->get_ctx(e);

    for (int i = 0; i < e->num_children; i++)
      ElementSetState(e->children[i], ELEMENT_HIDDEN);

    return ElementSetState(e, ELEMENT_SHOW);
  }

  return false;
}

bool ElementShow(ui_element_t* e){
  return ElementSetState(e, ELEMENT_SHOW);
}

bool ElementShowContext(ui_element_t* e){
  if(e->value){
    //e->value = e->set_val(e, e->ctx);
    e->sync_val = ElementSyncVal;

    ElementSetState(e, ELEMENT_SHOW);
  }

  return (e->ctx == NULL);

}

bool ElementSetContext(ui_element_t* e){
  if(e->get_ctx){
    e->ctx = e->get_ctx(e);

    if(e->set_val){
      e->value = e->set_val(e, e->ctx);
      e->sync_val = ElementSyncVal;
    }

  }
  return (e->ctx == NULL);

}

element_value_t GetDisplayHealth(ui_element_t* e){
  element_value_t ev = {0}; 
  ev.type = VAL_FLOAT;
  ev.f = malloc(sizeof(float));
  if(player->stats[STAT_HEALTH]->ratio ==NULL)
    *ev.f = 0.0f;
  else
    *ev.f = RATIO(player->stats[STAT_HEALTH]);
  return ev;
}

void ElementValueSyncSize(ui_element_t *e, element_value_t* ev){
  bool resize = false;
  ev->text_hei = 0;
  for(int i = 0; i < ev->num_ln; i++){
    ev->text_hei += ev->l[i]->r_hei;
    if(ev->l[i]->r_len > ev->text_len){
      ev->text_len = ev->l[i]->r_len;
      if(e->width >= ev->l[i]->r_wid)
        continue;

      e->width = ev->l[i]->r_wid;
      resize = true;
    }
  }

  if(ev->text_hei > e->height){
    e->height = ev->text_hei;
    resize = true;
  }

  if(resize && e->owner)
    ElementResize(e->owner);

}


element_value_t* GetContextStat(ui_element_t* e, void* context){
  if(context == NULL)
    return NULL;
  
  local_ctx_t* ctx = context;
  element_value_t *ev = calloc(1,sizeof(element_value_t));
  ev->rate = FETCH_TURN;

  for(int i = 0; i < MAX_LINE_ITEMS; i++){
    ev->l[i] = calloc(1,sizeof(line_item_t));
    ev->l[i]->des_len = e->owner->width;
  }

  ev->type = VAL_LN;

  char fmt[PARAM_ALL][MAX_NAME_LEN];
  memset(fmt, 0, sizeof(fmt));

  for(int i = 0; i < e->num_params; i++)
    strcpy(fmt[e->params[i]], "%S:%V");

  ev->num_ln = SetCtxParams(ctx, ev->l, fmt, e->spacing, false);

  ElementValueSyncSize(e, ev);

  return ev;

}

element_value_t* GetContextDetails(ui_element_t* e, void* context){
  if(context == NULL)
    return NULL;
    
  local_ctx_t* ctx = context;
  element_value_t *ev = calloc(1,sizeof(element_value_t));
  ev->rate = FETCH_ACTIVE;
    
  for(int i = 0; i < MAX_LINE_ITEMS; i++){
    ev->l[i] = calloc(1,sizeof(line_item_t));
    ev->l[i]->des_len = e->owner->width;
  }

  ev->type = VAL_LN;

  char fmt[PARAM_ALL][MAX_NAME_LEN];
  memset(fmt, 0, sizeof(fmt));

  for(int i = 0; i < e->num_params; i++)
    strcpy(fmt[e->params[i]], "%s");

  ev->num_ln = SetCtxDetails(ctx, ev->l, fmt, e->spacing, false);

  ElementValueSyncSize(e, ev);

  return ev;

}

bool ElementSetActiveTab(ui_element_t* e){
  for(int i = 0; i < e->num_children; i++){
    if(!ElementSetState(e->children[i], ELEMENT_LOAD))
      continue;

    if(!ElementSetState(e->children[i], ELEMENT_IDLE))
      continue;

    if(e->children[i]->type != UI_TAB_PANEL)
      continue;

    if(!ElementSetState(e->children[i], ELEMENT_SHOW))
      continue;

    ElementShowChildren(e->children[i]);
  }

  return true;
}

element_value_t* GetElementName(ui_element_t* e, void* context){
  ui_element_t* other = context;

  strcpy(e->text, other->name);

  return NULL;
  
}

element_value_t* GetContextName(ui_element_t* e, void* context){
  if(context == NULL)
    return NULL;
  
  local_ctx_t* ctx = context;
  element_value_t *ev = calloc(1,sizeof(element_value_t));
  ev->rate = FETCH_TURN;

  for(int i = 0; i < MAX_LINE_ITEMS; i++){
    ev->l[i] = calloc(1,sizeof(line_item_t));
    ev->l[i]->des_len = e->owner->width;
  }

  ev->type = VAL_LN;

  char fmt[PARAM_ALL][MAX_NAME_LEN];
  memset(fmt, 0, sizeof(fmt));

  for(int i = 0; i < e->num_params; i++)
    strcpy(fmt[e->params[i]], "%S");

  ev->num_ln = SetCtxParams(ctx, ev->l, fmt, e->spacing, false);

  ElementValueSyncSize(e, ev);

  return ev;
}

element_value_t* GetActivityEntry(ui_element_t* e, void* context){
  element_value_t *ev = calloc(1,sizeof(element_value_t));
  ev->rate = FETCH_EVENT;

  ev->context = e->ctx;
  ev->type = VAL_LN;

  ev->num_ln = SetActivityLines(ev, e->spacing);

  ElementValueSyncSize(e, ev);

  return ev;
}

element_value_t* GetContextIcon(ui_element_t* e, void* context){
  element_value_t *ev = calloc(1,sizeof(element_value_t));
  ev->rate = FETCH_ONCE;

  ev->type = VAL_ICO;

  ev->s = SetCtxIcons(e->ctx, e->params);
  return ev;
}

void UILogEvent(EventType event, void* data, void* user){
  activity_t* act = data;
  ui_element_t* e = user;
  if(ElementSetState(e, ELEMENT_IDLE))
    UISyncElement(e, FETCH_EVENT);
  
}

bool ElementActivityContext(ui_element_t* e){
  WorldTargetSubscribe(EVENT_COMBAT_ACTIVITY, UILogEvent, e, e->index);

  e->ctx = &e->index;
  e->value = e->set_val(e, e->ctx);
  return true;
}

bool ElementScreenContext(ui_element_t* e){
  WorldSubscribe(SCREEN_EVENT_SELECT, UIEventActivate, e);

  return ui.contexts[SCREEN_CTX_HOVER];
}

void* ElementPresetContext(void* p){
  ui_element_t* e = p; 

  return e->ctx;
}

void* ElementMatchTab(void* p){
  ui_element_t* e = p; 
}

void* ElementOwnerItemContext(void* p){
  ui_element_t* e = p; 
  local_ctx_t* ctx = e->owner->ctx;
}

void* ElementNiblings(void *p){
  ui_element_t* e = p; 
  ui_element_t* sib = NULL;
  for(int i = 0; i < e->owner->num_children; i++){
    if(e->owner->children[i]->hash == e->hash)
      continue;

    sib = e->owner->children[i];
    break; 
  }
  if(!sib)
    return NULL;

  e->ctx = sib->children;

  int count = imin(e->num_children, sib->num_children);
  for(int i = 0; i < count; i++)
    e->children[i]->ctx = sib->children[i];

  ElementSetState(e, ELEMENT_IDLE);
}

void* ElementOwnerChildren(void* p){
  ui_element_t* e = p;
  ui_element_t* others[e->owner->num_children - 1];

  int count = 0;
  for(int i = 0; i < e->owner->num_children; i++){
    if(e->owner->children[i]->hash == e->hash)
      continue;

    others[count++] = e->owner->children[i];
  }

  e->ctx = others;
  for (int i = 0; i < e->num_children; i++)
    e->children[i]->ctx = others[i];
}

void* ElementGetOwnerContext(void* p){
  ui_element_t* c = p; 

  if(!c->owner)
    return NULL;

  if(c->type == UI_TOOL_TIP){
    c->num_params = c->owner->num_params;
    memcpy(c->params, c->owner->params, sizeof(c->owner->params));
  }
  //TraceLog(LOG_INFO, "======= GET OWNER CONTEXT ====\n %s ctx set to %s ctx", c->name, c->owner->name);

  return c->owner->ctx;
}

void* ElementIndexContext(void* p){
   ui_element_t* e = p;

   return &e->index;

}

void* ElementGetScreenSelection(void* p){
   ui_element_t* e = p;

  return ui.contexts[SCREEN_CTX_HOVER];

}
