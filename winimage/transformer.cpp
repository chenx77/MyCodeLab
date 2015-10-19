#include "interpolator.h"
#include "transformer.h"
#include "gauss.h"
#include "Inflate.h"
#include "Spherize.h"
#include "Differ.h"
#include "Zoom.h"

#include <fstream> // for readTextFile
#include <sstream> // for readTextFile

#include <QtGui>
#include <QImage>
#include <QPainter>

#include "circle_detector.h"
#include "line_detector.h"

int Offset = 10;

///////////////////////////////////////////////////////////
//                                                       //
// Windows Based - xxxARGB32 (srcData, destData, ...)    //
//                                                       //
///////////////////////////////////////////////////////////
QImage zoomWinImage( QImage image, const float scale)
{
  QImage destImage = QImage( image);
  destImage.fill( 0);

  int* srcData = (int *)image.bits();
  int* destData = (int *)destImage.bits();

  int width = image.width(), height = image.height();
  int level = scale * 100;

  ZoomARGB32( srcData, destData, width, height, level, 0);

  return destImage;
}

QImage blendingWinImage( QImage base, QImage top, int blendingMode)
{
    int width = base.width(), height = base.height();
    int length = width * height;
    uchar *data = new uchar[4*length];
    int opacityLayer = 255;

    QImage destImage = QImage( data, width, height, QImage::Format_ARGB32);

    int* srcData = (int *)base.bits();
    int* destData = (int *)destImage.bits();
    int* topData = (int *)top.bits();

/* /####################################################
  time_t start, end;
  double dif;
  time (&start);
  for (int i=0; i<1; i++)
//#################################################### */

    //Blend(srcData,  destData, topData, width, height, opacityLayer, blendingMode);

/* /####################################################
  time (&end);
  dif = difftime (end, start);
  qDebug() << "tatol time spent: " << dif;
//#################################################### */

    return destImage;
}

QImage curvesWinImage( QImage image )
{
    int width = image.width(), height = image.height();
    int length = width * height;
    uchar *data = new uchar[4*length];
    //destImage.fill( 0);

    QImage destImage = QImage( data, width, height, QImage::Format_ARGB32);

    int* srcData = (int *)image.bits();
    int* destData = (int *)destImage.bits();
    int dataPointCount = 6;
    int inputMaster[] = {0, 16, 192, 224, 240, 255}; //{0, 255};
    int outputMaster[] = {16, 16, 160, 96, 240, 240}; //{0, 255};

    unsigned char dataPointCounts[4] =  { 6, 0, 7, 8 };
    unsigned char dataPoint[] = {
                        0, 16, 192, 224, 240, 255,
                        16, 16, 160, 96, 240, 240,
                        0, 30, 50, 70, 90, 110, 255,
                        0, 9, 16, 80, 224, 240, 0,
                        0, 30, 50, 70, 90, 110, 150, 255,
                        0, 60, 80, 160, 224, 240, 0, 40,
                        0, 100, 255,
                        255, 100, 0 };
/*####################################################
  time_t start, end;
  double dif;
  time (&start);
  for (int i=0; i<100; i++)
//####################################################*/

    //CurvesXunARGB32(srcData, width, height, destData, dataPointCounts, dataPoint);
    //CurvesARGB32(srcData, width, height, destData, dataPointCount, inputMaster, outputMaster);

/*####################################################
  time (&end);
  dif = difftime (end, start);
  qDebug() << "tatol time spent: " << dif;
//####################################################*/

    return destImage;
}

QImage resizeWinImage(QImage image, const float scale)
{
    int width = image.width(), height = image.height();
    int destWidth = scale * (float)width;
    int destHeight = scale * (float)height;
    int length = destWidth * destHeight;
    uchar *data = new uchar[4*length];

    QImage destImage = QImage( data, destWidth, destHeight, QImage::Format_ARGB32);
    //destImage.fill( 0);

    int* srcData = (int *)image.bits();
    int* destData = (int *)destImage.bits();

    //ResizeARGB32(srcData, width, height, destData, destWidth, destHeight, false, Interpolation::NearestNeighbour);

    return destImage;
}

QImage classicWinBlur(QImage image, const int radius)
{
    QImage destImage = QImage( image);
    destImage.fill( 0);
    int width = image.width(), height = image.height();

    int* srcData = (int *)image.bits();
    int* destData = (int *)destImage.bits();

    //ClassicBlurARGB32(srcData, destData, width, height, radius);

    return destImage;
}

QImage spherizeWinImage( QImage image, const float k3)
{
  QImage destImage = QImage( image);
  destImage.fill( 1);

  int* srcData = (int *)image.bits();
  int* destData = (int *)destImage.bits();

  int width = image.width(), height = image.height();
  int level = 100*k3;

  SpherizeARGB32( srcData, destData, width, height, level);

  return destImage;
}

QImage inflateWinImage( QImage image, const float s)
{
  QRect rect = image.rect();
  QImage destImage = QImage( image);
  destImage.fill( 0);

  int* srcData = (int *)image.bits();
  int* destData = (int *)destImage.bits();

  int width = rect.width(), height = rect.height();
  int level = 100*s;

  InflateARGB32( srcData, destData, width, height, level, 1);

  return destImage;
}

QImage fisheyeWinImage( QImage image, const float s, bool reverse)
{
  QRect rect = image.rect();
  QImage destImage = QImage( image);
  destImage.fill( 0);

  int* srcData = (int *)image.bits();
  int* destData = (int *)destImage.bits();

  int width = rect.width(), height = rect.height();
  int level = 100*s;

  //fisheyeARGB32( srcData, destData, width, height, level, 1);

  return destImage;
}

///////////////////////////////////////////////////////////
//                                                       //
// Qt Based, compressed, (*funcXY) as parameter          //
//                                                       //
///////////////////////////////////////////////////////////
QImage imageQuaters (const QImage image, const float scale, const int quality,
    float (*funcXY)(float*, float*, int, int, float, float, float))
{
    QImage destImage = QImage( image);
    QRect rect = image.rect();
    destImage.fill( 0);

    int height = rect.height(), width = rect.width();
    float H2 = height/2, W2 = width/2;

    float x, y, none;

    Interpolator<double> intpol;

    for (int i=0; i<=H2; i++)
    {
        float v1 = H2 - i, v2 = H2 + i;

        for (int j=0; j<=W2; j++)
        {
            none = (*funcXY) (&x, &y, i, j, H2, W2, scale);

            float u1 = W2 - j, u2 = W2 + j;

            float x1 = W2 - x, x2 = W2 + x;
            float y1 = H2 - y, y2 = H2 + y;

            if (!rect.contains( x1, y1)) continue;

            switch (quality)
            {
            case -1: // reverse nearest neighbour
                destImage.setPixel( x1, y1, image.pixel( u1, v1));
                destImage.setPixel( x2, y1, image.pixel( u2, v1));
                destImage.setPixel( x1, y2, image.pixel( u1, v2));
                destImage.setPixel( x2, y2, image.pixel( u2, v2));
                break;

            case 0: // nearest neighbour
                destImage.setPixel( u1, v1, image.pixel( x1, y1));
                if (u2 < width)
                    destImage.setPixel( u2, v1, image.pixel( x2, y1));
                if (v2 < height)
                    destImage.setPixel( u1, v2, image.pixel( x1, y2));
                if (v2 < height && u2 < width)
                    destImage.setPixel( u2, v2, image.pixel( x2, y2));
                break;

            case 1: // bilinear
                destImage.setPixel( u1, v1, intpol.bilinear( image, x1, y1).rgba());
                if (u2 < width)
                    destImage.setPixel( u2, v1, intpol.bilinear( image, x2, y1).rgba());
                if (v2 < height)
                    destImage.setPixel( u1, v2, intpol.bilinear( image, x1, y2).rgba());
                if (v2 < height && u2 < width)
                    destImage.setPixel( u2, v2, intpol.bilinear( image, x2, y2).rgba());
                break;

            case 2: // bicubic
                destImage.setPixel( u1, v1, intpol.bicubic( image, x1, y1).rgba());
                if (u2 < width)
                    destImage.setPixel( u2, v1, intpol.bicubic( image, x2, y1).rgba());
                if (v2 < height)
                    destImage.setPixel( u1, v2, intpol.bicubic( image, x1, y2).rgba());
                if (v2 < height && u2 < width)
                    destImage.setPixel( u2, v2, intpol.bicubic( image, x2, y2).rgba());
                break;
            }
        }
    }
    return destImage;
}

inline float zoomXY (float *x, float *y, int i, int j, float H2, float W2, float scale)
{
    *x = j * scale;
    *y = i * scale;
    return 0;
}

QImage zoomImage( const QImage image, const float scale, int quality)
{
    QImage destImage = imageQuaters (image, scale, quality, zoomXY);

    return destImage;
}

inline float distortXY (float *x, float *y, int i, int j, float H2, float W2, float scale)
{
    float k3 = scale, k1 = 1 - k3;
    float R = sqrt( H2*H2 + W2*W2);
    float r = sqrt( j*j + i*i)/R;
    float k = k1 + k3*r*r;
    *x = j * k;
    *y = i * k;
    return 0;
}

QImage distortImage( const QImage image, const float k3, int quality)
{
    QImage destImage = imageQuaters (image, k3, quality, distortXY);
    return destImage;
}


inline float spherizeXY (float *x, float *y, int i, int j, float H2, float W2, float scale)
{
    float theta = asin( j/W2/cos( asin( i/H2)));
    float phi = asin( i/H2);

    *y = phi * 2 / M_PI * H2;
    *x = theta * 2 / M_PI * W2;

    return 0;
}

QImage spherizeImage( const QImage image)
{
    QImage destImage = imageQuaters (image, 0, 1, spherizeXY);

    // don't remember why
    //int Wlimit = W2*cosphi;
    //for (int j=0; j<Wlimit; j++)

    return destImage;
}

inline float topSpherizeXY (float *x, float *y, int i, int j, float H2, float W2, float scale)
{
    // Polar test 1
    double R = sqrt( H2*H2 + W2*W2);
    *x = atan2(i, j) * W2 / M_PI * 2;
    *y = sqrt(i*i + j*j) * H2 / R * 1.21;

    // Polar test 2
    //double theta = atan2( i, j);
    //double x = r * cos( theta);
    //double y = r * sin( theta);

    // (x, y) <= (u[j], v[i]) <= (theta, phi)
    //double theta = j/W2 * M_PI/2;
    //double x = sin( theta) * sinphi * W2;
    //double y = cos( theta) * sinphi * H2;

    // Polar modle
    //double x = r * cos( theta) * W2;
    //double y = r * sin( theta) * H2;

    return 0;
}

QImage topSpherizeImage( const QImage image)
{
    QImage destImage = imageQuaters (image, 0, 1, topSpherizeXY);
    return destImage;
}

inline float inflateXY (float *x, float *y, int i, int j, float H2, float W2, float scale)
{
    //float kx = (sqrt(i / H2) - 1) / scale;
    //float ky = (sqrt(j / W2) - 1) / scale;
    //float kx = 1 + scale * (1 - 0.6 * i * i / (H2 * H2));
    //float ky = 1 + scale * (1 - 0.6 * j * j / (W2 * W2));
    float kx = 1 + 0.5 * scale * (1 - i * i / (H2 * H2));
    float ky = 1 + 0.5 * scale * (1 - j * j / (W2 * W2));

    *x = j / kx;
    *y = i / ky;

    return 0;
}

QImage inflateImage( const QImage image, const float scale, int quality)
{
    return imageQuaters (image, scale, quality, inflateXY);
}

inline float fisheyeXY (float *x, float *y, int i, int j, float Cy, float Cx, float k)
{
    *x = 100 + 0.2; //xscale + xshift;
    *y = 50 + 0.1; //yscale + yshift;

    *x += (*x-Cx) * k * ((*x-Cx) * (*x-Cx) + (*y-Cy)*(*y-Cy));
    *y += (*y-Cy) * k * ((*x-Cx) * (*x-Cx) + (*y-Cy)*(*y-Cy));

printf ("x=%f, y=%f, k=%f\n", *x, *y, k);

    return 0;
}

QImage fisheyeImage( const QImage image, const float scale)
{
    QImage destImage = imageQuaters (image, scale, 0, fisheyeXY);
    return destImage;
}

void cross( QImage *image, int Cx, int Cy, int size)
{
    for (int i=1; i<size; i++)
    {
        image->setPixel( Cx-i, Cy, 0xFFFF0000);
        image->setPixel( Cx+i, Cy, 0xFFFF0000);
        image->setPixel( Cx, Cy-i, 0xFFFF0000);
        image->setPixel( Cx, Cy+i, 0xFFFF0000);
    }
    //return image;
}

QImage diffImage( QImage image, QImage imageExp, const float level)
{
    QImage imageAct = QImage( image);
    int width = image.width(), height = image.height();

    int* actData = (int *)imageAct.bits();
    int* expData = (int *)imageExp.bits();

    float delta = Differ( actData, expData, width, height);

    return imageExp;
}

QImage kaleidoscopeImage( const QImage image, const float length)
{
    QImage outImage = QImage( image);
    QRect rect = image.rect();
    int height = image.height();
    int width = image.width();
    int Cx = rect.center().x();
    int Cy = rect.center().y();
    QPainter painter( &outImage);
    painter.setRenderHint( QPainter::SmoothPixmapTransform);
    outImage.fill( 0xFF888888);
    cross( &outImage, Cx+28.5, Cy-44.2, 20); //Cx+27.5, Cy-45.5, 20);
    cross( &outImage, 1.08*Cx, 0.815*Cy, 40);

    //float length = 100;
    float tan30 = tan( 30.f/180 * M_PI);
    float sideHalf = length * tan30;
    float side = sideHalf * 2;

    // make points for triangle
    QVector<QPointF> points;
    points.push_back( QPointF(0, 0));
    points.push_back( QPointF(length, sideHalf));
    points.push_back( QPointF(length, -sideHalf));

    // base triangle
    QPolygonF triangle = QPolygonF( points);

    QTransform trans1;
    trans1.rotate( 180, Qt::XAxis).rotate( -60, Qt::ZAxis);
    QPolygonF triangle1 = trans1.map( triangle);

    QTransform trans2;
    trans2.rotate( 180, Qt::XAxis).rotate( 60, Qt::ZAxis);
    QPolygonF triangle2 = trans2.map( triangle);

    QTransform trans3;
    trans3.rotate( 180, Qt::YAxis).translate( -length*2, 0);
    QPolygonF triangle3 = trans3.map ( triangle);

    QTransform trans;
    trans.translate(Cx, Cy);

    // Original triangle
    QPainterPath path( QPointF(0, 0));
    path.addPolygon( trans.map(triangle));
    painter.setClipPath( path);
    painter.drawImage( rect, image);

    QPainterPath path1( QPointF(0, 0));
    //path1.addRect( rect);
    path1.addPolygon( trans.map(triangle1));
    painter.setClipPath( path1);
    painter.drawImage( Cx, 0, outImage.transformed( trans1), 1.62*Cy, 0.543*Cx);
    //1.08*Cx, 0.815*Cy);

    QPainterPath path2( QPointF(0, 0));
    //path2.addRect( rect);
    path2.addPolygon( trans.map(triangle2));
    painter.setClipPath( path2);
    painter.drawImage( Cx, 0, outImage.transformed( trans2), 1.62*Cy, 0.543*Cx);
    //1.08*Cx, 0.815*Cy); //Cx+28.5, Cy-44.2);

    QPainterPath path3( QPointF(0, 0));
    //path3.addRect( rect);
    path3.addPolygon( trans.map(triangle3));
    painter.setClipPath( path3);
    painter.drawImage ( Cx+length, Cy-sideHalf, image.transformed( trans3), Cx-length, Cy-sideHalf);

    //painter.translate(Cx, Cy);

    QPainterPath path4( QPointF(0, 0));
    path4.addRect( rect);
    painter.setClipPath( path4);

    painter.drawPolygon( trans.map(triangle));
    //painter.drawPolygon( trans.map(triangle1));
    //painter.drawPolygon( trans.map(triangle2));
    //painter.drawPolygon( trans.map(triangle3));

    return outImage;
}

QImage formatImage(QImage image, const int format)
{
    QImage destImage = image.convertToFormat ( (QImage::Format) format);
    /*                                          Sauli Faulty
    QImage::Format_Invalid                  0   -
    QImage::Format_Mono                     1   No
    QImage::Format_MonoLSB                  2   No
    QImage::Format_Indexed8                 3   No
    QImage::Format_RGB32                    4   No
    QImage::Format_ARGB32                   5   No
    QImage::Format_ARGB32_Premultiplied     6   No
    QImage::Format_RGB16                    7   Yes XX
    QImage::Format_ARGB8565_Premultiplied   8   Yes XX
    QImage::Format_RGB666                   9   No
    QImage::Format_ARGB6666_Premultiplied   10  Yes X
    QImage::Format_RGB555                   11  Yes XXX
    QImage::Format_ARGB8555_Premultiplied   12  Yes XXX
    QImage::Format_RGB888                   13  No
    QImage::Format_RGB444                   14  Yes XXX
    QImage::Format_ARGB4444_Premultiplied   15  Yes XXX
    */
    return destImage;
}

void pan(int xc, int yc, float theta, int *dx, int *dy)
{
    float t = tan( theta * M_PI/180);
    float k = 1;
    // CX seemed 1.0 is batter than 1.2;
    // k can be optimized between 1 and 1.41421?
    *dx = yc * t * k, *dy = xc * t * k;
}

QImage blurLine( const QImage image, int x, int y, int xx, int yy)
{
    int w = xx - x;
    int h = yy - y;

    QRect rect = image.rect();

    float ang = atan((float)h/w)/M_PI*180;
    Gauss<int> bar( ang-90);
    vector < vector<int> > line = bar.getLine( x, y, xx, yy);
    vector<int> kernel = bar.getKernel( );
    int ksum = kernel[0] + 2 * (kernel[1] + kernel[2] + kernel[3] + kernel[4]);

    QImage outImage = QImage( image);
    QColor qc;

    // along the line
    int red(0), green(0), blue(0);
    for (unsigned k=0; k < line.size(); k++)
    {
        int lx = line[k][0], ly = line[k][1];

        if (!rect.contains( lx, ly)) continue;

        // to blur two sides
        for (int side=-1; side < 2; side += 2)
        {
            // bluring one side each time
            for (int b=0; b < ROWS; b++)
            {
                int bx = lx + side * bar.at( b)[0], by = ly + side * bar.at( b)[1];
                if (!rect.contains( bx, by)) continue;

                // get color of center pixel
                qc = QColor( image.pixel( bx, by));
                red += qc.red() * kernel[0];
                green += qc.green() * kernel[0];
                blue += qc.blue() * kernel[0];

                for ( int j=1; j<ROWS; j++)
                {
                    int xx = bar.at(j)[0], yy = bar.at(j)[1];
                    int x0 = bx+xx, y0 = by+yy;
                    int x1 = bx-xx, y1 = by-yy;

                    if (rect.contains( x0, y0))
                    {
                        // get color +neighbour pixels
                        qc = QColor( image.pixel( x0, y0));
                        red += qc.red() * kernel[j];
                        green += qc.green() * kernel[j];
                        blue += qc.blue() * kernel[j];
                    }
                    if (rect.contains( x1, y1))
                    {
                        // get color -neighbour pixels
                        qc = QColor( image.pixel( x1, y1));
                        red += qc.red() * kernel[j];
                        green += qc.green() * kernel[j];
                        blue += qc.blue() * kernel[j];
                    }
                }
                red /= ksum; green /= ksum; blue /= ksum;
                outImage.setPixel( bx, by, QColor( red, green, blue).rgba());
            }
        }
    }

    return outImage;
}

QImage blackWhiteImage(const QImage image)
{
    QImage destImage = QImage( image);
    QRect rect = image.rect();
    QColor qc;

    int H = rect.height();
    int W = rect.width();
    int bw;

    for (int i=0; i<H; i++)
        for (int j=0; j<W; j++)
        {
            qc = image.pixel( j, i);
            bw = (77 * qc.red() + 151 * qc.green() + 28 * qc.blue()) >> 8;
            qc.setRgb(bw, bw, bw);
            destImage.setPixel( j, i, qc.rgba());
        }

    return destImage;
}

QImage detectImage(const QImage image, int displayWhat)
{
    QImage destImage = QImage( image);
    QColor qc;

    int H = image.height();
    int W = image.width();

//#define DETECT_RING
#ifdef DETECT_RING
    Circle_detector detectorC = Circle_detector(100, 82, 50);

    // accumulate for circle test
    for (int i=0; i<H; i++)
        for (int j=0; j<W; j++)
        {
            qc = (QColor)image.pixel(j, i);
            if (qc.lightness() > 200)
                detectorC.vote(j, i);
        }

    // get circles
    vector<Form> circles = detectorC.get_circles();

    // display recoganized circles
    destImage = detectorC.display_circles(destImage, circles, 3);
#else
    Line_detector *detectorL = new Line_detector(W, H);

    // accumulate for lines
    for (int i=0; i<H; i++)
        for (int j=0; j<W; j++)
        {
            qc = (QColor)image.pixel(j, i);
            if (qc.lightness() > 200)
                detectorL->vote(j, i);
        }

    // get lines
    vector<Form> lines = detectorL->get_lines();
    detectorL->print(lines, MapV|LineFacts, -1);

    // black image
    if (!(displayWhat & 1))
        destImage.fill( 0);

    // display voting map
    if (displayWhat & 2)
        destImage = detectorL->display_voting_map(destImage, false);

    // display recoganized lines
    if (displayWhat & 4)
        destImage = detectorL->display_lines(destImage, lines, 3);

    delete detectorL;
#endif // DETECT_RING

    return destImage;
}

QColor colorPlus(int pixel)
{
    int plus = 55;

    QColor qc(pixel);
    int red = qc.red();
    int green = qc.green();
    int blue = qc.blue();

    if (blue != 1) blue += plus;
    else if (green != 1) green += plus;
    else if (red != 1) red += plus;

    if (blue > 255)
    {
        green += plus;
        blue = 255;
    }

    if (green > 255)
    {
        red += plus;
        green = 255;
        blue = 1;
    }

    if (red > 255)
    {
        blue += plus;
        red = 255;
        green = 1;
    }

    if (blue > 255)
    {
        green += plus;
        blue = 255;
    }

    qc.setRgb(red, green, blue);

    return qc;
}

QImage loadDataImage(const QImage image, float** data, int count, int scale)
{
    //////////////////////////////////////////////////////////////
    int W = 301, H = 201;
    int length = W * H;
    uchar *imageData = new uchar[4*length];
    QImage destImage = QImage( imageData, W, H, QImage::Format_ARGB32);
    destImage.fill(Qt::black);
    //////////////////////////////////////////////////////////////
    QColor qc;

    int* destData = (int *)destImage.bits();
    for (int i=0; i<count; i++)
    {
        int a = data[i][0];
        int b = data[i][1];

        a = a * scale + Offset;
        b = b * scale + Offset;

        qc = colorPlus(destData[a + b*W]);

        // show the data in an image zoomed to scale
        for (int i=-2; i<=2; i++)
            for (int j=-2; j<=2; j++)
                destData[a+i + (b+j)*W] = qc.rgba();
    }

    return destImage;
}

// read testdata from 3d gesture to form a virture image
QImage detectDataImage(const QImage image, float** data, int count, int scale)
{
    QImage destImage = QImage( image);

    Circle_detector detectorC = Circle_detector();

    for (int i=0; i<count; i++)
    {
        int a = data[i][0];
        int b = data[i][1];

        // send data for detecting directly
        detectorC.vote(a, b);
    }

    // get circles
    vector<Form> circles = detectorC.get_circles();

    // display recoganized circles
    destImage = detectorC.display_circles(destImage, circles, 3, scale);

    return destImage;
}


///////////////////////////////////////////////////////////
//                                                       //
// TESTERS                                               //
//                                                       //
///////////////////////////////////////////////////////////

QImage testColor( const QImage image, const int hue)
{
  QImage destImage = QImage( image);
  QRect rect = image.rect();
  destImage.fill( 0);
  QColor qc;

  float H = rect.height(), W = rect.width();

  for (int i=0; i<H; i++)
  {
    for (int j=0; j<W; j++)
    {
      if (!rect.contains( j, i)) continue;
      qc = image.pixel( j, i);
      qc = qc.toCmyk();
      qc.setCmyk( qc.cyan(), qc.magenta(), qc.yellow()*hue*0.01, qc.black());
      destImage.setPixel( j, i, qc.rgba());
    }
  }

  return destImage;
}

QImage testCmyk( QImage image)
{
  QImage destImage = QImage ( image);
  QRect rect = destImage.rect();
  destImage.fill( 0);

  int x = 0, H = 150;
  for (int i=0; i<32; i++)
    for (int j=0; j<32; j++)
      for (int k=0; k<32; k++)
      {
        // set a color
        Color<double> c (k*8, j*8, i*8);
        for (int p=0; p<7; p++)
        {
          QColor qc;
          switch (p)
          {
            case 0: { qc = Qt::red; break; }
            case 1: { qc = Qt::green; break; }
            case 2: { qc = Qt::blue; break; }
            case 3: { qc = Qt::cyan; break; }
            case 4: { qc = Qt::magenta; break; }
            case 5: { qc = Qt::yellow; break; }
            case 6: { qc = Qt::gray; break; }
          }
          double value = 0.5 * (c.toCmyk().getPrimary( (Color<double>::Primary)p));
          /*if (p>2)*/ destImage.setPixel( x, H - value, qc.rgba());
          //qDebug() << "p x value" << p << x << value ; // << value;
        }
        x++;
        if (x > 2048) { x = 0; H += 150; }
      }
  return destImage;
}

QImage bicubicColor( const QImage image, const int hue)
{
  QImage destImage = QImage( image);
  QRect rect = image.rect();
  destImage.fill( 0);

  int H = image.height(), W = image.width();

  Interpolator<double> intpol;

  for (int i=0; i<H; i++)
  {
    for (int j=0; j<W; j++)
    {
      if (!rect.contains( j, i)) continue;
      destImage.setPixel( j, i, intpol.bicubicDemo( W, H, j, i, hue).rgba());
    }
  }
  return destImage;
}

QImage bicubicColor9( const QImage image, const int hue)
{
    QImage destImage = QImage( image);
    QRect rect = image.rect();
    destImage.fill( 0);

    int H2 = rect.height()/2, W2 = rect.width()/2;
    int Cx = rect.center().x(), Cy = rect.center().y();

    Interpolator<double> intpol;

    for (int i=0; i<H2; i++)
    {
        int v1 = Cy-i, v2 = Cy+i;

        for (int j=0; j<W2; j++)
        {
            int u1 = Cx-j, u2 = Cx+j;

            if (!rect.contains( u1, v1)) continue;
            destImage.setPixel( u1, v1, intpol.bicubicDemo( W2, H2, u1, v1, hue).rgba());
            destImage.setPixel( u2, v1, intpol.bicubicDemo( W2, H2, u2, v1, hue).rgba());
            destImage.setPixel( u2, v2, intpol.bicubicDemo( W2, H2, u2, v2, hue).rgba());
            destImage.setPixel( u1, v2, intpol.bicubicDemo( W2, H2, u1, v2, hue).rgba());
        }
    }
    return destImage;
}

QImage bilinearColor( const QImage image, const int hue)
{
  QImage destImage = QImage( image);
  QRect rect = image.rect();
  destImage.fill( 0);

  int H = image.height(), W = image.width();

  Interpolator<double> intpol;

  for (int i=0; i<H; i++)
  {
    for (int j=0; j<W; j++)
    {
      if (!rect.contains( j, i)) continue;
      destImage.setPixel( j, i, intpol.bilinearDemo( W, H, j, i, hue).rgba());
    }
  }
  return destImage;
}

QImage bilinearColor9( const QImage image, const int hue)
{
    QImage destImage = QImage( image);
    QRect rect = image.rect();
    destImage.fill( 0);

    int H2 = rect.height()/2, W2 = rect.width()/2;
    int Cx = rect.center().x(), Cy = rect.center().y();

    Interpolator<double> intpol;

    for (int i=0; i<H2; i++)
    {
        int v1 = Cy-i, v2 = Cy+i;

        for (int j=0; j<W2; j++)
        {
            int u1 = Cx-j, u2 = Cx+j;

            if (!rect.contains( u1, v1)) continue;
            destImage.setPixel( u1, v1, intpol.bilinearDemo( W2, H2, u1, v1, hue).rgba());
            destImage.setPixel( u2, v1, intpol.bilinearDemo( W2, H2, u2, v1, hue).rgba());
            destImage.setPixel( u2, v2, intpol.bilinearDemo( W2, H2, u2, v2, hue).rgba());
            destImage.setPixel( u1, v2, intpol.bilinearDemo( W2, H2, u1, v2, hue).rgba());
        }
    }
    return destImage;
}

float** readTextFile(const char* filename, int *count)
{
    ifstream myfile(filename);
    float **data;

    if (!myfile.is_open())
    {
        cout << "Unable to open file";
        *count = 0;
        return data;
    }

    string line;

    if (myfile.good()) getline(myfile, line);
    stringstream lineStream(line);
    char text[20] = {' '};
    lineStream  >> text >> *count;
    data = new float*[*count];

    int i = 0;
    while (myfile.good())
    {
        char coma = ';';
        data[i] = new float[3];

        getline(myfile, line);

        if (line.find("#") != string::npos)
            continue;

        stringstream lineStream(line);

        int j = 0;
        while(lineStream.good())
            lineStream >> data[i][j++] >> coma;

        //qDebug() << data[i][0] << "," << data[i][1] << "," << data[i][2];

        i++;
    }
    *count = i - 1;

    myfile.close();

    return data;
}

/* BACKUP BACKUP BACKUP BACKUP BACKUP BACKUP BACKUP BACKUP BACKUP BACKUP
QImage distortImage( const QImage image, const float k3, int quality)
{
    QImage destImage = QImage( image);
    QRect rect = image.rect();
    destImage.fill( 0);

    float H2 = rect.height()/2, W2 = rect.width()/2;
    float R = sqrt( H2*H2 + W2*W2);
    float Cx = rect.center().x(), Cy = rect.center().y();
    float k1 = 1 - k3, k;

    Interpolator<double> intpol(2);

    for (int i=0; i<H2; i++)
    {
        int ii = i*i;
        float v1 = Cy-i, v2 = Cy+i;

        for (int j=0; j<W2; j++)
        {
            float r = sqrt( j*j + ii)/R;
            k = k1 + k3*r*r;

            float x = j * k;
            float y = i * k;

            float u1 = Cx-j, u2 = Cx+j;
            float x1 = Cx-x, x2 = Cx+x, y1 = Cy-y, y2 = Cy+y;

            if (!rect.contains( x1, y1)) continue;

            switch (quality)
            {
            case -1: // reverse nearest neighbour
                destImage.setPixel( x1, y1, image.pixel( u1, v1));
                destImage.setPixel( x2, y1, image.pixel( u2, v1));
                destImage.setPixel( x1, y2, image.pixel( u1, v2));
                destImage.setPixel( x2, y2, image.pixel( u2, v2));
                break;

            case 0: // nearest neighbour
                destImage.setPixel( u1, v1, image.pixel( x1, y1));
                destImage.setPixel( u2, v1, image.pixel( x2, y1));
                destImage.setPixel( u1, v2, image.pixel( x1, y2));
                destImage.setPixel( u2, v2, image.pixel( x2, y2));
                break;

            case 1: // bilinear
                destImage.setPixel( u1, v1, intpol.bilinear( image, x1, y1).rgba());
                destImage.setPixel( u2, v1, intpol.bilinear( image, x2, y1).rgba());
                destImage.setPixel( u2, v2, intpol.bilinear( image, x2, y2).rgba());
                destImage.setPixel( u1, v2, intpol.bilinear( image, x1, y2).rgba());
                break;

            case 2: // bicubic
                destImage.setPixel( u1, v1, intpol.bicubic( image, x1, y1).rgba());
                destImage.setPixel( u2, v1, intpol.bicubic( image, x2, y1).rgba());
                destImage.setPixel( u2, v2, intpol.bicubic( image, x2, y2).rgba());
                destImage.setPixel( u1, v2, intpol.bicubic( image, x1, y2).rgba());
                break;
            }
        }
    }
    return destImage;
}
*/
//////////////////////////////////////////////////////////
// function convertSphericalToCartesian( lat, lon)
// {
//   var x = cos( lat) * cos( lon);
//   var y = cos( lat) * sin( lon);
//   return new Cartesian( x,y);
// }
//////////////////////////////////////////////////////////

/*
// BACKUP BACKUP BACKUP BACKUP BACKUP BACKUP BACKUP BACKUP
//
// Cartesian to Spherical
//
QImage spherizeImage( const QImage image)
{
  QRect rect = image.rect();
  QImage destImage = QImage( image);
  float H2 = rect.height()/2, W2 = rect.width()/2;
  float Cx = rect.center().x(), Cy = rect.center().y();
  float Lati = M_PI/2.0f/H2;
  float Longi = M_PI/2.0f/W2;

  destImage.fill( 0);

  for (int i=0; i<H2; i++)
  {
    float sinY = sin( Lati * i), conY = cos( Lati * i);
    for (int j=0; j<W2; j++)
    {
      float sinX = sin( Longi * j);
      float u = sinX * conY * W2;
      float v = sinY * H2;

      float u1 = Cx-u, v1 = Cy-v, u2 = Cx+u, v2 = Cy+v;
      float x1 = Cx-j, y1 = Cy-i, x2 = Cx+j, y2 = Cy+i;

      destImage.setPixel( u1, v1, QColor( image.pixel( x1, y1)).rgba());
      destImage.setPixel( u2, v1, QColor( image.pixel( x2, y1)).rgba());
      destImage.setPixel( u2, v2, QColor( image.pixel( x2, y2)).rgba());
      destImage.setPixel( u1, v2, QColor( image.pixel( x1, y2)).rgba());
    }
  }
  return destImage;
}
*/
// cartian to fisheye
//   float r = sqrt(x*x + y*y)
//   float phi = atan(y/x)
// Fisheye to cartian
//   float u = r * cos( phi) // * W2;
//   float v = r * sin( phi) // * H2;

/*
QImage polarImage( const QImage image)
{
  QRect rect = image.rect();
  QImage destImage = QImage( image);
  float H2 = rect.height()/2, W2 = rect.width()/2;
  float Cx = rect.center().x(), Cy = rect.center().y();
  float Lati = M_PI/2.0f/H2;
  float Longi = M_PI/2.0f/W2;

  destImage.fill( 0);

  for (int i=0; i<H2; i++)
  {
    float sinY = sin( Lati * i), conY = cos( Lati * i);
    for (int j=0; j<W2; j++)
    {
      float sinX = sin( Longi * j);

      float x = sinX * conY * W2;
      float y = sinX * sinY * H2;
      //destImage.setPixel( Cx-x, Cy-y, QColor( image.pixel( Cx-j, Cy-i)).rgba());
      //destImage.setPixel( Cx+x, Cy-y, QColor( image.pixel( Cx+j, Cy-i)).rgba());
      destImage.setPixel( Cx+x, Cy+y, QColor( image.pixel( Cx+j, Cy+i)).rgba());
      destImage.setPixel( Cx-x, Cy+y, QColor( image.pixel( Cx-j, Cy+i)).rgba());
    }
  }
  return destImage;
}
*/

/*
QImage polarMesh( const QImage image)
{
  QRect rect = image.rect();
  QImage destImage = QImage( image);
  float H2 = rect.height()/2, W2 = rect.width()/2;

  QPainter painter( &destImage);
  painter.setRenderHint( QPainter::SmoothPixmapTransform);
  painter.setPen ( QColor(255, 255, 0));

  destImage.fill( 0);

  int N = 30;
  float r[4], theta[4], phi[4];
  QPoint C( rect.center()), p[4];
  float scale = M_PI / N;

  for (int j=0; j<N/2; j++) {
    for (int i=-N/2; i<N/2; i++) {
      theta[0] = j * scale;
      theta[1] = (j+1) * scale;
      theta[2] = theta[1];
      theta[3] = theta[0];
      phi[0] = i * 2 * scale;
      phi[1] = phi[0];
      phi[2] = (i+1) * 2 * scale;
      phi[3] = phi[2];

      for (int k=0; k<4; k++) {
        float y = sin( theta[k]);
        p[k].setX( y * sin( phi[k]) * W2);
        p[k].setY( y * cos( phi[k]) * H2);
        //p[k].z = cos(theta[k]);
      }
      for (int k=0; k<4; k++)
        r[k] = p[k].x()*p[k].x() + p[k].y()*p[k].y();
      //if (r[0] > 1 || r[1] > 1 || r[2] > 1 || r[3] > 1)
      //  continue;
      //for (k=0;k<4;k++)
      //   p[k].z = sqrt(1 - r[k]);
      //draw the polygon p[0], p[1], p[2], p[3]

      painter.drawLine( C+p[0], C+p[1]);
      painter.drawLine( C+p[1], C+p[2]);
      painter.drawLine( C+p[2], C+p[3]);
      painter.drawLine( C+p[3], C+p[0]);
    }
  }
  return destImage;
}

QImage cartesianMesh( const QImage image)
{
  QRect rect = image.rect();
  QImage destImage = QImage( image);
  float H2 = rect.height()/2, W2 = rect.width()/2;

  QPainter painter( &destImage);
  painter.setRenderHint( QPainter::SmoothPixmapTransform);
  painter.setPen ( QColor(255, 255, 0));

  destImage.fill( 0);

  int N = 20;
  float scale = 2.f / N * H2, r[4];
  QPoint C( rect.center()), p[4];

  for (int j=-N/2; j<N/2; j++) {
    for (int i=-N/2; i<N/2; i++) {
      p[0].setY( j * scale);
      p[1].setY( (j+1) * scale);
      p[2].setY( p[1].y());
      p[3].setY( p[0].y());
      p[0].setX( i * scale);
      p[1].setX( p[0].x());
      p[2].setX( (i+1) * scale);
      p[3].setX( p[2].x());
      for (int k=0; k<4; k++)
        r[k] = p[k].x()*p[k].x()+p[k].y()*p[k].y();
      //if (r[0] > 1 || r[1] > 1 || r[2] > 1 || r[3] > 1)
      //  continue;
      //for (k=0;k<4;k++)
      //   p[k].z = sqrt(1 - r[k]);
      // draw the polygon p[0], p[1], p[2], p[3]

      painter.drawLine( C+p[0], C+p[1]);
      painter.drawLine( C+p[1], C+p[2]);
      painter.drawLine( C+p[2], C+p[3]);
      painter.drawLine( C+p[3], C+p[0]);
    }
  }
  return destImage;
}
*/
