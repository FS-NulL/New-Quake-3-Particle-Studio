#ifndef LABEL_H
#define LABEL_H

#include "baseEnt.h"
#include "basicstructs.h"
#include "textdraw.h"

class label : public baseEnt
{
  char string[256];
  int (*onClick)(void);
 public:
  bool setOnClick(int (*p)(void));
  void setString(char *s);
  int draw();
  label();
  label(char *s);
  ~label();
  //vec2D location
  int eventHandler(UINT message,WPARAM key,int mousex,int mousey);
  int forceUnActive() { return 0;};
  int alignment;
  float txtSize;
  float maxSize;
};

#endif // LABEL_H
