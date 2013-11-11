#include "opencv/highgui.h"

int main() 
{
  CvCapture *cam = cvCaptureFromCAM(-1);
  
  const char *window = "Example 1";
  cvNamedWindow(window, CV_WINDOW_AUTOSIZE);
  
  while (cvWaitKey(4) == -1) {
    IplImage *frame = cvQueryFrame(cam);
    cvShowImage(window, frame);
  }
  
  cvDestroyAllWindows();
  cvReleaseCapture(&cam);
  
  return 0;
}
