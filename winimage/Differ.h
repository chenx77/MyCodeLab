#ifndef DIFFER_H
#define DIFFER_H

#include <math.h>

float Differ(int* actData, int* expData, int width, int height)
{
    if (!actData || !expData)
        return -1;

    int length = width * height;
    int expPixel = 0;
    int actPixel = 0;
    int deltaR = 0;
    int deltaG = 0;
    int deltaB = 0;
    int maxDeltaR = 0;
    int maxDeltaG = 0;
    int maxDeltaB = 0;

    float diff = 0, diffR = 0, diffG = 0, diffB = 0;
    //float d255 = 1.0f / 255 / length;

    for (int i = 0; i < length; i++)
    {
        actPixel = actData[i];
        expPixel = expData[i];

        deltaR = abs((actPixel & 0x00FF0000) - (expPixel & 0x00FF0000));
        deltaG = abs((actPixel & 0x0000FF00) - (expPixel & 0x0000FF00));
        deltaB = abs((actPixel & 0x000000FF) - (expPixel & 0x000000FF));
        expData[i] = 0xFF000000 + deltaR + deltaG + deltaB;

        deltaR = deltaR >> 16;
        deltaG = deltaG >> 8;

        if (maxDeltaR < deltaR) maxDeltaR = deltaR;
        if (maxDeltaG < deltaG) maxDeltaG = deltaG;
        if (maxDeltaB < deltaB) maxDeltaB = deltaB;

//printf("deltaR(%4x), maxDeltaR(%4x)\n", deltaR, maxDeltaR);

        diffR += deltaR;
        diffG += deltaG;
        diffB += deltaB;
    }
    diff = (diffR + diffG + diffB) / 3;

    printf("### Diff  = %f, Max (in 255 per pixel)\n", diff / length);
    printf("    Red   = %f, %i\n", diffR / length, maxDeltaR);
    printf("    Green = %f, %i\n", diffG / length, maxDeltaG);
    printf("    Blue  = %f, %i\n", diffB / length, maxDeltaB);

    return diff / length;
}
#endif /* DIFFER_H */
