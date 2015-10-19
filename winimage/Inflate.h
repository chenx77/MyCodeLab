//
// Copyright Xun Chen 2012
// Author: Xun Chen <chenx77@gmail.com>
//
#ifndef INFLATE_H
#define INFLATE_H

//#include "StdAfx.h"
//#include "ImageProcessingAlgorithms.h"
#include "Interpolation.h"
#include <cmath>

void InflateARGB32(int* srcData, int* destData, int width, int height, int level, int interpolation)
{
    if (!srcData || !destData)
        return;

    int H2 = height / 2;
    int W2 = width / 2;
    float H2H2r = 1.f / (H2 * H2);
    float W2W2r = 1.f / (W2 * W2);
    float s = 0.005f * level; // s=0.5 100% PS, s=0.25 50% PS

    int v1 = 0;
    int v2 = 0;
    int u1 = 0;
    int u2 = 0;
    float kxr = 0;
    float ky = 0;
    float x = 0;
    float y = 0;
    float x1 = 0;
    float x2 = 0;
    float y1 = 0;
    float y2 = 0;
    int px[4], py[4], pColor[4];

    for (int i = 0; i <= H2; i++)
    {
        v1 = H2 - i;
        v2 = H2 + i;
        kxr = 1 / (1 + s * (1 - i * i * H2H2r));

        for (int j = 0; j <= W2; j++)
        {
            u1 = W2 - j;
            u2 = W2 + j;

            ky = 1 + s * (1 - j * j * W2W2r);

            x = j * kxr;
            y = i / ky;

            x1 = W2 - x;
            x2 = W2 + x;
            y1 = H2 - y;
            y2 = H2 + y;

            if (x2 < 0 || y2 < 0 || x2 > width || y2 > height || u1 < 0 || v1 < 0)
                continue;

            switch (interpolation)
            {
            case 0: // Nearest neighbour
                destData[u1 + v1 * width] = srcData[(int)(x1 + (int)y1 * width)];
                if (u2 < width)
                    destData[u2 + v1 * width] = srcData[(int)(x2 + (int)y1 * width)];
                if (v2 < height)
                    destData[u1 + v2 * width] = srcData[(int)(x1 + (int)y2 * width)];
                if (v2 < height && u2 < width)
                    destData[u2 + v2 * width] = srcData[(int)(x2 + (int)y2 * width)];
                break;

            case 1: // Interpolation::Bilinear:
                destData[u1 + v1 * width] = Interpolation::InterpolateBilinear (srcData, width, height, x1, y1, false);
                if (u2 < width)
                    destData[u2 + v1 * width] = Interpolation::InterpolateBilinear (srcData, width, height, x2, y1, false);
                if (v2 < height)
                    destData[u1 + v2 * width] = Interpolation::InterpolateBilinear (srcData, width, height, x1, y2, false);
                if (v2 < height && u2 < width)
                    destData[u2 + v2 * width] = Interpolation::InterpolateBilinear (srcData, width, height, x2, y2, false);
                break;

            case 2: // Interpolation::Bicubic:
                destData[u1 + v1 * width] = Interpolation::InterpolateBicubic (srcData, width, height, x1, y1, px, py, pColor, false);
                if (u2 < width)
                    destData[u2 + v1 * width] = Interpolation::InterpolateBicubic (srcData, width, height, x2, y1, px, py, pColor, false);
                if (v2 < height)
                    destData[u1 + v2 * width] = Interpolation::InterpolateBicubic (srcData, width, height, x1, y2, px, py, pColor, false);
                if (v2 < height && u2 < width)
                    destData[u2 + v2 * width] = Interpolation::InterpolateBicubic (srcData, width, height, x2, y2, px, py, pColor, false);
                break;

            default:
                break;
            }
        }
    }
}
#endif /* INFLATE_H */
