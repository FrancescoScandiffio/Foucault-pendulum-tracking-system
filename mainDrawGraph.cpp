#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <thread>
#include <unistd.h>


using namespace cv;
using namespace std;

void usage() {
    printf("Program Options:\n");
    printf("While graph window is active press one of the symbols below for additional functionalities\n");
    printf("-    p or 1          To pause the execution. Press r to restart.\n");
    printf("-    r or 2          Restart from pause. Press p to pause again.\n");
    printf("-    v or 3          Change speed in a value from 0 to 4 (default value 1).\n");
    printf("-    s or 4          Save current image displayed to file.\n");
    printf("-    c or 5          Change number of current coordinates to be displayed (default value 30).\n");
    printf("-    h or ?          This message.\n");
}

void drawGraph(){

    usage();

    String path;
    fstream input_csv;
    printf("\n -------------------------------\n");
    printf("Insert the relative coordinate file path:\n");
    cin>>path;
    input_csv.open(path); // e.g. "../PendulumCsv/prova.csv"
    while(!input_csv.is_open()){
        printf("Not the expected file, try another one: \n");
        cin>>path;
        input_csv.open(path);
    }


    // Helper vars
    std::string line, colname;
    std::tuple<double, double, double> currentPoint;
    std::queue<Point2d> pointsVector;

    /// Create white empty image
    // image of 640x480 pixels (width x height)
    Mat plot_image = Mat::zeros( 480, 640, CV_8UC3);
    plot_image = cv::Scalar(255, 255, 255);

    // the first line is the header line, we discard it
    std::getline(input_csv, line);
    String windowName = "Graph";
    int speedArray[]={250, 200, 100, 50, 30};

    //setting default speed and number of points displayed
    int speed = 1;
    int pointNumber=30;

    Point pt =  Point(50, 50);
    String oldTime = "0.000000";
    // Read data, line by line
    while(std::getline(input_csv, line)){


        // Create a stringstream of the current line
        std::stringstream ss(line);

        double x, y;
        String time;
        ss >> time >> x >> y;

        pointsVector.push(Point2d(x,480-y));

        // delete the old time
        putText(plot_image, oldTime, pt, FONT_HERSHEY_SIMPLEX,0.5, Scalar(255,255,255), 3, LINE_AA,false);
        // print time on image
        putText(plot_image, time, pt, FONT_HERSHEY_SIMPLEX,0.5, Scalar(0, 0, 0), 1, LINE_AA,false);
        oldTime=time;
        // we start displaying the points
        cv::line(plot_image, Point2d(x,480-y), Point2d(x,480-y), cv::Scalar(0,0,0), 2);
        // we want to display in the graph no more than 30 points. The 30th point is discarded by coloring it white
        if (pointsVector.size()>=pointNumber){
            while(pointsVector.size()>pointNumber){
                Point2d lastPoint = pointsVector.front();
                cv::line(plot_image, lastPoint, lastPoint, cv::Scalar(255,255,255), 2);
                pointsVector.pop();
            }
        }


        bool paused = false;

        // command line options:
        do{
            imshow(windowName, plot_image);
            int k = waitKey(speedArray[speed]); // Wait (default 200) for a keystroke in the window
            if(k == 'p' || k == '1') {
                if (paused) {
                    printf("Display already paused. Press h for help or r to restart\n");
                } else {
                    paused = true;
                }
            }else if(k == 'r' || k == '2'){
                if(paused){
                    // restart the paused display
                    paused = false;
                }else{
                    printf("No need to restart, display already going. Press h for help or p to pause\n");
                }
            }else if(k == 'v' || k == '3'){
                // change the speed at which the points are displayed
                printf("Insert speed from 0 to 4:\n");
                cin>>speed;
                while(speed>4 || speed<0){
                    printf("Please insert valid speed from 0 to 4:\n");
                    cin>>speed;
                }
            }else if(k == 's' || k == '4'){
                // saving the current graph to file
                String name= "graph_"+time+".png";
                imwrite(name, plot_image);
                cout<<"File "<<name<<" saved in project directory."<<endl;
            }else if(k == 'c' || k == '5'){
                printf("Insert number of points to be displayed from now on: (default 30)\n");
                cin>>pointNumber;
                while(pointNumber<5){
                    printf("Please insert valid number of points > 5:\n");
                    cin>>pointNumber;
                }
            }else if(k == 'h' || k =='?'){
                usage();
            }else if(k != -1){
                printf("Invalid command. Press h for help\n");
            }
        }while(paused);
    }
    // Close file
    input_csv.close();
}
