//#include "StdAfx.h"
#include "Interpolation.h"
//#include <armintr.h>
#include <QtGui>

using namespace std;

inline int Spline(int* p, double t, int* pColor, bool alphaValueUsed)
{
    int value = 0;
    int i = 0;
    int j = 0;
    double tPower2 = (float)(t * t);
    double tPower3 = (float)(tPower2 * t);
    int maxChannelIndex = 2;
    if (alphaValueUsed)
    {
        maxChannelIndex++;
    }
    for (i = 0; i <= maxChannelIndex; i++)
    {
        for (j = 0; j <= 3; j++)
        {
            pColor[j] = (p[j] >> (i * 8)) & 0x000000FF;
        }
        int v = (int)(0.5 * ((pColor[1] << 1)
                    + (-pColor[0] + pColor[2]) * t
                    + ((pColor[0] << 1) - 5 * pColor[1] + (pColor[2] << 2) - pColor[3]) * tPower2
                    + (-pColor[0] + 3 * pColor[1] - 3 * pColor[2] + pColor[3]) * tPower3));
        //v = _arm_usat(8, v, _ARM_LSL, 0);
        v = max(0, min(255, v));

        value |= ((v & 0x000000FF) << i*8);
    }

    if (!alphaValueUsed)
    {
        // Use 0xFF alpha value.
        value |= 0xFF000000;
    }

    return value;
}

int Interpolation::InterpolateBicubic(int* srcData, int width, int height, double x, double y,
                                      int* px, int* py, int *pColor, bool alphaValueUsed)
{
    int value = 0;
    int yPos = 0;
    int xPosMin1 = 0;
    int xPos0 = 0;
    int xPos1 = 0;
    int xPos2 = 0;
    int xi = (int)x;
    int yi = (int)y;

    int rowStartPos = 0;
    for (int j = -1; j <= 2; j++)
    {
        // Pixels mapped outside of source bitmap get copy of border value.
        yPos = (yi + j < 0 ? 0 : yi + j < height ? yi + j : height - 1);
        rowStartPos = yPos * width;
        xPosMin1 = (xi - 1 > 0 ? xi - 1 : 0);
        xPos0 = xi;
        xPos1 = (xi + 1 < width ? xi + 1 : width -1);
        xPos2 = (xi + 2 < width ? xi + 2 : width -1);
        px[0] = srcData[rowStartPos + xPosMin1];
        px[1] = srcData[rowStartPos + xPos0];
        px[2] = srcData[rowStartPos + xPos1];
        px[3] = srcData[rowStartPos + xPos2];
        py[j+1] = Spline(px, x - xi, pColor, alphaValueUsed);
    }
    value = Spline(py, y - yi, pColor, alphaValueUsed);
    return value;
}

int Interpolation::InterpolateBilinear(int* srcData, int width, int height,
                                       double x, double y, bool alphaValueUsed)
{
    int result = 0;

    if (!srcData || width < 0 || height < 0 || x < 0 || y < 0)
    {
        return result;
    }

    int floorX = (int)x;
    int floorY = (int)y;

    if (floorX >= width)
    {
       floorX = width - 1;
    }

    if (floorY >= height)
    {
       floorY = height - 1;
    }

    int ceilingX = floorX + 1;
    int ceilingY = floorY + 1;

    if (ceilingX >= width)
    {
        ceilingX = width - 1;
    }

    if (ceilingY >= height)
    {
        ceilingY = height - 1;
    }

    float f1 = 0;
    float f2 = 0;

    f1 = (float)x - floorX;
    f2 = (float)y - floorY;

    int upperLeft = srcData[floorY * width + floorX];
    int upperRight = srcData[floorY * width + ceilingX];
    int bottomLeft = srcData[ceilingY * width + floorX];
    int bottomRight = srcData[ceilingY * width + ceilingX];

    int upperLeftRed = (upperLeft >> 16) & 0x000000FF;
    int upperLeftGreen = (upperLeft >> 8) & 0x000000FF;
    int upperLeftBlue = (upperLeft) & 0x000000FF;

    int upperRightRed = (upperRight >> 16) & 0x000000FF;
    int upperRightGreen = (upperRight >> 8) & 0x000000FF;
    int upperRightBlue = (upperRight) & 0x000000FF;

    int bottomLeftRed = (bottomLeft >> 16) & 0x000000FF;
    int bottomLeftGreen = (bottomLeft >> 8) & 0x000000FF;
    int bottomLeftBlue = (bottomLeft) & 0x000000FF;
    int bottomRightRed = (bottomRight >> 16) & 0x000000FF;
    int bottomRightGreen = (bottomRight >> 8) & 0x000000FF;
    int bottomRightBlue = (bottomRight) & 0x000000FF;

    int upperLeftAlpha = 0;
    int upperRightAlpha = 0;
    int bottomLeftAlpha = 0;
    int bottomRightAlpha = 0;
    if (alphaValueUsed)
    {
        upperLeftAlpha = (upperLeft >> 24) & 0x000000FF;
        upperRightAlpha = (upperRight >> 24) & 0x000000FF;
        bottomLeftAlpha = (bottomLeft >> 24) & 0x000000FF;
        bottomRightAlpha = (bottomRight >> 24) & 0x000000FF;
    }

    float f1Diff = 1.0f - f1;
    float f2Diff = 1.0f - f2;
    float f1F2 = f1 * f2;
    float f1DiffF2Diff = f1Diff * f2Diff;
    float f1F2Diff = f1 * f2Diff;
    float f1DiffF2 = f1Diff * f2;

    int resultRed = (int)(upperLeftRed * f1DiffF2Diff + upperRightRed * f1F2Diff + bottomLeftRed * f1DiffF2 + bottomRightRed * f1F2);
    int resultGreen = (int)(upperLeftGreen * f1DiffF2Diff + upperRightGreen * f1F2Diff + bottomLeftGreen * f1DiffF2 + bottomRightGreen * f1F2);
    int resultBlue = (int)(upperLeftBlue * f1DiffF2Diff + upperRightBlue * f1F2Diff + bottomLeftBlue * f1DiffF2 + bottomRightBlue * f1F2);
    if (alphaValueUsed)
    {
        int resultAlpha = (int)(upperLeftAlpha * f1DiffF2Diff + upperRightAlpha * f1F2Diff + bottomLeftAlpha * f1DiffF2 + bottomRightAlpha * f1F2);
        result = (resultAlpha << 24) | (resultRed << 16) | (resultGreen << 8) | resultBlue;
    }
    else
    {
        result = 0xFF000000 | (resultRed << 16) | (resultGreen << 8) | resultBlue;
    }

    return result;
}
