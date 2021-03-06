/*
   Name: PARTICLE STUDIO
   Author: Jonathan '$NulL' Norton
   Date: May, 2008 - Nov, 2012 (Blimey!)
*/

// Includes

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string.h>
#include <ctime>
#include <cmath>
#include <gl/gl.h>
#include <gl/glu.h>

#include <fstream> // File Output
#include <algorithm>

#include "basicstructs.h"
#include "horizslider.h"
#include "particleSystem.h"
#include "tabs.h"
#include "textbox.h"
#include "userEntry.h"
#include "sidePannel.h"

//#include "tgal.h"
#include "texture.h"
#include "textdraw.h"
#include "label.h"
#include "listbox.h"
#include "button.h"
#include "aseFormat.h"

#define GL_WIDTH 640
#define GL_HEIGHT 480
#define WN_WIDTH 1024
#define WN_HEIGHT 768

#define MAX_SYSTEMS 15

#define SYS_BUT_ACTIVE 0.5,0.5,0.9
#define SYS_BUT_DISABLE 0.5,0.5,0.5
#define SYS_BUT_CURRENT 0.85,0.85,1.0
#define SYS_BUT_DELETE 1.0,0.2,0.2
#define SYS_BUT_DELETEL 1.0,0.5,0.5

// Function Declarations

LRESULT CALLBACK 
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
VOID EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC);
VOID DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);
void strLower(char *s);
bool textureNameBoxChanged(char *str);
userEntry entry;

class ENTITIES 
{
public:
	static const int MaxEnts = 1024;	

private:
	class ENT 
	{
	   public:
		baseEnt *entPtr;
		ENT();
		int eventRelay(UINT message,WPARAM key,int mousex,int mousey);
	}ents[MaxEnts];
	
	public:
	int active;// location of active ent in array ents[x]
	int hoverActive; // location of hoverActive ent in array ents[x]
	int lastActive; // Location of last active ent, for key notching
	 ENTITIES();
	//~ENTITIES();
	int findEnt(int x,int y); // Return Location in array of ents[x], 0 if mouse not over ent;
	//int bindHorizSlider(horizSlider *ptr); // Returns -1 if ents[] is full
	//int unbindHorizSlider(horizSlider *ptr); // Returns -1 if not foud.
	int bindEnt(baseEnt *ptr); // Returns -1 if ents[] is full
	int unbindEnt(baseEnt *ptr); // Returns -1 if not foud.
	int eventRelay(UINT message,WPARAM key,int mousex,int mousey); // Message Type , Key Press, gl_mousex, gl_mousey
	int drawAll();
	int forceUnActive();
} entities;

ENTITIES::ENTITIES()
{
	active = -1;
	hoverActive = -1;
	lastActive = -1;
}

ENTITIES::ENT::ENT()
{
	entPtr = 0;
}

int ENTITIES::ENT::eventRelay(UINT message,WPARAM key,int mousex,int mousey)
{
	if (entPtr) (*entPtr).eventHandler(message,key,mousex,mousey);
	return 1;
}

int ENTITIES::bindEnt(baseEnt *ptr)
{
	int loc = 0;
	while (ents[loc].entPtr != 0) 
	{
	  loc++;  
	  if (loc > MaxEnts) return -1;   
	}
	ents[loc].entPtr = ptr;
	return loc;
}

int ENTITIES::unbindEnt(baseEnt *ptr)
{
	int loc = 0;
	while (ents[loc].entPtr != ptr) 
	{
	  loc++;  // watchout Infinite loop of ptr isnt an entry!
	  if (loc > MaxEnts) return -1;
	}
	ents[loc].entPtr = 0;
	return 0;
}

int ENTITIES::findEnt(int x,int y)
{
   int loc = 0;
   while (loc < MaxEnts) 
   {
	  if (ents[loc].entPtr != 0) 
	  {
		 baseEnt* ptr = ents[loc].entPtr;
		 if ( (x >= (*ptr).location.x ) && (x <= (*ptr).location.x + (*ptr).size.x ) && (y >= (*ptr).location.y) && (y <= (*ptr).location.y + (*ptr).size.y ) )
		 {
			return loc;
		 }       
	  }
	  loc++;
   }
   return -1;
}

int ENTITIES::eventRelay(UINT message,WPARAM key,int mousex,int mousey)
{
 
	int temp_active = active;
	baseEnt* ePtr = ents[temp_active].entPtr;
	
	if (temp_active != -1) // have active ent, LMB is down
	{
		// Send Event Direct
		ents[temp_active].eventRelay(message,key,mousex,mousey);
		// if the current entPtr has become corrupt during the event handling, return imediatly, reseting active hoveractive to -1
		if (ents[temp_active].entPtr == 0)
		{
			active = hoverActive = lastActive = -1;
			return -1;
		}
		// If LMB UP  set active to -1
		if (message == WM_LBUTTONUP) {
			// is active still hoverActive?		
			if ( (*ents[temp_active].entPtr).inBounds(mousex,mousey) ) hoverActive = temp_active;
			temp_active = -1;
		}  
		active = temp_active;
		return active;
	}
	int loc = findEnt(mousex,mousey);    
	// Mouse Left Hover Ent 
	if (message == WM_MOUSEMOVE)
	{
		if ((loc != hoverActive) && (hoverActive != -1))
		{
			ents[hoverActive].eventRelay(message,key,mousex,mousey);
		}
	}
	

	if (loc != -1)
	{
		if (message == WM_LBUTTONDOWN)
		{
			temp_active = lastActive = loc;
			hoverActive = -1;
			ents[temp_active].eventRelay(message,key,mousex,mousey);
			active = temp_active;
			return active;
		}
		else
		{
			if (message == WM_MOUSEMOVE)
			{
			  hoverActive = loc;
			  ents[hoverActive].eventRelay(message,key,mousex,mousey);
			}
			// right mouse click code here, but how to check if its a slider or not?
			if (message == WM_RBUTTONDOWN)
			{
			  if ((*ents[loc].entPtr).entType()) // default 0 , slider ==1
			  {
				entry.setActive();
				entry.initialBR.x = (*ents[loc].entPtr).location.x + (*ents[loc].entPtr).size.x;
				entry.initialBR.y = (*ents[loc].entPtr).location.y;
				entry.initialTL.x = (*ents[loc].entPtr).location.x;
				entry.initialTL.y = (*ents[loc].entPtr).location.y + (*ents[loc].entPtr).size.y;
				if ((*((horizSlider*)ents[loc].entPtr)).type == TYPE_FLOAT) 
				{
				  entry.isFloat = true;
				  ftoa(  ((float)(*((horizSlider*)ents[loc].entPtr)).value) / 10000  , entry.entryBox.string);
				  removeTrailingZeros( entry.entryBox.string );
				}
				else 
				{
				  entry.isFloat = false;
				  itoa(  (*((horizSlider*)ents[loc].entPtr)).value  , entry.entryBox.string);
				}
				entry.onChange = (*((horizSlider*)ents[loc].entPtr)).onChange;
				entry.ptr = ((horizSlider*)ents[loc].entPtr);
			  }              
			}
		}
	}

	if ((message == WM_KEYDOWN) || ((message == WM_KEYUP)))
	{
		if (lastActive > -1) ents[lastActive].eventRelay(message,key,mousex,mousey);
		return lastActive;
	}
	
	active = temp_active;

	return 0;
	
}

int ENTITIES::drawAll()  // Returns last ent location +1
{
  int loc = 0;
  int c   = 0;
  while (loc < MaxEnts) // What if theres a gap because an ent has been un bound?
  {
	if (ents[loc].entPtr != 0)
	{
	  baseEnt *p = ents[loc].entPtr;
	  if (c!=active) (*p).draw();   // Horrible Hack, something is corrupting the active variable -- the the processes of unbinding the systems button
	  c++;
	}
	loc++;
  }
  if (active!=-1) 
  {
	baseEnt *p = ents[active].entPtr;
	if(p) (*p).draw();
  }
  return c;
}

int ENTITIES::forceUnActive()
{
  // msg to active
  if (active != -1) if (ents[active].entPtr) (*ents[active].entPtr).forceUnActive();
  active = -1;
  // msg to hover active
  if (hoverActive != -1) if (ents[hoverActive].entPtr) (*ents[hoverActive].entPtr).forceUnActive();
  hoverActive = -1;
  return 0;
}

//---------------------------
// End of ENTITIES
//---------------------------

// Should the system hold its own texture
struct ParticleSystemStruct
{
	Texture texture;
	particleSystem ps;
	float x,y,z;
	bool active;
	char basePath[513];
	char texturePath[513];
	bool foundBasePath;
	bool foundTexturePath;
	char shaderName[513];
	bool foundShaderName;
	char sort;
	// Constructor
	ParticleSystemStruct() {
		active = false;
		foundShaderName=false;
		foundTexturePath=false;
		foundBasePath=false;
		x= y= z= 0.0f;
		basePath[0] = 0;
		texturePath[0] = 0;
		sort = 0;
	};
};

int activePS;
ParticleSystemStruct systems[MAX_SYSTEMS];

vec2D mouse;
vec2D pos;

RECT rc; // Window Size

bool paused;
bool minimized;

tab *currentTab = 0;

Texture texture[1]; // Only Have The ui texture

sidePannel BGPannel;

bool hideSidePannelBut;

label drawTime;
button firework;
button rgbPicker;
button lockSeed;
button runOnce;
button butShowPlayer;
button butLoad;
button butSave;
button butLoadAll;
button butSaveAll;
textBox txtBasePath;
textBox txtTextureName;
horizSlider sliderStartAngle;
horizSlider sliderEndAngle;
textBox txtShaderName;
button selectBasePath;
button selectTexture;
horizSlider slider1;
horizSlider slider2;
horizSlider sliderZoom;
horizSlider sliderNP;
horizSlider sliderPSize;
horizSlider sliderHeight;
horizSlider sliderHeightVar;
horizSlider sliderEmitterZ;
horizSlider sliderZBase;
button butNatH;
horizSlider sliderFreq;
button butNatF;
horizSlider sliderPG;
horizSlider sliderMasterPhase;
listBox listZWave;
horizSlider sliderRadius;
horizSlider sliderRadiusVar;
horizSlider sliderEmitterXY;
horizSlider sliderAngleGrouping;
listBox listXYWave;
horizSlider sliderXYPhase;
horizSlider sliderRGB1;
horizSlider sliderRGB2;
listBox listRGBWave;
horizSlider sliderAlpha1;
horizSlider sliderAlpha2;
listBox listAlphaWave;
listBox listSrcBlend;
listBox listDstBlend;
button butRGBXY;
button butAlphaXY;
horizSlider sliderStretch1;
horizSlider sliderStretch2;
listBox listStretchWave;
horizSlider sliderStretchPhase;
button butStretchXY;
horizSlider sliderRotSpeed;
horizSlider sliderRotSpeedVar;
horizSlider sliderRGBPhase;
horizSlider sliderAlphaPhase;
button butBGPS;

// BGPS Pannel Ents
//sliders & button pointers
button buttons[MAX_SYSTEMS];
label  labels[MAX_SYSTEMS];
horizSlider sorts[MAX_SYSTEMS];
horizSlider posXs[MAX_SYSTEMS];
horizSlider posYs[MAX_SYSTEMS];
horizSlider posZs[MAX_SYSTEMS];
horizSlider rotXs[MAX_SYSTEMS];
horizSlider rotYs[MAX_SYSTEMS];
horizSlider rotZs[MAX_SYSTEMS];
label BGLabelLoad, BGLabelName, BGLabelX, BGLabelY, BGLabelZ, BGLabelDel,BGLabelActive,BGLabelSort,BGLabelrX,BGLabelrY,BGLabelrZ;

char togglePlayer=0;

bool foundBasePath=false;
//char basePath[513];
char texturePath[513];
bool foundTexturePath=false;
char shaderName[513];
bool foundShaderName=false;
float bgred,bggreen,bgblue;
HWND hWnd;
char workingDir[MAX_PATH];

bool LoadTGA(Texture *, char *);
int BuildTexture(char *szPathName, GLuint &texid);

void scr2w()
{
	float x_temp,y_temp;
	x_temp = mouse.x;
	y_temp = mouse.y;
	x_temp = x_temp * ((float)GL_WIDTH/(float)rc.right);
	y_temp = y_temp * ((float)GL_HEIGHT/(float)rc.bottom);
	pos.x = (int) x_temp;
	pos.y = (int) ((GL_HEIGHT-1) - y_temp);
}

void draw0()
{
	glVertex2f((0),(0));glVertex2f((10),(0));
	glVertex2f((0),(0));glVertex2f((0),(10));
	glVertex2f((10),(10));glVertex2f((10),(0));
	glVertex2f((10),(10));glVertex2f((0),(10));
}

void draw1()
{
	glVertex2f(10,0);glVertex2f(10,10);
}

void draw2()
{
	glVertex2f(10,0);glVertex2f(0,0);
	glVertex2f(0,0);glVertex2f(0,5);
	glVertex2f(0,5);glVertex2f(10,5);
	glVertex2f(10,5);glVertex2f(10,10);
	glVertex2f(10,10);glVertex2f(0,10);
}

void draw3()
{
	glVertex2f(0,0);glVertex2f(10,0);
	glVertex2f(10,0);glVertex2f(10,10);
	glVertex2f(10,10);glVertex2f(0,10);
	glVertex2f(0,5);glVertex2f(10,5);
}

void draw4()
{
	glVertex2f(10,0);glVertex2f(10,10);
	glVertex2f(0,10);glVertex2f(0,5);
	glVertex2f(0,5);glVertex2f(10,5);
}

void draw5()
{
	glVertex2f(0,0);glVertex2f(10,0);
	glVertex2f(10,0);glVertex2f(10,5);
	glVertex2f(10,5);glVertex2f(0,5);
	glVertex2f(0,5);glVertex2f(0,10);
	glVertex2f(0,10);glVertex2f(10,10);
}

void draw6()
{
	glVertex2f(10,10);glVertex2f(0,10);
	glVertex2f(0,10);glVertex2f(0,0);
	glVertex2f(0,0);glVertex2f(10,0);
	glVertex2f(10,0);glVertex2f(10,5);
	glVertex2f(10,5);glVertex2f(0,5);
}

void draw7()
{
	glVertex2f(0,10);glVertex2f(10,10);
	glVertex2f(10,10);glVertex2f(10,0);
}

void draw8()
{
	glVertex2f(0,0);glVertex2f(10,0);
	glVertex2f(0,5);glVertex2f(10,5);
	glVertex2f(0,10);glVertex2f(10,10);
	glVertex2f(0,0);glVertex2f(0,10);
	glVertex2f(10,0);glVertex2f(10,10);
}

void draw9()
{
	glVertex2f(10,0);glVertex2f(10,10);
	glVertex2f(10,10);glVertex2f(0,10);
	glVertex2f(0,10);glVertex2f(0,5);
	glVertex2f(0,5);glVertex2f(10,5);
}

void drawMPos()
{
	int temp;
	int xhun = pos.x / 100; // Hundreds
	temp = pos.x % 100;
	int xten = temp / 10; // Tens
	temp = temp % 10; // Units
	
	
	
	
		  glPushMatrix();
		  glTranslatef((30),(450),0);
		  glBegin(GL_LINES);
			glColor3f( 1.0f, 1.0f, 1.0f );
			if (xhun == 0) draw0();if (xhun == 1) draw1();
			if (xhun == 2) draw2();if (xhun == 3) draw3();
			if (xhun == 4) draw4();if (xhun == 5) draw5();
			if (xhun == 6) draw6();if (xhun == 7) draw7();
			if (xhun == 8) draw8();if (xhun == 9) draw9();
		  glEnd();
		  glPopMatrix();
		  
		  glPushMatrix();
		  glTranslatef((45),(450),0);
		  glBegin(GL_LINES);
			glColor3f( 1.0f, 1.0f, 1.0f );
			if (xten == 0) draw0();if (xten == 1) draw1();
			if (xten == 2) draw2();if (xten == 3) draw3();
			if (xten == 4) draw4();if (xten == 5) draw5();
			if (xten == 6) draw6();if (xten == 7) draw7();
			if (xten == 8) draw8();if (xten == 9) draw9();
		  glEnd();
		  glPopMatrix();          
		  
		  glPushMatrix();
		  glTranslatef((60),(450),0);
		  glBegin(GL_LINES);
			glColor3f( 1.0f, 1.0f, 1.0f );
			if (temp == 0) draw0();if (temp == 1) draw1();
			if (temp == 2) draw2();if (temp == 3) draw3();
			if (temp == 4) draw4();if (temp == 5) draw5();
			if (temp == 6) draw6();if (temp == 7) draw7();
			if (temp == 8) draw8();if (temp == 9) draw9();
		  glEnd();
		  glPopMatrix();
		  
	int yhun = pos.y / 100;
	temp = pos.y % 100;
	int yten = temp / 10;
	temp = temp % 10;

	
		  glPushMatrix();
		  glTranslatef((30),(435),0);
		  glBegin(GL_LINES);
			glColor3f( 1.0f, 1.0f, 1.0f );
			if (yhun == 0) draw0();if (yhun == 1) draw1();
			if (yhun == 2) draw2();if (yhun == 3) draw3();
			if (yhun == 4) draw4();if (yhun == 5) draw5();
			if (yhun == 6) draw6();if (yhun == 7) draw7();
			if (yhun == 8) draw8();if (yhun == 9) draw9();
		  glEnd();
		  glPopMatrix();
		  
		  glPushMatrix();
		  glTranslatef((45),(435),0);
		  glBegin(GL_LINES);
			glColor3f( 1.0f, 1.0f, 1.0f );
			if (yten == 0) draw0();if (yten == 1) draw1();
			if (yten == 2) draw2();if (yten == 3) draw3();
			if (yten == 4) draw4();if (yten == 5) draw5();
			if (yten == 6) draw6();if (yten == 7) draw7();
			if (yten == 8) draw8();if (yten == 9) draw9();
		  glEnd();
		  glPopMatrix();          
		  
		  glPushMatrix();
		  glTranslatef((60),(435),0);
		  glBegin(GL_LINES);
			glColor3f( 1.0f, 1.0f, 1.0f );
			if (temp == 0) draw0();if (temp == 1) draw1();
			if (temp == 2) draw2();if (temp == 3) draw3();
			if (temp == 4) draw4();if (temp == 5) draw5();
			if (temp == 6) draw6();if (temp == 7) draw7();
			if (temp == 8) draw8();if (temp == 9) draw9();
		  glEnd();
		  glPopMatrix();

	
}


void swapSystems(int newPS, int oldPS)
{

	// For each ui element: update the current value with the value from systems[newPS]
	
	//Shader Name
	memset(txtShaderName.string,0,sizeof(txtShaderName.string)); // fill string with nulls before we write over it
	strcpy(txtShaderName.string,systems[newPS].shaderName );
	if (systems[newPS].foundTexturePath)

	//Texture Name
	memset(txtTextureName.string,0,sizeof(txtTextureName.string));
	strcpy(txtTextureName.string,systems[newPS].texturePath);

	// Fire work mode label
	if (systems[newPS].ps.getFireworkMode() == true) firework.l_name.setString("Experimental Firework Mode: ON");
	else firework.l_name.setString("Experimental Firework Mode: OFF");

	// # Particles
	sliderNP.setValue_nochange(systems[newPS].ps.numParticles);

	//Z Wavrform
	listZWave.selectedItem = systems[newPS].ps.getZWaveform();

	// Height
	sliderHeight.setValue_nochange(systems[newPS].ps.getHeight());

	//Height Var
	sliderHeightVar.setValue_nochange(systems[newPS].ps.getHeightVar());

	//Base Var
	sliderEmitterZ.setValue_nochange(systems[newPS].ps.getEmitterZ());

	// Frequency
	sliderFreq.setValue_nochange( 10000*systems[newPS].ps.getFrequency());

	// Phase Grouping
	sliderPG.setValue_nochange(10000*systems[newPS].ps.getPhaseGrouping());

	// Master Phase
	sliderMasterPhase.setValue_nochange(10000*systems[newPS].ps.getMasterPhase());

	// Size
	sliderPSize.setValue_nochange(systems[newPS].ps.getSize());


	// Radius
	sliderRadius.setValue_nochange(systems[newPS].ps.getRadius());

	// Radius Variance
	sliderRadiusVar.setValue_nochange(systems[newPS].ps.getRadiusVar());

	// Radius Base Var
	sliderEmitterXY.setValue_nochange(systems[newPS].ps.getEmitterXY());

	// XY Waveform
	listXYWave.selectedItem = systems[newPS].ps.getXYWaveform();

	// XY Phase
	sliderXYPhase.setValue_nochange(systems[newPS].ps.getXYPhase()*10000);

	// Start Angle
	sliderStartAngle.setValue_nochange(systems[newPS].ps.getStartAngle());

	// End Angle
	sliderEndAngle.setValue_nochange(systems[newPS].ps.getEndAngle());

	// Grouping
	sliderAngleGrouping.setValue_nochange(systems[newPS].ps.getAngleGrouping()*10000);

	//SRC BLEND
	switch (systems[newPS].ps.srcBlend)
	{
		case GL_ONE:
			listSrcBlend.selectedItem = 0;
			break;
		case GL_ZERO:
			listSrcBlend.selectedItem = 1;
			break;
		case GL_DST_COLOR:
			listSrcBlend.selectedItem = 2;
			break;
		case GL_ONE_MINUS_DST_COLOR:
			listSrcBlend.selectedItem = 3;
			break;
		case GL_SRC_ALPHA:
			listSrcBlend.selectedItem = 4;
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			listSrcBlend.selectedItem = 5;
			break;
	}

	//DST BLEND
	switch (systems[newPS].ps.dstBlend)
	{
		case GL_ONE:
			listDstBlend.selectedItem = 0;
			break;
		case GL_ZERO:
			listDstBlend.selectedItem = 1;
			break;
		case GL_SRC_COLOR:
			listDstBlend.selectedItem = 2;
			break;
		case GL_ONE_MINUS_SRC_COLOR:
			listDstBlend.selectedItem = 3;
			break;
		case GL_SRC_ALPHA:
			listDstBlend.selectedItem = 4;
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			listDstBlend.selectedItem = 5;
			break;
	}

	// RGB1
	sliderRGB1.setValue_nochange(systems[newPS].ps.getRGB1()*10000);

	//RGB2
	sliderRGB2.setValue_nochange(systems[newPS].ps.getRGB2()*10000);

	// RGB Wave
	listRGBWave.selectedItem = systems[newPS].ps.getRGBWave();

	// RGB Phase
	sliderRGBPhase.setValue_nochange(systems[newPS].ps.getRGBPhase()*10000);

	//Alpha1
	sliderAlpha1.setValue_nochange(systems[newPS].ps.getAlpha1()*10000);

	//alpha2
	sliderAlpha2.setValue_nochange(systems[newPS].ps.getAlpha2()*10000);

	//alpha wave
	listAlphaWave.selectedItem = systems[newPS].ps.getAlphaWave();

	// Alpha Phase
	sliderAlphaPhase.setValue_nochange(systems[newPS].ps.getAlphaPhase()*10000);




	// TCMOD STRETCH1
	sliderStretch1.setValue_nochange(systems[newPS].ps.getStretch1()*10000);

	// TCMOD STRETCH2
	sliderStretch2.setValue_nochange(systems[newPS].ps.getStretch2()*10000);

	listStretchWave.selectedItem = systems[newPS].ps.getStretchWave();

	sliderStretchPhase.setValue_nochange(systems[newPS].ps.getStretchPhase()*10000);

	sliderRotSpeed.setValue_nochange(systems[newPS].ps.getRotSpeed());

	sliderRotSpeedVar.setValue_nochange(systems[newPS].ps.getRotSpeedVar());


}

// ****************************************************************** //
// ********************** START EVENT HANDLERS ********************** //
// ****************************************************************** //

bool syncChanges = false;

template<typename Func>
void DoChange( Func func )
{
	if(syncChanges)  
	{
		for (auto &s : systems)	  
			if(s.active)
			{
				func( s.ps );
			}
	}
	else
	{
		func( systems[activePS].ps );
	}
}

void oc1(int p)
{  
	DoChange( [=](particleSystem &ps) {
			ps.setNumParticles(p);
			ps.buildParticles();
		});
}

void pSizeChange(int s)
{
	DoChange( [=](particleSystem &ps) {
			ps.setSize((float)s);
		});
}

void heightChange(int h)
{
	DoChange( [=](particleSystem &ps) {
			ps.setHeight((float)h);
			ps.buildParticles();
		});
}

void heightVarChange(int hv)
{
	DoChange( [=](particleSystem &ps) {
		ps.setHeightVar(hv);
		ps.buildParticles();
	});	
}

void emitterZChange(int z)
{
	DoChange( [=](particleSystem &ps) {
		ps.setEmitterZ((float)z);
		ps.buildParticles();
	});
}

void naturalHeight()
{
	DoChange( [=](particleSystem &ps) {
		double brackets = 1 / (4*ps.getFrequency());
		double sq = std::pow(brackets,2);
		ps.setHeight(  200 * (float) sq  );
		ps.buildParticles();
	});
}

void naturalFreq()
{
	DoChange( [=](particleSystem &ps) {
		float insqrt = ps.getHeight() / 200;
		ps.setFrequency( (float) 1 / ((float) 4 * std::sqrt(insqrt) ));
		ps.buildParticles();
	});
}

void frequencyChange(int f)
{
	DoChange( [=](particleSystem &ps) {
		ps.setFrequency((float) f/10000);
		ps.buildParticles();
	});
}

void zBaseChange(int zb)
{
	DoChange( [=](particleSystem &ps) {
		ps.zBase = ((float) zb)/10000;
		ps.buildParticles();
	});
}

void phaseGChange(int pg)
{
	DoChange( [=](particleSystem &ps) {
		ps.setPhaseGrouping((float) pg/10000);
		ps.buildParticles();
	});
}

void masterPhaseChange(int mp)
{
	DoChange( [=](particleSystem &ps) {
		ps.setMasterPhase((float) mp / 10000);
		ps.buildParticles();
	});
}

void xyPhaseChange(int p)
{
	DoChange( [=](particleSystem &ps) {
		ps.setXYPhase((float) p / 10000);
		ps.buildParticles();
	});
	
}

void zWaveChange(int i)
{
	DoChange( [=](particleSystem &ps) {
		ps.setZWaveform(i);
	});
  
}

void xyWaveChange(int i)
{
	DoChange( [=](particleSystem &ps) {
		ps.setXYWaveform(i);
	});
	
}

void radiusChange(int r)
{
	DoChange( [=](particleSystem &ps) {
		ps.setRadius((float) r);
		ps.buildParticles();
	});
	
}

void radiusVarChange(int rv)
{
	DoChange( [=](particleSystem &ps) {
		ps.setRadiusVar((rv));
		ps.buildParticles();
	});
	
}

void emitterXYChange(int e)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setEmitterXY((float) e);
		ps.buildParticles();
	});
	
}

void angleGroupingChanged(int g)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setAngleGrouping( (float) g / 10000 );
		ps.buildParticles();
	});
	
}

void rgb1Change(int v)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setRGB1((float)v/10000);
		ps.buildParticles();
	});
	
}

void rgb2Change(int v)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setRGB2((float)v/10000);
		ps.buildParticles();
	});
	
}

void alpha1Change(int v)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setAlpha1((float)v/10000);
		ps.buildParticles();
	});
	
}

void alpha2Change(int v)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setAlpha2((float)v/10000);
		ps.buildParticles();
	});
	
}

void rgbWaveChange(int i)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setRGBWave(i);
		ps.buildParticles();
	});
	
}

void alphaWaveChange(int i)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setAlphaWave(i);
		ps.buildParticles();
	});
	
}

void srcBlendChange(int i)
{
	DoChange( [=](particleSystem &ps) {	
		if (i==0) ps.setSrcBlend(GL_ONE); 
		if (i==1) ps.setSrcBlend(GL_ZERO); 
		if (i==2) ps.setSrcBlend(GL_DST_COLOR); 
		if (i==3) ps.setSrcBlend(GL_ONE_MINUS_DST_COLOR); 
		if (i==4) ps.setSrcBlend(GL_SRC_ALPHA); 
		if (i==5) ps.setSrcBlend(GL_ONE_MINUS_SRC_ALPHA); 
	});
	
}

void dstBlendChange(int i)
{
	DoChange( [=](particleSystem &ps) {	
		if (i==0) ps.setDstBlend(GL_ONE); 
		if (i==1) ps.setDstBlend(GL_ZERO); 
		if (i==2) ps.setDstBlend(GL_SRC_COLOR); 
		if (i==3) ps.setDstBlend(GL_ONE_MINUS_SRC_COLOR); 
		if (i==4) ps.setDstBlend(GL_SRC_ALPHA); 
		if (i==5) ps.setDstBlend(GL_ONE_MINUS_SRC_ALPHA);  
	});
	
}

void rgbPhaseChange(int i)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setRGBPhase((float)i/10000);
		ps.buildParticles();
	});
	
}

void alphaPhaseChange(int i)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setAlphaPhase((float)i/10000);
		ps.buildParticles();
	});
	
}

void rgbPhaseMatchXY()
{
	DoChange( [=](particleSystem &ps) {	
		ps.setRGBPhase(ps.getXYPhase());
		ps.buildParticles();
	});
	
}

void alphaPhaseMatchXY()
{
	DoChange( [=](particleSystem &ps) {	
		ps.setAlphaPhase(ps.getXYPhase());
		ps.buildParticles();
	});
	
}

void stretch1Change(int s)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setStretch1( ( (float) s ) / 10000);
		ps.buildParticles();
	});
	
}

void stretch2Change(int s)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setStretch2(((float)s)/10000);
		ps.buildParticles();
	});
	
}

void stretchWaveChange(int i)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setStretchWave(i);
		ps.buildParticles();
	});
	
}

void stretchPhaseChange(int i)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setStretchPhase(((float)i)/10000);
		ps.buildParticles();
	});
	
}

void stretchPhaseMatchXY()
{
	DoChange( [=](particleSystem &ps) {	
		ps.setStretchPhase(systems[activePS].ps.getXYPhase());
		ps.buildParticles();
	});
	
}

void rotSpeedChange(int r)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setRotSpeed(r);
		ps.buildParticles();
	});
	
}

void rotSpeedVarChange(int v)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setRotSpeedVar(v);
		ps.buildParticles();
	});
	
}

int numActiveSystems()
{
	int count = 0;
	for (int i=0;i<MAX_SYSTEMS;i++)
	{
		if (systems[i].active == true) count++;
	}
	return count;
}

void activeProcessor(int i)
{
	//if not active	

	if (systems[i].active == false)
	{
		systems[i].active = true;
		swapSystems(i,activePS);

		{			
			buttons[activePS].setColor(SYS_BUT_ACTIVE,BUTTON_COLOR);				//Light Blue
			buttons[activePS].setColor(SYS_BUT_CURRENT,BUTTON_COLOR_ACTIVE);		//Light Blue
			buttons[activePS].setColor(SYS_BUT_CURRENT,BUTTON_COLOR_HOVERACTIVE);	//Light Blue
		}
		activePS = i;

		{
			buttons[i].setColor(SYS_BUT_CURRENT,BUTTON_COLOR);				//Light Blue
			buttons[i].setColor(SYS_BUT_DELETE,BUTTON_COLOR_ACTIVE);		//Light Blue
			buttons[i].setColor(SYS_BUT_DELETEL,BUTTON_COLOR_HOVERACTIVE);	//Light Blue
		}
	}
	else if (systems[i].active && (i!=activePS)) // active but not current
	{
		//set current
		swapSystems(i,activePS);
		//change current active button to mid grey (active, not current)
		{
			buttons[activePS].setColor(SYS_BUT_ACTIVE,BUTTON_COLOR);				//Light Blue
			buttons[activePS].setColor(SYS_BUT_CURRENT,BUTTON_COLOR_ACTIVE);		//Light Blue
			buttons[activePS].setColor(SYS_BUT_CURRENT,BUTTON_COLOR_HOVERACTIVE);	//Light Blue
		}

		activePS=i;		
		{
			buttons[activePS].setColor(SYS_BUT_CURRENT,BUTTON_COLOR);				//Light Blue
			buttons[activePS].setColor(SYS_BUT_DELETE,BUTTON_COLOR_ACTIVE);		//Light Blue
			buttons[activePS].setColor(SYS_BUT_DELETEL,BUTTON_COLOR_HOVERACTIVE);	//Light Blue
		}

	}
	else // make current unactive and find a new current
	{
		if (numActiveSystems() > 1)
		{
			{
				buttons[activePS].setColor(SYS_BUT_DISABLE,BUTTON_COLOR);				//Light Blue
				buttons[activePS].setColor(SYS_BUT_ACTIVE,BUTTON_COLOR_ACTIVE);		//Light Blue
				buttons[activePS].setColor(SYS_BUT_ACTIVE,BUTTON_COLOR_HOVERACTIVE);	//Light Blue
			}
			int oldPS = activePS;
			systems[i].active = false;
			
			do {
				activePS = (++activePS) % MAX_SYSTEMS;
			} while (systems[activePS].active!=true);
			swapSystems(activePS,oldPS);
			
			{
				buttons[activePS].setColor(SYS_BUT_CURRENT,BUTTON_COLOR);				//Light Blue
				buttons[activePS].setColor(SYS_BUT_DELETE,BUTTON_COLOR_ACTIVE);		//Light Blue
				buttons[activePS].setColor(SYS_BUT_DELETEL,BUTTON_COLOR_HOVERACTIVE);	//Light Blue
			}
		}
	}
}

void Active1Pressed()
{
	// if activePS == 1: turn off and chage to another active system if numactivesystems>1
	activeProcessor(0);
}

void Active2Pressed()
{
	activeProcessor(1);
}

void Active3Pressed()
{
	activeProcessor(2);
}

void Active4Pressed()
{
	activeProcessor(3);
}

void Active5Pressed()
{
	activeProcessor(4);
}

void Active6Pressed()
{
	activeProcessor(5);
}

void Active7Pressed()
{
	activeProcessor(6);
}

void Active8Pressed()
{
	activeProcessor(7);
}

/*
----------------=====================================END OF ON-CHANGE CODE=============================--------------
*/

void getTexture()
{
  // if base path is found remove the base path from the beginning of the string
  // if the base path doesnt prefix the texturepath, then prompt the user with the message 'Please Select From Your Base Path'
  
  // if basepath isnt found, look for 'textures'
  // if not found prompt user to select base path first.
  
  OPENFILENAME ofn;
  char szFileName[MAX_PATH];
  char path[MAX_PATH];
  char temp_base[MAX_PATH];
  std::strcpy(temp_base,systems[0].basePath);
  strLower(temp_base);

  ZeroMemory(&ofn, sizeof(ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = "Texture File\0*.tga;*.jpg\0All Files (*.*)\0*.*\0\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = "tga";
  ofn.Flags = OFN_NONETWORKBUTTON | OFN_READONLY | OFN_NOCHANGEDIR /*| OFN_ALLOWMULTISELECT*/;
  if (GetOpenFileName(&ofn) != 0)
  {
	strLower(szFileName); // Lower Case
	char *p=szFileName;   // Convert Slashes
	while (*p) 
	{
	  if (*p == '\\') *p = '/';
	  p++;
	}
	//std:: ofstream oFile("textureFind.txt",std::ios::app);
	//oFile << foundBasePath << ' ' << basePath << ' ' << szFileName << '\n';
	if (systems[0].foundBasePath)
	{
	  // look for base path at beginning of string
	  // convert '\' to '/' in szFileName

	  //MessageBox(hWnd, temp_base, "Error", MB_OK );
	  //MessageBox(hWnd, szFileName, "Error", MB_OK );
	  if (std::strncmp( temp_base , szFileName , std::strlen(systems[0].basePath) ) == 0)
	  {
		std:strcpy(path, &szFileName[std::strlen(systems[0].basePath)]);
		//MessageBox(hWnd, path, "Error", MB_OK );
		systems[activePS].foundTexturePath = true;
		std::strcpy(systems[activePS].texturePath,path);
		std::strcpy(txtTextureName.string,path);
	  }
	  else
	  {
		MessageBox(hWnd, "Select a texture from inside your base path!", "ERROR!", MB_OK );
	  }
	}
	else // Base path not set search for '/textures/'
	{
	  char *p = std::strstr(szFileName,"/textures/");
	  if (p)
	  {
		p++; // point to fist 't' in /textures/'
		foundTexturePath = true;
		std::strcpy(systems[activePS].texturePath,p);
		std::strcpy(txtTextureName.string,p);
	  }
	  else 
	  {
		MessageBox(hWnd, "Please set your base path first!", "ERROR!", MB_OK );
	  }
	}
	//oFile.close();
  }
  textureNameBoxChanged(txtTextureName.string);
}

void getBasePathDir()
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH];
  char path[MAX_PATH];

  ZeroMemory(&ofn, sizeof(ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = "Any File in your basepath (*.*)\0*.*\0\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = "pk3";
  ofn.Flags = OFN_NONETWORKBUTTON | OFN_READONLY | OFN_NOCHANGEDIR;
  
  
  if (GetOpenFileName(&ofn) != 0)
  {
  
   //MessageBox(hWnd, ofn.lpstrFile, "Error", MB_OK );
   
	std::strncpy(path,szFileName,ofn.nFileOffset);
	path[ofn.nFileOffset] = 0;
	char *p=path;
	while (*p)
	{
	  if (*p == '\\') *p = '/';
	  p++;      
	}
	//MessageBox(hWnd, path, "Error", MB_OK );
	strcpy(systems[0].basePath,path);
	systems[0].foundBasePath = true;
	strcpy(txtBasePath.string,path);
	//after recieved chop off file name and set to base path.
	// add null term
	// convert \ to /
  }
   
   //SetCurrentDirectory(workingDir);
}

void toggleFirework()
{
  if ( systems[activePS].ps.getFireworkMode() == false)
  {
	// turn on
	firework.l_name.setString("Experimental Firework Mode: ON");
	systems[activePS].ps.setFireworkMode(true);
  }
  else
  {
	//turn off
	firework.l_name.setString("Experimental Firework Mode: OFF");
	systems[activePS].ps.setFireworkMode(false);
  }
  systems[activePS].ps.buildParticles();
}

void togglePlayerVis()
{
  if (togglePlayer == 0)
  {
	togglePlayer = 56;
	butShowPlayer.l_name.setString("Player, 56u");
  }
  else if (togglePlayer == 56)
  {
	togglePlayer = 64;
	butShowPlayer.l_name.setString("Player, 64u");
  }
  else if (togglePlayer == 64)
  {
	togglePlayer = 72;
	butShowPlayer.l_name.setString("Player, 72u");
  }
  else if (togglePlayer == 72)
  {
	togglePlayer = 0;
	butShowPlayer.l_name.setString("Show Player");
  }
}

void colourPickerPressed()
{
  //get bg_rgb values
  //convert to DWORD
  
  unsigned char red, green, blue;
  
  red = (unsigned char) (bgred * 255);
  blue = (unsigned char) (bgblue * 255);
  green = (unsigned char) (bggreen * 255);
  
  CHOOSECOLOR cc;                 // common dialog box structure 
  static COLORREF acrCustClr[16]; // array of custom colors 
  static DWORD rgbCurrent;        // initial color selection
  
  rgbCurrent = 0;
  rgbCurrent |= red;
  rgbCurrent |= (( ((DWORD) green) << 8 ) & 0x0000FF00);
  rgbCurrent |= (( ((DWORD) blue)  << 16) & 0x00FF0000);

  // Initialize CHOOSECOLOR 
  ZeroMemory(&cc, sizeof(cc));
  cc.lStructSize = sizeof(cc);
  cc.hwndOwner = 0;
  cc.lpCustColors = (LPDWORD) acrCustClr;
  cc.rgbResult = rgbCurrent;
  cc.Flags = CC_FULLOPEN | CC_RGBINIT;
  
  if (ChooseColor(&cc)==TRUE) {
	//hbrush = CreateSolidBrush(cc.rgbResult);
	rgbCurrent = cc.rgbResult; 
  }
  
  //char red,green,blue;
  
  red = rgbCurrent & 0x0000FF;
  green = (rgbCurrent & 0x00FF00) >> 8;
  blue = (rgbCurrent & 0xFF0000) >> 16;
  
  bgred = ((float)red) / 255;
  bggreen = ((float)green) / 255;
  bgblue = ((float)blue) / 255;
}

void toggleOnce()
{
	if(particleSystem::toggleOnce())
		runOnce.l_name.setString("Once");
	else
		runOnce.l_name.setString("Continuous");
}

void toggleSeed()
{
	particleSystem::toggleFixedSeed();

	if(particleSystem::getFixedSeed() == true)
		lockSeed.l_name.setString("Seed Locked");
	else
		lockSeed.l_name.setString("Random Seed");

	for (auto &s : systems)
	{
		s.ps.buildParticles(); // rebuild all systems, to reflect the change
	}
}

void strLower(char *s)
{
  while (*s)
  {
	if ( ( *s > 64 ) && ( *s < 92 ) ) *s = *s | 32;
	s++;
  }
}

//Write the ASE prefab
int writeASE(char *filename)
{
	// Try to open file

	std::ofstream oFile(filename,std::ios::out);
	if (oFile.is_open())
	{
		// Count number of active systems
		int sys_count=0;
		for (int i =0;i<MAX_SYSTEMS;i++)
			if (systems[i].active) sys_count++;

		// Write Header
		oFile << ASE_HEADER;
		oFile << sys_count;

		// Material List
		int count=0;
		for (int i =0;i<MAX_SYSTEMS;i++)
		{
			if (systems[i].active)
			{
				char shadername[513];
				strcpy(shadername,systems[i].shaderName);
				strLower(shadername);
				oFile << ASE_MATERIAL1;
				oFile << count;
				oFile << ASE_MATERIAL2;
				oFile << shadername;
				oFile << ASE_MATERIAL3;
				count++;
			}
		}
		oFile << ASE_MATERIALE;

		// End of Materials

		// Start of Geometry
		count=0;
		for (int i=0;i<MAX_SYSTEMS;i++)
		{
			if (!systems[i].active) continue;
			oFile << ASE_GEOM1;
			oFile << count;
			oFile << ASE_GEOM2;
			oFile << count;
			oFile << ASE_GEOM3;

			// Vertex list
			oFile << ASE_GEOM4;
			oFile << " 0\t"<< systems[i].x-0.5*systems[i].ps.getSize() <<"\t" << systems[i].y-0.5*systems[i].ps.getSize() << "\t" << systems[i].z;
			oFile << ASE_GEOM4;
			oFile << " 1\t"<< 0.5*systems[i].ps.getSize() + systems[i].x<<"\t"<<systems[i].y-0.5*systems[i].ps.getSize()<<"\t"<<systems[i].z;
			oFile << ASE_GEOM4;
			oFile << " 2\t"<< 0.5*systems[i].ps.getSize() +systems[i].x<<"\t"<<0.5*systems[i].ps.getSize() +systems[i].y<<"\t"<<systems[i].z;
			oFile << ASE_GEOM4;
			oFile << " 3\t"<< systems[i].x-0.5*systems[i].ps.getSize() <<"\t" << 0.5*systems[i].ps.getSize() +systems[i].y << "\t" << systems[i].z;

			oFile << ASE_GEOM5;
			oFile << count;
			oFile << ASE_GEOM6;

			count++;
		}
		oFile.close();
	}
	else 
	{
		MessageBox(NULL,"Error opening file for writing.","ASE Export",MB_ICONERROR);
		return 0;
	}

	return 1;
}

/*

Path needs moving to local system last 

Changes for multi system:

Produce an export .shader name from the first active system

Truncate export file before anything is written

Steps::

1, get index of first active system

2, produce export file name from basepath[0] and shadername[index] (or PS_TEST if shadername is not set)

3, loop through systems: for each active:	correct the texture name
											call the export method



ASE:::
	expoprt ase file containing each active system
	-- horizontal surface for each particle system
	-- ASE format allows for arbitary position and scale and rotation (don't need that one) -- must check q3map2 supports this.
	-- if above works - export single unit quad over origin (-0.5,-0.5  ->  0.5,0.5)

*/
int runExport(tab *t)
{
  bool status = true;

  char shaderPath[64];
  char exportPath[512];
  char exportShaderName[512];
  char exportTexturePath[512];

  if(1) // Dont run this yet!
  {
	  // Multi System Stuff goes in here

	  // Get index of first active system
	  int index=0;
	  while(!systems[index].active) index++; // There is always 1 active so bounds checking is not required

	  // Produce export filename from basepath[0]  or '\\' if base path doesnt exist

	  if (systems[index].foundShaderName)
	  {
		//extract path to use as filename
		char *s=systems[index].shaderName;
		char *d=shaderPath;
		while ((*s) != '/') *d++ = *s++;
		*d = 0;
		std::strcpy(exportShaderName,"textures/");
		std::strcat(exportShaderName,systems[index].shaderName);
	  }
	  else 
	  {
		std::strcpy(shaderPath,"ps_test");
		std::strcpy(exportShaderName,"textures/ps_test/ps_test");
	  }
	  
	  bool basePathOK = false;
	  
	  if (systems[0].foundBasePath)
	  {
		char scriptsDir[MAX_PATH];

		std::strcpy(exportPath,systems[0].basePath);
		std::strcat(exportPath,"scripts/");
		strcpy(scriptsDir,exportPath);
		std::strcat(exportPath,shaderPath);
		std::strcat(exportPath,".txt");

		// Add Save File Dialog for writing shader files

		  OPENFILENAME ofn;
		  char szFileName[MAX_PATH];
		  char path[MAX_PATH];
		  //bool status;

		  ZeroMemory(&ofn, sizeof(ofn));
		  szFileName[0] = 0;
		  strcpy(szFileName,shaderPath);

		  ofn.lStructSize = sizeof(ofn);
		  ofn.hwndOwner = hWnd;
		  ofn.lpstrFilter = "Shader File (*.shader)\0*.shader\0All Files (*.*)\0*.*\0\0";
		  ofn.lpstrFile = szFileName;
		  ofn.lpstrInitialDir = scriptsDir;
		  ofn.nMaxFile = MAX_PATH;
		  ofn.lpstrDefExt = "shader";
		  //ofn.Flags = OFN_NONETWORKBUTTON | OFN_NOCHANGEDIR;
		  
		  
		  if (GetSaveFileName(&ofn) != 0)
		  {
			  strcpy(exportPath,szFileName);
		  }
		  else
		  {
			  return 0;
		  }
		
		
		// TRY AND OPEN EXPORT PATH	    
		std::ofstream testExport(exportPath,std::ios::app);
		if (testExport.is_open())
		{
		  basePathOK = true;
		  testExport.close();
		}

		// If we cannot open the basepath\scripts folder for writing, then export to the current directory (most likely the programs folder)
		if (!basePathOK)
		{
			GetCurrentDirectory(MAX_PATH, exportPath);
			std::strcat(exportPath,"\\");
			std::strcat(exportPath,shaderPath);
			std::strcat(exportPath,".txt");
		}

		// Create and/or truncate exportpath file

		bool exportStatus = true;
		std::ofstream tFile(exportPath,std::ios::out);
		if (tFile.is_open()) tFile.close();
		else 
		{
			exportStatus = false;
			MessageBox(NULL,"Error opening file for writing.","Run Export",MB_OK|MB_ICONERROR);
			return 0;
		}


		for (int i=index;i<MAX_SYSTEMS;i++) // start at first active system
		{
			if (systems[i].active)
			{
				// Correct export texture name
				// Run Export

				  if (systems[i].foundShaderName)
				  {
					//extract path to use as filename
					char *s=systems[i].shaderName;
					char *d=shaderPath;
					while ((*s) != '/') *d++ = *s++;
					*d = 0;
					std::strcpy(exportShaderName,"textures/");
					std::strcat(exportShaderName,systems[i].shaderName);
				  }
				  else 
				  {
					std::strcpy(shaderPath,"ps_test");
					std::strcpy(exportShaderName,"textures/ps_test/ps_test");
				  }

				  if (systems[i].foundTexturePath)
				  {
					std::strcpy(exportTexturePath,systems[i].texturePath);
				  }
				  else
				  {
					std::strcpy(exportTexturePath,"textures/common/caulk.tga");
				  }
				  
				  strLower(exportTexturePath);
				  strLower(exportShaderName);
				  strLower(exportPath);
				  
				  //convert .jpg and .jpeg to .tga
					   
				  int length = std::strlen(exportTexturePath);
				  if ((( exportTexturePath[length-1] | 32 ) == 'g' ) && (( exportTexturePath[length-2] | 32 ) == 'p' ) && (( exportTexturePath[length-3] | 32 ) == 'j' ) )
				  {
					exportTexturePath[length-3] = 't';
					exportTexturePath[length-2] = 'g';
					exportTexturePath[length-1] = 'a';
				  }
				  else if (
					  (( exportTexturePath[length-1] | 32 ) == 'g' ) && 
					  (( exportTexturePath[length-2] | 32 ) == 'e' ) &&  
					  (( exportTexturePath[length-3] | 32 ) == 'p' ) &&  
					  (( exportTexturePath[length-4] | 32 ) == 'j' ) )
				  {
					exportTexturePath[length-4] = 't';
					exportTexturePath[length-3] = 'g';
					exportTexturePath[length-2] = 'a';
					exportTexturePath[length-1] = 0;
				  }			  
				  
				  std::strcpy(systems[i].ps.textureName,exportTexturePath);
				  std::strcpy(systems[i].ps.shaderBaseName,exportShaderName);

				  if (systems[i].sort > 0) systems[i].ps.sort = systems[i].sort;

				  char str[550];
				  
				  if( systems[i].ps.buildShaderFile(exportPath) ) // Try to export
				  {
					std::strcpy(str,"Shader File Exported: ");
					std::strcat(str,exportPath);
				  }
				  else
				  {
					std::strcpy(str,"Shader Export Failed (buildShaderFile returned false) : ");
					std::strcat(str,exportPath);
					MessageBox(NULL, str, "Export", MB_OK);
					return 0;
				  }
				  
			} // end if active
		} // end for

		// Done Shader Files
		// Now do ASE Export

		// Give user option dialog

		//MessageBox(NULL,"basepath","Export",MB_OK); // hWnd -> NULL
		if( MessageBox(hWnd,"Shader Exported.\nDo you wish to export an ASE model prefab of the system?","ASE Export",MB_YESNO|MB_ICONQUESTION) == IDYES)
		{
			// Export the ase model

			// Loop through sytems for material list and again for geometry list

			// Get a user specified file name

			  OPENFILENAME ofn;
			  char szFileName[MAX_PATH];
			  char path[MAX_PATH];
			  bool status;

			  ZeroMemory(&ofn, sizeof(ofn));
			  szFileName[0] = 0;

			  ofn.lStructSize = sizeof(ofn);
			  ofn.hwndOwner = hWnd;
			  ofn.lpstrFilter = "ASE Model (*.ase)\0*.ase\0All Files (*.*)\0*.*\0\0";
			  ofn.lpstrFile = szFileName;
			  ofn.nMaxFile = MAX_PATH;
			  ofn.lpstrDefExt = "pset";
			  ofn.Flags = OFN_NONETWORKBUTTON | OFN_NOCHANGEDIR;
			  
			  
			  if (GetSaveFileName(&ofn) != 0)
			  {
				if (writeASE(szFileName)) status = true;
				else status = false;
				if (status) MessageBox(NULL, "Save Successful", "ASE Saving...", MB_OK);
				else MessageBox(NULL, "Error During Save", "Saving...", MB_OK);
			  }

		}
		// IDYES = int 6 ;;;;; IDNO = int 7

	  }// if basepath found
	  else {
		  MessageBox(NULL,"basepath error","Export",MB_OK);
	  }
  }//if 0
  return 0;
}

int tabClicked(tab *t)
{
  if (!t) return -1;
  // set active of currentTab to false
  if (currentTab) currentTab->setActive(false);
  int i = 0;
  baseEnt * ent;
  if (currentTab)
	  while (ent = currentTab->getLinked(i))
	  {
		entities.unbindEnt(ent);
		i++;
	  }
  // remove ents
  // set new active tab
  currentTab=t;  
  if (currentTab) currentTab->setActive(true);
  // add ents
  i=0;
  ent=0;
  if (currentTab) 
	  while (ent = currentTab->getLinked(i))
	  {
		entities.bindEnt(ent);
		i++;
	  }
  return 0;
}

bool shaderNameBoxChanged(char *str)
{
  // Must be in format PATH/SHADER
  // so first char must not be '/'
  // '/' must be present once only
  // must be chars after '/'
  
  // count '/'s
  int count =0;
  char *p=str;
  while (*p)
  {
	if (*p == 32) *p = '/';
	if (*p == '/') count++;
	p++;
  }
  if (count == 1)    // only continue if count == 1
  {
	if (*str == '/') 
	{
	  systems[activePS].foundShaderName = false;
	  return false;  // check 1st char isnt '/'
	}
	p--;
	if (*p == '/') 
	{
	  systems[activePS].foundShaderName = false;
	  return false;  // check last char isnt '/'
	}
	systems[activePS].foundShaderName = true;
	std::strcpy(systems[activePS].shaderName,str);
	return true;
  }
  //std::ofstream oFile("shaBox.txt",std::ios::app);
  //oFile <<str << "   " << count << '\n';
  //oFile.close();
  systems[activePS].foundShaderName = false;
  return false;
}

bool basePathBoxChanged(char *str)
{
  // NEW VERSION
  
  // ammend '/' to end , if needed
  
  // copy to basePath
  
  // if *str = 0, basePathFound = false
  //  else basePathFound = true
  
  char basePathStr[523];
  std::strcpy(basePathStr,str);
  
  if ( (basePathStr[std::strlen(basePathStr)-1] != '/') && (basePathStr[std::strlen(basePathStr)-1] != '\\') )
  {
	basePathStr[std::strlen(basePathStr)+1] = 0;
	basePathStr[std::strlen(basePathStr)] = '/';
  }
  if (*str == 0) 
  {
	basePathStr[0] = basePathStr[1] = 0;
	systems[0].foundBasePath = false;
  }
  else systems[0].foundBasePath = true;
  
  std::strcpy(systems[0].basePath,basePathStr);
  
  return false;
}

// Texture Box Changed -- Alter for multi PS system
// How to alter for animmap

bool textureNameBoxChanged(char *str)
{
  bool status = false;
  std::strcpy(systems[activePS].texturePath,str);
  // Try and find texture if base path is found.
  if (systems[0].foundBasePath)
  {
	//std::ofstream oFile("textureName.txt",std::ios::app);
	//oFile << basePath << " + " << str << '\n';
	char pathString[1025];
	std::strcpy(pathString,systems[0].basePath);
	std::strcat(pathString,str);
	if ((pathString[std::strlen(pathString)-1]=='L') && (pathString[std::strlen(pathString)-2]=='U') && (pathString[std::strlen(pathString)-3]=='N')) return false;
	//oFile << pathString << '\n';
	// if lst 3 chars are "NUL" instantly return false
	std::ifstream tryFile(pathString,std::ios::binary);
	if (tryFile.is_open())
	{
	  tryFile.close();
	  
	  int length = std::strlen(pathString);
	  
	  if (
	  (( pathString[length-1] | 32 ) == 'a' ) && 
	  (( pathString[length-2] | 32 ) == 'g' ) &&  
	  (( pathString[length-3] | 32 ) == 't' ) )
	  {
		if (LoadTGA(&systems[activePS].texture, pathString))
		{
		  status = true;
		
		  glGenTextures(1, &systems[activePS].texture.texID);				// Create The Texture ( CHANGE )
			  glBindTexture(GL_TEXTURE_2D, systems[activePS].texture.texID);
			  if (systems[activePS].texture.bpp == 24) glTexImage2D(GL_TEXTURE_2D, 0, 3, systems[activePS].texture.width, systems[activePS].texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, systems[activePS].texture.imageData);
			  else glTexImage2D(GL_TEXTURE_2D, 0, 4, systems[activePS].texture.width, systems[activePS].texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, systems[activePS].texture.imageData);
			  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  
			  if (systems[activePS].texture.imageData)						// If Texture Image Exists ( CHANGE )
			  {
				  free(systems[activePS].texture.imageData);					// Free The Texture Image Memory ( CHANGE )
			  }
			  
			  //std::strcpy(ps.textureName,pathString);
		  
		}
	  }
	  else if (
	  (( pathString[length-1] | 32 ) == 'g' ) && 
	  (( pathString[length-2] | 32 ) == 'p' ) &&  
	  (( pathString[length-3] | 32 ) == 'j' ) )
	  {
		// Its a jpg
		if (BuildTexture(pathString,systems[activePS].texture.texID)) status = true;
	  }
	  else if (
	  (( pathString[length-1] | 32 ) == 'g' ) && 
	  (( pathString[length-2] | 32 ) == 'e' ) &&  
	  (( pathString[length-3] | 32 ) == 'p' ) &&  
	  (( pathString[length-4] | 32 ) == 'j' ) )
	  {
		// Its a jpg
		if (BuildTexture(pathString,systems[activePS].texture.texID)) status = true;
	  }
	}
  }
  
  if (systems[activePS].texturePath[0] == 0) systems[activePS].foundTexturePath = false;
  else systems[activePS].foundTexturePath = true;
  
  return status;
}

void setDefaultTexture(Texture &t)
{
  unsigned int px_w = 0;
  px_w = ~px_w;
  unsigned int px_b = 255;
  
  unsigned int data[64] = {
							  255,  ~0,  ~0,  ~0,  ~0,  ~0,  ~0,  255,
							  ~0,  255,  ~0,  ~0,  ~0,  ~0,  255,  ~0,
							  ~0,  ~0,  255,  ~0,  ~0,  255,  ~0,  ~0,
							  ~0,  ~0,  ~0,  255,  255,  ~0,  ~0,  ~0,
							  ~0,  ~0,  ~0,  255,  255,  ~0,  ~0,  ~0,
							  ~0,  ~0,  255,  ~0,  ~0,  255,  ~0,  ~0,
							  ~0,  255,  ~0,  ~0,  ~0,  ~0,  255,  ~0,
							  255,  ~0,  ~0,  ~0,  ~0,  ~0,  ~0,  255
						  };
  
  t.width  = 8;
  t.height = 8;
  
  t.imageData = new GLubyte[4*t.width*t.height];
  memcpy(t.imageData,data,4*t.width*t.height);
}

// From Nehe Lesson 33
int loadGLTextures()											// Load Bitmaps And Convert To Textures
{
	int Status=FALSE;											// Status Indicator
	
	//std::ofstream oFile("debug_img_2.txt",std::ios::app);
	//oFile << ps.textureName << '\n';
	//oFile.close();

	// Load The Bitmap, Check For Errors.
	//if (LoadTGA(&texture[0], "Data/def_sprite.tga") && LoadTGA(&texture[1], "Data/text.tga"))


	// Default Particle Texture
	for(int i=0;i<MAX_SYSTEMS;i++)
	{
		if (!LoadTGA(&systems[i].texture, "Data/def_sprite.tga")) setDefaultTexture(systems[i].texture);
	}

	/*if (!LoadTGA(&systems[0].texture, "Data/def_sprite.tga")) setDefaultTexture(systems[0].texture);
	if (!LoadTGA(&systems[1].texture, "Data/def_sprite.tga")) setDefaultTexture(systems[1].texture);
	if (!LoadTGA(&systems[2].texture, "Data/def_sprite.tga")) setDefaultTexture(systems[2].texture);
	if (!LoadTGA(&systems[3].texture, "Data/def_sprite.tga")) setDefaultTexture(systems[3].texture);*/

	// Text Image
	if (!LoadTGA(&texture[0], "Data/text.tga")) setDefaultTexture(texture[0]); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	//{
	Status=TRUE;											// Set The Status To TRUE

	for (int loop=0; loop<MAX_SYSTEMS; loop++)						// Loop Through Both Textures
	{
		// Typical Texture Generation Using Data From The TGA ( CHANGE )
		glGenTextures(1, &systems[loop].texture.texID);				// Create The Texture ( CHANGE )
		glBindTexture(GL_TEXTURE_2D, systems[loop].texture.texID);
		if (systems[loop].texture.bpp == 24) glTexImage2D(GL_TEXTURE_2D, 0, 3, systems[loop].texture.width, systems[loop].texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, systems[loop].texture.imageData);
		else glTexImage2D(GL_TEXTURE_2D, 0, 4, systems[loop].texture.width, systems[loop].texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, systems[loop].texture.imageData);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);


		if (systems[loop].texture.imageData)						// If Texture Image Exists ( CHANGE )
		{
			free(systems[loop].texture.imageData);					// Free The Texture Image Memory ( CHANGE )
			systems[loop].texture.imageData = NULL;
		}
	}
	//}


// Typical Texture Generation Using Data From The TGA ( CHANGE )
	glGenTextures(1, &texture[0].texID);				// Create The Texture ( CHANGE )
	glBindTexture(GL_TEXTURE_2D, texture[0].texID);
	if (texture[0].bpp == 24) glTexImage2D(GL_TEXTURE_2D, 0, 3, texture[0].width, texture[0].height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture[0].imageData);
	else glTexImage2D(GL_TEXTURE_2D, 0, 4, texture[0].width, texture[0].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture[0].imageData);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);


	if (texture[0].imageData)						// If Texture Image Exists ( CHANGE )
	{
		free(texture[0].imageData);					// Free The Texture Image Memory ( CHANGE )
	}

	return Status;												// Return The Status
}

void startAngleChange(int a)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setStartAngle(a);
		ps.buildParticles();
	});
}

void endAngleChange(int a)
{
	DoChange( [=](particleSystem &ps) {	
		ps.setEndAngle(a);
		ps.buildParticles();
	});
}

bool loadDefaultsFromFile()
{
  std::ifstream iFile("data/def.dat",std::ios::in);
  if (iFile.is_open())
  {
	char buffer[MAX_PATH];
	iFile.getline(buffer,MAX_PATH);
	if (iFile.fail()) return false;
	std::strcpy(txtBasePath.string,buffer);

	for (int i=0;i<MAX_SYSTEMS;i++)
	{
		std::strcpy(systems[i].basePath,buffer);
		systems[i].foundBasePath = true;
	}
  }
  else return false;
  return true;
}

bool saveDefaultsToFile()
{
  std::ofstream oFile("data/def.dat",std::ios::out);
  if (oFile.is_open())
  {
	oFile << systems[0].basePath << '\n';
	oFile.close();
  }
  else return false;
  return true;
}

bool saveStateToFile(char *filename,std::ofstream &oFile)
{
  // Do we write slider Values of particle system values. Or both?
  // -- Slider  Values
  
  //std::ofstream oFile(filename,std::ios::out | std::ios::binary);
  if (oFile.is_open())
  {
	oFile.write( (const char*) txtBasePath.string, sizeof(txtBasePath.string) );
	oFile.write( (const char*) txtTextureName.string, sizeof(txtTextureName.string) );
	oFile.write( (const char*) &txtShaderName.string, sizeof(txtShaderName.string) );
	
	oFile.write( (const char*) &sliderStartAngle.value, sizeof(int) );
	oFile.write( (const char*) &sliderEndAngle.value, sizeof(int) );
	
	oFile.write( (const char*) &slider1.value, sizeof(int) );
	oFile.write( (const char*) &slider2.value, sizeof(int) );
	oFile.write( (const char*) &sliderZoom.value, sizeof(int) );
	oFile.write( (const char*) &sliderNP.value, sizeof(int) );
	oFile.write( (const char*) &sliderPSize.value, sizeof(int) );
	oFile.write( (const char*) &sliderHeight.value, sizeof(int) );
	oFile.write( (const char*) &sliderHeightVar.value, sizeof(int) );
	oFile.write( (const char*) &sliderEmitterZ.value, sizeof(int) );
	oFile.write( (const char*) &sliderFreq.value, sizeof(int) );
	oFile.write( (const char*) &sliderPG.value, sizeof(int) );
	oFile.write( (const char*) &sliderMasterPhase.value, sizeof(int) );
	oFile.write( (const char*) &listZWave.selectedItem, sizeof(int) );
	oFile.write( (const char*) &sliderRadius.value, sizeof(int) );
	oFile.write( (const char*) &sliderRadiusVar.value, sizeof(int) );
	oFile.write( (const char*) &sliderEmitterXY.value, sizeof(int) );
	oFile.write( (const char*) &sliderAngleGrouping.value, sizeof(int) );
	oFile.write( (const char*) &listXYWave.selectedItem, sizeof(int) );
	oFile.write( (const char*) &sliderXYPhase.value, sizeof(int) );
	oFile.write( (const char*) &sliderRGB1.value, sizeof(int) );
	oFile.write( (const char*) &sliderRGB2.value, sizeof(int) );
	oFile.write( (const char*) &listRGBWave.selectedItem, sizeof(int) );
	oFile.write( (const char*) &sliderRGBPhase.value, sizeof(int) );
	oFile.write( (const char*) &sliderAlpha1.value, sizeof(int) );
	oFile.write( (const char*) &sliderAlpha2.value, sizeof(int) );
	oFile.write( (const char*) &listAlphaWave.selectedItem, sizeof(int) );
	oFile.write( (const char*) &sliderAlphaPhase.value, sizeof(int) );
	oFile.write( (const char*) &listSrcBlend.selectedItem, sizeof(int) );
	oFile.write( (const char*) &listDstBlend.selectedItem, sizeof(int) );
	oFile.write( (const char*) &sliderStretch1.value, sizeof(int) );
	oFile.write( (const char*) &sliderStretch2.value, sizeof(int) );
	oFile.write( (const char*) &listStretchWave.selectedItem, sizeof(int) );
	oFile.write( (const char*) &sliderStretchPhase.value, sizeof(int) );
	oFile.write( (const char*) &sliderRotSpeed.value, sizeof(int) );
	oFile.write( (const char*) &sliderRotSpeedVar.value, sizeof(int) );
	oFile.write( (const char*) &bgred, sizeof(float) );
	oFile.write( (const char*) &bggreen, sizeof(float) );
	oFile.write( (const char*) &bgblue, sizeof(float) );
	bool fire = systems[activePS].ps.getFireworkMode();
	oFile.write( (const char*) &fire, sizeof(bool) );
	oFile.write( (const char*) &sliderZBase.value, sizeof(int));
	//oFile.close();
	
	horizSlider* rotx[MAX_SYSTEMS];
	horizSlider* roty[MAX_SYSTEMS];
	horizSlider* rotz[MAX_SYSTEMS];/*
	rotx[0] = &BGrx1;	rotx[1] = &BGrx2;	rotx[2] = &BGrx3;	rotx[3] = &BGrx4;	
	roty[0] = &BGry1;	roty[1] = &BGry2;	roty[2] = &BGry3;	roty[3] = &BGry4;	
	rotz[0] = &BGrz1;	rotz[1] = &BGrz2;	rotz[2] = &BGrz3;	rotz[3] = &BGrz4;*/

	oFile.write( (const char*) &(rotXs[activePS].value), sizeof(int));
	oFile.write( (const char*) &(rotXs[activePS].value), sizeof(int));
	oFile.write( (const char*) &(rotXs[activePS].value), sizeof(int));



	return true;
	
  }
  else return false;
	
}

void openBGPSPannel()
{
	// Open the Side Pannel
	// Set Pannel to Active 
	// Bind Ents to Global Ents

	BGPannel.active = true;

	for (int i=0;i<sidePannel::MaxEnts;i++)
	{
		if (BGPannel.boundEnts[i] != 0)
			entities.bindEnt(BGPannel.boundEnts[i]);
	}
	// This needs to be delayed
	//entities.unbindEnt(&butBGPS);
	hideSidePannelBut = true;
	butBGPS.renderable = false;
}

int closeBGPSPannel() // Occurs when mouse is clicked outside of the pannel
{
	// Set active to false
	// Remove ents from Global

	BGPannel.active = false;

	for (int i=0;i<sidePannel::MaxEnts;i++)
	{
		if (BGPannel.boundEnts[i] != 0)
			entities.unbindEnt(BGPannel.boundEnts[i]);
	}
	//entities.bindEnt(&butBGPS);
	butBGPS.renderable = true;

	return 0;
}

bool loadStateFromFile(char *filename,particleSystem &ps,std::ifstream &iFile);
bool loadBGStateFromFile(char *filename, int index);
int pannelLoad(int index);

int pannelLoad1()
{
	pannelLoad(1);
	return 0;
}

int pannelLoad2()
{
	pannelLoad(2);
	return 0;
}

int pannelLoad3()
{
	pannelLoad(3);
	return 0;
}

int pannelLoad4()
{
	pannelLoad(4);
	return 0;
}

int pannelLoad(int index) // Particel System Index
{
	// Load data from file, store in bgPartices[index].ps
  OPENFILENAME ofn;
  char szFileName[MAX_PATH];
  char path[MAX_PATH];
  bool status;

  ZeroMemory(&ofn, sizeof(ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = "Settings File (*.pset)\0*.pset\0All Files (*.*)\0*.*\0\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = "pset";
  ofn.Flags = OFN_NONETWORKBUTTON | OFN_NOCHANGEDIR;
  
  return 0;
}

void saveAllSettings()
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH];
  char path[MAX_PATH];
  bool status;

  ZeroMemory(&ofn, sizeof(ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = "Settings File (*.mpset)\0*.mpset\0All Files (*.*)\0*.*\0\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = "pset";
  ofn.Flags = OFN_NONETWORKBUTTON | OFN_NOCHANGEDIR;
  
  
  if (GetSaveFileName(&ofn) != 0)
  {
	bool exists = false;
	std::ifstream testFile(szFileName);
	if (testFile.is_open()) 
	{
		exists = true;
		if (MessageBox(NULL,"Overwrite File?","Save",MB_YESNO|MB_ICONQUESTION) == IDNO) return;
	}

	std::ofstream oFile(szFileName,std::ios::out|std::ios::binary);
	int activeHold = activePS;
	status = true;
	for (int i=0;i<MAX_SYSTEMS;i++)
	{
		activePS = i;
		swapSystems(activePS,activePS);
		if (saveStateToFile(szFileName,oFile)) status = true;
		oFile.write((char*) &systems[i].active , sizeof(systems[i].active));
		oFile.write((char*) &systems[i].x , sizeof(systems[i].x));
		oFile.write((char*) &systems[i].y , sizeof(systems[i].y));
		oFile.write((char*) &systems[i].z , sizeof(systems[i].z));
		oFile.write((char*) &systems[i].sort , sizeof(systems[i].sort));
	}
	if (oFile.is_open()) oFile.close();

	// reset active system
	activePS = activeHold;
	swapSystems(activePS,activePS);

  } 
}

void saveSettings()
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH];
  char path[MAX_PATH];
  bool status;

  ZeroMemory(&ofn, sizeof(ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = "Settings File (*.pset)\0*.pset\0All Files (*.*)\0*.*\0\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = "pset";
  ofn.Flags = OFN_NONETWORKBUTTON | OFN_NOCHANGEDIR;
  
  
  if (GetSaveFileName(&ofn) != 0)
  {
	bool exists = false;
	std::ifstream testFile(szFileName);
	if (testFile.is_open()) 
	{
		exists = true;
		if (MessageBox(NULL,"Overwrite File?","Save",MB_YESNO|MB_ICONQUESTION) == IDNO) return;
	}


	std::ofstream oFile(szFileName,std::ios::out|std::ios::binary);
	if (saveStateToFile(szFileName,oFile)) status = true;
	else status = false;
	if (oFile.is_open()) oFile.close();
	if (status) MessageBox(NULL, "Save Sucessfull", "Saving...", MB_OK);
	else MessageBox(NULL, "Error During Save", "Saving...", MB_OK);
  }  
}

bool loadStateFromFile(char *filename, particleSystem &ps,std::ifstream &iFile)
{
  //std::ifstream iFile(filename, std::ios::out | std::ios::binary);
  if (iFile.is_open())
  {
	
	char buffer[512];
	int value;
	bool status = true;
	
	
	//iFile.seekg(0,std::ios::beg);
	
	iFile.read( buffer, sizeof(txtBasePath.string) );
	if (!iFile.eof() ) // not reached end of file
	{ 
	  std::strncpy(txtBasePath.string,buffer,512);
	  if (buffer[0] != 0) foundBasePath = true;
	  else foundBasePath = false;
	}
	else status = false; // Error reding from File
	
	
	if (status) //next
	{
	  iFile.read( buffer, sizeof(txtTextureName.string) );
	  if (!iFile.eof() )
	  {
		std::strncpy(txtTextureName.string,buffer,512);
		textureNameBoxChanged(buffer);
		if (buffer[0] != 0) foundTexturePath = true;
		else foundTexturePath = false;
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) buffer , sizeof(txtShaderName.string) );
	  if (!iFile.eof() )
	  {
		std::strncpy(txtShaderName.string,buffer,512);
		shaderNameBoxChanged(buffer);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderStartAngle.setValue_nochange(value);
		systems[activePS].ps.setStartAngle(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderEndAngle.setValue_nochange(value);
		systems[activePS].ps.setEndAngle(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		slider1.setValue_nochange(value);
	  }
	  else status = false;
	}

	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		slider2.setValue_nochange(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderZoom.setValue_nochange(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderNP.setValue_nochange(value);
		systems[activePS].ps.numParticles = value;
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderPSize.setValue_nochange(value);
		systems[activePS].ps.setSize((float)value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderHeight.setValue_nochange(value);
		systems[activePS].ps.setHeight((float)value);
	  }
	  else status = false;
	}    
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderHeightVar.setValue_nochange(value);
		systems[activePS].ps.setHeightVar(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderEmitterZ.setValue_nochange(value);
		systems[activePS].ps.setEmitterZ((float) value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderFreq.setValue_nochange(value);
		systems[activePS].ps.setFrequency(((float) value) / 10000);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderPG.setValue_nochange(value);
		systems[activePS].ps.setPhaseGrouping(((float)value)/10000);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderMasterPhase.setValue_nochange(value);
		systems[activePS].ps.setMasterPhase(((float) value )/10000);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		listZWave.setSelected(value);
		systems[activePS].ps.setZWaveform(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderRadius.setValue_nochange(value);
		systems[activePS].ps.setRadius(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderRadiusVar.setValue_nochange(value);
		systems[activePS].ps.setRadiusVar(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderEmitterXY.setValue_nochange(value);
		systems[activePS].ps.setEmitterXY(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderAngleGrouping.setValue_nochange(value);
		systems[activePS].ps.setAngleGrouping(((float)value)/10000);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		listXYWave.setSelected(value);
		systems[activePS].ps.setXYWaveform(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderXYPhase.setValue_nochange(value);
		systems[activePS].ps.setXYPhase(((float)value)/10000);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderRGB1.setValue_nochange(value);
		systems[activePS].ps.setRGB1(((float)value)/10000);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderRGB2.setValue_nochange(value);
		systems[activePS].ps.setRGB2(((float)value)/10000);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		listRGBWave.setSelected(value);
		systems[activePS].ps.setRGBWave(value);
	  }
	  else status = false;
	}
	
	 if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderRGBPhase.setValue_nochange(value);
		systems[activePS].ps.setRGBPhase(((float)value)/10000);
	  }
	  else status = false;
	}
	
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderAlpha1.setValue_nochange(value);
		systems[activePS].ps.setAlpha1(((float)value)/10000);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderAlpha2.setValue_nochange(value);
		systems[activePS].ps.setAlpha2(((float)value)/10000);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		listAlphaWave.setSelected(value);
		systems[activePS].ps.setAlphaWave(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderAlphaPhase.setValue_nochange(value);
		systems[activePS].ps.setAlphaPhase(((float)value)/10000);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		listSrcBlend.setSelected(value);
		if (value==0) systems[activePS].ps.setSrcBlend(GL_ONE); 
		if (value==1) systems[activePS].ps.setSrcBlend(GL_ZERO); 
		if (value==2) systems[activePS].ps.setSrcBlend(GL_DST_COLOR); 
		if (value==3) systems[activePS].ps.setSrcBlend(GL_ONE_MINUS_DST_COLOR); 
		if (value==4) systems[activePS].ps.setSrcBlend(GL_SRC_ALPHA); 
		if (value==5) systems[activePS].ps.setSrcBlend(GL_ONE_MINUS_SRC_ALPHA);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		listDstBlend.setSelected(value);
		if (value==0) systems[activePS].ps.setDstBlend(GL_ONE); 
		if (value==1) systems[activePS].ps.setDstBlend(GL_ZERO); 
		if (value==2) systems[activePS].ps.setDstBlend(GL_SRC_COLOR); 
		if (value==3) systems[activePS].ps.setDstBlend(GL_ONE_MINUS_SRC_COLOR); 
		if (value==4) systems[activePS].ps.setDstBlend(GL_SRC_ALPHA); 
		if (value==5) systems[activePS].ps.setDstBlend(GL_ONE_MINUS_SRC_ALPHA); 
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderStretch1.setValue_nochange(value);
		systems[activePS].ps.setStretch1(((float)value)/10000);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderStretch2.setValue_nochange(value);
		systems[activePS].ps.setStretch2(((float)value)/10000);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		listStretchWave.setSelected(value);
		systems[activePS].ps.setStretchWave(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderStretchPhase.setValue_nochange(value);
		systems[activePS].ps.setStretchPhase(((float)value)/10000);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderRotSpeed.setValue_nochange(value);
		systems[activePS].ps.setRotSpeed(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &value , sizeof(int) );
	  if (!iFile.eof() )
	  {
		sliderRotSpeedVar.setValue_nochange(value);
		systems[activePS].ps.setRotSpeedVar(value);
	  }
	  else status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &bgred , sizeof(float) );
	  if (iFile.eof() ) status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &bggreen , sizeof(float) );
	  if (iFile.eof() ) status = false;
	}
	
	if (status)
	{
	  iFile.read( (char*) &bgblue , sizeof(float) );
	  if (iFile.eof() ) status = false;
	}
	
	if (status)
	{
	  bool fire;
	  iFile.read( (char*) &fire, sizeof(bool) );
	  if (iFile.eof() ) status = false;
	  else
	  {
		systems[activePS].ps.setFireworkMode(!fire);
		toggleFirework();
	  }
	}

	if (status)
	{
		iFile.read( (char*) &value, sizeof(value));
		systems[activePS].ps.zBase = ((float)value)/10000;
		if (iFile.eof() ) status = false;
	}
	
	if (status)
	{
		iFile.read( (char*) &value, sizeof(value));
		systems[activePS].ps.rotx = ((float)value);
		if (iFile.eof() ) status = false;
	}
	if (status)
	{
		iFile.read( (char*) &value, sizeof(value));
		systems[activePS].ps.roty = ((float)value);
		if (iFile.eof() ) status = false;
	}
	if (status)
	{
		iFile.read( (char*) &value, sizeof(value));
		systems[activePS].ps.rotz = ((float)value);
		if (iFile.eof() ) status = false;
	}

	systems[activePS].ps.buildParticles();
	return status;
  }
  return false;
}

void updateSystemsTabUI()
{
	for (int i=0;i<MAX_SYSTEMS;i++)
	{
		sorts[i].setValue_nochange(systems[i].sort);
		posXs[i].setValue_nochange(systems[i].x   );
		posYs[i].setValue_nochange(systems[i].y   );
		posXs[i].setValue_nochange(systems[i].z   );
		rotXs[i].setValue_nochange(systems[i].ps.rotx);
		rotYs[i].setValue_nochange(systems[i].ps.roty);
		rotZs[i].setValue_nochange(systems[i].ps.rotz);
		if(systems[i].active)
		{
			buttons[i].setColor(SYS_BUT_ACTIVE,BUTTON_COLOR);
			buttons[i].setColor(SYS_BUT_CURRENT,BUTTON_COLOR_ACTIVE);
			buttons[i].setColor(SYS_BUT_CURRENT,BUTTON_COLOR_HOVERACTIVE);
		}
		else
		{			
			buttons[i].setColor(SYS_BUT_DISABLE,BUTTON_COLOR);				
			buttons[i].setColor(SYS_BUT_ACTIVE,BUTTON_COLOR_ACTIVE);		
			buttons[i].setColor(SYS_BUT_ACTIVE,BUTTON_COLOR_HOVERACTIVE);	
		}
	}
	
	buttons[activePS].setColor(SYS_BUT_CURRENT,BUTTON_COLOR);
	buttons[activePS].setColor(SYS_BUT_DELETE,BUTTON_COLOR_ACTIVE);
	buttons[activePS].setColor(SYS_BUT_DELETEL,BUTTON_COLOR_HOVERACTIVE);	
}


void loadSettings()
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH];
  //char path[MAX_PATH];
  bool status;

  ZeroMemory(&ofn, sizeof(ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = "Settings File (*.pset)\0*.pset\0All Files (*.*)\0*.*\0\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = "pset";
  ofn.Flags = OFN_NONETWORKBUTTON | OFN_NOCHANGEDIR;
  
  
  if (GetOpenFileName(&ofn) != 0)
  {
	std::ifstream iFile(szFileName, std::ios::out | std::ios::binary);
	if (loadStateFromFile(szFileName,systems[activePS].ps,iFile)) status = true;
	else status = false;
	if (!status) MessageBox(NULL, "End of file reached before loading completed, some settings may be incorrect", "Loading...", MB_OK);
	if (iFile.is_open()) iFile.close();
	//else MessageBox(NULL, "Error During Save", "Saving...", MB_OK);
	updateSystemsTabUI();
  }
}

void loadAllSettings()
{
	OPENFILENAME ofn;
	char szFileName[MAX_PATH];
	//char path[MAX_PATH];
	bool status;

	ZeroMemory(&ofn, sizeof(ofn));
	szFileName[0] = 0;

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "Multi-Settings File (*.mpset)\0*.mpset\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = "pset";
	ofn.Flags = OFN_NONETWORKBUTTON | OFN_NOCHANGEDIR;
  
  
	if (GetOpenFileName(&ofn) != 0)
	{
	std::ifstream iFile(szFileName, std::ios::out | std::ios::binary);
	status = true;
	int activeHold = activePS;
	for (int i=0;i<MAX_SYSTEMS;i++) 
	{
		// Can't use this as it updates slider values too 
		// - have to change active PS each loop and reset when load finishes
		// Set active = i
		// at end run swapsystems(0,activePS);
		// set active to system 0 at completing + swap active
		activePS = i;
		if (!loadStateFromFile(szFileName,systems[i].ps,iFile)) status = false;
		if(status == false) break;
		bool active = false;
		iFile.read((char*) &active , sizeof(bool));
		iFile.read((char*) &systems[i].x , sizeof(systems[i].x));
		iFile.read((char*) &systems[i].y , sizeof(systems[i].y));
		iFile.read((char*) &systems[i].z , sizeof(systems[i].z));
		iFile.read((char*) &systems[i].sort , sizeof(systems[i].sort));		
		systems[i].active = active;
	}

	// Set acitvePS to first active system
	for (int i=0;i<MAX_SYSTEMS;i++)
	{
		if (systems[i].active) 
		{
			activePS = i;
			swapSystems(activePS,0);
			break;
		}
	}
	if (status == false)
	{
		MessageBox(NULL, "End of file reached before loading completed, some settings may be incorrect", "Loading...", MB_OK);
	}
	if (iFile.is_open()) iFile.close();

	updateSystemsTabUI();
  }
}

/*int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int iCmdShow)*/
int WINAPI  WinMain (
	_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nShowCmd
	)
{
				   
	//std::ofstream oFile("ps_log.txt",std::ios::app);
	//oFile.close();
	WNDCLASS wc;
	//HWND hWnd;
	HDC hDC;
	HGLRC hRC;    
	MSG msg;
	BOOL bQuit = FALSE;
	pos.x = 200;
	pos.y = 200;
  
	GetCurrentDirectory(MAX_PATH, workingDir);
  
	// register window class
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "GLWindow";
	RegisterClass( &wc );

	// create main window
	hWnd = CreateWindow( 
	"GLWindow", "FS Particle Studio", 
	WS_MINIMIZEBOX | /*WS_MAXIMIZEBOX |*/ WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
	100, 100, WN_WIDTH-1,WN_HEIGHT-1,
	NULL, NULL, hInstance, NULL);
  
	//MessageBox (NULL, "About..." , "Windows example version 0.01", 1);
  
	// enable OpenGL for the window
	EnableOpenGL( hWnd, &hDC, &hRC );
	/*
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	*/

	activePS = 0;  // Select the first particle system
	systems[activePS].active = true;
	paused = false;
	minimized = false;
  
	// Load Textures
  
	loadGLTextures();

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  
	bgred = 0.0f;
	bggreen = 0.0f;
	bgblue = 0.0f;
  
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// Black Background
	gluOrtho2D(0,GL_WIDTH-1,0,GL_HEIGHT-1);
	glClearDepth(1.0f);									// Depth Buffer Setup
  
	GetClientRect(hWnd, &rc);
  
	mouse.x = rc.right;
	mouse.y = rc.bottom;

	for (auto &s : systems)
	{
		s.ps.buildParticles();
	}
  
	/*systems[0].ps.buildParticles();
	systems[1].ps.buildParticles();
	systems[2].ps.buildParticles();
	systems[3].ps.buildParticles();*/
  
	drawTime.location.x = 20;
	drawTime.location.y = 450;
	drawTime.size.x = 50;
	drawTime.size.y = 15;
	drawTime.txtSize = 15;
	drawTime.alignment = ALIGN_LEFT;
	drawTime.setString("");
	drawTime.maxSize = 200;
  
	label drawFPS;
	drawFPS.location.x = 620;
	drawFPS.location.y = 450;
	drawFPS.txtSize = 15;
	drawFPS.alignment = ALIGN_RIGHT;
	drawFPS.setString("");
	drawFPS.maxSize = 200;
  
	tab tabFile;
	entities.bindEnt(&tabFile);  
	tabFile.location.x = 25;
	tabFile.location.y = 0;
	tabFile.size.x = 105;
	tabFile.size.y = 15;
	tabFile.setOnClick(tabClicked);
	tabFile.l_name.location.x = tabFile.location.x + 10;
	tabFile.l_name.location.y = tabFile.location.y;
	tabFile.l_name.txtSize = tabFile.size.y;
	tabFile.l_name.maxSize = tabFile.size.x - 20;
	tabFile.l_name.setString("File");
  
	tab tabDyn1;
	entities.bindEnt(&tabDyn1);  
	tabDyn1.location.x = 120;
	tabDyn1.location.y = 0;
	tabDyn1.size.x = 105;
	tabDyn1.size.y = 15;
	tabDyn1.setOnClick(tabClicked);
	//tabDyn1.setColor(0.7,0.4,0.3,TAB_COLOR_ACTIVE);
	//tabDyn1.setColor(0.5,0.5,0.5,TAB_COLOR);
	tabDyn1.l_name.location.x = tabDyn1.location.x + 10;
	tabDyn1.l_name.location.y = tabDyn1.location.y;
	tabDyn1.l_name.txtSize = tabDyn1.size.y;
	tabDyn1.l_name.maxSize = tabDyn1.size.x - 20;
	tabDyn1.l_name.setString("Dynamics 1");
  
	tab tabDyn2;
	entities.bindEnt(&tabDyn2);
	tabDyn2.location.x = 215;
	tabDyn2.location.y = 0;
	tabDyn2.size.x = 105;
	tabDyn2.size.y = 15;
	tabDyn2.setOnClick(tabClicked);
	//tabDyn2.setColor(0.5,0.7,0.3,TAB_COLOR_ACTIVE);
	//tabDyn2.setColor(0.5,0.5,0.5,TAB_COLOR);
	tabDyn2.l_name.location.x = tabDyn2.location.x + 10;
	tabDyn2.l_name.location.y = tabDyn2.location.y;
	tabDyn2.l_name.txtSize = tabDyn2.size.y;
	tabDyn2.l_name.maxSize = tabDyn2.size.x - 20;
	tabDyn2.l_name.setString("Dynamics 2");
  
	tab tabBlend;
	entities.bindEnt(&tabBlend);
	tabBlend.location.x = 310;
	tabBlend.location.y = 0;
	tabBlend.size.x = 105;
	tabBlend.size.y = 15;
	tabBlend.setOnClick(tabClicked);
	//tabBlend.setColor(0.5,0.7,0.3,TAB_COLOR_ACTIVE);
	//tabBlend.setColor(0.5,0.5,0.5,TAB_COLOR);
	tabBlend.l_name.location.x = tabBlend.location.x + 10;
	tabBlend.l_name.location.y = tabBlend.location.y;
	tabBlend.l_name.txtSize = tabBlend.size.y;
	tabBlend.l_name.maxSize = tabBlend.size.x - 20;
	tabBlend.l_name.setString("Blending");
  
	tab tabTcMod;
	entities.bindEnt(&tabTcMod);
	tabTcMod.location.x = 405;
	tabTcMod.location.y = 0;
	tabTcMod.size.x = 105;
	tabTcMod.size.y = 15;
	tabTcMod.setOnClick(tabClicked);
	//tabExp.setColor(0.5,0.7,0.3,TAB_COLOR_ACTIVE);
	//tabTcMod.setColor(0.4,0.4,0.4,TAB_COLOR);
	tabTcMod.l_name.location.x = tabTcMod.location.x + 10;
	tabTcMod.l_name.location.y = tabTcMod.location.y;
	tabTcMod.l_name.txtSize = tabTcMod.size.y;
	tabTcMod.l_name.maxSize = tabTcMod.size.x - 20;
	tabTcMod.l_name.setString(" tcMod");
  
	tab tabExp;
	entities.bindEnt(&tabExp);
	tabExp.location.x = 500;
	tabExp.location.y = 0;
	tabExp.size.x = 105;
	tabExp.size.y = 15;
	tabExp.setOnClick(runExport);
	//tabExp.setColor(0.5,0.7,0.3,TAB_COLOR_ACTIVE);
	tabExp.setColor(0.4,0.4,0.4,TAB_COLOR);
	tabExp.l_name.location.x = tabExp.location.x + 10;
	tabExp.l_name.location.y = tabExp.location.y;
	tabExp.l_name.txtSize = tabExp.size.y;
	tabExp.l_name.maxSize = tabExp.size.x - 20;
	tabExp.l_name.setString("   Export   ");
  
	slider1.location.x = 20;
	slider1.location.y = 131;
	slider1.size.x = 600;
	slider1.size.y = 6;
	slider1.setSliderWidth(30);
	slider1.setBounds(-180,180);
	slider1.setValue(0);
	entities.bindEnt(&slider1);

  
	slider2.location.x = 20;
	slider2.location.y = 139;
	slider2.size.x = 600;
	slider2.size.y = 6;
	slider2.setSliderWidth(30);
	slider2.setBounds(-85,85);
	slider2.setValue(0);
	entities.bindEnt(&slider2);  
   
	sliderZoom.location.x = 20;
	sliderZoom.location.y = 147;
	sliderZoom.size.x = 600;
	sliderZoom.size.y = 6;
	sliderZoom.setSliderWidth(30);
	sliderZoom.setBounds(100,10000*3,10000);
	sliderZoom.setIntermediatePosition( sliderZoom.size.x / 2);
	sliderZoom.setValue(10000);  
	sliderZoom.setType(TYPE_FLOAT);
  
	entities.bindEnt(&sliderZoom);  
  
	//basePath[0] = basePath[1] = 0;
  
	txtBasePath.setOnChange(basePathBoxChanged);
	txtBasePath.l_name.setString("Base Path:");
	txtBasePath.useLabelName = true;
	txtBasePath.size.x = 193;
	txtBasePath.location.x = 125;
	txtBasePath.l_name.location.x = 125;
	tabFile.bindEnt(&txtBasePath);
	entities.bindEnt(&txtBasePath);
  
	selectBasePath.location.x = 325;
	selectBasePath.location.y = 100;
	selectBasePath.l_name.setString("Find");
	selectBasePath.l_name.alignment = ALIGN_LEFT;
	selectBasePath.useLabelName = true;
	selectBasePath.l_name.location.x = selectBasePath.location.x+selectBasePath.size.x;
	selectBasePath.l_name.location.y = selectBasePath.location.y;
	selectBasePath.l_name.txtSize = selectBasePath.size.y;
	selectBasePath.setOnClick(getBasePathDir);
  
	tabFile.bindEnt(&selectBasePath);
	entities.bindEnt(&selectBasePath); 
  
  
	txtShaderName.setOnChange(shaderNameBoxChanged);
	txtShaderName.l_name.setString("Shader Name: TEXTURES/");
	txtShaderName.useLabelName = true;
	txtShaderName.location.y = 75;
	txtShaderName.location.x = 203;
	txtShaderName.size.x = 115;
	txtShaderName.l_name.location.x = txtShaderName.location.x;
	txtShaderName.l_name.location.y = txtShaderName.location.y;
	txtShaderName.location.x = 202;
	txtShaderName.size.x = 116;
	tabFile.bindEnt(&txtShaderName);
	entities.bindEnt(&txtShaderName);
  
  
	txtTextureName.setOnChange(textureNameBoxChanged);
	txtTextureName.l_name.setString("Texture Name:");
	txtTextureName.useLabelName = true;
	txtTextureName.location.y = 50;
	txtTextureName.location.x = 125;
	txtTextureName.size.x = 193;
	txtTextureName.l_name.location.y = 50;
	txtTextureName.l_name.location.x = 125;
	tabFile.bindEnt(&txtTextureName);
	entities.bindEnt(&txtTextureName);
  
	
	selectTexture.location.x = 325;
	selectTexture.location.y = 50;
	selectTexture.l_name.setString("Find");
	selectTexture.l_name.alignment = ALIGN_LEFT;
	selectTexture.useLabelName = true;
	selectTexture.l_name.location.x = selectTexture.location.x+selectTexture.size.x;
	selectTexture.l_name.location.y = selectTexture.location.y;
	selectTexture.l_name.txtSize = selectTexture.size.y;
	selectTexture.setOnClick(getTexture);
  
	tabFile.bindEnt(&selectTexture);
	entities.bindEnt(&selectTexture);
  
	rgbPicker.location.x = 580;
	rgbPicker.location.y = 100;
	rgbPicker.l_name.setString("Background Colour");
	rgbPicker.useLabelName = true;
	rgbPicker.l_name.location.x = rgbPicker.location.x;
	rgbPicker.l_name.location.y = rgbPicker.location.y;
	rgbPicker.l_name.txtSize = rgbPicker.size.y;
	rgbPicker.setOnClick(colourPickerPressed);
  
	tabFile.bindEnt(&rgbPicker);
	entities.bindEnt(&rgbPicker);  

	lockSeed.location.x = 500;
	lockSeed.location.y =  50;
	lockSeed.l_name.setString("Lock Seed");
	lockSeed.useLabelName = true;
	lockSeed.l_name.location.x = lockSeed.location.x;
	lockSeed.l_name.location.y = lockSeed.location.y;
	lockSeed.l_name.txtSize = lockSeed.size.y;
	lockSeed.l_name.alignment = ALIGN_RIGHT;
	lockSeed.setOnClick(toggleSeed);

	tabFile.bindEnt(&lockSeed);
	entities.bindEnt(&lockSeed);

	runOnce.location.x = 500;
	runOnce.location.y =  25;
	runOnce.l_name.setString("Continuous");
	runOnce.useLabelName = true;
	runOnce.l_name.location.x = runOnce.location.x;
	runOnce.l_name.location.y = runOnce.location.y;
	runOnce.l_name.txtSize = runOnce.size.y;
	runOnce.l_name.alignment = ALIGN_RIGHT;
	runOnce.setOnClick(toggleOnce);

	tabFile.bindEnt(&runOnce);
	entities.bindEnt(&runOnce);
  
	butShowPlayer.location.x = 580;
	butShowPlayer.location.y = 75;
	butShowPlayer.l_name.setString("Show Player");
	butShowPlayer.useLabelName = true;
	butShowPlayer.l_name.location.x = butShowPlayer.location.x;
	butShowPlayer.l_name.location.y = butShowPlayer.location.y;
	butShowPlayer.l_name.txtSize = butShowPlayer.size.y;
	butShowPlayer.setOnClick(togglePlayerVis);
  
	tabFile.bindEnt(&butShowPlayer);
	entities.bindEnt(&butShowPlayer);
  
	firework.location.x = 315;
	firework.location.y = 23;
	firework.size.y = 10;
	firework.l_name.setString("Experimental Firework Mode: OFF");
	firework.useLabelName = true;
	firework.l_name.location.x = firework.location.x;
	firework.l_name.location.y = firework.location.y;
	firework.l_name.txtSize = firework.size.y;
	firework.setOnClick(toggleFirework);
  
	tabFile.bindEnt(&firework);
	entities.bindEnt(&firework);
  
	butLoad.location.x = 580;
	butLoad.location.y = 50;
	butLoad.l_name.setString("Load");
	butLoad.useLabelName = true;
	butLoad.l_name.location.x = butLoad.location.x;
	butLoad.l_name.location.y = butLoad.location.y;
	butLoad.l_name.txtSize = butLoad.size.y;
	butLoad.setOnClick(loadSettings);
  
	tabFile.bindEnt(&butLoad);
	entities.bindEnt(&butLoad);
  
	butSave.location.x = 580;
	butSave.location.y = 25;
	butSave.l_name.setString("Save");
	butSave.useLabelName = true;
	butSave.l_name.location.x = butSave.location.x;
	butSave.l_name.location.y = butSave.location.y;
	butSave.l_name.txtSize = butSave.size.y;
	butSave.setOnClick(saveSettings);
  
	tabFile.bindEnt(&butSave);
	entities.bindEnt(&butSave);

	// ALL

	butLoadAll.location.x = 598;
	butLoadAll.location.y = 50;
	butLoadAll.l_name.setString("All");
	butLoadAll.useLabelName = true;
	butLoadAll.l_name.alignment = ALIGN_LEFT;
	butLoadAll.l_name.location.x = butLoadAll.location.x + butLoadAll.size.x;
	butLoadAll.l_name.location.y = butLoadAll.location.y;
	butLoadAll.l_name.txtSize = butLoadAll.size.y;
	butLoadAll.setOnClick(loadAllSettings);
  
	tabFile.bindEnt(&butLoadAll);
	entities.bindEnt(&butLoadAll);
  
	butSaveAll.location.x = 598;
	butSaveAll.location.y = 25;
	butSaveAll.l_name.setString("All");
	butSaveAll.useLabelName = true;
	butSaveAll.l_name.alignment = ALIGN_LEFT;
	butSaveAll.l_name.location.x = butSaveAll.location.x + butSaveAll.size.x;
	butSaveAll.l_name.location.y = butSaveAll.location.y;
	butSaveAll.l_name.txtSize = butSaveAll.size.y;
	butSaveAll.setOnClick(saveAllSettings);
  
	tabFile.bindEnt(&butSaveAll);
	entities.bindEnt(&butSaveAll);
  
	// Dynamics #1

	// Number of Particles
  
  
	sliderNP.location.x = 100;
	sliderNP.location.y = 100;
	sliderNP.size.x = 200;
	sliderNP.setBounds(1,1000);
	sliderNP.setValue(systems[activePS].ps.numParticles);
	sliderNP.setOnChange( oc1 );
	sliderNP.setLabelName("# Particles");
  
	sliderNP.l_name.location.x = sliderNP.location.x;
	sliderNP.l_name.location.y = sliderNP.location.y;
	sliderNP.l_name.txtSize = sliderNP.size.y;
	sliderNP.useLabelName = true;
	
	sliderNP.l_value.location.x = sliderNP.location.x+sliderNP.size.x;
	sliderNP.l_value.location.y = sliderNP.location.y;
	sliderNP.l_value.txtSize = sliderNP.size.y;
	sliderNP.useLabelValue = true;  
  
	sliderNP.setType(TYPE_INT);
   
	currentTab = tabFile.setActive(true);
  
	tabDyn1.bindEnt(&sliderNP);
  
	// Particle Size
  
	sliderPSize.location.x = 375;
	sliderPSize.location.y = 25;
	sliderPSize.size.x = 200;
	sliderPSize.setBounds(1,128);
	sliderPSize.setValue( (int) systems[activePS].ps.getSize());
	sliderPSize.setOnChange(pSizeChange);
	sliderPSize.setLabelName("Size");
  
	sliderPSize.l_name.location.x = sliderPSize.location.x;
	sliderPSize.l_name.location.y = sliderPSize.location.y;
	sliderPSize.l_name.txtSize = sliderPSize.size.y;
	sliderPSize.useLabelName = true;
  
	sliderPSize.l_value.location.x = sliderPSize.location.x + sliderPSize.size.x;
	sliderPSize.l_value.location.y = sliderPSize.location.y;
	sliderPSize.l_value.txtSize = sliderPSize.size.y;
	sliderPSize.useLabelValue = true;
  
	tabDyn1.bindEnt(&sliderPSize);
  
	// Height
  
	sliderHeight.location.x = 70;
	sliderHeight.location.y = 75;
	sliderHeight.size.x = 140;
	sliderHeight.setBounds(0,1024);
	sliderHeight.setValue((int)systems[activePS].ps.getHeight());
	sliderHeight.setOnChange(heightChange);
	sliderHeight.setLabelName("Height");
  
	sliderHeight.l_name.location.x = sliderHeight.location.x;
	sliderHeight.l_name.location.y = sliderHeight.location.y;
	sliderHeight.l_name.txtSize = sliderHeight.size.y;
	sliderHeight.useLabelName = true;
  
	sliderHeight.l_value.location.x = sliderHeight.location.x + sliderHeight.size.x;
	sliderHeight.l_value.location.y = sliderHeight.location.y;
	sliderHeight.l_value.txtSize = sliderHeight.size.y;
	sliderHeight.useLabelValue = true;
  
	tabDyn1.bindEnt(&sliderHeight);
  
	// Height Variance 
  
  
  
	sliderHeightVar.location.x = 310;
	sliderHeightVar.location.y = 75;
	sliderHeightVar.size.x = 75;
	sliderHeightVar.setBounds(0,1024);
	sliderHeightVar.setValue(systems[activePS].ps.getHeightVar());
	sliderHeightVar.setOnChange(heightVarChange);
	sliderHeightVar.setLabelName("Var");
  
	sliderHeightVar.l_name.location.x = sliderHeightVar.location.x;
	sliderHeightVar.l_name.location.y = sliderHeightVar.location.y;
	sliderHeightVar.l_name.txtSize = sliderHeightVar.size.y;
	sliderHeightVar.useLabelName = true;
  
	sliderHeightVar.l_value.location.x = sliderHeightVar.location.x + sliderHeightVar.size.x;
	sliderHeightVar.l_value.location.y = sliderHeightVar.location.y;
	sliderHeightVar.l_value.txtSize = sliderHeightVar.size.y;
	sliderHeightVar.useLabelValue = true;
  
	//sliderHeightVar.setType(TYPE_FLOAT);
  
	tabDyn1.bindEnt(&sliderHeightVar);
  
	// Emitter Variance
  
  
  
	sliderEmitterZ.location.x = 500;
	sliderEmitterZ.location.y = 75;
	sliderEmitterZ.size.x = 75;
	sliderEmitterZ.setBounds(0,128);
	sliderEmitterZ.setValue((int)(systems[activePS].ps.getEmitterZ()));
	sliderEmitterZ.setOnChange(emitterZChange);
	sliderEmitterZ.setLabelName("Base Var");
  
	sliderEmitterZ.l_name.location.x = sliderEmitterZ.location.x;
	sliderEmitterZ.l_name.location.y = sliderEmitterZ.location.y;
	sliderEmitterZ.l_name.txtSize = sliderEmitterZ.size.y;
	sliderEmitterZ.useLabelName = true;
  
	sliderEmitterZ.l_value.location.x = sliderEmitterZ.location.x + sliderEmitterZ.size.x;
	sliderEmitterZ.l_value.location.y = sliderEmitterZ.location.y;
	sliderEmitterZ.l_value.txtSize = sliderEmitterZ.size.y;
	sliderEmitterZ.useLabelValue = true;
  
	//sliderEmitterZ.setType(TYPE_FLOAT);
  
	tabDyn1.bindEnt(&sliderEmitterZ);
  
	// Natual Height Button
  
  
	butNatH.location.x = 250;
	butNatH.location.y = 75;
	butNatH.l_name.setString("Natural");
	butNatH.useLabelName = true;
	butNatH.l_name.location.x = butNatH.location.x + butNatH.size.x + 30;
	butNatH.l_name.location.y = butNatH.location.y + butNatH.size.y;
	butNatH.l_name.txtSize = butNatH.size.y;
	butNatH.setOnClick(naturalHeight);
  
	tabDyn1.bindEnt(&butNatH);
  
	// Frequency Slider
  
  
  
	sliderFreq.location.x = 80;
	sliderFreq.location.y = 50;
	sliderFreq.size.x = 110;
	sliderFreq.setBounds(100,10000*5,10000);
	sliderFreq.setIntermediatePosition( sliderFreq.size.x / 2);
	//sliderFreq.intermediateValue
	sliderFreq.setValue((int)(10000*systems[activePS].ps.getFrequency()));
	sliderFreq.setOnChange(frequencyChange);
	sliderFreq.setLabelName("Frequency");
  
	sliderFreq.l_name.location.x = sliderFreq.location.x;
	sliderFreq.l_name.location.y = sliderFreq.location.y;
	sliderFreq.l_name.txtSize = sliderFreq.size.y;
	sliderFreq.useLabelName = true;
  
	sliderFreq.l_value.location.x = sliderFreq.location.x + sliderFreq.size.x;
	sliderFreq.l_value.location.y = sliderFreq.location.y;
	sliderFreq.l_value.txtSize = sliderFreq.size.y;
	sliderFreq.useLabelValue = true;  
  
	sliderFreq.setType(TYPE_FLOAT);
  
	tabDyn1.bindEnt(&sliderFreq);
  
	// Natual Freq Button
  
  
	butNatF.location.x = 250;
	butNatF.location.y = 50;
	butNatF.l_name.setString("Natural");
	butNatF.useLabelName = true;
	butNatF.l_name.location.x = butNatF.location.x + butNatF.size.x + 30;
	butNatF.l_name.location.y = butNatF.location.y + butNatF.size.y;
	butNatF.l_name.txtSize = butNatF.size.y;
	butNatF.setOnClick(naturalFreq);
  
	tabDyn1.bindEnt(&butNatF);
  
	// Phase Grouping Slider
  
  
  
	sliderPG.location.x = 400;
	sliderPG.location.y = 50;
	sliderPG.size.x = 65;
	sliderPG.setBounds(0,10000*5,10000);
	sliderPG.setIntermediatePosition( sliderPG.size.x / 2);
	sliderPG.setValue((int)(10000*systems[activePS].ps.getPhaseGrouping()));
	sliderPG.setOnChange(phaseGChange);
	sliderPG.setLabelName("Phase Grouping");
  
	sliderPG.l_name.location.x = sliderPG.location.x;
	sliderPG.l_name.location.y = sliderPG.location.y;
	sliderPG.l_name.txtSize = sliderPG.size.y;
	sliderPG.useLabelName = true;
  
	sliderPG.l_value.location.x = sliderPG.location.x + sliderPG.size.x;
	sliderPG.l_value.location.y = sliderPG.location.y;
	sliderPG.l_value.txtSize = sliderPG.size.y;
	sliderPG.useLabelValue = true;  
  
	sliderPG.setType(TYPE_FLOAT);
  
	tabDyn1.bindEnt(&sliderPG);

	sliderZBase.location.x = 560;
	sliderZBase.location.y = 50;
	sliderZBase.size.x = 30;
	sliderZBase.setBounds(-10000,10000,0);
	sliderZBase.setIntermediatePosition(sliderZBase.size.x/2);
	sliderZBase.setValue((int)(10000*systems[activePS].ps.zBase));
	sliderZBase.setOnChange(zBaseChange);
	sliderZBase.setLabelName("zBase");

	sliderZBase.l_name.location.x = sliderZBase.location.x;
	sliderZBase.l_name.location.y = sliderZBase.location.y;
	sliderZBase.l_name.txtSize = sliderZBase.size.y;
	sliderZBase.useLabelName = true;

	sliderZBase.l_value.location.x = sliderZBase.location.x + sliderZBase.size.x;
	sliderZBase.l_value.location.y = sliderZBase.location.y;
	sliderZBase.l_value.txtSize = sliderZBase.size.y;
	sliderZBase.useLabelValue = true; 

	sliderZBase.setType(TYPE_FLOAT);
	tabDyn1.bindEnt(&sliderZBase);

  
	// Master Phase Slider
  
  
  
	sliderMasterPhase.location.x = 100;
	sliderMasterPhase.location.y = 25;
	sliderMasterPhase.size.x = 150;
	sliderMasterPhase.setBounds(0,10000);
	//sliderMasterPhase.setIntermediatePosition( sliderPG.size.x / 2);
	sliderMasterPhase.setValue((int)(10000*systems[activePS].ps.getMasterPhase()));
	sliderMasterPhase.setOnChange(masterPhaseChange);
	sliderMasterPhase.setLabelName("Master Phase");
  
	sliderMasterPhase.l_name.location.x = sliderMasterPhase.location.x;
	sliderMasterPhase.l_name.location.y = sliderMasterPhase.location.y;
	sliderMasterPhase.l_name.txtSize = sliderMasterPhase.size.y;
	sliderMasterPhase.useLabelName = true;
  
	sliderMasterPhase.l_value.location.x = sliderMasterPhase.location.x + sliderMasterPhase.size.x;
	sliderMasterPhase.l_value.location.y = sliderMasterPhase.location.y;
	sliderMasterPhase.l_value.txtSize = sliderMasterPhase.size.y;
	sliderMasterPhase.useLabelValue = true;  
  
	sliderMasterPhase.setType(TYPE_FLOAT);
  
	tabDyn1.bindEnt(&sliderMasterPhase);
  
	// Z WaveForm List Box
  
	//listBox listZWave;
	listZWave.location.x = 420;
	listZWave.location.y = 100;
	listZWave.createItems(5);
	listZWave.fillItem(0,"SINE");
	listZWave.fillItem(1,"SQUARE");
	listZWave.fillItem(2,"TRIANGLE");
	listZWave.fillItem(3,"SAWTOOTH");
	listZWave.fillItem(4,"INVERSE SAWTOOTH");
	listZWave.setOnChange(zWaveChange);
	listZWave.setSelected(systems[activePS].ps.getZWaveform());
  
	listZWave.l_name.setString("Z Waveform");
	listZWave.useLabelName = true;
	listZWave.l_name.location.x = listZWave.location.x;
	listZWave.l_name.location.y = listZWave.location.y;
	listZWave.l_name.txtSize = listZWave.size.y;
  
	tabDyn1.bindEnt(&listZWave);
  
  
	// Dynamics 2 Tab
  
	// Radius
  
  
  
	sliderRadius.location.x = 70;
	sliderRadius.location.y = 100;
	sliderRadius.size.x = 140;
	sliderRadius.setBounds(0,1024);
	sliderRadius.setValue((int)systems[activePS].ps.getRadius());
	sliderRadius.setOnChange(radiusChange);
	sliderRadius.setLabelName("Radius");
  
	sliderRadius.l_name.location.x = sliderRadius.location.x;
	sliderRadius.l_name.location.y = sliderRadius.location.y;
	sliderRadius.l_name.txtSize = sliderRadius.size.y;
	sliderRadius.useLabelName = true;
  
	sliderRadius.l_value.location.x = sliderRadius.location.x + sliderRadius.size.x;
	sliderRadius.l_value.location.y = sliderRadius.location.y;
	sliderRadius.l_value.txtSize = sliderRadius.size.y;
	sliderRadius.useLabelValue = true;
  
	tabDyn2.bindEnt(&sliderRadius);
  
	// Radius Variance 
  
  
  
	sliderRadiusVar.location.x = 310;
	sliderRadiusVar.location.y = 100;
	sliderRadiusVar.size.x = 75;
	sliderRadiusVar.setBounds(0,1024);
	sliderRadiusVar.setValue(systems[activePS].ps.getRadiusVar());
	sliderRadiusVar.setOnChange( radiusVarChange);
	sliderRadiusVar.setLabelName("Var");
  
	sliderRadiusVar.l_name.location.x = sliderRadiusVar.location.x;
	sliderRadiusVar.l_name.location.y = sliderRadiusVar.location.y;
	sliderRadiusVar.l_name.txtSize = sliderRadiusVar.size.y;
	sliderRadiusVar.useLabelName = true;
  
	sliderRadiusVar.l_value.location.x = sliderRadiusVar.location.x + sliderRadiusVar.size.x;
	sliderRadiusVar.l_value.location.y = sliderRadiusVar.location.y;
	sliderRadiusVar.l_value.txtSize = sliderRadiusVar.size.y;
	sliderRadiusVar.useLabelValue = true;
  
	//sliderRadiusVar.setType(TYPE_FLOAT);
  
	tabDyn2.bindEnt(&sliderRadiusVar);
  
	// Emitter Variance
  
  
  
	sliderEmitterXY.location.x = 500;
	sliderEmitterXY.location.y = 100;
	sliderEmitterXY.size.x = 75;
	sliderEmitterXY.setBounds(0,128);
	sliderEmitterXY.setValue((int)(systems[activePS].ps.getEmitterXY()));
	sliderEmitterXY.setOnChange(emitterXYChange);
	sliderEmitterXY.setLabelName("Base Var");
  
	sliderEmitterXY.l_name.location.x = sliderEmitterXY.location.x;
	sliderEmitterXY.l_name.location.y = sliderEmitterXY.location.y;
	sliderEmitterXY.l_name.txtSize = sliderEmitterXY.size.y;
	sliderEmitterXY.useLabelName = true;
  
	sliderEmitterXY.l_value.location.x = sliderEmitterXY.location.x + sliderEmitterXY.size.x;
	sliderEmitterXY.l_value.location.y = sliderEmitterXY.location.y;
	sliderEmitterXY.l_value.txtSize = sliderEmitterXY.size.y;
	sliderEmitterXY.useLabelValue = true;
  
	//sliderEmitterZ.setType(TYPE_FLOAT);
  
	tabDyn2.bindEnt(&sliderEmitterXY);
  
	//sliderStartAngle
  
	sliderStartAngle.location.x = 92;
	sliderStartAngle.location.y = 33;
	sliderStartAngle.size.x = 110;
	sliderStartAngle.setBounds(0,358);
	sliderStartAngle.setValue((int)(systems[activePS].ps.getStartAngle()));
	sliderStartAngle.setOnChange(startAngleChange);
	sliderStartAngle.setLabelName("Start Angle");
  
	sliderStartAngle.l_name.location.x = sliderStartAngle.location.x;
	sliderStartAngle.l_name.location.y = sliderStartAngle.location.y;
	sliderStartAngle.l_name.txtSize = sliderStartAngle.size.y;
	sliderStartAngle.useLabelName = true;
  
	sliderStartAngle.l_value.location.x = sliderStartAngle.location.x + sliderStartAngle.size.x;
	sliderStartAngle.l_value.location.y = sliderStartAngle.location.y;
	sliderStartAngle.l_value.txtSize = sliderStartAngle.size.y;
	sliderStartAngle.useLabelValue = true;
  
	tabDyn2.bindEnt(&sliderStartAngle);
  
	//sliderEndAngle
  
	sliderEndAngle.location.x = 305;
	sliderEndAngle.location.y = 33;
	sliderEndAngle.size.x = 110;
	sliderEndAngle.setBounds(1,359);
	sliderEndAngle.setValue((int)(systems[activePS].ps.getEndAngle()));
	sliderEndAngle.setOnChange(endAngleChange);
	sliderEndAngle.setLabelName("End Angle");
  
	sliderEndAngle.l_name.location.x = sliderEndAngle.location.x;
	sliderEndAngle.l_name.location.y = sliderEndAngle.location.y;
	sliderEndAngle.l_name.txtSize = sliderEndAngle.size.y;
	sliderEndAngle.useLabelName = true;
  
	sliderEndAngle.l_value.location.x = sliderEndAngle.location.x + sliderEndAngle.size.x;
	sliderEndAngle.l_value.location.y = sliderEndAngle.location.y;
	sliderEndAngle.l_value.txtSize = sliderEndAngle.size.y;
	sliderEndAngle.useLabelValue = true;
  
	tabDyn2.bindEnt(&sliderEndAngle);
  
	// XY WAVEFORM
	// Radius Variance 
  
  
  
	sliderAngleGrouping.location.x = 515;
	sliderAngleGrouping.location.y = 33;
	sliderAngleGrouping.size.x = 70;
	sliderAngleGrouping.setBounds(0,10000);
	sliderAngleGrouping.setValue((int)(10000*systems[activePS].ps.getAngleGrouping()));
	sliderAngleGrouping.setOnChange(angleGroupingChanged);
	sliderAngleGrouping.setLabelName("Grouping");
  
	sliderAngleGrouping.l_name.location.x = sliderAngleGrouping.location.x;
	sliderAngleGrouping.l_name.location.y = sliderAngleGrouping.location.y;
	sliderAngleGrouping.l_name.txtSize = sliderAngleGrouping.size.y;
	sliderAngleGrouping.useLabelName = true;
  
	sliderAngleGrouping.l_value.location.x = sliderAngleGrouping.location.x + sliderAngleGrouping.size.x;
	sliderAngleGrouping.l_value.location.y = sliderAngleGrouping.location.y;
	sliderAngleGrouping.l_value.txtSize = sliderAngleGrouping.size.y;
	sliderAngleGrouping.useLabelValue = true;
  
	sliderAngleGrouping.setType(TYPE_FLOAT);
  
	tabDyn2.bindEnt(&sliderAngleGrouping);
  
  
	listXYWave.location.x = 100;
	listXYWave.location.y = 66;
	listXYWave.createItems(5);
	listXYWave.fillItem(0,"SINE");
	listXYWave.fillItem(1,"SQUARE");
	listXYWave.fillItem(2,"TRIANGLE");
	listXYWave.fillItem(3,"SAWTOOTH");
	listXYWave.fillItem(4,"INVERSE SAWTOOTH");
	listXYWave.setOnChange(xyWaveChange);
	listXYWave.setSelected(systems[activePS].ps.getXYWaveform());
  
	listXYWave.l_name.setString("X/Y Waveform");
	listXYWave.useLabelName = true;
	listXYWave.l_name.location.x = listXYWave.location.x;
	listXYWave.l_name.location.y = listXYWave.location.y;
	listXYWave.l_name.txtSize = 12;
  
	tabDyn2.bindEnt(&listXYWave);
  
  
  
	sliderXYPhase.location.x = 350;
	sliderXYPhase.location.y = 66;
	sliderXYPhase.size.x = 150;
	sliderXYPhase.setBounds(0,10000);
	//sliderMasterPhase.setIntermediatePosition( sliderPG.size.x / 2);
	sliderXYPhase.setValue((int)(10000*systems[activePS].ps.getMasterPhase()));
	sliderXYPhase.setOnChange(xyPhaseChange);
	sliderXYPhase.setLabelName("XY Phase");
  
	sliderXYPhase.l_name.location.x = sliderXYPhase.location.x;
	sliderXYPhase.l_name.location.y = sliderXYPhase.location.y;
	sliderXYPhase.l_name.txtSize = sliderXYPhase.size.y;
	sliderXYPhase.useLabelName = true;
  
	sliderXYPhase.l_value.location.x = sliderXYPhase.location.x + sliderXYPhase.size.x;
	sliderXYPhase.l_value.location.y = sliderXYPhase.location.y;
	sliderXYPhase.l_value.txtSize = sliderXYPhase.size.y;
	sliderXYPhase.useLabelValue = true;  
  
	sliderXYPhase.setType(TYPE_FLOAT);
  
	tabDyn2.bindEnt(&sliderXYPhase);
  
	// Blending Tab
  
	// RGBGEN 1 slider
  
  
  
	sliderRGB1.location.x = 41;
	sliderRGB1.location.y = 100;
	sliderRGB1.size.x = 150;
	sliderRGB1.setBounds(-50000,50000);
	//sliderMasterPhase.setIntermediatePosition( sliderPG.size.x / 2);
	sliderRGB1.setValue((int)(10000*systems[activePS].ps.getRGB1()));
	sliderRGB1.setOnChange(rgb1Change);
	sliderRGB1.setLabelName("RGB1");
  
	sliderRGB1.l_name.location.x = sliderRGB1.location.x;
	sliderRGB1.l_name.location.y = sliderRGB1.location.y;
	sliderRGB1.l_name.txtSize = sliderRGB1.size.y;
	sliderRGB1.useLabelName = true;
  
	sliderRGB1.l_value.location.x = sliderRGB1.location.x + sliderRGB1.size.x;
	sliderRGB1.l_value.location.y = sliderRGB1.location.y;
	sliderRGB1.l_value.txtSize = sliderRGB1.size.y;
	sliderRGB1.useLabelValue = true;  
  
	sliderRGB1.setType(TYPE_FLOAT);
  
	tabBlend.bindEnt(&sliderRGB1);

	// RGB 2
  
  
  
	sliderRGB2.location.x = 280;
	sliderRGB2.location.y = 100;
	sliderRGB2.size.x = 150;
	sliderRGB2.setBounds(-50000,50000);
	//sliderMasterPhase.setIntermediatePosition( sliderPG.size.x / 2);
	sliderRGB2.setValue((int)(10000*systems[activePS].ps.getRGB2()));
	sliderRGB2.setOnChange(rgb2Change);
	sliderRGB2.setLabelName("RGB2");
  
	sliderRGB2.l_name.location.x = sliderRGB2.location.x;
	sliderRGB2.l_name.location.y = sliderRGB2.location.y;
	sliderRGB2.l_name.txtSize = sliderRGB2.size.y;
	sliderRGB2.useLabelName = true;
  
	sliderRGB2.l_value.location.x = sliderRGB2.location.x + sliderRGB2.size.x;
	sliderRGB2.l_value.location.y = sliderRGB2.location.y;
	sliderRGB2.l_value.txtSize = sliderRGB2.size.y;
	sliderRGB2.useLabelValue = true;  
  
	sliderRGB2.setType(TYPE_FLOAT);
  
	tabBlend.bindEnt(&sliderRGB2);
  
	// rgb waveform type
  
  
	listRGBWave.location.x = 530;
	listRGBWave.location.y = 100;
	listRGBWave.size.x = 85;
	listRGBWave.createItems(6);
	listRGBWave.fillItem(0,"SINE");
	listRGBWave.fillItem(1,"SQUARE");
	listRGBWave.fillItem(2,"TRIANGLE");
	listRGBWave.fillItem(3,"SAWTOOTH");
	listRGBWave.fillItem(4,"INVERSE SAWTOOTH");
	listRGBWave.fillItem(5,"CONST");
	listRGBWave.setOnChange(rgbWaveChange);
	listRGBWave.setSelected(systems[activePS].ps.getRGBWave());
  
	listRGBWave.l_name.setString("Wave");
	listRGBWave.useLabelName = true;
	listRGBWave.l_name.location.x = listRGBWave.location.x;
	listRGBWave.l_name.location.y = listRGBWave.location.y;
	listRGBWave.l_name.txtSize = 12;
  
	tabBlend.bindEnt(&listRGBWave);
  
	// Alpha 1 slider
  
  
  
	sliderAlpha1.location.x = 50;
	sliderAlpha1.location.y = 50;
	sliderAlpha1.size.x = 150;
	sliderAlpha1.setBounds(-50000,50000);
	//sliderMasterPhase.setIntermediatePosition( sliderPG.size.x / 2);
	sliderAlpha1.setValue((int)(10000*systems[activePS].ps.getAlpha1()));
	sliderAlpha1.setOnChange(alpha1Change);
	sliderAlpha1.setLabelName("Alpha1");
  
	sliderAlpha1.l_name.location.x = sliderAlpha1.location.x;
	sliderAlpha1.l_name.location.y = sliderAlpha1.location.y;
	sliderAlpha1.l_name.txtSize = sliderRGB1.size.y;
	sliderAlpha1.useLabelName = true;
  
	sliderAlpha1.l_value.location.x = sliderAlpha1.location.x + sliderAlpha1.size.x;
	sliderAlpha1.l_value.location.y = sliderAlpha1.location.y;
	sliderAlpha1.l_value.txtSize = sliderAlpha1.size.y;
	sliderAlpha1.useLabelValue = true;  
  
	sliderAlpha1.setType(TYPE_FLOAT);
  
	tabBlend.bindEnt(&sliderAlpha1);
  
	// Alpha 2 slider
  
  
  
	sliderAlpha2.location.x = 289;
	sliderAlpha2.location.y = 50;
	sliderAlpha2.size.x = 150;
	sliderAlpha2.setBounds(-50000,50000);
	//sliderMasterPhase.setIntermediatePosition( sliderPG.size.x / 2);
	sliderAlpha2.setValue((int)(10000*systems[activePS].ps.getAlpha2()));
	sliderAlpha2.setOnChange(alpha2Change);
	sliderAlpha2.setLabelName("Alpha2");
  
	sliderAlpha2.l_name.location.x = sliderAlpha2.location.x;
	sliderAlpha2.l_name.location.y = sliderAlpha2.location.y;
	sliderAlpha2.l_name.txtSize = sliderRGB1.size.y;
	sliderAlpha2.useLabelName = true;
  
	sliderAlpha2.l_value.location.x = sliderAlpha2.location.x + sliderAlpha2.size.x;
	sliderAlpha2.l_value.location.y = sliderAlpha2.location.y;
	sliderAlpha2.l_value.txtSize = sliderAlpha1.size.y;
	sliderAlpha2.useLabelValue = true;  
  
	sliderAlpha2.setType(TYPE_FLOAT);
  
	tabBlend.bindEnt(&sliderAlpha2);
  
	// alpha waveform type
  
  
	listAlphaWave.location.x = 539;
	listAlphaWave.location.y = 50;
	listAlphaWave.size.x = 85;
	listAlphaWave.createItems(6);
	listAlphaWave.fillItem(0,"SINE");
	listAlphaWave.fillItem(1,"SQUARE");
	listAlphaWave.fillItem(2,"TRIANGLE");
	listAlphaWave.fillItem(3,"SAWTOOTH");
	listAlphaWave.fillItem(4,"INVERSE SAWTOOTH");
	listAlphaWave.fillItem(5,"CONST");
	listAlphaWave.setOnChange(alphaWaveChange);
	listAlphaWave.setSelected(systems[activePS].ps.getAlphaWave());
  
	listAlphaWave.l_name.setString("Wave");
	listAlphaWave.useLabelName = true;
	listAlphaWave.l_name.location.x = listAlphaWave.location.x;
	listAlphaWave.l_name.location.y = listAlphaWave.location.y;
	listAlphaWave.l_name.txtSize = 12;
  
	tabBlend.bindEnt(&listAlphaWave);
  
	//src blend
  
  
	listSrcBlend.location.x = 150;
	listSrcBlend.location.y = 115;
	listSrcBlend.createItems(6);
	listSrcBlend.fillItem(0,"GL_ONE");
	listSrcBlend.fillItem(1,"GL_ZERO");
	listSrcBlend.fillItem(2,"GL_DST_COLOR");
	listSrcBlend.fillItem(3,"GL_ONE_MINUS_DST_COLOR");
	listSrcBlend.fillItem(4,"GL_SRC_ALPHA");
	listSrcBlend.fillItem(5,"GL_ONE_MINUS_SRC_ALPHA");
	listSrcBlend.setOnChange(srcBlendChange);
  
	listSrcBlend.l_name.setString("SRC*");
	listSrcBlend.useLabelName = true;
	listSrcBlend.l_name.location.x = listSrcBlend.location.x;
	listSrcBlend.l_name.location.y = listSrcBlend.location.y;
	listSrcBlend.l_name.txtSize = 12;
  
	tabBlend.bindEnt(&listSrcBlend);
  
	//dst blend
  
  
	listDstBlend.location.x = 400;
	listDstBlend.location.y = 115;
	listDstBlend.createItems(6);
	listDstBlend.fillItem(0,"GL_ONE");
	listDstBlend.fillItem(1,"GL_ZERO");
	listDstBlend.fillItem(2,"GL_SRC_COLOR");
	listDstBlend.fillItem(3,"GL_ONE_MINUS_SRC_COLOR");
	listDstBlend.fillItem(4,"GL_SRC_ALPHA");
	listDstBlend.fillItem(5,"GL_ONE_MINUS_SRC_ALPHA");
	listDstBlend.setOnChange(dstBlendChange);
  
	listDstBlend.l_name.setString("DST*");
	listDstBlend.useLabelName = true;
	listDstBlend.l_name.location.x = listDstBlend.location.x;
	listDstBlend.l_name.location.y = listDstBlend.location.y;
	listDstBlend.l_name.txtSize = 12;
  
	tabBlend.bindEnt(&listDstBlend);
  
	// RGBGEN SLIDER   //  MATCH XY button
  
	sliderRGBPhase.location.x = 100;
	sliderRGBPhase.location.y = 75;
	sliderRGBPhase.size.x = 150;
	sliderRGBPhase.setBounds(0,10000);
	//sliderMasterPhase.setIntermediatePosition( sliderPG.size.x / 2);
	sliderRGBPhase.setValue((int)(10000*systems[activePS].ps.getRGBPhase()));
	sliderRGBPhase.setOnChange(rgbPhaseChange);
	sliderRGBPhase.setLabelName("RGB Phase");
  
	sliderRGBPhase.l_name.location.x = sliderRGBPhase.location.x;
	sliderRGBPhase.l_name.location.y = sliderRGBPhase.location.y;
	sliderRGBPhase.l_name.txtSize = sliderRGBPhase.size.y;
	sliderRGBPhase.useLabelName = true;
  
	sliderRGBPhase.l_value.location.x = sliderRGBPhase.location.x + sliderRGBPhase.size.x;
	sliderRGBPhase.l_value.location.y = sliderRGBPhase.location.y;
	sliderRGBPhase.l_value.txtSize = sliderRGBPhase.size.y;
	sliderRGBPhase.useLabelValue = true;  
  
	sliderRGBPhase.setType(TYPE_FLOAT);
  
	tabBlend.bindEnt(&sliderRGBPhase);
  
	// AlphaGen Phase
  
	sliderAlphaPhase.location.x = 100;
	sliderAlphaPhase.location.y = 25;
	sliderAlphaPhase.size.x = 150;
	sliderAlphaPhase.setBounds(0,10000);
	//sliderMasterPhase.setIntermediatePosition( sliderPG.size.x / 2);
	sliderAlphaPhase.setValue((int)(10000*systems[activePS].ps.getAlphaPhase()));
	sliderAlphaPhase.setOnChange(alphaPhaseChange);
	sliderAlphaPhase.setLabelName("Alpha Phase");
  
	sliderAlphaPhase.l_name.location.x = sliderAlphaPhase.location.x;
	sliderAlphaPhase.l_name.location.y = sliderAlphaPhase.location.y;
	sliderAlphaPhase.l_name.txtSize = sliderAlphaPhase.size.y;
	sliderAlphaPhase.useLabelName = true;
  
	sliderAlphaPhase.l_value.location.x = sliderAlphaPhase.location.x + sliderAlphaPhase.size.x;
	sliderAlphaPhase.l_value.location.y = sliderAlphaPhase.location.y;
	sliderAlphaPhase.l_value.txtSize = sliderAlphaPhase.size.y;
	sliderAlphaPhase.useLabelValue = true;  
  
	sliderAlphaPhase.setType(TYPE_FLOAT);
  
	tabBlend.bindEnt(&sliderAlphaPhase);
  
	// set xy rgb button
  
  
	butRGBXY.location.x = 440;
	butRGBXY.location.y = 75;
	butRGBXY.l_name.setString("Match XY Phase");
	butRGBXY.useLabelName = true;
	butRGBXY.l_name.location.x = butRGBXY.location.x;
	butRGBXY.l_name.location.y = butRGBXY.location.y;
	butRGBXY.l_name.txtSize = butNatH.size.y;
	butRGBXY.setOnClick(rgbPhaseMatchXY);
  
	tabBlend.bindEnt(&butRGBXY);

	butBGPS.location.x = 580;
	butBGPS.location.y = 468;
	butBGPS.size.x = 60;
	butBGPS.size.y = 12;
	butBGPS.l_name.setString("Systems");
	butBGPS.useLabelName = true;
	butBGPS.l_name.alignment = ALIGN_LEFT;
	butBGPS.l_name.location.x = butBGPS.location.x;
	butBGPS.l_name.location.y = butBGPS.location.y;
	butBGPS.l_name.txtSize  = butBGPS.size.y;
	butBGPS.setOnClick(openBGPSPannel);

	entities.bindEnt(&butBGPS);
  
	// match xy alphagen button
  
  
	butAlphaXY.location.x = 440;
	butAlphaXY.location.y = 25;
	butAlphaXY.l_name.setString("Match XY Phase");
	butAlphaXY.useLabelName = true;
	butAlphaXY.l_name.location.x = butAlphaXY.location.x;
	butAlphaXY.l_name.location.y = butAlphaXY.location.y;
	butAlphaXY.l_name.txtSize = butNatH.size.y;
	butAlphaXY.setOnClick(alphaPhaseMatchXY);
  
	tabBlend.bindEnt(&butAlphaXY);
  
  
	// TcMod Tab
  
	// Stretch1
  
  
  
	sliderStretch1.location.x = 60;
	sliderStretch1.location.y = 100;
	sliderStretch1.size.x = 150;
	sliderStretch1.setBounds(0,10000);
	//sliderMasterPhase.setIntermediatePosition( sliderPG.size.x / 2);
	sliderStretch1.setValue((int)(10000*systems[activePS].ps.getStretch1()));
	sliderStretch1.setOnChange(stretch1Change);
	sliderStretch1.setLabelName("Scale1");
  
	sliderStretch1.l_name.location.x = sliderStretch1.location.x;
	sliderStretch1.l_name.location.y = sliderStretch1.location.y;
	sliderStretch1.l_name.txtSize = sliderStretch1.size.y;
	sliderStretch1.useLabelName = true;
  
	sliderStretch1.l_value.location.x = sliderStretch1.location.x + sliderStretch1.size.x;
	sliderStretch1.l_value.location.y = sliderStretch1.location.y;
	sliderStretch1.l_value.txtSize = sliderStretch1.size.y;
	sliderStretch1.useLabelValue = true;  
  
	sliderStretch1.setType(TYPE_FLOAT);
  
	tabTcMod.bindEnt(&sliderStretch1);

	// RGB 2
  
  
  
	sliderStretch2.location.x = 312;
	sliderStretch2.location.y = 100;
	sliderStretch2.size.x = 150;
	sliderStretch2.setBounds(0,10000);
	//sliderMasterPhase.setIntermediatePosition( sliderPG.size.x / 2);
	sliderStretch2.setValue((int)(10000*systems[activePS].ps.getStretch2()));
	sliderStretch2.setOnChange(stretch2Change);
	sliderStretch2.setLabelName("Scale2");
  
	sliderStretch2.l_name.location.x = sliderStretch2.location.x;
	sliderStretch2.l_name.location.y = sliderStretch2.location.y;
	sliderStretch2.l_name.txtSize = sliderStretch2.size.y;
	sliderStretch2.useLabelName = true;
  
	sliderStretch2.l_value.location.x = sliderStretch2.location.x + sliderStretch2.size.x;
	sliderStretch2.l_value.location.y = sliderStretch2.location.y;
	sliderStretch2.l_value.txtSize = sliderStretch2.size.y;
	sliderStretch2.useLabelValue = true;  
  
	sliderStretch2.setType(TYPE_FLOAT);
  
	tabTcMod.bindEnt(&sliderStretch2);
  
	// stretch waveform type
  
  
	listStretchWave.location.x = 550;
	listStretchWave.location.y = 100;
	listStretchWave.size.x = 85;
	listStretchWave.createItems(6);
	listStretchWave.fillItem(0,"SINE");
	listStretchWave.fillItem(1,"SQUARE");
	listStretchWave.fillItem(2,"TRIANGLE");
	listStretchWave.fillItem(3,"SAWTOOTH");
	listStretchWave.fillItem(4,"INVERSE SAWTOOTH");
	listStretchWave.fillItem(5,"DISABLED");
	listStretchWave.setOnChange(stretchWaveChange);
	listStretchWave.setSelected(systems[activePS].ps.getStretchWave());
  
	listStretchWave.l_name.setString("Wave");
	listStretchWave.useLabelName = true;
	listStretchWave.l_name.location.x = listStretchWave.location.x;
	listStretchWave.l_name.location.y = listStretchWave.location.y;
	listStretchWave.l_name.txtSize = 12;
  
	tabTcMod.bindEnt(&listStretchWave);
  
  
  
	sliderStretchPhase.location.x = 100;
	sliderStretchPhase.location.y = 75;
	sliderStretchPhase.size.x = 150;
	sliderStretchPhase.setBounds(0,10000);
	//sliderMasterPhase.setIntermediatePosition( sliderPG.size.x / 2);
	sliderStretchPhase.setValue((int)(10000*systems[activePS].ps.getStretchPhase()));
	sliderStretchPhase.setOnChange(stretchPhaseChange);
	sliderStretchPhase.setLabelName("Scale Phase");
  
	sliderStretchPhase.l_name.location.x = sliderStretchPhase.location.x;
	sliderStretchPhase.l_name.location.y = sliderStretchPhase.location.y;
	sliderStretchPhase.l_name.txtSize = sliderStretchPhase.size.y;
	sliderStretchPhase.useLabelName = true;
  
	sliderStretchPhase.l_value.location.x = sliderStretchPhase.location.x + sliderStretchPhase.size.x;
	sliderStretchPhase.l_value.location.y = sliderStretchPhase.location.y;
	sliderStretchPhase.l_value.txtSize = sliderStretchPhase.size.y;
	sliderStretchPhase.useLabelValue = true;  
  
	sliderStretchPhase.setType(TYPE_FLOAT);
  
	tabTcMod.bindEnt(&sliderStretchPhase);
  
  
	butStretchXY.location.x = 440;
	butStretchXY.location.y = 75;
	butStretchXY.l_name.setString("Match XY Phase");
	butStretchXY.useLabelName = true;
	butStretchXY.l_name.location.x = butStretchXY.location.x;
	butStretchXY.l_name.location.y = butStretchXY.location.y;
	butStretchXY.l_name.txtSize = butStretchXY.size.y;
	butStretchXY.setOnClick(stretchPhaseMatchXY);
  
	tabTcMod.bindEnt(&butStretchXY);
  
  
  
	sliderRotSpeed.location.x = 100;
	sliderRotSpeed.location.y = 33;
	sliderRotSpeed.size.x = 125;
	sliderRotSpeed.setBounds(0,360);
	sliderRotSpeed.setValue((int)(systems[activePS].ps.getRotSpeed()));
	sliderRotSpeed.setOnChange(rotSpeedChange);
	sliderRotSpeed.setLabelName("Rotate Speed");
  
	sliderRotSpeed.l_name.location.x = sliderRotSpeed.location.x;
	sliderRotSpeed.l_name.location.y = sliderRotSpeed.location.y;
	sliderRotSpeed.l_name.txtSize = sliderRotSpeed.size.y;
	sliderRotSpeed.useLabelName = true;
  
	sliderRotSpeed.l_value.location.x = sliderRotSpeed.location.x + sliderRotSpeed.size.x;
	sliderRotSpeed.l_value.location.y = sliderRotSpeed.location.y;
	sliderRotSpeed.l_value.txtSize = sliderRotSpeed.size.y;
	sliderRotSpeed.useLabelValue = true;
  
	tabTcMod.bindEnt(&sliderRotSpeed);
  
  
  
	sliderRotSpeedVar.location.x = 350;
	sliderRotSpeedVar.location.y = 33;
	sliderRotSpeedVar.size.x = 125;
	sliderRotSpeedVar.setBounds(0,360);
	sliderRotSpeedVar.setValue((int)(systems[activePS].ps.getRotSpeedVar()));
	sliderRotSpeedVar.setOnChange(rotSpeedVarChange);
	sliderRotSpeedVar.setLabelName("Var");
  
	sliderRotSpeedVar.l_name.location.x = sliderRotSpeedVar.location.x;
	sliderRotSpeedVar.l_name.location.y = sliderRotSpeedVar.location.y;
	sliderRotSpeedVar.l_name.txtSize = sliderRotSpeedVar.size.y;
	sliderRotSpeedVar.useLabelName = true;
  
	sliderRotSpeedVar.l_value.location.x = sliderRotSpeedVar.location.x + sliderRotSpeedVar.size.x;
	sliderRotSpeedVar.l_value.location.y = sliderRotSpeedVar.location.y;
	sliderRotSpeedVar.l_value.txtSize = sliderRotSpeedVar.size.y;
	sliderRotSpeedVar.useLabelValue = true;
  
	tabTcMod.bindEnt(&sliderRotSpeedVar);

	int ypos = 447;
	for (auto &b : buttons) {
		b.location.y = ypos;
		ypos-=15;
		b.location.x = 150;
		BGPannel.bindEnt(&b);
		b.setColor(SYS_BUT_DISABLE,BUTTON_COLOR);
		b.setColor(SYS_BUT_ACTIVE,BUTTON_COLOR_ACTIVE);
		b.setColor(SYS_BUT_ACTIVE,BUTTON_COLOR_HOVERACTIVE);
	}	

	buttons[0].setColor(SYS_BUT_CURRENT,BUTTON_COLOR);					// Colour the currently active button
	buttons[0].setColor(SYS_BUT_DELETE,BUTTON_COLOR_ACTIVE);
	buttons[0].setColor(SYS_BUT_DELETEL,BUTTON_COLOR_HOVERACTIVE);
	
	//BGLoad1.setOnClick(pannelLoad1); BGLoad2.setOnClick(pannelLoad2); BGLoad3.setOnClick(pannelLoad3); BGLoad4.setOnClick(pannelLoad4);

	ypos = 447;
	for (int i=0;i<MAX_SYSTEMS;i++)
	{

		buttons[i].setOnClick([i](void)
			{
				activeProcessor(i);
			}); 


		sorts[i].location.y = posXs[i].location.y = posYs[i].location.y = posZs[i].location.y = rotXs[i].location.y = 
			rotXs[i].location.y = rotYs[i].location.y = rotZs[i].location.y = ypos;
		
		posXs[i].location.x = 378; posYs[i].location.x = 422; posZs[i].location.x = 466;
		rotXs[i].location.x = 510; rotYs[i].location.x = 554; rotZs[i].location.x = 598;

		posXs[i].size.x = posYs[i].size.x = posZs[i].size.x = 40;
		rotXs[i].size.x = rotYs[i].size.x = rotZs[i].size.x = 40;

		posXs[i].setIntermediatePosition(posXs[i].size.x/2);
		posYs[i].setIntermediatePosition(posYs[i].size.x/2);
		posZs[i].setIntermediatePosition(posZs[i].size.x/2);

		rotXs[i].setIntermediatePosition(rotXs[i].size.x/2);
		rotYs[i].setIntermediatePosition(rotYs[i].size.x/2);
		rotZs[i].setIntermediatePosition(rotZs[i].size.x/2);

		posXs[i].setBounds(-256,256,0); posYs[i].setBounds(-256,256,0); posZs[i].setBounds(-256,256,0);		
		rotXs[i].setBounds(-180,180,0); rotYs[i].setBounds(-180,180,0); rotZs[i].setBounds(-180,180,0);

		posXs[i].setValue(0); posYs[i].setValue(0); posZs[i].setValue(0);
		rotXs[i].setValue(0); rotYs[i].setValue(0); rotZs[i].setValue(0);

		BGPannel.bindEnt(&posXs[i]);BGPannel.bindEnt(&posYs[i]);BGPannel.bindEnt(&posZs[i]);
		BGPannel.bindEnt(&rotXs[i]);BGPannel.bindEnt(&rotYs[i]);BGPannel.bindEnt(&rotZs[i]);
		BGPannel.bindEnt(&sorts[i]);

		sorts[i].location.x = 330;
		sorts[i].size.x = 30;
		sorts[i].setBounds(0,16);
		sorts[i].l_value.location.x = sorts[i].location.x + sorts[i].size.x;
		sorts[i].l_value.location.y = sorts[i].location.y;
		sorts[i].l_value.txtSize = sorts[i].size.y;
		sorts[i].useLabelValue = true;
		sorts[i].setValue(0);

		labels[i].location.x = 200;
		labels[i].location.y = ypos;
		labels[i].maxSize = 130;
		labels[i].txtSize = sorts[i].size.y;

		BGPannel.bindEnt(&labels[i]);

		posXs[i].setOnChange( [i](int x) { systems[i].x = x;		});
		posYs[i].setOnChange( [i](int y) { systems[i].y = y;		});
		posZs[i].setOnChange( [i](int z) { systems[i].z = z;		});
		rotXs[i].setOnChange( [i](int x) { systems[i].ps.rotx = x;	}); 	
		rotYs[i].setOnChange( [i](int y) { systems[i].ps.roty = y;	}); 	
		rotZs[i].setOnChange( [i](int z) { systems[i].ps.rotz = z;	}); 	
		sorts[i].setOnChange( [i](int s) { systems[i].sort = s;		});

		ypos-=15;
	}

	BGPannel.size.y = 25 + MAX_SYSTEMS * 15;		
	BGPannel.location.y = GL_HEIGHT - BGPannel.size.y;	

	BGLabelLoad.location.y = BGLabelName.location.y = BGLabelX.location.y = BGLabelY.location.y = BGLabelZ.location.y = 
		BGLabelDel.location.y = BGLabelSort.location.y = BGLabelrX.location.y = BGLabelrY.location.y = BGLabelrZ.location.y = 460;
	BGLabelLoad.setString("Active");BGLabelLoad.location.x = 120;	BGLabelLoad.txtSize = 15;	BGPannel.bindEnt(&BGLabelLoad);
	BGLabelName.setString("Name");	BGLabelName.location.x = 200;	BGLabelName.txtSize = 15;	BGPannel.bindEnt(&BGLabelName);
	BGLabelSort.setString("Sort");	BGLabelSort.location.x = 320;	BGLabelSort.txtSize = 15;	BGPannel.bindEnt(&BGLabelSort);
	BGLabelX.setString("X");		BGLabelX.location.x = 390;		BGLabelX.txtSize = 15;		BGPannel.bindEnt(&BGLabelX);
	BGLabelY.setString("Y");		BGLabelY.location.x = 434;		BGLabelY.txtSize = 15;		BGPannel.bindEnt(&BGLabelY);
	BGLabelZ.setString("Z");		BGLabelZ.location.x = 478;		BGLabelZ.txtSize = 15;		BGPannel.bindEnt(&BGLabelZ);
	BGLabelrX.setString("\x9DX");	BGLabelrX.location.x = 515;		BGLabelrX.txtSize = 15;		BGPannel.bindEnt(&BGLabelrX);
	BGLabelrY.setString("\x9DY");	BGLabelrY.location.x = 560;		BGLabelrY.txtSize = 15;		BGPannel.bindEnt(&BGLabelrY);
	BGLabelrZ.setString("\x9DZ");	BGLabelrZ.location.x = 605;		BGLabelrZ.txtSize = 15;		BGPannel.bindEnt(&BGLabelrZ);

	loadDefaultsFromFile();
  
	int viewyrot=0;
	int viewxrot=0;
  
	RGBf pannelColor;
  
	int lastTime = clock();
	//int nowTime;
  
	glViewport (0, 0, rc.right-1, rc.bottom-1);
	glMatrixMode (GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity ();							// Reset The Projection Matrix
	// Set Up Ortho Mode To Fit 1/4 The Screen (Size Of A Viewport)
	gluOrtho2D(0,GL_WIDTH-1,0,GL_HEIGHT-1);
	glMatrixMode (GL_MODELVIEW);									// Select The Modelview Matrix
	glLoadIdentity ();
  
	// program main loop
	while (!bQuit) 
	{
		// check for messages
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			// handle or dispatch messages
			if (msg.message == WM_QUIT) 
			{
				bQuit = TRUE;
			} 
			else 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

		} 
		else if (paused || minimized)  // if paused, stop entering the render loop
		{ 
			Sleep(100);
		} 
		else 
		{
		  
			//because height can change by pressing button, we must set it every loop
			sliderHeight.setValue_nochange((int)systems[activePS].ps.getHeight());
			sliderFreq.setValue_nochange((int)( 10000*systems[activePS].ps.getFrequency() ));
			sliderRGBPhase.setValue_nochange((int)( 10000*systems[activePS].ps.getRGBPhase()));
			sliderAlphaPhase.setValue_nochange((int)( 10000*systems[activePS].ps.getAlphaPhase()));
			sliderStretchPhase.setValue_nochange((int)( 10000*systems[activePS].ps.getStretchPhase()));

			//label *BGNameLabels[MAX_SYSTEMS] = { &BGName1,&BGName2,&BGName3,&BGName4,&BGName5,&BGName6,&BGName7,&BGName8 };

			for(int i=0;i<MAX_SYSTEMS;i++)
			{
				labels[i].setString(systems[i].shaderName);
			}		 

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindTexture(GL_TEXTURE_2D, texture[0].texID);
		 
		 // Draw background color quad
		  glBegin(GL_QUADS);
			glColor3f(bgred,bggreen,bgblue);
			glVertex2i(10,154);
			glVertex2i(rc.right-3,154);
			glVertex2i(rc.right-3,485);
			glVertex2i(10,485);
		  glEnd();

		  // ENT RENDER TAKEN FROM HERE

		  viewyrot = slider1.value;
		  viewxrot = slider2.value;
		  float zoom = (float) sliderZoom.value / 10000;
		  
		  float tempTime = systems[activePS].ps.calcTimeN();
		  char bufferT[15];	  
		  
		  // Init 3d View
		  
		  glViewport (10, 0, rc.right-20, rc.bottom/*-130*/);
		  glMatrixMode (GL_PROJECTION);						// Select The Projection Matrix
		  glLoadIdentity ();							// Reset The Projection Matrix
		  // Set Up Perspetive Mode To Fit 1/4 The Screen (Size Of A Viewport)
		  gluPerspective( 60.0, (GLfloat)(rc.right-20)/(GLfloat)(rc.bottom/*-130*/), 1.0f, 8192.0 ); 
		  glMatrixMode (GL_MODELVIEW);									// Select The Modelview Matrix
		  glLoadIdentity ();
		  
		  glScissor(10, 145, rc.right-20, rc.bottom-145); // Use scissor to stop from drawing over 2d. 
		  glEnable(GL_SCISSOR_TEST);
		  
		  glEnable(GL_DEPTH_TEST);
		  
		  // View Translations
		  // Then push new matrix to hold sprite position transformations
		  // then pop transform matrix      	  
		  
		  //glTranslatef(((float)pos.x),((float)pos.y),0.0f);
		  glTranslatef(0.0f, -180.0f * zoom * systems[activePS].ps.zBase + -30.0f * zoom, -512.0f * zoom); // Back and down a bit
		  glRotatef(viewxrot, 1.0f, 0.0f, 0.0f); // Pan up/down
		  glRotatef(viewyrot, 0.0f, 1.0f, 0.0f);  // Pan Arround
		  //glRotatef(180, (float)viewxrot/(float)180, (float)viewyrot/(float)180, 0.0f);  // Pan Arround
		  
		  // AXIS
		  glBegin(GL_LINES);
			glColor3f(1.0f,0.0f,0.0f);
			glVertex3f(0.0f,0.0f,0.0f); glVertex3f(16.0,0.0f,0.0f);
			glColor3f(0.0f,0.0f,1.0f);
			glVertex3f(0.0f,0.0f,0.0f); glVertex3f(0.0,16.0f,0.0f);
			glColor3f(0.0f,1.0f,0.0f);
			glVertex3f(0.0f,0.0f,0.0f); glVertex3f(0.0,0.0f,-16.0f);
		 glEnd();
		 
		// Draw the player Box

		  if (togglePlayer>0)
		  {
			glPushMatrix();
			glTranslatef(128,0,0);
			
			glBegin(GL_QUADS);
			  glColor3f(1.0f,0.0f,0.0f);
			
			  //Bottom
			  glVertex3f(-16,0,-16); glVertex3f(-16,0,16); glVertex3f(16,0,16); glVertex3f(16,0,-16);
			  //top
			  glVertex3f(-16,togglePlayer,-16); glVertex3f(-16,togglePlayer,16); glVertex3f(16,togglePlayer,16); glVertex3f(16,togglePlayer,-16);
			  
			  glVertex3f(-16,0,-16);glVertex3f(16,0,-16);glVertex3f(16,togglePlayer,-16);glVertex3f(-16,togglePlayer,-16);
			  glVertex3f(-16,0,16);glVertex3f(16,0,16);glVertex3f(16,togglePlayer,16);glVertex3f(-16,togglePlayer,16);
			  
			  glVertex3f(-16,0,-16);glVertex3f(-16,0,16);glVertex3f(-16,togglePlayer,16);glVertex3f(-16,togglePlayer,-16);
			  glVertex3f(16,0,-16);glVertex3f(16,0,16);glVertex3f(16,togglePlayer,16);glVertex3f(16,togglePlayer,-16);
			
			glEnd();
			
			glPopMatrix();
		  }
		 
		  // Draw Particles
		  
		  //ps.calcTimeN();          
		  glEnable(GL_BLEND);
		  glEnable(GL_TEXTURE_2D);
		  //glDisable(GL_DEPTH_TEST);
		  glDepthMask(GL_FALSE);

		  // Rewrite the particle render with sort order
		  float px,py,pz;
		  int np;
		  //glBindTexture(GL_TEXTURE_2D, 1);
		  for (int order=0;order!=17;order++)
		  {
			  // Loop through all systems, render if their sort order == loop order
			  for (int i=0;i<MAX_SYSTEMS;i++)
			  {
				  if (systems[i].sort == order)
				  {
					  // Render system
					  if (systems[i].active)
					  {
						  float duration = 1.f / systems[i].ps.getFrequency();
						  if(tempTime < duration || particleSystem::getOnce() == false)
						  {
								glPushMatrix();     				  
								glTranslatef(systems[i].x,systems[i].z,-systems[i].y);
								np = systems[i].ps.numParticles;
								systems[i].ps.calcTimeN();
								while(np)
								{
									glPushMatrix();
									np--;
									systems[i].ps.getPositionVector(np,px,py,pz);            
									systems[i].ps.applyRotation(px,py,pz);
									glTranslatef(px, pz, -py);
									glRotatef(-viewyrot, 0.0f, 1.0f, 0.0f);  // Pan Arround
									glRotatef(-viewxrot, 1.0f, 0.0f, 0.0f); // Pan up/down
									glBindTexture(GL_TEXTURE_2D, systems[i].texture.texID);
									systems[i].ps.drawParticle(np);
									glPopMatrix();
								}
								glPopMatrix();
						  }
						  else
						  {

						  }
					  }
				  }
			  }
		  }
		  
		  glDisable(GL_BLEND);
		  glDisable(GL_TEXTURE_2D);
		  //glEnable(GL_DEPTH_TEST);
		  glDepthMask(GL_TRUE);
		  
		  glDisable(GL_SCISSOR_TEST);
		  glDisable(GL_DEPTH_TEST);
		  
		  glViewport (0, 0, rc.right-1, rc.bottom-1);
		  glMatrixMode (GL_PROJECTION);						// Select The Projection Matrix
		  glLoadIdentity ();							// Reset The Projection Matrix
		  // Set Up Ortho Mode To Fit 1/4 The Screen (Size Of A Viewport)
		  gluOrtho2D(0,GL_WIDTH-1,0,GL_HEIGHT-1);
		  glMatrixMode (GL_MODELVIEW);									// Select The Modelview Matrix
		  glLoadIdentity ();


		  // DRAW USERENTRY!
		  glBindTexture(GL_TEXTURE_2D, texture[0].texID);

		  // Move Drawing entities here

		  //Pannel
		  if (currentTab) currentTab->getColor(pannelColor.R,pannelColor.G,pannelColor.B,TAB_COLOR_ACTIVE);
		  else pannelColor.R = pannelColor.G = pannelColor.B = 0.6f;
		  glBegin(GL_QUADS);
			glColor3f( pannelColor.R, pannelColor.G, pannelColor.B );
			glVertex2f((0),(15));
			glVertex2f((0),(130));
			glVertex2f((639),(130));
			glVertex2f((639),(15));         
		  glEnd();
		  // draw side pannel
		  if (BGPannel.active)
		  {
				glBegin(GL_QUADS);
					glColor3f( pannelColor.R, pannelColor.G, pannelColor.B );
					glVertex2f((BGPannel.location.x)				,(BGPannel.location.y));
					glVertex2f((BGPannel.location.x+BGPannel.size.x),(BGPannel.location.y));
					glVertex2f((BGPannel.location.x+BGPannel.size.x),(BGPannel.location.y+BGPannel.size.y));
					glVertex2f((BGPannel.location.x)				,(BGPannel.location.y+BGPannel.size.y));         
				glEnd();
		  }

		  //glEnable(GL_TEXTURE_2D);          
		  
		   // Set texture to text map
		  entities.drawAll(); // Draw all bound entities
		  
		  //glDisable(GL_TEXTURE_2D);

		  ftoa(tempTime,bufferT);
		  //removeTrailingZeros(bufferT);
		  bufferT[6]=0;
		  drawTime.setString(bufferT);
		  drawTime.draw();

		  glEnable(GL_BLEND);
		  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		  //IF ACTIVE: DRAW
		  if (entry.active) entry.draw();
		  glDisable(GL_BLEND);
			  
		  SwapBuffers( hDC );
		  //theta += 0.2f;

		  if(particleSystem::getOnce() == true)
		  {

			  float smallestFreq = 100000;

			  for (auto &sy : systems)
			  {
				  if(sy.active)
					if(sy.ps.getFrequency() < smallestFreq) smallestFreq = sy.ps.getFrequency();
			  }

			  // get max duration

			  float maxDuration = 1.f / smallestFreq;

			  if(tempTime+(0.01*maxDuration) >= maxDuration)		// Run to completion reset the time
				  particleSystem::resetTime();

		  }

		  // get the smallest particle system frequency 
		  
		} // End Render Block
  }
  
  //saveSettings();
  
  // shutdown OpenGL
  DisableOpenGL( hWnd, hDC, hRC );
  // destroy the window explicitly
  DestroyWindow( hWnd );
  saveDefaultsToFile();
  return msg.wParam;
}

// Window Procedure

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Only change mouse coords on mouse move, LMB up/down    
	if ((message == WM_MOUSEMOVE) || (message == WM_LBUTTONDOWN) || (message == WM_LBUTTONUP) || (message == WM_RBUTTONDOWN) )
	{
	  int mx,my;        
	  mx = (int) (lParam & 0x0000ffff);        
	  my = (int) ((lParam >> 16) & 0x0000ffff);    
	  mouse.x = mx;
	  mouse.y = my;
	  scr2w(); 
	}
	//LRESULT res;

	if (!BGPannel.inBounds(pos.x,pos.y) && (message == WM_LBUTTONDOWN) && (BGPannel.active) && (entry.active==false)) 
	{
		closeBGPSPannel();
	}
	{
		hideSidePannelBut = false;
	}
	   
  switch (message) 
	{
	  case WM_CREATE:
		return 0;

	  case WM_CLOSE:
		//save settings
		PostQuitMessage( 0 );
		return 0;

	  case WM_SIZE:
		  switch (wParam)
		  {
		  case SIZE_MINIMIZED:
				minimized = true;
			  return 0;
		  default:
			  minimized = false;
		  }
		  return 0;

	  case WM_DESTROY:
		return 0;
		
	  case WM_SETFOCUS:
		entities.forceUnActive(); // Remove Any Active State
		return 0;
		
	  case WM_KILLFOCUS:
		entities.forceUnActive(); // Remove Any Active State		
		return 0;
		

		default:
		  if ((message == WM_KEYDOWN) && (wParam == VK_F5)) 
		  {
			  paused ^= 1; // toggle paused on f5
		  }
		  if((message == WM_KEYDOWN) && (wParam == VK_SHIFT))
		  {
			  syncChanges = true;
			  butBGPS.setColor(1.f,0.f,0.f);
			  //OutputDebugString("Sync: ON\n");
		  }
		  if((message == WM_KEYUP) && (wParam == VK_SHIFT))
		  {
			  syncChanges = false;
			  butBGPS.setColor(.5f,.5f,.5f);
			  //OutputDebugString("Sync: OFF\n");
		  }
		  if (entry.active) entry.eventHandler(message,wParam,pos.x,pos.y);
		  else 
		  {
			  // Reset the timer when clicked on
			  if (message == WM_LBUTTONDOWN)
			  {
				  //Timer bounds: 28,18    101,32
				  
				  if ((pos.x >= drawTime.location.x) 
					&& (pos.x <= drawTime.location.x + drawTime.size.x)
					&& (pos.y >= drawTime.location.y) 
					&& (pos.y <= drawTime.location.y + drawTime.size.y))
				  {
					  particleSystem::resetTime();
				  }
			  }
			  entities.eventRelay(message,wParam,pos.x,pos.y);
		  }
		  return DefWindowProc(hWnd, message, wParam, lParam);
  }
}

// Enable OpenGL

VOID EnableOpenGL( HWND hWnd, HDC * hDC, HGLRC * hRC )
{
  PIXELFORMATDESCRIPTOR pfd;
  int iFormat;

  // get the device context (DC)
  *hDC = GetDC( hWnd );

  // set the pixel format for the DC
  ZeroMemory( &pfd, sizeof( pfd ) );
  pfd.nSize = sizeof( pfd );
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | 
  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 16;
  pfd.iLayerType = PFD_MAIN_PLANE;
  iFormat = ChoosePixelFormat( *hDC, &pfd );
  SetPixelFormat( *hDC, iFormat, &pfd );

  // create and enable the render context (RC)
  *hRC = wglCreateContext( *hDC );
  wglMakeCurrent( *hDC, *hRC );
}

// Disable OpenGL

VOID DisableOpenGL( HWND hWnd, HDC hDC, HGLRC hRC )
{
  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( hRC );
  ReleaseDC( hWnd, hDC );
} 
