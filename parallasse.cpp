//
// Created by francis on 12/06/20.
//
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <bits/stdc++.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <thread>
#include <zconf.h>
#include "ourFunctions.h"

#define point pair<double, double>
using namespace cv;
using namespace std;


int match_method=5;
int max_Trackbar = 5;
int frame_number=0;

int position_x=-1;
int position_y=-1;

void MatchingMethod( int, void* );
Mat coord_image = Mat::zeros( 7000, 1200, CV_8UC3);

void draw() {
    while (true) {
        imshow("Coordinates:", coord_image);
        int k = waitKey(1); // Wait for a keystroke in the window, press "s" to save the image
    }
}


/** @function main */
void disegnaParallasse() {

    changeCoordinates3();
    ifstream input_txt ("../output3.txt");
    if (input_txt.is_open())
        cout << "Opened input file.txt\n";

    /// Create white empty image
    coord_image = cv::Scalar(255, 255, 255);

    string line;
    int iter=0;
    Point new_pt;

    std::thread first(draw);
    first.detach();

    while(getline(input_txt, line) && iter < 50000) {
        // extract the coordinates
        size_t pos_first_bracket = line.find("(");
        size_t pos_second_bracket = line.find(")");
        string coordinates = line.substr(pos_first_bracket + 1, pos_second_bracket - pos_first_bracket - 1);

        // detect position of the comma to separate x from y
        size_t pos_comma = coordinates.find(",");
        string x = coordinates.substr(0, pos_comma);
        string y = coordinates.substr(pos_comma + 1);
        //cout<<"x: "<<stod(x)<<", y: "<<stod(y)<<endl;

        // taking the last two points extracted from the txt

        new_pt = Point(stod(x), stod(y));

        // printing a line between the two points
        cv::line(coord_image, new_pt, new_pt, cv::Scalar(0,0,0), 2);
        unsigned int t = 50;
        usleep(t);
        iter++;
    }
    sleep(100);
    return;
}


#define point pair<double, double>
point twoLineIntersection(point A, point B, point C, point D)
{
    // Line AB represented as a1x + b1y = c1
    double a1 = B.second - A.second;
    double b1 = A.first - B.first;
    double c1 = a1*(A.first) + b1*(A.second);

    // Line CD represented as a2x + b2y = c2
    double a2 = D.second - C.second;
    double b2 = C.first - D.first;
    double c2 = a2*(C.first)+ b2*(C.second);

    double determinant = a1*b2 - a2*b1;

    if (determinant == 0)
    {
        // The lines are parallel. This is simplified
        // by returning a pair of FLT_MAX
        std::cerr<<"Lines does NOT intersect"<<endl;
        return make_pair(FLT_MAX, FLT_MAX);
    }
    else
    {
        double x = (b2*c1 - b1*c2)/determinant;
        double y = (a1*c2 - a2*c1)/determinant;
        return make_pair(x, y);
    }
}

//lines camera,pendulum --> y = x * dx/dy - xp * dx/dy + yp     // m = dx/dy
//lines perpendicular -->   y = -1/m *x + (xp/m + yp)

//equazione della retta perpendicolare nel punto givenPoinat ad un'altra retta con coefficiente angolare noto.
point getPointOnTilted(double angularCoeff, point givenPoint, double x){
    double y = -x/angularCoeff + givenPoint.first/angularCoeff + givenPoint.second;
    return make_pair(x,y);
}


int changeCoordinates3(){

    double pixelCm = 0.2; //un pixel corrisponde a 0.2258 cm per come è il nostro sistema //todo controlla anche questo
    point camera = make_pair(130,140); //TODO controllale. Stima molto grossolana
    point pendulum = make_pair(150,50);
    //coefficiente angolare della retta passante per il pendolo e la fotocamera. Chiamiamola retta r
    double angularCoeff = (pendulum.second - camera.second)/(pendulum.first - camera.first);

    //non potendo usare l'equazione esplicita della retta perpendicolare alla retta r nel Pendolo, seleziono due punti
    //che si trovano su tale retta. Questi due punti permettono di rappresentare la retta in un altro modo a noi utile
    point tiltedA = getPointOnTilted(angularCoeff, pendulum, 0);
    point tiltedB = getPointOnTilted(angularCoeff, pendulum, 10);


    // frame height of raspberry
    int height=480;


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
        double oldX = stoi(x);


        //uso i punti tilted estratti per calcolare l'interesezione tra la retta e la retta che passa tra la camera ed il
        //generico punto P

        point oldPoint = make_pair(oldX * pixelCm,0);
        point newPoint = twoLineIntersection(tiltedA, tiltedB, camera, oldPoint);

        string start_string = line.substr (0,pos_first_bracket);
        // saving to txt
        output_txt <<start_string<<"("<<newPoint.first/pixelCm<<","<<new_y<<")\n";
        output_txt.flush();
    }

    // closing the txt files
    input_txt.close();
    output_txt.close();
    return 0;
}