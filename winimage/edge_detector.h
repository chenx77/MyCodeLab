#ifndef EDGE_DETECTOR_H
#define EDGE_DETECTOR_H

#include <QImage>
#include "merge_find.h"

using namespace std;

class edge_detector
{
public:
    edge_detector();
    QImage detectEdgeImage(const QImage image, int kernel = 0);
    QImage smoothImage(const QImage image);
    QImage nmsImage(const QImage image, int test = 0);
    QImage double_thresholding(const QImage image);
    QImage blob_extract(const QImage image);

private:
    void edge_detect(int **a, int kernel, qreal *gradient, qreal *gtheta);
    int** get_neighbor_pixels(QImage image, int x, int y, int range, bool copy_margin = false);
    int convolve(int  **f, int **g, int dim);
    int **copy(int array[3][3]);
    void deleteArray(int **array, int size);
};

#endif // EDGE_DETECTOR_H
