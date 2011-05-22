
#include "horizslider.h"

#include <windows.h>
#include <gl/gl.h>
#include <fstream>
//#include <iostream>

horizSlider::horizSlider() // Constructor
{
    onChange=0;
    type = TYPE_INT;
    location.x = 100;
    location.y = 100;
    size.x = 75;
    size.y = 12;
    leftValue = 0;
    rightValue = 100;
    useIntermediate = true;
    intermediateValue = 10;
    intermediatePosition = 37;
    bgColor.R = 0.5f;
    bgColor.G = 0.5f;
    bgColor.B = 0.5f;
    sliderColor.R = 1.0f;    
    sliderColor.G = 1.0f;
    sliderColor.B = 1.0f;
    
    bgColor_active.R = 0.5f;
    bgColor_active.G = 0.5f;
    bgColor_active.B = 0.9f;
    sliderColor_active.R = 1.0f;    
    sliderColor_active.G = 1.0f;
    sliderColor_active.B = 1.0f;
    
    bgColor_hoverActive.R = 0.5f;
    bgColor_hoverActive.G = 0.5f;
    bgColor_hoverActive.B = 0.7f;
    sliderColor_hoverActive.R = 1.0f;    
    sliderColor_hoverActive.G = 1.0f;
    sliderColor_hoverActive.B = 1.0f;
    
    sliderWidth = 5;
    active = false;
    hoverActive = false;
    
    setSliderPosition(0); // Always do this after bounds have been set!    
    
    l_name.alignment = ALIGN_RIGHT;
    l_name.location.x = location.x;
    l_name.location.y = location.y;
    l_name.txtSize = size.y;
    
    l_value.alignment = ALIGN_LEFT;
    l_value.location.x = location.x+size.x;
    l_value.location.y = location.y;
    l_value.txtSize = size.y;
    
    useLabelValue = false;
    useLabelName  = false;
}

horizSlider::~horizSlider() // Destructor
{
}

int horizSlider::draw()
{
    //setValueIntern();
    glBegin(GL_QUADS);
        //BackGround
        glColor3f( 
        active ? bgColor_active.R : hoverActive ? bgColor_hoverActive.R :  bgColor.R, 
        active ? bgColor_active.G : hoverActive ? bgColor_hoverActive.G :  bgColor.G, 
        active ? bgColor_active.B : hoverActive ? bgColor_hoverActive.B :  bgColor.B );
        
        glVertex2f(location.x,location.y);
        glVertex2f(location.x + size.x,location.y);
        glVertex2f(location.x + size.x,location.y + size.y);
        glVertex2f(location.x,location.y + size.y);
        //Slider
        glColor3f( 
        active ? sliderColor_active.R : hoverActive ? sliderColor_hoverActive.R :  sliderColor.R, 
        active ? sliderColor_active.G : hoverActive ? sliderColor_hoverActive.G :  sliderColor.G, 
        active ? sliderColor_active.B : hoverActive ? sliderColor_hoverActive.B :  sliderColor.B );
        
        glVertex2f(( ( sliderPosition - (int)(( (float) sliderWidth)/2) ) > 0 ) ? location.x + sliderPosition - (int)(( (float) sliderWidth)/2) : location.x , location.y + size.y);
        glVertex2f(( ( sliderPosition - (int)(( (float) sliderWidth)/2) ) > 0 ) ? location.x + sliderPosition - (int)(( (float) sliderWidth)/2) : location.x ,location.y);
        glVertex2f(( sliderPosition + (int)(( (float) sliderWidth)/2)  < size.x ) ? location.x + sliderPosition + (int)(( (float) sliderWidth)/2) : location.x + size.x, location.y);
        glVertex2f(( sliderPosition + (int)(( (float) sliderWidth)/2)  < size.x ) ? location.x + sliderPosition + (int)(( (float) sliderWidth)/2) : location.x + size.x, location.y+size.y);
    glEnd();    
    

        if (useIntermediate) 
        {
          glBegin(GL_LINES);
            glColor3f(0.7f, 0.7f, 0.7f);
            glVertex2f(location.x+intermediatePosition,location.y);
            glVertex2f(location.x+intermediatePosition,location.y+size.y);
          glEnd();
        }
    if (useLabelName) l_name.draw();
    if (useLabelValue)
    {
      // convert label to string
      char buffer[15];
      if (type==TYPE_INT) itoa(value,buffer);
      else 
      {
        ftoa(((float)value)/10000,buffer);
        removeTrailingZeros(buffer);
      }
      l_value.setString(buffer);
      l_value.draw();
    }
	return 0;
}

int horizSlider::eventHandler(UINT message,WPARAM key,int mousex,int mousey)
{
    if (message == WM_LBUTTONDOWN) // Set active / move slider to position of mouse
    {
        active = true;
        hoverActive = false;
        if (inBounds(mousex,mousey)) setSliderPosition(mousex - location.x);
    }
    if (message == WM_MOUSEMOVE)
    {
        if (active) 
        {
          // Move slider position to mouse x if can! calc value
          if (mousex < location.x) setSliderPosition(0);
          else if (mousex > location.x + size.x) setSliderPosition(size.x);
          else setSliderPosition(mousex - location.x);
        }
        else  // Hovering? How do we detect left hover?
        {
          if (inBounds(mousex,mousey)) hoverActive = true;
          else hoverActive = false; 
        }
    }
    if (message == WM_LBUTTONUP)   // Unset Active
    {
        active = false;
        if (inBounds(mousex,mousey)) hoverActive = true;
        //bgColor.B = 0.5f;
    }
    if (message == WM_KEYDOWN)
    {
        // VK_LEFT
        // VK_RIGHT
        if (key == VK_LEFT)
        {
          setValue(value - 1);
        }
        if (key == VK_RIGHT)
        {
          setValue(value + 1);
        }
    }
	return 0;
}

int horizSlider::forceUnActive()
{
  active= false;
  hoverActive = false;
  return 0;
}

int horizSlider::setValue(int v)
{
  // Check v is in range
  //if ( (v < leftValue) || (v > rightValue)) return -1;
  value = v;
  if (onChange)
  {
    onChange(value); 
  }
  // Calculate Slider Position
  
  if (v < leftValue) return sliderPosition = 0;
  if (v > rightValue) return sliderPosition = size.x;
  
  if (useIntermediate)
  {
    if (value == intermediateValue) return sliderPosition = intermediatePosition;
    else if (value < intermediateValue) return sliderPosition = (int)(((float)(value - leftValue) / (intermediateValue - leftValue)) * intermediatePosition);
    return sliderPosition = (int)(((float)(value - intermediateValue) / (rightValue - intermediateValue)) * (size.x - intermediatePosition))   + intermediatePosition;
  }
  else
  { 
    return sliderPosition = (int)(((float)(value - leftValue) / (rightValue - leftValue)) * size.x);        
  }
}

int horizSlider::setValue_nochange(int v)
{
  // Check v is in range
  //if ( (v < leftValue) || (v > rightValue)) return -1;
  value = v;
  // Calculate Slider Position
  
  if (v < leftValue) return sliderPosition = 0;
  if (v > rightValue) return sliderPosition = size.x;
  
  if (useIntermediate)
  {
    if (value == intermediateValue) return sliderPosition = intermediatePosition;
    else if (value < intermediateValue) return sliderPosition = (int)(((float)(value - leftValue) / (intermediateValue - leftValue)) * intermediatePosition);
    return sliderPosition = (int)(((float)(value - intermediateValue) / (rightValue - intermediateValue)) * (size.x - intermediatePosition))   + intermediatePosition;
  }
  else
  { 
    return sliderPosition = (int)(((float)(value - leftValue) / (rightValue - leftValue)) * size.x);        
  }
}

int horizSlider::setValueIntern()
{
  // Calculate Slider Position
  if (useIntermediate)
  {
    if (value == intermediateValue) return sliderPosition = intermediatePosition;
    else if (value < intermediateValue) return sliderPosition = (int)(((float)(value - leftValue) / (intermediateValue - leftValue)) * intermediatePosition);
    return sliderPosition = (int)(((float)(value - intermediateValue) / (rightValue - intermediateValue)) * (size.x - intermediatePosition))   + intermediatePosition;
  }
  else
  { 
    return sliderPosition = (int)(((float)(value - leftValue) / (rightValue - leftValue)) * size.x);        
  }
}

int horizSlider::setSliderPosition(int x)
{
  sliderPosition = x;
  
  calcValue();
     
  if (onChange)
  {
    onChange(value);
  }
  return 0;
}

int horizSlider::calcValue()
{
  if (useIntermediate) // 3 Point Interpolation
  {
    if (sliderPosition == intermediatePosition) return value = intermediateValue;
    else if (sliderPosition < intermediatePosition) return value = (int)((intermediateValue - leftValue) * ((float)sliderPosition / (float)intermediatePosition)) + leftValue;
    return value = (int) ((rightValue - intermediateValue) * ((float)(sliderPosition-intermediatePosition) / (float)(size.x-intermediatePosition))) + intermediateValue;
  }
  else // Standard Linear Interpolation 
  {
    return value = (int)((rightValue - leftValue) * ((float)sliderPosition / (float)size.x)) + leftValue;
  }
}

int horizSlider::setSliderWidth(int x)
{
  if (x > 0) sliderWidth = x;
  else return -2;
  if (x > size.x) return -1;
  return 0;
}

int horizSlider::setBounds(float left, float right)
{
  if (right>left)
  {
    leftValue = (int)left;
    rightValue = (int)right;
    useIntermediate = false;
  }
  else return -1;
  return 0;
}

int horizSlider::setBounds(float left, float right,float inter)
{
  if (right>left)
  {
    leftValue = (int)left;
    rightValue = (int)right;
    intermediateValue = (int) inter;
    useIntermediate = true;
  }
  else return -1;
  return 0;
}

int horizSlider::setOnChange(int (*p)(int))
{
  onChange = p;
  return 0;
}

void horizSlider::setLabelName(char *s)
{
  l_name.setString(s);
}


void horizSlider::setType(int t)
{
  type = t;
}

float horizSlider::getFloatValue()
{
  return ((float) value) / 10000;
  return 0;
}


int horizSlider::setIntermediatePosition(int x)
{
  intermediatePosition = x;
  return 0;
}

int horizSlider::entType()
{
  return 1;
}
