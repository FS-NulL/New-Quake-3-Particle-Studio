
#include "basicstructs.h"
#include <cstdio>

float w2glx(float x)
{
  return (x/320)-1;
}

float w2gly(float y)
{
  return (y/240)-1;
}

int itoa(int i, char *s)
{
  sprintf(s,"%d",i);
  return 0;
}

int ftoa(float i,char *s)
{
  sprintf(s,"%f",i);
  return 0;
}

int removeTrailingZeros(char *s)
{
  char *e=s;
  while (*e) e++;
  e--;
  while ((*e == '0') && (e != s))
  {
    *e-- = 0;
  }
  return 0;
}
