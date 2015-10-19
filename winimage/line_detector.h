#ifndef LINE_DETECTOR_H
#define LINE_DETECTOR_H

#include "detector.h"

class Line_detector : public Detector
{
public:
//    Line_detector(int dimA = 200, int dimB = 300);
    Line_detector(int image_width, int image_height);
    vector<Form> get_lines();
    void vote(int x, int y);
    void get_line_parameters(Form line, int *x0, int *y0, int *x1, int *y1, qreal *a, qreal *b, qreal *alpha);
    void reset();
    void print(vector<Form> lines, int what, int range = 0, string message = "");
    QImage display_lines(const QImage image, vector<Form> lines, int mode);
    QImage display_voting_map(const QImage image, bool clear);
    void get_line_ends_and_alpha(Form line, int *x0, int *y0, int *x1, int *y1, qreal *alpha);

private:
    void initialize();
    string int2string(int int1, int int2);

    int m_image_height;
    int m_image_width;
    int m_dimB0;
};

#endif // LINE_DETECTOR_H
