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

#define FRAMES 200;
using namespace std;
using namespace cv;

bool startTest= false;
int correctFrames = 0;
int totalFrames = 0;
int minimum(int a, int b);
bool contains(CvRect r1, CvRect r2);

struct gesture {
  float hu1;
  float hu2;
  float hu3;
  float num_def;
  float num_hull;
};

gesture closeHand, clickHand, moveHand;
int h1=0, s1=0, v1=0, h2=255, s2=255, v2=255;
CvRect bound;
CvPoint hand_center;
int x = 360, y = 160, width = 300, height = 300;
bool endProgram = false;

void init_windows() {
  //Windows for displaying images and trackbars
  cvNamedWindow("Original Image",CV_WINDOW_AUTOSIZE);


  cvNamedWindow("cnt",CV_WINDOW_AUTOSIZE);
  cvNamedWindow("Thresholded Image", CV_WINDOW_AUTOSIZE);


  cvMoveWindow("cnt", 1000, 0);
  cvMoveWindow("Thresholded Image", 700, 500);

  //Creating the trackbars
  cvCreateTrackbar("R1","cnt",&h1,255,0);
  cvCreateTrackbar("R2","cnt",&h2,255,0);
  cvCreateTrackbar("G1","cnt",&s1,255,0);
  cvCreateTrackbar("G2","cnt",&s2,255,0);
  cvCreateTrackbar("B1","cnt",&v1,255,0);
  cvCreateTrackbar("B2","cnt",&v2,255,0);
}

void customThresh(IplImage *thresh) {
  IplConvKernel*  kernel = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_RECT, NULL);

  cvSmooth(thresh, thresh, CV_MEDIAN, 7, 7);
  cvErode(thresh,thresh,kernel,1);
  cvDilate(thresh,thresh,kernel,1);
  cvErode(thresh,thresh,kernel,1);
  cvDilate(thresh,thresh,kernel,1);
}

CvSeq * getBiggestContour(IplImage *image)
{

  CvMemStorage * storage = cvCreateMemStorage(0);
  CvSeq * first = NULL;
  CvSeq * contour = NULL;
  CvSeq *maxC = NULL;
  cvFindContours(image, storage, &first, sizeof(CvContour),CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
  maxC = first;
  float maxArea = 0.0;

  for(contour = first; contour != 0; contour = contour->h_next)
  {
    CvRect bound = cvBoundingRect(contour,0);
    if(maxArea < bound.width * bound.height) 
    {
     maxC = contour;
     maxArea = bound.width * bound.height;
   }
 }
  //cvReleaseMemStorage(&storage);

 return maxC;
}

void displayContAndHull(CvSeq *maxC, IplImage *image)
{
  CvMemStorage *  hull_st = cvCreateMemStorage(0);
  CvSeq *hull = cvConvexHull2(maxC, hull_st, CV_CLOCKWISE, 1);
  bound = cvBoundingRect(maxC,0);
  hand_center =  cvPoint((bound.x+bound.x+bound.width)/2,(bound.y+bound.y+bound.height)/2);


  //cvSetImageROI(image,cvRect(x,y,width,height));
  cvDrawContours(image,maxC,CV_RGB(0,255,255),CV_RGB(0,255,255),CV_FILLED);
  cvRectangle(image,cvPoint(bound.x,bound.y),cvPoint(bound.x+bound.width,bound.y+bound.height),CV_RGB(255,0,0),2);
  cvCircle(image , hand_center, 5, CV_RGB(255,0,255), 1, CV_AA, 0); //rysowanie środka kwadratu = środek dłoni
  if (hull)
    cvDrawContours(image,hull,CV_RGB(0,255,0),CV_RGB(0,255,0),10);
  //cvResetImageROI(image);

  //cvReleaseMemStorage(&hull_st);
}

void mousemove(int x_pos, int y_pos, int licznik)
{
  ///Strings that will contain the conversions
  string xcord; string ycord;

  ///These are buffers or something? I don't really know... lol.
  stringstream sstr; stringstream sstr2;
  static int prevx_pos;
  static int prevy_pos;
  static bool first = true;


  string command;
  if(!first)
  {
    int diffX = x_pos-prevx_pos;
    int diffY = y_pos-prevy_pos;
  //  printf("X : %d\n", diffX);
 //   printf("Y: %d\n", diffY);

      ///Getting the command string
    if(diffX>=0) 
     sstr<<(-3)*diffX;
   else
     sstr<<abs(diffX);

   sstr2<<3*diffY;

   xcord = sstr.str();
   ycord = sstr2.str();

   if((diffX >=0) || (diffY <0))
     command = "xdotool mousemove_relative -- " + xcord + " " + ycord;
   else
     command = "xdotool mousemove_relative " + xcord + " " + ycord;
 }
  ///Converting command string to a form that system() accepts.
 const char *com = command.c_str();
 system(com);

 prevx_pos = x_pos;
 prevy_pos = y_pos;
 first = false;

}
void mouse_click()
{
    
        system("xdotool click 1");
    
}



CvRect detectFaceInImage(IplImage *inputImg, CvHaarClassifierCascade* cascade)
{
  // Smallest face size.
  CvSize minFeatureSize = cvSize(20, 20);
  // Only search for 1 face.
  int flags = CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_ROUGH_SEARCH;
  // How detailed should the search be.
  float search_scale_factor = 1.1f;
  IplImage *detectImg;
  IplImage *greyImg = 0;
  CvMemStorage* storage;
  CvRect rc;
  double t;
  CvSeq* rects;
  CvSize size;
  int i, ms, nFaces;

  storage = cvCreateMemStorage(0);
  cvClearMemStorage( storage );


  // If the image is color, use a greyscale copy of the image.
  detectImg = (IplImage*)inputImg;
  if (inputImg->nChannels > 1) {
    size = cvSize(inputImg->width, inputImg->height);
    greyImg = cvCreateImage(size, IPL_DEPTH_8U, 1 );
    cvCvtColor( inputImg, greyImg, CV_BGR2GRAY );
    detectImg = greyImg;// Use the greyscale image.
  }

  // Detect all the faces in the greyscale image.
  t = (double)cvGetTickCount();
  rects = cvHaarDetectObjects( detectImg, cascade, storage,
    search_scale_factor, 3, flags, minFeatureSize);
  t = (double)cvGetTickCount() - t;
  ms = cvRound( t / ((double)cvGetTickFrequency() * 1000.0) );
  nFaces = rects->total;
  //printf("Face Detection took %d ms and found %d objects\n", ms, nFaces);

  // Get the first detected face (the biggest).
  if (nFaces > 0)
    rc = *(CvRect*)cvGetSeqElem( rects, 0 );
  else
    rc = cvRect(-1,-1,-1,-1);// Couldn't find the face.

  if (greyImg)
    cvReleaseImage( &greyImg );

  cvReleaseMemStorage( &storage );
  //cvReleaseHaarClassifierCascade( &cascade );

  return rc;// Return the biggest face found, or (-1,-1,-1,-1).
}

int calibration(CvCapture *capture) {

  IplImage* img = cvQueryFrame(capture);

  // Setting ROI for smaller image
  cvSetImageROI(img,cvRect(x,y,width,height));
  IplImage* thresh  = cvCreateImage(cvGetSize(img),8,1);
  IplImage* tmp1  = cvCreateImage(cvGetSize(img),8,1);
  cvResetImageROI( img );

  CvSeq *maxCont = NULL, *hull = NULL, *defects= NULL;
  CvMemStorage * hull_st = cvCreateMemStorage(0);
  CvMemStorage * defects_st = cvCreateMemStorage(0);
  CvConvexityDefect *defect_array;
  int num_defects, actualDefects = 0;
  int c;

  while(1) {
    img = cvQueryFrame(capture);
    cvRectangle(img,cvPoint(x,y),cvPoint(x+width, y+height),(CV_RGB(255,0,255)),1);
    cvSetImageROI(img,cvRect(x,y,width,height));
    cvInRangeS(img,cvScalar(h1,s1,v1),cvScalar(h2,s2,v2),thresh);
    cvResetImageROI(img);

    customThresh(thresh);
    cvCopy(thresh, tmp1);
    maxCont = getBiggestContour(tmp1);
    if (maxCont) {
     hull = cvConvexHull2(maxCont, hull_st, CV_CLOCKWISE, 0);
     cvSetImageROI(img,cvRect(x,y,width,height));
     displayContAndHull(maxCont, img);
     cvResetImageROI(img);

     if (hull) {
       defects = cvConvexityDefects(maxCont, hull, defects_st);
       if (defects && defects->total) {
        defect_array = (CvConvexityDefect*)calloc(defects->total, sizeof(CvConvexityDefect));

        cvCvtSeqToArray(defects, defect_array, CV_WHOLE_SEQ);
        num_defects = defects->total;
        for(int j=0; j<num_defects;j++) {
          if(defect_array[j].depth > bound.height/6) {
            cvSetImageROI(img,cvRect(x,y,width,height));
            cvCircle( img, *(defect_array[j].depth_point), 5, CV_RGB(0,0,255), 2, 8,0);
            cvResetImageROI(img);
            actualDefects++;
          }
        }
        free(defect_array);
      }

    }
  }



  cvShowImage("Original Image",img);
  cvShowImage("Thresholded Image",thresh);

  c = cvWaitKey(10) & 255 ;

  if (c == 27) {
   endProgram = true;
   break;
 } else if (c == 49) {
   static CvMoments* moments = new CvMoments();
   cvMoments(maxCont, moments);
   static CvHuMoments* huMoments = new CvHuMoments();  
   cvGetHuMoments(moments, huMoments);

   closeHand.hu1 =  huMoments->hu1;
   closeHand.hu2 =  huMoments->hu2;
   closeHand.hu3 =  huMoments->hu3;
   closeHand.num_def = actualDefects;

   cout<<"CLOSE HU1:"<<closeHand.hu1<<endl;
   cout<<"CLOSE HU2:"<<closeHand.hu2<<endl;  
   cout<<"CLOSE HU3:"<<closeHand.hu3<<endl;
   cout<<"CLOSE DEF:"<<closeHand.num_def<<endl;

 }else if(c==50) {
   static CvMoments* moments = new CvMoments();
   cvMoments(maxCont, moments);
   static CvHuMoments* huMoments = new CvHuMoments();  
   cvGetHuMoments(moments, huMoments);

   clickHand.hu1 =  huMoments->hu1;
   clickHand.hu2 =  huMoments->hu2;
   clickHand.hu3 =  huMoments->hu3;
   clickHand.num_def = actualDefects;

   cout<<"CLICK HU1:"<<clickHand.hu1<<endl;
   cout<<"CLICK HU2:"<<clickHand.hu2<<endl;  
   cout<<"CLICK HU3:"<<clickHand.hu3<<endl;
   cout<<"CLICK DEF:"<<clickHand.num_def<<endl;

 } else if (c==51) {
   static CvMoments* moments = new CvMoments();
   cvMoments(maxCont, moments);
   static CvHuMoments* huMoments = new CvHuMoments();  
   cvGetHuMoments(moments, huMoments);

   moveHand.hu1 =  huMoments->hu1;
   moveHand.hu2 =  huMoments->hu2;
   moveHand.hu3 =  huMoments->hu3;
   moveHand.num_def = actualDefects;

   cout<<"MOVE HU1:"<<moveHand.hu1<<endl;
   cout<<"MOVE HU2:"<<moveHand.hu2<<endl;  
   cout<<"MOVE HU3:"<<moveHand.hu3<<endl;
   cout<<"MOVE DEF:"<<moveHand.num_def<<endl;

   break;
 }
 actualDefects = 0;

}
//cvReleaseImage(&img);
//cvReleaseImage(&thresh);
cvZero(img);
cvZero(thresh);
/*
cvReleaseMemStorage(&hull_st);
cvReleaseMemStorage(&defects_st);
*/
}

void mainLoop(CvCapture *capture, int fRecog)
{

  IplImage* img = cvQueryFrame(capture);

  IplImage* thresh  = cvCreateImage(cvGetSize(img),8,1);
  IplImage* tmp1  = cvCreateImage(cvGetSize(img),8,1);

  CvSeq *maxCont = NULL, *hull = NULL, *defects = NULL;
  CvMemStorage * hull_st = cvCreateMemStorage(0);
  CvMemStorage * defects_st = cvCreateMemStorage(0);
  CvConvexityDefect *defect_array;
  int num_defects, actualDefects = 0;
  int c, licznik = 0;
  /*Wykrywanie twarzy*/
  CvHaarClassifierCascade* faceCascade;
  if (fRecog) 
  {
    char *faceCascadeFilename = "haarcascade_frontalface_alt.xml"; 
    faceCascade = (CvHaarClassifierCascade*)cvLoad(faceCascadeFilename, 0, 0, 0);
    if( !faceCascade ) {
      printf("Couldn't load Face detector '%s'\n", faceCascadeFilename);
      exit(1);
    }
  }


  CvRect faceRect;

  while(1)
  {  
    img = cvQueryFrame(capture);
    cvInRangeS(img,cvScalar(h1,s1,v1),cvScalar(h2,s2,v2),thresh);
    customThresh(thresh);
    
    if (fRecog) {
      faceRect = detectFaceInImage(img, faceCascade);
      if (faceRect.width > 0) {
       cvRectangle(img,cvPoint(faceRect.x,faceRect.y),cvPoint(faceRect.x+faceRect.width, faceRect.y+faceRect.height),(CV_RGB(255,255,0)),1);
       cvRectangle(thresh,cvPoint(faceRect.x,faceRect.y),cvPoint(faceRect.x+faceRect.width, faceRect.y+faceRect.height),(CV_RGB(0,0,0)),CV_FILLED);
     }
   }
   cvCopy(thresh, tmp1);

   maxCont = getBiggestContour(tmp1);
   if (startTest ==true)
      {  totalFrames++;
          printf("------- %d \n", totalFrames);
        }
   if (maxCont)
   {
     hull = cvConvexHull2(maxCont, hull_st, CV_CLOCKWISE, 0);
     displayContAndHull(maxCont, img);
     if (hull)
     {
       defects = cvConvexityDefects(maxCont, hull, defects_st);
       if (defects && defects->total) 
       {
        defect_array = (CvConvexityDefect*)calloc(defects->total, sizeof(CvConvexityDefect));

        cvCvtSeqToArray(defects, defect_array, CV_WHOLE_SEQ);
        num_defects = defects->total;
        for(int j=0; j<num_defects;j++)
        {
          if(defect_array[j].depth > bound.height/6) 
          {
            cvCircle( img, *(defect_array[j].depth_point), 5, CV_RGB(0,0,255), 2, 8,0);

            actualDefects++;
          }
        }
        free(defect_array);
      }
    }

    static CvMoments* moments = new CvMoments();
    cvMoments(maxCont, moments);
    static CvHuMoments* huMoments = new CvHuMoments();  
    cvGetHuMoments(moments, huMoments);

//    cout<<"H1:"<<huMoments->hu1<<endl;

//    cout<<"H2:"<<huMoments->hu2<<endl;

//    cout<<"H3:"<<huMoments->hu3<<endl;




    if ((abs(moveHand.hu1 - huMoments->hu1)<0.01) && (abs(moveHand.hu2 - huMoments->hu2)<0.01) &&
     (abs(moveHand.hu3 - huMoments->hu3)<0.0008) && (moveHand.num_def == actualDefects))
    { 
     mousemove(hand_center.x,hand_center.y, licznik);
     if (startTest== false) startTest=true;
     correctFrames++;
    // printf("MOVE! %d\n", licznik);
   } else if ((abs(closeHand.hu1 - huMoments->hu1)<0.08) && (abs(closeHand.hu2 - huMoments->hu2)<0.004) &&
     (abs(closeHand.hu3 - huMoments->hu3)<0.0005) && (closeHand.num_def == actualDefects))
   {
    // printf("CLOSE! %d\n", licznik);
     return;
   } else if ((abs(clickHand.hu1 - huMoments->hu1)<0.008) && (abs(clickHand.hu2 - huMoments->hu2)<0.01) &&
     (abs(clickHand.hu3 - huMoments->hu3)<0.001) && (clickHand.num_def == actualDefects))
   { mouse_click();
   //  printf("CLICK! %d\n", licznik);
   }

   licznik ++;

 }

 cvShowImage("Original Image",img);
 cvShowImage("Thresholded Image",thresh);

 if ((cvWaitKey(10) & 255) == 27)
  break;

actualDefects = 0;
if (totalFrames > 200 ) break;
}
/*vReleaseMemStorage(&hull_st);
cvReleaseMemStorage(&defects_st);
cvReleaseImage(&img);
cvReleaseImage(&thresh);*/


}
int main(int argc, const char* argv[]) {

  if (argc != 2) {
    cout<<"Podaj parametr"<<endl;
    return 1;
  }

  CvCapture *capture = cvCaptureFromCAM(0);
  init_windows();
  calibration(capture); 
  if (!endProgram)
    mainLoop(capture, atoi(argv[1]));

  /*cvReleaseCapture(&capture);
  cvDestroyWindow("Original Image");
  cvDestroyWindow("cnt");
  cvDestroyWindow("Thresholded Image");
*/
  printf("correct: %d, total: %d\n", correctFrames, totalFrames);

  return 0;
}

int minimum(int a, int b)
{
  if (a <= b) return a;
  else return b;
}

