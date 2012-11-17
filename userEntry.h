#include <windows.h>
#include <functional>
#include "basicStructs.h"
#include "textbox.h"
#include "label.h"
#include "horizSlider.h"

class userEntry 
{
  void beginShutDown();  
  //bool txtBoxChanged(char *s);
  label l_ok;
  label l_cancel;
  void updateSlider();
 public:
  horizSlider* ptr;
  textBox entryBox;
  bool isFloat;
  userEntry();  
  bool active;
  void draw();
  void setActive();
  signed char animState; // 1-10 = opening, -1--10 = closing
  int eventHandler(UINT message,WPARAM key,int mousex,int mousey);
  vec2D initialTL; // x/y coords of the bounds of the slider entity
  vec2D initialBR; // only need topleft / bottom right, its a RECT!
  std::function<void(int)> onChange;
  //void (*onChange)(int);
};


// problems, when do we update the animation? -- in draw?
