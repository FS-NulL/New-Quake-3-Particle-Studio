#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <ctime>  // For Clock Timing

#include "basicstructs.h"

enum  {
  WAVE_SIN,
  WAVE_SQUARE,
  WAVE_TRIANGLE,
  WAVE_SAWTOOTH,
  WAVE_INVSAWTOOTH,
  WAVE_CONST
};

enum {
  RGB_MASTER,
  RGB_XY,
  RGB_CUSTOM
};

enum {
  ALPHA_MASTER,
  ALPHA_XY,
  ALPHA_CUSTOM
};

class particleSystem {


  int startAngle;
  int endAngle;
  float timeN; // Normalised Time 0-1
  
  float height;      // Std particle height
  int heightVar;   // % variance in height
  float radius;      // Radius
  int radiusVar;   // Radius Variation
  float speed;       // Pparticle Speed  // NEVER USED
  float hz;          // Frequency
  float speedVar;    // Speed Variance // NEVER USED
  float startAlpha;  // Alpha at t=0;
  float endAlpha;    // Alpha at t=1; NOTE:: particles disappear at t=0.5
  float startRGB;
  float endRGB;
  float size;
  float phaseGrouping;
  float masterPhase;
  float xyPhase;
  int   zWaveform;
  int   xyWaveform;
  int alphagen_waveform;
  int rgbgen_waveform;
  int scale_waveform;
  float emitterZvar;
  float emitterXYVar;
  float angleGrouping;
  int rgbgen_phase_option;
  int alphagen_phase_option;
  float rgbPhase;
  float alphaPhase;
  float scalePhase;
  int rotSpeed;
  int rotSpeedVar;
  float startStretch;
  float endStretch;
  
  
  bool fireworkMode;
  
  class particle {

   public: 
    float phase;
    float angle;
    float aXRatio;  // Z angle vector decomposition
    float aYRatio;
    float height;
    //float heightBaseVar; // Low Percentage ie 0.1 NOT UNITS
    float hz;
    float radius;
    //float emitterBaseSpread; // low Percentage ie 0.1 NOT UNITS
    float speed; // if Required, or just set hz
    bool active;
    float alphagen_base;
    float alphagen_amp;
    float rgbgen_base;
    float rgbgen_amp;
    float emitterZBase; // in game units
    float emitterXYBase;
    particle();
    float ampz; // fix for emitter variance code, by setting amp to 0 and displacement vector to emitter variance
    float ampxy;
    float scale_base;
    float scale_amp;
    float rotSpeed;
  };
  
  particle particles[4096];

  float clampFloat(float input,float min,float max);

  time_t lastResetTime;
  
   
  
 public:
  particleSystem();
  ~particleSystem();
  int buildParticles();
  float calcTimeN();
  time_t resetTime();
  void getPositionVector(int i, float &x, float &y, float &z); // Pass particle number
  void applyRotation(float &x, float &y, float &z);
  float normalise(float);
  void drawParticle(int i);
  float waveform(int type, float base, float amp, float phase, float freq, float time);
  bool buildShaderFile(char *filename);
  void rotateTex(float &x1,float &x2,float &x3,float &x4,float &y1,float &y2,float &y3,float &y4,float angle);
  
  char sort;
  float zBase;
  float rotx,roty,rotz;
  int numParticles;
  int srcBlend;
  int dstBlend;
  char textureName[256];
  char shaderBaseName[64];
  bool setNumParticles(int p);
  float getSize();
  bool setSize(float s);
  float getHeight();
  bool setHeight(float h);
  int getHeightVar();
  bool setHeightVar(int h);
  float getEmitterZ();
  bool setEmitterZ(float h);
  float getEmitterXY();
  bool setEmitterXY(float h);
  float getFrequency();
  bool setFrequency(float f);
  float getPhaseGrouping();
  bool setPhaseGrouping(float f);
  float getMasterPhase();
  bool setMasterPhase(float f);
  float getXYPhase();
  bool setXYPhase(float p);
  int getZWaveform();
  bool setZWaveform(int i);
  bool setXYWaveform(int i);
  float getRadius();
  bool setRadius(float r);
  int getRadiusVar();
  bool setRadiusVar(int r);
  int getStartAngle();
  bool setStartAngle(int a);
  int getEndAngle();
  bool setEndAngle(int a);
  int getXYWaveform();
  float getAngleGrouping();
  bool setAngleGrouping(float g);
  float getRGB1();
  float getRGB2();
  float getAlpha1();
  float getAlpha2();
  bool setRGB1(float v);
  bool setRGB2(float v);
  bool setAlpha1(float v);
  bool setAlpha2(float v);
  int getRGBWave();
  int getAlphaWave();
  bool setRGBWave(int i);
  bool setAlphaWave(int i);
  bool setSrcBlend(int i);
  bool setDstBlend(int i);
  int getAlphagenPhaseOption();
  int getRgbgenPhaseOption();
  bool setAlphagenPhaseOption(int i);
  bool setRgbgenPhaseOption(int i);
  float getRGBPhase();
  bool setRGBPhase(float p);
  float getAlphaPhase();
  bool setAlphaPhase(float p);
  
  float getStretch1();
  float getStretch2();
  int getStretchWave();
  float getStretchPhase();
  
  int getRotSpeed();
  int getRotSpeedVar();
  
  bool setStretch1(float s);
  bool setStretch2(float s);
  bool setStretchWave(int i);
  bool setStretchPhase(float p);
  
  bool setRotSpeed(int s);
  bool setRotSpeedVar(int s);
  
  bool getFireworkMode();
  bool setFireworkMode(bool mode);
  
};


#endif // PARTICLESYSTEM_H

