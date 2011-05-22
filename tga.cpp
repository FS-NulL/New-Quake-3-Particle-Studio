#include <fstream>
#include <cstring>

#include "tga.h"

tgaFile::tgaFile()
{
  data = 0;
}

tgaFile::tgaFile(char *path) 
{
  data = 0;
  std::strcpy(filename,path);
  readHeader();
  readData();
}

tgaFile::~tgaFile()
{
  // Must delete any data
  if (data != 0) delete [] data;
}

bool tgaFile::readHeader()
{
  inFile.open(filename,std::ios::binary);
  std::ofstream oFile("D:/Programming/Particle Studio 2/ps_tga_log.txt",std::ios::app);
  oFile << filename << ":\n";
  if (inFile.is_open())
  {
    inFile.seekg(0,std::ios::beg);
    //inFile.read((char*) &header, 18 * sizeof(char));
    
    inFile.read((char*) &header.identsize, sizeof(char));
    inFile.read((char*) &header.colourmaptype, sizeof(char));
    inFile.read((char*) &header.imagetype, sizeof(char));
    
    inFile.read((char*) &header.colourmapstart, sizeof(short));
    inFile.read((char*) &header.colourmaplength, sizeof(short));
    inFile.read((char*) &header.colourmapbits, sizeof(char));
    
    inFile.read((char*) &header.xstart, sizeof(short));
    inFile.read((char*) &header.ystart, sizeof(short));
    inFile.read((char*) &header.width, sizeof(short));
    inFile.read((char*) &header.height, sizeof(short));
    inFile.read((char*) &header.bits, sizeof(char));
    inFile.read((char*) &header.descriptor, sizeof(char));
        
    inFile.close();
    
    //std::ofstream oFile("D:/Programming/Particle Studio 2/ps_tga_log.txt",std::ios::app);
    oFile << "Type: " << (int) header.imagetype << '\n';
    oFile << "BPP: "  << (int) header.bits << '\n';
    oFile << "WDTH: " << header.width << '\n';
    oFile << "HEGT: " << header.height << '\n';
    //oFile << sizeof(TGA_HEADER) << '\n';
    
  }
  else // File not open, probably doesnt exist
  {
    oFile.close();
    return false;
  }
  oFile.close();
  return true;
}

bool tgaFile::readData()
{
  byteCount = header.bits / 8;
  
  if (header.colourmaptype!= 0 || header.imagetype != 2) return false;
	
	if (byteCount != 3 && byteCount != 4) return false;
	
	unsigned int imageSize = header.width * header.height * byteCount;
	
	data = new unsigned char [imageSize];
	
	std::ofstream oFile("D:/Programming/Particle Studio 2/ps_tga_log.txt",std::ios::app);
	oFile << "readData():\n";
	oFile << "data Size: " << imageSize << '\n';
	oFile << "ColourMapType: " << (int) header.colourmaptype << '\n';
	oFile << "ImageType: " << (int) header.imagetype << '\n';
	oFile.close();
	
	inFile.open(filename,std::ios::binary);
	inFile.seekg(18,std::ios::beg);
	inFile.read((char*)data,imageSize);
	inFile.close();
	return true;
	
}

/*
bool loadTGA(const char *filename, STGA& tgaFile)
{
	FILE *file;
	unsigned char type[4];
	unsigned char info[6];

        file = fopen(filename, "rb");

        if (!file)
		return false;

	fread (&type, sizeof (char), 3, file);
	fseek (file, 12, SEEK_SET);
	fread (&info, sizeof (char), 6, file);

	//image type either 2 (color) or 3 (greyscale)
	if (type[1] != 0 || (type[2] != 2 && type[2] != 3))
	{
		fclose(file);
		return false;
	}

	tgaFile.width = info[0] + info[1] * 256;
	tgaFile.height = info[2] + info[3] * 256;
	tgaFile.byteCount = info[4] / 8;

	if (tgaFile.byteCount != 3 && tgaFile.byteCount != 4) {
		fclose(file);
		return false;
	}

	long imageSize = tgaFile.width * tgaFile.height
 * tgaFile.width * tgaFile.byteCount;

	//allocate memory for image data
	tgaFile.data = new unsigned char[imageSize];

	//read in image data
	fread(tgaFile.data, sizeof(unsigned char), imageSize, file);

	//close file
	fclose(file);

	return true;
}
*/
