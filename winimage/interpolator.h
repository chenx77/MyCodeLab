#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <iostream>
#include <QtGui>

using namespace std;

template<class T>
class Color
{
public:
  enum Mode { RGBA, CMYK };
  enum Primary { RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW, BLACK };

  // construct with QRgb
  explicit Color( QRgb qRgb = QRgb( 0))
  {
    _mode = RGBA;
    _red = qRed( qRgb);
    _green = qGreen( qRgb);
    _blue = qBlue( qRgb);
    _alpha = qAlpha( qRgb);
  }

  // constructor with T type
  explicit Color( T red, T green, T blue, T alpha=0)
  {
    _mode = RGBA;
    _red = red;
    _green = green;
    _blue = blue;
    _alpha = alpha;
  }

  // constructor with T type
  explicit Color( Mode mode, T cR, T mG, T yB)
  {
    _mode = mode;

    if (mode == RGBA)
    {
      _cyan = cR;
      _magenta = mG;
      _yellow = yB;
      _alpha = 0;
    }
    else
    {
      _cyan = cR;
      _magenta = mG;
      _yellow = yB;
      _black = min(cR, min(mG, yB));
    }
  }

  // define overloaded + (plus) operator
  Color operator+ (const Color &color) const
  {
    Color result = *this;
    result._red += color._red;
    result._green += color._green;
    result._blue += color._blue;
    result._alpha += color._alpha;
    return result;
  }

  // define overloaded - (minus) operator
  Color operator- (const Color &color) const
  {
    Color result = *this;
    result._red -= color._red;
    result._green -= color._green;
    result._blue -= color._blue;
    result._alpha -= color._alpha;
    return result;
  }

  // define overloaded * (multiply) operator
  Color operator* (const T scalar) const
  {
    Color result = *this;
    result._red *= scalar;
    result._green *= scalar;
    result._blue *= scalar;
    result._alpha *= scalar;
    return result;
  }

  // 'friend' makes * commutative
  friend Color operator* (const T& scalar, const Color& rhs)
  {
    return rhs * scalar;
  }

  friend ostream &operator<< ( ostream &out, Color &color)
  {
    Mode m = color.getMode();
    out << "Color(";
    if (m==RGBA)
      out << "RGBA " << color._red << " " << color._green
          << " " << color._blue << " " << color._alpha << ")";
    else out << "CMYK " << color._cyan << " " << color._magenta
             << " " << color._yellow << " " << color._black << ")";
    return out;
  }

  Mode getMode( void) { return _mode; }

  QRgb rgba( void) const
  {
    QColor result( max(0, min(255, (int)_red)), max(0, min(255, (int)_green)), max(0, min(255, (int)_blue)), max(0, min(255, (int)_alpha)));
    return result.rgba();
  }

  int integer( void) const
  {
    int result = (int)_alpha << 24;
    result += (int)_red << 16;
    result += (int)_green << 8;
    result += (int)_blue;
    return result;
  }

  // to be implemented later
  Color toCmyk( void)
  {
    _mode = CMYK;
    _cyan = 255 -_red;
    _magenta = 255 -_green;
    _yellow = 255 -_blue;

    _black = min(_cyan, min( _magenta, _yellow));
    T white = 255-_black;

    _cyan = (_cyan-_black)/white * 255;
    _magenta = (_magenta-_black)/white * 255;
    _yellow = (_yellow-_black)/white * 255;

    return *this;
  }

  Color toRgb()
  {
    if (_mode!=RGBA)
    {
      _mode = RGBA;
      _red = (255 -_cyan)/(255 -_black) * 255;
      _green = (255 -_magenta)/(255 -_black) * 255;
      _blue = (255 -_yellow)/(255 -_black) * 255;
      _alpha = 0;
    }
    return *this;
  }

  T getPrimary( Primary color)
  {
    switch (color)
    {
      case RED: return _red;
      case GREEN: return _green;
      case BLUE: return _blue;
      case CYAN: return _cyan;
      case MAGENTA: return _magenta;
      case YELLOW: return _yellow;
      case BLACK: return _black;
      default: return 0;
    }
  }

private:
  Mode _mode;
  T _red, _green, _blue, _alpha;
  T _cyan, _magenta, _yellow, _black;
};


template<class T>
class Interpolator
{
private:
  // Catmull-Rom spline
  Color<T> cubic( Color<T> p[4], T fx)
  {
    Color<T> result = p[1] + 0.5*fx*(p[2] - p[0] + fx*(2.0*p[0] - 5.0*p[1] +
                      4.0*p[2] - p[3] + fx*(3.0*(p[1] - p[2]) + 1*p[3] - p[0])));
    return result;
  }

public:
  /*int _quality;

  Interpolator (int quality=1)
  {
    _quality = quality;
  }
  */
  Color<T> bicubic( QImage image, T x, T y)
  {
    int H = image.height(), W = image.width();

    T ix, iy;
    T fx = modf( x, &ix), fy = modf( y, &iy);

    Color<T> p[4][4];
    for (int m=0; m<4; m++)
    {
        iy = max(0, min(H-1, (int)y+m-1));
        for (int n=0; n<4; n++)
        {
            ix = max(0, min(W-1, (int)x+n-1));
            p[m][n] = Color<T> (image.pixel( ix, iy));
        }
    }
    return bicubic( p, fx, fy);
  }

  Color<T> bicubic( Color<T> p[4][4], T fx, T fy)
  {
    Color<T> result[4];

    result[0] = cubic( p[0], fx);
    result[1] = cubic( p[1], fx);
    result[2] = cubic( p[2], fx);
    result[3] = cubic( p[3], fx);

    return cubic(result, fy);
  }

  Color<T> bilinear( const QImage image, const T x, const T y)
  {
    QRect rect = image.rect();

    T ix, iy;
    T fx = modf( x, &ix), fy = modf( y, &iy);

    QRgb q00, q10, q01, q11;

    if (rect.contains( ix, iy))     q00 = image.pixel( ix, iy);
    if (rect.contains( ix+1, iy))   q10 = image.pixel( ix+1, iy);
    if (rect.contains( ix, iy+1))   q01 = image.pixel( ix, iy+1);
    if (rect.contains( ix+1, iy+1)) q11 = image.pixel( ix+1, iy+1);

    Color<T> c00( q00), c10( q10), c01( q01), c11( q11);

    Color<T> result = (c00*(1-fx)+c10*fx)*(1-fy) + (c01*(1-fx)+c11*fx)*fy;

    return result;
  }

  int bilinear( const int* srcData, const int width, const int height, const T x, const T y)
  {
    T ix, iy;
    T fx = modf( x, &ix), fy = modf( y, &iy);
    QRgb q00, q10, q01, q11;

    q00 = srcData[(int)(ix+(int)iy*width)];

    if (ix+1<=width) q10 = srcData[(int)((ix+1)+(int)iy*width)];
    else q10 = q00;

    if (iy+1<=height) q01 = srcData[(int)(ix+(int)(iy+1)*width)];
    else q01 = q00;

    if (ix+1<=width && iy+1<=height) q11 = srcData[(int)((ix+1)+(int)(iy+1)*width)];
    else q11 = q00;

    Color<T> c00( q00), c10( q10), c01( q01), c11( q11);

    Color<T> result = (c00*(1-fx)+c10*fx)*(1-fy) + (c01*(1-fx)+c11*fx)*fy;

    return result.integer();
  }

  // Colour rectangular demo
  Color<T> bilinearDemo( int width, int height, const T x, const T y, const int hue)
  {
    QColor c; c.setHsv ( hue, 255, 255);
    T fx = x/width, fy = y/height;

    Color<T> r( 255, 0, 0);
    Color<T> g( 0, 255, 0);
    Color<T> b( 0, 0, 255);
    Color<T> h( c.rgba());

    Color<T> result = (r*(1-fx)+g*fx)*(1-fy) + (b*(1-fx)+h*fx)*fy;

    return result;
  }

  // Colour rectangular demo
  Color<T> bicubicDemo( int width, int height, const T x, const T y, const int hue)
  {
    QColor c; c.setHsv ( hue, 255, 255);
    T fx = x/width, fy = y/height;

    Color<T> r( 255, 0, 0);
    Color<T> g( 0, 255, 0);
    Color<T> b( 0, 0, 255);
    Color<T> h( c.rgba());
    Color<T> k( 0, 0, 0);
    Color<T> w( 255, 255, 255);

    Color<T> p[4][4] = {{r, r, g, g},
                        {r, r, g, g},
                        {b, b, h, h},
                        {b, b, h, h}};
    Color<T> result = bicubic( p, fx, fy) ;

    return result;
  }
};

//////////////////////
//
// TESTS
//
template <typename T>
//
// Non-graphic/Console tester
//
void test( void)
{
  //Color color1( QRgb( 210, 200, 100));
  //Color color2 = color2 - color1;
  //Color color3 = color2 * 0.3123412341;
  //Color color4 = Color ( QRgb( 255, 255, 255));

  //qDebug() << color1.rgba();

  Color<T> b1( 0, 0, 255);
  Color<T> b2 = Color<T> ( 255, 0, 0) - b1;
  Color<T> b3 = Color<T> ( 255, 0, 0) - b1;
  Color<T> b4 = b1 - b2 - b3 + Color<T> ( 0, 255, 0);

  T fx = 0.99, fy = 0.986667;

  Color<T> acolor = b1 + b2*fx + b3*fy + b4*fx*fy;

  qDebug() << "fx fy fx*fy" << fx << fy << fx*fy;
  b1.print();
  (b1-b2).print();
  (b1-b2-b3).print();
  b4.print();
  acolor.print();
}

#endif /* INTERPOLATOR_H */
