#ifndef HSL_H
#define HSL_H

//#define _USE_MATH_DEFINES
#include <math.h>

// Input: (r, g, b); Output: (h, s, l)
void rgb2hsl(int *red, int *green, int *blue)
{
    int r = *red, g = *green, b = *blue;
    float s, l;
    float h;
    int Min, Max;
    float delta;

    Max = max(max(r, g), b);
    Min = min(min(r, g), b);

    l = (Max + Min) / 2.0;

    if (Max == Min)
    {
        s = 0.0;
        h = 0.0;
    }
    else
    {
        delta = (Max - Min);

        if (l < 128)
            s = 255 * delta / (Max + Min);
        else
            s = 255 * delta / (511 - Max - Min);

        if (r == Max)
            h = (g - b) / delta;
        else if (g == Max)
            h = 2 + (b - r) / delta;
        else
            h = 4 + (r - g) / delta;

        h *= 42.5;

        if (h < 0)
            h += 255;
        else if (h > 255)
            h -= 255;
    }

    *red   = h;
    *green = s;
    *blue  = l;

    return;
}

int hslValue(float m1, float m2, int hue)
//int hslValue(int m1, int m2, int hue)
{
    float value;
    //int value;

    if (hue > 255) hue -= 255;
    else if (hue < 0) hue += 255;

    if (hue < 42.5)
    {
        value = m1 + (m2 - m1) * (hue / 42.5);
    }
    else if (hue < 127.5)
    {
        value = m2;
    }
    else if (hue < 170)
    {
        value = m1 + (m2 - m1) * ((170 - hue) / 42.5);
    }
    else
    {
        value = m1;
    }

    return value * 255.f;
}

// Input: (h, s, l); Output: (r, g, b)
void hsl2rgb(int *hue, int *saturation, int *lightness)
{
    int h = *hue, s = *saturation, l = *lightness;

    if (s == 0)
    {
        *hue        = l;
        *lightness  = l;
        *saturation = l;
    }
    else
    {
        float m1, m2;

        if (l < 128){
            m2 = (l * (255 + s)) / 65025.f;
        }
        else
        {
            m2 = (l + s - (l * s) / 255.f) / 255.f;
        }
        m1 = (l / 127.5) - m2;

        *hue        = hslValue (m1, m2, h + 85);
        *saturation = hslValue (m1, m2, h);
        *lightness  = hslValue (m1, m2, h - 85);
    }
    return;
}
#endif // HSL_H
