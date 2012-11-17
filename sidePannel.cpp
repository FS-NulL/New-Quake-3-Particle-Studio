#include <windows.h>
#include <GL/gl.h>

#include "sidePannel.h"


sidePannel::sidePannel()
{
	location.x = 120; // Default Location
	location.y = 335;
	size.x = 520;
	size.y = 145;

	visible = false;

	bgColor.R = 0.2f; bgColor.G = 0.3f; bgColor.B = 0.6f;

	for (int i=0;i<MaxEnts;i++) boundEnts[i] = 0; // Initialise array of entitys
}

sidePannel::~sidePannel()
{
}

int sidePannel::setOnClick(int (*p)(void))
{
	onClick = p;
	return 0;
}

int sidePannel::draw()
{
	return 0;
}

int sidePannel::bindEnt(baseEnt *p)
{
	int i = 0;
	while (boundEnts[i] != 0) {
		i++;
		if (i>=MaxEnts) return -1;
	}
	boundEnts[i] = p;
	return i;
}

int sidePannel::forceUnActive()
{
	active = false;
	hoverActive = false;
	return 0;
}

int sidePannel::eventHandler(UINT message,WPARAM key,int mousex,int mousey)
{
	// Do we need to handle anything here?

	// Potentially catch mouse leaving the pannel.
	return 0;
}

/*class sidePannel : public baseEnt
{
  bool active;
  bool hoverActive;
  RGBf bgColor;
  RGBf bgColor_active;
  RGBf bgColor_hoverActive;
  int (*onClick)(void);
  BaseEnt *boundEnts[32];
 public:
  int eventHandler(UINT message,WPARAM key,int mousex,int mousey);
  int setOnClick(int (*p)(void));
  void setColor(float red,float green, float blue, int style=BUTTON_COLOR);
   sidePannel();
  ~sidePannel();
  int draw();
  int forceUnActive();
  bool useLabelName;
  label l_name;
};*/