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

#define NUM_DEFECTS 8


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
  int x1 =0, y1 = 0;
  float huHand[3];

  IplConvKernel*  kernel = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_RECT, NULL);

//creating capture image
  CvCapture* capture = cvCaptureFromCAM(0);
  IplImage* img = cvQueryFrame(capture);
  IplImage* rimg   = cvCreateImage(cvGetSize(img),8,3);
// Setting ROI for smaller image
  cvSetImageROI(img,cvRect(x,y,width,height));
  IplImage* kopia2 = cvCreateImage(cvGetSize(img), 8, 1);
  IplImage* thresh  = cvCreateImage(cvGetSize(img),8,1);
  IplImage* kopia   = cvCreateImage(cvGetSize(img),8,3);
  //IplImage* kopia2   = cvCreateImage(cvGetSize(img),8,1);
  cvResetImageROI( img );


//Windows for displaying images and trackbars
  cvNamedWindow("Original Image",CV_WINDOW_AUTOSIZE);
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
  double area, max_area = 0.0;
  CvSeq *contours = NULL;
  CvSeq *tmp = NULL;
 
  CvMemStorage *  contour_st = cvCreateMemStorage(0);
  CvSeq *defects, *hull = NULL;
  CvConvexityDefect *defect_array;
  CvMemStorage *  hull_st = cvCreateMemStorage(0);

  CvPoint *xdefects;
  //calloc powoduje problemy :((((
  xdefects = (CvPoint*)calloc(NUM_DEFECTS, sizeof(CvPoint));
  int num_defects;
  CvPoint hand_center;
  int hand_radius;
  int dist = 0;

  while (1) {

    cvZero(kopia);
    rimg = cvQueryFrame(capture);

    cvRectangle(rimg,cvPoint(x,y),cvPoint(x+width, y+height),(CV_RGB(255,0,0)),1);
    cvSetImageROI(rimg,cvRect(x,y,width,height));
    cvInRangeS(rimg,cvScalar(h1,s1,v1),cvScalar(h2,s2,v2),thresh);  
    cvResetImageROI( rimg ); 

    cvSmooth(thresh, thresh, CV_MEDIAN, 7, 7);

    cvErode(thresh,thresh,kernel,1);
    cvDilate(thresh,thresh,kernel,1);

    //cvCopy(thresh,kopia, NULL);
    cvCopy(thresh, kopia2, NULL);
  /* cvFindContours modifies input image, so make a copy */
    cvFindContours(kopia2, contour_st, &contours,
     sizeof(CvContour), CV_RETR_EXTERNAL,
     CV_CHAIN_APPROX_SIMPLE);//, cvPoint(0, 0));
     CvSeq *contour = contours;
    for (tmp = contours; tmp != 0; tmp = tmp->h_next) {
      area = fabs(cvContourArea(tmp, CV_WHOLE_SEQ, 0));
      if (area > max_area) {
        max_area = area;
        contour = tmp;
      }
    }

    if (contour) {
      printf("CONT!\n");
      contour = cvApproxPoly(contour, sizeof(CvContour),contour_st, CV_POLY_APPROX_DP, 2);
    }
  
    if (contour)
    {
      printf("YES\n");
      cvDrawContours(kopia,contour,CV_RGB(0,255,255),CV_RGB(0,255,255),CV_FILLED); 
      hull = cvConvexHull2(contour, 0, CV_CLOCKWISE, 0);
    

    if (hull){
    /* Get convexity defects of contour w.r.t. the convex hull */
      printf("HULL!\n");
       //cvDrawContours(kopia,hull,CV_RGB(255,255,0),CV_RGB(255,255,0),CV_FILLED); 
      defects = cvConvexityDefects(contour, hull, defects_st);
      if (defects && defects->total) {
        defect_array = (CvConvexityDefect*)calloc(defects->total, sizeof(CvConvexityDefect));
        cvCvtSeqToArray(defects, defect_array, CV_WHOLE_SEQ);
      //cvDrawContours(kopia,defects,CV_RGB(255,255,0),CV_RGB(255,255,0),CV_FILLED); 
      // Average depth points to get hand center 
       for (i = 0; i < defects->total && i < NUM_DEFECTS; i++) {
        x1 += defect_array[i].depth_point->x;
        y1 += defect_array[i].depth_point->y;

       xdefects[i] = cvPoint(defect_array[i].depth_point->x,
                defect_array[i].depth_point->y);
      }
     

      x1 /= defects->total;
      y1 /= defects->total;

      num_defects = defects->total;
      hand_center = cvPoint(x1, y1);
     
      /*for(int j=0; j<num_defects;j++) {
        printf(" defect depth for defect %d %f \n",j,defect_array[j].depth);
        cvLine(kopia, *(defect_array[j].start), *(defect_array[j].depth_point),CV_RGB(255,255,0),1, CV_AA, 0 );
        //cvCircle( kopia, *(defect_array[j].depth_point), 5, CV_RGB(0,0,164), 2, 8,0);
        //cvCircle( kopia, *(defect_array[j].start), 5, CV_RGB(0,0,164), 2, 8,0);
        cvLine(kopia, *(defect_array[j].depth_point), *(defect_array[j].end),CV_RGB(255,255,0),1, CV_AA, 0 );
      }*/

        int hullcount = hull->total;



       CvPoint pt0 = **CV_GET_SEQ_ELEM( CvPoint*, hull, hullcount - 1 );

        for(int k = 0; k < hullcount; k++ )
        {

            CvPoint pt = **CV_GET_SEQ_ELEM( CvPoint*, hull, k );
            cvLine( kopia, pt0, pt, CV_RGB( 0, 255, 0 ), 1, CV_AA, 0 );
            pt0 = pt;
        }

      /* Compute hand radius as mean of distances of
         defects' depth point to hand center */
      
      for (i = 0; i < defects->total; i++) {
        int d = (x1 - defect_array[i].depth_point->x) *
          (x1 - defect_array[i].depth_point->x) +
          (y1 - defect_array[i].depth_point->y) *
          (y1 - defect_array[i].depth_point->y);

        dist += sqrt(d);
      }

      hand_radius = dist / defects->total;
      cvCircle(kopia, hand_center, 5, CV_RGB(255,0,255), 1, CV_AA, 0);
      cvCircle(kopia, hand_center, hand_radius, CV_RGB(255,0,0), 1, CV_AA, 0);
      free(defect_array);

    }
 }
}

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


//cvReleaseMemStorage(&storage);
 } //while end

 cvDestroyWindow("mywindow");
 cvDestroyWindow("afrerEffects");

 return 0;
}
