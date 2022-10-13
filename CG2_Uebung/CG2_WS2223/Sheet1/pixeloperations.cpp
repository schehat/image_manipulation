#include "pixeloperations.h"
#include "imageviewer-qt5.h"


namespace cg2 {
    
    /**
     * @brief calcImageCharacteristics
     *      calculation of the histogram, average value and variance of the image
     *      no return values, just set the references to the correct values
     * @param image
     *      working image
     * @param histogram_ref
     *      Reference to "inline double* histogramm = new double[256]"
     *      doubles allowed from [0-100]
     *      calculate the histogram and fill the histogramm:
     *      the histogramm[0] is corresponding to the luminance value 0 and so on
     *      NOTE!: the histogram must be scaled (linear or logarithmic) so that the
     *          highest value is 100!
     *      (reason: histogram image is 256x100 pixel)
     * @param variance_ref
     *      calculate and set the variance of the image to the variance_ref
     * @param average_ref
     *      calculate and set the average of the image to the average_ref
     * @param linear_scaling
     *      boolean, if true -> scale the histogram linear
     *               if false -> scale the histogram logarithmic
     */
    void calcImageCharacteristics(QImage * image, double*& histogram_ref, int& variance_ref, int& average_ref, const bool linear_scaling){
        average_ref=0;
        variance_ref=0;
        // init histogram
        for (int i = 0; i < 256; ++i) {
            histogramm[i] = 0;
        }

        // fill histogramm
        for (int row = 0; row < image->width(); row++) {
            for (int col = 0; col < image->height(); col++) {
                QRgb pixel = image->pixel(row, col);
                int grey = round(0.299*qRed(pixel) + 0.587*qGreen(pixel) + 0.114*qBlue(pixel));
                histogram_ref[grey]+=1;
            }

        }

        double nPixel = (double) image->width() * image->height();

        // calc average
        int maxOccurGrey = 0;
        for (int i = 0; i < 256; i++) {
            if (histogram_ref[i] > maxOccurGrey)
                maxOccurGrey = histogram_ref[i];
            average_ref+=histogram_ref[i]*i;
        }

        average_ref *= 1 / nPixel;

        // calc variance
        for (int i = 0; i < 256; i++) {
            variance_ref+=histogram_ref[i]*(i-average_ref)*(i-average_ref);
        }

        variance_ref *= 1 / nPixel;

        for (int i = 0; i < 256; ++i) {
            histogram_ref[i] *= 100.0 / maxOccurGrey;
        }
        logFile << maxOccurGrey << "\n";
        logFile << "Image characteristics calculated:" << std::endl << "--- Average: " << average_ref << " ; Variance: " << variance_ref << std::endl << "--- Histogram calculated: " << "linear scaling = " << linear_scaling << std::endl;
    }

    /**
     * @brief changeImageDynamic
     *      calculate an image with the desired resolution (given bit depth -> dynamic value)
     *      and return the image pointer
     * @param image
     *      input image
     * @param newDynamicValue
     *      bit depth value for resolution values from 8 to 1
     * @return new Image to show in GUI
     */
    QImage* changeImageDynamic(QImage * image, int newDynamicValue) {
        workingImage = new QImage(*backupImage);
        int area = round(256.0 / newDynamicValue * newDynamicValue);
        for (int row = 0; row < image->width(); row++) {
            for (int col = 0; col < image->height(); col++) {
                QRgb pixel = image->pixel(row, col);
                int grey = round(0.299*qRed(pixel) + 0.587*qGreen(pixel) + 0.114*qBlue(pixel));
                int cb = round(-0.169*qRed(pixel) + -0.331*qGreen(pixel) + 0.5*qBlue(pixel));
                int cr = round(0.5*qRed(pixel) + -0.419*qGreen(pixel) - 0.08*qBlue(pixel));

                grey = area * round(grey / newDynamicValue);
                cb = area * round(cb + 128/ newDynamicValue) - 128;
                cr = area * round(cb + 128/ newDynamicValue) - 128;

                int r = grey + 45 * cr / 32 ;
                int g = grey - (11 * cb + 23 * cr) / 32 ;
                int b = grey + 113 * cb / 64 ;

               workingImage->setPixel(row, col, qRgb(r,g,b));
            }
        }
        logFile << "Dynamik des Bildes geändert auf: " + std::to_string(newDynamicValue) + " Bit" << std::endl;
        return workingImage;

    }

    /**
     * @brief adjustBrightness
     *      Add brightness adjust on each pixel in the Image
     * @param image
     *      Input Image to work with
     * @param brightness_adjust_factor
     *      [-255,255]
     *      the brightness adjust shift, will be added on each pixel
     * @return result image, will be shown in the GUI
     */
    QImage* adjustBrightness(QImage * image, int brightness_adjust_factor){
        for(int i=0;i<image->width();i++)
        {
            for(int j=0;j<image->height();j++)
            {

            }
        }

        logFile << "Brightness adjust applied with factor = " <<brightness_adjust_factor << std::endl;
        return image;

    }

    /**
     * @brief adjustContrast
     *      calculate an contrast adjustment on each pixel in the Image
     * @param image
     *      Input Image to work with
     * @param contrast_adjust_factor
     *      [0,3]
     *      the contrast adjust factor
     * @return result image, will be shown in the GUI
     */
    QImage* adjustContrast(QImage * image, double contrast_adjust_factor){

        for(int i=0;i<image->width();i++)
        {
            for(int j=0;j<image->height();j++)
            {
            }
        }

        logFile << "Contrast calculation done with contrast factor: " << contrast_adjust_factor << std::endl;
        return image;
    }


    /**
     * @brief doRobustAutomaticContrastAdjustment
     *      calculate the robust automatic contrast adjustment algorithm with the image as input
     * @param image
     *      Input Image to work with
     * @param plow
     *      [0%,5%]
     *      brightness saturation
     * @param phigh
     *      [0%,5%]
     *      dark saturation
     * @return result image, will be shown in the GUI
     */
    QImage* doRobustAutomaticContrastAdjustment(QImage * image, double plow, double phigh){

        for(int i=0;i<image->width();i++)
        {
            for(int j=0;j<image->height();j++)
            {
            }
        }

        logFile << "Robust automatic contrast adjustment applied with:"<< std::endl << "---plow = " << (plow*100) <<"%" << std::endl << "---phigh = " << (phigh*100)<<"%" << std::endl;

        return image;
    }

}


