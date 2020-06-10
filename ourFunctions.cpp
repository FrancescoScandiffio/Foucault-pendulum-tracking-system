#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>

using namespace cv;
using namespace std;

cv::Point2f rotate_and_translate(cv::Point2f);

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

    while(1) {
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
    cv::VideoCapture capture("../videos/test.mp4");
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

int circleDetector(){

    ///If on Raspberry:
    //VideoCapture capture(0); // open the default camera
    //setting fps rate of video to grab
    //capture.set(CAP_PROP_FPS, int(12));

    ///If working locally:
    VideoCapture capture("../videos/prova.h264");

    if(!capture.isOpened()){
        // check if we succeeded
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    if(!capture.isOpened()){
        // check if we succeeded
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    Mat frame;
    Mat cropped_frame;
    Mat frame_gray;
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

        // Convert it to gray
        cvtColor( cropped_frame, frame_gray, COLOR_BGR2GRAY );

        // Reduce the noise so we avoid false circle detection
        GaussianBlur( frame_gray, frame_gray, Size(9, 9), 2, 2 );
        vector<Vec3f> circles;

        // Apply the Hough Transform to find the circles
        HoughCircles( frame_gray, circles, HOUGH_GRADIENT, 1, frame_gray.rows/8, 200, 100, 0, 0 );

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
        namedWindow("gray");
        imshow("gray",frame_gray);

        namedWindow( "Hough Circle Transform", WINDOW_AUTOSIZE );
        imshow( "Hough Circle Transform", cropped_frame );

        // show live and wait for a key with timeout long enough to show images
        if (waitKey(5) >= 0)
            break;
    }
    return 0;
}

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

int colorTracking(string chosenColor){

    ///If on Raspberry:
    // open the default camera
    VideoCapture capture(0);
    // setting fps rate of video to grab
    capture.set(CAP_PROP_FPS, int(12));

    ///If working locally:
    //VideoCapture capture("../videos/output.mp4");

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
    vector<cv::Point> locations;     // locations of non-zero pixels (white pixels)


    // Opening the file where will be saved the coordinates of white pixels on each frame
    ofstream txt_file ("../positions.txt");
    if (!txt_file.is_open())
        cout << "Unable to open file positions.txt";

    int i=0;
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

        cv::findNonZero(frame_threshold, locations);

        // access pixel coordinates
        for (int count=0; count<locations.size(); count++){
            Point pnt = locations[count];
            // printf("Position of blank pixel on frame %d is (%d, %d)\n", i, pnt.x, pnt.y);

            // saving the position back to the file
            txt_file << "Frame "<< i << ", position ("<< pnt.x<<", "<<pnt.y<<")\n";
        }


        /*
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
        */


        imshow("thresh", frame_threshold);
        imshow("video", frame);

        i++;
        // show live and wait for a key with timeout long enough to show images
        if (waitKey(5) >= 0)
            break;
    }

    // closing the txt file
    txt_file.close();
    // the camera will be de-initialized automatically in VideoCapture destructor
    return 0;
}

int changeCoordinates(){

    // frame height of raspberry
    int height=480;

    ifstream input_txt ("../Wed May 13 10:40:40 2020.txt");
    if (input_txt.is_open())
        cout << "Opened input file.txt\n";

    ofstream output_txt ("../output.txt");
    if (output_txt.is_open())
        cout << "Opened output file.txt\n";

    string line;
    int count_frame=0;
    string date;
    string old_date="";
    while(getline(input_txt, line)) {
        size_t pos_first_bracket = line.find("(");

        // if there is no ")" in this line, it means that the line contains the timestamp
        if (pos_first_bracket==string::npos){
            // extracting just the time
            size_t pos_colon = line.find(":");
            size_t pos_2020 = line.find("2020");
            date = line.substr (pos_colon-2,pos_2020-pos_colon+1);
            //cout<<"Date: "<<date<<endl;
            if(old_date==date){
                count_frame++;
            } else
                count_frame=0;
            old_date=date;

        } else{
            // extract the coordinates
            size_t pos_second_bracket = line.find(")");
            string coordinates = line.substr (pos_first_bracket+1, pos_second_bracket-pos_first_bracket-1);
            //cout<<coordinates<<endl;

            // detect position of the comma to separate x from y
            size_t pos_comma = coordinates.find(",");
            string x=coordinates.substr (0,pos_comma);
            string y=coordinates.substr (pos_comma+2);
            //cout<<"x: "<<x<<", y: "<<y<<endl;

            // updating with proper value. At the moment y indicates the distance from the point to the top of the image
            // now y is the distance from point to image bottom
            int new_y=height-stoi(y);
            //cout<<"Old y: "<<y<<", New y: "<<new_y<<endl;

            // find frame number
            size_t pos_frame_number = line.find("frame ");
            size_t pos_position = line.find("position");
            string frame_number=line.substr (pos_frame_number,pos_position-2);
            cout<<"Frame number: "<<frame_number<<endl;

            // saving to txt
            output_txt <<date<< "_"<< count_frame << " "<< frame_number<<" position ("<<x<<","<<new_y<<")\n";
            output_txt.flush();
        }
    }

    // closing the txt files
    input_txt.close();
    output_txt.close();
    return 0;
}

int changeCoordinates2(){
    // frame height of raspberry
    int height=480;

    // using sin of the angle (1.87036) in degrees
    double sin_angle_degrees = 0.0326;

    // camera center
    double camera_center = 310;

    ifstream input_txt ("../Fri May 29 14-02-27 2020.txt");
    if (input_txt.is_open())
        cout << "Opened input file.txt\n";

    ofstream output_txt ("../output3.txt");
    if (output_txt.is_open())
        cout << "Opened output file.txt\n";

    string line;
    while(getline(input_txt, line)) {

        // extract the coordinates
        size_t pos_first_bracket = line.find("(");
        size_t pos_second_bracket = line.find(")");
        string coordinates = line.substr (pos_first_bracket+1, pos_second_bracket-pos_first_bracket-1);
        //cout<<coordinates<<endl;

        // detect position of the comma to separate x from y
        size_t pos_comma = coordinates.find(",");
        string x=coordinates.substr (0,pos_comma);
        string y=coordinates.substr (pos_comma+2);
        //cout<<"x: "<<x<<", y: "<<y<<endl;

        // updating with proper value. At the moment y indicates the distance from the point to the top of the image
        // now y is the distance from point to image bottom
        int new_y=height-stoi(y);

        /// correcting the parallax:
        // translating the system to camera coordinates
        double translated_x = stoi(x) - camera_center ;

        // finding new x
        double rotated_x = translated_x+ translated_x * sin_angle_degrees;
        //cout<<new_x<<endl;

        double new_x = rotated_x + camera_center;

        string start_string = line.substr (0,pos_first_bracket);
        // saving to txt
        output_txt <<start_string<<"("<<new_x<<","<<new_y<<")\n";
        output_txt.flush();
    }

    // closing the txt files
    input_txt.close();
    output_txt.close();
    return 0;
}

int drawPoints(){

    ifstream input_txt ("../Fri May 29 14-02-27 2020.txt");
    if (input_txt.is_open())
        cout << "Opened input file.txt\n";

    /// Create white empty image
    Mat coord_image = Mat::zeros( 600, 600, CV_8UC3);
    coord_image = cv::Scalar(255, 255, 255);

    string line;
    int iter=0;
    Point old_pt;
    Point new_pt;

    while(getline(input_txt, line)) {
        // extract the coordinates
        size_t pos_first_bracket = line.find("(");
        size_t pos_second_bracket = line.find(")");
        string coordinates = line.substr(pos_first_bracket + 1, pos_second_bracket - pos_first_bracket - 1);

        // detect position of the comma to separate x from y
        size_t pos_comma = coordinates.find(",");
        string x = coordinates.substr(0, pos_comma);
        string y = coordinates.substr(pos_comma + 2);
        //cout<<"x: "<<x<<", y: "<<y<<endl;

        // taking the last two points extracted from the txt
        if (iter==0){
            old_pt = Point(stoi(x), stoi(y));
            new_pt = old_pt;
        }else if(iter>5000){
            // discarding the first 5000 points in order to remove possible anomalies while starting the pendulum
            old_pt = new_pt;
            new_pt = Point(stoi(x), stoi(y));
        }
        // printing a line between the two points
        cv::line(coord_image, old_pt, new_pt, cv::Scalar(0,0,0), 2);

        iter++;
    }

    imshow("Coordinates:", coord_image);
    int k = waitKey(0); // Wait for a keystroke in the window, press "s" to save the image
    if(k == 's') {
        imwrite("../outputs/coordinates.png", coord_image);
    }

    return 0;
}

cv::Point2f rotate_and_translate(cv::Point2f old_point){

    // computing the angle
    float angle;
    // depth is the wire's length in cm
    float depth = 1734-111;

    // z is the hypotenuse in cm, and we compute it by pythagoras theorem
    float z;
    float distance_from_wire = 53; //cm
    z = float(sqrt(pow(distance_from_wire,2)+pow(depth,2)));
    // we find the angle from the hypotenuse and depth
    angle = asin(distance_from_wire/z);
    cout<<angle<<endl;

    // creating the matrix of translation to point in camera
    float T0_data[16] = { 1, 0, 0, old_point.x, 0, 1, 0, old_point.y, 0, 0, 1, z, 0, 0, 0, 1 };
    cv::Mat T0_matrix = cv::Mat(4, 4, CV_32F, T0_data);

    // creating the matrix of rotation
    float Ry_data[16] = {cos(angle), 0, -sin(angle), 0, 0, 1, 0, 0, sin(angle), 0, cos(angle), 0, 0, 0, 0, 1};
    cv::Mat Ry_matrix = cv::Mat(4, 4, CV_32F, Ry_data);

    // creating the matrix that translate the point back to the origin
    float T0_inv_data[16] = { 1, 0, 0, -old_point.x, 0, 1, 0, -old_point.y, 0, 0, 1, -z, 0, 0, 0, 1 };
    cv::Mat T0_inv_matrix = cv::Mat(4, 4, CV_32F, T0_inv_data);

    cv::Mat affine_transformation_matrix = T0_matrix * Ry_matrix * T0_inv_matrix;
    cout<<"affine_transformation_matrix: "<<affine_transformation_matrix<<endl;

    // vector of original coordinates
    float original_coordinates_data[4] = { old_point.x, old_point.y, z, 1};
    cv::Mat original_coordinates = cv::Mat(4, 1, CV_32F, original_coordinates_data);
    cout<<"original_coordinates: "<<original_coordinates<<endl;

    // new rotated point
    cv::Mat new_point =  affine_transformation_matrix * original_coordinates;
    cout<<"new_point: "<<new_point<<endl;

    // we are only interested in the x and y coordinates
    cout<<new_point.at<float>(cv::Point(0,0))<<endl;
    cout<<new_point.at<float>(cv::Point(1,0))<<endl;
    cv::Point2f new_coordinates(new_point.at<float>(cv::Point(0,0)), new_point.at<float>(cv::Point(0,1)));
    return new_coordinates;

}