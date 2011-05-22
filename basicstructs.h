#ifndef _BASIC_STRUCTS
#define _BASIC_STRUCTS

struct vec2D {
    int x;
    int y;
};

struct vec3D {
    int x;
    int y;
    int z;
};

struct RGBf {
    float R;
    float G;
    float B;
};

int itoa(int i, char *s);
int ftoa(float i, char *s);
int removeTrailingZeros(char *s);

#endif // _BASIC_STRUCTS
