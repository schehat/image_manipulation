#ifndef HOUGH_H
#define HOUGH_H

#include <qimage.h>
#include <cmath>



namespace cg2 {


    QImage* showAccumulatorArray(QImage* image, int & angular_steps, int & radial_steps, bool do_bias_compensation, double threshhold_a_min,
                             bool do_threshhold_reduction, bool do_non_maxima_suppression,
                             double canny_sigma, int canny_tHi, int canny_tLo);
    QImage* drawHoughLinesInImage(QImage* image, int & angular_steps, int & radial_steps, bool do_bias_compensation, double threshhold_a_min,
                             double canny_sigma, int canny_tHi, int canny_tLo);


    inline int** accuMaxY = nullptr;
    inline int** accuMinY = nullptr;
    inline int** accuMaxX = nullptr;
    inline int** accuMinX = nullptr;
    inline int anzahlLines = 0;
    void deleteHoughArrays(int angular_steps);
}
#endif // HOUGH_H
