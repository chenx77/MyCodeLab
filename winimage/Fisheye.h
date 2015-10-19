#ifndef INFLATE_H
#define INFLATE_H

#include <math.h>

Interpolator<double> interpolator;

float calc_shift(float x1,float x2,float cx,float k)
{
    float x3 = (float)(x1+(x2-x1)*0.5);
    float res1 = x1+((x1-cx)*k*((x1-cx)*(x1-cx)));
    float res3 = x3+((x3-cx)*k*((x3-cx)*(x3-cx)));

    if(res1>-thresh && res1 < thresh)
        return x1;
    if(res3<0){
        return calc_shift(x3,x2,cx,k);
    }
    else{
        return calc_shift(x1,x3,cx,k);
    }
}

float getRadialX(float x,float y,float cx,float cy,float k)
{
    dx = (x*xscale+xshift);
    dy = (y*yscale+yshift);
    float res = x+((x-W2)*k*((x-W2)*(x-W2)+(y-H2)*(y-H2)));
    return res;
}

float getRadialY(float x,float y,float cx,float cy,float k)
{
    x = (x*xscale+xshift);
    y = (y*yscale+yshift);
    float res = y+((y-cy)*k*((x-cx)*(x-cx)+(y-cy)*(y-cy)));
    return res;
}

void FisheyeARGB32 (int* srcData, int* destData, int width, int height, int level, int interpolation)
{
    if (!srcData || !destData)
        return;

    int H2 = height / 2;
    int W2 = width / 2;
    double H2H2r = 1.f / (H2 * H2);
    double W2W2r = 1.f / (W2 * W2);
    double s = 0.005 * level; // s=0.5 100% PS, s=0.25 50% PS

    int v1 = 0;
    int v2 = 0;
    int u1 = 0;
    int u2 = 0;
    double kxr = 0;
    double ky = 0;
    double x = 0;
    double y = 0;
    double x1 = 0;
    double x2 = 0;
    double y1 = 0;
    double y2 = 0;
    int px[4], py[4], pColor[4];

    for (int i = 0; i <= H2; i++)
    {
        v1 = H2 - i;
        v2 = H2 + i;

        for (int j = 0; j <= W2; j++)
        {
            u1 = W2 - j;
            u2 = W2 + j;

            x = j * kxr;
            y = i / ky;

            x1 = W2 - x;
            x2 = W2 + x;
            y1 = H2 - y;
            y2 = H2 + y;
/*
xscale = (width-xshift-xshift_2)/width;
yscale = (height-yshift-yshift_2)/height;

xshift = calc_shift(0,centerX-1,centerX,k);
float newcenterX = width-centerX;
float xshift_2 = calc_shift(0,newcenterX-1,newcenterX,k);

yshift = calc_shift(0,centerY-1,centerY,k);
float newcenterY = height-centerY;
float yshift_2 = calc_shift(0,newcenterY-1,newcenterY,k);

float x = getRadialX ((float)i, (float)j, centerX, centerY,K);
float y = getRadialY ((float)i, (float)j, centerX, centerY,K);
*/
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
                destData[u1 + v1 * width] = interpolator.bilinear (srcData, width, height, x1, y1);
                if (u2 < width)
                    destData[u2 + v1 * width] = interpolator.bilinear (srcData, width, height, x2, y1);
                if (v2 < height)
                    destData[u1 + v2 * width] = interpolator.bilinear (srcData, width, height, x1, y2);
                if (v2 < height && u2 < width)
                    destData[u2 + v2 * width] = interpolator.bilinear (srcData, width, height, x2, y2);
                break;
            /*
            case 2: // Interpolation::Bicubic:
                destData[u1 + v1 * width] = interpolator.bicubic (srcData, width, height, x1, y1, px, py, pColor);
                if (u2 < width)
                    destData[u2 + v1 * width] = interpolator.bicubic (srcData, width, height, x2, y1, px, py, pColor);
                if (v2 < height)
                    destData[u1 + v2 * width] = interpolator.bicubic (srcData, width, height, x1, y2, px, py, pColor);
                if (v2 < height && u2 < width)
                    destData[u2 + v2 * width] = interpolator.bicubic (srcData, width, height, x2, y2, px, py, pColor);
                break;
            */
            default:
                break;
            }
        }
    }
}
#endif /* INFLATE_H */
