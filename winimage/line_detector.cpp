#include <QtGui>
#include <sstream> // for int to string

#include "line_detector.h"

Line_detector::Line_detector(int image_width, int image_height) :
    Detector(200, max(image_width, image_height) * 3)//2*min(image_width, image_height)+max(image_width, image_height))//
{
    m_neighbor_range = 12; // 8;
    m_threshold = 16;
    m_image_width = image_width;
    m_image_height = image_height;
    m_dimB0 = max(image_width, image_height);//min(image_width, image_height);//
}

void Line_detector::reset()
{
    Detector::reset();
}

void Line_detector::vote(int x, int y)
{
    // y = ax + b, a = [0, -1] => i = [0, -50] => V[p:0-50][b], a(p) = 0.02*(-p)
    // b = [-m_dimB0, 2*m_dimB0] => V[a][p:0-3*m_dimB0] => p(b) = b+m_dimB0 => b(p) = p-m_dimB0
    for (int i=0; i<=50; i++)
    {
        int b = y + (i * 0.02) * x;
        if (m_dimB0+b >= 0 && m_dimB0+b < m_dimB)
            V[i][m_dimB0+b]++;
    }

    // x = ay + b, a = (-1, 1) => i = (-50, 50) => V(p:50-150)[b], a(p) = 0.02*(p-100)
    //             b = [-m_dimB0, m_dimB0] => V[a][p:0-2m_dimB0] => p = b+m_dimB0 => b(p) = p-m_dimB0
    for (int i=-49; i<50; i++)
    {
        int b = x - (i * 0.02) * y;
        if (m_dimB0+b >= 0 && m_dimB0+b < m_dimB)
            V[i+100][m_dimB0+b]++;
    }

    // y = ax + b, a = [1, 0) => i = [50, 0) => V[150-200)[b], a(p) = 0.02*(200-p)
    //             b = [m_dimB0, -m_dimB0] => V[a][p:0-2m_dimB0] => p = m_dimB0-b => b(p) = m_dimB0-p
    for (int i=50; i>0; i--)
    {
        int b = y - (i * 0.02) * x;
        if (m_dimB0-b >= 0 && m_dimB0-b < m_dimB)
            V[m_dimA-i][m_dimB0-b]++;
    }
    m_vote_count++;
}

vector<Form> Line_detector::get_lines()
{
    // TODO: optimize, so far 3, 0.75 is the best
    m_threshold = 3 + sqrt(m_vote_count);// * 0.75;

    qDebug() << "m_threshold = 3 + sqrt(m_vote_count):" << m_vote_count << "m_threshold:" << m_threshold;

    vector<Form> lines = get_local_tops();

    // to print LineFacts: range -1 all,
    // or 0+, only line with that index
    //string HW = int2string(m_image_width, m_image_height);
    //print(lines, LineFacts | FormTable | Report, -1, HW);
    //Detector::print(lines, FormTable | Report, -1, "Lines found:");

    return lines;
}

void Line_detector::get_line_ends_and_alpha(Form line, int *x0, int *y0, int *x1, int *y1, qreal *alpha)
{
    qreal a(0), b(0);
    get_line_parameters(line, x0, y0, x1, y1, &a, &b, alpha);
}

/// get key parameters of a line:
/// a, slope rate; b, axis cut-off value;
/// (x0, y0), line's near end (close to original point O)
/// (x1, y1), line's far end (normally reachs the width/height of image)
/// alpha (0-180 degree), angle between x axis and the line
///
void Line_detector::get_line_parameters(Form line, int *x0, int *y0, int *x1, int *y1, qreal *a, qreal *b, qreal *alpha)
{
    *x0 = *y0 = *x1 = *y1 = *alpha = *a = *b = 0;

    if (line.group == 1) // y = ax + b, red
    {
        // calculate a, b
        if (line.a <= 50)
        {
            *a = - 0.02 * line.a;
            *b = line.b - m_dimB0;
        }
        else
        {
            *a = 0.02 * (200 - line.a);
            *b = m_dimB0 - line.b;
        }

        // calculate x0, y0
        if (*b >=0 && *b <= m_image_height)
        {
            *x0 = 0;
            *y0 = *b;
        }
        else
        {
            *x0 = *a == 0 ? 0 : -(*b) / *a;
            *y0 = 0;
        }
        if (*x0 < 0 || *x0 > m_image_width)
        {
            *y0 = m_image_height;
            *x0 = (*a == 0) ? 0 : (*y0 - *b) / *a;
        }

        // calculate x1, y1
        *x1 = m_image_width;
        *y1 = *a * *x1 + *b;
        if (*y1 < 0 || *y1 > m_image_height)
        {
            *y1 = m_image_height;
            *x1 = (*a == 0) ? 0 : (*y1 - *b) / *a;
        }
        if (*x1 < 0 || *x1 > m_image_width)
        {
            /*if (x0 == 0)*/ *y1 = 0; //TODO: what if (y0 == 0)
            *x1 = (*a == 0) ? 0 : (- *b) / *a;
        }

        // calculate alpha
        *alpha = atan(*a) * 180 / M_PI;
    }
    else if (line.group == -1) // x = ay + b, green
    {
        // calculate a, b
        *a = 0.02 * (line.a - 100);
        *b = line.b - m_dimB0;

        // calculate x0, y0
        if (*b >=0 && *b <= m_image_width)
        {
            *y0 = 0;
            *x0 = *b;
        }
        else
        {
            *y0 = *a == 0 ? 0 : -(*b) / *a;
            *x0 = 0;
        }
        if (*y0 < 0 || *y0 > m_image_height)
        {
            *x0 = m_image_width;
            *y0 = (*a == 0) ? 0 : (*x0 - *b) / *a;
        }

        // calculate x1, y1
        *y1 = m_image_height;
        *x1 = *a * *y1 + *b;
        if (*x1 < 0 || *x1 > m_image_width)
        {
            *x1 = 0;
            *y1 = (*a == 0) ? 0 : (*x1 - *b) / *a;
        }
        if (*y1 < 0 || *y1 > m_image_height)
        {
            *x1 = m_image_width;
            *y1 = (*a == 0) ? 0 : (*x1 - *b) / *a;
        }

        // calculate alpha
        *alpha = atan(1 / *a) * 180 / M_PI;
    }

//    if (*a < 0) *alpha += 90; //180;

    return;
}

// display mode: 0 = text, 1 = cross, 2 = lines, 3 = all
QImage Line_detector::display_lines(const QImage image, vector<Form> lines, int mode)
{
    QImage destImage = QImage( image);

    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    qreal a = 0, b = 0, alpha = 0; // line's angle against axele X

    // to print LineFacts: range -1 all,
    // or 0+, only line with that index
    //string HW = int2string(H, W);
    //print(lines, LineFacts, -1, HW);

    // resolve line function
    for (int i=0; i<(int)lines.size(); i++)
    {
        int pa = lines.at(i).a;
        int pb = lines.at(i).b;
        int group = lines.at(i).group;

        QColor qc(0, 0, 255); // color of lines

        if (group == 1) qc.setRgb(255, 0, 0);
        else if (group == -1) qc.setRgb(0, 255, 0);

        if (pb >= 2*m_image_width) qc.setBlue(255);

        // y = ax + b, a = [0, -1] => i = [0, -50] => V[p:0-50][b], a(p) = 0.02*(-p), b(p) = p-100
        // x = ay + b, a = (-1, 1) => i = (-50, 50) => V(p:50-150)[b], a(p) = 0.02*(p-100), b(p) = p-100
        // y = ax + b, a = [1, 0] => i = [50, 0] => V[150-200][b], a(p) = 0.02*(200-p), b(p) = 100-p
        if (mode == 2 || mode == 3) // display recoganized line
        {
            QPainter painter( &destImage);
            painter.setPen(qc);
            get_line_parameters(lines.at(i), &x0, &y0, &x1, &y1, &a, &b, &alpha);
            painter.drawLine(x0, y0, x1, y1);
        }

        if (mode == 1 || mode == 3) // hair cross on voting map
        {
            int* destData = (int *)destImage.bits();
            int sizeOfImage = m_image_width * m_image_height;

            int pixel = m_image_width * pa + (pb+2);
            if (pixel < sizeOfImage && pixel > -1)
                destData[pixel] = qc.rgba();
            else qDebug() << 1 << "### Hair Cross OUT OF RANGE" << pixel << "pa" << pa << "pb" << pb;

            pixel = m_image_width * pa + (pb-2);
            if (pixel < sizeOfImage && pixel > -1)
                destData[pixel] = qc.rgba();
            else qDebug() << 2 << "### Hair Cross OUT OF RANGE" << pixel << "pa" << pa << "pb" << pb;

            pixel = m_image_width * (pa+2) + pb;
            if (pixel < sizeOfImage && pixel > -1)
                destData[pixel] = qc.rgba();
            else qDebug() << 3 << "### Hair Cross OUT OF RANGE" << pixel << "pa" << pa << "pb" << pb;

            pixel = m_image_width * (pa-2) + pb;
            if (pixel < sizeOfImage && pixel > -1)
                destData[pixel] = qc.rgba();
            else qDebug() << 4 << "### Hair Cross OUT OF RANGE" << pixel << "pa" << pa << "pb" << pb;
        }
    }

    return destImage;
}

QImage Line_detector::display_voting_map(const QImage image, bool clear)
{
    QImage destImage = QImage( image);

    if (clear) destImage.fill( 0);
    int* destData = (int *)destImage.bits();

    // TODO: to auto fit different original image size
    for (int i=0; i<m_dimA; i++)
        for (int j=0; j<m_dimB; j++)
        {
            int v = V[i][j];
            int shift = (i>50 && i<150) ? 8 : 16;
            destData[m_image_width * i + j] |= (v*4) << shift;
            destData[m_image_width * i + j] |= (j>=2*m_image_width) ? (v*4) : 0; // folder space with blue
        }

    return destImage;
}

void Line_detector::print(vector<Form> lines, int what, int range, string message)
{
    // print lines facts
    if (what & LineFacts)
    {
        int x0, y0, x1, y1;
        qreal a, b, alpha; // line's angle against axele X

        int from = 0, until = lines.size();
        if (range != -1) from = range, until = from + 1;

        QString numberOfLines = QString::number(until) + " lines";
        qDebug().nospace() << "\nvo formular " << numberOfLines << "\tx0" << "\ty0" << "\tx1" << "\ty1" << "\talpha" ;
        qDebug() << "==============================================================================";

        for (int i=from; i<until; i++)
        {
            Form line = lines.at(i);
            get_line_parameters(line, &x0, &y0, &x1, &y1, &a, &b, &alpha);

            string left = lines.at(i).group==1 ? " y = " : " x = ";
            string right = lines.at(i).group==1 ? "x + " : "y + ";

//            if (line.vote < 5) continue;
            qDebug().nospace() << line.vote << left.data() << a << right.data() << b << "     \t"
                     << x0 << "\t" << y0 << "\t" << x1 << "\t" << y1 << "\t" << alpha;
        }
        qDebug() << "==============================================================================";
    }
    Detector::print(lines, what, range, message);
}

string Line_detector::int2string(int int1, int int2)
{
    stringstream int2string;
    int2string << int1 << "," << int2;
    string result = int2string.str();

    return result;
}
