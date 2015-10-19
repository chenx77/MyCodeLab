#ifndef WINIMAGE_H
#define WINIMAGE_H

#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QCheckBox>

class QAction;

class WinImage :public QMainWindow
{
    Q_OBJECT

public:
    explicit WinImage( QWidget* parent=0 );
    ~WinImage();

private:
    virtual void paintEvent( QPaintEvent* );
    void setupFileMenu();
    void setupEditMenu();
    void setupColorMenu();
    void setupToolsMenu();

    void loadImage(const QString &fileName);
    bool saveImage(const QString &fileName);
    QImage _image;
    QImage _imageOrig;
    QPoint _offset;
    QRect _rect;
    QDoubleSpinBox *_a_number;
    QCheckBox *_check_box;

    // for rotation compensation
    float _alpha;
    float _scale;
    int _x0;

    // for detection
    float** _data;
    int _count;
    QString _filenameStem;

private slots:
    // File menu
    void slotAboutQt();
    void slotOpen();
    bool slotSaveAs();
    //bool slotSave();

    // Edit menu
    void slotBlur();
    void slotDistort();
    void slotInflate();
    void slotKaleidoscope();
    void slotRotate();
    void slotSpherize();

    // Color menu
    void slotBicubic();
    void slotBilinear();
    void slotBlending();
    void slotBlackWhite();
    void slotEdgeDetect();

    // Tools menu
    void slotBigO();
    void slotChangeFormat();
    void slotDiffer();
    void slotTest0();
    void slotTest1();
    void slotTest2();
    void slotTest3();
    void slotTest4();
    void slotTest5();
    void slotTest6();
    void slotTest7();
    void slotTest8();
    void slotTest9();
    void slotZoom();
};
#endif /* WINIMAGE_H */
