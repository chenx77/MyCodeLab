#include "edge_detector.h"
#include "rotater.h"

#include <QtGui>
#include <QPainter>

#ifndef isinf
#define isinf !_finite
#endif
///////////////////////////////////////////////////////////
//                                                       //
// Rotate with corner fillet                             //
//                                                       //
///////////////////////////////////////////////////////////
/*!
    Constructs a Rotater with the given \a image and \a theta.
*/
Rotater::Rotater( const QImage image, const qreal theta)
{
    m_width = image.width(), m_height = image.height();
    m_line_detector = new Line_detector(m_width, m_height);

    m_theta = theta;
    m_sign = m_theta >= 0 ? 1 : -1;
    m_scale = cos(m_theta * m_sign * M_PI/180);
    m_transform.translate( m_width * 0.5, m_height * 0.5);
    m_transform.rotate( m_theta); // m_theta is in degrees
    m_transform.translate( -m_width * 0.5, -m_height * 0.5);
    for (int i=0; i<=4; i++) m_dx[i] = 0, m_dy[i] = 0;

    qDebug() << "### m_width" << m_width << "m_height" << m_height << "m_theta" << m_theta;
}

Rotater::~Rotater()
{
    delete m_line_detector;
}

// \fn QImage Rotater::rotateImage()

/*!
    \sa rotateWithCornerFilling(const QImage image)
*/
QImage Rotater::RotateImage( const QImage image, const QImage imageOrig)
{
    QImage outImage = QImage( image);
//    outImage.fill( 0); /////////////////////////////////
    QRect rect = image.rect();
    QPainter painter( &outImage);
    painter.setRenderHint( QPainter::SmoothPixmapTransform);

    // Paint original as background
    ///// painter.drawImage ( rect, image);
    // a. no need to do this as outImage is a already copied
    // b. can't agree: without above line, the rotated image is bluring
    // a. it is bluring regardless having that line or not

    // Clip path
    QPainterPath path( QPointF( 0, 0));
    path.addRect( rect);
    painter.setClipPath( path);

    // Transformer: rotate at center of original image with angle m_theta
    // use m_transform instead
    painter.setTransform( m_transform /*transform*/);

    // Overlap it to original
    painter.drawImage( rect, imageOrig);

    return outImage;
}

QImage Rotater::shiftImage( const QImage image)
{
    QImage outImage = QImage( image);

    QPainter painter( &outImage);
    painter.setRenderHint( QPainter::SmoothPixmapTransform);

    // using new method to calculate shift
    int move1 = m_sign < 0 ? 1 : 0, move2 = move1 ? 0 : 1;

    // Transformer: pan each half of original image outwards
    QRect rect_half(0, 0, m_width/2, m_height/2);
    QTransform transform;

    // quater-1: top-left
    transform.scale( move1 > 0 ? m_scale : 1, move2 > 0 ? m_scale : 1);
    transform.translate( -m_dx[1], -m_dy[1]); // (-,-)
    painter.setTransform ( transform);
    // as shift may deamage previous quater, configure the rect carefully
    // so pan only 3/4 of half image (1 + move2/2.0)
    rect_half.setRect( 0, 0, m_width/2 * (1 + move1), m_height/2 * (1 + move2/2.0));
    painter.drawImage( rect_half, image, rect_half);
//    qDebug() << "##### 1:" << rect_half;

    // quater-2: top-right
    transform.reset();
    transform.scale( move2 > 0 ? m_scale : 1, move1 > 0 ? m_scale : 1);
    transform.translate( (1-m_scale) * m_width * move2, 0);
    transform.translate(m_dx[2], -m_dy[2]); // (+,-)
    painter.setTransform ( transform);
    // as shift may deamage previous quater, configure the rect carefully
    // so pan only 3/4 of half image (move1 + move2/2.0)
    rect_half.setRect( m_width/2 * (move1 + move2/2.0), 0, m_width/2 * (1 + move2), m_height/2 * (1 + move1));
    painter.drawImage( rect_half, image, rect_half);
//    qDebug() << "##### 2:" << rect_half;

    // quater-3: bottom-right
    transform.reset();
    transform.scale( move1 > 0 ? m_scale : 1, move2 > 0 ? m_scale : 1);
    transform.translate( (1-m_scale) * m_width * move1, (1-m_scale) * m_height * move2);
    transform.translate( m_dx[3], m_dy[3]); // (+,+)
    painter.setTransform ( transform);
    // as shift may deamage previous quater, configure the rect carefully
    // so pan only 3/4 of half image (move1 + move2/2.0)
    rect_half.setRect( m_width/2 * move2, m_height/2 * (move1 + move2/2.0), m_width/2 * (1 + move1), m_height/2 * (1 + move2));
    painter.drawImage( rect_half, image, rect_half);
//    qDebug() << "##### 3:" << rect_half;

    // quater-4: bottom-left
    transform.reset();
    // the order of transform makes sense. if sequence was 2+(3.1+3,2),
    // then the 3-combined transform steps should be (3.1+3.2)+2
    transform.scale( move2 > 0 ? m_scale : 1, move1 > 0 ? m_scale : 1); // 3.1 scale
    transform.translate( 0, (1-m_scale) * m_height * move1);              // 3.2 compensate to scale
    transform.translate( -m_dx[4], m_dy[4]); // (-,+)                   // 2   original shift
    painter.setTransform ( transform);
    // as shift may deamage the 3rd quater, configure the rect carefully
    // so pan only 3/4 of a half (move2 + move2/2.0), (1 + move2/2.0)
    rect_half.setRect( 0, m_height/2 * (move2 + move1/2.0), m_width/2 * (1 + move2/2.0), m_height/2 * (1 + move1));
    painter.drawImage( rect_half, image, rect_half);
//    qDebug() << "##### 4:" << rect_half;

    return outImage;
}

/// algrithm:
///
/// 1. calculate cutting position (x0, y0) or (x1, y1)
/// position is the cutting position of the line and image edges before
/// rotation. but not simply points of ends (x0, 0) or (x1, y1) of the
/// line. it depends on where the edge is directed, and we are dealing
/// with. so position can be x0, y0, x1, y1 against the reference corners
/// of the quater like (0,0), (W,0), (W,H) or (0,H)
///
/// 2. calculate angle alpha
/// alpha becoming the angle of the line against the edge of image frame
/// it cuts through at position, so it's needed to re-calculate the alpha
/// according to where the edge is located; we also keep the angle in
/// range of [0, 180] for convinience
///
/// 3. calculate shift distance
/// shift outward to each corner as (dx, 0) or (0, dy)
///
void Rotater::get_shift_values(int quater, Form line)
{
    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    qreal a = 0, b = 0, alpha = 0 /*degree*/;

    m_line_detector->get_line_parameters(line, &x0, &y0, &x1, &y1, &a, &b, &alpha);

    qreal alpha_rad = alpha * M_PI / 180;

    qreal xr = 0, yr = 0; // xr is rotated x0/x1, yr is rotated y0/y1;
    qreal xx = 0, yy = 0; // xx is x0 or x1, yy is y0 or y1, depends which to use
    qreal dx = 0, dy = 0; // shift distance, one of them is always 0

    bool shift_x;       //true => dx, false => dy

    if (m_sign == 1) switch(quater) // clockwise (screen view)
    {
    case 1:
        if (x0 == 0 && y0 != 0) // use end (x0, y0)
        {
            alpha = 90 - alpha;
            xx = x0, yy = y0;
        }
        else if (x1 == 0) // use the other end (x1, y1)
        {
            alpha = 90 - alpha; // ???
            xx = x1, yy = y1;
        }
        else // no ideal line found, use pseudo point instead
        {
            alpha = 90;
            xx = 0, yy = m_height / 8;
        }
        shift_x = false;
        break;
    case 2:
        if (y0 == 0)
        {
            alpha = 180 - alpha;
            xx = x0, yy = y0;
        }
        else if (y1 == 0)
        {
            alpha = -alpha;
            xx = x1, yy = y1;
        }
        else
        {
            alpha = 90;
            xx = m_width * 7 / 8, yy = 0;
        }
        shift_x = true;
        break;
    case 3:
        if (x1 == m_width)
        {
            alpha = 90 - alpha;
            xx = x1, yy = y1;
        }
        else //if no idea line
        {
            alpha = 90;
            xx = m_width, yy = m_height * 7 / 8;
        }
        shift_x = false;
        break;
    case 4:
        if (y1 == m_height)
        {
            alpha = 180 - alpha;
            xx = x1, yy = y1;
        }
        else if (y0 == m_height)
        {
            alpha = -alpha;
            xx = x0, yy = y0;
        }
        else //if no idea line
        {
            alpha = 90;
            xx = m_width / 8, yy = m_height;
        }
        shift_x = true;
    }
    else switch(quater) // count clockwise (screen view)
    {
    case 1:        
        if (y0 == 0 && x0 != 0)
        {
            xx = x0, yy = y0;
        }
        else if (y1 == 0 && x1 != 0)
        {
            alpha = 180 + alpha;
            xx = x1, yy = y1;
        }
        else // no ideal line
        {
            alpha = 90;
            xx = m_width / 8, yy = 0;
        }
        shift_x = true;
        break;
    case 2:
        if (x1 == m_width)
        {
            alpha = 90 + alpha;
            xx = x1, yy = y1;
        }
        else if (x0 == m_width)
        {
            alpha = 90 + alpha;
            xx = x0, yy = y0;
        }
        else //if no idea line
        {
            alpha = 90;
            xx = m_width, yy = m_height / 8;
        }
        shift_x = false;
        break;
    case 3:
        if (y1 == m_height) // use this end (x1, y1)
        {
            xx = x1, yy = y1;
        }
        else if (x1 == m_width) // use the other end (x0, y0)
        {
            alpha = 180 + alpha;
            xx = x0, yy = y0;
        }
        else //if no idea line
        {
            alpha = 90;
            xx = m_width * 7 / 8, yy = m_height;
        }
        shift_x = true;
        break;
    case 4:
        if (x0 == 0)
        {
            alpha = 90 + alpha;
            xx = x0, yy = y0;
        }
        else if (x1 == 0)
        {
            alpha = 90 + alpha;
            xx = x1, yy = y1;
        }
        else //if no idea line
        {
            alpha = 90;
            xx = 0, yy = m_height * 7 / 8;
        }
        shift_x = false;
    }

    alpha_rad = alpha * M_PI / 180;

    m_transform.map(xx, yy, &xr, &yr);
    if (shift_x) dx = fabs(xr - xx) + fabs(yr - yy)/tan(alpha_rad);
    else dy = fabs(yr - yy) + fabs(xr - xx)/tan(alpha_rad);

    // try to do things that get_shift_value was doing
    if (alpha_rad == 0 || isinf(tan(alpha_rad)))
    {
        m_dx[quater] = m_dy[quater] = 0;
        qDebug() << "*** m_dx[quater] = m_dy[quater] = 0;";
    }
    else
    {
        m_dx[quater] = dx;
        m_dy[quater] = dy;
    }
    qDebug().nospace() << "*** quater " << quater << ", (m_dx, m_dy) = (" << m_dx[quater] << ", " << m_dy[quater] << ")\n"
                       << "(alpha, tan(alpha_rad)) = (" << alpha << ", " << tan(alpha_rad) << ")";
}

/// get strongest line in a quater, if there is no line
/// at all, an empty line (vote == 0) will be given
///
void Rotater::get_master_line(const QImage image, int quater, Form *line)
{
    QImage destImage = QImage( image);
    int h = m_height, w = m_width;

    // deal with 4 quaters of image accordingly
    int x_from, x_to, y_from, y_to;
    switch(quater)
    {
    case 1: x_from = 0, x_to = m_width/2, y_from = 0, y_to = m_height/2;
        break;
    case 2: x_from = m_width/2, x_to = w, y_from = 0, y_to = m_height/2;
        break;
    case 3: x_from = m_width/2, x_to = w, y_from = m_height/2, y_to = h;
        break;
    case 4: x_from = 0, x_to = m_width/2, y_from = m_height/2, y_to = h;
        break;
    }

    // voting lines in area of the quater
    QColor qc;
    m_line_detector->reset();
    for (int i=y_from; i<y_to; i++)
    {
        for (int j=x_from; j<x_to; j++)
        {
            qc = (QColor)destImage.pixel(j, i);
            if (qc.lightness() > 200)
                m_line_detector->vote(j, i);
        }
    }
    vector<Form> lines = m_line_detector->get_lines();
    int prefered_group = m_sign * ((quater == 1 || quater == 3) ? 1 : -1);

    // get the line that has hightest voting in the quater
    qreal alpha, min_alpha = 10;
    bool alphaOk = false, lineEndOk = false;
    int x0, y0, x1, y1;
    for (int i=0; i<(int)lines.size(); i++)
    {
        if (lines.at(i).vote <= line->vote) continue;

        m_line_detector->get_line_ends_and_alpha(lines.at(i), &x0, &y0, &x1, &y1, &alpha);
        alphaOk = (prefered_group == 1 && fabs(alpha) < 90 - min_alpha) ||
                  (prefered_group == -1 && fabs(alpha) > min_alpha && fabs(alpha) < 180 - min_alpha);
        lineEndOk = (quater == 4 && m_sign > 0 && (y0 == m_height || y1 == m_height)) ||
                    (quater == 4 && m_sign < 0 && (x0 == 0 || x1 == 0)) ||
                    (quater == 3 && m_sign > 0 && (x0 == m_width || x1 == m_width)) ||
                    (quater == 3 && m_sign < 0 && (y0 == m_height || y1 == m_height)) ||
                    (quater == 2 && m_sign > 0 && (y0 == 0 || y1 == 0)) ||
                    (quater == 2 && m_sign < 0 && (x0 == m_width || x1 == m_width)) ||
                    (quater == 1 && m_sign > 0 && (x0 == 0 || x1 == 0)) ||
                    (quater == 1 && m_sign < 0 && (y0 == 0 || y1 == 0));

        if (alphaOk && lineEndOk) *line = lines.at(i);
    }
}

/// get 4 lines from 4 quater correspondingly, a line
/// can be empry if there is no line in that quater
///
QImage Rotater::readLines( const QImage image, vector<Form> *lines)
{
    QImage destImage = QImage( image);
    QImage blackImage = QImage( image);
    blackImage.fill( 0);

    // edges detection
    edge_detector ed = edge_detector();
//    destImage = ed.smoothImage(destImage);
    destImage = ed.detectEdgeImage(destImage, 0);
    destImage = ed.nmsImage(destImage);
    destImage = ed.double_thresholding(destImage);
    destImage = ed.blob_extract(destImage);
    destImage = RotateImage(destImage, blackImage);
    destImage = HideRollOutCorners(destImage);

    // collect master line (1 or 0) from each quater
    for (int i=1; i<=4; i++)
    {
        Form line;
        get_master_line(destImage, i, &line);
        lines->push_back(line);

        // commented off to avoid interfering the result
//        if (line.vote == 0) continue;                                       // TEST
//        destImage = m_line_detector->display_voting_map(destImage, false);  // TEST
//        destImage = m_line_detector->display_lines(destImage, *lines, 3);   // TEST
    }
    m_line_detector->print(*lines, LineFacts, -1);

    return destImage;
}

QImage Rotater::RotateWithCornerFilling(const QImage image)
{
    QImage destImage = QImage( image);
    QImage origImage = QImage( image);

    vector<Form> lines;

    readLines(destImage, &lines);
//    m_line_detector->print(lines, LineFacts, -1);

    for (int quater=1; quater<=4; quater++)
    {
        get_shift_values(quater, lines.at(quater-1));
//        if (quater != 1) continue;
//        destImage = markOnImage( destImage, m_dx[0], m_dy[0], 1); // TEST
//        destImage = markOnImage( destImage, m_dx[5], m_dy[5], 0); // TEST
    }
    destImage = shiftImage( destImage);

    destImage = RotateImage( destImage, origImage);

    return destImage;
}

QPoint Rotater::get_line_cross_point(QLine base_line, QLine rotate_line)
{
    qreal k = (qreal)rotate_line.dy() / rotate_line.dx();
    int x(0), y(0);

    if (base_line.dy() == 0)
    {
        x = (base_line.y1() - rotate_line.y1()) / k + rotate_line.x1();
        y = base_line.y1();
    }
    else if (base_line.dx() == 0)
    {
        y = (base_line.x1() - rotate_line.x1()) * k + rotate_line.y1();
        x = base_line.x1();
    }

//    qDebug() << "k dx dy" << k << rotate_line.dx() << rotate_line.dy();
//    qDebug() << "rotate_line x1 y1" << rotate_line.x1() << rotate_line.y1();
//    qDebug() << "rotate_line x2 y2" << rotate_line.x2() << rotate_line.y2();

    QPoint p(x, y);
    return p;
}

QImage Rotater::markOnImage(const QImage image, int x, int y, MarkType type)
{
    QImage outImage = QImage( image);
    QPainter painter( &outImage);

    const QPoint *p = new QPoint(x, y);
    switch(type)
    {
    case Ellip:
        painter.drawEllipse(*p, 5, 5);
        break;
    case CrossLines:
        painter.drawLine(0, y, m_width, y);
        painter.drawLine(x, 0, x, m_height);
        break;
    case Rect:
        painter.drawRect(x - 5, y - 5, 10, 10);
    }
    return outImage;
}

QImage Rotater::HideRollOutCorners(const QImage image)
{
    QImage destImage = QImage( image);

    QLine hline, vlineR, vlineL;
    int x0, y0, x1, y1;
    if (m_sign < 0)
    {
        x0 = 0, y0 = 0, x1 = m_width, y1 = 0;
    }
    else
    {
        x0 = 0, y0 = m_height, x1 = m_width, y1 = m_height;
    }

    hline = QLine(x0, y0, x1, y1);
    vlineR = QLine(m_width, 0, m_width, m_height);
    vlineL = QLine(0, 0, 0, m_height);

    QLine rvline = m_transform.map(vlineR);
    QPoint pa = get_line_cross_point(hline, rvline);
    QPoint pb = get_line_cross_point(vlineR, rvline);

    QLine rhline = m_transform.map(hline);
    QPoint pc = get_line_cross_point(vlineL, rhline);
    QPoint pd = get_line_cross_point(hline, rhline);

//    destImage = markOnImage( image, pa.x(), pa.y(), MarkType::CrossLines);
//    destImage = markOnImage( destImage, pb.x(), pb.y(), MarkType::CrossLines);
//    destImage = markOnImage( destImage, pc.x(), pc.y(), MarkType::CrossLines);
//    destImage = markOnImage( destImage, pd.x(), pd.y(), MarkType::CrossLines);

    QPolygonF triagle1, triagle2;
    triagle1 << pa << pb << QPointF(x1, y1);
    triagle2 << pc << pd << QPointF(x0, y0);

    QPainterPath myPath;
    myPath.addPolygon(triagle1);
    myPath.addPolygon(triagle2);

    QTransform transform;
    transform.translate(0, m_height * 0.5);
    transform.rotate(180, Qt::XAxis);
    transform.translate(0, -m_height * 0.5);
    QPainterPath myPath1 = transform.map(myPath);

    transform.reset();
    transform.translate(m_width * 0.5, 0);
    transform.rotate(180, Qt::YAxis);
    transform.translate(-m_width * 0.5, 0);
    myPath = transform.map(myPath);
    myPath.addPath(myPath1);

    QPainter painter(&destImage);
    painter.setBrush(Qt::SolidPattern);
    painter.drawPath(myPath);

    return destImage;
}
