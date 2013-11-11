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

using namespace cv;
using std::cout;

/*--------------- SKIN SEGMENTATION ---------------*/
int main () {
  VideoCapture cap(0);

        if(!cap.isOpened()){
                cout << "Errore"; return -1;
        }
        Mat3b frame;
        while(cap.read(frame)){

                /* THRESHOLD ON HSV*/
                cvtColor(frame, frame, CV_BGR2HSV);
                GaussianBlur(frame, frame, Size(7,7), 1, 1);
                //medianBlur(frame, frame, 15);
                for(int r=0; r<frame.rows; ++r){
                        for(int c=0; c<frame.cols; ++c)
                                // 0<H<0.25 - 0.15<S<0.9 - 0.2<V<0.95
                                if( (frame(r,c)[0]>5) && (frame(r,c)[0] < 17) && (frame(r,c)[1]>38) && (frame(r,c)[1]<250) && (frame(r,c)[2]>51) && (frame(r,c)[2]<242) ); 
				// do nothing
                                else for(int i=0; i<3; ++i)        frame(r,c)[i] = 0;
                }

                /* BGR CONVERSION AND THRESHOLD */
                Mat1b frame_gray;
                cvtColor(frame, frame, CV_HSV2BGR);
                cvtColor(frame, frame_gray, CV_BGR2GRAY);
                threshold(frame_gray, frame_gray, 60, 255, CV_THRESH_BINARY);
                morphologyEx(frame_gray, frame_gray, CV_MOP_ERODE, Mat1b(3,3,1), Point(-1, -1), 3);
                morphologyEx(frame_gray, frame_gray, CV_MOP_OPEN, Mat1b(7,7,1), Point(-1, -1), 1);
                morphologyEx(frame_gray, frame_gray, CV_MOP_CLOSE, Mat1b(9,9,1), Point(-1, -1), 1);

                medianBlur(frame_gray, frame_gray, 15);
		imshow("Threshold", frame_gray);

                cvtColor(frame, frame, CV_BGR2HSV);
                resize(frame, frame, Size(), 0.5, 0.5);
                imshow("Video",frame);
                waitKey(5);
        }
}
