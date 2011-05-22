#ifndef TGA_H
#define TGA_H

#include <fstream>

class tgaFile
{
  struct TGA_HEADER
  {
    char  identsize;          // size of ID field that follows 18 byte header (0 usually)
    char  colourmaptype;      // type of colour map 0=none, 1=has palette
    char  imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    short colourmapstart;     // first colour map entry in palette
    short colourmaplength;    // number of colours in palette
    char  colourmapbits;      // number of bits per palette entry 15,16,24,32

    short xstart;             // image x origin
    short ystart;             // image y origin
    short width;              // image width in pixels
    short height;             // image height in pixels
    char  bits;               // image bits per pixel 8,16,24,32
    char  descriptor;         // image descriptor bits (vh flip bits)
    
    // pixel data follows header
    
  };
  struct TGA_RGBA
  {
    char R;
    char G;
    char B;
    char A;
  };
  
  struct TGA_RGB
  {
    char R;
    char G;
    char B;
  };
  
  
  char filename[256];
  std::ifstream inFile;
  
  unsigned char byteCount;
  unsigned char *data;
  
 public:
  tgaFile(char *path);
  tgaFile();
  ~tgaFile();
  TGA_HEADER header;
  bool readHeader();
  bool readData();
  unsigned char getByteCount() { return byteCount; };
  unsigned char * getData() { return data; };
    
};


#endif // TGA_H
