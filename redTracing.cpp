#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
using namespace cv;

IplImage* GetThresholdedImage(IplImage* img)
{
    // Convert the image into an HSV image
    IplImage* imgHSV = cvCreateImage(cvGetSize(img), 8, 3);
    cvCvtColor(img, imgHSV, CV_BGR2HSV);
    IplImage* imgThreshed = cvCreateImage(cvGetSize(img), 8, 1);
    cvInRangeS(imgHSV, cvScalar(140, 139, 120), cvScalar(179, 255, 255), imgThreshed);
    cvReleaseImage(&imgHSV);
    return imgThreshed;
}

int main()
{
    // Initialize capturing live feed from the camera
    CvCapture* capture = 0;
    capture = cvCaptureFromCAM(0);

    // The two windows we'll be using
    cvNamedWindow("video");
    cvNamedWindow("thresh");

    // This image holds the "scribble" data...
    // the tracked positions of the ball
    IplImage* imgScribble = NULL;

    while(true)
    {
        // Will hold a frame captured from the camera
        IplImage* frame = 0;
        frame = cvQueryFrame(capture);
        // If we couldn't grab a frame... quit
        if(!frame)
            break;
        // If this is the first frame, we need to initialize it
        if(imgScribble == NULL)
        {
            imgScribble = cvCreateImage(cvGetSize(frame), 8, 3);
        }
        // Holds the yellow thresholded image (yellow = white, rest = black)
        IplImage* imgYellowThresh = GetThresholdedImage(frame);

        // Calculate the moments to estimate the position of the ball
        CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));

        cvMoments(imgYellowThresh, moments, 1);

        // The actual moment values
        double moment10 = cvGetSpatialMoment(moments, 1, 0);
        double moment01 = cvGetSpatialMoment(moments, 0, 1);

        double area = cvGetCentralMoment(moments, 0, 0);

        // Holding the last and current ball positions
        static int posX = 0;

        static int posY = 0;

        int lastX = posX;

        int lastY = posY;

        posX = moment10/area;
        posY = moment01/area;

        // Print it out for debugging purposes
        printf("position (%d,%d)", posX, posY);


        // Add the scribbling image and the frame...
        cvAdd(frame, imgScribble, frame);
        cvShowImage("thresh", imgYellowThresh);
        cvShowImage("video", frame);

        // Wait for a keypress
        int c = cvWaitKey(10);
        if(c!=-1)
        {
            // If pressed, break out of the loop
            break;
        }

        // Release the thresholded image+moments... we need no memory leaks.. please
        cvReleaseImage(&imgYellowThresh);
        delete moments;
    }

    // We're done using the camera. Other applications can now use it
    cvReleaseCapture(&capture);
    return 0;
}

