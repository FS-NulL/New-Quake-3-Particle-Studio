#include <windows.h>
#include <GL/gl.h>


#include "button.h"


button::button()
{
  active = false;
  hoverActive = false;
  
  location.x = 275; 
  location.y = 75;
  size.x = 12;
  size.y = 12;
  
  bgColor.R = 0.5f;
  bgColor.G = 0.5f;
  bgColor.B = 0.5f;
  
  bgColor_active.R = 0.5f;
  bgColor_active.G = 0.5f;
  bgColor_active.B = 0.9f;
  
  bgColor_hoverActive.R = 0.5f;
  bgColor_hoverActive.G = 0.5f;
  bgColor_hoverActive.B = 0.7f;
  
  useLabelName = false;
  
  l_name.location.x = location.x;
  l_name.location.y = location.y;
  l_name.alignment = ALIGN_RIGHT;
  
  l_name.txtSize = size.y;
  
}

button::~button()
{
}

int button::draw()
{
  glBegin(GL_QUADS);
        //BackGround
        glColor3f( 
        active ? bgColor_active.R : hoverActive ? bgColor_hoverActive.R :  bgColor.R, 
        active ? bgColor_active.G : hoverActive ? bgColor_hoverActive.G :  bgColor.G, 
        active ? bgColor_active.B : hoverActive ? bgColor_hoverActive.B :  bgColor.B );
        
        glVertex2f(location.x,location.y);
        glVertex2f(location.x + size.x,location.y);
        glVertex2f(location.x + size.x,location.y + size.y);
        glVertex2f(location.x,location.y + size.y);
  glEnd();
  
  if (useLabelName) l_name.draw();
  return 0;
}

int button::forceUnActive()
{
  active = false;
  hoverActive = false;
  return 0;
}

int button::setOnClick(int (*p)(void))
{
  onClick = p;
  return 0;
}

int button::eventHandler(UINT message,WPARAM key,int mousex,int mousey)
{
    if (message == WM_LBUTTONDOWN)
    {
        active = true;
        hoverActive = false;
    }
    if (message == WM_MOUSEMOVE)
    {
        if (active) 
        {
          // Move slider position to mouse x if can! calc value
        }
        else  // Hovering? How do we detect left hover?
        {
          if (inBounds(mousex,mousey)) hoverActive = true;
          else hoverActive = false; 
        }
    }
    if (message == WM_LBUTTONUP)   // Unset Active
    {
        active = false;
        if (inBounds(mousex,mousey)) 
        {
          hoverActive = true;
          // Run external code here?  
          if (onClick) onClick();
        }
        
        
        
        //bgColor.B = 0.5f;
    }
	return 0;
}


void button::setColor(float red,float green, float blue, int style)
{

  switch (style)
  {
    case BUTTON_COLOR:
      bgColor.R = red;
      bgColor.G = green;
      bgColor.B = blue;
      return;
      
    case BUTTON_COLOR_ACTIVE:
      bgColor_active.R = red;
      bgColor_active.G = green;
      bgColor_active.B = blue;
      return;
      
    case BUTTON_COLOR_HOVERACTIVE:
      bgColor_hoverActive.R = red;
      bgColor_hoverActive.G = green;
      bgColor_hoverActive.B = blue;
      return;
  }
}
