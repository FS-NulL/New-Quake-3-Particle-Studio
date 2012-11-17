#ifndef LISTBOX_H
#define LISTBOX_H

#include "baseEnt.h"
#include "basicstructs.h"
#include "label.h"

class listBox : public baseEnt
{
  
  class listItem
  {
    
    
   public: 
      char name[100];
      int id;
    listItem();
    ~listItem();
  };
  
  bool active;
  bool hoverActive;
  RGBf bgColor;
  RGBf bgColor_active;
  RGBf bgColor_hoverActive;
  void (*onChange)(int);
  listItem *items;
  int changeOccured();
  int listOffset;
 public:
  int selectedItem;
  listBox();
  listBox(int ni); // Pass initializing number of items (ni)
  ~listBox();
  int eventHandler(UINT message,WPARAM key,int mousex,int mousey);
  int draw();
  int forceUnActive();
  int setOnChange(void (*p)(int));
  label l_name;
  bool useLabelName;
  int createItems(unsigned int ni);
  bool fillItem(int i,char *s);
  bool setSelected(int i);
  int numItems;
};
/*

  bool active;
  bool hoverActive;
  RGBf bgColor;
  RGBf bgColor_active;
  RGBf bgColor_hoverActive;
  RGBf forceColor;
  bool useForceColor;
  bool changeOccured();
  bool (*onChange)(char*);
  char string[512];
  
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
  */

#endif // LISTBOX_H
