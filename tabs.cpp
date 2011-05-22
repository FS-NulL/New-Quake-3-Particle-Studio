
#include <windows.h>
#include <gl/gl.h>

#include "tabs.h"

tab::tab()
{
  // Initialise Linked Entity Array
  for (int i =0;i<TAB_CAPACITY;i++) linked[i] = 0;
  
  onClick=0;
  
  color.R = 0.2;
  color.G = 0.3;
  color.B = 0.6;
  activeColor.R = 0.3;
  activeColor.G = 0.4;
  activeColor.B = 0.7;
  useLabelName = true;
  l_name.alignment = ALIGN_LEFT;
  l_name.location.x = location.x + 10;
  l_name.location.y = location.y;
  l_name.txtSize = size.y;
  l_name.maxSize = size.x - 20;
  active = false;
}

tab::~tab()
{
  active = false;
}

int tab::bindEnt(baseEnt *ptr)
{
  int loc=0;
  while (linked[loc]) {
    loc++;
    if (loc>=TAB_CAPACITY) return -1;
  }
  linked[loc] = ptr;
  return loc;
}

int tab::unBindEnt(baseEnt *ptr)
{
  int loc = 0;
  while (linked[loc] != ptr)
  {
    loc++;
    if (loc = TAB_CAPACITY) return -1; //not found
  }
  linked[loc] = 0;
  return loc;
}

int tab::draw()
{
  // Note change shape to indicate active tab <_N_> \_A_/
  glBegin(GL_POLYGON);
    if (active)
    {
      glColor3f(activeColor.R,activeColor.G,activeColor.B);
      glVertex2f( location.x,location.y + size.y );
      glVertex2f( location.x+10,location.y );
      glVertex2f( location.x+size.x-10,location.y );
      glVertex2f( location.x+size.x,location.y + size.y );
    }
    else
    {
      glColor3f(color.R,color.G,color.B);
      glVertex2f( location.x+10,location.y+size.y );
      glVertex2f( location.x+5,location.y + size.y/2 );
      glVertex2f( location.x+10,location.y );
      glVertex2f( location.x+size.x-10,location.y );
      glVertex2f( location.x+size.x-5,location.y + size.y/2 );
      glVertex2f( location.x+size.x-10,location.y + size.y );
      
    }
  glEnd();
  l_name.draw();
  return 0;
}

int tab::eventHandler(UINT message,WPARAM key,int mousex,int mousey)
{
  // If mouse is in bounds and action = LMB down run external code
  if (inBounds(mousex,mousey))
  {
    if (message == WM_LBUTTONDOWN) if (onClick) onClick(this);
    if (message == WM_MOUSEMOVE) hoverActive = true;
  }
  else hoverActive = false;
  return 0;
}

int tab::forceUnActive()
{
// Do nothing as active doesnt need to be un locked
  return 0;
}

tab* tab::setActive(bool t)
{
  active = t;
  return this;
}

int tab::setOnClick(int (*p)(tab*))
{
  onClick = p;
  return 0;
}

baseEnt* tab::getLinked(int i)
{
  return linked[i];
}

void tab::getColor(float &red,float &green, float &blue,int style)
{
  switch (style)
  {
    case TAB_COLOR:
      red = color.R;
      green = color.G;
      blue = color.B;
      return;
    
    case TAB_COLOR_ACTIVE:
      red = activeColor.R;
      green = activeColor.G;
      blue = activeColor.B;
    return;
    
    case TAB_COLOR_HOVER:
    red   = hoverColor.R;
    green = hoverColor.G;
    blue  = hoverColor.B;
    return;
  }
}

void tab::setColor(float red,float green, float blue, int style)
{
  switch (style)
  {
    case TAB_COLOR:
      color.R = red;
      color.G = green;
      color.B = blue;
      return;
      
    case TAB_COLOR_ACTIVE:
      activeColor.R = red;
      activeColor.G = green;
      activeColor.B = blue;
      return;
      
    case TAB_COLOR_HOVER:
      hoverColor.R = red;
      hoverColor.G = green;
      hoverColor.B = blue;
      return;
  }
}
