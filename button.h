#ifndef BUTTON_H
#define BUTTON_H

#include "baseEnt.h"
#include "label.h"

#include <functional>

enum {
  BUTTON_COLOR,
  BUTTON_COLOR_ACTIVE,
  BUTTON_COLOR_HOVERACTIVE
};

class button : public baseEnt
{
  bool active;
  bool hoverActive;
  RGBf bgColor;
  RGBf bgColor_active;
  RGBf bgColor_hoverActive;
  //void (*onClick)(void);
  std::function<void()> onClick;
 public:
  int eventHandler(UINT message,WPARAM key,int mousex,int mousey);
  int setOnClick(std::function<void()> p);
  void setColor(float red,float green, float blue, int style=BUTTON_COLOR);
   button();
  ~button();
  int draw();
  bool renderable;
  int forceUnActive();
  bool useLabelName;
  label l_name;
};

#endif
