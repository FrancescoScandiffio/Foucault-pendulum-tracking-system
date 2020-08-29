#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace cv;

int resultHeight = 600;
int resultWidth = 600;

Point2f pointArray[4] = {Point(-10,-10), Point(-10,-10), Point(-10,-10), Point(-10,-10)};
Point2f perspectiveArray[4] = {Point(-10,-10), Point(-10,-10), Point(-10,-10), Point(-10,-10)};

Point p5 = Point(0, 0);
Point p6 = Point(resultWidth, 0);
Point p7 = Point(0, resultHeight);
Point p8 = Point(resultWidth, resultHeight);

Point2f v2[] = {p5,p6,p7,p8};

int currentButton = -1;
bool dragMouse=true;


int getLength(int xA, int yA, int xB, int yB){
    return (int)(sqrt( pow((xA- xB),2)+ pow((yA - yB),2) ));

}

void updateSize(){
    int a= getLength(pointArray[0].x,pointArray[0].y,pointArray[1].x,pointArray[1].y);
    int b = getLength(pointArray[0].x,pointArray[0].y,pointArray[2].x,pointArray[2].y);
    cout<<"Length 1: "<<a<<endl;
    cout<<"Length 2: "<<b<<endl;
    double ratio = ((double)(a))/b;
    resultWidth = 600;
    resultHeight = resultWidth/ratio;
    p6 = Point(resultWidth, 0);
    p7 = Point(0, resultHeight);
    p8 = Point(resultWidth, resultHeight);
    String transform = "New View";

    v2[0] = p5;
    v2[1] = p6;
    v2[2] = p7;
    v2[3] = p8;
    resizeWindow(transform, resultWidth,resultHeight);
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
            pointArray[currentButton] = Point(x,y);

    }
}

void savePoints(int status, void* data){
    ofstream output("calibration.txt");
    for(int i=0; i<4; i++) {
        output << pointArray[i].x << ";" << pointArray[i].y << endl;
        cout<<"Point "<<i+1<<" X:"<<pointArray[i].x<<" Y: "<<pointArray[i].y<<endl;
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
                pointArray[currentLine] = Point(posX, posY);
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
        resizeWindow(original, 600,600);
        resizeWindow(transform, 500,500);
    }

    Mat matrix;
    Mat result;
    waitKey(1);

    while(true) {
        video.read(frame);
        for(int i=0; i<4; i++){
            if(i==currentButton)
                circle(frame,pointArray[i],7,Scalar( 0, 0, 255),2);
            else
                circle(frame,pointArray[i],7,Scalar( 255, 255, 255 ),2);
            circle(frame,pointArray[i],0,Scalar( 0, 0, 255 ),1);

        }
        cv::line(frame, pointArray[0], pointArray[1], cv::Scalar(0,0,0), 1);
        cv::line(frame, pointArray[0], pointArray[2], cv::Scalar(0,0,0), 1);
        cv::line(frame, pointArray[2], pointArray[3], cv::Scalar(0,0,0), 1);
        cv::line(frame, pointArray[1], pointArray[3], cv::Scalar(0,0,0), 1);

        matrix = getPerspectiveTransform(pointArray, v2);
        Size dsize = Size(resultWidth, resultHeight);
        warpPerspective(frame, result, matrix, dsize);
        imshow(transform, result);
        imshow(original, frame);
        waitKey(1);
    }
    return 0;
}




int exampleMain(int argc, char *argv[]) {

    if (argc > 2) {
        cerr << "Too many arguments" << endl;
        cout << "Execute with option '-h' or '-help' (without quotes) to see all the possible configuration" << endl;
        exit(1);
    }
    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-help") == 0) {
            //TODO aggiungere testo. Da mettere alla fine
            exit(0);

            //help

        }
        if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "-calibrate") == 0) {
            calibrateCamera();
            exit(0);
        }

        if (strcmp(argv[1], "-g") == 0 || strcmp(argv[1], "-graph") == 0) {
            //TODO qui metti la chiamata alla tua funzione che disegna il grafico
            exit(0);

            cerr << argv[1] << " is an unknown option" << endl;
            exit(1);
        }

        cout << "Execute with option '-h' or '-help' (without quotes) to see all the possible configuration" << endl;

    }




}