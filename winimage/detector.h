#ifndef DETECTOR_H
#define DETECTOR_H

#include <QImage>
#include <QDebug>

using namespace std;

struct Form
{
    int vote;
    int group;
    int a;
    int b;
    int r;
    Form(int vote=0, int group=0, int a=0, int b=0, int r=0)
        : vote(vote), group(group), a(a), b(b), r(r) {}
};

enum PrintWhat
{
    Nothing     = 0,
    Message     = 1,
    Neighbor    = 2,
    Report      = 4,
    FormTable   = 8,
    LineFacts   = 16,
    CircleFacts = 32,
    MapV        = 64
};

class Detector
{
public:    
    Detector(int dimA, int dimB);
    //void vote(int x, int y);
    void print(vector<Form> forms, int what, int range = 0, string message = "");
    ~Detector();

protected:
    void reset();
    void initialize();
    bool is_local_high(Form form, int range);
    bool is_local_strong(Form form, int range);
    int get_local_support(Form form, int range);
    int get_local_delta(Form form, int range = 1);
    vector<Form> get_local_tops(int layer = -1);
    vector<Form> get_all_above_threshold(int layer = -1);

    int m_dimA;
    int m_dimB;
    int m_threshold;
    int m_neighbor_range;
    int m_vote_count;
    int **V;

private:

};

#endif // DETECTOR_H
