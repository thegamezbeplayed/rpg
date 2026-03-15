#include "game_ui.h"

float ElementGetWidthSum(ui_element_t *e){
  if(e->state < ELEMENT_IDLE)
    return 0;

  float width = e->bounds.width;// + e->spacing[UI_MARGIN] + e->spacing[UI_MARGIN_LEFT];
/*
  if(e->owner)
  width += e->owner->spacing[UI_PADDING_RIGHT];
*/
  float cwidth = 0;
  if (e->layout == LAYOUT_GRID){
    cwidth = e->bounds.width * e->owner->num_children / UI_GRID_WIDTH;
    return width+cwidth;
  }
    

  for(int i = 0; i < e->num_children; i++){
    if(e->children[i]->align & ALIGN_OVER)
      continue;

  int wid = ElementGetWidthSum(e->children[i]);
    switch(e->layout){
      case LAYOUT_HORIZONTAL:
      case LAYOUT_GRID:
        cwidth += ElementGetWidthSum(e->children[i]);
        break;
      default:
        width = (width<wid)?wid:width;
        break;
    }
  }

  return width+cwidth;
}

float ElementGetHeightSum(ui_element_t *e){
  if(e->state < ELEMENT_LOAD)
    return 0;

  float height = e->bounds.height;
  int padding = 0;
  if(e->owner)
    padding = e->owner->spacing[UI_PADDING_TOP];
  
  if(e->index > 0)
   padding += e->owner->spacing[UI_PADDING_BOT]; 
  float cheight = 0;
  if (e->layout == LAYOUT_GRID){
    //height = e->bounds.height * e->owner->num_children / UI_GRID_HEIGHT;
    return 0;
  }

  for(int i = 0; i < e->num_children; i++){
    if(e->children[i]->align & ALIGN_OVER)
      continue;

    int hei = ElementGetHeightSum(e->children[i]);

    switch(e->layout){
      case LAYOUT_VERTICAL:
      case LAYOUT_GRID:
        height = e->spacing[UI_MARGIN_TOP] + e->spacing[UI_MARGIN_BOT];
        cheight += hei+padding;
        break;
      default:
        height = (height < hei)?hei:height;
        break;
    }
  }
  return height+cheight;
}

void LayoutGrid(ui_element_t *e, ui_layout_t* layout){ 
  layout->base->inc[POINT_X] = e->bounds.x;
  layout->base->inc[POINT_Y] = e->bounds.y;
}

void LayoutHorizontal(ui_element_t *e, ui_layout_t* layout){ 

  layout->base->inc[POINT_X] = e->bounds.x;
  layout->base->inc[POINT_Y] = e->bounds.y;
  layout->base->inc[POINT_W] = e->width;
  layout->base->inc[POINT_H] = 0;
}

void LayoutVertical(ui_element_t *e, ui_layout_t* layout){ 


  layout->base->inc[POINT_X] = e->bounds.x;
  layout->base->inc[POINT_Y] = e->bounds.y;
  layout->base->inc[POINT_H] = e->height;
  layout->base->inc[POINT_W] = 0;

  if (!e->owner)
    return;


}

ui_layout_t* LayoutElement(ui_element_t *e, ui_bounds_t *root){
  ui_layout_t* lay = GameCalloc("LayoutElement", 1, sizeof(ui_layout_t));
  lay->base = GameCalloc("LayoutElement", 1, sizeof(ui_bounds_t));
  lay->bounds = GameCalloc("LayoutElement", e->num_children, sizeof(ui_bounds_t));

  int o_spacing[UI_POSITIONING] = {0};

  if(e->owner)
    for (int i = 0; i < UI_POSITIONING; i++)
    o_spacing[i] = e->owner->spacing[i];

  if(!root){
    root = GameCalloc("LayoutElement", 1, sizeof(ui_bounds_t));
    for (int i = 0; i < POINT_ALL; i++)
      root->points[i] = 0;
  }

  lay->base->points[POINT_X] = e->bounds.x;
  lay->base->points[POINT_Y] = e->bounds.y;
  lay->base->points[POINT_W] = e->bounds.width;
  lay->base->points[POINT_H] = e->bounds.height;

  ui_bounds_t* prior = root;
     if(e->num_children > 3)
       DO_NOTHING();

    for(int i = 0; i < e->num_children; i++){
     ui_layout_t* c_lay = LayoutElement(e->children[i], lay->base);
     if(!c_lay)
       continue;

     c_lay->base->points[POINT_X] += prior->points[POINT_X] + prior->inc[POINT_W];
     c_lay->base->points[POINT_Y] += prior->points[POINT_Y] + prior->inc[POINT_H];
     ui_bounds_t* c_bounds = &lay->bounds[lay->num_children++];

     e->children[i]->debug = c_lay->base;

     memcpy(c_bounds, c_lay->base, sizeof(ui_bounds_t));
     prior = c_lay->base;
   }

  lay->layout = e->layout;
  switch(e->layout){
    case LAYOUT_VERTICAL:
      lay->base->inc[POINT_H] = e->height;
      if(e->num_children == 0)
        lay->base->inc[POINT_W] = e->width;
      break;
    case LAYOUT_HORIZONTAL:
      lay->base->inc[POINT_W] = e->width;
      if(e->num_children == 0)
        lay->base->inc[POINT_H] = e->height;
      break;
    case LAYOUT_GRID:
      break;
    default:
      break;
  }

  if(e->align & ALIGN_LEFT)
    lay->base->points[POINT_X] = o_spacing[UI_PADDING_LEFT] + e->spacing[UI_MARGIN_LEFT];
  lay->base->sum[POINT_X] =  lay->base->inc[POINT_X]; 
  lay->base->sum[POINT_Y] =  lay->base->inc[POINT_Y]; 
  lay->base->sum[POINT_W] =  lay->base->inc[POINT_W]; 
  lay->base->sum[POINT_H] =  lay->base->inc[POINT_H]; 

  for(int i = 0; i < e->num_children; i++){
    int c_wid = lay->bounds[i].sum[POINT_W];
    int c_hei = lay->bounds[i].sum[POINT_H];

    switch(e->layout){
    case LAYOUT_VERTICAL:
      lay->base->sum[POINT_H] += lay->bounds[i].sum[POINT_H];
      if(c_wid > lay->base->sum[POINT_W])
        lay->base->sum[POINT_W] = c_wid;
      break;
    case LAYOUT_HORIZONTAL:
      lay->base->sum[POINT_W] += lay->bounds[i].sum[POINT_W];
      if(c_hei > lay->base->sum[POINT_H])
        lay->base->sum[POINT_H] = c_hei;
      break;
    case LAYOUT_STACK:
      if(c_hei > lay->base->sum[POINT_H])
        lay->base->sum[POINT_H] = c_hei;
     if(c_wid > lay->base->sum[POINT_W])
        lay->base->sum[POINT_W] = c_wid;

      break;
    }
  }

  return lay;
}
