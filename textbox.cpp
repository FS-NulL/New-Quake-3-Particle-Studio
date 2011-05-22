#include "textbox.h"
#include "textdraw.h"
//#include "label.h"

#include "fstream"

#include <windows.h>
#include <gl/gl.h>
#include <cstring>

textBox::textBox()
{
  
  location.x = 100;
  location.y = 100;
  size.x = 175;
  size.y = 12;
  
  active = false;
  hoverActive = false;
  
  bgColor.R = 0.5f;
  bgColor.G = 0.5f;
  bgColor.B = 0.5f;
  
  bgColor_active.R = 0.5f;
  bgColor_active.G = 0.5f;
  bgColor_active.B = 0.9f;
  
  bgColor_hoverActive.R = 0.5f;
  bgColor_hoverActive.G = 0.5f;
  bgColor_hoverActive.B = 0.7f;
  
  forceColor.R = 0.3f;
  forceColor.G = 0.6f;
  forceColor.B = 0.3f;
  
  useForceColor = false;
  
  onChange = 0;
  for (int i=0;i<512;i++) string[i] = 0;
  
  l_name.location.x = location.x;
  l_name.location.y = location.y;
  l_name.txtSize = size.y;
  l_name.alignment = ALIGN_RIGHT;
  useLabelName = false;
  
  shiftDown = false;
}

textBox::~textBox()
{
}

int textBox::forceUnActive()
{
  active= false;
  hoverActive = false;
  return 0;
}

int textBox::draw()
{
  glBegin(GL_QUADS);
    //BackGround
    glColor3f( 
    useForceColor ? forceColor.R : active ? bgColor_active.R : hoverActive ? bgColor_hoverActive.R :  bgColor.R, 
    useForceColor ? forceColor.G : active ? bgColor_active.G : hoverActive ? bgColor_hoverActive.G :  bgColor.G, 
    useForceColor ? forceColor.B : active ? bgColor_active.B : hoverActive ? bgColor_hoverActive.B :  bgColor.B );
        
    glVertex2f(location.x,location.y);
    glVertex2f(location.x + size.x,location.y);
    glVertex2f(location.x + size.x,location.y + size.y);
    glVertex2f(location.x,location.y + size.y);
  glEnd();
  
  print(string,location.x,location.y,size.y,size.x,ALIGN_LEFT);
  
  if (useLabelName) l_name.draw();
  return 0;
}

int textBox::eventHandler(UINT message,WPARAM key,int mousex,int mousey)
{
  
  
    if (message == WM_LBUTTONDOWN) // Set active
    {
      active = true;
      hoverActive = false;
    }
    if (message == WM_MOUSEMOVE)
    {
      // Hovering?
        
      if (inBounds(mousex,mousey)) hoverActive = true;
      else hoverActive = false; 
        
    }
    if (message == WM_LBUTTONUP)   // Unset Active
    {
        active = false;
        if (inBounds(mousex,mousey)) hoverActive = true;
    }
    if (message == WM_KEYUP)
    {
      if (key==16) 
      {
        shiftDown = false;
      }
      //oFile.close();
    }
    if (message == WM_KEYDOWN)
    {
      // Interesting Stuff happens here
      // add charector to string
      // string[strlen(string)] = char;
      // string[strlen(string)] = 0;
      //std::ofstream oFile("writer.txt",std::ios::app);
      //oFile << "RECIEVED KEY: " << key << '\n';
      //oFile << "SIZEOF KEY: " << sizeof(key) << '\n';
      if ( (key >=32) && (key<=160) )                                       //       ; = 186 (shift:) / =191 \ = 220
      {
        //oFile << "KEY PASSED IF 1 \n" << std::strlen(string) << '\n';
        if (std::strlen(string) < 511)
        {
          string[std::strlen(string)] = key;
          //oFile << std::strlen(string) << '\n';
          string[std::strlen(string)] = 0;
          //oFile << std::strlen(string) << '\n';
          changeOccured();
        }
      }
      if (key == 186)
      {
        string[std::strlen(string)] = ':';
        //oFile << std::strlen(string) << '\n';
        string[std::strlen(string)] = 0;
        //oFile << std::strlen(string) << '\n';
        changeOccured();
      }
      else if (key == 191)
      {
        string[std::strlen(string)] = '/';
        //oFile << std::strlen(string) << '\n';
        string[std::strlen(string)] = 0;
        //oFile << std::strlen(string) << '\n';
        changeOccured();
      }
      else if (key == 220)
      {
        string[std::strlen(string)] = '/';
        //oFile << std::strlen(string) << '\n';
        string[std::strlen(string)] = 0;
        //oFile << std::strlen(string) << '\n';
        changeOccured();
      }
      else if (key == VK_BACK) 
      {
        //delete char
        if (std::strlen(string) > 0) string[std::strlen(string)-1] = 0;
        //oFile << "BACK \n" << std::strlen(string) << '\n';
        changeOccured();
      }
      else if (key==16)
      {
        //oFile << "Shift DOWN\n";
        shiftDown = true;
      }
      else if (key==190)
      {
        string[std::strlen(string)] = '.';
        string[std::strlen(string)] = 0;
        changeOccured();
      }
      else if (key==189)
      {
        if (shiftDown) string[std::strlen(string)] = '_'; 
        else string[std::strlen(string)] = '-';
        string[std::strlen(string)] = 0;
        changeOccured();
      }
      //oFile << std::strlen(string) << ' ' << string << "  " << key << "  " << (char) key << '\n';
      //oFile.close();   
    }
  return 0;
}

bool textBox::changeOccured()
{
  if (onChange) 
  {
    if(onChange(string))
    {
      useForceColor = true;
    }
    else 
    {
      useForceColor = false;
    }
    return true;
  }
  else return false;
}

int textBox::setOnChange(bool (*p)(char*))
{
  onChange = p;
  return 0;
}
