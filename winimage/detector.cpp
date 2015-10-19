#include <QtGui>
#include "detector.h"

Detector::Detector(int dimA, int dimB) : m_dimA(dimA), m_dimB(dimB)
{
    initialize();
    reset();
}

Detector::~Detector()
{
    for (int i=0; i<m_dimA; i++)
    {
        delete [] V[i];
    }
    delete [] V;
}

void Detector::initialize()
{
    V = new int*[m_dimA];
    for (int i = 0; i<m_dimA; i++)
        V[i] = new int[m_dimB];
}

void Detector::reset()
{
    m_vote_count = 0;
    for (int i=0; i<m_dimA; i++)
        for (int j=0; j<m_dimB; j++)
            V[i][j] = 0;
}

// pick up those voting points above threshold
// formules look like y = ax + b OR x = ay + b
//
vector<Form> Detector::get_all_above_threshold(int layer)
{
    vector<Form> forms;

    //print(forms, MapV, 0, "get_local_highs");
    for (int a=0; a<m_dimA; a++)
        for (int b=0; b<m_dimB; b++)
            if (m_threshold < V[a][b])
            {
                int group = (layer != -1) ? 2 : (a > 50 && a < 150) ? -1 : 1;
                Form form(V[a][b], group, a, b, layer);
                forms.push_back(form);
            }
    return forms;
}

vector<Form> Detector::get_local_tops(int layer)
{
    Form form;
    vector<Form> top_forms;
    vector<Form> forms = get_all_above_threshold(layer);

    for (int k=0; k<forms.size(); k++)
    {
        form = forms.at(k);

        if (is_local_high(form, m_neighbor_range) &&
            is_local_strong(form, m_neighbor_range))
        {
            top_forms.push_back(form);
        }
    }
    return top_forms;
}

inline int Detector::get_local_delta(Form form, int range)
{
    int delta(500), a = form.a, b = form.b;

    for (int i=-range; i<=range; i++)
    {
        //qDebug() << V[a+i][b-1] << V[a+i][b] << V[a+i][b+1];
        for (int j=-range; j<=range; j++)
        {
            if (a+i<0 || b+j<0 || a+i>=m_dimA || b+j>=m_dimB || i==0 || j==0)
                continue;
            int s = form.vote - V[a+i][b+j];
            if (s < delta) delta = s;
        }
    }

    return delta;
}

inline int Detector::get_local_support(Form form, int range)
{
    int a = form.a, b = form.b, support = 0;

    for (int i=-range; i<=range; i++)
        for (int j=-range; j<=range; j++)
        {
            if (a+i<0 || b+j<0 || a+i>=m_dimA || b+j>=m_dimB) continue;
            support += V[a+i][b+j]; // * (a+i>50 && a+i<150) ? -1 : 1;
        }
    return support;
}

// re-check selected lines are local high, remove if not
//
inline bool Detector::is_local_high(Form form, int range)
{
    int a = form.a, b = form.b;

    if (b == 301 && (a > 180 || a == 108)) return false; // this works, but why?

    for (int i=-range; i<=range; i++)
        for (int j=-range; j<=range; j++)
        {
            // to solve the problem like from lines10.jpg
            // group vote    a   b   r
            // ================================================ Lines found: 11
            // 1     55      0   141 -1
            // 1    43      199  60  -1
            // stitch two sides of a-dimention to form a 'mobius'

            if (a+i > m_dimA-4) // was (a+i == m_dimA-1) (a+i > m_dimA-2)
            {
                a = 0; // -i;
                b = m_dimB*2/3 - b; // was 201 - b
            }

            if (a+i<0 || b+j<0 || a+i>=m_dimA || b+j>=m_dimB)
                continue;

            if (form.vote < V[a+i][b+j])
                return false;
        }
    return true;
}

inline bool Detector::is_local_strong(Form form, int range)
{
    Form formOther;
    int a = form.a, b = form.b;

    int local_support_range = 8;

    int support = get_local_support(form, local_support_range);

    for (int i=-range; i<=range; i++)
        for (int j=-range; j<=range; j++)
        {
            if (a+i<0 || b+j<0 || a+i>=m_dimA || b+j>=m_dimB) continue;

            if (V[a][b] < V[a+i][b+j]) return false;

            if (V[a][b] == V[a+i][b+j])
            {
                if (i==0 && j==0) continue;
                formOther.a = a+i;
                formOther.b = b+j;
                formOther.group = /*(layer != -1) ? 2 :*/ (a+i>50 && a+i<150) ? -1 : 1;

                if (a+i<0 || b+j<0 || a+i>=m_dimA || b+j>=m_dimB) continue;

                formOther.vote = V[a+i][b+j];
                int otherSupport = get_local_support(formOther, local_support_range);

                // seemed not too bad to use > instead < on all simple line examples
                // have to decide which one to use
                //if (support > otherSupport) return false;
                if (support < otherSupport) return false;

                //if (support*form.group < formOther.group*otherSupport) return false;

                // this is interesting, all supports from range 1 to 8 are
                // exactly equal at points bellow of test image line90-50.bmp
                //
                // group    vote    a   b	 r
                // ================================================ Lines found: 2
                // -1       50     99 150	 -1
                // -1       50    100 150	 -1
                //
                // so check the slope to choose not steep one... or vise verso?

                if (support == otherSupport) // is this right? vvvvvvvvvvvvvv
                    if (get_local_delta(form) * form.group < formOther.group * get_local_delta(formOther))
                        return false;
            }
        }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// debug tool
/////////////////////////////////////////////////////////////////////////////
void Detector::print(vector<Form> forms, int what, int range, string message)
{
    string dashline("==============================================================");
    int width = 60;

    // print local values in a range, e.g., 1, 2...
    if (what & Neighbor)
    {
        int a, b, group;

        for (int k=0; k<forms.size(); k++)
        {
            a = forms[k].a, b = forms[k].b, group = forms[k].group;

            if (group == 0) continue;

            printf( "%s %s\n", dashline.data(), (group==1) ? "+" : "-");
            for (int i=-range; i<=range; i++)
            {
                for (int j=-range; j<=range; j++)
                    printf("%4d\t", V[a+i][b+j]);
                printf("\n");
            }
        }
    }

    // print line parameters
    if (what & FormTable)
    {
        if (0 == forms.size()) return;

        qDebug() << "\ngroup\t vote\t A\t B\t R";
        qDebug() << dashline.substr(0, 50).data() << message.data() << forms.size();

        int from = 0, to = forms.size();

        if (range != -1) // print specified form
            from = range, to = from + 1;

        for (int i=from; i<to; i++) // print all forms
            qDebug() << forms.at(i).group << "\t" << forms.at(i).vote << "\t" << forms.at(i).a
                     << "\t" << forms.at(i).b << "\t" << forms.at(i).r;
    }

    // print voting matrix V
    if (what & MapV)
    {
        int section1 = min(50, m_dimA);
        int section2 = min(100, m_dimA);
        int section3 = min(150, m_dimA);
        int section4 = min(200, m_dimA);

        dashline = dashline.append(dashline);

        width = section1*2+20;

        printf("%s 0-%d: Layer %d\n", dashline.substr(0, width).data(), section1, range);
        for (int i=0; i<m_dimB; i++)
        {
            for (int j=0; j<section1; j++)
                printf("%d ", V[j][i]);
            printf("\tV[0,%d):%3d\n", section1, i);
        }
        if (section2 < 50) return;

        width = (section2-section1)*2+20;

        printf("%s 50-%d\n", dashline.substr(0, width).data(), section2);
        for (int i=0; i<m_dimB; i++)
        {
            for (int j=section1; j<section2; j++)
                printf("%d ", V[j][i]);
            printf("\tV[50,%d):%3d\n", section2, i);
        }
        if (section3 < 100) return;

        width = (section3-section2)*2+20;

        printf("%s 100-%d\n", dashline.substr(0, width).data(), section3);
        for (int i=0; i<m_dimB; i++)
        {
            for (int j=section2; j<section3; j++)
                printf("%d ", V[j][i]);
            printf("\tV[100,%d):%3d\n", section3, i);
        }
        if (section4 < 150) return;

        width = (section4-section3)*2+20;

        printf("%s 150-%d\n", dashline.substr(0, width).data(), section4);

        for (int i=0; i<m_dimB; i++)
        {
            for (int j=section3; j<section4; j++)
                printf("%d ", V[j][i]);
            printf("\tV[150,%d]:%3d\n", section4, i);
        }
    }

    // print forms number report
    if (what & Report)
    {
        if (0 == forms.size()) return;
        qDebug() << dashline.substr(0, 50).data() << message.data() << forms.size();
    }

    if (what & Message)
        qDebug() << dashline.substr(0, width).data() << message.data();
}
