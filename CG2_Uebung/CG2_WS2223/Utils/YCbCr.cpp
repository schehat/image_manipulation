#include "YCbCr.h"

YCbCr::YCbCr(float y, float cb, float cr)
{
        Y = y;
        Cb = cb;
        Cr = cr;
}

float YCbCr::Min(float a, float b) {
    return a <= b ? a : b;
}

float YCbCr::Max(float a, float b) {
    return a >= b ? a : b;
}

RGB YCbCr::YCbCrToRGB(YCbCr ycbcr) {
    float r = Max(0.0f, Min(1.0f, (float)(ycbcr.Y + 0.0000 * ycbcr.Cb + 1.4022 * ycbcr.Cr)));
    float g = Max(0.0f, Min(1.0f, (float)(ycbcr.Y - 0.3456 * ycbcr.Cb - 0.7145 * ycbcr.Cr)));
    float b = Max(0.0f, Min(1.0f, (float)(ycbcr.Y + 1.7710 * ycbcr.Cb + 0.0000 * ycbcr.Cr)));

    return RGB((unsigned char)(r * 255), (unsigned char)(g * 255), (unsigned char)(b * 255));
}
