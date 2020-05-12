#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <fstream>

using namespace std;
using namespace cv;

/// Global Variables
Mat frame;
Mat templ;
Mat cropped_frame;
Mat result;
Point position;     // locations of recognized centers
String image_window = "Source Image";
String result_window = "Result window";
ofstream txt_file;

int match_method;
int max_Trackbar = 5;
int frame_number=0;

int position_x=-1;
int position_y=-1;

/// Function Headers
void MatchingMethod( int, void* );


/** @function main */
int main() {

    /// Load image and template
    templ = imread( "../images/template2.png", 1 );

    ///If on Raspberry:
    // open the default camera
    //VideoCapture capture(0);
    // setting fps rate of video to grab
    //capture.set(CAP_PROP_FPS, int(12));

    ///If working locally:
    //img = imread( "../images/pendoloFermo2.png", 1 );
    VideoCapture capture("../videos/output.mp4");


    /// Create windows
    namedWindow( image_window );
    //namedWindow( result_window );

    /// Create Trackbar
    String trackbar_label = "Method: \n 0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED";
    createTrackbar( trackbar_label, image_window, &match_method, max_Trackbar, MatchingMethod );

    // Opening the file where will be saved the coordinates of centers on each frame
    ofstream txt_file ("../positions.txt");
    if (!txt_file.is_open())
        cout << "Unable to open file positions.txt";

    while(true) {

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

        MatchingMethod(0, 0);

        // saving to txt the positions found in MatchingMethod
        txt_file << "Frame "<< frame_number << ", position ("<< position_x<<", "<<position_y<<")\n";

        //imshow( result_window, result );
        imshow( image_window, cropped_frame );


        frame_number++;
        // show live and wait for a key with timeout long enough to show images
        waitKey(1);
    }

    // closing the txt file
    txt_file.close();

    // the camera will be de-initialized automatically in VideoCapture destructor
    return 0;
}

/**
 * @function MatchingMethod
 * @brief Trackbar callback
 */
void MatchingMethod( int, void* ) {

    /// Create the result matrix
    int result_cols =  cropped_frame.cols - templ.cols + 1;
    int result_rows = cropped_frame.rows - templ.rows + 1;

    result.create( result_rows, result_cols, CV_32FC1 );

    /// Do the Matching and Normalize
    matchTemplate( cropped_frame, templ, result, match_method );
    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    /// Localizing the best match with minMaxLoc
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    Point matchLoc;

    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if( match_method  == TM_SQDIFF || match_method == TM_SQDIFF_NORMED ) {
        matchLoc = minLoc;
    } else {
        matchLoc = maxLoc;
    }

    /// Show me what you got
    rectangle( cropped_frame, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );
    rectangle( result, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );

    // saving the center back to txt file
    position = Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows );
    printf("Position of center on frame %d is (%d, %d)\n", frame_number, position.x, position.y);
    position_x=position.x;
    position_y=position.y;

    return;
}

