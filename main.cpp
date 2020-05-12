#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

/// Global Variables
Mat img; Mat templ; Mat result;
String image_window = "Source Image";
String result_window = "Result window";

int match_method;
int max_Trackbar = 5;

/// Function Headers
void MatchingMethod( int, void* );

/** @function main */
int main()
{
    /// Load image and template
  //  img = imread( "../images/pendoloFermo2.png", 1 );
    templ = imread( "../images/template2.png", 1 );
    VideoCapture capture(0);
    // setting fps rate of video to grab
    capture.set(CAP_PROP_FPS, int(12));

    /// Create windows
    namedWindow( image_window, WINDOW_AUTOSIZE );
    namedWindow( result_window, WINDOW_AUTOSIZE );

    /// Create Trackbar
    String trackbar_label = "Method: \n 0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED";
    createTrackbar( trackbar_label, image_window, &match_method, max_Trackbar, MatchingMethod );

    while(true) {

        // wait for a new frame from camera and store it into 'frame'
        capture.read(img);
        // check if we succeeded
        if (img.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }
        MatchingMethod(0, 0);
        imshow( result_window, result );
        imshow( image_window, img );

        waitKey(1);

    }

    return 0;
}

/**
 * @function MatchingMethod
 * @brief Trackbar callback
 */
void MatchingMethod( int, void* )
{
    /// Source image to display


    /// Create the result matrix
    int result_cols =  img.cols - templ.cols + 1;
    int result_rows = img.rows - templ.rows + 1;

    result.create( result_rows, result_cols, CV_32FC1 );

    /// Do the Matching and Normalize
    matchTemplate( img, templ, result, match_method );
    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    /// Localizing the best match with minMaxLoc
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    Point matchLoc;

    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if( match_method  == TM_SQDIFF || match_method == TM_SQDIFF_NORMED )
    { matchLoc = minLoc; }
    else
    { matchLoc = maxLoc; }

    /// Show me what you got
    rectangle( img, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );
    rectangle( result, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );



    return;
}
