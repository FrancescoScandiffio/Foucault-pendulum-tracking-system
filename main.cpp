
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


int colorTracking(string chosenColor){

    VideoCapture capture(0); // open the default camera
    //setting fps rate of video to grab, it should work with the raspberry camera
    capture.set(CAP_PROP_FPS, int(12));

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

        // Convert from BGR to HSV colorspace
        cvtColor(frame, frame_HSV, COLOR_BGR2HSV);

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

        int lastX = posX;

        int lastY = posY;

        posX = moment10/area;
        posY = moment01/area;

        // Print it out for debugging purposes
        printf("position (%d,%d)", posX, posY);

        imshow("thresh", frame_threshold);
        imshow("video", frame);

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
    colorTracking(color);

    return 0;
}




