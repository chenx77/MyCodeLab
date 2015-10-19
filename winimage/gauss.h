#ifndef GAUSS_H
#define GAUSS_H

#include <iostream>
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif
using namespace std;

template <typename T> class Gauss;

const int ROWS = 5;
const int COLS = 2;
const int matrix [ROWS] = {8, 8, 2, 1, 1};
const int actant[][ROWS][COLS] =
{
  // a short line (5 pixels) with angle = 0-45 degree, or slop m =
  { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}}, // 0: [      0,  22/160)
  { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 1}}, // 1: [ 22/160,  28/160)
  { {0, 0}, {1, 0}, {2, 0}, {3, 1}, {4, 1}}, // 2: [ 28/160,  41/160)
  { {0, 0}, {1, 0}, {2, 1}, {3, 1}, {4, 1}}, // 3: [ 41/160,  61/160)
  { {0, 0}, {1, 0}, {2, 1}, {3, 1}, {4, 2}}, // 4: [ 61/160,  81/160)
  { {0, 0}, {1, 1}, {2, 1}, {3, 2}, {4, 2}}, // 5: [ 81/160, 101/160)
  { {0, 0}, {1, 1}, {2, 1}, {3, 2}, {4, 3}}, // 6: [101/160, 107/142)
  { {0, 0}, {1, 1}, {2, 2}, {3, 2}, {4, 3}}, // 7: [107/142, 107/128)
  { {0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 3}}, // 8: [107/128, 107/122)
  { {0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}}  // 9: [107/122, 1)
};

template<class T>
class Gauss
{
  // declare a vector of vectors of type T
  vector < vector<T> > s;

public:
  enum Quad { O, I, II, III, IV };
  // initialize the size of s to ROWS by COLS
  Gauss( T const dx, T const dy);
  Gauss( const float ang);
  Gauss( T const in[][COLS]);
  Gauss( ) : s(ROWS, vector<T>(COLS)) { init( 0); }

  void init( const float ang);
  void setQuadrant( const Quad quad);
  void flip( void);
  vector<T> at( const int i);
  vector < vector<T> > getLine( int x, int y, int x2, int y2);
  vector<T> getKernel( void) { return _kernel; }
  void read();
  void print();

private:
  Quad _quadrant;
  vector<T> _kernel;
};

//
// Dont understand why should I put definition of getLine to the
// header file. Gcc linker will compalain otherwise:
//   "undefined reference to `Gauss<int>::getLine..."
// As other member functions, like init(), at() doing well in
// seperated cpp file
//
template<class T>
vector < vector<T> > Gauss<T>::getLine( int x, int y, int x2, int y2)
{
  int w = x2 - x;
  int h = y2 - y;
  int dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;

  if (w<0) dx1 = -1; else if (w>0) dx1 = 1;
  if (h<0) dy1 = -1; else if (h>0) dy1 = 1;
  if (w<0) dx2 = -1; else if (w>0) dx2 = 1;

  int longest = abs((float)w);
  int shortest = abs((float)h);
  if ( !(longest>shortest)) {
    longest = abs((float)h);
    shortest = abs((float)w);
    if (h<0) dy2 = -1;
    else if (h>0) dy2 = 1;
    dx2 = 0;
  }

  vector < vector<T> > vout;
  //vector vout(ROWS, vector<T>(COLS));
  int numerator = longest >> 1;
  for ( int i=0; i<=longest; i++) {

    vector<T> v;
    v.push_back( x);
    v.push_back( y);
    vout.push_back( v);

    numerator += shortest;
    if ( !(numerator<longest)) {
      numerator -= longest;
      x += dx1;
      y += dy1;
    } else {
      x += dx2;
      y += dy2;
    }
  }
  return vout;
}

////////////////////
//
// Testers
//
void testGuass( void);
vector< vector<int> > testLine ( void);

#endif /* GAUSS_H */
