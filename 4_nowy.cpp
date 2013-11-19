#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>
#include <string>
#include <list>

/*
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
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>*/
//#include <algorithm>
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
	
	float huHand[3];

// pole konturu temp
	double area = 0 ;
	//bounding_rect
	int largest_area = 0;
	int largest_contour_index=0;
// erosion element
			//Mat element = getStructuringElement( MORPH_RECT,
			//	Size( 5, 5 ),
			//	Point( 2, 2 ) );
	IplConvKernel*	kernel = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_RECT, NULL);


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
	cvNamedWindow("result", CV_WINDOW_AUTOSIZE);
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
	int i = 0;
	int MAXIMAGE = 10;
/*

// background averaging
	IplImage * image = cvCreateImage(cvGetSize(img),8,3);
       // we should create 32 bits of image buffer
	IplImage * buffer = cvCreateImage(cvGetSize(image),32,3);
    cvZero(buffer);    // clear data in buffer
       // result image buffer
    IplImage * result = cvCreateImage(cvGetSize(image),8,3);
    cvZero(result);    // clear data in result 
    IplImage * temp = cvCreateImage(cvGetSize(image),32,3);  
*/
// miało to robić taką magię, że brać 10klatek i je uśredniać.. i to chyba robi :P 
// ale nie potrafię odjąć od siebie obrazu z kamerki z tym wzorcem który jest obliczany :(

/*
    for(i=0;i< MAXIMAGE;i++){
         cvZero(image);           // clear image
         image=cvQueryFrame(capture);   // custom function for get image from camera

         cvSmooth(image, image, CV_MEDIAN, 5, 5);
         cvScale(image,temp,1.0);  // must change 8 bit image to 32 bit image before do cvAdd

         cvAdd(image,buffer,buffer); // add image and buffer then keep in buffer
     }
       // then divide summarized image with image number
       cvScale(buffer,result,1.0/MAXIMAGE*1.0); // must include 1.0 for floating point operation
       // now you can use noise reduce image in result buffer
       	rimg = cvQueryFrame(capture);
       	cvSmooth(rimg, rimg, CV_MEDIAN, 5, 5);
        cvSub(rimg,image, result);

       cvShowImage("result", result);
       getchar();
*/

       while (1) {
       	cvSet(kopia, cvScalar(0,0,0));
       	rimg = cvQueryFrame(capture);
 //	cvAbsDiff(rimg, img, rimg);

       	cvSetImageROI(rimg,cvRect(x,y,width,height));
       	cvInRangeS(rimg,cvScalar(h1,s1,v1),cvScalar(h2,s2,v2),thresh);  
       	cvResetImageROI( rimg );

	//cvInRangeS(img,cvScalar(h1,s1,v1),cvScalar(h2,s2,v2),hsvimg);   

       	cvSmooth(thresh, thresh, CV_MEDIAN, 7, 7);
	//cvCanny(thresh, kopia, 10, 20, 3);
       	cvErode(thresh,thresh,kernel,1);
       	cvDilate(thresh,thresh,kernel,1);
//cv::erode( thresh, thresh, element );

       	cvSetImageROI(rimg,cvRect(x,y,width,height));
//cvCopy( img, kopia );
       	cvResetImageROI( rimg );

       	cvCopy(thresh, kopia2);

/*	

       	CvMemStorage* storage = cvCreateMemStorage(0);
       	CvSeq* contour = NULL;
       	CvSeq* contourMax = contour;
       	CvSeq* contour_temp = NULL;
       //	contour2 = 0;
	// finding contour MAGIC! ^^
       	cvFindContours(kopia2, storage, &contour, sizeof(CvContour),
       		CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);


	//contour = 0;
  //     	printf("cokolwiek\n");
       	for( contour_temp=contour; contour_temp != 0; contour_temp = contour_temp->h_next) // iterate through each contour. 
       	{
       		
     //  area=cvContourArea( contour,CV_WHOLE_SEQ, false);  //  Find the area of contour
     //  if(area>largest_area){
     //  	largest_area=area;
     //  	largest_contour_index=i;
       	contourMax = contour_temp;
///////////////////////////////////////////////////////////////////////////
// tu jest problem !
// jak skopiować kontur? 
// trzeba to jakoś zainicjalizować?
// zaalokować pamięc?? 
// chcę zapamiętać tylko wskaźnik na strukturę tego konkretnego konturu
// teraz &contour2 = - duża liczba
///////////////////////////////////////////////////////////////////////////////       	
  //     printf("cokolwiek %d %d \n", &contour2, &contour2->h_next);
  //     	contour2 = contour;  


              //Store the index of largest contour
  //     bounding_rect=boundingRect(contour); // Find the bounding rectangle for biggest contour
     //   }
*/
       	static CvHuMoments* huMoments = new CvHuMoments();	
      CvMemStorage * storage = cvCreateMemStorage(0);
      CvSeq * first = NULL;
      CvSeq * contour = NULL;
      cvFindContours(kopia2, storage, &first, sizeof(CvContour),CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
      CvSeq * max = first;
      CvSeq * hull = first;
      int maxArea = 0;
      for(contour = first; contour != 0; contour = contour->h_next)
	{
	  CvRect bound = cvBoundingRect(contour,0);
	  if(maxArea < bound.width * bound.height)
	    {
	      max = contour;
	      maxArea = bound.width * bound.height;
	    }
	}
      for(contour = first; contour != 0; contour = contour->h_next)
	if(contour != max)
	printf("");
	  //cvDrawContours(kopia,contour,CV_RGB(0,255,255),CV_RGB(0,255,255),CV_FILLED);
	else
	  {
	    CvScalar color(CV_RGB(255,0,0));
	    CvRect bound = cvBoundingRect(contour,0);
	    cvDrawContours(kopia,contour,color,color,CV_FILLED);
	    cvRectangle(kopia,cvPoint(bound.x,bound.y),cvPoint(bound.x+bound.width,bound.y+bound.height),color,1);
	  }

  // }

   Scalar color( 255,255,255);
 //cvDrawContours(kopia, contour2,0, colorB, colorB, CV_FILLED, ); // Draw the largest contour using previously stored index.
 //cvRectangle(src, bounding_rect,  Scalar(0,255,0),1, 8,0);  
 //  cvDrawContours(kopia, contourMax, colorB,colorB, 0, 1, 8 ); 
//printf("%d\n", &contourMax);



// convex defects i hull ! 

   static CvMoments* moments = new CvMoments();
 //printf("cokolwiek %d %d \n", &contour2, &contour2->h_next);
  for(contour = max; contour != 0; contour = contour->h_next)
  { cvMoments(contour, moments);

   cvGetHuMoments(moments, huMoments);

 	}


 	//printf("test\n");

// defects hull, convex - nie działa :(

//hull = cvConvexHull2(max, storage, CV_CLOCKWISE, 0 );
//cvDrawContours(kopia,hull,CV_RGB(0,255,255),CV_RGB(0,255,255),CV_FILLED);
// printf("test2\n");




 		//printf("hu1: %f\n", huMoments->hu1); 
  // CvRect r = cvBoundingRect(contourMax, 1);
//   cvDrawContours(kopia, contourMax, colorB, colorB, CV_FILLED);
/*
   if ((cvWaitKey(10) & 255) == 32) { 
   	huHand[0]= huMoments->hu1;
   	printf("huHand[0]: %f\n", huHand[0]);
   	huHand[1]= huMoments->hu2;
   	printf("huHand[1]: %f\n", huHand[1]);
   	huHand[2]= huMoments->hu3;
   	printf("huHand[2]: %f\n", huHand[2]);



   }*/








//Tu kod liczący momenty i porównujący ze wzorcem


 printf("hu1: %f\n", huMoments->hu1);
 printf("hu2: %f\n", huMoments->hu2);
 printf("hu3: %f\n\n", huMoments->hu3);


   cvReleaseMemStorage(&storage);


	//displaying images
   cvShowImage("Original Image",rimg);
   cvShowImage("Color Image",kopia2);
   cvShowImage("Thresholded Image",thresh);
   cvShowImage("afterEffects", kopia);

	//Stop the clock and show FPS
   time(&end);
   ++counter;
   sec=difftime(end,start);
   fps=counter/sec;
	//printf("\n%lf",fps);

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
