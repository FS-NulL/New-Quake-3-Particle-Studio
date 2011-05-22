#ifndef TEXTDRAW_H
#define TEXTDRAW_H

#include "texture.h"

// NEED char to coord function

const float txtSize = 10.0f;
const float charWidth = 0.65f;

enum {
  ALIGN_RIGHT,
  ALIGN_LEFT
};


bool char2Coord(char c, float &x,float &y,int alignment);
bool print(char *str, float xloc, float yloc,int alignment);
bool print(char *str, float xloc, float yloc, float size,int alignment);
bool print(char *str, float xloc, float yloc, float size, float maxSize,int alignment);


#endif // TEXTDRAW_H
