#ifndef CIRCLE_DETECTOR_H
#define CIRCLE_DETECTOR_H

#include "detector.h" // ? or move to caller transformer.cpp

class Circle_detector : public Detector
{
public:
    Circle_detector(int dimA = 7, int dimB = 12, int dimR = 6);
    vector<Form> get_circles();
    void vote(int x, int y);
    void reset();
    QImage display_circles(const QImage image, vector<Form> circles, int mode, int scale = 1);

private:
    void initialize();
    vector<Form> get_local_tops();
    vector<Form> get_all_above_threshold();
    void print(vector<Form> circle, int what);
    bool is_local_high(const Form circle, int range);
    bool is_local_strong(Form circle, int range);
    int get_local_support(Form circle, int range);

    int ***R;
    int m_dimR;
    int m_offset;
};

#endif // CIRCLE_DETECTOR_H
