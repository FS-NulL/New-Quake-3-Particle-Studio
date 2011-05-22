#ifndef TABS_H
#define TABS_H

#include <windows.h>

#include "baseEnt.h"
#include "basicStructs.h"
#include "label.h"

#define TAB_CAPACITY 128

enum {
  TAB_COLOR_ACTIVE,
  TAB_COLOR_HOVER,
  TAB_COLOR
};

class tab : public baseEnt 
{
  RGBf color;
  RGBf hoverColor;
  RGBf activeColor;
  char name[20];
  int (*onClick)(tab*);
  bool active;
  bool hoverActive;
 public:
  tab();
  ~tab();
  int bindEnt(baseEnt *ptr);
  int unBindEnt(baseEnt *ptr);
  int eventHandler(UINT message,WPARAM key,int mousex,int mousey);
  int draw(void);
  baseEnt *linked[TAB_CAPACITY];
  int forceUnActive();
  tab* setActive(bool t=true);
  int setOnClick(int (*p)(tab*));
  baseEnt* getLinked(int i);
  void getColor(float &red,float &green, float &blue, int style=TAB_COLOR_ACTIVE);
  void setColor(float red,float green, float blue, int style=TAB_COLOR_ACTIVE);
  label l_name;
  bool useLabelName;
};

#endif
