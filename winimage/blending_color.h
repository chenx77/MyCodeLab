#ifndef BLENDING_COLOR_H
#define BLENDING_COLOR_H

#include "rgb_hls.h" // float
#include "hsl.h" // int

// For colour blending
#define RED(pixel) ((pixel >> 16) & 0xff)
#define GREEN(pixel) ((pixel >> 8) & 0xff)
#define BLUE(pixel) (pixel & 0xff)
#define PIXEL(r, g, b) ((r << 16) + (g << 8) + b)
#define LUM(pixel) (RED(pixel) * 0.3 + GREEN(pixel) * 0.59 + BLUE(pixel) * 0.11)
//#define LUM(pixel) ((MAX(MAX(RED(pixel), GREEN(pixel)), BLUE(pixel)) + MIN(MIN(RED(pixel), GREEN(pixel)), BLUE(pixel))) / 2)
#define SAT(pixel) = (MAX(MAX(RED(pixel), GREEN(pixel)), BLUE(pixel)) - MIN(MIN(RED(pixel), GREEN(pixel)), BLUE(pixel)))

int ColourBlend(int base, int top)
{
    int result;
    float hue, lumination, saturation;
    int red = RED(top), green = GREEN(top), blue = BLUE(top);
    //printf("+++ %3i %3i %3i\n", red, green, blue);

    RgbToHls(red, green, blue, &hue, &lumination, &saturation);
    printf("+++ %3.0f %3.0f %3.0f\n", hue*255, saturation*255, lumination*255);

    lumination = LUM(base)/255.f;
    HlsToRgb(hue, lumination, saturation, &red, &green, &blue);

    result = PIXEL(red, green, blue);
    return result;
}

int blend_color(int base, int top) // int
{
    int result;
    float hue, lumination, saturation;
    int red = RED(top), green = GREEN(top), blue = BLUE(top);
    //printf("--- %3i %3i %3i\n", red, green, blue);

    rgb2hsl(&red, &green, &blue); // {h:red,    s:green,    l:blue}
    printf("--- %3i %3i %3i\n", red, green, blue);

    blue = LUM(base);

    hsl2rgb(&red, &green, &blue); // {r:hue,    g:saturation,    b:lightness}
    //printf("    %3i %3i %3i\n", red, green, blue);

    result = PIXEL(red, green, blue);
    return result;
}

int ClipColour(int red, int green, int blue, int l)
{
    int cMin = MIN(red, MIN(green, blue));
    int cMax = MAX(red, MAX(green, blue));

    if (cMin < 0)
    {
        red = l + ((red - l) * l) / (l - cMin);
        green = l + ((green - l) * l) / (l - cMin);
        blue = l + ((blue - l) * l) / (l - cMin);
    }
    if (cMax > 255)
    {
        red = l + ((red - l) * (255 - l)) / (cMax - l);
        green = l + ((green - l) * (255 - l)) / (cMax - l);
        blue = l + ((blue - l) * (255 - l)) / (cMax - l);
    }

    return PIXEL(red, green, blue);
}

int setLum(int base, int top)
{
    int red = RED(top), green = GREEN(top), blue = BLUE(top);
    int lum = LUM(base);
    int diff = lum - LUM(top);

    red += diff;
    green += diff;
    blue += diff;

    return ClipColour(red, green, blue, lum);
}

/* Non-separable blend modes */
int lum(int r, int g, int b)
{
    /* 0.3, 0.59, 0.11 in 16.16 fixed point */
    return (19662 * r + 38666 * g + 7208 * b) >> 16;
}

void clipcolor(int r, int g, int b, int *dr, int *dg, int *db)
{
    int l = lum(r, g, b);
    int n = MIN(MIN(r, g), b);
    int x = MAX(MAX(r, g), b);
    if (n < 0) {
        *dr = l + 255 * (r - l) / (l - n);
        *dg = l + 255 * (g - l) / (l - n);
        *db = l + 255 * (b - l) / (l - n);
    }
    else {
        *dr = l + 255 * (255 - l) / (x - l);
        *dg = l + 255 * (255 - l) / (x - l);
        *db = l + 255 * (255 - l) / (x - l);
    }
}

int setlum(int base, int top)
{
    int result = 0;
    //int d = LUM(base) - LUM(top);
    int r = RED(top), g = GREEN(top), b = BLUE(top);
    int d = 255 - lum(RED(base), GREEN(base), BLUE(base));
    int dr = 0, dg = 0, db = 0;

    clipcolor(r + d, g + d, b + d, &dr, &dg, &db);
    printf("### setlum 3: %i, %i, %i\n", dr, dg, db);

    return PIXEL(dr, dg, db);
}

int Clip_Color(int top)
{
    int l = LUM(top);
    int red = RED(top), green = GREEN(top), blue = BLUE(top);
    int n = min(red, min(green, blue));
    int x = max(red, max(green, blue));

    if (n < 0)
    {
        // C = L + (((C - L) * L) / (L - n))
        red =   l + (((red - l) * l) / (float)(l - n));
        green = l + (((green - l) * l) / (float)(l - n));
        blue =  l + (((blue - l) * l) / (float)(l - n));
    }
    if (x > 1)
    {
        //C = l + (((Cred - L) * (1 - L)) / (x - L))
        red = l + (((red - l) * (255.f - l)) / (x - l));
        green = l + (((green - l) * (255.f - l)) / (x - l));
        blue = l + (((blue - l) * (255.f - l)) / (x - l));
    }
    return PIXEL(red, green, blue);
}

int Set_Lum(int base, int top)
{
    int d = LUM(base) - LUM(top);
    int red = RED(top), green = GREEN(top), blue = BLUE(top);

    red += d;
    green += d;
    blue += d;

    return Clip_Color(PIXEL(red, green, blue));
}

//The subscripts min, mid, and max in the next function refer to the color
//components having the minimum, middle, and maximum values upon entry to the function.
/*
int Set_Sat(int base, int top)
{
    int red = RED(top), green = GREEN(top), blue = BLUE(top);
    int n = min(red, min(green, blue));
    int x = max(red, max(green, blue));
    int s = x - n;

    if(x > n)
    {
        Cmid = (((Cmid - Cmin) x s) / (Cmax - Cmin))
        Cmax = s
    }
    else
    {
        Cmid = Cmax = 0
        Cmin = 0
    }
    return PIXEL(red, green, blue);
}*/

#define uint16_t int
#define int16_t int
#define int32_t int
#define uint32_t int

inline static int set_rgb16_lum_from_rgb16(int base, int top)
{
    // Spec: SetLum()
    // Colours potentially can go out of band to both sides, hence the
    // temporary representation inflation.
    const uint16_t botlum = LUM(base); // (1<<15);
    const uint16_t toplum = LUM(top); // (1<<15);
    const int16_t diff = botlum - toplum;
    const uint16_t topr = RED(top), topg = GREEN(top), topb = BLUE(top);
    int32_t r = topr + diff;
    int32_t g = topg + diff;
    int32_t b = topb + diff;

    // Spec: ClipColor()
    // Clip out of band values
    int32_t lum = LUM(PIXEL(r, g, b)); // (1<<15);
    int32_t cmin = MIN(r, MIN(g, b));
    int32_t cmax = MAX(r, MAX(g, b));

    printf("r=%3i, g=%3i, b=%3i, blum=%3i, tlum=%3i, diff=%3i, lum=%3i, max=%3i, min=%3i\n", r, g, b, botlum, toplum, diff, lum, cmax, cmin);

    if (cmin < 0) {
        r = lum + (((r - lum) * lum) / (lum - cmin));
        g = lum + (((g - lum) * lum) / (lum - cmin));
        b = lum + (((b - lum) * lum) / (lum - cmin));
        printf("-\n");
    }
    if (cmax > 255 /*(1<<15)*/) {
        r = lum + (((r - lum) * (/*(1<<15)*/255-lum)) / (cmax - lum));
        g = lum + (((g - lum) * (/*(1<<15)*/255-lum)) / (cmax - lum));
        b = lum + (((b - lum) * (/*(1<<15)*/255-lum)) / (cmax - lum));
        printf("+\n");
    }
    #ifdef HEAVY_DEBUG
    assert((0 <= r) && (r <= 255 /*(1<<15)*/));
    assert((0 <= g) && (g <= 255 /*(1<<15)*/));
    assert((0 <= b) && (b <= 255 /*(1<<15)*/));
    #endif

    return PIXEL(r, g, b);
}
/*
void draw_dab_pixels_BlendMode_Color (uint16_t *mask, uint16_t *rgba, // b=bottom, premult
    uint16_t color_r, // }
    uint16_t color_g, // }-- a=top, !premult
    uint16_t color_b, // }
    uint16_t opacity)
{
    while (1)
    {
        for (; mask[0]; mask++, rgba+=4)
        {
            // De-premult
            uint16_t r, g, b;
            const uint16_t a = rgba[3];
            r = g = b = 0;
            if (rgba[3] != 0)
            {
                r = ((1<<15)*((uint32_t)rgba[0])) / a;
                g = ((1<<15)*((uint32_t)rgba[1])) / a;
                b = ((1<<15)*((uint32_t)rgba[2])) / a;
            }

            // Apply luminance
            set_rgb16_lum_from_rgb16(color_r, color_g, color_b, &r, &g, &b);

            // Re-premult
            r = ((uint32_t) r) * a / (1<<15);
            g = ((uint32_t) g) * a / (1<<15);
            b = ((uint32_t) b) * a / (1<<15);

            // And combine as normal.
            uint32_t opa_a = mask[0] * opacity / (1<<15); // topAlpha
            uint32_t opa_b = (1<<15) - opa_a; // bottomAlpha
            rgba[0] = (opa_a*r + opa_b*rgba[0])/(1<<15);
            rgba[1] = (opa_a*g + opa_b*rgba[1])/(1<<15);
            rgba[2] = (opa_a*b + opa_b*rgba[2])/(1<<15);
        }
        if (!mask[1]) break;
        rgba += mask[1];
        mask += 2;
    }
}*/
#endif // BLENDING_COLOR_H
