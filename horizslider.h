#ifndef _HORIZ_SLIDER
#define _HORIZ_SLIDER

#include <windows.h>

#include "basicstructs.h"
#include "baseEnt.h"
#include "label.h"

enum {
  TYPE_FLOAT,
  TYPE_INT
};

class horizSlider : public baseEnt
{
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
    int setValueIntern();
public:
    int (*onChange)(int);
    int type;
    //vec2D location;
    //vec2D size;
    int setLocation(int x, int y); // Return -1 when out of bounds?
    int setSize(int x, int y); // Return -1 when error
    int setSliderWidth(int x); // Return -1 on error
    int setSliderPosition(int x); // Default Position, NOTE position on screen not value, also set value
    int setValue(int v); // Must set sliderPosition Inside function, Return -1 on bounds error
    int setValue_nochange(int v);
    int setBounds(float left, float right); // Standard Linear interpolation
    int setBounds(float left, float right, float inter); // 3 Point interpolation
    int setIntermediatePosition(int x);
    int setBgColor(float r, float g, float b);
    int setSliderColor(float r, float g, float b);
    int eventHandler(UINT message,WPARAM key,int mousex,int mousey);
    int setOnChange(int (*p)(int));
    void setLabelName(char *s);
     horizSlider();
    ~horizSlider();
    int draw();
    int forceUnActive();
    int value;
    bool useLabelName;
    bool useLabelValue;
    label l_name;
    label l_value;
    void setType(int t);
    float getFloatValue();
    int entType();
};

#endif // _HORIZ_SLIDER
