#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <thread>

using namespace cv;
using namespace std;

void usage() {
    printf("Program Options:\n");
    printf("  -p or -1          Time paused. Press r to restart.\n");
    printf("  -r or -2          Restart from pause. Press p to pause again.\n");
    printf("  -v or -3          Change speed in a value from 0 to 4 (default value 1).\n");
    printf("  -s or -4          Save current image displayed to file.\n");
    printf("  -c or -5          Change number of current coordinates to be displayed (default value 30).\n");
    printf("  -h or -?          This message.\n");
}

void drawGraph(){
    ifstream input_csv ("../PendulumCsv/prova.csv");
    if (input_csv.is_open())
        cout << "Opened input file\n";

    // Helper vars
    std::string line, colname;
    std::tuple<double, double, double> currentPoint;
    std::queue<Point2d> pointsVector;

    /// Create white empty image
    Mat plot_image = Mat::zeros( 650, 650, CV_8UC3);
    plot_image = cv::Scalar(255, 255, 255);

    // the first line is the header line, we discard it
    std::getline(input_csv, line);
    String windowName;
    int speedArray[]={250, 200, 100, 50, 30};

    //setting default speed and number of points displayed
    int speed = 1;
    int pointNumber=30;

    // Read data, line by line
    while(std::getline(input_csv, line)){

        // Create a stringstream of the current line
        std::stringstream ss(line);

        double x, y;
        String time;
        ss >> time >> x >> y;
        pointsVector.push(Point2d(x,480-y));

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

        // destroy old window
        destroyWindow(windowName);
        windowName = "Time: "+time;

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
                String name = "../graph_"+time+".png";
                imwrite(name, plot_image);
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

int main() {
    drawGraph();
    return 0;
}

