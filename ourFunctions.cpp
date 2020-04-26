//
// Created by francis on 26/04/20.
//
#include "ourFunctions.h"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

int oldMain(){

    //TODO uncomment the following lines on Raspberry to get the camera and set fps rate
    VideoCapture video(0); // open the default camera
    video.set(CAP_PROP_FPS, int(10));

    if(!video.isOpened()){
        // check if we succeeded
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    // Start grabbing frames
    Mat frame;
    cout << "Start grabbing" << endl
         << "Press any key to terminate" << endl;

    for (;;) {
        // wait for a new frame from camera and store it into 'frame'
        video.read(frame);
        // check if we succeeded
        if (frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }
        // show live and wait for a key with timeout long enough to show images
        imshow("Live", frame);
        if (waitKey(5) >= 0)
            break;
    }
    // the camera will be de-initialized automatically in VideoCapture destructor
    return 0;
}


int meanShiftTest(){
    cv::VideoCapture capture("test.avi");
    if (!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open file!" << endl;
        return 0;
    }
    Mat frame, roi, hsv_roi, mask;
    // take first frame of the video
    capture >> frame;
    // setup initial location of window
    Rect track_window(300, 200, 100, 50); // simply hardcoded the values
    // set up the ROI for tracking
    roi = frame(track_window);
    cvtColor(roi, hsv_roi, COLOR_BGR2HSV);
    inRange(hsv_roi, Scalar(0, 60, 32), Scalar(180, 255, 255), mask);
    float range_[] = {0, 180};
    const float* range[] = {range_};
    Mat roi_hist;
    int histSize[] = {180};
    int channels[] = {0};
    calcHist(&hsv_roi, 1, channels, mask, roi_hist, 1, histSize, range);
    normalize(roi_hist, roi_hist, 0, 255, NORM_MINMAX);
    // Setup the termination criteria, either 10 iteration or move by atleast 1 pt
    TermCriteria term_crit(TermCriteria::EPS | TermCriteria::COUNT, 10, 1);
    while(true){
        Mat hsv, dst;
        capture >> frame;
        if (frame.empty())
            break;
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        calcBackProject(&hsv, 1, channels, roi_hist, dst, range);
        // apply camshift to get the new location
        RotatedRect rot_rect = CamShift(dst, track_window, term_crit);
        // Draw it on image
        Point2f points[4];
        rot_rect.points(points);
        for (int i = 0; i < 4; i++)
            line(frame, points[i], points[(i+1)%4], 255, 2);
        imshow("img2", frame);
        int keyboard = waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;
    }

    return 0;
}
