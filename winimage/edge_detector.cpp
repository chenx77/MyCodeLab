#include <QtGui>
#include "edge_detector.h"

edge_detector::edge_detector()
{
}

QImage edge_detector::detectEdgeImage(const QImage image, int kernel)
{
    QImage destImage = QImage( image);
    QRect rect = image.rect();
    QColor qc;

    int H = rect.height();
    int W = rect.width();
    int **a;
    qreal gradient, gangle;

    int r, g, b;
    int maxg(0);

    for (int y=0; y<H; y++) // y=1; y<H-1;
    {
        for (int x=0; x<W; x++) // x=1; x<W-1;
        {
            a = get_neighbor_pixels(image, x, y, 1, true);
            edge_detect(a, kernel, &gradient, &gangle);
            deleteArray(a, 3); //size of array allocated by get_neighbor_pixels is 3x3, 3 = (range=1)*2+1

            if ((gangle >= -22.5 && gangle < 22.5) || (gangle >= 157.5 || gangle < -157.5))
            {
                r = 0;
                g = 0;
                b = 255;
            }
            else if ((gangle >= 22.5 && gangle < 67.5) || (gangle <= -112.5 && gangle > -157.5))
            {
                r = 0;
                g = 255;
                b = 0;
            }
            else if ((gangle >= 67.5 && gangle < 112.5) || (gangle <= -67.5 && gangle > -112.5))
            {
                r = 255;
                g = 255;
                b = 0;
            }
            else if ((gangle >= 112.5 && gangle < 157.5) || (gangle <= -22.5 && gangle > -67.5))
            {
                r = 255;
                g = 0;
                b = 0;
            }

            if (gradient > maxg)
                maxg = gradient;

            qc.setRgb((int)gradient & r, (int)gradient & g, (int)gradient & b);

            destImage.setPixel( x, y, qc.rgba());
        }
    }

    //printf("gangle = %f, maxg = %d\n", gangle, maxg);
    return destImage;
}

QImage edge_detector::smoothImage(const QImage image)
{
    QImage destImage = QImage( image);
    QColor qc;
    int Gauss[5][5] = {{2, 4, 5, 4, 2}, {4, 9, 12, 9, 4}, {5, 12, 15, 12, 5},
                       {4, 9, 12, 9, 4}, {2, 4, 5, 4, 2}};

    int **G = new int*[5];
    for (int i=0; i<5; i++)
    {
        G[i] = new int[5];
        for(int j=0; j<5; j++)
            G[i][j] = Gauss[i][j];
    }

    int H = image.height();
    int W = image.width();

    int* destData= (int *)destImage.bits();

    int **a, smooth;
    for (int y=2; y<H-2; y++)
    {
        for (int x=2; x<W-2; x++)
        {
            a = get_neighbor_pixels(image, x, y, 2);
            smooth = convolve(a, G, 5) / 159.f;
            deleteArray(a, 5); //2*2+1

            qc.setRgb(smooth, smooth, smooth);
            destData[x + y*W] = qc.rgba();
        }
    }
    return destImage;
}

QImage edge_detector::double_thresholding(const QImage image)
{
    QImage destImage = QImage( image);
    QColor qc;

    int T1 = 16;
    int T2 = 12;

    int H = image.height();
    int W = image.width();

    for (int i=0; i<H; i++)
        for (int j=0; j<W; j++)
        {
            qc = ((QColor)image.pixel(j, i));
            int lightness = qc.lightness();
            int r = qc.red(), g = qc.green(), b = qc.blue();

            if (lightness > 200 || lightness < T2)
            {
                qc.setRgb(0, 0, 0);
            }
            else if (lightness > T1)
            {                
                qc.setRgb(255, 255, 255);
            }
            else
            {
                qc.setRgb(r?255:0, g?255:0, b?255:0);
            }
            destImage.setPixel( j, i, qc.rgba());
        }
    return destImage;
}

inline void edge_detector::edge_detect(int **a, int kernel, qreal *gradient, qreal *gangle)
{
    // Scharr kernel
    int SCx[3][3] = {{-3, 0, 3}, {-10, 0, 10}, {-3, 0, 3}};
    int SCy[3][3] = {{-3,-10,-3}, { 0, 0, 0}, { 3, 10, 3}};

    // Sobel kernel
    int Sx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int Sy[3][3] = {{-1,-2,-1}, { 0, 0, 0}, { 1, 2, 1}};

    // Prewitt kernel
    int Px[3][3] = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
    int Py[3][3] = {{-1,-1,-1}, { 0, 0, 0}, { 1, 1, 1}};

    // Roberts Cross
    int Rx[3][3] = {{1, 0, 0}, {0, -1, 0}, {0, 0, 0}};
    int Ry[3][3] = {{0, 1, 0}, {-1, 0, 0}, {0, 0, 0}};

    int **Kx, **Ky;
    int k, gx, gy;

    if (kernel == 0) // Scharr kernel
    {
        k = 16;
        Kx = copy(SCx);
        Ky = copy(SCy);
    }
    else if (kernel == 1) // Sobel kernel
    {
        k = 4;
        Kx = copy(Sx);
        Ky = copy(Sy);
    }
    else if (kernel == 2) // Prewitt kernel
    {
        k = 3;
        Kx = copy(Px);
        Ky = copy(Py);
    }
    else if (kernel == 3) // Roberts Cross
    {
        k = 2;
        Kx = copy(Rx);
        Ky = copy(Ry);
    }

    gx = convolve(a, Kx, 3);
    gy = convolve(a, Ky, 3);

    deleteArray(Kx, 3);
    deleteArray(Ky, 3);

    *gradient = sqrt(gx*gx + gy*gy)/k;
    *gangle = 180/M_PI * atan2(gy, gx);
}

QImage edge_detector::nmsImage(const QImage image, int test)
{
    QImage destImage = QImage( image);
    QColor qc[9];

    int H = destImage.height();
    int W = destImage.width();

    int r, g, b;

    for (int i=1; i<H-1; i++)
    {
        for (int j=1; j<W-1; j++)
        {
            qc[0] = ((QColor)destImage.pixel(j-1, i-1));
            qc[1] = ((QColor)destImage.pixel(j-1, i));
            qc[2] = ((QColor)destImage.pixel(j-1, i+1));
            qc[3] = ((QColor)destImage.pixel(j, i-1));
            qc[4] = ((QColor)destImage.pixel(j, i));
            qc[5] = ((QColor)destImage.pixel(j, i+1));
            qc[6] = ((QColor)destImage.pixel(j+1, i-1));
            qc[7] = ((QColor)destImage.pixel(j+1, i));
            qc[8] = ((QColor)destImage.pixel(j+1, i+1));

            r = qc[4].red();
            g = qc[4].green();
            b = qc[4].blue();

            if (r > 0 && g > 0) // yellow, horizontal line --
            {
                if ((r <= qc[3].red() || r <= qc[5].red()) &&
                         (g <= qc[3].green() || g <= qc[5].green()))
                {
                    r = 0;
                    g = 0;
                }
            }
            else if (r > 0 && g < r) // red & not yellowish, diagonal line \.
            {
                if (r <= qc[2].red() || r <= qc[6].red())
                    r = 0;
            }
            else if (g > 0 && r < g) // green & not yellowish, diagonal line /
            {
                if (g <= qc[0].green() || g <= qc[8].green())
                    g = 0;
            }
            else if (b > 0) // blue, vertical line |
            {
                if (b <= qc[1].blue() || b <= qc[7].blue())
                    b = 0;
            }

            qc[4].setRgb(r, g, b);

            destImage.setPixel( j, i, qc[4].rgba());
        }
    }
    return destImage;
}

// two-pass blob discovery
#define Canny_Edge_Detection
QImage edge_detector::blob_extract(const QImage image)
{
    QImage destImage = QImage( image);
    merge_find mf;

    int H = destImage.height();
    int W = destImage.width();

    int* srcData = (int *)image.bits();
    int* destData = (int *)destImage.bits();

    // target image initialized with numbers
    for (int row=0; row<H; row++)
    {
        for (int column=0; column<W; column++)
        {
            QColor qc = QColor(srcData[row*W+column]);
#ifdef Canny_Edge_Detection
            // this initiation is used as step 5, edge tracking, of Canny Edge Detection
            // assign the pixel as black:0, white:1, yellow:2, red:3, green:4, blue:5
            int color = 0;
            if (qc.red() == 255 && qc.green() == 255 && qc.blue() == 255) color = 1;
            else if (qc.red() == 255 && qc.green() == 255) color = 2;
            else if (qc.red() == 255) color = 3;
            else if (qc.green() == 255) color = 4;
            else if (qc.blue() == 255) color = 5;
            destData[row*W+column] = color;
#else
            // this initiation is used for blobs categorizing
            // to be white if not background.
            destData[row*W+column] = (srcData[row*W+column] & 0x0000ff) > 200 ? 1 : 0;
#endif // Canny_Edge_Detection
       }
    }

    vector<Node> linked;
    vector<int> n_labels;

    int nextLabel(1);

    // First pass
    for (int row=0; row<H; row++)
    {
        for (int column=0; column<W; column++)
        {
            if (int color = destData[row*W+column] & 0x000007) // is not Background
            {
                // neighbors = connected elements with the current element's value
                int** neighbors = get_neighbor_pixels(destImage, column, row, 1);

                int lowest = mf.find_lowest(neighbors, 1);

                if (lowest == 0) // if neighbor is empty
                {
                    Node node = /*new*/ Node(nextLabel, destData[row*W+column] & 0x000007);
                    linked.push_back(node);
                    destData[row*W+column] = nextLabel;
                    nextLabel++;
                }
                else // to assign the smallest label
                {
                    destData[row*W+column] = lowest;
                    n_labels = mf.get_neighbor_labels(neighbors, 1);

                    // this is the key to link weak points to strong
                    if (color == 1)
                        linked[lowest-1].color = 1;

                    for (int l=0; l<(int)n_labels.size(); l++)
                        if (n_labels[l] > 0)
                            mf.merge(linked[n_labels[l]-1], linked[lowest-1]);
                }
                deleteArray(neighbors, 3); // 3=1*2+1
            }
        }
    }

    // Second pass
    for (int row=0; row<H; row++)
    {
        for (int column=0; column<W; column++)
        {
            int x = destData[row*W+column] & 0x0000ff;

            if (x) // is not Background
            {
                Node root = mf.find(linked[x-1]);
#ifdef Canny_Edge_Detection
                // this color recovery is for Canny Edge Detection
                QColor qc = (root.color == 1) ? Qt::white : Qt::black;
//                            (root.color == 2) ? Qt::yellow : // test
//                            (root.color == 3) ? Qt::red :
//                            (root.color == 4) ? Qt::green :
//                            (root.color == 5) ? Qt::blue : Qt::black;
                destData[row*W+column] = qc.rgba();
#else
                // Blobs categorizing
                destData[row*W+column] = 0x0000ff << root.label;
#endif // Canny_Edge_Detection
                //printf("(%i %i) ", x, root.color);
            }
        }
    }

    return destImage;
}


inline int edge_detector::convolve(int **f, int **g, int dim)
{
    int result(0);

    for (int i=0; i<dim; i++)
        for (int j=0; j<dim; j++)
            result += f[i][j] * g[i][j];

    return result;
}

inline int** edge_detector::get_neighbor_pixels(QImage image, int x, int y, int range, bool copy_margin)
{
    int H = image.height(), W = image.width();
    int cells = range*2+1;
    int** a = new int*[cells];
    for (int i = 0; i<cells; i++)
        a[i] = new int[cells];

    for (int j=-range; j<=range; j++)
        for(int i=-range; i<=range; i++)
        {
            if ((x+i<0 || x+i>=W) || (y+j<0 || y+j>=H))
            {
                if (!copy_margin)
                {
                    a[range+j][range+i] = 0;
                    continue;
                }

                if ((x+i<0 || x+i>=W) && (y+j<0 || y+j>=H))
                    a[range+j][range+i] = ((QColor)image.pixel(x, y)).lightness();
                else if (x+i<0 || x+i>=W)
                    a[range+j][range+i] = ((QColor)image.pixel(x, y+j)).lightness();
                else if (y+j<0 || y+j>=H)
                    a[range+j][range+i] = ((QColor)image.pixel(x+i, y)).lightness();
            }
            else
                a[range+j][range+i] = ((QColor)image.pixel(x+i, y+j)).lightness();
        }

    return a;
}

int **edge_detector::copy(int array[3][3])
{
    int **K = new int*[3];
    for (int i=0; i<3; i++)
    {
        K[i] = new int[3];
        for(int j=0; j<3; j++)
            K[i][j] = array[i][j];
    }
    return K;
}

void edge_detector::deleteArray(int **array, int size)
{
    for (int i=0; i<size; i++)
        delete[] array[i];

    delete[] array;
}
