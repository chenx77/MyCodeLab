#include "winimage.h"
#include "transformer.h"
#include "edge_detector.h"
#include "rotater.h"

#include <time.h>
#include <QtGui>
#include <QMessageBox>
#include <QActionGroup>
#include <QToolBar>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QFileDialog>
#include <QPainter>
#include <QTime>

WinImage::WinImage( QWidget* parent) : QMainWindow( parent)
{
    setupFileMenu();
    setupEditMenu();
    setupColorMenu();
    setupToolsMenu();

    _offset = QPoint( 10, 30);

    _a_number = new QDoubleSpinBox();
    _a_number->setMaximum ( 30.0);
    _a_number->setMinimum (-30.0);
    _a_number->setSingleStep( 0.1);
    _a_number->setDecimals ( 2);
    _a_number->setValue( 7);

    _check_box = new QCheckBox ( "Reverse");

    statusBar()->addPermanentWidget (_check_box);
    statusBar()->addPermanentWidget (_a_number);

    // for rotation & detection
    _alpha = 90; // the alpha should between (0, 180) excluding ends, better to be [135, 45]
    _scale = 1; // no re-scaling, 0 => re-scaling  // scale to display circle in faked image
    _x0 = 100; // interesting join point postion
}

WinImage::~WinImage()
{
    // does this work?
    if (!_image.isNull())
        _image.detach();
}

void WinImage::paintEvent( QPaintEvent* )
{
    QPainter painter( this);
    painter.setRenderHint( QPainter::SmoothPixmapTransform );

    QTransform transform;
    transform.translate( _offset.x(), _offset.y());
    painter.setTransform( transform);

    painter.drawImage ( _image.rect(), _image);
}

///////////////////////////////////////////////////////////
//                                                       //
// Tools Menu                                            //
//                                                       //
///////////////////////////////////////////////////////////
void WinImage::setupToolsMenu()
{
    // Actions
    QAction* bigO = new QAction( "Bi&gO", this );
    bigO->setShortcut( Qt::CTRL + Qt::Key_G );
    bigO->setStatusTip("Big O");
    connect( bigO, SIGNAL( triggered() ), this, SLOT( slotBigO()));

    QAction* zoom = new QAction( "&Zoom", this );
    zoom->setShortcut( Qt::CTRL + Qt::Key_Z );
    zoom->setStatusTip("Zoom with scale");
    connect( zoom, SIGNAL( triggered() ), this, SLOT( slotZoom()));

    QAction* differ = new QAction( "Di&ffer", this );
    differ->setShortcut( Qt::CTRL + Qt::Key_F );
    differ->setStatusTip("Compare generated image against expected");
    connect( differ, SIGNAL( triggered() ), this, SLOT( slotDiffer()));

    QAction* format = new QAction( "&Change format", this );
    format->setShortcut( Qt::CTRL + Qt::Key_C );
    format->setStatusTip("Change format: 1 to 15");
    connect( format, SIGNAL( triggered() ), this, SLOT( slotChangeFormat()));

    QAction* test1 = new QAction( "Test &1: black white ", this );
    test1->setShortcut( Qt::CTRL + Qt::Key_1 );
    test1->setStatusTip("Test case 1");
    connect(test1, SIGNAL( triggered()), this, SLOT( slotTest1()));

    QAction* test2 = new QAction( "Test &2: smooth", this );
    test2->setShortcut( Qt::CTRL + Qt::Key_2 );
    test2->setStatusTip("Test case 2");
    connect(test2, SIGNAL( triggered()), this, SLOT( slotTest2()));

    QAction* test3 = new QAction( "Test &3: detect edges", this );
    test3->setShortcut( Qt::CTRL + Qt::Key_3 );
    test3->setStatusTip("Test case 3");
    connect(test3, SIGNAL( triggered()), this, SLOT( slotTest3()));

    QAction* test4 = new QAction( "Test &4: non-maximum suppression", this );
    test4->setShortcut( Qt::CTRL + Qt::Key_4 );
    test4->setStatusTip("Test case 4");
    connect(test4, SIGNAL( triggered()), this, SLOT( slotTest4()));

    QAction* test5 = new QAction( "Test &5: double thresholding", this );
    test5->setShortcut( Qt::CTRL + Qt::Key_5 );
    test5->setStatusTip("Test case 5");
    connect(test5, SIGNAL( triggered()), this, SLOT( slotTest5()));

    QAction* test6 = new QAction( "Test &6: blob extract", this );
    test6->setShortcut( Qt::CTRL + Qt::Key_6 );
    test6->setStatusTip("Test case 6");
    connect(test6, SIGNAL( triggered()), this, SLOT( slotTest6()));

    QAction* test7 = new QAction( "Test &7: detect all lines", this );
    test7->setShortcut( Qt::CTRL + Qt::Key_7 );
    test7->setStatusTip("Test case 7");
    connect(test7, SIGNAL( triggered()), this, SLOT( slotTest7()));

    QAction* test8 = new QAction( "Test &8: rotate blackout", this );
    test8->setShortcut( Qt::CTRL + Qt::Key_8 );
    test8->setStatusTip("Test case 8");
    connect(test8, SIGNAL( triggered()), this, SLOT( slotTest8()));

    QAction* test9 = new QAction( "Test &9: blackout corners", this );
    test9->setShortcut( Qt::CTRL + Qt::Key_9 );
    test9->setStatusTip("Test case 9");
    connect(test9, SIGNAL( triggered()), this, SLOT( slotTest9()));

    QAction* test0 = new QAction( "Test &0: detect master lines", this );
    test0->setShortcut( Qt::CTRL + Qt::Key_0 );
    test0->setStatusTip("Test case 0");
    connect(test0, SIGNAL( triggered()), this, SLOT( slotTest0()));

    QMenu* tools = new QMenu( "&Tools", menuBar() );
    menuBar()->addMenu( tools);
    tools->addAction( bigO);
    tools->addAction( zoom);
    tools->addAction( differ);
    tools->addAction( format);
    tools->addAction( test1);
    tools->addAction( test2);
    tools->addAction( test3);
    tools->addAction( test4);
    tools->addAction( test5);
    tools->addAction( test6);
    tools->addAction( test7);
    tools->addAction( test8);
    tools->addAction( test9);
    tools->addAction( test0);
}

void WinImage::slotBigO()
{
    int counter = 0, steps = 10000;
    double full_circle = 2*M_PI;
    double delta_angle = full_circle / steps;
    double sigma = 0, r;
    bool linear = true;

    if (_check_box->isChecked())
        linear = false;

    time_t start, end;
    double dif;
    time (&start);
    while (counter++ < 100000)
    {
        int x = qrand(), y = qrand();
        for (double theta=0; theta<full_circle; theta+=delta_angle)
        {
            if (linear) r = y + theta * x;
            else r = x * cos(theta) + y * sin(theta);
//            sigma += r;
        }
    }
    time (&end);
    dif = difftime (end, start);
    qDebug() << "Total time spent: " << dif; // << "sigma" << sigma;
}

void WinImage::slotZoom()
{
    double scale = _a_number->value();
    scale = (scale==0) ? 1 : 1 / scale;
    _image = zoomImage(_image, scale, 0);

    update();

    QString message = "Zoom scale: " + QString::number( _a_number->value());
    statusBar()->showMessage( message);
}

void WinImage::slotDiffer()
{
    // Differ specific
    QImage imageExpected;
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
        imageExpected = QImage( fileName);
    _image = diffImage( _image, imageExpected, _a_number->value());

    update();

    //printf("File %s differs ", fileName);
    QString message = "Differs... ";
    statusBar()->showMessage( message);
}

/// Hide Roll Out Corners
void WinImage::slotTest9()
{
    // test get_line_cross_point()
    qreal theta = _a_number->value(); // rotating angle
//    QImage blackImage = QImage(_image);
//    blackImage.fill("white");

    Rotater *rotater = new Rotater(_image, theta);
//    _image = rotater->RotateImage(_image, blackImage);
    _image = rotater->HideRollOutCorners(_image);

// test 1+3+4+5+6+8+9
//    qreal theta = _a_number->value(); // rotating angle
//    _image = blackWhiteImage(_image);

//    Rotater *rotater = new Rotater(_image, theta);
//    vector<Form> lines;
//    _image = rotater->readLines(_image, &lines);
//    delete rotater;

// test line detection
//    edge_detector ed = edge_detector();
//    _image = ed.blob_extract(_image);

    update();
}


void WinImage::slotTest1()
{
    _image = blackWhiteImage(_image);
    update();
}

void WinImage::slotTest2()
{
    // test smooth
    edge_detector ed = edge_detector();
    _image = ed.smoothImage( _image);

    update();

// older test resize & blur
//    float scale = _a_number->value();
//    _image = resizeWinImage( _image, scale);

//    resize(_image.width() + 2*_offset.x(), _image.height() + 2*_offset.y());
//    update();
//    _image = classicWinBlur( _image, 13);

//    QString message = "Test 1: resizeImage to " +
//        QString::number( scale) + " then classicBlur radius=13";
//    statusBar()->showMessage( message);
}

void WinImage::slotTest3()
{
//    qreal thita = _a_number->value();

    // edge detection in steps
    edge_detector ed = edge_detector();
    _image = ed.detectEdgeImage(_image, 0);

    update();

    // old test case
    //_image = testCmyk( _image);
    // testColor( _image, _a_number->value());
    //_image = spherizeWinImage( _image, _a_number->value());
    //_image = topSpherizeImage( _image);
    //_image = bilinearColor9( _image, _a_number->value());
    //_image = bicubicColor9( _image, _a_number->value());
    //_image = fisheyeImage(_image, _a_number->value());
    //_image = inflateImage( _image, _a_number->value(), 2);
    //_image = curvesWinImage( _image);

    //QString message = "Test 2:";
    //statusBar()->showMessage( message);
}

void WinImage::slotTest4()
{
//    qreal thita = _a_number->value();

    // edge detection in steps: NMS
    edge_detector ed = edge_detector();
    _image = ed.nmsImage(_image);

    update();
}

void WinImage::slotTest5()
{
    // edge detection in steps
    edge_detector ed = edge_detector();
    _image = ed.double_thresholding(_image);

    update();
}

void WinImage::slotTest6()
{
    edge_detector ed = edge_detector();
    _image = ed.blob_extract(_image);
//    _image = detectImage( _image);
    update();
}

void WinImage::slotTest7()
{
    // displayWhat line|voting map|original image
    int displayWhat = (int)_a_number->value();
    _image = detectImage( _image, displayWhat);
    update();
}

void WinImage::slotTest8()
{
//    _image = detectImage( _image);
    qreal thita = _a_number->value(); // rotating angle
    Rotater *rotater = new Rotater(_image, thita);
    QImage blackImage = QImage(_image);
    blackImage.fill( 40);
    _image = rotater->RotateImage(_image, blackImage);
    delete rotater;
    update();
}

void WinImage::slotTest0()
{
    qreal thita = _a_number->value(); // rotating angle
    int w = _image.width(), h = _image.height();

    Line_detector *line_detector = new Line_detector(w, h);
    Rotater *rotater = new Rotater(_image, thita);

    vector<Form> lines;
    for (int i=1; i<=4; i++)
    {
        Form line;
        rotater->get_master_line(_image, i, &line);
        lines.push_back(line);
        if (line.vote == 0) continue;                              // TEST
        _image = line_detector->display_lines(_image, lines, 3);   // TEST
    }
    delete rotater;

    line_detector->print(lines, LineFacts, -1);
    delete line_detector;

    update();
}

void WinImage::slotEdgeDetect()
{
    edge_detector ed = edge_detector();

    _image = blackWhiteImage(_image);
    _image = ed.smoothImage(_image);
    _image = ed.detectEdgeImage(_image, 0);
    _image = ed.nmsImage(_image);
    _image = ed.double_thresholding(_image);
    _image = ed.blob_extract(_image);
    _image = detectImage( _image, 7);

    update();

//    float scale = _a_number->value();
//    //_image = zoomWinImage( _image, scale);
//    _image = resizeWinImage( _image, scale);

//    QString message = "Test 3: zoomed " + QString::number( scale) + " times";
//    statusBar()->showMessage( message);
}

void WinImage::slotChangeFormat()
{
    int format = _a_number->value();
    QString Format;

    switch (format)
    {
    case 0: Format = "Invalid";
        break;
    case 1: Format = "Mono";
        break;
    case 2: Format = "MonoLSB";
        break;
    case 3: Format = "Indexed8";
        break;
    case 4: Format = "RGB32";
        break;
    case 5: Format = "ARGB32";
        break;
    case 6: Format = "ARGB32_Premultiplied";
        break;
    case 7: Format = "RGB16";
        break;
    case 8: Format = "ARGB8565_Premultiplied";
        break;
    case 9: Format = "RGB666";
        break;
    case 10: Format = "ARGB6666_Premultiplied";
        break;
    case 11: Format = "RGB555";
        break;
    case 12: Format = "ARGB8555_Premultiplied";
        break;
    case 13: Format = "RGB888";
        break;
    case 14: Format = "RGB444";
        break;
    case 15: Format = "ARGB4444_Premultiplied";
        break;
    }
    _image = formatImage( _image, format);

    update();

    QString message = "Test 3: formatImage " + Format;
    statusBar()->showMessage( message);
}

///////////////////////////////////////////////////////////
//                                                       //
// Color Menu                                            //
//                                                       //
///////////////////////////////////////////////////////////
void WinImage::setupColorMenu()
{
    // Actions
    QAction* bilinear = new QAction( "Bi&linear", this );
    bilinear->setShortcut( Qt::CTRL + Qt::Key_L );
    bilinear->setStatusTip("Bilinear color field within 4 known corners");
    connect( bilinear, SIGNAL( triggered() ), this, SLOT( slotBilinear()));

    QAction* bicubic = new QAction( "Bic&ubic", this );
    bicubic->setShortcut( Qt::CTRL + Qt::Key_U );
    bicubic->setStatusTip("BiCubic color field within 4 known corners");
    connect( bicubic, SIGNAL( triggered() ), this, SLOT( slotBicubic()));

    QAction* blending = new QAction( "&Blending", this );
    blending->setShortcut( Qt::CTRL + Qt::Key_B );
    blending->setStatusTip("Blending base image with top");
    connect( blending, SIGNAL( triggered() ), this, SLOT( slotBlending()));

    QAction* blackwhite = new QAction( "Black&White", this );
    blackwhite->setShortcut( Qt::CTRL + Qt::Key_W );
    blackwhite->setStatusTip("Black & white");
    connect( blackwhite, SIGNAL( triggered() ), this, SLOT( slotBlackWhite()));

    QAction* edgeDetect = new QAction( "&EdgeDetect", this );
    edgeDetect->setShortcut( Qt::CTRL + Qt::Key_E );
    edgeDetect->setStatusTip("Edge detect");
    connect( edgeDetect, SIGNAL( triggered() ), this, SLOT( slotEdgeDetect()));

    QMenu* color = new QMenu( "&Color", menuBar() );
    menuBar()->addMenu( color);
    color->addAction( bilinear);
    color->addAction( bicubic);
    color->addAction( blending);
    color->addAction( blackwhite);
    color->addAction( edgeDetect);
}

void WinImage::slotBilinear()
{
  _image = bilinearColor( _image, _a_number->value());

  update();

  QString message = "Test bilinear";
  statusBar()->showMessage( message/*, 3000*/);
}

void WinImage::slotBicubic()
{
  _image = bicubicColor( _image, _a_number->value());

  update();

  QString message = "Test bicubic";
  statusBar()->showMessage( message/*, 3000*/);
}

void WinImage::slotBlending()
{
    QImage _top;
    QString fileName = QFileDialog::getOpenFileName(this);

    if (!fileName.isEmpty())
        _top = QImage( fileName);

    _image = blendingWinImage( _image, _top, (int)_a_number->value());

    update();

    QString message = "Test blending";
    statusBar()->showMessage( message/*, 3000*/);
}

void WinImage::slotBlackWhite()
{
    _image = blackWhiteImage( _image);
    update();
}

///////////////////////////////////////////////////////////
//                                                       //
// Edit Menu                                             //
//                                                       //
///////////////////////////////////////////////////////////
void WinImage::setupEditMenu()
{
    // Actions
    QAction* rotate = new QAction( "&Rotate", this );
    rotate->setShortcut( Qt::CTRL + Qt::Key_R );
    rotate->setStatusTip("To rotate the image with an angle "
                         "from -10 to 10 degree: ");
    rotate->setWhatsThis("This is a button, stupid!");
    connect( rotate, SIGNAL( triggered()), this, SLOT( slotRotate()));

    QAction* spheric = new QAction( "&Spherize", this );
    spheric->setShortcut( Qt::CTRL + Qt::Key_S );
    spheric->setStatusTip("This will sphereize the image");
    connect( spheric, SIGNAL( triggered()), this, SLOT( slotSpherize()));

    QAction* inflate = new QAction( "&Inflate", this );
    inflate->setShortcut( Qt::CTRL + Qt::Key_I );
    inflate->setStatusTip("To inflate the image, set number from 0 to 0.5");
    connect( inflate, SIGNAL( triggered()), this, SLOT( slotInflate()));

    QAction* distort = new QAction( "&Distort", this );
    distort->setShortcut( Qt::CTRL + Qt::Key_D );
    distort->setStatusTip("To distort the image, please set number "
                          "from -0.51 (Pincushion) to 1.06 (Barrel)");
    connect( distort, SIGNAL( triggered()), this, SLOT( slotDistort()));

    QAction* kaleidoscope = new QAction( "&Kaleidoscope", this );
    kaleidoscope->setShortcut( Qt::CTRL + Qt::Key_K );
    kaleidoscope->setStatusTip("Kaleidoscope");
    connect( kaleidoscope, SIGNAL( triggered()), this, SLOT( slotKaleidoscope()));

    QMenu* edit = new QMenu( "&Edit", menuBar() );
    menuBar()->addMenu( edit);
    edit->addAction( rotate);
    edit->addAction( spheric);
    edit->addAction( inflate);
    edit->addAction( distort);
    edit->addAction( kaleidoscope);
}

void WinImage::slotRotate()
{
    qreal thita = _a_number->value(); // rotating angle

    Rotater *rotater = new Rotater(_image, thita);
    _image = rotater->RotateWithCornerFilling(_image);
    delete rotater;

    // don't use, there are problems to fix
    //slotBlur();

    update();

    QString message = QString::number( thita) + " degrees rotated!";
    statusBar()->showMessage( message);
}

void WinImage::slotKaleidoscope()
{
    _image = kaleidoscopeImage( _image, _a_number->value());

    update();

    QString message = "Kaleidoscope";
    statusBar()->showMessage( message);
}

void WinImage::slotBlur()
{
    qreal thita = _a_number->value();
    _rect = _image.rect(); // CX

    QPoint pTL = _rect.topLeft();
    QPoint pTR = _rect.topRight();
    QPoint pBR = _rect.bottomRight();
    QPoint pBL = _rect.bottomLeft();

    QLine line1 = QLine( pTL, pTR);
    QLine line2 = QLine( pTR, pBR);
    QLine line3 = QLine( pBR, pBL);
    QLine line4 = QLine( pBL, pTL);

    // Transformer: rotate at center of original image with angle thita
    QTransform transform;
    transform.translate( _rect.center().x(), _rect.center().y());
    transform.rotate( thita);
    transform.translate( -_rect.center().x(), -_rect.center().y());

    line1 = transform.map( line1);
    line2 = transform.map( line2);
    line3 = transform.map( line3);
    line4 = transform.map( line4);

    _image = blurLine( _image, line1.x1(), line1.y1(), line1.x2(), line1.y2());
    _image = blurLine( _image, line2.x1(), line2.y1(), line2.x2(), line2.y2());
    _image = blurLine( _image, line3.x1(), line3.y1(), line3.x2(), line3.y2());
    _image = blurLine( _image, line4.x1(), line4.y1(), line4.x2(), line4.y2());

    update();

    QString message( "Blur the square !");
    statusBar()->showMessage( message);
}

void WinImage::slotSpherize()
{
  _image = spherizeImage( _image);

  update();

  QString message = " Spherized!";
  statusBar()->showMessage( message/*, 3000*/);
}

void WinImage::slotInflate()
{
  _image = inflateWinImage( _image, _a_number->value());

  update();

  QString message = "Image in inflate.";
  statusBar()->showMessage( message/*, 3000*/);
}

void WinImage::slotDistort()
{
  _image = distortImage( _image, _a_number->value(), 2);

  update();

  QString message = "Image distortion";
  statusBar()->showMessage( message);
}

///////////////////////////////////////////////////////////
//                                                       //
// File Menu                                             //
//                                                       //
///////////////////////////////////////////////////////////
void WinImage::setupFileMenu()
{
  // Actions
  QAction* open = new QAction( "&Open", this );
  open->setShortcut( Qt::CTRL + Qt::Key_O );
  open->setStatusTip( "This will open a file");
  connect( open, SIGNAL( triggered() ), this, SLOT( slotOpen() ) );

  //QAction* save = new QAction( "&Save", this );
  //connect( save, SIGNAL( triggered() ), this, SLOT( save() ) );

  QAction* saveAs = new QAction( "Save &As", this );
  saveAs->setShortcut( Qt::CTRL + Qt::Key_A );
  connect( saveAs, SIGNAL( triggered() ), this, SLOT( slotSaveAs() ) );

  QAction* quit = new QAction( "&Quit", this );
  quit->setShortcut( Qt::CTRL + Qt::Key_Q );
  quit->setStatusTip("This will quit the application unconditionally");
  quit->setWhatsThis("This is a button, stupid!");
  connect( quit, SIGNAL( triggered() ), qApp, SLOT( quit() ) );

  // Menubarvoid Test::paintEvent( QPaintEvent* )

  QMenu* file = new QMenu( "&File", menuBar());
  menuBar()->addMenu( file);
  file->addAction( open);
  //file->addAction( save);
  file->addAction( saveAs);
  file->addAction( quit);
}

bool WinImage::saveImage(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }
    _image.save( fileName /*, const char * format = 0, int quality = -1 */);

    //setCurrentFile(fileName);

    QString message = "File" + fileName + "saved";
    statusBar()->showMessage(message, 2000);

    return true;
}

void WinImage::loadImage(const QString &fileName)
{
    _imageOrig = QImage( fileName);
    _image = QImage( _imageOrig);
//    _rect = _image.rect();
//    update ( _rect);

    update();

    int H = _image.height(), W = _image.width();

    resize (W + 2 * _offset.x(), H + 2 * _offset.y());

    int index = fileName.length()-fileName.lastIndexOf ("/")-1;
    QString message = fileName.right (index) + ": "
                    + QString::number(W) +"x"+ QString::number(H);

    setWindowTitle (message);
    statusBar()->showMessage ("Image " + fileName + " loaded.");
}

bool WinImage::slotSaveAs()
{
    QString suffix;
    QString fileName = QFileDialog::getSaveFileName(this, "Save image file", _filenameStem, "Bitmap (*.bmp);;Jpeg (*.jpg);;Png (*.png)"/*, &suffix*/);

    fileName.append(suffix.right(5).left(4));
    //qDebug() << fileName;

    if (fileName.isEmpty())
        return false;

    return saveImage(fileName);
}

void WinImage::slotOpen()
{
    //if ( maybeSave()) {
    QString fileName = QFileDialog::getOpenFileName(this, "Open image or text file", ".", "*.bmp *.jpg *.txt");

    _filenameStem = fileName.right(fileName.length() - fileName.lastIndexOf('/') - 1);
    _filenameStem = _filenameStem.left(_filenameStem.lastIndexOf('.'));

    qDebug() << fileName;

    if (fileName.endsWith("txt"))
    {
        _data = readTextFile(fileName.toStdString().data(), &_count);
        if (!_data) return;

        _scale = _a_number->value();
        _image = loadDataImage(_image, _data, _count, _scale);
        //_imageOrig = QImage( _image);
        update();

        int H = _image.height(), W = _image.width();
        resize (W + 2 * _offset.x(), H + 2 * _offset.y());
    }
    else if (!fileName.isEmpty()) loadImage(fileName);
    //}
}

/*
bool WinImage::slotSave()
{
    QMessageBox::information( this, "Save image", "Do you want to save this image?" );
    return true;
}
*/
void WinImage::slotAboutQt()
{
  QMessageBox::aboutQt( this, "About Qt" );
}
