#include "hough.h"

#include <qimage.h>
#include "imageviewer-qt5.h"
#include <cmath>
#include <iostream>

namespace cg2 {

    // delete
    void deleteHoughArrays(int angular_steps){

        int m = angular_steps;
        for(int i = 0; i < m; ++i)
            delete [] accuMaxY[i];
          delete [] accuMaxY;

        for(int i = 0; i < m; ++i)
            delete [] accuMinY[i];
          delete [] accuMinY;

        for(int i = 0; i < m; ++i)
            delete [] accuMaxX[i];
          delete [] accuMaxX;

        for(int i = 0; i < m; ++i)
            delete [] accuMinX[i];
          delete [] accuMinX;


    }

    // delete
    bool houghIsLocalMax(QImage* &Emag,int v,int u){
        int center = qRed(Emag->pixel(v,u));

        int xMax = Emag->width() -1;
        int yMax = Emag->height() -1;


        int obenLinks = qRed(Emag->pixel(std::max(v - 1,0), std::max((u -1),0)));
        if(center > obenLinks){
            return false;
        }

        int obenRechts = qRed(Emag->pixel(std::min(v + 1,xMax), std::max(u -1,0)));
        if(center > obenRechts){
            return false;
        }

        int oben = qRed(Emag->pixel(v , std::max(u -1,0)));
        if(center > oben){
            return false;
        }

        int links = qRed(Emag->pixel(std::max(v - 1,0) , u));
        if(center > links){
            return false;
        }

        int rechts = qRed(Emag->pixel(std::min(v + 1, xMax), u));
        if(center > rechts){
            return false;
        }

        int untenLinks = qRed(Emag->pixel(std::max((v - 1),0), std::min((u +1),yMax)));
        if(center > untenLinks){
            return false;
        }

        int  unten = qRed(Emag->pixel(v ,  std::min((u +1),xMax)));
        if(center > unten){
            return false;
        }

        int  untenRechts = qRed(Emag->pixel(std::min(v + 1, xMax),  std::min((u +1),yMax)));
        if(center > untenRechts){
            return false;
        }

        return true;
    }

    /**
     * @brief showAccumulatorArray
     *          apply Hough-Transformation and return the accumulator array in four 4 ways:
     *          1. without any changes
     *          2. with threshhold reduction
     *          3. with non maxima suppression
     *          4. with non maxima suppression and threshhold reduction
     * @param image
     *          input image
     * @param angular_steps
     *          angular steps for the first demension of accumulator array
     * @param radial_steps
     *          radial steps for the second demension of accumulator array
     * @param do_bias_compensation
     *          Boolean: if true apply bias compensation
     * @param threshhold_a_min
     *          Hough transformation threshhold parameter in percent
     * @param do_threshhold_reduction
     *          Boolean: if true apply threshold reduction
     * @param do_non_maxima_suppression
     *          Boolean: if true apply non maxima suppression
     * @param canny_sigma
     *          Canny edge detector parameter
     *          sigma for gauss smoothing
     * @param canny_tHi
     *          Canny edge detector parameter
     *          Threshhold tHi
     * @param canny_tLo
     *          Canny edge detector parameter
     *          Threshhold tLo
     * @return
     *          Accumulator array as QImage pointer in dimension [angular_steps * radial_steps]
     */
    QImage* showAccumulatorArray(QImage* image, int & angular_steps, int & radial_steps, bool do_bias_compensation, double threshhold_a_min,
                                 bool do_threshhold_reduction, bool do_non_maxima_suppression,
                                 double canny_sigma, int canny_tHi, int canny_tLo){
        logFile << "Starte Hough-Transformation" << std::endl;

        logFile << "Hough Transformation done | angular steps = " <<  angular_steps << "  |  radial steps  = " << radial_steps << std::endl;
        if(do_threshhold_reduction && do_non_maxima_suppression){
            logFile << "Show accumulator Array with combined evaluation" << std::endl;
        } else if(do_threshhold_reduction) {
            logFile << "Show accumulator Array with threshold evaluation" << std::endl;
        } else if(do_non_maxima_suppression) {
            logFile << "Show accumulator Array with non-maxima suppression evaluation" << std::endl;
        } else {
            logFile << "Show accumulator Array" << std::endl;
        }
        logFile << "Bias Kompensation " << do_bias_compensation << std::endl;

        delete image;
        image = new QImage(*backupImage);
        int width = image->width();
        int height = image->height();

        int xr = width / 2;
        int yr = height /2;
        int m = angular_steps;
        int n = radial_steps;

        if(accuMaxY != nullptr){
            deleteHoughArrays(angular_steps);
        }
        // dynamic allocation
        double** accuArray = new double*[m];
        for(int i = 0; i < m; ++i)
            accuArray[i] = new double[n];


        // dynamic allocation
        int** accuMaxPossible = new int*[m];
        for(int i = 0; i < m; ++i)
            accuMaxPossible[i] = new int[n];


        // dynamic allocation
        accuMaxY = new int*[m];
        for(int i = 0; i < m; ++i)
            accuMaxY[i] = new int[n];

        // dynamic allocation
        accuMinY = new int*[m];
        for(int i = 0; i < m; ++i)
            accuMinY[i] = new int[n];


        // dynamic allocation
        accuMinX = new int*[m];
        for(int i = 0; i < m; ++i)
            accuMinX[i] = new int[n];

        // dynamic allocation
        accuMaxX = new int*[m];
        for(int i = 0; i < m; ++i)
            accuMaxX[i] = new int[n];


        //fill
        for(int i = 0; i < m; ++i)
          for(int j = 0; j < n; ++j){
              accuMinY[i][j] = height;
              accuMaxX[i][j] = 0;
              accuMinX[i][j] = width;
              accuMaxY[i][j] = 0;
              accuMaxPossible[i][j] = 0;
              accuArray[i][j] = 0;

          }


        double dTheta = M_PI / m;
        double dRadial = sqrt(width * width  +  height * height) / n;
        int jNull = (n) / 2;
        QImage *accumulator = new QImage(m,n, QImage::Format_RGB32); // n x m != M x N !!!

        image = cg2::doCanny(image, canny_sigma, canny_tHi, canny_tLo);

        double allMaxAccu = 1;
        for(int u = 0; u<width;u++){
            for(int v = 0; v<height;v++) {

                int x = u - xr;
                int y = v - yr;
                if(qGreen(image->pixel(u,v)) > 0) {

                    for(int i = 0; i < m ;i++){
                        double theta = dTheta * (i+0) ;
                        double radial = x * cos(theta) + y * sin(theta);
                        int j = (jNull + (radial/dRadial)) + 0.5;

                        accuArray[i][j] = accuArray[i][j] + (double)(qRed(image->pixel(u,v)) / 255.);
                        accuMaxPossible[i][j] += 1;
                        if(allMaxAccu < accuArray[i][j]){
                            allMaxAccu = accuArray[i][j];
                        }

                        if(u > accuMaxX[i][j]) {
                            accuMaxX[i][j] = u;
                        }
                        if ( u < accuMinX[i][j] ) {
                            accuMinX[i][j] = u;
                        }

                        if(v > accuMaxY[i][j]) {
                            accuMaxY[i][j] = v;
                        }
                        if ( v < accuMinY[i][j] ) {
                            accuMinY[i][j] = v;
                        }

                   }

                } else if(do_bias_compensation){
                    for(int i = 0; i < m ;i++){
                        double theta = dTheta * (i+0) ;
                        double radial = x * std::cos(theta) + y * std::sin(theta);
                        int j = (jNull + (radial/dRadial)) + 0.5;
                        accuMaxPossible[i][j] += 1;


                    }

                }
            }
        }
        double max_accu_array_value = 0;
        for(int w = 0; w<m;w++){
            for(int h = 0; h<n;h++) {


                double accu = accuArray[w][h];
                double doubleColor;
                if(do_bias_compensation) {

                    double accuMaxPossibleValue = accuMaxPossible[w][h];

                    doubleColor = (accu / std::max(accuMaxPossibleValue,1.)) * 255.;
                } else {
                    doubleColor = (accu / allMaxAccu) *255.;
                }
                int color = (int)doubleColor +0.5;
                if(max_accu_array_value < doubleColor) {
                    max_accu_array_value = doubleColor;
                }
                color = color - 255;
                QRgb colorRGB = qRgb(std::abs(color),std::abs(color),std::abs(color));
                accumulator->setPixel(w,h,colorRGB);
            }
        }

        image = new QImage(*accumulator);
        int aMin = threshhold_a_min * max_accu_array_value;
        anzahlLines =0;
        int low_width = image->width();
        int low_height = image->height();

        for(int i = 0; i < m; ++i)
            delete [] accuArray[i];
          delete [] accuArray;

        for(int i = 0; i < m; ++i)
            delete [] accuMaxPossible[i];
          delete [] accuMaxPossible;


        if(do_threshhold_reduction && do_non_maxima_suppression){
            int invAMin = std::abs(aMin -255);
            std::cout << " invAMin: " << invAMin << std::endl;

            for(int w = 0; w<low_width;w++){
                for(int h = 0; h<low_height;h++) {
                    if( (!houghIsLocalMax(image,w,h)) || (qRed(image->pixel(w,h)) > invAMin)) {
                        QRgb colorRGB = qRgb(255,255,255);
                        image->setPixel(w,h,colorRGB);
                    } else {
                        anzahlLines++;
                    }
                }
            }
            logFile << " Hough Transformation Combined" << std::endl;
            return image;
        } else if(do_threshhold_reduction) {
            //int aMin = threshhold_a_min * max_accu_array_value;

            int invAMin = std::abs(aMin -255);
            std::cout << " invAMin: " << invAMin << std::endl;
            for(int w = 0; w<low_width;w++){
                for(int h = 0; h<low_height;h++) {

                    if( qRed(image->pixel(w,h)) > invAMin) {

                        QRgb colorRGB = qRgb(255,255,255);
                        image->setPixel(w,h,colorRGB);
                    }
                }
            }
            logFile << " Hough Transformation Schwellwertoperation | aMin = " << aMin << std::endl;
            return image;
        } else if(do_non_maxima_suppression) {
            for(int w = 0; w<low_width;w++){
                for(int h = 0; h<low_height;h++) {
                    if( !houghIsLocalMax(image,w,h)) {
                        QRgb colorRGB = qRgb(255,255,255);
                        image->setPixel(w,h,colorRGB);
                    }
                }
            }
            logFile << " Hough Transformation Non-Maxima Suppression" << std::endl;
            return image;
        } else {
            return image;
        }
    }

    /**
     * @brief drawHoughLinesInImage
     *            apply Hough-Transformation, draw the line in the input image
     *            and return the this image
     *            with non maxima suppression and threshhold reduction
     * @param image
     *          input image
     * @param angular_steps
     *          angular steps for the first demension of accumulator array
     * @param radial_steps
     *          radial steps for the second demension of accumulator array
     * @param do_bias_compensation
     *          Boolean: if true apply bias compensation
     * @param threshhold_a_min
     *          Hough transformation threshhold parameter in percent
     * @param canny_sigma
     *          Canny edge detector parameter
     *          sigma for gauss smoothing
     * @param canny_tHi
     *          Canny edge detector parameter
     *          Threshhold tHi
     * @param canny_tLo
     *          Canny edge detector parameter
     *          Threshhold tLo
     * @return
     *          input image as QImage pointer with found lines
     */
    QImage* drawHoughLinesInImage(QImage* image, int & angular_steps, int & radial_steps, bool do_bias_compensation, double threshhold_a_min,
                                  double canny_sigma, int canny_tHi, int canny_tLo){


        QImage* akkuArray = showAccumulatorArray(image, angular_steps,radial_steps,do_bias_compensation,threshhold_a_min,
                                     true, true,
                                     canny_sigma, canny_tHi, canny_tLo);
        image = new QImage(*backupImage);


        double lines[anzahlLines][6];
        int m = angular_steps;
        int n = radial_steps;
        int width = akkuArray->width();
        int height = akkuArray->height();

        int startWidth = image->width();
        int startHeight = image->height();
        int xr = startWidth / 2;
        int yr = startHeight /2;
        double dTheta = M_PI / m;
        double dRadial = sqrt(startWidth * startWidth  +  startHeight * startHeight) / n;
        int jNull = (n) / 2;

        int count = 0;
        for(int i = 0; i<width;i++){
            for(int j = 0; j<height;j++) {
                if( qRed(akkuArray->pixel(i,j)) < 255) {  //schwarz und weiß inventiert
                    double theta = dTheta * (i);
                    double radial = dRadial * (j-jNull);

                    lines[count][0] = theta;
                    lines[count][1] = radial;
                    lines[count][2] = accuMinX[i][j];
                    lines[count][3] = accuMaxX[i][j];
                    lines[count][4] = accuMinY[i][j];
                    lines[count][5] = accuMaxY[i][j];
                    count ++;
                }
            }
        }
        logFile << count << " Geraden wurden gefunden"  << std::endl;

        for(int i = 0; i<startWidth;i++){
            for(int j = 0; j<startHeight;j++) {
                for(int l = 0; l < anzahlLines; l++){

                    int minX = lines[l][2];
                    int maxX = lines[l][3];
                    int minY = lines[l][4];
                    int maxY = lines[l][5];

                    if((i >= minX) && (i <= maxX) && (j >= minY) && (j <= maxY)){

                        double theta =  lines[l][0];
                        double radial = lines[l][1];
                        if(((((i-xr)*cos(theta) + (j-yr) * sin(theta)) <= (radial +0.5)) && ((((i-xr)*cos(theta) + (j-yr) * sin(theta)) >= (radial - 0.5)) ))){

                            QRgb color = qRgb(255,0,0);
                            image->setPixel(i,j,color);
                            break;
                        }
                    }

                }
            }
        }


        return image;
    }
}
