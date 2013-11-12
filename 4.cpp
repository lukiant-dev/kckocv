
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
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
 #include <opencv2/opencv.hpp>

#include<vector>
#include<algorithm>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <time.h>
#define OBJB1 0.1665 // pierwszy
#define OBJB2 0.0004 // drugi
#define OBJB3 0.0001 // trzeci moment Hu dla kwadratu
int x = 360;
int y = 160;
int width = 300;
int height = 300;

using namespace std;
using namespace cv;

int main(int argc, const char* argv[]) {

//variables for fps counting
time_t start, end;
double sec,fps;

// erosion element
Mat element = getStructuringElement( MORPH_RECT,
                                       Size( 5, 5 ),
                                       Point( 2, 2 ) );
IplConvKernel*	kernel;
kernel = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_RECT, NULL);


//creating capture image
CvCapture* capture = cvCaptureFromCAM(0);
IplImage* img = cvQueryFrame(capture);

// Setting ROI for smaller image
cvSetImageROI(img,cvRect(x,y,width,height));
IplImage *kopia2 = cvCreateImage(cvGetSize(img), 8, 1);


// other images declarations				1 means greyscale 
IplImage* rimg		= cvCreateImage(cvGetSize(img),8,3);
IplImage* hsvimg	= cvCreateImage(cvGetSize(img),8,3);
IplImage* thresh	= cvCreateImage(cvGetSize(img),8,1);
IplImage* kopia 	= cvCreateImage(cvGetSize(img),8,3);
cvResetImageROI( img );

//Windows for displaying images and trackbars
cvNamedWindow("Original Image",CV_WINDOW_AUTOSIZE);
cvNamedWindow("Color Image",CV_WINDOW_AUTOSIZE);
cvNamedWindow("Thresholded Image",CV_WINDOW_AUTOSIZE);
cvNamedWindow("cnt",CV_WINDOW_AUTOSIZE);
cvNamedWindow("afterEffects", CV_WINDOW_AUTOSIZE);

//Variables for trackbar
int h1=0;int s1=0;int v1=0;
int h2=0;int s2=0;int v2=0;
//Creating the trackbars
cvCreateTrackbar("H1","cnt",&h1,255,0);
cvCreateTrackbar("H2","cnt",&h2,255,0);
cvCreateTrackbar("S1","cnt",&s1,255,0);
cvCreateTrackbar("S2","cnt",&s2,255,0);
cvCreateTrackbar("V1","cnt",&v1,255,0);
cvCreateTrackbar("V2","cnt",&v2,255,0);

    //whats that???? contour finding?
 CvScalar colorB = CV_RGB( 0, 255, 0 );
 // ??? useless?
int minimalnaWielkosc = 10000, pole = 0;
float iloraz = 1;

//fps start
time(&start);
int counter=0;


while (1) {
 
	img = cvQueryFrame(capture);
 
     
 	cvSetImageROI(img,cvRect(x,y,width,height));
	cvInRangeS(img,cvScalar(h1,s1,v1),cvScalar(h2,s2,v2),thresh);  
	cvResetImageROI( img );
	
	//cvInRangeS(img,cvScalar(h1,s1,v1),cvScalar(h2,s2,v2),hsvimg);   
 
        cvSmooth(thresh, thresh, CV_GAUSSIAN, 15, 15, 2, 2);
	//cvCanny(thresh, kopia, 10, 20, 3);
 	//cvErode(kopia,kopia,kernel,1);
//	erode( kopia, kopia, element );

cvSetImageROI(img,cvRect(x,y,width,height));
//cvCopy( img, kopia );
cvResetImageROI( img );

cvCopy(thresh, kopia2);
/*	for(int rows = y; rows < height; rows++) {
    		for(int cols = x; rows < width; cols++) {        
        		kopia2->imageData[(rows-y)*kopia2->width + (cols-x)] = img->imageData[int(rows*img->width + cols)];
    		}
	}
*/        CvMemStorage* storage = cvCreateMemStorage(0);
        CvSeq* contour = 0;
	// finding contour MAGIC! ^^
        cvFindContours(kopia2, storage, &contour, sizeof(CvContour),
                CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
 
        bool elem1 = 1;
        int rX1 = 0, rY1 = 0;
 	
	//contour = 0;
        for (int i = 0; contour != 0; contour = contour->h_next, i++) {
 		cvDrawContours(kopia, contour, colorB, colorB, CV_FILLED);
            static CvMoments* moments = new CvMoments();
            cvMoments(contour, moments);
            static CvHuMoments* huMoments = new CvHuMoments();
            cvGetHuMoments(moments, huMoments);
 		//printf("hu1: %f\n", huMoments->hu1); 
            CvRect r = cvBoundingRect(contour, 1);
 
            iloraz = (float) (abs(r.width - r.height))
                    / (float) max(r.width, r.height);
 
            pole = r.width * r.height;
 
            if ((pole > minimalnaWielkosc) && (iloraz < 0.4)
                    && (abs(huMoments->hu1 - OBJB1) < 0.5)
                    && (abs(huMoments->hu2 - OBJB2) < 0.005)) { // kwadrat
  		printf("kwadrat!!!"); 
                cvDrawContours(kopia, contour, colorB, colorB, CV_FILLED);
 
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
 		//??????????????
               //     sendAngle(a*1.3); // zaimplementuj funkcje wysyłającą wartość do jakiegoś urządzenia/pliku/po sieci/...
 
                    elem1 = 0;
 
                }
 
            }
 
            cvReleaseMemStorage(&storage);
 
        }
	//displaying images
 	cvShowImage("Original Image",img);
	cvShowImage("Color Image",kopia2);
	cvShowImage("Thresholded Image",thresh);
        cvShowImage("afterEffects", kopia);

	//Stop the clock and show FPS
	time(&end);
	++counter;
	sec=difftime(end,start);
	fps=counter/sec;
//	printf("\n%lf",fps);
      
	// ESC key ends program
        if ((cvWaitKey(10) & 255) == 27) { 
 		/*  cvReleaseImage(&kopia);
     
		cvReleaseCapture(&capture);
		cvReleaseImage(&img);
		cvReleaseImage(&thresh);
		cvReleaseImage(&rimg);
		cvReleaseImage(&hsvimg);*/
                break;
 	}
     
} //while end


	
cvDestroyWindow("mywindow");
cvDestroyWindow("afrerEffects");
 
    return 0;
}
