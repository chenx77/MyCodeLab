#include <QtGui>
#include "merge_find.h"

merge_find::merge_find()
{
    m_print = false;
}

Node merge_find::find(Node x)
{    
    // Canny: spread strong point to root
    if (x.color == 1)
        x.parent->color = 1;
    // recruise to get root parent
    if ((*x.parent) != x)
        (*x.parent) = find((*x.parent));

    return (*x.parent);
}

Node merge_find::merge(Node x, Node y)
{
    Node xRoot = find(x);
    Node yRoot = find(y);
    Node root(0, 0);

    if (xRoot == yRoot) return xRoot;

    // if x and y are not already in same set, merge them
    if (xRoot.rank < yRoot.rank)
        root = *xRoot.parent = yRoot;
    else if (xRoot.rank > yRoot.rank)
        root = *yRoot.parent = xRoot;
    else
    {
        xRoot.rank = xRoot.rank + 1;
        root = *yRoot.parent = xRoot;
    }

    return root;
}

int merge_find::find_lowest(int** neighbors, int range)
{
    int lowest = 10000;

    for (int i=0; i<=range*2; i++)
        for (int j=0; j<=range*2; j++)
        {
            if (i==range && j==range) goto stop;
            int n = neighbors[i][j] & 0x0000ff;
            if (n > 0 && n < lowest) lowest = n;
        }
stop:
    if (lowest == 10000) lowest = 0;
    return lowest;
}

vector<int> merge_find::get_neighbor_labels(int** neighbors, int range)
{
    static vector<int> result;
    result.clear();

    for (int i=0; i<=range*2; i++)
    {
        if (m_print) qDebug() << neighbors[i][0] << neighbors[i][1] << neighbors[i][2];
        for (int j=0; j<=range*2; j++)
        {
            if (i==range && j==range) goto ok;
            if (neighbors[i][j] > 0)
                result.push_back(neighbors[i][j]);
        }
    }
ok:
    if (m_print) qDebug();
    return result;
}

void merge_find::test()
{
    Node *root1 = new Node(1, 5);
    Node *root2 = new Node(2, 2);
    Node *root3 = new Node(3, 3);
    Node *root4 = new Node(4, 4);
    Node *root5 = new Node(5, 1);

    qDebug() << "root1(" << root1->label << root1->rank << root1->color << "), " << root1->parent;
    qDebug() << "root5(" << root5->label << root5->rank << root5->color << "), " << root5->parent;

    merge(*root1, *root2);
    merge(*root2, *root3);
    merge(*root3, *root4);
    merge(*root4, *root5);

    qDebug() << "### merge(root1, root2)...(root4, root5)";
    qDebug() << "### root5(" << root5->label << root5->rank << root5->color << "), root(" << find(*root5).label << find(*root5).rank << find(*root5).color << ")";
    qDebug() << "### root4(" << root4->label << root4->rank << root4->color << "), root(" << find(*root4).label << find(*root4).rank << find(*root4).color << ")";
    qDebug() << "### root3(" << root3->label << root3->rank << root3->color << "), root(" << find(*root3).label << find(*root3).rank << find(*root3).color << ")";
    qDebug() << "### root2(" << root2->label << root2->rank << root2->color << "), root(" << find(*root2).label << find(*root2).rank << find(*root2).color << ")";
    qDebug() << "### root1(" << root1->label << root1->rank << root1->color << "), root(" << find(*root1).label << find(*root1).rank << find(*root1).color << ")";

//    qDebug() << "root3" << root3.label << root3.rank << "parent" << root3.parent->label;
//    qDebug() << "root4" << root4.label << root4.rank << "parent" << root4.parent->label;

//    merge(root3, root4);

//    qDebug() << "*** root3 = merge(root3, root4)";
//    qDebug() << "*** root3" << root3.label << root3.rank << "parent" << root3.parent->label;
//    qDebug() << "*** root4" << root4.label << root4.rank << "parent" << root4.parent->label;

//    qDebug() << "root5" << root5.label << root5.rank << root5.parent->label;
//    merge(root4, root5);
//    qDebug() << "merge(root4, root5);";
//    qDebug() << "### root4" << root4.label << root4.rank << root4.parent->label;
//    qDebug() << "### root5" << root5.label << root5.rank << root5.parent->label;

//    Node root6 = makeSet(6);
//    merge(root6, root4);
//    qDebug() << "merge(root6, root4)";
//    qDebug() << "### root6" << root6.label << root6.rank << "parent" << root6.parent->label;
//    qDebug() << "### root4" << root4.label << root4.rank << "parent" << root4.parent->label;

//    Node root7 = find(root6);
//    qDebug() << "find(root6)" << root7.label << root7.rank << root7.parent;

//    bool same = (root1 != root3);
//    qDebug() << "###" << same;
}
