#include "edgefilter.h"
#include "imageviewer-qt5.h"
#include <cmath>
#include<iostream>



namespace cg2 {

    /**
     * @brief doEdgeFilter
     *      calculate edge filter like sobel or prewitt with the help of separability.
     *      returns either the X, the Y or the entire Gradient.
     * @param image
     *      input image
     * @param derivative_filter
     *      1x3 1D filter array for derivative part
     * @param smoothing_filter
     *      1x3 1D filter array for the smoothing part
     * @param desired_image
     *      code for the desired image, that has to be shown:
     *      - desired_image = 1 -> show only X Gradient
     *      - desired_image = 2 -> show only Y Gradient
     *      - desired_image = 0 -> show Gradient: |∇I|
     * @return new Image to show in GUI
     */
    QImage* doEdgeFilter(QImage * & image, int*& derivative_filter, int*& smoothing_filter, int desired_image){
        delete image;
        image = new QImage(*backupImage);
        QImage* copy = new QImage(*image);

        QImage* gradientX = new QImage(*image);
        QImage* gradientY = new QImage(*image);
        QImage* gaussBlurredImage = new QImage(*image);

        /******************
         * Bild glätten mit smoothing_filter
         *****************/
            int center = 1;
            int *h = smoothing_filter; // Ungerade Anzahl, Größe des Gauss-Filters

            // Gauss-Kernel füllen
            int hLength = 2*center+1; // Größe des Gauss-Filters, wie oben 1D Array dieser Größe erstellt

            // Berechne Summe der Koeffizienten, um double s ermitteln zu können
            double sumOfCoefficients = 0.0;
            for (int i = 0; i < hLength; i++) {
                logFile << "h at " << i << ": " <<+ h[i] << std::endl;
                sumOfCoefficients += h[i];
            }

            // Ermittle double s als Gewichtungswert außerhalb der Matrix
            double s = 1.0/sumOfCoefficients; // Summe der Koeffizienten aus der Filtermatrix

            logFile << "sumOfCoefficients: " + std::to_string(sumOfCoefficients) << std::endl;
            logFile << "double s: " + std::to_string(s) << std::endl;


            // Sigma Gauß-Glättung über das Bild laufen lassen
            // Graubild

            // Gehe alle Pixel der einzelnen ZEILEN durch und wende den Filter an;
            for (int v = 0; v < image->height(); v++) {
                for (int u = 0; u < image->width(); u++) {
                    // Filter-Ergebnis der Position (u,v) berechnen
                    double sum = 0;
                    for (int i = -center; i <= center; i++) { // Hier nur noch eine For-Schleife

                        // Bestimmte Intensitätswert mit gespiegelter Randbedingung
                        QColor color;
                        int uNew = u+i;
                        if (u+i < 0) {
                            // Spiegle u ins Positive durch Multiplikation mit -1
                            uNew = (-1)*(u+i);
                        } else if (u+i >= image->width()) {
                            // Spiegle u ins Positive indem vom maximalen Randpixel i abgezogen werden
                            uNew = image->width()-1-i;
                        }
                        color = copy->pixelColor(uNew, v);

                        // Hole Intensitätswert p und summiere gewichtet auf
                        double p = 0.299*color.red() + 0.587*color.green() + 0.114*color.blue() ;
                        double c = h[i+center]; // Filter mit center verschieben, sodass auf h[0] zugegriffen werden kann
                        sum = sum + c*p;
                    }

                    int q = (int) (s * sum + 0.5); // Gewichtung einbeziehen und runden
                    if (q < 0) q = 0; // Clamping
                    if (q > 255) q = 255; // Clamping

                    image->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
                }
            }

            // Zeilen durchlaufen ist fertig. image hält nun das zeilenveränderte Bild
            delete copy;
            copy = new QImage(*image); // Neue Kopie des Bildes anlegen, um auf Werten des zuvor angewendeten Filters zu rechnen

            // Gehe alle Pixel der einzelnen SPALTEN durch und wende den Filter an;
            for (int v = 0; v < image->height(); v++) {
                for (int u = 0; u < image->width(); u++) {
                    // Filter-Ergebnis der Position (u,v) berechnen
                    double sum = 0;
                    for (int i = -center; i <= center; i++) { // Hier nur noch eine For-Schleife

                        // Bestimmte Intensitätswert je nach Randbedingungen
                        QColor color;
                        int vNew = v+i;
                        if (v+i < 0) {
                            // Spiegle u ins Positive durch Multiplikation mit -1
                            vNew = (-1)*(v+i);
                        } else if (v+i >= image->height()) {
                            // Spiegle u ins Positive indem vom maximalen Randpixel i abgezogen werden
                            vNew = image->height()-1-i;
                        }
                        color = copy->pixelColor(u, vNew);

                        // Hole Intensitätswert p und summiere gewichtet auf
                        int p = color.red(); // copy.pixelColor(u+i, v+j);
                        double c = h[i+center]; // Filter mit center verschieben, sodass auf h[0] zugegriffen werden kann
                        sum = sum + c*p;
                    }

                    int q = (int) (s * sum + 0.5); // Gewichtung einbeziehen und runden
                    if (q < 0) q = 0; // Clamping
                    if (q > 255) q = 255; // Clamping
                    image->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
                }
            }


        delete gaussBlurredImage;
        gaussBlurredImage = new QImage(*image); // Neues blurred Image cachen
        logFile << "BlurredImage nach Gauss-Filter wurde gecached." << std::endl;
        logFile << "Image desired: " << desired_image << std::endl;


        /******************
         * Berechne x-Gradienten
         *****************/


        // Graubild
        // Gradientberechnung für X-Richtung
        for (int v = 0; v <= image->height()-1; v++) { // v<=N-L-1 (Zentralbereich) bzw. v<=N-1 (andere Randbedingungen)
            for (int u = 0; u <= image->width()-1; u++) { // v<=M-K-1 (Zentralbereich) bzw. v<=M-1 (andere Randbedingungen)
                // Filter-Ergebnis der Position (u,v) berechnen
                float sum = 0;
                for (int i = -1; i <= 1; i++) {
                    // Gespiegelte Randbedingung
                    QColor color;
                    int uNew = u+i;
                    if (u+i < 0) {
                        // Spiegle u ins Positive durch Multiplikation mit -1
                        uNew = (-1)*(u+i);
                    } else if (u+i >= image->width()) {
                        // Spiegle u ins Positive indem vom maximalen Randpixel i abgezogen werden
                        uNew = image->width()-1-i;
                    }
                    color = gaussBlurredImage->pixelColor(uNew, v);

                    // Hole Intensitätswert p und summiere gewichtet auf
                    int p = color.red(); // copy.pixelColor(u+i, v+j);
                    float c = derivative_filter[i+1]; // filter[j+L][i+K]; L und K sind halbe Filterbreite
                    sum = sum + c*p;
                }

                int q = (int) (sum + 0.5); // Gewichtung einbeziehen und runden
                q += 127; // Wertebereich der Ableitung ist [-127, 127]
                if (q < 0) q = 0; // Clamping
                if (q > 255) q = 255; // Clamping
                gradientX->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);

            }
        }

        if (desired_image == 1 ){
            delete image;
            delete gradientY;
            delete copy;
            delete gaussBlurredImage;
            return gradientX;
        }

        /******************
         * Berechne y-Gradienten
         *****************/

        // Graubild
        // Gradientberechnung für Y-Richtung
        for (int v = 0; v <= image->height()-1; v++) {
            for (int u = 0; u <= image->width()-1; u++) {
                // Filter-Ergebnis der Position (u,v) berechnen
                float sum = 0;
                for (int j = -1; j <= 1; j++) {

                    // Gespiegelte Randbedingung
                    QColor color;
                    int vNew = v+j;
                    // Behandle u und v in getrennten if's, da es Fälle gibt, wo u und v eine Bearbeitung benötigen
                    if (v+j < 0) {
                        // Spiegle v ins Positive durch Multiplikation mit -1
                        vNew = (-1)*(v+j);
                    } else if (v+j >= image->height()) {
                        // Spiegle v ins Positive indem vom maximalen Randpixel j abgezogen werden
                        vNew = image->height()-1-j;
                    }
                    color = gaussBlurredImage->pixelColor(u, vNew);

                    // Hole Intensitätswert p und summiere gewichtet auf
                    double p = color.red(); // copy.pixelColor(u+i, v+j);
                    float c = derivative_filter[j+1]; // filter[j+L][i+K]; L und K sind halbe Filterbreite
                    sum = sum + c*p;
                }
                int q = (int) (sum + 0.5); // Gewichtung einbeziehen und runden
                q += 127; // Wertebereich der Ableitung ist [-127, 127]
                if (q < 0) q = 0; // Clamping
                if (q > 255) q = 255; // Clamping
                gradientY->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
            }
        }
        if (desired_image == 2 ){
            delete gradientX;
            delete image;
            delete copy;
            delete gaussBlurredImage;
            return gradientY;
        }
        for (int u = 0; u < image->width(); u++) {
            for (int v = 0; v < image->height(); v++) {
                // Setze in Emag die Beträge der Gradienten
                QColor colorX = gradientX->pixelColor(u, v);
                QColor colorY = gradientY->pixelColor(u, v);

                int colorXInt = colorX.red();
                int colorYInt = colorY.red();

                int gradientX = colorXInt - 127; // Wertebereich bei Lesen aus Ableitung wieder anpassen
                int gradientY = colorYInt - 127; // Werdebereich bei Lesen aus Ableitung wieder anpassen

                double powGradientX = std::pow(gradientX, 2);
                double powGradientY = std::pow(gradientY, 2);
                double powAddition = powGradientX + powGradientY;
                double gradientenBetrag = std::sqrt(powAddition); // |∇I| -> Betrag des Gradienten (Vektors)

                image->setPixel(u, v, QColor(gradientenBetrag, gradientenBetrag, gradientenBetrag).rgb());

            }
        }
        delete gradientX;
        delete gradientY;
        delete copy;
        delete gaussBlurredImage;

        return image;
    }


    /**
     * @brief doLaplaceFilter
     *      calculate the laplace edge filter
     * @param image
     *      input image
     * @param laplace_filter
     *      3x3 2D laplace filter matrix
     * @return new Image to show in GUI
     */
    QImage* doLaplaceFilter(QImage * & image, int**& laplace_filter){


        logFile << "Do Laplace: " << std::endl;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                logFile <<  + laplace_filter[i][j] << " ";
            }
            logFile << std::endl;
        }

        QImage* copy = new QImage(*image);
        for (int v = 0; v < image->height(); v++) {
            for (int u = 0; u < image->width(); u++) {
                double sum = 0;
                for (int i = -1; i <= 1; i++) {
                    for (int j = -1; j <= 1; j++) {
                        // Bestimmte Intensitätswert je nach Randbedingungen
                        QColor color;
                        int uNew = u+i;
                        int vNew = v+j;
                        if (u+i < 0) {
                            // Spiegle u ins Positive durch Multiplikation mit -1
                            uNew = (-1)*(u+i);
                        } else if (u+i >= image->width()) {
                            // Spiegle u ins Positive indem vom maximalen Randpixel i abgezogen werden
                            uNew = image->width()-1-i;
                        }
                        // Behandle u und v in getrennten if's, da es Fälle gibt, wo u und v eine Bearbeitung benötigen
                        if (v+j < 0) {
                            // Spiegle v ins Positive durch Multiplikation mit -1
                            vNew = (-1)*(v+j);
                        } else if (v+j >= image->height()) {
                            // Spiegle v ins Positive indem vom maximalen Randpixel j abgezogen werden
                            vNew = image->height()-1-j;
                        }
                        color = copy->pixelColor(uNew, vNew);

                        // Hole Intensitätswert p und summiere gewichtet auf
                        double p = 0.299*color.red() + 0.587*color.green() + 0.114*color.blue(); // copy.pixelColor(u+i, v+j);
                        double c = laplace_filter[j+1][i+1];
                        sum = sum + c*p;
                    }
                }

                int q = (int) (sum+0.5); // Gewichtung einbeziehen und runden
                if (q < 0) q = 0; // Clamping
                if (q > 255) q = 255; // Clamping
                image->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
            }
        }
        return image;
    }

    // delete
    int getOrientationSector(int dX, int dY) {
        // Gibt den Orientation-Sektor s0 für den 2D Vektor (dX, dY) zurück
        int dPrimeX = 0; // d'x
        int dPrimeY = 0; // d'y

        double PI = 3.14159265358979323846;

        // Rotate (dX dY) by PI/8
        dPrimeX = std::cos(PI/8) * dX - std::sin(PI/8) * dY;
        dPrimeY = std::sin(PI/8) * dY + std::cos(PI/8) * dY;

        if (dPrimeY < 0) {
            // Mirror to octants 0, ..., 3
            dPrimeX = -dPrimeX;
            dPrimeY = -dPrimeY;
        }

        int s0 = 0;
        if (dPrimeX >= 0 && dPrimeX >= dPrimeY) {
            s0 = 0;
        } else if (dPrimeX >= 0 && dPrimeX < dPrimeY) {
            s0 = 1;
        } else if (dPrimeX < 0 && -dPrimeX < dPrimeY) {
            s0 = 2;
        } else if (dPrimeX < 0 && -dPrimeX >= dPrimeY) {
            s0 = 3;
        }

        // Sektor-Index s0 ∈ {0,1,2,3}
        return s0;
    }

    // delete
    bool isLocalMax(QImage* &Emag, int u, int v, int s0, int tLo) {
        // Gibt an, ob der Gradient ein lokales Maximum an der Position (u0, v0) in der Richtung s0 ∈ {0,1,2,3} ist

        // mC <- Emag(u,v)
        int mC = Emag->pixelColor(u,v).red();

        if (mC < tLo) {
            return false;
        } else {
            int mL = 0;
            int mR = 0;

            if (s0 == 0) {
                mL = Emag->pixelColor(u-1, v).red();
                if (u+1 >= Emag->width()) return false; // Wertezugriff außerhalb des Bildes verbieten
                mR = Emag->pixelColor(u+1, v).red();
            } else if (s0 == 1) {
                mL = Emag->pixelColor(u-1, v-1).red();
                if (u+1 >= Emag->width() || v+1 >= Emag->height()) return false; // Wertezugriff außerhalb des Bildes verbieten
                mR = Emag->pixelColor(u+1, v+1).red();
            } else if (s0 == 2) {
                mL = Emag->pixelColor(u, v-1).red();
                if (v+1 >= Emag->height()) return false; // Wertezugriff außerhalb des Bildes verbieten
                mR = Emag->pixelColor(u, v+1).red();
            } else if (s0 == 3) {
                if (v+1 >= Emag->height()) return false; // Wertezugriff außerhalb des Bildes verbieten
                mL = Emag->pixelColor(u-1, v+1).red();
                if (u+1 >= Emag->width()) return false; // Wertezugriff außerhalb des Bildes verbieten
                mR = Emag->pixelColor(u+1, v-1).red();
            }

            return (mL <= mC && mC >= mR);
        }
    }

    // delete
    void traceAndThreshold(QImage* &Enms, QImage* &Ebin, int u0, int v0, int tLo) {
        // Sammelt und markiert rekursiv alle Pixel einer Kante, die mit (u0, v0) verbunden sind und einen Gradienten über tLo haben

        // Markiere (u0, v0) als Kantenpixel
        Ebin->setPixelColor(u0, v0, QColor(255,255,255).rgb());

        // Limitiere auf Bild-Größe
        int uL = std::max(u0-1, 0);
        int uR = std::min(u0+1, Enms->width()-1);
        int vT = std::max(v0-1, 0);
        int vB = std::min(v0+1, Enms->height()-1);

        for (int u = uL; u <= uR; u++) {
            for (int v = vT; v <= vB; v++) {
                if (Enms->pixelColor(u,v).red() >= tLo && Ebin->pixelColor(u,v).red() == 0 ) {
                    traceAndThreshold(Enms, Ebin, u, v, tLo);
                }
            }
        }
    }


    /**
     * @brief doCanny
     *      calculate the Canny Edge Detector
     * @param img
     *      input image
     * @param sigma
     *      Canny Edge Detector parameter
     *      sigma for gauss smoothing
     * @param tHi
     *      Canny Edge Detector parameter
     *      Threshhold tHi
     * @param tLo
     *      Canny Edge Detector parameter
     *      Threshhold tLo
     * @return new Image to show in GUI
     */
    QImage* doCanny(QImage * & img, double sigma, int tHi, int tLo){
        logFile << "-----------\nBeginne Canny Algorithmus:\nSigma: " + std::to_string(sigma) + "\ntHi: " + std::to_string(tHi) + "\ntLo: " + std::to_string(tLo) << std::endl;

        delete img;
        img = new QImage(*backupImage);
        bool sigmaAndGradientsChanged = false;
        bool EnmsChanged = false;
        bool EbinChanged = false;

        QImage* copy = new QImage(*img);

        QImage* gradientX = new QImage(*img);
        QImage* gradientY = new QImage(*img);


        /******************
         * Bild glätten mit Gauß-Filter der Breite sigma | NUR WENN SIGMA SICH VERÄNDERT HAT
         *****************/
        if (cannyCacheSigma != sigma || imageGotChangedFlag == true) {
            cannyCacheSigma = sigma; // Neuen Sigma-Wert cachen
            logFile << "Sigma wurde gecached." << std::endl;

            // Gauss-Kernel h als 1D-Array erzeugen
            int center = (int) (3.0*sigma);
            float *h = new float[2*center+1]; // Ungerade Anzahl, Größe des Gauss-Filters

            // Gauss-Kernel füllen
            double sigma2 = sigma*sigma; // σ²
            int hLength = 2*center+1; // Größe des Gauss-Filters, wie oben 1D Array dieser Größe erstellt
            for (int i = 0; i < hLength; i++) { // Setzt Index-Werte in die Gauss-Funktion ein
                double r = center-i; // 0 in die Mitte des Arrays verschieben, um bei Auswertung der Gauss-Funktion um den Ursprung nicht in den negativen Indexbereich zu kommen
                h[i] = (float) std::exp(-0.5*(r*r)/sigma2); // Sampling (Auswertung) der Gauss-Funktion
                logFile << "h["+ std::to_string(i) +"]" + std::to_string(h[i]) << std::endl;
            }

            // Berechne Summe der Koeffizienten, um double s ermitteln zu können
            double sumOfCoefficients = 0.0;
            for (int i = 0; i < hLength; i++) {
                sumOfCoefficients += h[i];
            }

            // Ermittle double s als Gewichtungswert außerhalb der Matrix
            double s = 1.0/sumOfCoefficients; // Summe der Koeffizienten aus der Filtermatrix

            logFile << "sumOfCoefficients: " + std::to_string(sumOfCoefficients) << std::endl;
            logFile << "double s: " + std::to_string(s) << std::endl;


            // Sigma Gauß-Glättung über das Bild laufen lassen
                // Graubild

                // Gehe alle Pixel der einzelnen ZEILEN durch und wende den Filter an;
            for (int v = 0; v < img->height(); v++) {
                for (int u = 0; u < img->width(); u++) {
                    // Filter-Ergebnis der Position (u,v) berechnen
                    double sum = 0;
                    for (int i = -center; i <= center; i++) { // Hier nur noch eine For-Schleife

                        // Bestimmte Intensitätswert mit gespiegelter Randbedingung
                        QColor color;
                        int uNew = u+i;
                        if (u+i < 0) {
                            // Spiegle u ins Positive durch Multiplikation mit -1
                            uNew = (-1)*(u+i);
                        } else if (u+i >= img->width()) {
                            // Spiegle u ins Positive indem vom maximalen Randpixel i abgezogen werden
                            uNew = img->width()-1-i;
                        }
                        color = copy->pixelColor(uNew, v);

                        // Hole Intensitätswert p und summiere gewichtet auf
                        double p = 0.299*color.red() + 0.587*color.green() + 0.114*color.blue() ;
                        double c = h[i+center]; // Filter mit center verschieben, sodass auf h[0] zugegriffen werden kann
                        sum = sum + c*p;
                    }

                    int q = (int) (s * sum + 0.5); // Gewichtung einbeziehen und runden
                    if (q < 0) q = 0; // Clamping
                    if (q > 255) q = 255; // Clamping

                    img->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
                }
            }

            // Zeilen durchlaufen ist fertig. image hält nun das zeilenveränderte Bild
            delete copy;
            copy = new QImage(*img); // Neue Kopie des Bildes anlegen, um auf Werten des zuvor angewendeten Filters zu rechnen

            // Gehe alle Pixel der einzelnen SPALTEN durch und wende den Filter an;
            for (int v = 0; v < img->height(); v++) {
                for (int u = 0; u < img->width(); u++) {
                    // Filter-Ergebnis der Position (u,v) berechnen
                    double sum = 0;
                    for (int i = -center; i <= center; i++) { // Hier nur noch eine For-Schleife

                        // Bestimmte Intensitätswert je nach Randbedingungen
                        QColor color;
                        int vNew = v+i;
                        if (v+i < 0) {
                            // Spiegle u ins Positive durch Multiplikation mit -1
                            vNew = (-1)*(v+i);
                        } else if (v+i >= img->height()) {
                            // Spiegle u ins Positive indem vom maximalen Randpixel i abgezogen werden
                            vNew = img->height()-1-i;
                        }
                        color = copy->pixelColor(u, vNew);

                        // Hole Intensitätswert p und summiere gewichtet auf
                        int p = color.red(); // copy.pixelColor(u+i, v+j);
                        double c = h[i+center]; // Filter mit center verschieben, sodass auf h[0] zugegriffen werden kann
                        sum = sum + c*p;
                    }

                    int q = (int) (s * sum + 0.5); // Gewichtung einbeziehen und runden
                    if (q < 0) q = 0; // Clamping
                    if (q > 255) q = 255; // Clamping
                    img->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
                }
            }

            if(gaussBlurredImageCanny != nullptr){
                delete gaussBlurredImageCanny;
            }
            gaussBlurredImageCanny = new QImage(*img); // Neues blurred Image cachen
            logFile << "BlurredImage nach Gauss-Filter wurde gecached." << std::endl;


            /******************
             * Berechne x-Gradienten | NUR WENN SIGMA UND SOMIT DAS BLUR-IMAGE SICH VERÄNDERT HAT
             *****************/
            float* gradientFilter = new float[3];
            gradientFilter[0] = {-0.5};
            gradientFilter[1] = {0};
            gradientFilter[2] = {0.5};

            if (img->isGrayscale()) {
                // Graubild
                // Gradientberechnung für X-Richtung
                for (int v = 0; v <= img->height()-1; v++) { // v<=N-L-1 (Zentralbereich) bzw. v<=N-1 (andere Randbedingungen)
                    for (int u = 0; u <= img->width()-1; u++) { // v<=M-K-1 (Zentralbereich) bzw. v<=M-1 (andere Randbedingungen)
                        // Filter-Ergebnis der Position (u,v) berechnen
                        float sum = 0;
                        for (int i = -1; i <= 1; i++) {
                            // Gespiegelte Randbedingung
                            QColor color;
                            int uNew = u+i;
                            if (u+i < 0) {
                                // Spiegle u ins Positive durch Multiplikation mit -1
                                uNew = (-1)*(u+i);
                            } else if (u+i >= img->width()) {
                                // Spiegle u ins Positive indem vom maximalen Randpixel i abgezogen werden
                                uNew = img->width()-1-i;
                            }
                            color = gaussBlurredImageCanny->pixelColor(uNew, v);

                            // Hole Intensitätswert p und summiere gewichtet auf
                            int p = color.red(); // copy.pixelColor(u+i, v+j);
                            float c = gradientFilter[i+1]; // filter[j+L][i+K]; L und K sind halbe Filterbreite
                            sum = sum + c*p;
                        }

                        int q = (int) (sum + 0.5); // Gewichtung einbeziehen und runden
                        q += 127; // Wertebereich der Ableitung ist [-127, 127]
                        if (q < 0) q = 0; // Clamping
                        if (q > 255) q = 255; // Clamping
                        gradientX->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
                    }
                }
            }

            /******************
             * Berechne y-Gradienten | NUR WENN SIGMA UND SOMIT DAS BLUR-IMAGE SICH VERÄNDERT HAT
             *****************/
            if (img->isGrayscale()) {
                // Graubild
                // Gradientberechnung für Y-Richtung
                for (int v = 0; v <= img->height()-1; v++) {
                    for (int u = 0; u <= img->width()-1; u++) {
                        // Filter-Ergebnis der Position (u,v) berechnen
                        float sum = 0;
                        for (int j = -1; j <= 1; j++) {

                            // Gespiegelte Randbedingung
                            QColor color;
                            int vNew = v+j;
                            // Behandle u und v in getrennten if's, da es Fälle gibt, wo u und v eine Bearbeitung benötigen
                            if (v+j < 0) {
                                // Spiegle v ins Positive durch Multiplikation mit -1
                                vNew = (-1)*(v+j);
                            } else if (v+j >= img->height()) {
                                // Spiegle v ins Positive indem vom maximalen Randpixel j abgezogen werden
                                vNew = img->height()-1-j;
                            }
                            color = gaussBlurredImageCanny->pixelColor(u, vNew);

                            // Hole Intensitätswert p und summiere gewichtet auf
                            int p = color.red(); // copy.pixelColor(u+i, v+j);
                            float c = gradientFilter[j+1]; // filter[j+L][i+K]; L und K sind halbe Filterbreite
                            sum = sum + c*p;
                        }
                        int q = (int) (sum + 0.5); // Gewichtung einbeziehen und runden
                        q += 127; // Wertebereich der Ableitung ist [-127, 127]
                        if (q < 0) q = 0; // Clamping
                        if (q > 255) q = 255; // Clamping
                        gradientY->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
                    }
                }
            }

            sigmaAndGradientsChanged = true;

            if(cannyCacheGradientX != nullptr){
                delete cannyCacheGradientX;
            }
            cannyCacheGradientX = new QImage(*gradientX); // Neuen Gradient-X cachen

            if(cannyCacheGradientY != nullptr){
                delete cannyCacheGradientY;
            }
            cannyCacheGradientY = new QImage(*gradientY); // Neuen Gradient-Y cachen
            logFile << "gradientX und gradientY wurden gecached." << std::endl;

            delete [] h;
        } // end if (cannySigma != sigma)
        else {
            // Sigma ist noch wie vorher, setze img auf gecachtes Blurred-Image und aktualisiere Gradient X und Y
            delete img;
            img = new QImage(*gaussBlurredImageCanny);

            delete gradientX;
            gradientX = new QImage(*cannyCacheGradientX);
            delete gradientY;
            gradientY = new QImage(*cannyCacheGradientY);
            // Debug
            logFile << "Sigma hat sich nicht geändert. Benutze BlurredImage, gradientX und gradientY aus dem Cache." << std::endl;
        }



        /******************
         * Maps erstellen
         *****************/
        if(Emag != nullptr){
            delete Emag;
        }
        Emag = new QImage(*img); // Gradient magnitude


        if(Emag != nullptr){
            delete Enms;
        }
        Enms = new QImage(*img); // Maximum magnitude

        if(Ebin != nullptr){
            delete Ebin;
        }
        Ebin = new QImage(*img); // Binary Edge Pixels


        if (sigmaAndGradientsChanged == true || imageGotChangedFlag == true) {

            /******************
             * Bild durchlaufen | NUR WENN SIGMA UND SOMIT GRADIENTEN SICH GEÄNDERT HABEN
             *****************/
            for (int u = 0; u < img->width(); u++) {
                for (int v = 0; v < img->height(); v++) {
                    // Setze in Emag die Beträge der Gradienten
                    QColor colorX = gradientX->pixelColor(u, v);
                    QColor colorY = gradientY->pixelColor(u, v);

                    int colorXInt = colorX.red();
                    int colorYInt = colorY.red();

                    int gradientX = colorXInt - 127; // Wertebereich bei Lesen aus Ableitung wieder anpassen
                    int gradientY = colorYInt - 127; // Werdebereich bei Lesen aus Ableitung wieder anpassen

                    double powGradientX = std::pow(gradientX, 2);
                    double powGradientY = std::pow(gradientY, 2);
                    double powAddition = powGradientX + powGradientY;
                    double gradientenBetrag = std::sqrt(powAddition); // |∇I| -> Betrag des Gradienten (Vektors)

                    Emag->setPixel(u, v, QColor(gradientenBetrag, gradientenBetrag, gradientenBetrag).rgb());

                    // Setze in Enms auf 0
                    Enms->setPixel(u, v, 0);

                    // Setze in Ebin auf 0 (false)
                    Ebin->setPixel(u, v, 0);
                }
            }

            if(cannyCacheEmag != nullptr){
                delete cannyCacheEmag;
            }
            if(cannyCacheEnms != nullptr){
                delete cannyCacheEnms;
            }

            if(cannyCacheEbin != nullptr){
                delete cannyCacheEbin;
            }
            cannyCacheEmag = new QImage(*Emag);
            cannyCacheEnms = new QImage(*Enms);
            cannyCacheEbin = new QImage(*Ebin);

            logFile << "Emag, Enms, Ebin wurden gecached." << std::endl;
        } // end if (sigmaAndGradientsChanged == true)
        else {
            // Verwende Emag, Enms und Ebin aus dem Cache
            delete Emag;
            Emag = new QImage(*cannyCacheEmag);
            delete Enms;
            Enms = new QImage(*cannyCacheEnms);
            delete Ebin;
            Ebin = new QImage(*cannyCacheEbin);

            for (int u = 0; u < img->width(); u++) {
                for (int v = 0; v < img->height(); v++) {
                    // Setze in Enms auf 0
                    Enms->setPixel(u, v, 0);

                    // Setze in Ebin auf 0 (false)
                    Ebin->setPixel(u, v, 0);
                }
            }

            logFile << "Emag wurden aus dem Cache verwendet, Enms und Ebin mit 0 initialisiert." << std::endl;
        }


        if (sigmaAndGradientsChanged == true || cannyCacheTLo != tLo || imageGotChangedFlag == true) {
           /******************
            * Lokale Maxima finden und in Enms markieren | NUR WENN SIGMA UND SOMIT GRADIENTEN SICH GEÄNDERT HABEN ODER ES EINEN NEUEN TLO WERT GIBT
            *****************/
            for (int u = 1; u < img->width()-2; u++) {
                for (int v = 1; v < img->height(); v++) {
                    int dX = gradientX->pixelColor(u, v).red() -127;
                    int dY = gradientY->pixelColor(u, v).red() -127;
                    int s0 = getOrientationSector(dX, dY);
                    if ( isLocalMax(Emag, u, v, s0, tLo) ) {
                        Enms->setPixel(u, v, Emag->pixelColor(u, v).rgb()); // Nur lokale Maxima behalten
                    }
                }
            }
            cannyCacheTLo = tLo; // Neues tLo cachen
            delete cannyCacheEnms;
            cannyCacheEnms = new QImage(*Enms); // Neues Enms cachen

            EnmsChanged = true;
            EbinChanged = true;

            logFile << "tLo und Enms wurden gecached, weil sich Sigma/Gradienten oder tLo geändert haben." << std::endl;
        } // end if (sigmaAndGradientsChanged == true || cannyCacheTLo != tLo)
        else {
            delete Enms;
            Enms = new QImage(*cannyCacheEnms);

            tLo = cannyCacheTLo;

            logFile << "Sigma/Gradienten ODER tLo haben sich nicht geändert. Benutze Enms und tLo aus dem Cache." << std::endl;
        }

        if (cannyCacheTHi != tHi || EnmsChanged == true || EbinChanged == true || imageGotChangedFlag == true) {
            /******************
             * TraceAndThreshold | WENN tHI, Enms ODER Ebin SICH GEÄNDERT HABEN
             *****************/
            for (int u = 1; u < img->width()-2; u++) {
                for (int v = 1; v < img->height(); v++) {
                    if (Enms->pixelColor(u, v).red() >= tHi && Ebin->pixelColor(u, v).red() == 0) {
                        traceAndThreshold(Enms, Ebin, u, v, tLo);
                    }
                }
            }
            cannyCacheTHi = tHi; // Neuen tHi cachen
            delete cannyCacheEbin;
            cannyCacheEbin = new QImage(*Ebin); // Neuen Ebin cachen

            logFile << "tHi/Enms/Ebin hat sich geändert, berechne Ebin neu." << std::endl;
        } // end if
        else {
            delete Ebin;
            Ebin = new QImage(*cannyCacheEbin);

            logFile << "tHi hat sich nicht geändert. Ebin aus Cache verwenden." << std::endl;
        }

        logFile << "Werte des Canny:\nSigma: " + std::to_string(cannyCacheSigma) + "\ntHi: " + std::to_string(cannyCacheTHi) + "\ntLo: " + std::to_string(cannyCacheTLo) << std::endl;

        imageGotChangedFlag = false; // Zurücksetzen

        /******************
         * Speicher freigeben
         *****************/
        delete gradientX;
        delete gradientY;
        delete Emag; Emag = NULL;
        delete Enms; Enms = NULL;

        delete img;
        img = new QImage(*Ebin);
        delete Ebin; Ebin =nullptr;

        return img;

    }


    /**
     * @brief doUSM
     *      calculate the Unsharp Masking
     * @param image
     *      input image
     * @param sharpening_value
     *      Unsharp Masking parameter
     *      sharpening parameter (a)
     * @param sigma
     *      Unsharp Masking parameter
     *      sigma for gauss smoothing
     * @param tc
     *      Unsharp Masking parameter
     *      |∇I| must greater then threshhold tc
     * @return new Image to show in GUI
     */
    QImage* doUSM(QImage *& image, double sharpening_value, double sigma, int tc){

        // Gauss-Kernel h als 1D-Array erzeugen
        int center = (int) (3.0*sigma);
        float *h = new float[2*center+1]; // Ungerade Anzahl, Größe des Gauss-Filters

        // Gauss-Kernel füllen
        double sigma2 = sigma*sigma; // σ²
        int hLength = 2*center+1; // Größe des Gauss-Filters, wie oben 1D Array dieser Größe erstellt
        for (int i = 0; i < hLength; i++) { // Setzt Index-Werte in die Gauss-Funktion ein
            double r = center-i; // 0 in die Mitte des Arrays verschieben, um bei Auswertung der Gauss-Funktion um den Ursprung nicht in den negativen Indexbereich zu kommen
            h[i] = (float) std::exp(-0.5*(r*r)/sigma2); // Sampling (Auswertung) der Gauss-Funktion
            logFile << "h["+ std::to_string(i) +"]" + std::to_string(h[i]) << std::endl;
        }

        // Berechne Summe der Koeffizienten, um double s ermitteln zu können
        double sumOfCoefficients = 0.0;
        for (int i = 0; i < hLength; i++) {
            sumOfCoefficients += h[i];
        }

        // Ermittle double s als Gewichtungswert außerhalb der Matrix
        double s = 1.0/sumOfCoefficients; // Summe der Koeffizienten aus der Filtermatrix

        logFile << "sumOfCoefficients: " + std::to_string(sumOfCoefficients) << std::endl;
        logFile << "double s: " + std::to_string(s) << std::endl;

        QImage* copy = new QImage(*image);
        QImage* copy_blurred = new QImage(*image);
        QImage* usmGradientX = new QImage(*image);
        QImage* usmGradientY = new QImage(*image);
        QImage* usmGradientBetrag = new QImage(*image);

        float* gradientFilter = new float[3];
        gradientFilter[0] = {-0.5};
        gradientFilter[1] = {0};
        gradientFilter[2] = {0.5};

        if (image->isGrayscale()) {
            // Graubild
            // Gradientberechnung für X-Richtung
            for (int v = 0; v <= image->height()-1; v++) { // v<=N-L-1 (Zentralbereich) bzw. v<=N-1 (andere Randbedingungen)
                for (int u = 0; u <= image->width()-1; u++) { // v<=M-K-1 (Zentralbereich) bzw. v<=M-1 (andere Randbedingungen)
                    // Filter-Ergebnis der Position (u,v) berechnen
                    float sum = 0;
                    for (int i = -1; i <= 1; i++) {
                        // Gespiegelte Randbedingung
                        QColor color;
                        int uNew = u+i;
                        if (u+i < 0) {
                            // Spiegle u ins Positive durch Multiplikation mit -1
                            uNew = (-1)*(u+i);
                        } else if (u+i >= image->width()) {
                            // Spiegle u ins Positive indem vom maximalen Randpixel i abgezogen werden
                            uNew = image->width()-1-i;
                        }
                        color = copy->pixelColor(uNew, v);

                        // Hole Intensitätswert p und summiere gewichtet auf
                        int p = color.red(); // copy.pixelColor(u+i, v+j);
                        float c = gradientFilter[i+1]; // filter[j+L][i+K]; L und K sind halbe Filterbreite
                        sum = sum + c*p;
                    }

                    int q = (int) (sum + 0.5); // Gewichtung einbeziehen und runden
                    q += 127; // Wertebereich der Ableitung ist [-127, 127]
                    if (q < 0) q = 0; // Clamping
                    if (q > 255) q = 255; // Clamping
                    usmGradientX->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
                }
            }
            // Gradientberechnung für Y-Richtung
            for (int v = 0; v <= image->height()-1; v++) {
                for (int u = 0; u <= image->width()-1; u++) {
                    // Filter-Ergebnis der Position (u,v) berechnen
                    float sum = 0;
                    for (int j = -1; j <= 1; j++) {

                        // Gespiegelte Randbedingung
                        QColor color;
                        int vNew = v+j;
                        // Behandle u und v in getrennten if's, da es Fälle gibt, wo u und v eine Bearbeitung benötigen
                        if (v+j < 0) {
                            // Spiegle v ins Positive durch Multiplikation mit -1
                            vNew = (-1)*(v+j);
                        } else if (v+j >= image->height()) {
                            // Spiegle v ins Positive indem vom maximalen Randpixel j abgezogen werden
                            vNew = image->height()-1-j;
                        }
                        color = copy->pixelColor(u, vNew);

                        // Hole Intensitätswert p und summiere gewichtet auf
                        int p = color.red(); // copy.pixelColor(u+i, v+j);
                        float c = gradientFilter[j+1]; // filter[j+L][i+K]; L und K sind halbe Filterbreite
                        sum = sum + c*p;
                    }
                    int q = (int) (sum + 0.5); // Gewichtung einbeziehen und runden
                    q += 127; // Wertebereich der Ableitung ist [-127, 127]
                    if (q < 0) q = 0; // Clamping
                    if (q > 255) q = 255; // Clamping
                    usmGradientY->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
                }
            }
            // Gradientenbetrag ausrechnen
            for (int u = 0; u < image->width(); u++) {
                for (int v = 0; v < image->height(); v++) {
                    // Setze in Emag die Beträge der Gradienten
                    QColor colorX = usmGradientX->pixelColor(u, v);
                    QColor colorY = usmGradientY->pixelColor(u, v);

                    int colorXInt = colorX.red();
                    int colorYInt = colorY.red();

                    int gradientX = colorXInt - 127; // Wertebereich bei Lesen aus Ableitung wieder anpassen
                    int gradientY = colorYInt - 127; // Werdebereich bei Lesen aus Ableitung wieder anpassen

                    double powGradientX = std::pow(gradientX, 2);
                    double powGradientY = std::pow(gradientY, 2);
                    double powAddition = powGradientX + powGradientY;
                    double gradientenBetrag = std::sqrt(powAddition); // |∇I| -> Betrag des Gradienten (Vektors)

                    usmGradientBetrag->setPixel(u, v, QColor(gradientenBetrag, gradientenBetrag, gradientenBetrag).rgb());
                }
            }

            // Erstelle geglättete Version vom Original
            // Gehe alle Pixel der einzelnen ZEILEN durch und wende den Filter an;
            for (int v = 0; v < image->height(); v++) {
                for (int u = 0; u < image->width(); u++) {
                    // Filter-Ergebnis der Position (u,v) berechnen
                    double sum = 0;
                    for (int i = -center; i <= center; i++) { // Hier nur noch eine For-Schleife

                        // Bestimmte Intensitätswert mit gespiegelter Randbedingung
                        QColor color;
                        int uNew = u+i;
                        if (u+i < 0) {
                            // Spiegle u ins Positive durch Multiplikation mit -1
                            uNew = (-1)*(u+i);
                        } else if (u+i >= image->width()) {
                            // Spiegle u ins Positive indem vom maximalen Randpixel i abgezogen werden
                            uNew = image->width()-1-i;
                        }
                        color = copy->pixelColor(uNew, v);

                        // Hole Intensitätswert p und summiere gewichtet auf
                        int p = color.red(); // copy.pixelColor(u+i, v+j);
                        double c = h[i+center]; // Filter mit center verschieben, sodass auf h[0] zugegriffen werden kann
                        sum = sum + c*p;
                    }

                    int q = (int) (s * sum + 0.5); // Gewichtung einbeziehen und runden
                    if (q < 0) q = 0; // Clamping
                    if (q > 255) q = 255; // Clamping

                    copy_blurred->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
                }
            }

            // Zeilen durchlaufen ist fertig. image hält nun das zeilenveränderte Bild
            delete copy;
            copy = new QImage(*copy_blurred); // Neue Kopie des Bildes anlegen, um auf Werten des zuvor angewendeten Filters zu rechnen

            // Gehe alle Pixel der einzelnen SPALTEN durch und wende den Filter an;
            for (int v = 0; v < image->height(); v++) {
                for (int u = 0; u < image->width(); u++) {
                    // Filter-Ergebnis der Position (u,v) berechnen
                    double sum = 0;
                    for (int i = -center; i <= center; i++) { // Hier nur noch eine For-Schleife

                        // Bestimmte Intensitätswert je nach Randbedingungen
                        QColor color;
                        int vNew = v+i;
                        if (v+i < 0) {
                            // Spiegle u ins Positive durch Multiplikation mit -1
                            vNew = (-1)*(v+i);
                        } else if (v+i >= image->height()) {
                            // Spiegle u ins Positive indem vom maximalen Randpixel i abgezogen werden
                            vNew = image->height()-1-i;
                        }
                        color = copy->pixelColor(u, vNew);

                        // Hole Intensitätswert p und summiere gewichtet auf
                        int p = color.red(); // copy.pixelColor(u+i, v+j);
                        double c = h[i+center]; // Filter mit center verschieben, sodass auf h[0] zugegriffen werden kann
                        sum = sum + c*p;
                    }

                    int q = (int) (s * sum + 0.5); // Gewichtung einbeziehen und runden
                    if (q < 0) q = 0; // Clamping
                    if (q > 255) q = 255; // Clamping
                    copy_blurred->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
                }
            }

            // Geglättete Version des Bildes existiert nun in copy_blurred

            // Addition der gewichteten Maske zum Originalbild
            for (int u = 0; u < image->width(); u++) {
                for (int v = 0; v < image->height(); v++) {

                    // Maske erstellen: Subtraktion der geglätteten Version vom Originalbild
                    // M = I - I*Hblur
                    int intensityMask = 0;
                    int intensityOrig = 0;
                    int intensityBlur = 0;

                    QColor colorOrig = image->pixelColor(u, v);
                    QColor colorBlur = copy_blurred->pixelColor(u, v);

                    intensityOrig = colorOrig.red();
                    intensityBlur = colorBlur.red();

                    intensityMask = intensityOrig - intensityBlur;

                    // Lokalen Bildkontrast beachten: Schärfe nur für |∇I|(u,v) >= tc
                    int gradientBetragUV = usmGradientBetrag->pixelColor(u,v).red();
                    if (gradientBetragUV >= tc) {
                        int newIntensity = 0;
                        double a = sharpening_value; // Schärfungsgrad

                        newIntensity = intensityOrig + a*intensityMask;

                        // Clamping
                        if (newIntensity < 0) newIntensity = 0;
                        if (newIntensity > 255) newIntensity = 255;

                        image->setPixel(u, v, QColor(newIntensity, newIntensity, newIntensity).rgb());
                    }
                }
            }
        } else {
            // Farbbild
            // Gradientberechnung für X-Richtung
            for (int v = 0; v <= image->height()-1; v++) { // v<=N-L-1 (Zentralbereich) bzw. v<=N-1 (andere Randbedingungen)
                for (int u = 0; u <= image->width()-1; u++) { // v<=M-K-1 (Zentralbereich) bzw. v<=M-1 (andere Randbedingungen)
                    // Filter-Ergebnis der Position (u,v) berechnen
                    float sum = 0;
                    for (int i = -1; i <= 1; i++) {
                        // Gespiegelte Randbedingung
                        QColor color;
                        int uNew = u+i;
                        if (u+i < 0) {
                            // Spiegle u ins Positive durch Multiplikation mit -1
                            uNew = (-1)*(u+i);
                        } else if (u+i >= image->width()) {
                            // Spiegle u ins Positive indem vom maximalen Randpixel i abgezogen werden
                            uNew = image->width()-1-i;
                        }
                        color = copy->pixelColor(uNew, v);

                        // Hole Intensitätswert p und summiere gewichtet auf
                        double p = 0.299*color.red() + 0.587*color.green() + 0.114*color.blue(); // copy.pixelColor(u+i, v+j);
                        float c = gradientFilter[i+1]; // filter[j+L][i+K]; L und K sind halbe Filterbreite
                        sum = sum + c*p;
                    }

                    int q = (int) (sum + 0.5); // Gewichtung einbeziehen und runden
                    q += 127; // Wertebereich der Ableitung ist [-127, 127]
                    if (q < 0) q = 0; // Clamping
                    if (q > 255) q = 255; // Clamping
                    usmGradientX->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
                }
            }
            // Gradientberechnung für Y-Richtung
            for (int v = 0; v <= image->height()-1; v++) {
                for (int u = 0; u <= image->width()-1; u++) {
                    // Filter-Ergebnis der Position (u,v) berechnen
                    float sum = 0;
                    for (int j = -1; j <= 1; j++) {

                        // Gespiegelte Randbedingung
                        QColor color;
                        int vNew = v+j;
                        // Behandle u und v in getrennten if's, da es Fälle gibt, wo u und v eine Bearbeitung benötigen
                        if (v+j < 0) {
                            // Spiegle v ins Positive durch Multiplikation mit -1
                            vNew = (-1)*(v+j);
                        } else if (v+j >= image->height()) {
                            // Spiegle v ins Positive indem vom maximalen Randpixel j abgezogen werden
                            vNew = image->height()-1-j;
                        }
                        color = copy->pixelColor(u, vNew);

                        // Hole Intensitätswert p und summiere gewichtet auf
                        double p = 0.299*color.red() + 0.587*color.green() + 0.114*color.blue(); // copy.pixelColor(u+i, v+j);
                        float c = gradientFilter[j+1]; // filter[j+L][i+K]; L und K sind halbe Filterbreite
                        sum = sum + c*p;
                    }
                    int q = (int) (sum + 0.5); // Gewichtung einbeziehen und runden
                    q += 127; // Wertebereich der Ableitung ist [-127, 127]
                    if (q < 0) q = 0; // Clamping
                    if (q > 255) q = 255; // Clamping
                    usmGradientY->setPixel(u, v, QColor(q, q, q).rgb()); // image->setPixel(u, v, q);
                }
            }
            // Gradientenbetrag ausrechnen
            for (int u = 0; u < image->width(); u++) {
                for (int v = 0; v < image->height(); v++) {
                    // Setze in Emag die Beträge der Gradienten
                    QColor colorX = usmGradientX->pixelColor(u, v);
                    QColor colorY = usmGradientY->pixelColor(u, v);

                    int colorXInt = colorX.red();
                    int colorYInt = colorY.red();

                    int gradientX = colorXInt - 127; // Wertebereich bei Lesen aus Ableitung wieder anpassen
                    int gradientY = colorYInt - 127; // Werdebereich bei Lesen aus Ableitung wieder anpassen

                    double powGradientX = std::pow(gradientX, 2);
                    double powGradientY = std::pow(gradientY, 2);
                    double powAddition = powGradientX + powGradientY;
                    double gradientenBetrag = std::sqrt(powAddition); // |∇I| -> Betrag des Gradienten (Vektors)

                    usmGradientBetrag->setPixel(u, v, QColor(gradientenBetrag, gradientenBetrag, gradientenBetrag).rgb());
                }
            }

            // Erstelle geglättete Version vom Original
            // Gehe alle Pixel der einzelnen ZEILEN durch und wende den Filter an;
            for (int v = 0; v < image->height(); v++) {
                for (int u = 0; u < image->width(); u++) {

                    // Wert des mittleren Pixel für spätere Berechnung sichern
                    QColor colorMiddlePixel = image->pixelColor(u,v);
                    double Cb = -0.169*colorMiddlePixel.red() - 0.331*colorMiddlePixel.green() + 0.5*colorMiddlePixel.blue() +128;
                    double Cr = 0.5*colorMiddlePixel.red() - 0.419*colorMiddlePixel.green() - 0.08*colorMiddlePixel.blue() +128;

                    // Filter-Ergebnis der Position (u,v) berechnen
                    double sum = 0;
                    for (int i = -center; i <= center; i++) { // Hier nur noch eine For-Schleife

                        // Bestimmte Intensitätswert mit gespiegelter Randbedingung
                        QColor color;
                        int uNew = u+i;
                        if (u+i < 0) {
                            // Spiegle u ins Positive durch Multiplikation mit -1
                            uNew = (-1)*(u+i);
                        } else if (u+i >= image->width()) {
                            // Spiegle u ins Positive indem vom maximalen Randpixel i abgezogen werden
                            uNew = image->width()-1-i;
                        }
                        color = copy->pixelColor(uNew, v);

                        // Hole Intensitätswert p und summiere gewichtet auf
                        double p = 0.299*color.red() + 0.587*color.green() + 0.114*color.blue(); // copy.pixelColor(u+i, v+j);
                        double c = h[i+center]; // Filter mit center verschieben, sodass auf h[0] zugegriffen werden kann
                        sum = sum + c*p;
                    }

                    double q = (int) (s * sum); // Gewichtung einbeziehen und runden
                    if (q < 0) q = 0; // Clamping
                    if (q > 255) q = 255; // Clamping

                    // YCbCr to RGB
                    double CbM = Cb-128;
                    double CrM = Cr-128;
                    int red = 1*q - 0.000926744840399*CbM + 1.401686760243916*CrM + 0.5; // Beziehe neue, gemischte Helligkeit q mit ein
                    int green = 1*q - 0.343695384472312*CbM - 0.714169039951589*CrM + 0.5; // Beziehe neue, gemischte Helligkeit q mit ein
                    int blue = 1*q + 1.772160415723348*CbM + 0.000990220514491*CrM + 0.5; // Beziehe neue, gemischte Helligkeit q mit ein

                    // Clamping
                    if (red<0) {red=0;}
                    if (red>255) {red=255;}
                    if (green<0) {green=0;}
                    if (green>255) {green=255;}
                    if (blue<0) {blue=0;}
                    if (blue>255) {blue=255;}

                    copy_blurred->setPixel(u, v, QColor(red, green, blue).rgb()); // image->setPixel(u, v, q);
                }
            }

            // Zeilen durchlaufen ist fertig. image hält nun das zeilenveränderte Bild
            delete copy;
            copy = new QImage(*copy_blurred); // Neue Kopie des Bildes anlegen, um auf Werten des zuvor angewendeten Filters zu rechnen

            // Gehe alle Pixel der einzelnen SPALTEN durch und wende den Filter an;
            for (int v = 0; v < image->height(); v++) {
                for (int u = 0; u < image->width(); u++) {

                    // Wert des mittleren Pixel für spätere Berechnung sichern
                    QColor colorMiddlePixel = image->pixelColor(u,v);
                    double Cb = -0.169*colorMiddlePixel.red() - 0.331*colorMiddlePixel.green() + 0.5*colorMiddlePixel.blue() +128;
                    double Cr = 0.5*colorMiddlePixel.red() - 0.419*colorMiddlePixel.green() - 0.08*colorMiddlePixel.blue() +128;

                    // Filter-Ergebnis der Position (u,v) berechnen
                    double sum = 0;
                    for (int i = -center; i <= center; i++) { // Hier nur noch eine For-Schleife

                        // Bestimmte Intensitätswert je nach Randbedingungen
                        QColor color;
                        int vNew = v+i;
                        if (v+i < 0) {
                            // Spiegle u ins Positive durch Multiplikation mit -1
                            vNew = (-1)*(v+i);
                        } else if (v+i >= image->height()) {
                            // Spiegle u ins Positive indem vom maximalen Randpixel i abgezogen werden
                            vNew = image->height()-1-i;
                        }
                        color = copy->pixelColor(u, vNew);

                        // Hole Intensitätswert p und summiere gewichtet auf
                        double p = 0.299*color.red() + 0.587*color.green() + 0.114*color.blue(); // copy.pixelColor(u+i, v+j);
                        double c = h[i+center]; // Filter mit center verschieben, sodass auf h[0] zugegriffen werden kann
                        sum = sum + c*p;
                    }

                    double q = (int) (s * sum + 0.5); // Gewichtung einbeziehen und runden
                    if (q < 0) q = 0; // Clamping
                    if (q > 255) q = 255; // Clamping

                    // YCbCr to RGB
                    double CbM = Cb-128;
                    double CrM = Cr-128;
                    int red = 1*q - 0.000926744840399*CbM + 1.401686760243916*CrM +0.5; // Beziehe neue, gemischte Helligkeit q mit ein
                    int green = 1*q - 0.343695384472312*CbM - 0.714169039951589*CrM + 0.5; // Beziehe neue, gemischte Helligkeit q mit ein
                    int blue = 1*q + 1.772160415723348*CbM + 0.000990220514491*CrM + 0.5; // Beziehe neue, gemischte Helligkeit q mit ein

                    // Clamping
                    if (red<0) {red=0;}
                    if (red>255) {red=255;}
                    if (green<0) {green=0;}
                    if (green>255) {green=255;}
                    if (blue<0) {blue=0;}
                    if (blue>255) {blue=255;}

                    copy_blurred->setPixel(u, v, QColor(red, green, blue).rgb()); // image->setPixel(u, v, q);
                }
            }

            // Geglättete Version des Bildes existiert nun in copy_blurred

            // Addition der gewichteten Maske zum Originalbild
            for (int u = 0; u < image->width(); u++) {
                for (int v = 0; v < image->height(); v++) {

                    // Maske erstellen: Subtraktion der geglätteten Version vom Originalbild
                    // M = I - I*Hblur
                    double intensityMask = 0;
                    double intensityOrig = 0;
                    double intensityBlur = 0;

                    QColor colorOrig = image->pixelColor(u, v);
                    QColor colorBlur = copy_blurred->pixelColor(u, v);

                    intensityOrig = 0.299*colorOrig.red() + 0.587*colorOrig.green() + 0.114*colorOrig.blue();
                    intensityBlur = 0.299*colorBlur.red() + 0.587*colorBlur.green() + 0.114*colorBlur.blue();

                    intensityMask = intensityOrig - intensityBlur;

                    // Lokalen Bildkontrast beachten: Schärfe nur für |∇I|(u,v) >= tc
                    int gradientBetragUV = usmGradientBetrag->pixelColor(u,v).red();
                    if (gradientBetragUV >= tc) {
                        double newIntensity = 0;
                        double a = sharpening_value; // Schärfungsgrad

                        // Wert des mittleren Pixel für spätere Berechnung sichern
                        double Cb = -0.169*colorOrig.red() - 0.331*colorOrig.green() + 0.5*colorOrig.blue() +128;
                        double Cr = 0.5*colorOrig.red() - 0.419*colorOrig.green() - 0.08*colorOrig.blue() +128;

                        // Maske gewichtet anwenden
                        newIntensity = intensityOrig + a*intensityMask;

                        // Clamping
                        if (newIntensity < 0) newIntensity = 0;
                        if (newIntensity > 255) newIntensity = 255;

                        // YCbCr to RGB
                        double CbM = Cb-128;
                        double CrM = Cr-128;
                        int red = 1*newIntensity - 0.000926744840399*CbM + 1.401686760243916*CrM +0.5; // Beziehe neue, gemischte Helligkeit q mit ein
                        int green = 1*newIntensity - 0.343695384472312*CbM - 0.714169039951589*CrM +0.5; // Beziehe neue, gemischte Helligkeit q mit ein
                        int blue = 1*newIntensity + 1.772160415723348*CbM + 0.000990220514491*CrM +0.5; // Beziehe neue, gemischte Helligkeit q mit ein

                        // Clamping
                        if (red<0) {red=0;}
                        if (red>255) {red=255;}
                        if (green<0) {green=0;}
                        if (green>255) {green=255;}
                        if (blue<0) {blue=0;}
                        if (blue>255) {blue=255;}

                        image->setPixel(u, v, QColor(red, green, blue).rgb());
                    }
                }
            }

        }

        logFile << "Unsharp Masking ausgeführt mit Schärfungsgrad " << sharpening_value << ", Sigma " << sigma << " und tc " << tc << std::endl;

        delete copy;
        delete copy_blurred;
        delete usmGradientX;
        delete usmGradientY;
        delete usmGradientBetrag;
        delete [] h;

        return image;
    }


}


