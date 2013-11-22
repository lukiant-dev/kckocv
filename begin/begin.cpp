#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>
#include <string>
#include <list>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <time.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/opencv.hpp>

#define NUM_DEFECTS 8


struct gesture {
  float hu1;
  float hu2;
  float hu3;
  float num_def;
  float num_hull;
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

int minimum(int a, int b);

using namespace std;
using namespace cv;

int main(int argc, const char* argv[]) {

//variables for fps counting
  time_t start, end;
  double sec,fps;
  int x1 =0, y1 = 0;
  float huHand[3];

  IplConvKernel*  kernel = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_RECT, NULL);

//creating capture image
  CvCapture* capture = cvCaptureFromCAM(0);
  IplImage* img = cvQueryFrame(capture);
  IplImage* rimg   = cvCreateImage(cvGetSize(img),8,3);
// Setting ROI for smaller image
  cvSetImageROI(img,cvRect(x,y,width,height));
  IplImage* imgTbs = cvCreateImage(cvGetSize(img), 8, 1);
  IplImage* thresh  = cvCreateImage(cvGetSize(img),8,1);
  IplImage* imgCont = cvCreateImage(cvGetSize(img), 8, 3);

  IplImage* imgFF   = cvCreateImage(cvGetSize(img),8,3);
  IplImage* imgCurr   = cvCreateImage(cvGetSize(img),8,3);
  IplImage* imgGrayFF  = cvCreateImage(cvGetSize(img),8,1);
  IplImage* imgGrayCurr   = cvCreateImage(cvGetSize(img),8,1);
  IplImage* imgDiff   = cvCreateImage(cvGetSize(img),8,1);

  IplImage* tmp1  = cvCreateImage(cvGetSize(img),8,1); 
  IplImage* tmp3  = cvCreateImage(cvGetSize(img),8,3); 


  cvResetImageROI( img );


//Windows for displaying images and trackbars
  cvNamedWindow("Original Image",CV_WINDOW_AUTOSIZE);
  cvNamedWindow("cnt",CV_WINDOW_AUTOSIZE);
  cvNamedWindow("Contours", CV_WINDOW_AUTOSIZE);
  cvNamedWindow("Thresholded Image", CV_WINDOW_AUTOSIZE);
  cvNamedWindow("TBS", CV_WINDOW_AUTOSIZE);
  cvNamedWindow("Diff", CV_WINDOW_AUTOSIZE);


  cvMoveWindow("cnt", 1000, 0);
  cvMoveWindow("Contours", 700, 0);
  cvMoveWindow("TBS", 0,500);
  cvMoveWindow("Thresholded Image", 700, 500);
  cvMoveWindow("Diff", 1000, 500);


  //cvNamedWindow("result", CV_WINDOW_AUTOSIZE);



//Variables for trackbar
  int h1=0;int s1=0;int v1=0;
  int h2=255;int s2=255;int v2=255;
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
  //double area, max_area = 0.0;
  //CvSeq *contours = NULL;
  //CvSeq *tmp = NULL;


  CvMemStorage *  contour_st = cvCreateMemStorage(0);
  CvSeq *defects, *hull2,  *hull = NULL;
  CvConvexityDefect *defect_array;
  CvMemStorage *  hull_st = cvCreateMemStorage(0);
 // CvMemStorage *  hull_st2 = cvCreateMemStorage(0);

  CvPoint *xdefects;
  //calloc powoduje problemy :((((

  int num_defects;
  CvPoint hand_center;
  int hand_radius;
  int dist = 0;


  
  cvSetImageROI(img,cvRect(x,y,width,height));
  imgFF = cvQueryFrame(capture);
  cvSmooth(imgFF, tmp3, CV_MEDIAN, 7, 7);
  cvCvtColor(tmp3, imgGrayFF, CV_RGB2GRAY);
  //cvErode(tmp1, tmp1, kernel,1);
 // cvShowImage("result", tmp1);

  //cvShowImage("result", back);

  cvResetImageROI(img);
  int c;
  gesture openHand;
  int hullcount;
  while (1) {

    cvZero(imgCont);
    rimg = cvQueryFrame(capture);

    cvRectangle(rimg,cvPoint(x,y),cvPoint(x+width, y+height),(CV_RGB(255,0,0)),1);
    cvSetImageROI(rimg,cvRect(x,y,width,height));
    cvInRangeS(rimg,cvScalar(h1,s1,v1),cvScalar(h2,s2,v2),thresh);
    cvCopy(rimg, imgCurr);
    //imgCurr = cvQueryFrame(capture);
    //cvSmooth(imgCurr, tmp3, CV_MEDIAN, 7, 7);
    //imgCurr = cvQueryFrame(capture);


    cvCvtColor(imgCurr, imgGrayCurr, CV_RGB2GRAY);
    //cvAdd(tmp1, tmp1, tmp1);
      //cvErode(tmp2, tmp2, kernel,1);
    cvAbsDiff(imgGrayCurr,imgGrayFF,imgDiff);
    //cvAnd(tmp1, tmp2, result);
//   cvNot(result,tmp3);
    cvThreshold(imgDiff, tmp1, 15, 255, THRESH_BINARY);
    cvErode(tmp1, imgDiff, kernel,  1);
    //cvDilate(result1, result1,kernel,  3);


   //cvAnd(tmp1, tmp2 ,result);
   // cvShowImage("xxx", result);

    cvResetImageROI( rimg );


    cvSmooth(thresh, thresh, CV_MEDIAN, 7, 7);

    cvErode(thresh,thresh,kernel,1);
    cvDilate(thresh,thresh,kernel,1);
    cvAnd(thresh, imgDiff, imgTbs);
    cvDilate(imgTbs,imgTbs,kernel,1);
    cvCopy(imgTbs, tmp1);
    //cvCopy(thresh,kopia2, NULL);

    //cvCopy(thresh, kopia2, NULL);
  /* cvFindContours modifies input image, so make a copy */
    /*cvFindContours(kopia2, contour_st, &contours,
     sizeof(CvContour), CV_RETR_EXTERNAL,
     CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
     CvSeq *contour = contours;*/


CvMemStorage * storage = cvCreateMemStorage(0);
CvSeq * first = NULL;
CvSeq * contour = NULL;
cvFindContours(tmp1, storage, &first, sizeof(CvContour),CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
CvSeq * maxC = first;
int maxArea = 0;
for(contour = first; contour != 0; contour = contour->h_next)
{
  CvRect bound = cvBoundingRect(contour,0);
  if(maxArea < bound.width * bound.height)
  {
    maxC = contour;
    maxArea = bound.width * bound.height;
  }
}

      if (maxC)
      {
       CvRect bound = cvBoundingRect(maxC,0);
       cvDrawContours(imgCont,maxC,CV_RGB(0,255,255),CV_RGB(0,255,255),CV_FILLED);

       cvRectangle(imgCont,cvPoint(bound.x,bound.y),cvPoint(bound.x+bound.width,bound.y+bound.height),CV_RGB(255,0,0),1);


       hull = cvConvexHull2(maxC, hull_st, CV_CLOCKWISE, 0);
      // hull2 = cvConvexHull2(maxC, hull_st2, CV_CLOCKWISE, 0);
       //double hullArea = fabs(cvContourArea(hull, CV_WHOLE_SEQ, 0));
       //printf("%f || %d\n", hullArea, maxArea);
       
       if ( hull){

        cvDrawContours(imgCont,hull,CV_RGB(0,255,0),CV_RGB(0,255,0),CV_FILLED);

  
      //printf("HULL!\n");
       //cvDrawContours(kopia,hull,CV_RGB(255,255,0),CV_RGB(255,255,0),CV_FILLED); 
        defects = cvConvexityDefects(maxC, hull, defects_st);
        if (defects && defects->total) {
          defect_array = (CvConvexityDefect*)calloc(defects->total, sizeof(CvConvexityDefect));

          cvCvtSeqToArray(defects, defect_array, CV_WHOLE_SEQ);


          num_defects = defects->total;
          hand_center =  cvPoint((bound.x+bound.x+bound.width)/2,(bound.y+bound.y+bound.height)/2);


          for(int j=0; j<num_defects;j++) {
        //printf(" defect depth for defect %d %f \n",j,defect_array[j].depth);
            cvLine(imgCont, *(defect_array[j].start), *(defect_array[j].depth_point),CV_RGB(255,255,0),1, CV_AA, 0 );
            cvCircle( imgCont, *(defect_array[j].depth_point), 5, CV_RGB(0,0,164), 2, 8,0);
            cvCircle( imgCont, *(defect_array[j].start), 5, CV_RGB(0,0,164), 2, 8,0);
            cvLine(imgCont, *(defect_array[j].depth_point), *(defect_array[j].end),CV_RGB(255,255,0),1, CV_AA, 0 );
            cvLine(imgCont,hand_center, *(defect_array[j].end),CV_RGB(128,0,128),1, CV_AA, 0 );
          }

        

      

            for (i = 0; i < defects->total; i++) {
              int d = (x1 - defect_array[i].depth_point->x) *
              (x1 - defect_array[i].depth_point->x) +
              (y1 - defect_array[i].depth_point->y) *
              (y1 - defect_array[i].depth_point->y);

              dist += sqrt(d);
            }

            hand_radius = dist / defects->total;
            cvCircle(imgCont, hand_center, 5, CV_RGB(255,0,255), 1, CV_AA, 0);
            cvCircle(imgCont, hand_center, hand_radius, CV_RGB(255,0,0), 1, CV_AA, 0);
            free(defect_array);

          }
        }
      }

 //displaying images
      cvShowImage("Original Image",rimg);
      cvShowImage("Diff", imgDiff);
      cvShowImage("Thresholded Image",thresh);
      cvShowImage("Contours", imgCont);
      cvShowImage("TBS", imgTbs);

 //Stop the clock and show FPS
      time(&end);
      ++counter;
      sec=difftime(end,start);
      fps=counter/sec;


 // ESC key ends program
      c = cvWaitKey(5) & 255 ;


  if (c == 27)
  {
    break;
  } else if (c == 32)
  {
    printf("SPACJA\n");

    static CvMoments* moments = new CvMoments();
    cvMoments(maxC, moments);
    static CvHuMoments* huMoments = new CvHuMoments();  
    cvGetHuMoments(moments, huMoments);

    openHand.hu1 =  huMoments->hu1;
    openHand.hu2 =  huMoments->hu2;
    openHand.hu3 =  huMoments->hu3;
    openHand.num_def = num_defects;
   /* openHand.num_hull = hullcount;*/

    cout<<"HU1:"<<openHand.hu1<<endl;
    cout<<"HU2:"<<openHand.hu2<<endl;

    cout<<"HU3:"<<openHand.hu3<<endl;

    cout<<"DEF:"<<openHand.num_def<<endl;
/*
    cout<<"HULL:"<<openHand.num_hull<<endl;*/


  }

     } //while end

     cvDestroyWindow("mywindow");
     cvDestroyWindow("afrerEffects");

     return 0;
   }

   int minimum(int a, int b)
   {
    if (a <= b) return a;
    else return b;
  }