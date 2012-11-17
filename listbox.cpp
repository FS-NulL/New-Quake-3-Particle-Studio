#include "listbox.h"
#include <cstring>
#include <windows.h>
#include <fstream>
#include <cmath>

#include <GL/gl.h>

listBox::listBox()
{
  active = false;
  hoverActive = false; 
  useLabelName = false;
  items = 0;
  location.x = 200;
  location.y = 100;
  size.x = 125;
  size.y = 12;
  selectedItem = 0;
  
  l_name.location.x = location.x;
  l_name.location.y = location.y;
  l_name.alignment = ALIGN_RIGHT;
  l_name.txtSize = size.y;
  
  bgColor.R = 0.5;
  bgColor.G = 0.5;
  bgColor.B = 0.5;
  
  bgColor_active.R = 0.5;
  bgColor_active.G = 0.5;
  bgColor_active.B = 0.9;
  
  bgColor_hoverActive.R = 0.5;
  bgColor_hoverActive.G = 0.5;
  bgColor_hoverActive.B = 0.9;
  
  listOffset = 0;
}

listBox::listBox(int ni)
{
  listBox();
  createItems(ni);
}

listBox::listItem::listItem()
{
  name[0] = name[1] = 0;
  id =-1;
}

listBox::listItem::~listItem()
{
}

listBox::~listBox()
{
}

int listBox::eventHandler(UINT message,WPARAM key,int mousex,int mousey)
{
	  if (message == WM_LBUTTONDOWN) // Set active / move slider to position of mouse
	{
		active = true;
		hoverActive = false;
		//CALC OFFSET HERE
		
		//std::ofstream oFile("listbox.txt",std::ios::app);
		listOffset = location.y - (numItems * size.y);
		//oFile << '\n' << listOffset << '\n';
		if ((listOffset<0) && (size.y > 0)) 
		{
		  listOffset /= (-size.y);
		  listOffset *= size.y;
		  listOffset += size.y;
		  listOffset += location.y;
		}
		else listOffset = location.y;
		
		
		//oFile << location.y << ' ' << listOffset << '\n';
		//oFile.close();
	}
	if (message == WM_MOUSEMOVE)
	{
		if (active) 
		{
		}
		else  // Hovering? How do we detect left hover?
		{
		  if (inBounds(mousex,mousey)) hoverActive = true;
		  else hoverActive = false; 
		}
	}
	if (message == WM_LBUTTONUP)   // Unset Active
	{
	  if (active)
	  {
		//select option, but only if mouse is over an item
		// step 1 mouse bounds checking
		// set selected item
		
		// 1.1 generate bounds which is bottom of ent to bottom of list
		vec2D topleft;
		vec2D bottomright;
		
		topleft.x = location.x;
		topleft.y = listOffset;
		bottomright.x = location.x+size.x;
		bottomright.y = listOffset - numItems*size.y;
		
		if ((mousex >= topleft.x) 
		&& (mousex <= bottomright.x) 
		&& (mousey >= bottomright.y) 
		&& (mousey <= topleft.y)) 
		{
		  // in bounds
		  // convert location to list item number only working on y coordinates
		  // 0
		  // 1
		  // ..N
		  // top = location.y;
		  float relMPos = listOffset - mousey; // positive number to be divided by size.y
		  relMPos /= size.y;
		  int si = (int) floor(relMPos);
		  selectedItem = si;
		  changeOccured();
		  
		}
		
	  }
		active = false;        
		if (inBounds(mousex,mousey)) hoverActive = true;
	}
	if (message == WM_KEYDOWN)
	{
		// VK_UP
		// VK_DOWN
		if (key == VK_UP)
		{
		  //try and change selected
		  if (selectedItem >0) 
		  {
			selectedItem--;
			changeOccured();
		  }
		}
		if (key == VK_DOWN)
		{
		  //try and change selected
		  if (selectedItem<(numItems-1)) 
		  {
			selectedItem++;
			changeOccured();
		  }
		}
	}
	return 0;
}

int listBox::draw()
{
  //draw ent
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
  glEnd();
  
  if (selectedItem >=0 ) print(items[selectedItem].name,location.x,location.y,size.y,size.x-size.y,ALIGN_LEFT);
  // draw drop down box
  glBegin(GL_QUADS);
	
	glColor3f( 0.6f, 0.6f, 0.6f );
		
	glVertex2f(location.x+size.x,location.y);
	glVertex2f(location.x + size.x - size.y ,location.y);
	glVertex2f(location.x + size.x-size.y,location.y + size.y);
	glVertex2f(location.x+size.x,location.y + size.y);
  glEnd();
  
  glBegin(GL_TRIANGLES);
	
	glColor3f( 0.0f, 0.0f, 0.0f );
		
	glVertex2f(location.x + size.x - (size.y/4),location.y + size.y - (size.y/4));
	glVertex2f(location.x + size.x - (size.y/2),location.y + (size.y/4));
	glVertex2f(location.x + size.x - 3*(size.y/4),location.y + size.y - (size.y/4));
  glEnd();
  
  
  if (useLabelName) l_name.draw();
  if (active)
  {
	// draw roll out of items
	for (int i=0;i<numItems;i++)
	{
	  // Draw background
	  glBegin(GL_QUADS);
		glColor3f( 0.6f, 0.6f, 0.6f );
		
		glVertex2f(location.x,listOffset-i*size.y-size.y);
		glVertex2f(location.x + size.x ,listOffset-i*size.y-size.y);
		glVertex2f(location.x + size.x,listOffset-i*size.y);
		glVertex2f(location.x,listOffset-i*size.y);
	  glEnd();
	  // Display text
	  print(items[i].name,location.x,listOffset-(i+1)*size.y,size.y,size.x,ALIGN_LEFT);
	}
  }
  if (useLabelName) l_name.draw();
  return 0;
}

int listBox::forceUnActive()
{
  active = false;
  hoverActive = false;
  return 0;
}

int listBox::createItems(unsigned int ni)
{
  if (items) delete [] items;
  items = new listItem[ni];
  numItems = ni;
  selectedItem = 0;
  return ni;
}

bool listBox::fillItem(int i,char *s)
{
  if (i<numItems)
  {
	std::strcpy(items[i].name,s);
	items[i].id = i;
  }
  else return false;
  return true;
}

int listBox::changeOccured()
{
  // pass selected item
  if (onChange) onChange(selectedItem);
  return 0;
}

int listBox::setOnChange(void (*p)(int))
{
  onChange = p;
  return 0;
}


bool listBox::setSelected(int i)
{
  return selectedItem = i;
}
