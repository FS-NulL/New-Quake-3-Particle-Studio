
#include "particleSystem.h"

#include <ctime> // For Clock ticks
#include <cmath> // For sin/cosine functions 
#include <cstdlib> // Rand
#include <windows.h>
#include <gl/gl.h>

#include <fstream>

#define PI 3.14159265


particleSystem::particle::particle()
{
  active = false;
}

particleSystem::particleSystem()
{
  lastResetTime = clock();
  height = 128;
  heightVar = 16;
  radius = 256;
  radiusVar = 16;
  size= 64;
  hz = 0.3125;
  startAlpha = 1.0f;
  endAlpha = -1.0f;
  startRGB = 1.0;
  endRGB = -1.0;
  numParticles = 100;
  phaseGrouping = 1.0;
  startAngle = 0;
  endAngle = 359;
  angleGrouping = 0.0f;
  std::strcpy(shaderBaseName,"textures/ps_test/pstest"); // If This isnt set dont export shaders
  std::strcpy(textureName,"textures/common/caulk.tga");
  zWaveform = WAVE_SIN;
  xyWaveform = WAVE_SAWTOOTH;
  srcBlend = GL_ONE;
  dstBlend = GL_ONE;
  alphagen_waveform = WAVE_CONST;
  rgbgen_waveform = WAVE_SQUARE;
  scale_waveform = WAVE_CONST;
  emitterZvar = 0;
  emitterXYVar = 0;
  masterPhase = 0;
  xyPhase = 0;
  alphaPhase = 0;
  rgbPhase = 0;
  scalePhase = 0;
  alphagen_phase_option = ALPHA_MASTER;
  rgbgen_phase_option = RGB_MASTER;
  rotSpeed = 0;
  rotSpeedVar = 0;
  startStretch = 1;
  endStretch = 1;
  fireworkMode = false;
  sort=0;
}

int particleSystem::buildParticles()
{
  srand(time(NULL));
  int i= numParticles;
  particles[i].active = false;
  while (i)
  {
    i--;
    particles[i].active=true;
    
    // Phase
    particles[i].phase = pow( (float) i / (float)numParticles ,phaseGrouping)  + masterPhase;
    
    // Angle
    if ((endAngle - startAngle) == 0) particles[i].angle = (float)startAngle;
    else 
    {
      float x = ((float)(rand() % 1000) )/ 1000;
      float WF = angleGrouping*(0.5*pow( 2*(x-0.5) ,3 ) + 0.5 ) + (1-angleGrouping) * x;
      particles[i].angle = (WF * abs(endAngle - startAngle)) + startAngle;
    }
    particles[i].aXRatio = cos((float)particles[i].angle * PI/180);
    particles[i].aYRatio = sin((float)particles[i].angle * PI/180);   
    
    // Height
    
    if (heightVar > 0) particles[i].height = height - heightVar + (rand() % ((int)(2*heightVar)));
    else particles[i].height = height;
    
    /*int hv = (int)((float)height * heightVar);
    if (hv==0) particles[i].height = height;
    else
    {
      hv = rand() % hv;
      particles[i].height = height + hv - (0.5 * ((float)height*heightVar));
    }*/
    
    particles[i].emitterXYBase = particles[i].emitterZBase = 0.0f;
    
    particles[i].ampz = 1.0f;
    if (emitterZvar<=0) particles[i].emitterZBase = 0;
    else
    {
      if (particles[i].height>0) particles[i].emitterZBase = ( (float)(rand() % ((int)(2*emitterZvar))) - emitterZvar) / particles[i].height;
      else
      {
        particles[i].ampz = 0.0f;
        particles[i].height = ( (float)(rand() % ((int)(2*emitterZvar))) - emitterZvar);
        particles[i].emitterZBase = 1.0f;
      }
    }
    
    // Frequency
    particles[i].hz = hz;
    
    // Radius
    
    if (height ==0) particles[i].radius = radius;
    else particles[i].radius = radius * (particles[i].height / height);
    if (radiusVar > 0) particles[i].radius = particles[i].radius - radiusVar + (rand() % ((int) (2*radiusVar)));
    
    if (fireworkMode)
    {
      // radius is a function of height
      // then add radius Var
      //std::ofstream oFile("firew.txt",std::ios::app);
      particles[i].radius = radius;
      float temp = cos( asin( (particles[i].height)/(height+heightVar) ) );
      particles[i].radius *= temp;
      if (radiusVar > 0) particles[i].radius = particles[i].radius - radiusVar + (rand() % ((int) (2*radiusVar)));
      //oFile.close();
    }
    
    /*int rv = (int)((float)radius * radiusVar);
    if (rv>0) 
    {
      rv = rand() % rv;
      particles[i].radius = particles[i].radius + rv - (0.5* ((float)radius*radiusVar));
    }*/
    //particles[i].emitterXYBase = 0.0f;
    
    
    // XY Emitter Var
    particles[i].ampxy = 1.0f;
    if (emitterXYVar<=0) particles[i].emitterXYBase = 0;
    else
    {
      if (particles[i].radius>0) particles[i].emitterXYBase = ( (float)(rand() % ((int)(2*emitterXYVar))) - emitterXYVar) / particles[i].radius;
      else
      {
        particles[i].ampxy = 0.0f;
        particles[i].emitterXYBase = 1.0f;
        particles[i].radius = ( (float)(rand() % ((int)(2*emitterXYVar))) - emitterXYVar);
      }
    }
    
    //std::ofstream oFile("emitterxy.txt",std::ios::app);
    //oFile << i << ' ' << particles[i].radius << ' ' << particles[i].emitterXYBase << ' ' << particles[i].ampxy << ' ' << emitterXYVar << '\n';
    //oFile.close();
    
    //Scale / stretch
    
    particles[i].scale_amp  = 0;
    particles[i].scale_base = 1;
  
    if (scale_waveform == WAVE_SIN)
    {
      particles[i].scale_base = startStretch;
      particles[i].scale_amp = endStretch;
    }
    if (scale_waveform == WAVE_SQUARE)
    {
      particles[i].scale_base = (startStretch+endStretch)/2;
      particles[i].scale_amp = (startStretch - endStretch) / 2;
    }
    if (scale_waveform == WAVE_TRIANGLE)
    {
      particles[i].scale_base = startStretch;
      particles[i].scale_amp = endStretch;
    }
    if (scale_waveform == WAVE_SAWTOOTH)
    {
      particles[i].scale_base = startStretch;
      particles[i].scale_amp = endStretch - startStretch;
    }
    if (scale_waveform == WAVE_INVSAWTOOTH)
    {
      particles[i].scale_base = endStretch;
      particles[i].scale_amp = startStretch - endStretch;
    }
    if (scale_waveform == WAVE_CONST)
    {
      particles[i].scale_base = startStretch;
    }
    
    // Rotation
    
    if (rotSpeedVar == 0) particles[i].rotSpeed = (float) rotSpeed;
    else 
    {
      particles[i].rotSpeed = (float) (rotSpeed - rotSpeedVar + ( rand() % (2*rotSpeedVar) ));
    }
    
    // RGBGEN
    if (rgbgen_waveform == WAVE_SIN)
    {
      particles[i].rgbgen_base = startRGB;
      particles[i].rgbgen_amp = endRGB;
    }
    if (rgbgen_waveform == WAVE_SQUARE)
    {
      particles[i].rgbgen_base = (startRGB+endRGB)/2;
      particles[i].rgbgen_amp = (startRGB - endRGB) / 2;
    }
    if (rgbgen_waveform == WAVE_TRIANGLE)
    {
      particles[i].rgbgen_base = startRGB;
      particles[i].rgbgen_amp = endRGB;
    }
    if (rgbgen_waveform == WAVE_SAWTOOTH)
    {
      particles[i].rgbgen_base = startRGB;
      particles[i].rgbgen_amp = endRGB - startRGB;
    }
    if (rgbgen_waveform == WAVE_INVSAWTOOTH)
    {
      particles[i].rgbgen_base = endRGB;
      particles[i].rgbgen_amp = startRGB - endRGB;
    }
    if (rgbgen_waveform == WAVE_CONST)
    {
      particles[i].rgbgen_base = startRGB;
    }
    
    // ALPHAGEN
    
    if (alphagen_waveform == WAVE_SIN)
    {
      particles[i].alphagen_base = startAlpha;
      particles[i].alphagen_amp = endAlpha;
    }
    if (alphagen_waveform == WAVE_SQUARE)
    {
      particles[i].alphagen_base = (startAlpha+endAlpha)/2;
      particles[i].alphagen_amp =  (startAlpha-endAlpha)/2;
    }
    if (alphagen_waveform == WAVE_TRIANGLE)
    {
      particles[i].alphagen_base = startAlpha;
      particles[i].alphagen_amp =  endAlpha;
    }
    if (alphagen_waveform == WAVE_SAWTOOTH)
    {
      particles[i].alphagen_base = startAlpha;
      particles[i].alphagen_amp = endAlpha - startAlpha;
    }
    if (alphagen_waveform == WAVE_INVSAWTOOTH)
    {
      particles[i].alphagen_base = endAlpha;
      particles[i].alphagen_amp = startAlpha - endAlpha;
    }
    if (alphagen_waveform == WAVE_CONST)
    {
      particles[i].alphagen_base = startAlpha;
      particles[i].alphagen_amp = 0;
    }
  }
  return 0;
}

particleSystem::~particleSystem()
{
}

float particleSystem::calcTimeN()
{
	// Need to change this to ticks from last reset ticks //lastResetTime
  // Get Ticks Since Begining of Exceqution 
  clock_t clockNow = clock() - lastResetTime;
  
  // Convert to normalised floating point number
  timeN = (float) clockNow / CLOCKS_PER_SEC;
  
  return timeN;
}
void particleSystem::getPositionVector(int i, float &x, float &y, float &z)
{
  // Return position on particle to use in gltTanslatef()
  // x/y = sawtooth wave
  // z = sin wave height*sin (2PIft+phase)
  // double temp; // not used in new system
  
  // **********************************************************
  // Position Vector is DISPLACEMENT VECTOR * WAVEFORM FUNCTION
  // **********************************************************
 
  // SAWTOOTH
  // old system
  //x = (int) ((double)particles[i].aXRatio * (double)particles[i].radius * modf( (double)(hz*timeN + particles[i].phase) ,&temp));
  //y = (int) ((double)particles[i].aYRatio * (double)particles[i].radius * modf( (double) (hz*timeN + particles[i].phase) ,&temp));
  // new waveform system
  x =  ( (float)(particles[i].aXRatio * particles[i].radius) * waveform(xyWaveform, particles[i].emitterXYBase, particles[i].ampxy , particles[i].phase+xyPhase, hz, timeN )  );
  y =  ( (float)(particles[i].aYRatio * particles[i].radius) * waveform(xyWaveform, particles[i].emitterXYBase, particles[i].ampxy , particles[i].phase+xyPhase, hz, timeN )  );
  // SIN
  //z = (int) (particles[i].height * sin( 2*PI* normalise( hz*timeN + particles[i].phase) ) );
  z =  (particles[i].height * waveform(zWaveform, particles[i].emitterZBase, particles[i].ampz, particles[i].phase, hz, timeN));
  
}

inline float particleSystem::normalise(float f)
{
  double temp;
  return modf(f,&temp);
}

void particleSystem::rotateTex(float &x1,float &x2,float &x3,float &x4,float &y1,float &y2,float &y3,float &y4,float angle)
{
  //std::ofstream oFile("rot.txt",std::ios::out);
  //oFile << x1 <<',' << y1 << "  " << x2 << ',' << y2 << "  " << x3 <<',' << y3 << "  " << x4 << ',' << y4 << "   " << angle << '\n';
  float nx1,nx2,nx3,nx4,ny1,ny2,ny3,ny4;
  
  float ca = cos(angle*PI/180);
  float sa = sin(angle*PI/180);
  
  nx1 = x1 * ca - y1 * sa;
  ny1 = y1 * ca + x1 * sa;
  
  nx2 = x2 * ca - y2 * sa;
  ny2 = y2 * ca + x2 * sa;
  
  nx3 = x3 * ca - y3 * sa;
  ny3 = y3 * ca + x3 * sa;
  
  nx4 = x4 * ca - y4 * sa;
  ny4 = y4 * ca + x4 * sa;
  
  //oFile << nx1 <<',' << ny1 << "  " << nx2 << ',' << ny2 << "  " << nx3 << ',' << ny3 << "  " << nx4 << ',' << ny4 << '\n';
  
  x1 = nx1;
  x2 = nx2;
  x3 = nx3;
  x4 = nx4;
  y1 = ny1;
  y2 = ny2;
  y3 = ny3;
  y4 = ny4;
  //oFile.close();
}


void particleSystem::drawParticle(int i)
{

  float aRange = endAlpha - startAlpha;
  float vAlpha; // = normalise(particles[i].hz * timeN + particles[i].phase) * aRange + startAlpha;
  float rgb;
  float stretch;

  vAlpha = waveform(alphagen_waveform, particles[i].alphagen_base, particles[i].alphagen_amp, particles[i].phase+alphaPhase, hz, timeN);
  rgb = waveform(rgbgen_waveform, particles[i].rgbgen_base, particles[i].rgbgen_amp, particles[i].phase+rgbPhase, hz, timeN);
  if (scale_waveform != WAVE_CONST) stretch = waveform(scale_waveform, particles[i].scale_base,particles[i].scale_amp, particles[i].phase+scalePhase, hz , timeN);
  else stretch = 1.0f;
  
  glBlendFunc(srcBlend,dstBlend);
  
  glPushMatrix();
  // Stretch / Rotate Fuctionallity
  glRotatef(particles[i].rotSpeed*timeN,0,0,-1);
  
  if (stretch <= 1) glScalef(stretch,stretch,0);
  else
  {
    // bring in texture coords
  }

  glBegin(GL_QUADS);
    // use glColor4f(f,f,f,f) for rgb + alpha
    glColor4f( rgb, rgb, rgb ,vAlpha);
            
    if (stretch > 1.0f)
    {
      stretch = 0.5 - ((float) 1) / (2*stretch);
      glTexCoord2f(0.0+ stretch, 0.0+stretch); glVertex3f( -(size/2), -(size/2),0.0f); 
      glTexCoord2f(1.0-stretch, 0.0+stretch); glVertex3f( (size/2), -(size/2), 0.0f );
      glTexCoord2f(1.0-stretch, 1.0-stretch); glVertex3f( (size/2),(size/2),0.0f); 
      glTexCoord2f(0.0+stretch, 1.0-stretch); glVertex3f( -(size/2),(size/2),0.0f);
    }
    else
    {
      glTexCoord2f(0.0, 0.0 ); glVertex3f( -(size/2), -(size/2),0.0f); 
      glTexCoord2f(1.0, 0.0 ); glVertex3f( (size/2), -(size/2), 0.0f );
      glTexCoord2f(1.0, 1.0); glVertex3f( (size/2),(size/2),0.0f); 
      glTexCoord2f(0.0, 1.0); glVertex3f( -(size/2),(size/2),0.0f);
    }
  glEnd();
  glPopMatrix();
}

float particleSystem::waveform(int type, float base, float amp, float phase, float freq, float time)
{  
  float value;
  float nom = normalise(freq*time + phase);
  switch (type) 
  {
    case WAVE_SIN:
      value = base + amp * sin( 2*PI* nom );
      break;
      
    case WAVE_SQUARE:
      if ( nom < 0.5) value = base + amp;
      else value = base - amp;
      break;
      
    case WAVE_TRIANGLE: 
      if (nom<0.25) value = base + 4*amp*nom;
      else if (nom<0.75) value = base + amp - 4*(nom-0.25)*amp;
      else value = base + 4*amp*(nom-1);
      
      break;
      
    case WAVE_SAWTOOTH:
      value = base + amp * nom;
      break;
      
    case WAVE_INVSAWTOOTH:
      value = base + amp - (amp * nom);
      break;
      
    default: // WAVE CONST
      value = base;
      break;
  }
  
  return value;    
}
/*
textures/ps_test/test                             // foldername and shadername are variable later shaders have zz<name>_X
{
	qer_editorimage textures/null_twist/corona.tga  // Variable path
	q3map_cloneshader textures/ps_test/ztest_1      // Generate baased on current number, but not if current = (numParticles -1)
	surfaceparm nonsolid                            // not optional
	surfaceparm trans                               // not optional
	deformVertexes move 0 0 64 sin 0 1 0 1          // Deform 1 remove if (x y z) < 1 or base + amp < 1
	deformVertexes move 0 64 0 sin 0 1 0.25 1       // Deform 2 remove if (x y z) < 1 or base + amp < 1
	deformvertexes autosprite                       // not optional
	cull disable                                    // not optiona
	{
		map textures/null_twist/corona.tga            // you might need clampmap later
		blendfunc add                                 // user specifies both src and dest, so this line will always be full form 
		//rgbgen wave sin 0 1 0 1
		//alphagen wave sin 0 1 0 1
	}
}
*/

bool particleSystem::buildShaderFile(char *filename)
{
  //std::ofstream debug("debug.txt");
  
  // CHecks must be done first
  // like shader name must be set

  // For multi particle system, the output file must not be truncated
  
	std::ofstream outFile(filename,std::ios::out|std::ios::app);
  if (outFile.is_open())
  {
    
    time_t rawtime;
    tm * timeinfo;
    time( &rawtime );
    timeinfo = localtime( &rawtime );
    
    outFile << "// Generated by Frozen Sand Particle Studio\n// " << asctime(timeinfo);
	outFile << "// Use on a " << size << "x" << size << " unit brush face\n";
    outFile << "\n\n";
    char newShaderName[64];
    char sepLocation;
    char shader[64];
    // GO
    for (int i=0; i<numParticles; i++)
    {
      // Shader's Title
      sepLocation = std::strcspn(&shaderBaseName[9],"/");
      std::strncpy(newShaderName,shaderBaseName,sepLocation + 10);
      newShaderName[sepLocation+10]=0; // ADD NULL-TERMINATOR
      std::strcpy(shader,&shaderBaseName[sepLocation+10]);
      std::strcat(newShaderName,"zz");
      std::strcat(newShaderName,shader);
      std::strcat(newShaderName,"_");
      
      if (i == 0) outFile << shaderBaseName << '\n';
      else outFile << newShaderName << i << '\n';
      outFile << "{\n";
      //char tempStr[64];
      //char imageName[64];
      //char basePathLocation;
      //std::strcpy(tempStr,std::strstr(textureName,"q3ut4"));
      //char *strPtr=&tempStr[6];
      //std::strcpy(tempStr,&tempStr[6]);
      // Working on pulling qer_editorimage from image path
      outFile << "\tqer_editorimage " << textureName << '\n';
      if ((i+1)<numParticles) outFile << "\tq3map_cloneshader " << newShaderName << i+1 << '\n';
      outFile << "\tsurfaceparm nonsolid\n\tsurfaceparm trans\n\tsurfaceparm nomarks\n\tsurfaceparm nodlight\n";
      // Z axis movement (Vertical)
      //Deformvertexes 0 0 p.h wave FUNC 0 1 p ps
      outFile << "\tdeformvertexes move 0 0 " << particles[i].height << " ";
      if (zWaveform == WAVE_SIN) outFile << "sin ";
      else if (zWaveform == WAVE_SQUARE) outFile << "square ";
      else if (zWaveform == WAVE_TRIANGLE) outFile << "triangle ";
      else if (zWaveform == WAVE_SAWTOOTH) outFile << "sawtooth ";
      else if (zWaveform == WAVE_INVSAWTOOTH) outFile << "inversesawtooth ";
      outFile << particles[i].emitterZBase << ' ' << particles[i].ampz << ' '<< particles[i].phase << ' ' << hz << '\n';
      
      // XY Plane Movement
      // Deformvertexes X Y 0 wave FUNC 0 1 phase hz
      outFile << "\tdeformvertexes move " << particles[i].aXRatio * particles[i].radius << ' ' << particles[i].aYRatio * particles[i].radius << " 0 ";
      if (xyWaveform == WAVE_SIN) outFile << "sin ";
      else if (xyWaveform == WAVE_SQUARE) outFile << "square ";
      else if (xyWaveform == WAVE_TRIANGLE) outFile << "triangle ";
      else if (xyWaveform == WAVE_SAWTOOTH) outFile << "sawtooth ";
      else if (xyWaveform == WAVE_INVSAWTOOTH) outFile << "inversesawtooth ";
      outFile << particles[i].emitterXYBase << ' ' << particles[i].ampxy << ' ' << particles[i].phase+xyPhase << " " << hz << '\n';
      outFile << "\tdeformvertexes autosprite\n";
	  if (sort) outFile << "\tsort " << (int) sort <<"\n";
      
      outFile << "\tcull disable\n\t{\n\t\tclampmap " << textureName << '\n';
      outFile << "\t\tblendfunc ";
      if (srcBlend == GL_ONE) outFile << "GL_ONE ";
      if (srcBlend == GL_ZERO) outFile << "GL_ZERO ";
      if (srcBlend == GL_DST_COLOR) outFile << "GL_DST_COLOR ";
      if (srcBlend == GL_ONE_MINUS_DST_COLOR) outFile << "GL_ONE_MINUS_DST_COLOR ";
      if (srcBlend == GL_SRC_ALPHA) outFile << "GL_SRC_ALPHA ";
      if (srcBlend == GL_ONE_MINUS_SRC_ALPHA) outFile << "GL_ONE_MINUS_SRC_ALPHA ";
      
      if (dstBlend == GL_ONE) outFile << "GL_ONE\n";
      if (dstBlend == GL_ZERO) outFile << "GL_ZERO\n";
      if (dstBlend == GL_SRC_COLOR) outFile << "GL_SRC_COLOR\n";
      if (dstBlend == GL_ONE_MINUS_SRC_COLOR) outFile << "GL_ONE_MINUS_SRC_COLOR\n";
      if (dstBlend == GL_SRC_ALPHA) outFile << "GL_SRC_ALPHA\n";
      if (dstBlend == GL_ONE_MINUS_SRC_ALPHA) outFile << "GL_ONE_MINUS_SRC_ALPHA\n";
      
      //rgbgen wave FUNC b a p f
      outFile << "\t\trgbgen ";
      if (rgbgen_waveform == WAVE_SIN) outFile << "wave sin ";
      if (rgbgen_waveform == WAVE_SQUARE) outFile << "wave square ";
      if (rgbgen_waveform == WAVE_TRIANGLE) outFile << "wave triangle ";
      if (rgbgen_waveform == WAVE_SAWTOOTH) outFile << "wave sawtooth ";
      if (rgbgen_waveform == WAVE_INVSAWTOOTH) outFile << "wave inversesawtooth ";
      if (rgbgen_waveform == WAVE_CONST) 
      {
        outFile << "const ( " << clampFloat( particles[i].rgbgen_base,0.0f,1.0f) << " " << clampFloat( particles[i].rgbgen_base,0.0f,1.0f) << " " << clampFloat( particles[i].rgbgen_base,0.0f,1.0f) << " )\n";
      }
      else
      {
        //base amp phase freq
        outFile << particles[i].rgbgen_base << " " << particles[i].rgbgen_amp << " " << particles[i].phase+rgbPhase << " " << hz << '\n';
      }
      
      
      //alphagen
      outFile << "\t\talphagen ";
      if (alphagen_waveform == WAVE_SIN) outFile << "wave sin ";
      if (alphagen_waveform == WAVE_SQUARE) outFile << "wave square ";
      if (alphagen_waveform == WAVE_TRIANGLE) outFile << "wave triangle ";
      if (alphagen_waveform == WAVE_SAWTOOTH) outFile << "wave sawtooth ";
      if (alphagen_waveform == WAVE_INVSAWTOOTH) outFile << "wave inversesawtooth ";
      if (alphagen_waveform == WAVE_CONST) 
      {
        outFile << "const " << clampFloat(particles[i].alphagen_base,0.0f,1.0f) << "\n" ;
      }
      else
      {
        outFile << particles[i].alphagen_base << " " << particles[i].alphagen_amp << " " << particles[i].phase+alphaPhase << " " << hz << '\n';
      }
      
      // tcmod rotate
      
      if (particles[i].rotSpeed != 0) outFile << "\t\ttcMod rotate " << particles[i].rotSpeed << '\n';
      
      //tcmod stretch
      
      if (scale_waveform != WAVE_CONST) 
      {
        outFile << "\t\ttcMod stretch ";
        if (scale_waveform == WAVE_SIN) outFile << "sin ";
        if (scale_waveform == WAVE_SQUARE) outFile << "square ";
        if (scale_waveform == WAVE_TRIANGLE) outFile << "triangle ";
        if (scale_waveform == WAVE_SAWTOOTH) outFile << "sawtooth ";
        if (scale_waveform == WAVE_INVSAWTOOTH) outFile << "inversesawtooth ";
        outFile << particles[i].scale_base << " " << particles[i].scale_amp << " " << particles[i].phase+scalePhase << " " << hz << '\n';
      }
      
      outFile << "\t}\n}\n\n"; 
      
    }
  }
  else return false;
  
  outFile.close();
  //debug.close();
  return true;
}


float particleSystem::getSize()
{
  return size;
}


bool particleSystem::setSize(float s)
{
  if (s > 0) size = s;
  return 0;
}

float particleSystem::getHeight()
{
  return height;
}

bool particleSystem::setHeight(float h)
{
  if (h >= 0) height = h;
  return 0;
}

int particleSystem::getHeightVar()
{
  return heightVar;
}

bool particleSystem::setHeightVar(int h)
{
  heightVar = h;
  return 0;
}

float particleSystem::getEmitterZ()
{
  return emitterZvar;
}

bool particleSystem::setEmitterZ(float h)
{
  if (h >= 0) emitterZvar = h;
  return 0;
}

float particleSystem::getFrequency()
{
  return hz;
}

bool particleSystem::setFrequency(float h)
{
  if (h >= 0) hz = h;
  return 0;
}

float particleSystem::getPhaseGrouping()
{
  return phaseGrouping;
}

bool particleSystem::setPhaseGrouping(float h)
{
  if (h >= 0) phaseGrouping = h;
  return 0;
}

float particleSystem::getMasterPhase()
{
  return masterPhase;
}

bool particleSystem::setMasterPhase(float h)
{
  /*if (h >= 0)*/ masterPhase = h;
  return 0;
}

int particleSystem::getZWaveform()
{
  return zWaveform;
}

bool particleSystem::setZWaveform(int i)
{
  zWaveform = i;
  return 0;
}

bool particleSystem::setXYWaveform(int i)
{
  xyWaveform = i;
  return 0;
}

float particleSystem::getRadius()
{
  return radius;
}

bool particleSystem::setRadius(float r)
{
  if(r>=0) radius = r;
  return 0;
}

int particleSystem::getRadiusVar()
{
  return radiusVar;
}

bool particleSystem::setRadiusVar(int r)
{
  radiusVar = r;
  return 0;
}

float particleSystem::getEmitterXY()
{
  return emitterXYVar;
}

bool particleSystem::setEmitterXY(float h)
{
  if (h >= 0) emitterXYVar = h;
  return 0;
}

int particleSystem::getStartAngle()
{
  return startAngle;
}

bool particleSystem::setStartAngle(int a) // start angle is always lower
{
  startAngle = a;
  if (startAngle>endAngle) endAngle = startAngle;
  return 0;
}

int particleSystem::getEndAngle()
{
  return endAngle;
}

bool particleSystem::setEndAngle(int a)
{
  endAngle = a;
  if (endAngle<startAngle) startAngle = endAngle;
  return 0;
}

float particleSystem::getXYPhase()
{
  return xyPhase;
}

bool particleSystem::setXYPhase(float p)
{
  xyPhase = p;
  return 0;
}

int particleSystem::getXYWaveform()
{
  return xyWaveform;
}

float particleSystem::getAngleGrouping()
{
  return angleGrouping;
}

bool particleSystem::setAngleGrouping(float g)
{
  if ((g>=0)&&(g<=1)) angleGrouping = g;
  return 0;
}

float particleSystem::getRGB1()
{
  return startRGB;
}

float particleSystem::getRGB2()
{
  return endRGB;
}

float particleSystem::getAlpha1()
{
  return startAlpha;
}

float particleSystem::getAlpha2()
{
  return endAlpha;
}

bool particleSystem::setRGB1(float v)
{
  startRGB = v;
  return 0;
}

bool particleSystem::setRGB2(float v)
{
  endRGB = v;
  return 0;
}

bool particleSystem::setAlpha1(float v)
{
  startAlpha = v;
  return 0;
}

bool particleSystem::setAlpha2(float v)
{
  endAlpha = v;
  return 0;
}

int particleSystem::getRGBWave()
{
  return rgbgen_waveform;
}

int particleSystem::getAlphaWave()
{
  return alphagen_waveform;
}

bool particleSystem::setRGBWave(int i)
{
  rgbgen_waveform = i;
  return 0;
}

bool particleSystem::setAlphaWave(int i)
{
  alphagen_waveform = i;
  return 0;
}

bool particleSystem::setSrcBlend(int i)
{
  srcBlend = i;
  return 0;
}

bool particleSystem::setDstBlend(int i)
{
  dstBlend = i;
  return 0;
}


int particleSystem::getAlphagenPhaseOption()
{
  return alphagen_phase_option;
}

int particleSystem::getRgbgenPhaseOption()
{
  return rgbgen_phase_option;
}

bool particleSystem::setAlphagenPhaseOption(int i)
{
  alphagen_phase_option = i;
  return 0;
}

bool particleSystem::setRgbgenPhaseOption(int i)
{
  rgbgen_phase_option = i;
  return 0;
}

float particleSystem::getRGBPhase()
{
  return rgbPhase;
}

bool particleSystem::setRGBPhase(float p)
{
  rgbPhase = p;
  return 0;
}

float particleSystem::getAlphaPhase()
{
  return alphaPhase;
}

bool particleSystem::setAlphaPhase(float p)
{
  alphaPhase = p;
  return 0;
}

float particleSystem::getStretch1()
{
  return startStretch;
}

float particleSystem::getStretch2()
{
  return endStretch;
}

int particleSystem::getStretchWave()
{
  return scale_waveform;
}

float particleSystem::getStretchPhase()
{
  return scalePhase;
}

bool particleSystem::setStretch1(float s)
{
  startStretch = s;
  return 0;
}

bool particleSystem::setStretch2(float s)
{
  endStretch = s;
  return 0;
}

bool particleSystem::setStretchWave(int i)
{
  scale_waveform = i;
  return 0;
}

bool particleSystem::setStretchPhase(float p)
{
 scalePhase = p; 
  return 0;
}

int particleSystem::getRotSpeed()
{
  return rotSpeed;
}

int particleSystem::getRotSpeedVar()
{
  return rotSpeedVar;
}

bool particleSystem::setRotSpeed(int s)
{
  rotSpeed = s;
  return true;
}

bool particleSystem::setRotSpeedVar(int s)
{
  rotSpeedVar = s;
  return true;
}

bool particleSystem::setNumParticles(int p)
{
  if (p>0 && p<=4096) numParticles = p;
  return 0;
}

bool particleSystem::getFireworkMode()
{
  return fireworkMode;
}

bool particleSystem::setFireworkMode(bool mode)
{
  fireworkMode = mode;
  return 0;
}


float particleSystem::clampFloat(float input,float min,float max)
{
	if (input > max) input = max;
	if (input < min) input = min;
	return input;
}

time_t particleSystem::resetTime()
{
	return lastResetTime = clock();
}