#ifndef SPHERIZE_H
#define SPHERIZE_H

#include <math.h>
#include <cstdio>

void SpherizeARGB32(int* srcData, int* destData, int width, int height, int level)
{
    if (!srcData || !destData) return;

    double H2 = height/2, W2 = width/2;
    //double k3 = level*0.01;

    for (int i=0; i<H2; i++)
    {
        int v1 = H2-i, v2 = H2+i;

        // Spherical to Cartesian, angle in Rad: 0 -> PI/2
        double phi = asin( i/H2);
        double y = phi * 2 / M_PI * H2;
        double cosphi = cos( phi);
        int Wlimit = W2*cosphi;

        for (int j=0; j<Wlimit; j++)
        {
            int u1 = W2-j, u2 = W2+j;

            double theta = asin( j/W2/cosphi);
            double x = theta * 2 / M_PI * W2;

            double x1 = W2-x, x2 = W2+x;
            double y1 = H2-y, y2 = H2+y;

            if (x1<0 || x2>width || y1<0 || y2>height) continue;

            // Nearest neibour
            destData[u1+v1*width] = srcData[(int)(x1+(int)y1*width)];
            destData[u2+v1*width] = srcData[(int)(x2+(int)y1*width)];
            destData[u2+v2*width] = srcData[(int)(x2+(int)y2*width)];
            destData[u1+v2*width] = srcData[(int)(x1+(int)y2*width)];
        }
    }
}
#endif /* SPHERIZE_H */
