#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <tuple>
#include <time.h>
#include <iostream>
#include <string>
#include "guiFunctions.h"
//even if CLion marks it as unused, the following include is required:
#include <fstream>


using namespace std;
using namespace cv;

bool is_graph_activated=false;
int graphPoints= 100;
int* ptrGraphPoints = &graphPoints;
int expectedFrameNumber=0;
void toggleView(int status, void* data){
    if(status==0)
        is_graph_activated=false;
    else
        is_graph_activated=true;

}

void checkTrackbar(int trackPos, void* data) {
    if(trackPos<1)
        *ptrGraphPoints = 1;
}

/// Global Variables
Mat frame;
Mat templ;
double myMatrix[3][3];

String image_window = "Window";
Mat result_A;
Mat result_B;

std::queue<Point2d> pointsVector;
Mat plot_image;

int match_method=5;

/// This is how we handle the frames, using two queues for the two threads
std::queue<std::tuple<Mat, int, double>> frameQueue_A;
std::queue<std::tuple<Mat, int, double>> frameQueue_B;

std::queue<tuple<Mat, int, double, double, double>> resultQueue_A;
std::queue<tuple<Mat, int, double, double, double>> resultQueue_B;

/// to make request for extracting from queue the inserted frames to write back to file
struct request{
    std::mutex mx;
    std::condition_variable cv;
};
request requestA;
request requestB;

/// Function Headers
tuple<double, double> MatchingMethod( int, void*, const string&, Mat croppedFrame);
void frameComputation(const string& whichThread);
[[noreturn]] void writeFile(const bool perspectiveCorrection);


/** @function main */
int main(int argc, char *argv[]) {

    string calibrationFile = "calibration.txt";
    string templateFile = "testTemplate2.png";

    if (argc > 2) {
        cerr << "Too many arguments" << endl;
        cout << "Execute with option '-h' or '-help' (without quotes) to see all the possible configuration" << endl;
        exit(1);
    }
    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-help") == 0) {
            printf("Program Options:\n");
            printf("Execute the program with none or one of the following arguments.\n");
            printf("-c  or  -calibrate		To calibrate the camera.\n");
            printf("-g  or  -graph          To display a 2D graph of coordinates from CSV file.\n");
            printf("-h  or  -help			To show this message.\n");
            exit(0);
        }

        if (strcmp(argv[1], "-g") == 0 || strcmp(argv[1], "-graph") == 0) {
            drawGraph();
            exit(0);
        }

        if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "-calibrate") == 0) {
            calibrateCamera();
            exit(0);
        }

        cerr << argv[1] << " is an unknown option" << endl;
        exit(1);
    }

    cout << "Execute with option '-h' or '-help' (without quotes) to see all the possible arguments" << endl;

    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::duration<double> TimeCast;

    /// Load image and template

    if(fileExist(templateFile))
        templ = imread(templateFile, 1);
    else {
        cerr << "ERROR. Can't find template file with name: "<< templateFile<<" \n Closing..." << endl;
        cerr.flush();
        exit(-1);
    }

    ///If on Raspberry:
    // open the default camera
    VideoCapture capture(0);
    // setting fps rate of video to grab
    capture.set(CAP_PROP_FPS, int(9));

    ///If working locally:
    //VideoCapture capture("../videos/output.mp4");


    Mat originalFrame;
    capture.read(originalFrame);
    int frameWidth = originalFrame.size().width;
    int frameHeight = originalFrame.size().height;
    plot_image = Mat::zeros( frameHeight, frameWidth, CV_8UC3);



    ///ask if the user wants perspectiveCorrection
    string answer;
    do{
        cout<<"Do you want the program to apply perspective correction? [y/n]"<<endl;
        getline(cin,answer);
    }
    while(answer!="y" && answer!="n");

    bool perspectiveCorrection = false;
    if(answer=="y") {
        perspectiveCorrection = true;
        Point2f inputPoints[4];
        if(fileExist(calibrationFile)) {
            fstream file(calibrationFile);
            string textLine;
            int currentLine = 0;
            int posX, posY;
            string valX,valY;
            while (getline(file, textLine) && currentLine<4) {
                try {
                    int dotComma = textLine.find(";");
                    valX = textLine.substr(0, dotComma);
                    valY = textLine.substr(dotComma + 1, textLine.length());

                    posX = stoi(valX);
                    posY = stoi(valY);
                    inputPoints[currentLine] = Point(posX, posY);
                    currentLine++;

                } catch (...) {
                    cerr << "ERROR: something went wrong while parsing old points from "<<calibrationFile<<endl;
                    cerr<< " Wrong value: x "<<valX<<" Y "<<valY;
                    cerr << "\nPlease, retry calibration process";
                    cout<<"Closing...";
                    cerr.flush();
                    file.close();
                    exit(-2);
                }
            }
            cout<<"Calibration file ok.";

        }
        else{
            cerr<<"Configuration file "<<calibrationFile<<" not found"<<endl;
            cerr << "\nPlease, retry calibration process";
            cout<<"Closing...";
            cerr.flush();
            exit(-3);
        }

        Point p5 = Point(0, 0);
        int ABx = abs(inputPoints[0].x - inputPoints[1].x);
        int CDx =  abs(inputPoints[2].x - inputPoints[3].x);
        int perspectiveWidth = ABx < CDx ? CDx : ABx;
        int ACy = abs(inputPoints[0].y - inputPoints[2].y);
        int BDy =  abs(inputPoints[1].y - inputPoints[3].y);
        int perspectiveHeight = ACy < BDy ? BDy : ACy;

        Point p6 = Point(perspectiveWidth, 0);
        Point p7 = Point(0, perspectiveHeight);
        Point p8 = Point(perspectiveWidth, perspectiveHeight);
        String transform = "New View";

        Point2f outPoints[] = {p5, p6, p7, p8};
        Mat matrix = getPerspectiveTransform(inputPoints, outPoints);
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                myMatrix[i][j] = matrix.at<double>(i, j);
    }

    /// starting the two threads that handle the frame computation here
    std::thread thread1 (frameComputation, "threadA");
    std::thread thread2 (frameComputation, "threadB");
    std::thread thread3 (writeFile,perspectiveCorrection);
    thread1.detach();
    thread2.detach();
    thread3.detach();

    auto startTime = Time::now();
    auto newTime = Time::now();
    TimeCast elapsed=newTime - startTime;
    bool start = false;
    std::tuple<Mat, int, double> frameInfo;
    int frame_number=0;



    /// initializing Mat of graph of movements
    plot_image = cv::Scalar(255, 255, 255);

    while(true) {

        /// wait for a new frame from camera and store it into 'frame'
        capture.read(frame);
        if (frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        /// Getting the current timestamp to save it to the file
        newTime = Time::now();
        if(!start){
            startTime = newTime;
            start=true;
        }
        elapsed = newTime - startTime;

        /// preparing frame info to pass on the proper thread
        frameInfo = std::make_tuple(frame.clone(), frame_number, elapsed.count());

        /// equally distributing the work on the two queues
        if (frame_number % 2 ==0){
            frameQueue_A.push(frameInfo);
        }else{
            frameQueue_B.push(frameInfo);
        }

        frame_number++;
        if (frame_number % 100 ==0)
            cout<<frame_number<<" Elements in FrameA: "<<frameQueue_A.size()<<", elements in FrameB: "<<frameQueue_B.size()<<endl;

    }

    /// the camera will be de-initialized automatically in VideoCapture destructor
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
    circle(croppedFrame,Point(matchLoc.x + templ.cols/2, matchLoc.y + templ.rows/2),0,Scalar( 0, 255, 0 ),5);

    /// computing and returning the position
    return std::make_tuple(matchLoc.x + templ.cols/2, matchLoc.y + templ.rows/2);
}

void frameComputation(const string& whichThread){
    Mat myFrame;
    int myFrameNumber;
    double ourElapsed;
    std::queue<std::tuple<Mat, int, double>> *frameQueue_X;
    std::queue<tuple<Mat, int, double, double, double>> *resultQueue_X;
    double position_X;
    double position_Y;
    request *requestX;

    // taking the reference of the proper queues
    if(whichThread=="threadA"){
        frameQueue_X = &frameQueue_A;
        resultQueue_X = &resultQueue_A;
        requestX = &requestA;
    }else{
        frameQueue_X = &frameQueue_B;
        resultQueue_X = &resultQueue_B;
        requestX = &requestB;
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
            tuple<double, double> myResult;

            // we pass in MatchingMethod a view of the Mat myCroppedFrame. When the functions returns the content of
            // myCroppedFrame will be changed as we want (in order to show the tracking rectangle)
            myResult = MatchingMethod(0, 0, whichThread, myFrame);
            tie( position_X, position_Y) = myResult;
            cout.flush();


            // creating the output tuple of the form (cropped_frame, frame_number, time, position_x, position_y)
            resultQueue_X->push(std::make_tuple(myFrame.clone(), myFrameNumber, ourElapsed, position_X, position_Y));

            // acquiring the lock and notify the consumer (writeFile) of added element to the queue
            std::lock_guard<std::mutex> lock(requestX->mx);
            requestX->cv.notify_one();
        }
    }
}

[[noreturn]] void writeFile(const bool perspectiveCorrection){

    time_t theTime = time(NULL);
    struct tm *aTime = localtime(&theTime);

    /// Getting the current time
    int day = aTime->tm_mday;
    int month = aTime->tm_mon + 1; // Month is 0 - 11, add 1 to get a jan-dec 1-12
    int year = aTime->tm_year + 1900; // Year is # years since 1900
    int hour = aTime->tm_hour;
    int min = aTime->tm_min;


    /// Setting the right name for the file that will store the centers positions
    /// Y means "yes perspective correction"
    std::ostringstream oss;
    std::ostringstream oss_Y;
    oss << "../PendulumCsv/" <<year<<"_"<<month<<"_"<<day<<"_"<<hour<<"_"<<min<< "_N.csv";

    if(perspectiveCorrection)
        oss_Y << "../PendulumCsv/" <<year<<"_"<<month<<"_"<<day<<"_"<<hour<<"_"<<min<< "_Y.csv";

    std::string file_name = oss.str();
    std::string file_name_Y = oss_Y.str();

    /// Opening the file where will be saved the coordinates of centers on each frame
    ofstream txt_file (file_name);
    if (txt_file.is_open())
        cout << "Opened file "<< file_name<<"\n";

    ofstream txt_file_Y (file_name_Y);
    if (txt_file_Y.is_open())
        cout << "Opened file "<< file_name_Y<<"\n";

    txt_file <<"time;x;y\n";
    txt_file_Y <<"time;x;y\n";

    int frameNumber_X=-1;
    double elapsed_X=-1.0;
    double pos_X;
    double pos_Y;
    std::queue<tuple<Mat, int, double, double, double>> *resultQueue_X;
    Mat extracted_Mat_X;
    Point2d lastPoint;

    namedWindow(image_window,WINDOW_GUI_EXPANDED);
    createButton("Show graph", toggleView, NULL, QT_CHECKBOX, 0);
    createTrackbar("Number of graph points",image_window, ptrGraphPoints,1000,checkTrackbar,NULL);


    // variables for the perspective
    double new_position_x;
    double new_position_y;
    double num;
    double dem;

    while(true){

        // checking if the expected frame number is even or odd (if even we extract from the resultQueue_A, if
        // odd from the resultQueue_B)
        if (expectedFrameNumber % 2 ==0){
            //waiting until some elements is added to the queue of results
            std::unique_lock<std::mutex> lock(requestA.mx);
            requestA.cv.wait(lock, []{return !resultQueue_A.empty();});
            resultQueue_X = &resultQueue_A;
        }else{
            //waiting until some elements is added to the queue of results
            std::unique_lock<std::mutex> lock(requestB.mx);
            requestB.cv.wait(lock, []{return !resultQueue_B.empty();});
            resultQueue_X = &resultQueue_B;
        }

        //a frame has been added to the queue we were waiting for, now we can extract the desired frame and related variables
        tie(extracted_Mat_X, frameNumber_X, elapsed_X, pos_X, pos_Y) = resultQueue_X->front();

        resultQueue_X->pop();

        /// Perspective adjustments, only if required by the user
        if(perspectiveCorrection) {
            num = myMatrix[0][0] * pos_X + myMatrix[0][1] * pos_Y + myMatrix[0][2];
            dem = myMatrix[2][0] * pos_X + myMatrix[2][1] * pos_Y + myMatrix[2][2];

            new_position_x = num / dem;
            num = myMatrix[1][0] * pos_X + myMatrix[1][1] * pos_Y + myMatrix[1][2];
            dem = myMatrix[2][0] * pos_X + myMatrix[2][1] * pos_Y + myMatrix[2][2];
            new_position_y = num / dem;
            txt_file_Y << fixed << elapsed_X << ";" << (int)(new_position_x) << ";" << (int)(new_position_y) << "\n";
            txt_file_Y.flush();
        }
            /// saving to txt the positions found in MatchingMethod

        txt_file << fixed << elapsed_X << ";" << (int)(pos_X) << ";" << (int)(pos_Y) << "\n";
        txt_file.flush();

        /// we add the OLD point to the pointsVector to be shown on plot_image Mat
        pointsVector.push(Point2d(pos_X,pos_Y));
        /// we start displaying the points
        cv::line(plot_image, Point2d(pos_X,pos_Y), Point2d(pos_X,pos_Y), cv::Scalar(0,0,0), 1);

        /// we want to display in the graph no more than 30 points. The 30th point is discarded by coloring it white
        if (pointsVector.size()>=graphPoints){
            while(pointsVector.size()>graphPoints){
                lastPoint = pointsVector.front();
                cv::line(plot_image, lastPoint, lastPoint, cv::Scalar(255,255,255), 1);
                pointsVector.pop();
            }
        }
        if(is_graph_activated)
            imshow(image_window, plot_image);
        else
            imshow(image_window, extracted_Mat_X);

        waitKey(1);
        // incrementing the expectedFrameNumber because we handled the frame and we can pass to the later one
        expectedFrameNumber++;
    }
}