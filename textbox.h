#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <windows.h>

#include "basicstructs.h"
#include "baseEnt.h"
#include "label.h"

/*
Text is amended to the end of the string if any charector is recieved as a message
unless the char is < 32, remember to terminatee the string and not allow any bounds errors

have a RGBf forceRGB and bool useForceColor to allow user control of color
*/

class textBox : public baseEnt
{
  
  bool active;
  bool hoverActive;
  RGBf bgColor;
  RGBf bgColor_active;
  RGBf bgColor_hoverActive;
  RGBf forceColor;
  bool useForceColor;
  bool changeOccured();
  bool (*onChange)(char*);
  bool shiftDown;
  
public:
  int setLocation(int x, int y); // Return -1 when out of bounds?
  int setSize(int x, int y); // Return -1 when error
  int eventHandler(UINT message,WPARAM key,int mousex,int mousey);
  //int setOnChange(int (*p)(char*));
  textBox();
  ~textBox();
  int draw();
  int forceUnActive();
  int value;
  int setOnChange(bool (*p)(char*));
  label l_name;
  bool useLabelName;
  char string[512];
  
  /*
  
    int sliderWidth;
    int sliderPosition;

    int leftValue;
    int rightValue;
    int intermediateValue;
    int intermediatePosition;
    bool useIntermediate;
    bool active;
    bool hoverActive;
    RGBf bgColor;
    RGBf sliderColor;
    RGBf bgColor_active;
    RGBf bgColor_hoverActive;
    RGBf sliderColor_active;
    RGBf sliderColor_hoverActive;
    int calcValue();
    int (*onChange)(int);
public:
    //vec2D location;
    //vec2D size;
    int setLocation(int x, int y); // Return -1 when out of bounds?
    int setSize(int x, int y); // Return -1 when error
    int setSliderWidth(int x); // Return -1 on error
    int setSliderPosition(int x); // Default Position, NOTE position on screen not value, also set value
    int setValue(int v); // Must set sliderPosition Inside function, Return -1 on bounds error
    int setBounds(float left, float right); // Standard Linear interpolation
    int setBounds(float left, float right, float inter); // 3 Point interpolation
    int setBgColor(float r, float g, float b);
    int setSliderColor(float r, float g, float b);
    int eventHandler(UINT message,WPARAM key,int mousex,int mousey);
    int setOnChange(int (*p)(int));
     horizSlider();
    ~horizSlider();
    int draw();
    int forceUnActive();
    int value;
    */
};


#endif // TEXTBOX_H
