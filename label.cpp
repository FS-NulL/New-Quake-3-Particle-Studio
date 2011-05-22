#include "label.h"
#include <cstring>

label::label()
{
  location.x = 20;
  location.y = 20;
  alignment = ALIGN_LEFT;
  txtSize = 10;
  string[0] = string[1] = 0;
  maxSize = 0;
}

label::label(char *s)
{
  //label();
  location.x = 20;
  location.y = 20;
  alignment = ALIGN_LEFT;
  txtSize = 10;
  std::strcpy(string,s);
}

label::~label()
{
}

void label::setString(char *s)
{
  std::strcpy(string,s);
}

int label::draw()
{
  if (maxSize > 1) print(string,location.x,location.y,txtSize,maxSize,alignment);
  else print(string,location.x,location.y,txtSize,alignment);
  return 0;
}

int label::eventHandler(UINT message,WPARAM key,int mousex,int mousey)
{
	if (message == WM_LBUTTONUP)
	{
		if (onClick) onClick();
	}
	return 0;
}

bool label::setOnClick(int (*p)(void))
{
	onClick = p;
	return p;
}