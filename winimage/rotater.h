#ifndef ROTATER_H
#define ROTATER_H

#include "line_detector.h"

class Rotater
{
public:
    Rotater( const QImage image, const qreal theta);
    QImage RotateWithCornerFilling(const QImage image);
    QImage RotateImage( const QImage image, const QImage imageOrig);
    QImage HideRollOutCorners(const QImage image);
    void get_master_line(const QImage image, int quater, Form *line);
    enum MarkType { CrossLines, Ellip, Rect };
    ~Rotater();

private:    
    QImage markOnImage(const QImage image, int x, int y, MarkType type);
    QImage shiftImage( const QImage image);
    QImage readLines( const QImage image, vector<Form> *lines);
    QPoint get_line_cross_point(QLine base_line, QLine rotate_line);
    void get_shift_values(int quater, Form line);

    Line_detector *m_line_detector;
    QTransform m_transform;
    qreal m_width, m_height;
    qreal m_dx[5], m_dy[5];
    qreal m_scale;
    qreal m_theta; // angle to rotate
    int m_sign;
};

#endif // ROTATER_H
