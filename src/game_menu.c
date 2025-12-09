//#include "game_ui.h"
#include "game_utils.h"
#include "screens.h"
#include "game_tools.h"
#include "game_process.h"
#include "game_ui.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
static dice_roll_t selection = {6,1,RollDie};


void GuiDrawRectangle(Rectangle rec, int borderWidth, Color borderColor, Color baseColor) { }
ui_manager_t ui;
void InitUI(void){
  Font font = LoadFontEx("resources/fonts/kenney-pixel-square.ttf", 64, 0, 0);
  SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);
  GuiSetFont(font);
  ui.font = font;
#if defined(PLATFORM_WEB)
  GuiSetStyle(DEFAULT,TEXT_SIZE,27);
#elif defined(PLATFORM_ANDROID)
  GuiSetStyle(DEFAULT,TEXT_SIZE,58);
#else
  GuiSetStyle(DEFAULT,TEXT_SIZE,20*UI_SCALE);
#endif

  SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);
  GuiSetStyle(DEFAULT,TEXT_ALIGNMENT,TEXT_ALIGN_CENTER);
  GuiSetStyle(LABEL,TEXT_ALIGNMENT,TEXT_ALIGN_LEFT);
  GuiSetStyle(DEFAULT,BORDER_COLOR_NORMAL,ColorToInt(WHITE));
  //GuiSetStyle(STATUSBAR,BASE_COLOR_NORMAL,ColorToInt(Fade(WHITE,0.5f)));

  for (int i = 0; i< MENU_DONE; i++)
    ui.menu_key[i] = KEY_NULL;


  ui.menus[MENU_MAIN] = InitMenu(MENU_MAIN,VECTOR2_ZERO,DEFAULT_MENU_SIZE,ALIGN_CENTER|ALIGN_MID,LAYOUT_VERTICAL,false);

  ui.menus[MENU_HUD] = InitMenu(MENU_HUD,VECTOR2_ZERO,VECTOR2_ZERO,ALIGN_CENTER,LAYOUT_HORIZONTAL,false);

  ui_element_t* playerPanel = InitElement("PLAYER_PANEL", UI_PANEL, VECTOR2_ZERO,VECTOR2_ZERO, ALIGN_TOP, LAYOUT_VERTICAL);

  ui_element_t* playerInfo = InitElement("PLAYER_INFO", UI_PANEL, VECTOR2_ZERO,VECTOR2_ZERO, ALIGN_TOP, LAYOUT_VERTICAL);

  ui_element_t *attrPanel = InitElement("ATTR_PANEL",UI_PANEL,VECTOR2_ZERO, VECTOR2_ZERO,ALIGN_CENTER,LAYOUT_VERTICAL);

  ui_element_t *playBtn = InitElement("PLAY_BTN",UI_BUTTON,VECTOR2_ZERO,DEFAULT_BUTTON_SIZE,ALIGN_CENTER|ALIGN_MID,0); 
  strcpy(playBtn->text, "PLAY");
  playBtn->cb[ELEMENT_ACTIVATE] = UITransitionScreen;
  ElementAddChild(ui.menus[MENU_MAIN].element,playBtn);

  ui_element_t *continueBtn = InitElement("CONTINUE_BTN",UI_BUTTON,VECTOR2_ZERO,DEFAULT_BUTTON_SIZE,ALIGN_CENTER|ALIGN_MID,0); 
  strcpy(continueBtn->text, "CONTINUE");
  continueBtn->cb[ELEMENT_ACTIVATE] = UITransitionScreen;
 
  ui.menus[MENU_OPTIONS] = InitMenu(MENU_OPTIONS, VECTOR2_ZERO,DEFAULT_MENU_SIZE,ALIGN_CENTER,LAYOUT_HORIZONTAL,false);


  ui_element_t *die = InitElement("DIE_ICON",UI_STATUSBAR,VECTOR2_ZERO, SQUARE_PANEL,ALIGN_CENTER,0);

  die->get_val = GetSelectionRoll; 

   ui_element_t *attrPanelB = InitElement("ATTR_PANEL",UI_PANEL,VECTOR2_ZERO, VECTOR2_ZERO,ALIGN_CENTER,LAYOUT_VERTICAL);

  ElementAddChild(ui.menus[MENU_OPTIONS].element,attrPanel);
  ElementAddChild(ui.menus[MENU_OPTIONS].element,attrPanelB);
  ElementAddChild(ui.menus[MENU_OPTIONS].element,die);
  for(int i = 0; i < ATTR_DONE; i++){

    for(int j = 0; j <2; j++){
      ui_element_t *attrBtn = InitElement("ATTR_BUTTON",UI_BUTTON,VECTOR2_ZERO,DEFAULT_BUTTON_WIDE,ALIGN_CENTER|ALIGN_MID,0);

      strcpy(attrBtn->text,attributes[i].name);
      attrBtn->cb[ELEMENT_ACTIVATE] = UISelectOption;
      attrBtn->cb[ELEMENT_ACTIVATED] = UIHideElement;

      ElementAddChild(ui.menus[MENU_OPTIONS].element->children[j],attrBtn);
    }
  }

  ElementAddChild(ui.menus[MENU_OPTIONS].element,continueBtn);
  
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
  uint32_t hash = hash_str(name);

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

  u->hash = hash_str(name);
  u->num_children = 0;
  u->type = type;
  u->state = ELEMENT_IDLE;
  u->get_val = NULL;//CHAR_DO_NOTHING;
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

ui_element_t* InitGameElement(ent_t* e){
  ui_element_t* u = malloc(sizeof(ui_element_t));
  *u = (ui_element_t) {0};

  u->ent = e;
  Vector2 pos = VECTOR2_ZERO;
  Vector2 size = RectSize(RectScale(e->sprite->slice->bounds,ScreenSized(SIZE_SCALE)));
  const char* name = TextFormat("TILE%i%i",e->pos.x,e->pos.y);
  u->hash = hash_str(name);
  u->num_children = 0;
  u->type = UI_GAME;
  u->state = ELEMENT_NONE;
  u->get_val = NULL;//CHAR_DO_NOTHING;
  u->bounds = Rect(pos.x,pos.y,size.x,size.y);
  u->width = size.x;
  u->height = size.y;
  
  for(int i = 0; i < ELEMENT_DONE; i++)
    u->cb[i] = UI_BOOL_DO_NOTHING;

  for (int i = 0; i < UI_POSITIONING; i++)
    u->spacing[i] = 0.0f;

  ui.elements[ui.num_elements++] = u;
  return u;

}

ui_element_t* ElementGetChild(ui_element_t* owner, uint32_t child_id){
  for(int i = 0; i < MAX_ELEMENTS; i++){
    if(ui.elements[i] == NULL)
      return NULL;

    if(ui.elements[i]->hash == child_id)
      return ui.elements[i];
  }

  return NULL;
}


void ElementAddGameElement( ent_t* e){
  ui_element_t *o = ui.menus[MENU_PLAY_AREA].element;

  if(!o->menu)
    o->menu = &ui.menus[MENU_PLAY_AREA];
  ui_element_t* g = InitGameElement(e);
  g->owner = o;
  g->layout = LAYOUT_GRID;
  g->menu = o->menu;
  g->index = o->num_children;
  o->children[o->num_children++] = g;
}

void ElementAddChild(ui_element_t *o, ui_element_t* c){
  Vector2 pos_adjustment = Vector2FromXY(c->bounds.x,c->bounds.y);
  c->index = o->num_children;
  c->menu = o->menu;
  c->owner = o;
  ElementSetState(c,ELEMENT_IDLE);
  o->children[o->num_children++] = c;
}

float ElementGetHeightSum(ui_element_t *e){
  if(e->state < ELEMENT_IDLE)
    return 0;

  float height = e->bounds.height;// + e->spacing[UI_MARGIN] + e->spacing[UI_MARGIN_TOP];

  float cheight = 0;
  if (e->layout == LAYOUT_GRID){
      cheight = e->bounds.height * e->owner->num_children / GRID_HEIGHT;
      return height+cheight;
  }

  for(int i = 0; i < e->num_children; i++)
    if(e->layout == LAYOUT_VERTICAL)
      cheight += ElementGetHeightSum(e->children[i]);
    else
      cheight = (cheight < ElementGetHeightSum(e->children[i]))?ElementGetHeightSum(e->children[i]):cheight;

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

  for(int i = 0; i < e->num_children; i++)
    if(e->layout == LAYOUT_HORIZONTAL)
      cwidth += ElementGetWidthSum(e->children[i]);
    else
      cwidth = (cwidth < ElementGetWidthSum(e->children[i]))?ElementGetWidthSum(e->children[i]):cwidth;

  return width+cwidth;
}


void ElementResize(ui_element_t *e){
  e->bounds.x = e->bounds.y = 0;
  float centerx = VECTOR2_CENTER_SCREEN.x;
  float centery = VECTOR2_CENTER_SCREEN.y;
  float xinc =0,yinc = 0;

  float omarginx = e->spacing[UI_MARGIN] + e->spacing[UI_MARGIN_LEFT];
  float omarginy = e->spacing[UI_MARGIN] + e->spacing[UI_MARGIN_TOP];

  float owidth = omarginx;
  float oheight = omarginy; 
  float cwidths =0, cheights= 0;

  for(int i = 0; i<e->num_children; i++){
    switch(e->layout){
      case LAYOUT_VERTICAL:
        oheight+=ElementGetHeightSum(e->children[i]);
        if(ElementGetWidthSum(e->children[i]) > cwidths)
          cwidths = ElementGetWidthSum(e->children[i]);
        break;
      case LAYOUT_HORIZONTAL:
        owidth += ElementGetWidthSum(e->children[i]);
        if(e->children[i]->height > cheights)
          cheights = e->children[i]->height;
        break;
      case LAYOUT_GRID:
        cwidths = 0;//ElementGetWidthSum(e->children[i]);
        cheights = 0;//ElementGetHeightSum(e->children[i]);
        break;
      default:
        if(e->children[i]->height > cheights)
          cheights = e->children[i]->height;
        if(e->children[i]->width > cwidths)
          cwidths = e->children[i]->width;
        break;
    }
  }

  e->bounds.width = e->width + owidth+cwidths;
  e->bounds.height = e->height+ oheight+cheights;

  UIAlignment align = e->align;
  if(e->owner){
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
 
  
  xinc += omarginx;
  yinc += omarginy;

  UILayout layout = e->layout;
  if(e->owner)
    layout = e->owner->layout;

  Rectangle prior = RectPos(Vector2XY(xinc/ScreenSized(SIZE_SCALE),yinc),RECT_ZERO);
  if(e->index > 0){
    prior = e->owner->children[e->index-1]->bounds;

    omarginx = e->owner->spacing[UI_MARGIN] + e->owner->spacing[UI_MARGIN_LEFT];
    omarginy = e->owner->spacing[UI_MARGIN] + e->owner->spacing[UI_MARGIN_TOP];
  }

  switch(layout){
    case LAYOUT_VERTICAL:
      if(e->index > 0)
        yinc = omarginy+prior.y + prior.height;
      break;
    case LAYOUT_HORIZONTAL:
      if(e->index > 0)
        xinc = omarginx + prior.x + prior.width;
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
      break;
  }

  e->bounds = RectInc(e->bounds,xinc,yinc);
/*
  if(e->type == UI_GAME && e->ent)
    EntSetPos(e->ent,Vector2Add(e->ent->sprite->slice->center,RectXY(e->bounds)));
*/
  for(int i = 0; i < e->num_children; i++)
    ElementResize(e->children[i]);
}

void UISync(void){
  for(int i = 0; i < MENU_DONE; i++){
    if(IsKeyPressed(ui.menu_key[i]))
      MenuSetState(&ui.menus[i],MENU_OPENED);

    UISyncMenu(&ui.menus[i]);
  }
}

void UISyncMenu(ui_menu_t* m){
  if(m->state < MENU_ACTIVE)
    return;

  if(!m->element->menu )
    m->element->menu = m; 

  UISyncElement(m->element);

  if(IsKeyPressed(KEY_ESCAPE)){
    if(m->cb[MENU_CLOSE](m))
      MenuSetState(m,MENU_CLOSE);
  }
}

void UISyncElement(ui_element_t* e){
  if(!e->menu && e->owner)
    e->menu = e->owner->menu;

  if(e->state < ELEMENT_IDLE)
    return;

  int clicked = 0,toggle = 0,focused = 0;
  if(e->get_val){
    if( e->value==NULL ){
      e->value = malloc(sizeof(ElementValue));
      memset(e->value,0,sizeof(ElementValue));
    }

    if( e->value->rate == FETCH_UPDATE || e->value->rate == 0)
      *e->value = e->get_val(e);

    switch(e->value->type){
      case VAL_CHAR:
        strcpy(e->text, e->value->c);
        break;
      case VAL_INT:
        strcpy(e->text,TextFormat("%i",*e->value->i));
        break;
      default:
        break;
    }
  }

    if(e->texture)
    DrawNineSlice(e->texture,e->bounds);

  switch(e->type){
    case UI_BUTTON:
      clicked = GuiButton(e->bounds,e->text);
      break;
    case UI_PANEL:
      GuiPanel(e->bounds,NULL);//e->text);
      break;
    case UI_LABEL:
      GuiLabel(e->bounds,e->text);
      break;
    case UI_BOX:
      GuiGroupBox(e->bounds,NULL);//e->text);
    case UI_PROGRESSBAR:
      GuiProgressBar(e->bounds, NULL,NULL, e->get_val(e).f,0,1);
      break;
    case UI_STATUSBAR:
      GuiStatusBar(e->bounds, e->text);
      break;
    default:
      break;
  }

   if(clicked>0)
    ElementSetState(e,ELEMENT_ACTIVATE);

  for(int i = 0; i<e->num_children; i++)
    UISyncElement(e->children[i]);
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
  ui_element_t* die = ElementGetChild(ui.menus[MENU_OPTIONS].element,hash_str("DIE_ICON"));

  int* selection = die->value->i;

  CATEGORY_STATS[MOB_PLAYER].attr[e->index-1] += *selection;

  *die->value = die->get_val(die);
  strcpy(die->text,TextFormat("%i",*die->value->i));

}

bool UITransitionScreen(ui_element_t* e){
  GameTransitionScreen();
  return true; 
}

void MenuOnStateChanged(ui_menu_t*m, MenuState old, MenuState s){
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

  return true;
}

void ElementStepState(ui_element_t* e, ElementState s){
  switch(s){
    case ELEMENT_TOGGLE:
    case ELEMENT_ACTIVATE:
      ElementSetState(e,ELEMENT_ACTIVATED);
      break;
    default:
     break;
  } 
}

bool ElementSetState(ui_element_t* e, ElementState s){
  if(!ElementCanChangeState(e->state, s))
    return false;
  
  e->state = s;
  if(e->cb[s](e))
    ElementStepState(e,s);

  return true;
}

ElementValue GetDisplayHealth(ui_element_t* e){
  ElementValue ev = {0}; 
  ev.type = VAL_FLOAT;
  ev.f = malloc(sizeof(float));
  if(player->stats[STAT_HEALTH]->ratio ==NULL)
    *ev.f = 0.0f;
  else
    *ev.f = RATIO(player->stats[STAT_HEALTH]);
  return ev;
}


ElementValue GetSelectionRoll(ui_element_t* e){
  ElementValue ev = {0};
  ev.rate = FETCH_ACTIVE;
  ev.type = VAL_INT;
  ev.i = malloc(sizeof(int));
  *ev.i = selection.roll(&selection);

  return ev;
}
