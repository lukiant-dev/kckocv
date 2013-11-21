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

struct openHand {
  float hu1;
  float hu2;
  float hu3;
  CvSeq* hull;
  CvPoint* defects;
};

struct indicFinger {
  float hu1;
  float hu2;
  float hu3;
  CvSeq* hull;
  CvPoint* defects;
};


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
  CvSeq *defects;
  float huHand[3];

// pole konturu temp
  double area = 0 ;
//bounding_rect
  int largest_area = 0;
  int largest_contour_index=0;
// erosion element

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



//fps start
  time(&start);
  int counter=0;
  int i = 0;
  int MAXIMAGE = 10;
  CvMemStorage *  defects_st = cvCreateMemStorage(0);

// background averaging
  IplImage * image = cvCreateImage(cvGetSize(img),8,3);
// we should create 32 bits of image buffer
  IplImage * buffer = cvCreateImage(cvGetSize(image),32,3);
  IplImage* img2    = cvCreateImage(cvGetSize(img),8,3);

cvZero(buffer);    // clear data in buffer
// result image buffer
IplImage * result = cvCreateImage(cvGetSize(image),8,1);
IplImage * res2 = cvCreateImage(cvGetSize(image),8,3);
cvZero(result);    // clear data in result 
IplImage * temp = cvCreateImage(cvGetSize(image),8,1);  
IplImage * temp2 = cvCreateImage(cvGetSize(image),8,1); 
IplImage * temp1 = cvCreateImage(cvGetSize(image),8,1);  
IplImage * temp3 = cvCreateImage(cvGetSize(image),8,1); 
IplImage * buff2 = cvCreateImage(cvGetSize(image),8,1);
cvZero(buff2); 

for(i=0;i< MAXIMAGE;i++){
  cvZero(image);           // clear image
  image=cvQueryFrame(capture);   // custom function for get image from camera

  cvSmooth(image, image, CV_MEDIAN, 5, 5);
  cvCvtColor(image,temp, CV_RGB2GRAY);

  cvScale(image,temp1,0.5,128);  // must change 8 bit image to 32 bit image before do cvAdd
 cvAdd(image,buffer,buffer); // add image and buffer then keep in buffer
}
// then divide summarized image with image number
  //cvScale(buffer,buff2,1.0/MAXIMAGE*1.0); // must include 1.0 for floating point operation


while (1) {
  cvSet(kopia, cvScalar(0,0,0));
  rimg = cvQueryFrame(capture);


// background subtraction!!
  
//  img2 = cvQueryFrame(capture);
//  cvSmooth(img2, img2, CV_MEDIAN, 5, 5);
 // cvCvtColor(img2,temp2, CV_RGB2GRAY);
  //cvScale(img2, temp3, 0.5, 0);
//  cvAbsDiff(temp, temp2, result);
  //cvScale(temp, res2,1.0);
//  cvShowImage("result", result);

  cvRectangle(rimg,cvPoint(x,y),cvPoint(x+width, y+height),(CV_RGB(255,0,0)),1);
  cvSetImageROI(rimg,cvRect(x,y,width,height));
  cvInRangeS(rimg,cvScalar(h1,s1,v1),cvScalar(h2,s2,v2),thresh);  
  cvResetImageROI( rimg ); 

  cvSmooth(thresh, thresh, CV_MEDIAN, 7, 7);

  cvErode(thresh,thresh,kernel,1);
  cvDilate(thresh,thresh,kernel,1);

  cvCopy(thresh, kopia2);


//////////////////////////////////////////////////////////////////////////////////
////////////// NOWY KOD Z NETA /////////////////////////

  CvMemStorage* storage = cvCreateMemStorage();
  CvSeq* first_contour = NULL;
  CvSeq* maxitem=NULL;
  double area=0,areamax=0;
  int maxn=0;

//function to find the white objects in the image and return the object boundaries
  int Nc = cvFindContours(img_8uc1,storage, &first_contour, sizeof(CvContour), CV_RETR_LIST);
  int n=0;

//Here we find the contour with maximum area
  if(Nc>0)
  {
    for( CvSeq* c=first_contour; c!=NULL; c=c->h_next )
    {
//cvCvtColor( img_8uc1, img_8uc3, CV_GRAY2BGR );
      area=cvContourArea(c,CV_WHOLE_SEQ );
      if(area>areamax)
        {areamax=area;
          maxitem=c;
          maxn=n;
        }
        n++;
      }

      CvMemStorage* storage3 = cvCreateMemStorage(0);
//if (maxitem) maxitem = cvApproxPoly( maxitem, sizeof(maxitem), storage3, CV_POLY_APPROX_DP, 3, 1 );

if(areamax>5000) //check for area greater than certain value and find convex hull
{
  maxitem = cvApproxPoly( maxitem, sizeof(CvContour), storage3, CV_POLY_APPROX_DP, 10, 1 );
  CvPoint pt0;
  CvMemStorage* storage1 = cvCreateMemStorage(0);
  CvMemStorage* storage2 = cvCreateMemStorage(0);
  CvSeq* ptseq = cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvContour),
    sizeof(CvPoint), storage1 );
  CvSeq* hull;
  CvSeq* defects;
  for(int i = 0; i < maxitem->total; i++ )
    { CvPoint* p = CV_GET_SEQ_ELEM( CvPoint, maxitem, i );
      pt0.x = p->x;
      pt0.y = p->y;
      cvSeqPush( ptseq, &pt0 );
    }
    hull = cvConvexHull2( ptseq, 0, CV_CLOCKWISE, 0 );
    int hullcount = hull->total;
    defects= cvConvexityDefects(ptseq,hull,storage2 );
//printf(" defect no %d \n",defects->total);

    CvConvexityDefect* defectArray;
    int j=0;
//int m_nomdef=0;
// This cycle marks all defects of convexity of current contours.
    for(;defects;defects = defects->h_next)
    {
int nomdef = defects->total; // defect amount
//outlet_float( m_nomdef, nomdef );
//printf(" defect no %d \n",nomdef);
if(nomdef == 0)
  continue;
// Alloc memory for defect set.
//fprintf(stderr,"malloc\n");
defectArray = (CvConvexityDefect*)malloc(sizeof(CvConvexityDefect)*nomdef);
// Get defect set.
//fprintf(stderr,"cvCvtSeqToArray\n");
cvCvtSeqToArray(defects,defectArray, CV_WHOLE_SEQ);
// Draw marks for all defects.
for(int i=0; i
  { printf(" defect depth for defect %d %f \n",i,defectArray[i].depth);
  cvLine(img_8uc3, *(defectArray[i].start), *(defectArray[i].depth_point),CV_RGB(255,255,0),1, CV_AA, 0 );
  cvCircle( img_8uc3, *(defectArray[i].depth_point), 5, CV_RGB(0,0,164), 2, 8,0);
  cvCircle( img_8uc3, *(defectArray[i].start), 5, CV_RGB(0,0,164), 2, 8,0);
  cvLine(img_8uc3, *(defectArray[i].depth_point), *(defectArray[i].end),CV_RGB(255,255,0),1, CV_AA, 0 );


///////////////////////////////////////////////////////////////////////////////////////////


  static CvHuMoments* huMoments = new CvHuMoments();	
  CvMemStorage * storage = cvCreateMemStorage(0);
  CvMemStorage * storage_h = cvCreateMemStorage(0);
  CvSeq * first = NULL;
  CvSeq * contour = NULL;
  cvFindContours(kopia2, storage, &first, sizeof(CvContour),CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
  CvSeq * max = first;
  CvSeq * hull = first;
  int maxArea = 0;
  for(contour = first; contour != 0; contour = contour->h_next)
  {
   area = fabs(cvContourArea(contour, CV_WHOLE_SEQ, 0));
   if(maxArea < area)
   {
    max = contour;
    maxArea = area;
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
       //cvRectangle(kopia,cvPoint(bound.x,bound.y),cvPoint(bound.x+bound.width,bound.y+bound.height),color,1);

   hull = cvConvexHull2(contour, storage_h, CV_CLOCKWISE, 0 );

   if (hull)
   {  
    defects = cvConvexityDefects(contour,hull,
     defects_st);
    cvDrawContours(kopia,defects,CV_RGB(0,128,128),CV_RGB(0,128,128),CV_FILLED);
    printf("!!! \n");

  } 
}

 // momenty! 

static CvMoments* moments = new CvMoments();
for(contour = max; contour != 0; contour = contour->h_next)
{
  cvMoments(contour, moments);
  cvGetHuMoments(moments, huMoments);
}


 //Tu kod liczący momenty i porównujący ze wzorcem


   printf("hu1: %f\n", huMoments->hu1);
   printf("hu2: %f\n", huMoments->hu2);
   printf("hu3: %f\n\n", huMoments->hu3);


   cvReleaseMemStorage(&storage);


 //displaying images
   cvShowImage("Original Image",rimg);
 //cvShowImage("Color Image",kopia2);
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
