#include "textdraw.h"

#include <windows.h>
#include <gl/gl.h>
#include <fstream>
#include <cstring>


bool char2Coord(char c, float &x,float &y)
{
  if (c<32) return false;
  c -= 32;
  x = ((float) (c % 16)) / 16;
  y = ((float) ( c / (char) 16) )/16; 
  y = 1 - y;
  return true;
  
  // REMEMBER WHICH WAY TO TAKE COORDS
  
  // OTHER CORNERS x = x + 0.0625
  //               y = y - 0.0625
  
}


bool print(char *str, float xloc, float yloc,int alignment)
{
  // Default Size = DEFAULT_TEXT_SIZE
  float cx,cy;
  int count = 0;
  glBlendFunc(GL_ONE,GL_ONE);
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  if (alignment == ALIGN_RIGHT)
  {
    // Calculate new xloc
    // string px length = txtSize * charWidth * strlen(str)
    float len =  txtSize * charWidth * strlen(str);
    xloc -= (len  + 0.5*txtSize);
  }
  while (*str)
  {
    
    if (char2Coord(*str,cx,cy))
    {
      glBegin(GL_QUADS);
        glColor4f(1.0f,1.0f,1.0f,1.0f);            
        glTexCoord2f(cx,        cy       ); glVertex3f( xloc + (txtSize*charWidth) * count          , yloc + txtSize ,0.0f);  // TOP LEFT
        glTexCoord2f(cx+0.0625, cy       ); glVertex3f( xloc + (txtSize*charWidth) * count + txtSize, yloc + txtSize ,0.0f);  // TOP RIGHT
        glTexCoord2f(cx+0.0625, cy-0.0625); glVertex3f( xloc + (txtSize*charWidth) * count + txtSize, yloc ,0.0f);  // LOWER RIGHT
        glTexCoord2f(cx,        cy-0.0625); glVertex3f( xloc + (txtSize*charWidth) * count          , yloc ,0.0f);  // LOWER LEFT
      glEnd();
      count++;
    }
    str++;
  }
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  return 0;
}

bool print(char *str, float xloc, float yloc, float size,int alignment)
{
  // Default Size = DEFAULT_TEXT_SIZE
  float cx,cy;
  float nTxtSize = size;
  int count = 0;
  glBlendFunc(GL_ONE,GL_ONE);
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  if (alignment == ALIGN_RIGHT)
  {
    // Calculate new xloc
    // string px length = txtSize * charWidth * strlen(str)
    float len =  nTxtSize * charWidth * strlen(str);
    xloc -= (len  + 0.5*nTxtSize);
  }
  while (*str)
  {
    
    if (char2Coord(*str,cx,cy))
    {
      glBegin(GL_QUADS);
        glColor4f(1.0f,1.0f,1.0f,1.0f);            
        glTexCoord2f(cx,        cy       ); glVertex3f( xloc + (nTxtSize*charWidth) * count          , yloc + nTxtSize ,0.0f);  // TOP LEFT
        glTexCoord2f(cx+0.0625, cy       ); glVertex3f( xloc + (nTxtSize*charWidth) * count + nTxtSize, yloc + nTxtSize ,0.0f);  // TOP RIGHT
        glTexCoord2f(cx+0.0625, cy-0.0625); glVertex3f( xloc + (nTxtSize*charWidth) * count + nTxtSize, yloc ,0.0f);  // LOWER RIGHT
        glTexCoord2f(cx,        cy-0.0625); glVertex3f( xloc + (nTxtSize*charWidth) * count          , yloc ,0.0f);  // LOWER LEFT
      glEnd();
      count++;
    }
    str++;
  }
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  return 0;
}

bool print(char *str, float xloc, float yloc, float size, float maxSize,int alignment)
{
  //std::ofstream oFile("print_debug_15.txt",std::ios::app);
  //oFile << "ENTERED PRINT, VALUES: " << str << ' ' << xloc << ' ' << yloc << ' ' << size << ' ' << maxSize << '\n';
  float nTxtSize;
  nTxtSize = txtSize;
  if (size > 0) nTxtSize = size; 
  
  float nTxtWidth = nTxtSize;
  
  maxSize -= 0.75*nTxtWidth*charWidth;
  
  // (txtSize*charWidth) * count + txtSize = maxSize
  
  float pxSize = nTxtSize*charWidth * std::strlen(str);
  if (pxSize > maxSize)
  {
    nTxtWidth = ((maxSize) / (float) std::strlen(str)) / charWidth;
  }
  
  float cx,cy;
  int count = 0;
  glBlendFunc(GL_ONE,GL_ONE);
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  
  if (alignment == ALIGN_RIGHT)
  {
    // Calculate new xloc
    // string px length = txtSize * charWidth * strlen(str)
    float len =  nTxtWidth * charWidth * strlen(str);
    xloc -= (len + 0.5*nTxtWidth);
  }
  
  while (*str)
  {
    
    if (char2Coord(*str,cx,cy))
    {
      glBegin(GL_QUADS);
        glColor4f(1.0f,1.0f,1.0f,1.0f);            
        glTexCoord2f(cx,        cy       ); glVertex3f( xloc + (nTxtWidth*charWidth) * count          , yloc + nTxtSize ,0.0f);  // TOP LEFT
        glTexCoord2f(cx+0.0625, cy       ); glVertex3f( xloc + (nTxtWidth*charWidth) * count + nTxtWidth, yloc + nTxtSize ,0.0f);  // TOP RIGHT
        glTexCoord2f(cx+0.0625, cy-0.0625); glVertex3f( xloc + (nTxtWidth*charWidth) * count + nTxtWidth, yloc ,0.0f);  // LOWER RIGHT
        glTexCoord2f(cx,        cy-0.0625); glVertex3f( xloc + (nTxtWidth*charWidth) * count          , yloc ,0.0f);  // LOWER LEFT
      glEnd();
      count++;
    }
    str++;
  }
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  
  //oFile.close();
  return 0;
}
