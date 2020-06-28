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

String image_window = "Source Image";
Mat result_A;
Mat result_B;

int match_method=5;
int max_Trackbar = 5;

// frame height of raspberry
int height_frame=480;

/// This is how we handle the frames, using two queues for the two threads
std::queue<std::tuple<Mat, int, double>> frameQueue_A;
std::queue<std::tuple<Mat, int, double>> frameQueue_B;

std::queue<tuple<Mat, int, double, double, double>> resultQueue_A;
std::queue<tuple<Mat, int, double, double, double>> resultQueue_B;

/// Function Headers
tuple<double, double> MatchingMethod( int, void*, const string&, Mat croppedFrame);
void frameComputation(const string& whichThread);


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

    std::tuple<Mat, int, double> frameInfo;

    int frame_number=0;
    int expectedFrameNumber=0;
    bool alreadyExtracted_A = false;
    bool alreadyExtracted_B = false;

    int frameNumber_A=-1;
    int frameNumber_B=-1;
    double elapsed_A=-1.0;
    double elapsed_B=-1.0;
    double pos_X_A;
    double pos_X_B;
    double pos_Y_A;
    double pos_Y_B;
    Mat extracted_Mat_A;
    Mat extracted_Mat_B;

    // starting the two threads that handle the frame computation here
    std::thread thread1 (frameComputation, "threadA");
    std::thread thread2 (frameComputation, "threadB");

    while(true) {

        // wait for a new frame from camera and store it into 'frame'
        capture.read(frame);
        if (frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        /// Getting the current timestamp to save it to the file
        t1 = Time::now();
        if(!start){
            t0 = t1;
            start=true;
        }
        ourElapsed = t1 - t0;

        // preparing frame info to pass on the proper thread
        frameInfo = std::make_tuple(frame.clone(), frame_number, ourElapsed.count());

        // equally distributing the work on the two queues
        if (frame_number % 2 ==0){
            frameQueue_A.push(frameInfo);
        }else{
            frameQueue_B.push(frameInfo);
        }

        // extracting elements from the first queue if not already done in the previous iteration without writing the
        // result to the txt file
        if(!alreadyExtracted_A){
            if(!resultQueue_A.empty()){
                tie(extracted_Mat_A, frameNumber_A, elapsed_A, pos_X_A, pos_Y_A) = resultQueue_A.front();
                resultQueue_A.pop();
                // we extracted the element from the queue
                alreadyExtracted_A=true;
            }
        }
        // if the frame extracted from resultQueue_A is the frame that has to be written in the txt (and we know it by
        // looking at the current frame number expected to be written)
        if(expectedFrameNumber==frameNumber_A){
            // saving to txt the positions found in MatchingMethod
            txt_file <<"time: "<< elapsed_A <<" position ("<< pos_X_A<<", "<<pos_Y_A<<")\n";
            txt_file.flush();

            // incrementing the expectedFrameNumber because we handled the frame and we can pass to the later one
            expectedFrameNumber++;
            alreadyExtracted_A=false;
            imshow( image_window, extracted_Mat_A);
            // show live and wait for a key with timeout long enough to show images
            waitKey(1);
        }
        // doing the same for the other queue and thread
        if(!alreadyExtracted_B){
            if(!resultQueue_B.empty()){
                tie(extracted_Mat_B, frameNumber_B, elapsed_B, pos_X_B, pos_Y_B) = resultQueue_B.front();
                resultQueue_B.pop();
                // we extracted the element from the queue
                alreadyExtracted_B=true;
            }
        }
        if(expectedFrameNumber==frameNumber_B){
            // saving to txt the positions found in MatchingMethod
            txt_file <<"time: "<< elapsed_B <<" position ("<< pos_X_B<<", "<<pos_Y_B<<")\n";
            txt_file.flush();

            // incrementing the expectedFrameNumber because we handled the frame and we can pass to the later one
            expectedFrameNumber++;
            alreadyExtracted_B=false;
            imshow( image_window, extracted_Mat_B);
            // show live and wait for a key with timeout long enough to show images
            waitKey(1);
        }

        frame_number++;

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
tuple<double, double> MatchingMethod( int, void*, const string& whichThread, Mat croppedFrame) {
    Mat *result_X;
    if(whichThread=="threadA"){
        result_X = &result_A;
    }else{
        result_X = &result_B;
    }

    /// Create the result matrix
    int result_cols =  croppedFrame.cols - templ.cols + 1;
    int result_rows = croppedFrame.rows - templ.rows + 1;

    result_X->create(result_rows, result_cols, CV_32FC1);

    /// Do the Matching and Normalize
    matchTemplate( croppedFrame, templ, *result_X, match_method );
    normalize( *result_X, *result_X, 0, 1, NORM_MINMAX, -1, Mat() );

    /// Localizing the best match with minMaxLoc
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    Point matchLoc;

    minMaxLoc( *result_X, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if( match_method  == TM_SQDIFF || match_method == TM_SQDIFF_NORMED ) {
        matchLoc = minLoc;
    } else {
        matchLoc = maxLoc;
    }

    /// Show me what you got
    rectangle( croppedFrame, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );
    rectangle( *result_X, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );

    // computing and returning the position
    return std::make_tuple(matchLoc.x + templ.cols, matchLoc.y + templ.rows);
}

void frameComputation(const string& whichThread){
    Mat myFrame;
    Mat myCroppedFrame;
    int myFrameNumber;
    double ourElapsed;
    std::queue<std::tuple<Mat, int, double>> *frameQueue_X;
    std::queue<tuple<Mat, int, double, double, double>> *resultQueue_X;
    double position_X;
    double position_Y;

    // taking the reference of the proper queues
    if(whichThread=="threadA"){
        frameQueue_X = &frameQueue_A;
        resultQueue_X = &resultQueue_A;
    }else{
        frameQueue_X=&frameQueue_B;
        resultQueue_X=&resultQueue_B;
    }

    // iterating through the queue
    while(true){
        if(!frameQueue_X->empty()){
            //extracting first element available from queue and unpacking the tuple content
            tie(myFrame, myFrameNumber, ourElapsed) = frameQueue_X->front();
            frameQueue_X->pop();

            // check if we succeeded
            if (myFrame.empty()) {
                cerr << "ERROR! blank frame grabbed\n";
                break;
            }

            // getting frame size
            Size s = myFrame.size();
            int rows = s.height;
            int cols = s.width;

            // Cropping the frame to exclude unwanted area on video
            // The area of interest is of the form Rect(Point(x, y), Point(x,y)) in which the first point indicates the
            // top left corner of the box
            myFrame(Rect(Point(30, 0), Point(cols-30,rows))).copyTo(myCroppedFrame);

            // we pass in MatchingMethod a view of the Mat myCroppedFrame. When the functions returns the content of
            // myCroppedFrame will be changed as we want (in order to show the tracking rectangle)
            tuple<double, double> myResult = MatchingMethod(0, 0, whichThread, myCroppedFrame);
            tie( position_X, position_Y) = myResult;

            // updating with proper value. At the moment y indicates the distance from the point to the top of the image
            // now y is the distance from point to image bottom
            double new_position_y = height_frame - position_Y;

            // creating the output tuple of the form (cropped_frame, frame_number, time, position_x, position_y)
            resultQueue_X->push(std::make_tuple(myCroppedFrame.clone(), myFrameNumber, ourElapsed, position_X, new_position_y));
        }
    }
}
