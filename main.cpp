#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "ourFunctions.h"

using namespace cv;
using namespace std;


int colorTrackingAndCircleDetection(string chosenColor){

    ///If on Raspberry:
    // open the default camera
    //VideoCapture capture(0);
    // setting fps rate of video to grab
    //capture.set(CAP_PROP_FPS, int(12));

    ///If working locally:
    VideoCapture capture("../videos/prova.h264");

    if(!capture.isOpened()){
        // check if we succeeded
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    // The two windows we'll be using
    namedWindow("video");
    namedWindow("thresh");

    // Creating frame objects
    Mat frame;
    Mat cropped_frame;
    Mat frame_HSV;
    Mat frame_threshold;
    while(true){

        // wait for a new frame from camera and store it into 'frame'
        capture.read(frame);
        // check if we succeeded
        if (frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        // getting frame size
        Size s = frame.size();
        int rows = s.height;
        int cols = s.width;

        // Cropping the frame to exclude unwanted area on video
        // The area of interest is of the form Rect(Point(x, y), Point(x,y)) in which the first point indicates the
        // top left corner of the box
        frame(Rect(Point(40, 0), Point(cols-20,rows))).copyTo(cropped_frame);
        //imshow("cropped_image", cropped_frame);

        // Convert from BGR to HSV colorspace
        cvtColor(cropped_frame, frame_HSV, COLOR_BGR2HSV);

        // Detect the object based on HSV Range Values
        if(chosenColor.compare("green")){
            inRange(frame_HSV, Scalar(140, 139, 120), Scalar(179, 255, 255), frame_threshold);
        }
        if(chosenColor.compare("red")){
            inRange(frame_HSV, Scalar(52, 68, 39), Scalar(92, 163, 178), frame_threshold);
        }

        // Calculate the moments to estimate the position of the ball
        Moments moment;
        moment=moments(frame_threshold);

        // The actual moment values
        double moment10 =moment.m10;
        double moment01 =moment.m01;
        double area = moment.m00;

        // Holding the last and current ball positions
        static int posX = 0;
        static int posY = 0;

        posX = moment10/area;
        posY = moment01/area;

        // Print it out for debugging purposes
        printf("position (%d,%d)", posX, posY);

        imshow("thresh", frame_threshold);
        imshow("video", frame);

        // Reduce the noise so we avoid false circle detection
        GaussianBlur( frame_threshold, frame_threshold, Size(9, 9), 2, 2 );
        vector<Vec3f> circles;

        // Apply the Hough Transform to find the circles
        HoughCircles( frame_threshold, circles, HOUGH_GRADIENT, 1, frame_threshold.rows/8, 200, 100, 0, 0 );

        // Draw the circles detected
        for( size_t i = 0; i < circles.size(); i++ ) {
            Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            // circle center
            circle( cropped_frame, center, 3, Scalar(0,255,0), -1, 8, 0 );
            // circle outline
            circle( cropped_frame, center, radius, Scalar(0,0,255), 3, 8, 0 );
        }
        // Show your results
        namedWindow("Modified threshold with gaussian filter");
        imshow("Modified threshold with gaussian filter",frame_threshold);

        namedWindow( "Hough Circle Transform", WINDOW_AUTOSIZE );
        imshow( "Hough Circle Transform", cropped_frame );


        // show live and wait for a key with timeout long enough to show images
        if (waitKey(5) >= 0)
            break;
    }

    // the camera will be de-initialized automatically in VideoCapture destructor
    return 0;
}

int main(int, char**){
    //oldMain();
    //meanShiftTest();

    //string color("green");
    string color("red");
    //colorTracking(color);
    //circleDetector();
    colorTrackingAndCircleDetection(color);
    return 0;
}
