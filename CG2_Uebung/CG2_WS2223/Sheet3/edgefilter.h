#ifndef EDGEFILTER_H
#define EDGEFILTER_H

#include <qimage.h>




namespace cg2 {
    QImage* doEdgeFilter(QImage * & image, int*& derivative_filter, int*& smoothing_filter, int desired_image);
    QImage* doLaplaceFilter(QImage * & image, int**& laplace_filter);
    QImage* doCanny(QImage * & img, double sigma, int tHi, int tLo);
    inline bool imageGotChangedFlag = true;
    QImage* doUSM(QImage *& image, double sharpening_value, double sigma, int tc);

    inline QImage* gaussBlurredImageCanny = nullptr;
    inline QImage* cannyCacheGradientX = nullptr;
    inline QImage* cannyCacheGradientY = nullptr;
    inline QImage* Emag = nullptr;
    inline QImage* Enms = nullptr;
    inline QImage* Ebin = nullptr;
    inline QImage* cannyCacheEmag = nullptr;
    inline QImage* cannyCacheEnms = nullptr;
    inline QImage* cannyCacheEbin = nullptr;
    inline int cannyCacheTLo = -1;
    inline double cannyCacheSigma = -1;
    inline double cannyCacheTHi = -1;
    int getOrientationSector(int dX, int dY);
    bool isLocalMax(QImage* &Emag, int u, int v, int s0, int tLo);
    void traceAndThreshold(QImage* &Enms, QImage* &Ebin, int u0, int v0, int tLo);
    QImage* doCanny(QImage * & img, double sigma, int tHi, int tLo);

}

#endif // EDGEFILTER_H
