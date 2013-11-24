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


using namespace std;
using namespace cv;

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

/*Czerwony prostokąt*/
int x = 360;
int y = 160;
int width = 300;
int height = 300;

int minimum(int a, int b);

int main(int argc, const char* argv[]) {

  //variables for fps counting
  /* time_t start, end;
     double sec,fps;
  */
  int c; //naciśnięty przycisk klawiatury
  gesture openHand;
  int hullcount;

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
  IplImage* sum  = cvCreateImage(cvGetSize(img),32,1); 
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

  //Variables for trackbar
  int h1=0;int s1=0;int v1=0;
  int h2=255;int s2=255;int v2=255;
  int t1 = 15;
  //Creating the trackbars
  cvCreateTrackbar("H1","cnt",&h1,255,0);
  cvCreateTrackbar("H2","cnt",&h2,255,0);
  cvCreateTrackbar("S1","cnt",&s1,255,0);
  cvCreateTrackbar("S2","cnt",&s2,255,0);
  cvCreateTrackbar("V1","cnt",&v1,255,0);
  cvCreateTrackbar("V2","cnt",&v2,255,0);
  cvCreateTrackbar("Diff","cnt",&t1,255,0);

  //fps start
  //time(&start);
  int counter=0;
  int MAXIMAGE = 10;
  CvMemStorage *  defects_st = cvCreateMemStorage(0);
  CvMemStorage *  contour_st = cvCreateMemStorage(0);
  CvMemStorage * storage = cvCreateMemStorage(0);
  CvMemStorage *  hull_st = cvCreateMemStorage(0);
  CvMemStorage *  hull_st2 = cvCreateMemStorage(0);

  CvSeq *defects, *hull2,  *hull = NULL;
  CvSeq * maxC = NULL;
  CvConvexityDefect *defect_array;
  int maxArea = 0;
  int num_defects;
  int actualDefects = 0;
  CvPoint hand_center;

  cvSetImageROI(img,cvRect(x,y,width,height));
  while (counter <= MAXIMAGE)
    {

      imgFF = cvQueryFrame(capture);
      cvSmooth(imgFF, tmp3, CV_MEDIAN, 7, 7);
      cvCvtColor(tmp3, imgGrayFF, CV_RGB2GRAY);
      //    cvCopy(tmp1, imgGrayFF);

      cvAcc(imgGrayFF, sum);

      counter++;

    }
  cvResetImageROI(img);
  cvConvertScale(sum, imgGrayFF, 1.0/MAXIMAGE, 0);


  while (1) {

    cvZero(imgCont);
    rimg = cvQueryFrame(capture);

    cvRectangle(rimg,cvPoint(x,y),cvPoint(x+width, y+height),(CV_RGB(255,0,0)),1);
    cvSetImageROI(rimg,cvRect(x,y,width,height));
    cvInRangeS(rimg,cvScalar(h1,s1,v1),cvScalar(h2,s2,v2),thresh);
    cvCopy(rimg, imgCurr);
    //imgCurr = cvQueryFrame(capture);
    cvSmooth(imgCurr, tmp3, CV_MEDIAN, 7, 7);
    //imgCurr = cvQueryFrame(capture);


    cvCvtColor(tmp3, imgGrayCurr, CV_RGB2GRAY);
    cvErode(imgGrayCurr, imgGrayCurr, kernel, 1);
    cvDilate(imgGrayCurr, imgGrayCurr, kernel, 1);
    //cvCopy(tmp1, imgGrayCurr);
    //cvAdd(tmp1, tmp1, tmp1);
    //cvErode(tmp2, tmp2, kernel,1);
    cvAbsDiff(imgGrayCurr,imgGrayFF,imgDiff);
    //cvAnd(tmp1, tmp2, result);
    //   cvNot(result,tmp3);
    cvThreshold(imgDiff, tmp1, t1, 255, THRESH_BINARY);
    cvDilate(imgDiff, imgDiff,kernel,  1);
    cvErode(tmp1, imgDiff, kernel,  1);
    
    cvResetImageROI( rimg );


    cvSmooth(thresh, thresh, CV_MEDIAN, 7, 7);
    
    cvErode(thresh,thresh,kernel,1);
    cvDilate(thresh,thresh,kernel,1);
    
    cvErode(thresh,thresh,kernel,1);
    cvDilate(thresh,thresh,kernel,1);
    cvCopy(thresh, tmp1);
/*    cvAnd(thresh, imgDiff, imgTbs);
    
    cvErode(imgTbs,imgTbs,kernel,1);
    cvDilate(imgTbs,imgTbs,kernel,1);
    
    cvCopy(imgTbs, tmp1);*/
    //cvCopy(thresh,kopia2, NULL);




    CvSeq * first = NULL;
    CvSeq * contour = NULL;
    cvFindContours(thresh, storage, &first, sizeof(CvContour),CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    maxC = first;
    maxArea = 0.0;

    for(contour = first; contour != 0; contour = contour->h_next)
      {
  CvRect bound = cvBoundingRect(contour,0);
  if(maxArea < bound.width * bound.height)
    {
      maxC = contour;
      maxArea = bound.width * bound.height;
    }
      }

    /*if (maxC) {
      maxC = cvApproxPoly(maxC, sizeof(CvContour),contour_st, CV_POLY_APPROX_DP, 2,1);

      }*/

    if (maxC)
      {
  //Rysowanie największego konturu + otaczającego go prostokąta
  CvRect bound = cvBoundingRect(maxC,0);
  cvDrawContours(imgCont,maxC,CV_RGB(0,255,255),CV_RGB(0,255,255),CV_FILLED);
  cvRectangle(imgCont,cvPoint(bound.x,bound.y),cvPoint(bound.x+bound.width,bound.y+bound.height),CV_RGB(255,0,0),1);
  hand_center =  cvPoint((bound.x+bound.x+bound.width)/2,(bound.y+bound.y+bound.height)/2);
  cvCircle(imgCont, hand_center, 5, CV_RGB(255,0,255), 1, CV_AA, 0); //rysowanie środka kwadratu = środek dłoni

  hull = cvConvexHull2(maxC, hull_st, CV_CLOCKWISE, 1);
  hull2 = cvConvexHull2(maxC, hull_st2, CV_CLOCKWISE, 0);

  if (hull2 && hull){

    //Rysowanie convex hulla
    cvDrawContours(imgCont,hull,CV_RGB(0,255,0),CV_RGB(0,255,0),CV_FILLED);

    defects = cvConvexityDefects(maxC, hull2, defects_st);
    if (defects && defects->total) {
      defect_array = (CvConvexityDefect*)calloc(defects->total, sizeof(CvConvexityDefect));

      cvCvtSeqToArray(defects, defect_array, CV_WHOLE_SEQ);

      num_defects = defects->total;



      for(int j=0; j<minimum(8, num_defects);j++) {
        
        if(defect_array[j].depth > bound.height/5) 
    {
      cvCircle( imgCont, *(defect_array[j].depth_point), 5, CV_RGB(0,0,255), 2, 8,0);
      actualDefects++;
    }
      }

      free(defect_array);

    }
  }
      }

    //displaying images
    cvShowImage("Original Image",rimg);
    cvShowImage("Diff", imgDiff);
    cvShowImage("Thresholded Image",tmp1);
    cvShowImage("Contours", imgCont);
    cvShowImage("TBS", imgTbs);

    //Stop the clock and show FPS
    //time(&end);
    /*++counter;
      sec=difftime(end,start);
      fps=counter/sec*/;


    c = cvWaitKey(10) & 255 ;



    if (c == 27)
      {
  break;
      } else if (c == 32)
      {

  static CvMoments* moments = new CvMoments();
  cvMoments(maxC, moments);
  static CvHuMoments* huMoments = new CvHuMoments();  
  cvGetHuMoments(moments, huMoments);

  openHand.hu1 =  huMoments->hu1;
  openHand.hu2 =  huMoments->hu2;
  openHand.hu3 =  huMoments->hu3;
  openHand.num_def = actualDefects;

  cout<<"HU1:"<<openHand.hu1<<endl;
  cout<<"HU2:"<<openHand.hu2<<endl;  
  cout<<"HU3:"<<openHand.hu3<<endl;
  cout<<"DEF:"<<openHand.num_def<<endl;

  break;
      }//1. while end

    //

    actualDefects = 0;  

  } //while end

  cvReleaseCapture(&capture);
  cvReleaseImage(&img);
  cvReleaseImage(&thresh);
  cvReleaseImage(&rimg);

  cvDestroyWindow("afterEffects");
  cvDestroyWindow("Original Image");
  cvDestroyWindow("cnt");
  cvDestroyWindow("TBS");

  cvReleaseMemStorage(&storage);
  cvReleaseMemStorage(&contour_st);
  cvReleaseMemStorage(&hull_st);
  cvReleaseMemStorage(&hull_st2);
  cvReleaseMemStorage(&defects_st);

  return 0;
}

int minimum(int a, int b)
{
  if (a <= b) return a;
  else return b;
}
