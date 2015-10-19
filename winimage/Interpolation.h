#ifndef INTERPOLATION_H
#define INTERPOLATION_H

namespace Interpolation
{
    enum InterpolationMethod
    {
        NearestNeighbour = 0,
        Bilinear = 1,
        Bicubic = 2
    };

    int InterpolateBicubic(int* srcData, int width, int height, double x, double y,
                           int* px, int* py, int *pColor, bool alphaValueUsed);

    int InterpolateBilinear(int* srcData, int width, int height,
                            double x, double y, bool alphaValueUsed);

}

#endif // INTERPOLATION_H
