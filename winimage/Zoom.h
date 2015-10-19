//
// Copyright Xun Chen 2012
// Author: Xun Chen <chenx77@gmail.com>
//
#ifndef ZOOM_H
#define ZOOM_H

//#include "StdAfx.h"
//#include "ImageProcessingAlgorithms.h"
#include "Interpolation.h"
#include <cmath>

void ZoomARGB32(int* srcData, int* destData, int width, int height, int level, int interpolation)
{
    if (!srcData || !destData) return;

    if (level == 0) level = 100;

    float scale = (float)level / 100;

    int H2 = height / 2;
    int W2 = width / 2;

    int v1 = 0;
    int v2 = 0;
    int u1 = 0;
    int u2 = 0;

    float x = 0;
    float y = 0;
    float x1 = 0;
    float x2 = 0;
    float y1 = 0;
    float y2 = 0;

    int px[4], py[4], pColor[4]; // for bicubic

    scale = 1 / scale;

    for (int i = 0; i <= H2; i++)
    {
        v1 = H2 - i;
        v2 = H2 + i;

        for (int j = 0; j <= W2; j++)
        {
            u1 = W2 - j;
            u2 = W2 + j;

            x = (float)j * scale;
            y = (float)i * scale;

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
#endif /* ZOOM_H */
