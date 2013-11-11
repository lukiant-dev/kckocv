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
#include <opencv2/imgproc/imgproc.hpp>
#include<vector>
#include<algorithm>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <time.h>
#define h 280
#define w 320
int main()
{
//Image variables
CvCapture* capture = cvCaptureFromCAM(0);
//IplImage* img=cvLoadImage("F:\\t4.jpg");
IplImage* img = cvQueryFrame(capture);

IplImage* rimg=cvCreateImage(cvGetSize(img),8,3);
IplImage* hsvimg=cvCreateImage(cvGetSize(img),8,3);
IplImage* thresh=cvCreateImage(cvGetSize(img),8,1);

//Windows
cvNamedWindow("Original Image",CV_WINDOW_AUTOSIZE);
cvNamedWindow("Thresholded Image",CV_WINDOW_AUTOSIZE);
cvNamedWindow("cnt",CV_WINDOW_AUTOSIZE);
 
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
 
//Resizing the image
//cvResize(img,rimg,CV_INTER_LINEAR);
//Changing into HSV plane
cvCvtColor(rimg,hsvimg,CV_BGR2HSV);
while(1)
{     
 rimg = cvQueryFrame(capture);
//img = cvQueryFrame(capture);
//Thresholding the image


//Showing the images

     cvInRangeS(rimg,cvScalar(h1,s1,v1),cvScalar(h2,s2,v2),thresh);  
 
cvShowImage("Original Image",rimg);
cvShowImage("Thresholded Image",thresh);
//Escape Sequence
char c=cvWaitKey(33);
if(c==27)
break;
}
//Showing the image
cvShowImage("Original Image",rimg);
cvShowImage("Thresholded Image",thresh);
//Waiting for user to press any key
cvWaitKey(0);
//Cleanup
cvReleaseImage(&img);
cvReleaseImage(&thresh);
cvReleaseImage(&rimg);
cvReleaseImage(&hsvimg);
cvDestroyAllWindows();
 
}
