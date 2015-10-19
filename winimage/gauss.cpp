#include "gauss.h"

// Can C++ call constructor from constructor? We can do that in C#, but
// unfortunately we can't in C++. Using MyClass::init() is the right way to
// handle the issue in C++. See more from link:
// http://stackoverflow.com/questions/308276/c-call-constructor-from-constructor

template<class T>
Gauss<T>::Gauss( T const dx, T const dy) : s(ROWS, vector<T>(COLS))
{
  float a = atan( (float)dy/dx)/M_PI*180;
  init( a);
}

template<class T>
Gauss<T>::Gauss( const float ang) : s(ROWS, vector<T>(COLS))
{
  float a = (ang < 0)? 360+ang : ang;
  init( a);
}

template<class T>
Gauss<T>::Gauss( T const in[][COLS]) : s(ROWS, vector<T>(COLS))
{
  _quadrant = O;
  for(int i = 0; i < ROWS; i++)
    for(int j = 0; j < COLS; j++)
      this->s[i][j] = in[i][j];
}

template<class T>
void Gauss<T>::read()
{
  for(int i = 0; i < ROWS; i++)
    for(int j = 0; j < COLS; j++)
      cin >> this->s[i][j];
}

template<class T>
void Gauss<T>::print()
{
  cout << "-------------------------- quadrant " << _quadrant << endl;

  vector < vector<int> >::iterator ps = s.begin();
  while (ps < s.end()) cout << "(" << (*ps++)[0] << "," << (*ps)[1] << ") ";
  cout << endl;

  vector<int>::iterator pk = _kernel.begin();
  cout << "(";
  while (pk < _kernel.end()) cout << *pk++ << " ";
  cout << ")" << endl;
}

template<class T>
void Gauss<T>::flip( void)
{
  T temp;
  for (int i = 0; i < ROWS; i++)
  {
    temp = this->s[i][0];
    this->s[i][0] = this->s[i][1];
    this->s[i][1] = temp;
  }
}

template<class T>
void Gauss<T>::setQuadrant( const Quad quad)
{
  _quadrant = quad;
  for (int i = 0; i < ROWS; i++)
  {
      if (quad == II || quad == III)
        this->s[i][0] = -this->s[i][0];
      if (quad == IV || quad == III)
        this->s[i][1] = -this->s[i][1];
  }
}

template<class T>
vector<T> Gauss<T>::at( const int i)
{
  return s.at(i);
}

template<class T>
void Gauss<T>::init( const float ang)
{
  bool flip(false);
  float a = ang;
  if (a > 180) a = 360 - a;                // flip over y=0
  if (a > 90) a = 180 - a;                 // flip over x=0
  if (a > 45) { a = 90 - a; flip = true; } // flip over x=y

  float m = (float)tan( M_PI*a/180);

  int k=0;
  if ( m < 22.0/160.0)     k=0;
  else if (m < 22.0/160.0) k=1;
  else if (m < 28.0/160.0) k=2;
  else if (m < 41.0/160.0) k=3;
  else if (m < 61.0/160.0) k=4;
  else if (m < 81.0/160.0) k=5;
  else if (m <101.0/160.0) k=6;
  else if (m <107.0/142.0) k=7;
  else if (m <107.0/128.0) k=8;
  else if (m <= 1.0)       k=9;

  // set vertor pattern
  for (int i = 0; i < ROWS; i++)
    for (int j = 0; j < COLS; j++)
      this->s[i][j] = actant[k][i][j];
  ////////////////////////////////////////////////////////////
  //vector < vector<int> > ss( *actant[k], (*actant[k])+ROWS);
  //s = ss;

  // flip x, y if angle greater than 45
  if (flip) this->flip();

  // set quadrant
  Quad quad = (ang > 270)? IV : (ang > 180)? III : (ang > 90)? II : I;
  setQuadrant( quad);
  vector<int> kernel( matrix, matrix+ROWS);

  _kernel = kernel;

  //print();
}

////////////////////
//
// Testers
//

void testGuass( void)
{
  int test[5][2] = { {0, 0}, {1, 0}, {2, 1}, {3, 1}, {4, 2}}; // 4: [ 61/160,  81/160)

  // default creator
  Gauss<int> a; a.print();

  // copy creator
  Gauss<int> b( a); b.print();

  // angle creator
  Gauss<int> c( 30.0); //c.print();

  // angle = 0 - 360 degreee, delta 30
  for ( int i=0; i<12; i++)
  {
    float angle = 30 * i - 1;
    cout << endl << "c.init(" << angle << ") is:" << endl;
    c.init( angle);
  }

  // vector creator
  Gauss<int> d( test); d.print();

  // flip it
  d.flip(); d.print();

  // get point at 2, middle of five
  cout << "### d.at( 4) = (" << d.at( 2)[0] << ", " << d.at( 2)[1] << ")" << endl;
}

vector< vector<int> > testLine ( void)
{
  // constructors used in the same order as described above:
  vector<int> first;                                // empty vector of ints
  vector<int> second (2,100);                       // four ints with value 100
  vector<int> third (second.begin(),second.end());  // iterating through second
  vector<int> fourth (third);                       // a copy of third

  // the iterator constructor can also be used to construct from arrays:
  int myints[] = {16,2,77,29};
  vector<int> fifth (myints, myints + sizeof(myints) / sizeof(int) );

  ///////////////////////////
  //first.clear();
  //first.push_back( 1);
  //first.push_back( 2);

  vector < vector<int> > v;
/*
  for (int k=0; k<ROWS; k++)
  {
    vector<int> w;
    w.push_back(k);
    w.push_back(k*2);
    v.push_back( w);
  }
*/
  v.push_back( first);
  v.push_back( second);
  v.push_back( third);
  v.push_back( fourth);
  v.push_back( fifth);

  cout << "v.size() => " << v.size() << endl;
  cout << "v.capacity() => " << v.capacity() << endl;
  cout << "v.empty() => " << v.empty() << endl;
  cout << "v.max_size() => " << v.max_size() << endl;
  cout << "v[1] => " << v[1][0] << endl;

  Gauss<int> bar( 45);
  vector < vector<int> > vec = bar.getLine( 0, 0, 30, 200);
  for (int k=1; !vec[k].empty(); k++)
  {
    cout << endl << "v[" << k << "] =>";
    for (unsigned j=0; j < vec[k].size(); j++)
      cout << " " << vec[k][j];
  }
  ///////////////////////////

  cout << endl << "The contents of fifth are:";
  for (unsigned int i=0; i < fifth.size(); i++)
    cout << " " << fifth[i];

  cout << endl;

  return v;
}
