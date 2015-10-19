#include "interpolator.h"
#include "winimage.h"
#include "gauss.h"

#include <QApplication>
#include <QDebug>

int main( int argc, char* argv[] )
{
  QApplication myapp( argc, argv );

  WinImage* mywidget = new WinImage();
  mywidget->resize( 640, 480);
  mywidget->show();

  return myapp.exec();
}

