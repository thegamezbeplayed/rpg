#include "game_strings.h"
#include <stdio.h>

void StringPrependPadding(char* s, size_t padding){
    size_t len = strlen(s);

    if (len >= padding)
        return;

    size_t pad = padding - len;

    /* shift existing text right */
    memmove(s + pad, s, len + 1);   // +1 to move the '\0'

    /* fill the front with spaces */
    for (size_t i = 0; i < pad; i++)
        s[i] = ' ';
}

void StringAppendPadding(char* s, size_t padding){
  size_t len = strlen(s);

  if(len >= padding)
    return;

    
  size_t p_len = LIST_LEFT_HAND_PAD - len;
  s[len] = ' ';
  s[LIST_LEFT_HAND_PAD] = '\0';
  char l_pad[p_len+1];
  RepeatChar(l_pad, p_len+1, ' ', p_len);

  strcat(s, l_pad);
//  s[padding] = '\0';
}

void RepeatChar(char* out, size_t cap, char c, int times)
{
    if ((size_t)times + 1 > cap) return;

    for (int i = 0; i < times; i++)
        out[i] = c;

    out[times] = '\0';
}

ParseToken TokenFromString(const char* s){
  for (int i = 0; i < TOKE_ALL; i++)
    if (strcmp(s, TOKEN_TABLE[i].name) == 0)
      return TOKEN_TABLE[i].token;

  return TOKE_ALL; // invalid / fallback
}

int ReadToken(const char* fmt, int start, char* out, int out_sz)
{
  int k = 0;
  int i = start;

  while (fmt[i] && fmt[i] != '}' && k < out_sz - 1) {
    out[k++] = fmt[i++];
  }

  out[k] = 0;

  return (fmt[i] == '}') ? i : -1;
}

char* ParseDetails(const char *fmt, char* buffer, size_t buf_size, param_t tokens[TOKE_ALL]){
  size_t out = 0;
  int token_index = 0;

   buffer[0] = '\0';

  for (int i = 0; fmt[i] != 0 && out < buf_size - 1; i++)
  {
    if (fmt[i] == '{')
    {
      char tok[32];
      int end = ReadToken(fmt, i + 1, tok, sizeof(tok));

      if (end != -1)
      {
        ParseToken t = TokenFromString(tok);

        param_t* p = &tokens[t];

        char tmp[64];
        const char* str = "";

        switch (p->type_id)
        {
          case DATA_STRING:
            str = ParamReadString(p);
            break;

          case DATA_INT:
            snprintf(tmp, sizeof(tmp), "%i", ParamReadInt(p));
            str = tmp;
            break;

          case DATA_FLOAT:
            snprintf(tmp, sizeof(tmp), "%.1f", ParamReadFloat(p));
            str = tmp;
            break;
        }

        out += snprintf(buffer + out, buf_size - out, "%s", str);

        i = end; // skip past '}'
        continue;
      }
    }

    buffer[out++] = fmt[i];
    buffer[out] = 0;
  }

  return buffer;

}

line_item_t* StringDetailsFormat(const char *fmt, int num_tokens, param_t params[num_tokens]){
  size_t buf = strlen(fmt);
  buf += MAX_NAME_LEN * num_tokens;
  
  element_value_t* val = GameCalloc("StringDetailsFormat", 1,sizeof(element_value_t));

  val->type = VAL_CHAR;
  val->c = GameMalloc("StringDetailsFormat", buf * sizeof(char));
  ParseDetails(fmt, val->c, buf, params);
  

  element_value_t* vals[1];
  vals[0] = val;
  return InitLineItem(vals, 1, "%s");
}


void StringBounds(Rectangle *b, char* buff){
  buff[0] = '\0';

  element_value_t* vals = GameCalloc("StringBounds", 4, sizeof(element_value_t));

  element_value_t* x = &vals[0];

  x->f = GameMalloc("StringBounds", sizeof(float));
  x->type = VAL_FLOAT;
  *x->f = b->x;

  element_value_t* y = &vals[1];

  y->type = VAL_FLOAT;
  y->f = GameMalloc("StringBounds", sizeof(float));
  *y->f = b->y;

  element_value_t* w = &vals[2];

  w->type = VAL_FLOAT;
  w->f = GameMalloc("StringBounds", sizeof(float));
  *w->f = b->width;

  element_value_t* h = &vals[3];

  h->type = VAL_FLOAT;
  h->f = GameMalloc("StringBounds", sizeof(float));
  *h->f = b->height;

  element_value_t* ptrs[4];

  ptrs[0] = &vals[0];
  ptrs[1] = &vals[1];
  ptrs[2] = &vals[2];
  ptrs[3] = &vals[3];

  line_item_t* li = InitLineItem(ptrs, 4, "[%f , %f] [%f , %f]");

  strcpy(buff, TextFormatLineItem(li));
}

void StringUIBounds(ui_bounds_t *b, char* buff){
  
  element_value_t* vals = GameCalloc("StringBounds", 4, sizeof(element_value_t));
  element_value_t* ptrs[4];

  for (int i = 0; i < POINT_ALL; i++){
    element_value_t* val = &vals[i];
    val->type = VAL_INT;
    val->i = GameMalloc("StringBounds", sizeof(int));
    *val->i = b->sum[i];

    ptrs[i] = &vals[i];

  }

  line_item_t* li = InitLineItem(ptrs, 4, "[%i , %i] [%i , %i]");

  strcpy(buff, TextFormatLineItem(li));
}
