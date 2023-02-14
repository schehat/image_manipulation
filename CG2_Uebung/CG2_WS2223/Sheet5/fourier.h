#ifndef FOURIER_H
#define FOURIER_H


#include <qimage.h>
#include <cmath>


#include <complex>
#include <iostream>
#include <valarray>


namespace cg2 {

    QImage* doIFT(QImage* image);
    QImage* doFT(QImage* image);

    inline const double PI = 3.141592653589793238460;
    inline double** fourier_input = nullptr;
    inline double** fourier_im = nullptr;
    inline double** fourier_real = nullptr;
    typedef std::complex<double> Complex;
    typedef std::valarray<Complex> CArray;
    inline double** leistungsspektrum;

}

#endif // FOURIER_H
