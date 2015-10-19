#ifndef MERGE_FIND_H
#define MERGE_FIND_H

using namespace std;

struct Node
{
    Node(int _label, int _color = 0)
    {
        this->parent = this;
        rank   = 0;
        label  = _label;
        color = _color;
    }
    Node *parent;
    int color;
    int label;
    int rank;
    bool operator==(const Node &other) const {
        return ((*this).label == other.label);
    }
    bool operator!=(const Node &other) const {
        return !(*this == other);
    }
};

class merge_find
{
public:
    merge_find();
    Node find(Node x);
    Node merge(Node x, Node y);
    int find_lowest(int** neighbors, int range);
    vector<int> get_neighbor_labels(int** neighbors, int range);
    void test();

private:
    bool m_print;
};

#endif // MERGE_FIND_H
