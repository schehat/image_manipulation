#ifndef YCBCR_H
#define YCBCR_H

#include "RGB.h"

class YCbCr
{
public:
    float Y;
    float Cb;
    float Cr;
    YCbCr(float y, float cb, float cr);
    float Min(float a, float b);
    float Max(float a, float b);
    RGB YCbCrToRGB(YCbCr ycbcr) ;
};

#endif // YCBCR_H
