#ifndef BASEENT_H
#define BASEENT_H

#include <windows.h>
#include "basicstructs.h"

class baseEnt 
{

 public:
  vec2D location;
  vec2D size;
  virtual int eventHandler(UINT message,WPARAM key,int mousex,int mousey)=0;
  virtual int draw(void)=0;
  bool inBounds(int mousex,int mousey);
  virtual int forceUnActive()=0;
  virtual int entType() {return 0;}
};



#endif // BASEENT_H
