#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <thread>
#include <tuple>
#include "utilityFunctions.h"

#include "guiFunctions.h"

using namespace std;
using namespace cv;

/// Global Variables
Mat frame;
Mat templ;
double myMatrix[3][3];

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
void writeFile();


/** @function main */
int main(int argc, char *argv[]) {

    if(argc > 2) {
        cerr << "Too many arguments" << endl;
        cout << "Execute with option '-h' or '-help' (without quotes) to see all the possible configuration" << endl;
        exit(1);
    }
    if(argc==2){
        if(strcmp(argv[1],"-h")==0 || strcmp(argv[1],"-help")==0 ){
            //TODO aggiungere testo. Da mettere alla fine
            exit(0);

            //help

        }
        if(strcmp(argv[1],"-c")==0 || strcmp(argv[1],"-calibrate")==0 ) {
            calibrateCamera();
            exit(0);
        }

        if(strcmp(argv[1],"-g")==0 || strcmp(argv[1],"-graph")==0 ) {
            //TODO qui metti la chiamata alla tua funzione che disegna il grafico
            exit(0);

        cerr << argv[1]<<" is an unknown option" << endl;
        exit(1);
    }

    cout << "Execute with option '-h' or '-help' (without quotes) to see all the possible configuration" << endl;

    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::duration<double> elapsedDouble;

    /// Load image and template
    templ = imread( "../images/template2.png", 1 );

    ///If on Raspberry:
    // open the default camera
    //VideoCapture capture(0);
    // setting fps rate of video to grab
    //capture.set(CAP_PROP_FPS, int(9));

    ///If working locally:
    VideoCapture capture("../videos/output.mp4");

    Mat originalFrame;
    capture.read(originalFrame);
    Point p1 = Point(32, 0);
    Point p2 = Point(610, 0);
    Point p3 = Point(23, 478);
    Point p4 = Point(600, 475);
    Point p5 = Point(0, 0);

    int frameWidth = originalFrame.size().width;
    int frameHeight = originalFrame.size().height;
    Point2f v1[] = {p1,p2,p3,p4};
    Point p6 = Point(frameWidth, 0);
    Point p7 = Point(0, frameHeight);
    Point p8 = Point(frameWidth, frameHeight);
    Point2f v2[] = {p5,p6,p7,p8};
    Mat matrix = getPerspectiveTransform(v1, v2);
    for(int i=0; i<3; i++)
        for(int j=0; j<3; j++){
            myMatrix[i][j] = matrix.at<double>(i,j);
            cout<<myMatrix[i][j]<<"  ";
        }

    auto t0 = Time::now();
    auto t1 = Time::now();
    elapsedDouble ourElapsed=t1-t0;
    double start = false;

    std::tuple<Mat, int, double> frameInfo;
    int frame_number=0;

    // starting the two threads that handle the frame computation here
    std::thread thread1 (frameComputation, "threadA");
    std::thread thread2 (frameComputation, "threadB");
    std::thread thread3 (writeFile);
    thread1.detach();
    thread2.detach();
    thread3.detach();

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

        frame_number++;
        if (frame_number % 100 ==0)
            cout<<frame_number<<" Elements in FrameA: "<<frameQueue_A.size()<<", elements in FrameB: "<<frameQueue_B.size()<<endl;

    }

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

            // we pass in MatchingMethod a view of the Mat myCroppedFrame. When the functions returns the content of
            // myCroppedFrame will be changed as we want (in order to show the tracking rectangle)
            tuple<double, double> myResult = MatchingMethod(0, 0, whichThread, myFrame);
            tie( position_X, position_Y) = myResult;


            double new_position_x = -1;
            double new_position_y = -1;

            double num = myMatrix[0][0]*position_X+myMatrix[0][1]*position_Y+myMatrix[0][2];
            double dem = myMatrix[2][0]*position_X+myMatrix[2][1]*position_Y+myMatrix[2][2];

            new_position_x = num/dem;

            num = myMatrix[1][0]*position_X+myMatrix[1][1]*position_Y+myMatrix[1][2];
            dem = myMatrix[2][0]*position_X+myMatrix[2][1]*position_Y+myMatrix[2][2];
            new_position_y = height_frame - num/dem;

            // creating the output tuple of the form (cropped_frame, frame_number, time, position_x, position_y)
            resultQueue_X->push(std::make_tuple(myFrame.clone(), myFrameNumber, ourElapsed, new_position_x, new_position_y));
        }
    }
}

void writeFile(){

    /// Getting the current time
    chrono::system_clock::time_point p = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(p);

    /// Setting the right name for the file that will store the centers positions
    std::ostringstream oss;
    oss << "../PendulumCsv/" << ctime(&t) << ".txt";
    std::string file_name = oss.str();

    /// Opening the file where will be saved the coordinates of centers on each frame
    ofstream txt_file (file_name);
    if (txt_file.is_open())
        cout << "Opened file "<< file_name<<"\n";

    txt_file <<"time x y\n";

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

    while(true){

        this_thread::sleep_for(chrono::seconds(5));
        for(int iter=0; iter<100; iter++){

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
                txt_file <<elapsed_A <<" "<<pos_X_A<<" "<<pos_Y_A<<"\n";
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
                txt_file << elapsed_B<<" "<<pos_X_B<<" "<<pos_Y_B<<"\n";
                txt_file.flush();

                // incrementing the expectedFrameNumber because we handled the frame and we can pass to the later one
                expectedFrameNumber++;
                alreadyExtracted_B=false;
                imshow( image_window, extracted_Mat_B);
                // show live and wait for a key with timeout long enough to show images
                waitKey(1);
            }
        }
    }
    txt_file <<ctime(&t)<<endl;;
    // closing the txt file
    txt_file.close();
}

