#include <QtGui>
#include <math.h>

#include "circle_detector.h"

Circle_detector::Circle_detector(int dimA, int dimB, int dimR) : Detector(dimA, dimB), m_dimR(dimR)
{
    m_offset = 10;
    m_neighbor_range = 8;
    m_threshold = 8;
    initialize();
    reset();
}

void Circle_detector::initialize()
{
    R = new int**[m_dimR];
    for (int i = 0; i<m_dimR; i++)
    {
        R[i] = new int*[m_dimA];
        for (int j=0; j<m_dimA; j++)
            R[i][j] = new int[m_dimB];
//        V = R[i];
//        Detector::initialize();
    }
}

// set voting matrixes to zero
void Circle_detector::reset()
{
    m_vote_count = 0;
    for (int i=0; i<m_dimR; i++)
    {
        V = R[i];
        Detector::reset();
    }
}

void Circle_detector::vote(int x, int y)
{
    for (int i=0; i<m_dimA; i++)
        for (int j=0; j<m_dimB; j++)
        {
            int r = sqrt((x - i) * (x - i) + (y - j) * (y - j));
            if (r<m_dimR) R[r][i][j]++;
        }
    m_vote_count++;
}

vector<Form> Circle_detector::get_circles()
{
    vector<Form> circles = get_local_tops();
    Detector::print(circles, FormTable, -1, "get_circles:");
    return circles;
}

vector<Form> Circle_detector::get_all_above_threshold()
{
    vector<Form> highs;

    m_threshold = 5 + sqrt(m_vote_count);
    qDebug() << "Circle_detector: m_vote_count:" << m_vote_count << "m_threshold:" << m_threshold;

    for(int k=0; k<m_dimR; k++)
    {
        V = R[k];
        //if (k == 10) Detector::print(highs, MapV, k, "get_local_highs"); // CX test
        vector<Form> circles = Detector::get_all_above_threshold(k);
        for (int j=0; j<circles.size(); j++)
            highs.push_back(circles[j]);
    }
    Detector::print(highs, Report, 0, "get_local_highs");

    return highs;
}

vector<Form> Circle_detector::get_local_tops()
{
    vector<Form> tops;
    vector<Form> highs = get_all_above_threshold();

    for (int k=0; k<highs.size(); k++)
    {
        Form circle = highs.at(k);

        if (is_local_high(circle, m_neighbor_range) &&
            is_local_strong(circle, m_neighbor_range))
        {
            tops.push_back(circle);
        }
    }

    return tops;
}

inline int Circle_detector::get_local_support(Form circle, int range)
{
    int support = 0;
    int r = circle.r;

    for (int k=-range; k<=range; k++)
    {
        if (r+k<0 || r+k>=m_dimR) continue;
        V = R[r+k];
        support += Detector::get_local_support(circle, range);
    }
    return support;
}

inline bool Circle_detector::is_local_high(Form circle, int range)
{
    int r = circle.r, a = circle.a, b = circle.b;

    //TODO: to see if we can use this solution if send circle.vote to Detector
    //for (int k=-range; k<=range; k++)
    //{
    //    V = R[r+k]; //circle.r = r+k;
    //    if (!Detector::is_local_high(circle, range))
    //        return false;
    //}

    for (int i=-range; i<=range; i++)
        for (int j=-range; j<=range; j++)
            for (int k=-range; k<=range; k++)
            {
                if (a+i<0 || b+j<0 || r+k<0 || a+i>=m_dimA || b+j>=m_dimB || r+k>=m_dimR) continue;
                if (R[r][a][b] < R[r+k][a+i][b+j])
                    return false;
            }

    return true;
}

inline bool Circle_detector::is_local_strong(Form circle, int range)
{
    Form circleOther;
    int a = circle.a, b = circle.b, r = circle.r, group = circle.group;
    int support = get_local_support(circle, 3);

    for (int i=-range; i<=range; i++)
        for (int j=-range; j<=range; j++)
            for (int k=-range; k<=range; k++)
            {
                if (a+i<0 || b+j<0 || r+k<0 || a+i>=m_dimA || b+j>=m_dimB || r+k>=m_dimR) continue;
                if (R[r][a][b] == R[r+k][a+i][b+j])
                {
                    if (i==0 && j==0 && k==0) continue;
                    circleOther.a = a+i;
                    circleOther.b = b+j;
                    circleOther.r = r+k;
                    circleOther.group = group;
                    if (a+i<0 || b+j<0 || r+k<0 || a+i>=m_dimA || b+j>=m_dimB || r+k>=m_dimR) continue;
                    circleOther.vote = R[r+k][a+i][b+j];

                    int otherSupport = get_local_support(circleOther, 3);
                    if (support < otherSupport) return false;
                    else if (support == otherSupport)
                    {
                        V = R[r];
                        // to be finalized as it differs from what used in line detector
                        if (get_local_delta(circle) < get_local_delta(circleOther))
                            return false;
                    }
                }
            }
    return true;
}

// display mode: 0 = text, 1 = cross, 2 = circle, 3 = all
QImage Circle_detector::display_circles(const QImage image, vector<Form> circles, int mode, int scale)
{
    QImage destImage = QImage( image);
    QRect rect = image.rect();

    int H = rect.height();
    int W = rect.width();

    int a, b, r;

    // resolve circle function
    for (int i=0; i<circles.size(); i++)
    {
        a = circles.at(i).a;
        b = circles.at(i).b;
        r = circles.at(i).r;

        // temporary solution for faked image from 3d gesture
        if (r > 12) continue;

        if (scale != 1)
        {
            a *= scale;
            b *= scale;
            r *= scale;

            a += m_offset;
            b += m_offset;
        }

        QColor qc(255, 0, 0); // color of circles

        if (mode == 1 || mode == 3) // circle center as hair cross
        {
            int* destData = (int *)destImage.bits();
            if (a+2<=W) destData[a+2 + b*W] = qc.rgba();
            if (a-2>=0) destData[a-2 + b*W] = qc.rgba();
            if (b+2<=H) destData[a + (b+2)*W] = qc.rgba();
            if (b-2>=0) destData[a + (b-2)*W] = qc.rgba();
        }

        if (mode == 2 || mode == 3) // display circle
        {
            QPainter painter( &destImage);
            painter.setPen(qc);
            painter.drawEllipse(a-r, b-r, r*2, r*2);
        }
    }

    return destImage;
}

void Circle_detector::print(vector<Form> forms, int what)
{
    // print lines facts
    if (what & MapV)
    {
        vector<int> printed;

        for (int i=0; i<forms.size(); i++)
        {
            bool toPrint(true);
            int layer = forms.at(i).r;

            for (int j=0; j<printed.size(); j++)
                if (layer == printed[j]) toPrint = false;

            if (toPrint)
            {
                printed.push_back(layer);
                V = R[layer];
                Detector::print(forms, MapV, layer);
            }
        }
    }
}
