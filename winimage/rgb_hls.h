#ifndef RGB_HLS_H
#define RGB_HLS_H

#define int32 int
#define float64 float
#define uint8 uchar

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

int32 RgbToHls(int Red, int Green, int Blue, float *Hue, float *Lumination, float *Saturation)
{
    float Delta;
    int Max, Min;
    //float64 Redf, Greenf, Bluef;

    //Redf    = ((float64)Red   / 255.0F);
    //Greenf  = ((float64)Green / 255.0F);
    //Bluef   = ((float64)Blue  / 255.0F);

    Max = MAX(MAX(Red, Green), Blue); // max(max(Redf, Greenf), Bluef);
    Min = MIN(MIN(Red, Green), Blue); // min(min(Redf, Greenf), Bluef);

    *Hue        = 0;
    *Lumination = (Max + Min) / 2.0F;
    *Saturation = 0;

    //Delta = Max - Min;

    if (Max == Min)
        return TRUE;

    Delta = Max - Min;

    if (*Lumination < 128)
        *Saturation = Delta / (Max + Min);
    else
        *Saturation = Delta / (511 - Max - Min);

    if (Red /*f*/ == Max)
    {
        *Hue = (Green/*f*/ - Blue/*f*/) / (float)Delta;
printf("Red == Max: Hue(%3.0f), Red(%i), Green(%i), Blue(%i), Max(%i)-Min(%i)=Delta(%3.0f)\n", *Hue, Red, Green, Blue, Max, Min, Delta);
    }
    else if (Green/*f*/ == Max)
    {
    printf("Green == Max\n");
        *Hue = 2.0 + (Blue/*f*/ - Red/*f*/) / (float)Delta;
    }
    else
    {
    printf("Blue == Max\n");
        *Hue = 4.0 + (Red/*f*/ - Green/*f*/) / (float)Delta;
    }
    *Hue /= 6.0;

    if (*Hue < 0.0)
        *Hue += 255;
    //else if (*Hue > 255)
    //    *Hue -= 255;

    return TRUE;
}

int HueToRgb(float64 M1, float64 M2, float64 Hue, float64 *Channel)
{
    if (Hue < 0.0)
        Hue += 1.0;
    else if (Hue > 1.0)
        Hue -= 1.0;

    if ((6.0 * Hue) < 1.0)
        *Channel = (M1 + (M2 - M1) * Hue * 6.0);
    else if ((2.0 * Hue) < 1.0)
        *Channel = (M2);
    else if ((3.0 * Hue) < 2.0)
        *Channel = (M1 + (M2 - M1) * ((2.0F / 3.0F) - Hue) * 6.0);
    else
        *Channel = (M1);

    return *Channel * 255;
}
/*
void HlsToRgb(float hue, float luminance, float saturation, int *red, int *green, int *blue)
{
    float rgb[3] = {0, 0, 0};
    float ls = luminance * saturation;
    float sls = saturation - ls;

    for (int channel = 0; channel < 3; channel++)
    {
        float Hue = hue + 0.3333 * (1 - channel);

        if (Hue < 0.0) Hue += 1.0;
        else if (Hue > 1.0) Hue -= 1.0;

        if ((6.0 * Hue) < 1.0)
        {
            if (luminance <= 0.5)
                rgb[channel] = luminance + ls * (Hue * 12 - 1);
            else
                rgb[channel] = luminance + sls * (3 + (Hue * 12 - 4));
        }
        else if ((2.0 * Hue) < 1.0)
        {
            if (luminance <= 0.5)
                rgb[channel] = luminance + ls;
            else
                rgb[channel] = luminance + sls;
        }
        else if ((3.0 * Hue) < 2.0)
        {
            if (luminance <= 0.5)
                rgb[channel] = luminance + ls * (3 - (Hue * 12 - 4));
            else
                rgb[channel] = luminance + sls * (3 - (Hue * 12 - 4));
        }
        else
        {
            if (luminance <= 0.5)
                rgb[channel] = luminance - ls;
            else
                rgb[channel] = luminance - sls;
        }
    }

    *red = 255 * rgb[0];
    *green = 255 * rgb[1];
    *blue = 255 * rgb[2];
}
*/
int32 HlsToRgb(float64 Hue, float64 Lumination, float64 Saturation, int *Red, int *Green, int *Blue)
{
    float64 M1, M2;
    float64 Redf, Greenf, Bluef;

    if (Saturation == 0)
        {
        Redf    = Lumination;
        Greenf  = Lumination;
        Bluef   = Lumination;
        }
    else
        {
        if (Lumination <= 0.5)
            M2 = Lumination * (1.0 + Saturation);
        else
            M2 = Lumination + Saturation - Lumination * Saturation;

        M1 = (2.0 * Lumination - M2);

        HueToRgb(M1, M2, Hue + (1.0F / 3.0F), &Redf);
        HueToRgb(M1, M2, Hue, &Greenf);
        HueToRgb(M1, M2, Hue - (1.0F / 3.0F), &Bluef);
        }

    *Red    = (uint8)(Redf * 255);
    *Blue   = (uint8)(Bluef * 255);
    *Green  = (uint8)(Greenf * 255);

    return TRUE;
}
#endif // RGB_HLS_H
