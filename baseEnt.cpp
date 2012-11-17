
#include "baseEnt.h"

bool baseEnt::inBounds(int mousex,int mousey)
{
  if ((mousex >= location.x) 
  && (mousex <= location.x + size.x) 
  && (mousey >= location.y) 
  && (mousey <= location.y + size.y)) 
  return true;
  
  return false;
}

// HOURS


