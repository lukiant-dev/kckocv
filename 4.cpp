
#include <opencv/highgui.h>
#include <iostream>
#include <string>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
 #include<opencv2/opencv.hpp>

#include<vector>
#include<algorithm>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define OBJB1 0.1665 // pierwszy
#define OBJB2 0.0004 // drugi
#define OBJB3 0.0001 // trzeci moment Hu dla kwadratu
 
using namespace std;
 
int main(int argc, const char* argv[]) {
 
    CvCapture* capture = cvCaptureFromCAM(0);
// std::cout << "jest okej!" << endl;
    cvNamedWindow("afterEffects", CV_WINDOW_AUTOSIZE);
 
    CvScalar colorB = CV_RGB( 0, 255, 0 );
 
    int minimalnaWielkosc = 10000, pole = 0;
    float iloraz = 1;
 
    while (1) {
 
        IplImage* frame = cvQueryFrame(capture);
 
     //   frame = ContrastBrightness(frame, 70, 40);
 
        minimalnaWielkosc = (640 / 12) * (480 / 12);

        IplImage *kopia = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
 
        cvFlip(frame, frame, 2);
 
        cvCvtColor(frame, kopia, CV_RGB2GRAY);
 
        cvSmooth(kopia, kopia, CV_GAUSSIAN, 11, 11, 2, 2);
        cvCanny(kopia, kopia, 10, 60, 3);
 
        CvMemStorage* storage = cvCreateMemStorage(0);
        CvSeq* contour = 0;


        cvFindContours(kopia, storage, &contour, sizeof(CvContour),
                CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
 
        bool elem1 = 1;
        int rX1 = 0, rY1 = 0;
 // std::cout << "jest okej!" << endl;
        for (int i = 0; contour != 0; contour = contour->h_next, i++) {
 
            static CvMoments* moments = new CvMoments();
            cvMoments(contour, moments);
            static CvHuMoments* huMoments = new CvHuMoments();
            cvGetHuMoments(moments, huMoments);
 // std::cout << i << endl;
            CvRect r = cvBoundingRect(contour, 1);
 
            iloraz = (float) (abs(r.width - r.height))
                    / (float) max(r.width, r.height);
 
            pole = r.width * r.height;
 
            if ((pole > minimalnaWielkosc) && (iloraz < 0.4)
                    && (abs(huMoments->hu1 - OBJB1) < 0.5)
                    && (abs(huMoments->hu2 - OBJB2) < 0.005)) { // kwadrat
  //std::cout << "jest okej!" << endl;
                cvDrawContours(frame, contour, colorB, colorB, CV_FILLED);
 std::cout << "win!!"<< endl;
                if (elem1 == 0) {
 
                    elem1 = 1;
                    rX1 = r.x;
                    rY1 = r.y;
 
                }
 
                if ((elem1 == 1)
                        && ((abs(rY1 - r.y) > 50) || ((abs(rX1 - r.x) > 50)))) {
 
                    double a = ((double) (rY1 - r.y)) / ((double) (rX1 - r.x));
                    a = atan(a);
                    std::cout << a * 180 / 3.14 << endl;
 		//	std::cout << "win!!"<< endl;
               //     sendAngle(a*1.3); // zaimplementuj funkcje wysyłającą wartość do jakiegoś urządzenia/pliku/po sieci/...
 
                    elem1 = 0;
 
                }
 
            }
 
            cvReleaseMemStorage(&storage);
 
        }
 
        cvShowImage("afterEffects", kopia);

      //  cvReleaseImage(&kopia);
      //  cvReleaseImage(&frame);
//  cout << "co jest 5" << endl;
        if ((cvWaitKey(10) & 255) == 27) { // Wciśnięcie ESC kończy działanie programu
   cvReleaseImage(&kopia);
       cvReleaseImage(&frame);
cvReleaseCapture(&capture);
            break;
 
        }
     
    }
 //cvReleaseCapture(&capture);

    cvDestroyWindow("mywindow");
    cvDestroyWindow("afrerEffects");
 
    return 0;
}
