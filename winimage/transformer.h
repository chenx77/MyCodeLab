#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include <QImage>

// Windows
QImage classicWinBlur(QImage image, const int radius);
QImage curvesWinImage( QImage image);
QImage inflateWinImage( QImage image, const float level);
QImage resizeWinImage(QImage image, const float level);
QImage zoomWinImage(QImage image, const float level);
QImage blendingWinImage( QImage base, QImage top, int blendingMode);

//QImage classic_blur(QImage image, const float level);
QImage bicubicColor( const QImage image, const int hue);
QImage bicubicColor9( const QImage image, const int hue);
QImage bilinearColor( const QImage image, const int hue);
QImage bilinearColor9( const QImage image, const int hue);
QImage blurLine( const QImage image, int x, int y, int x2, int y2);
QImage diffImage( QImage image, QImage imageExp, const float level);
QImage distortImage( const QImage image, const float k3, int quality);
QImage formatImage(QImage image, const int format);
QImage inflateImage( QImage image, const float level, int quality);
QImage spherizeImage( const QImage image);
QImage spherizeWinImage( QImage image, const float level);
QImage testColor( const QImage image, const int hue);
QImage testCmyk( const QImage image);
QImage topSpherizeImage( const QImage image);
QImage zoomImage( const QImage image, const float scale, int quality);
QImage fisheyeImage( const QImage image, const float scale);
QImage kaleidoscopeImage( const QImage image, const float length);
QImage blackWhiteImage(const QImage image);
QImage detectImage(const QImage image, int diaplayWhat);
QImage detectDataImage(const QImage image, float** data, int count, int scale);
QImage loadDataImage(const QImage image, float** data, int count, int scale);
float** readTextFile(const char* filename, int *count);
#endif /* TRANSFORMER_H */
