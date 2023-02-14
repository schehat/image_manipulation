#include "fourier.h"

#include <qimage.h>
#include "imageviewer-qt5.h"
#include <cmath>

namespace cg2 {

    /**
     * @brief doIFT
     *          calculate the inverse fourier transformation with an 2D filter-image ()
     *          as input and returns the resulting 2D image
     *          Note:
     *              Use the strored complex values from doFT(...)
     * @param image
     *          reference on the Fourier coefficient filter image
     * @return  new image to show in the GUI
     */
    QImage* doIFT(QImage* image){

    int width = image->width();
    int height = image->height();

    if(fourier_im == nullptr){
        logFile << "First do Fourier-Transformation" << std::endl;
        return image;
    }

    int height_half = height/2;
    int width_half = width/2;
    // Apply Filter on stored fourier Im and real
    for (int v = 0; v <= height-1; v++) {
        for (int u = 0; u <= width-1; u++) {

            // Quadrant switching
            int x_index = u;
            int y_index = v;
            if(u < width_half){
                x_index+=width_half;
            } else {
                x_index-=width_half;
            }

            if(v < height_half){
                y_index+=height_half;
            } else {
                y_index-=height_half;
            }

            int value = image->pixelColor(x_index, y_index).red();

            if(value == 0) {
                fourier_im[u][v] = 0;
                leistungsspektrum[u][v] = 0;
                fourier_real[u][v] = 0;
            }



        }
    }


    double s_for_rows = 1 / std::sqrt(width);
    double s_for_colums = 1 / std::sqrt(height);

    // Für Alle rows:
    for (int v = 0; v <= height-1; v++) {
        int M = width;
        double temp_RE[M];
        double temp_IM[M];
        for(int m = 0; m < M; m++){

            // g ist die row in image[0-M][v]
            double sumRe = 0;
            double sumIm = 0;
            double phim = 2 * PI * m / M;
            for(int k = 0; k < M; k++){
                double gRe = fourier_real[k][v];
                double gIm = fourier_im[k][v];
                double cosw = std::cos(phim * k);
                double sinw = std::sin(phim *k);
                // is not forward pass!
                sinw = -sinw;
                sumRe += gRe *  cosw + gIm * sinw;
                sumIm += gIm * cosw - gRe * sinw;
            }
            temp_RE[m] = s_for_rows * sumRe;
            temp_IM[m] = s_for_rows * sumIm;

        }
        // Speicher temp_RE/IM persistent
        for(int m = 0; m < M; m++){
            fourier_real[m][v] = temp_RE[m];
            fourier_im[m][v] = temp_IM[m];
        }


    }


    // Für alle Spalten:
    for (int u = 0; u <= width-1; u++) {
        int M = height;
        double temp_RE[M];
        double temp_IM[M];
        for(int m = 0; m < M; m++){
            // g ist die column in image[u][0-M]
            double sumRe = 0;
            double sumIm = 0;
            double phim = 2 * PI * m / M;
            for(int k = 0; k < M; k++){
                double gRe = fourier_real[u][k];
                double gIm = fourier_im[u][k];
                double cosw = std::cos(phim * k);
                double sinw = std::sin(phim *k);
                // is not forward pass!
                sinw = -sinw;
                sumRe += gRe *  cosw + gIm * sinw;
                sumIm += gIm * cosw - gRe * sinw;
            }
            temp_RE[m] = s_for_colums * sumRe;
            temp_IM[m] = s_for_colums * sumIm;
        }
        for(int m = 0; m < M; m++){
            fourier_real[u][m] = temp_RE[m];
            fourier_im[u][m] = temp_IM[m];
        }
    }



    QImage* return_image = new QImage(*image);

    for (int v = 0; v <= height-1; v++) {
        for (int u = 0; u <= width-1; u++) {
            double imageValue = fourier_real[u][v];
            int q = (int) imageValue +0.5;
            q = std::max(q,0);
            q = std::min(q,255);
            return_image->setPixel(u, v, QColor(q, q, q).rgb());
        }
    }
    logFile << "Inverse Fourier Transformation done" << std::endl;
    return return_image;
    }

    // delete
    void deleteFourierArrays(int m) {
        for(int i = 0; i < m; ++i)
            delete [] fourier_im[i];
          delete [] fourier_im;

        for(int i = 0; i < m; ++i)
            delete [] fourier_real[i];
          delete [] fourier_real;

        for(int i = 0; i < m; ++i)
            delete [] leistungsspektrum[i];
        delete [] leistungsspektrum;

    }

    /**
     * @brief doFT
     *          calculate the fourier transformation with an 2D image as input and
     *          returns the visualization of the fourier coefficients as an image
     *          Note:
     *              store the complex values, they are needed in the inverse
     *              fourier transformation
     * @param image
     *          reference on the input image
     * @return  fourier coefficients (Leistungsspektrun) as image pointer
     */
    QImage* doFT(QImage* image){


        int width = image->width();
        int height = image->height();

        if(fourier_im != nullptr){
            deleteFourierArrays(width);
        }


        // dynamic allocation
        fourier_im = new double*[width];
        for(int i = 0; i < width; ++i)
            fourier_im[i] = new double[height];

        fourier_real = new double*[width];
        for(int i = 0; i < width; ++i)
            fourier_real[i] = new double[height];

        leistungsspektrum = new double*[width];
        for(int i = 0; i < width; ++i)
            leistungsspektrum[i] = new double[height];



        for (int v = 0; v <= image->height()-1; v++) {
            for (int u = 0; u <= image->width()-1; u++) {
                // was ist der Startzustand im komplexen 2D Signal???
                fourier_im[u][v] = image->pixelColor(u, v).red();
                fourier_real[u][v] = image->pixelColor(u, v).red();
            }
        }


        double s_for_rows = 1 / std::sqrt(width);
        double s_for_colums = 1 / std::sqrt(height);

        // Für Alle rows:
        for (int v = 0; v <= height-1; v++) {
            int M = width;
            double temp_RE[M];
            double temp_IM[M];
            for(int m = 0; m < M; m++){

                // g ist die row in image[0-M][v]
                double sumRe = 0;
                double sumIm = 0;
                double phim = 2 * PI * m / M;
                for(int k = 0; k < M; k++){
                    double gRe = fourier_real[k][v];
                    double gIm = fourier_im[k][v];
                    double cosw = std::cos(phim * k);
                    double sinw = std::sin(phim *k);
                    // is forward pass!
                    sumRe += gRe *  cosw + gIm * sinw;
                    sumIm += gIm * cosw - gRe * sinw;
                }
                temp_RE[m] = s_for_rows * sumRe;
                temp_IM[m] = s_for_rows * sumIm;

            }
            // Speicher temp_RE/IM persistent
            for(int m = 0; m < M; m++){
                fourier_real[m][v] = temp_RE[m];
                fourier_im[m][v] = temp_IM[m];
            }


        }

        // Für alle Spalten:
        for (int u = 0; u <= width-1; u++) {
            int M = height;
            double temp_RE[M];
            double temp_IM[M];
            for(int m = 0; m < M; m++){
                // g ist die column in image[u][0-M]
                double sumRe = 0;
                double sumIm = 0;
                double phim = 2 * PI * m / M;
                for(int k = 0; k < M; k++){
                    double gRe = fourier_real[u][k];
                    double gIm = fourier_im[u][k];
                    double cosw = std::cos(phim * k);
                    double sinw = std::sin(phim *k);
                    // is forward pass!
                    sumRe += gRe *  cosw + gIm * sinw;
                    sumIm += gIm * cosw - gRe * sinw;
                }
                temp_RE[m] = s_for_colums * sumRe;
                temp_IM[m] = s_for_colums * sumIm;
            }
            for(int m = 0; m < M; m++){
                fourier_real[u][m] = temp_RE[m];
                fourier_im[u][m] = temp_IM[m];
            }
        }

        // Erstelle aus fourier_real und fourier_im das leistungsspektrum
        int height_half = height/2;
        int width_half = width/2;
        for (int v = 0; v <= image->height()-1; v++) {
            for (int u = 0; u <= image->width()-1; u++) {
                double value = std::abs(std::sqrt((fourier_real[u][v]*fourier_real[u][v])+(fourier_im[u][v]*fourier_im[u][v])));
                leistungsspektrum[u][v] = value;

                // Erstelle aus dem leistungsspektrum das image

                int q = (int) value +0.5;
                q = std::max(q,0);
                q = std::min(q,255);

                // Quadrant switching
                int x_index = u;
                int y_index = v;
                if(u < width_half){
                    x_index+=width_half;
                } else {
                    x_index-=width_half;
                }

                if(v < height_half){
                    y_index+=height_half;
                } else {
                    y_index-=height_half;
                }

                image->setPixel(x_index, y_index, QColor(q, q, q).rgb());
            }
        }

        logFile << " Fourier Transformation done" << std::endl;
        return image;
    }

}
