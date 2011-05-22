#ifndef SIDEPANNEL_H
#define SIDEPANNEL_H

#include "baseEnt.h"
#include "label.h"

class sidePannel : public baseEnt
{
  bool hoverActive;
  RGBf bgColor;
  int (*onClick)(void);
 public:
  bool active;
  bool visible;
  int eventHandler(UINT message,WPARAM key,int mousex,int mousey);
  int setOnClick(int (*p)(void));
  void setColor(float red,float green, float blue);
   sidePannel();
  ~sidePannel();
  int draw();
  int forceUnActive();
  int bindEnt(baseEnt *p);
  bool useLabelName;
  label l_name;
    baseEnt *boundEnts[128];
};

#endif
