#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

int resultHeight = 500;
int resultWidth = 500;

Point2f calibrationFirstArray[4] = {Point(-10,-10), Point(-10,-10), Point(-10,-10), Point(-10,-10)};
Point2f calibrationSecondArray[4] = {Point(-10,-10), Point(-10,-10), Point(-10,-10), Point(-10,-10)};

Point p5 = Point(0, 0);
Point p6 = Point(resultWidth, 0);
Point p7 = Point(0, resultHeight);
Point p8 = Point(resultWidth, resultHeight);

Point2f v2[] = {p5,p6,p7,p8};

int currentButton = -1;
bool dragMouse=true;

void updateSize(){
    int ABx = abs(calibrationFirstArray[0].x - calibrationFirstArray[1].x);
    int CDx =  abs(calibrationFirstArray[2].x - calibrationFirstArray[3].x);
    resultWidth = ABx < CDx ? CDx : ABx;
    int ACy = abs(calibrationFirstArray[0].y - calibrationFirstArray[2].y);
    int BDy =  abs(calibrationFirstArray[1].y - calibrationFirstArray[3].y);
    resultHeight = ACy < BDy ? BDy : ACy;
    p6 = Point(resultWidth, 0);
    p7 = Point(0, resultHeight);
    p8 = Point(resultWidth, resultHeight);
    String transform = "New View";
    v2[0] = p5;
    v2[1] = p6;
    v2[2] = p7;
    v2[3] = p8;
    cout<<"PerspectiveWindow now displays a "<<resultWidth<<"x"<<resultHeight<<" frame"<<endl;

}

void changePointFocus(int status, void* data){
    int buttonId = *((int *) data);
    currentButton = buttonId;
    dragMouse=false;
}

void toggleDrag(int status, void* data){
    currentButton = -1;
    dragMouse = true;
}

void mouseCallBack(int event, int x, int y, int flags, void *userdata){
    if(event == EVENT_LBUTTONDOWN && !dragMouse) {
        if(currentButton<0 || currentButton > 3){
            cerr<<"BUTTON ERROR. CAN'T HANDLE";
            exit(-3);
        }
        else
            calibrationFirstArray[currentButton] = Point(x,y);

    }
}

void savePoints(int status, void* data){
    ofstream output("calibration.txt");
    for(int i=0; i<4; i++) {
        output << calibrationFirstArray[i].x << ";" << calibrationFirstArray[i].y << endl;
        cout<<"\nPoint "<<i+1<<" X:"<<calibrationFirstArray[i].x<<" Y: "<<calibrationFirstArray[i].y<<endl;
    }
    output.flush();
    output.close();

    updateSize();
}

inline bool fileExist (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

void MatchingMethod(Mat img, String window, Mat templ, bool copy)
{
    /// Source image to display

    /// Create the result matrix

    Mat result;
    int result_cols =  img.cols - templ.cols + 1;
    int result_rows = img.rows - templ.rows + 1;

    result.create( result_rows, result_cols, CV_32FC1 );

    /// Do the Matching and Normalize
    matchTemplate( img, templ, result, 5 );
    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    /// Localizing the best match with minMaxLoc
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    Point matchLoc;

    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    matchLoc = maxLoc;

    /// Show me what you got
    if(copy)
        img = img.clone();
    Point p = Point(matchLoc.x+templ.cols/2,matchLoc.y+templ.rows/2 );
    rectangle( img, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );
    circle(img,p,0,Scalar( 0, 255, 0 ),1);
    imshow( window, img );

    return;
}

int calibrateCamera(){

    bool wasFile=false;
    if(fileExist("calibration.txt")) {
        fstream file("calibration.txt");
        string textLine;
        int currentLine = 0;
        int posX, posY;
        string valX,valY;
        while (getline(file, textLine) && currentLine<4) {
            try {
                int dotComma = textLine.find(";");
                std::cout << textLine << "\n";
                valX = textLine.substr(0, dotComma);
                valY = textLine.substr(dotComma + 1, textLine.length());

                posX = stoi(valX);
                posY = stoi(valY);
                calibrationFirstArray[currentLine] = Point(posX, posY);
                currentLine++;
                wasFile=true;
            } catch (...) {
                cerr << "ERROR: something went wrong while parsing old points from calibration.txt" << endl;
                cerr<< " Wrong value: x "<<valX<<" Y "<<valY;
                cerr.flush();
                cout << "A new file will be created...\n";
                file.close();
                fstream newFile("calibration.txt");
                newFile.close();
                wasFile=false;
                break;
            }
        }
    }
    else{
        cout<<"Configuration file calibration.txt not found. A new file will be created...\n"<<endl;
        fstream newFile("calibration.txt");
        newFile.close();
    }
    Mat templ = imread( "testTemplate2.png", 1 );
    VideoCapture video(0);
    video.set(CAP_PROP_FPS, int(10));
    Mat frame;
    video.read(frame);
    String original = "Camera View";
    String btn1 = "Top Left point";
    String btn2 = "Top Right point";
    String btn3 = "Bottom Left point";
    String btn4 = "Bottom Right point";
    String btnDrag = "Move image with drag";

    String updateBtn = "Save points";
    String transform = "New View";
    resultHeight = frame.rows;

    namedWindow(original,WINDOW_GUI_EXPANDED);
    namedWindow(transform,WINDOW_GUI_EXPANDED);
    moveWindow(original,70,70);
    moveWindow(transform,690,170);

    int d1=0;
    int d2=1;
    int d3=2;
    int d4=3;

    void* pt1 = &d1;
    void* pt2 = &d2;
    void* pt3 = &d3;
    void* pt4 = &d4;

    createButton(btnDrag,toggleDrag,NULL,QT_RADIOBOX,1);
    createButton(btn1,changePointFocus,pt1,QT_RADIOBOX, 0);
    createButton(btn2,changePointFocus,pt2,QT_RADIOBOX,0);
    createButton(btn3,changePointFocus,pt3,QT_RADIOBOX,0);
    createButton(btn4,changePointFocus,pt4,QT_RADIOBOX,0);

    createButton(updateBtn,savePoints,NULL,QT_PUSH_BUTTON | QT_NEW_BUTTONBAR,0);

    setMouseCallback(original,mouseCallBack,NULL );

    if(wasFile)
        updateSize();
    else{
        p6 = Point(resultWidth, 0);
        p7 = Point(0, resultHeight);
        p8 = Point(resultWidth, resultHeight);
        v2[0] = p5;
        v2[1] = p6;
        v2[2] = p7;
        v2[3] = p8;
    }
    resizeWindow(original, 600,600);
    resizeWindow(transform, 500,500);

    Mat matrix;
    Mat result;
    double myMatrix[3][3];
    matrix = getPerspectiveTransform(calibrationFirstArray, v2);
    for(int i=0; i<3; i++)
        for(int j=0; j<3; j++){
            myMatrix[i][j] = matrix.at<double>(i,j);
        }

    int oldx= 388;
    int oldy = 219;
    cout<<"Frame height: "<<resultHeight;
    double num = myMatrix[0][0]*oldx+myMatrix[0][1]*oldy+myMatrix[0][2];
    double dem = myMatrix[2][0]*oldx+myMatrix[2][1]*oldy+myMatrix[2][2];
    double new_position_x = num/dem;
    num = myMatrix[1][0]*oldx+myMatrix[1][1]*oldy+myMatrix[1][2];
    dem = myMatrix[2][0]*oldx+myMatrix[2][1]*oldy+myMatrix[2][2];
    double new_position_y = num/dem;
    cout<<"\n new: "<<new_position_x<<" y: "<<new_position_y;
    while(true) {
        video.read(frame);

        matrix = getPerspectiveTransform(calibrationFirstArray, v2);
        Size dsize = Size(resultWidth, resultHeight);
        warpPerspective(frame, result, matrix, dsize);
        MatchingMethod(frame,original,templ,false);
        MatchingMethod(result,transform,templ,true);

        for(int i=0; i<4; i++){
            if(i==currentButton)
                circle(frame,calibrationFirstArray[i],7,Scalar( 0, 0, 255),2);
            else
                circle(frame,calibrationFirstArray[i],7,Scalar( 255, 255, 255 ),2);
            circle(frame,calibrationFirstArray[i],0,Scalar( 0, 0, 255 ),1);

        }
        cv::line(frame, calibrationFirstArray[0], calibrationFirstArray[1], cv::Scalar(0,0,0), 1);
        cv::line(frame, calibrationFirstArray[0], calibrationFirstArray[2], cv::Scalar(0,0,0), 1);
        cv::line(frame, calibrationFirstArray[2], calibrationFirstArray[3], cv::Scalar(0,0,0), 1);
        cv::line(frame, calibrationFirstArray[1], calibrationFirstArray[3], cv::Scalar(0,0,0), 1);



        waitKey(1);
    }
    return 0;
}





int main(int arg, char*argv[]){
    calibrateCamera();
}