#include "userEntry.h"
#include "label.h"

#include <windows.h>
#include <GL/GL.h>
#include <cmath>
#include <fstream>
#include <cstdlib>


bool ue_txtBoxChanged(char *s)
{
  //char * p=s;
  //std::ofstream oFile("entry_txtchange.txt",std::ios::app);
  //oFile << s << ' ';
  //only allows numbers and '.' and '-'
  if (*s == '-') s++;
  while(*s)
  {
    //oFile << *s <<'(';
    if ((*s != '.' ) && ((*s<48)||(*s>57)) ) *s = 0;
    //oFile << *s << ")";
    s++;
  }
  //oFile << "   ::   " << p << '\n';
  //oFile.close();
  return 0;
}

userEntry::userEntry()
{
  active = false;
  onChange = nullptr;
  isFloat = false;
  ptr = 0;
  
  entryBox.location.x = 200;
  entryBox.location.y = 260;
  entryBox.size.x = 240;
  entryBox.size.y = 25;
  entryBox.l_name.alignment = ALIGN_RIGHT;
  entryBox.l_name.txtSize = (float) entryBox.size.y;
  entryBox.l_name.setString("Value: ");
  entryBox.useLabelName = true;
  entryBox.l_name.location.x = entryBox.location.x;
  entryBox.l_name.location.y = entryBox.location.y;
  entryBox.setOnChange(ue_txtBoxChanged);
  
  l_ok.location.x = 229;
  l_ok.location.y = 232;
  l_ok.txtSize = 20;
  l_ok.alignment = ALIGN_LEFT;
  l_ok.setString(" OK ");
  l_ok.maxSize = 100;
  
  l_cancel.location.x = 332;
  l_cancel.location.y = 232;
  l_cancel.txtSize = 20;
  l_cancel.alignment = ALIGN_LEFT;
  l_cancel.setString(" CANCEL ");
  l_cancel.maxSize = 100;
}

void userEntry::setActive()
{
  active = true;
  animState = 1;
  onChange = nullptr;
}

void userEntry::draw()
{
  // set blendmode
  // draw bg
  // draw textBox
  // draw buttons
  //std::ofstream oFile("entry_draw.txt",std::ios::app);
  //oFile << entryBox.string << '\n';
  
  if ((animState>=1) && (animState <10)) animState++;
  else if ((animState<=-1) && (animState >-10)) animState++;
  
  //calc drawrect
  vec2D nTL;
  vec2D nBR;
  float per = ((float)animState) / 10;
  if (per < 0) per = -per;
  nTL.x = (int) (              (1-per) * initialTL.x);
  nTL.y = (int) ((per * 480) + (1-per) * initialTL.y);
  nBR.x = (int) ((per * 640) + (1-per) * initialBR.x);
  nBR.y = (int) (              (1-per) * initialBR.y);
  
  //oFile << "1: " << (int)animState << "   TL: (" <<nTL.x<<','<<nTL.y<<")   BR: ("<<nBR.x<<','<<nBR.y<<")\n";
  
  glColor4f(0.4f,0.4f,0.4f,0.75f);
  glBegin(GL_QUADS);
    glVertex2f((GLfloat)nTL.x,(GLfloat)nTL.y);
    glVertex2f((GLfloat)nTL.x,(GLfloat)nBR.y);
    glVertex2f((GLfloat)nBR.x,(GLfloat)nBR.y);
    glVertex2f((GLfloat)nBR.x,(GLfloat)nTL.y);
  glEnd();
  
  if (abs(animState) == 10)
  {
    entryBox.draw();
    //draw txtBox  / buttons, buttons can be fake
    glColor4f(0.35f,0.35f,0.5f,0.8f);
    //OK
    glBegin(GL_QUADS);
      glVertex2f(205,255);
      glVertex2f(205,230);
      glVertex2f(315,230);
      glVertex2f(315,255);
    glEnd();
    //CANCEL
    glBegin(GL_QUADS);
      glVertex2f(325,255);
      glVertex2f(325,230);
      glVertex2f(435,230);
      glVertex2f(435,255);
    glEnd();
    
    l_ok.draw();
    l_cancel.draw();
  }
  if (abs(animState) == 1) active = false;
  //oFile.close();
}

int userEntry::eventHandler(UINT message,WPARAM key,int mousex,int mousey)
{
  if (animState == 10) // fully open only
  {
    if (message == WM_LBUTTONDOWN)
    {
      // check if in bounds of buttons
      if (mousey >= 230 && mousey <= 255)
      {
        if (mousex >= 205 && mousex <= 315) 
        {
          updateSlider();
          beginShutDown();
        }
        else if (mousex >= 325 && mousex <= 435) 
        {
          beginShutDown();
        }
      }
    }
    if (message == WM_KEYDOWN)
    {
      if (key == VK_ESCAPE) beginShutDown();
      else if (key == VK_RETURN) 
      {
        updateSlider();
        beginShutDown();
      }
      else
      {
        entryBox.eventHandler(message,key,mousex,mousey);
      }
    } 
  }
  return 0;
}

void userEntry::beginShutDown()
{
  animState = -9; // start close
  memset (entryBox.string,0,512);
}

void userEntry::updateSlider()
{
  int value;
  if (isFloat)
  {
    value = (int) (atof(entryBox.string) * 10000);
  }
  else
  {
    value = atoi(entryBox.string);
  }
  if (ptr) ptr->setValue_nochange(value);
  if (onChange) onChange(value);
  
}
