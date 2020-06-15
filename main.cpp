#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <thread>
#include <tuple>
#include "utilityFunctions.h"

using namespace std;
using namespace cv;

/// Global Variables
Mat frame;
Mat templ;
Mat cropped_frame;
Mat result;
Point position;     // locations of recognized centers
String image_window = "Source Image";

int match_method=5;
int max_Trackbar = 5;
int frame_number=0;

// frame height of raspberry
int height_frame=480;

/// Function Headers
pair<double, double> MatchingMethod( int, void* );
std::tuple<int, double, double, double> frameComputation(Mat, int, double);


/** @function main */
int main() {
    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::duration<double> elapsedDouble;

    /// Load image and template
    templ = imread( "../images/template2.png", 1 );

    ///If on Raspberry:
    // open the default camera
    //VideoCapture capture(0);
    // setting fps rate of video to grab
    //capture.set(CAP_PROP_FPS, int(30));

    ///If working locally:
    VideoCapture capture("../videos/output.mp4");

    /// Create windows
    namedWindow( image_window );
    //namedWindow( result_window );

    /// Create Trackbar
    String trackbar_label = "Method: \n 0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED";
    //createTrackbar( trackbar_label, image_window, &match_method, max_Trackbar, MatchingMethod );

    /// Getting the current time
    chrono::system_clock::time_point p = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(p);
    
    auto t0 = Time::now();
    auto t1 = Time::now();
    elapsedDouble ourElapsed=t1-t0;
    double start = false;

    /// Setting the right name for the file that will store the centers positions
    std::ostringstream oss;
    oss << "../" << ctime(&t) << ".txt";
    std::string file_name = oss.str();

    /// Opening the file where will be saved the coordinates of centers on each frame
    ofstream txt_file (file_name);
    if (txt_file.is_open())
        cout << "Opened file "<< file_name<<"\n";

    /// This is how we handle the frames
    // creating a vector that stores the frames to be computed
    std::vector<Mat> frameBuffer;
    double pos_x;
    double pos_y;
    double our_time;
    int our_frame;
    std::tuple<int, double, double, double> result;

    while(true) {

        // wait for a new frame from camera and store it into 'frame'
        capture.read(frame);

        /// Getting the current timestamp to save it to the file
        t1 = Time::now();
        if(!start){
            t0 = t1;
            start=true;
        }
        ourElapsed = t1 - t0;

        try{
            // the result is in the form (frame_number, ourElapsed, position_x, position_y)
            result = frameComputation(frame, frame_number, ourElapsed.count());
            // unpacking values in tuple
            tie(our_frame, our_time, pos_x, pos_y) = result;

        }catch(...) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        // saving to txt the positions found in MatchingMethod
        txt_file <<"time: "<< our_time <<" position ("<< pos_x<<", "<<pos_y<<")\n";
        txt_file.flush();

        //imshow( result_window, result );
        imshow( image_window, cropped_frame );

        frame_number++;
        // show live and wait for a key with timeout long enough to show images
        waitKey(1);
    }
    txt_file <<ctime(&t)<<endl;;
    // closing the txt file
    txt_file.close();

    // the camera will be de-initialized automatically in VideoCapture destructor
    return 0;
}

/**
 * @function MatchingMethod
 * @brief Trackbar callback
 */
pair<double, double> MatchingMethod( int, void* ) {

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

    pair<double, double> positions;
    positions.first=position.x;
    positions.second=position.y;

    return positions;
}

tuple<int, double, double, double> frameComputation(Mat frame, int frame_number, double ourElapsed){
    // check if we succeeded
    if (frame.empty()) {
        throw;
    }

    // getting frame size
    Size s = frame.size();
    int rows = s.height;
    int cols = s.width;

    // Cropping the frame to exclude unwanted area on video
    // The area of interest is of the form Rect(Point(x, y), Point(x,y)) in which the first point indicates the
    // top left corner of the box
    frame(Rect(Point(30, 0), Point(cols-30,rows))).copyTo(cropped_frame);

    pair<double, double> result = MatchingMethod(0, 0);

    // updating with proper value. At the moment y indicates the distance from the point to the top of the image
    // now y is the distance from point to image bottom
    double new_position_y = height_frame - result.second;

    // returning an array containing (frame_number, time, position_x, position_y)
    return std::make_tuple(frame_number, ourElapsed, result.first, new_position_y);
}
