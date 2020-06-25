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
#include <algorithm>
#include <iostream>
#include <cctype>

using namespace cv;
using namespace std;


int match_method=5;
int max_Trackbar = 5;
int frame_number=0;

int position_x=-1;
int position_y=-1;

void MatchingMethod( int, void* );
Mat coord_image = Mat::zeros( 480, 640, CV_8UC3);

void draw() {
    while (true) {
        imshow("Coordinates:", coord_image);
        int k = waitKey(1); // Wait for a keystroke in the window, press "s" to save the image
    }
}


/** @function main */
void drawParallax(int val) {

    if(val==0)
        changeCoordinatesGeometry();
    else
        changeCoordinatesMatrix();

    ifstream input_txt ("../giugno.txt");
    if (input_txt.is_open())
        cout << "Opened input file.txt\n";

    /// Create white empty image
    coord_image = cv::Scalar(255, 255, 255);

    string line;
    int iter=0;
    Point new_pt;

    std::thread first(draw);
    first.detach();

    while(getline(input_txt, line)) {
        // extract the coordinates
        int lBracket = line.find("(");
        int rBracket = line.find(")");
        int comma = line.find(",");

        string x = line.substr(lBracket+1, comma-lBracket-1);
        string y = line.substr(comma+1, rBracket-comma-1);
        x.erase(remove_if(x.begin(), x.end(), ::isspace),x.end());
        y.erase(remove_if(y.begin(), y.end(), ::isspace),y.end());


       new_pt = Point(stod(x), 480-stod(y));

        // printing a line between the two points
        cv::line(coord_image, new_pt, new_pt, cv::Scalar(0,0,0), 2);
        unsigned int t = 1;
        usleep(t);
        iter++;
    }
    sleep(100);
    return;
}

// C style struct for a 3D point
struct Triple{
    double x;
    double y;
    double z;
};

/*
3D line defined by a direction vector and a point of the line
parametric equations can be computed as
x = point.x + vector.x *t
y = point.y + vector.y *t
z = point.z + vector.z *t
 */
struct vectorLine{
    Triple point;
    Triple vector;
};

//make a line given two points
vectorLine makeLine(Triple pointA, Triple pointB){
    Triple point = pointA;
    Triple vector = {pointB.x-pointA.x, pointB.y - pointA.y, pointB.z - pointA.z};
    return {point,vector};
}

//3D plane defined by its normal vector and the deviation value
// ax + by + cz + d
struct Plane{
    Triple vector;
    double d;
};

//get the plane which contains the pointA and is perpendicular to the given line
Plane getPerpendicularPlane(vectorLine line, Triple pointA){
    Triple vector = line.vector;
    double d = - vector.x * pointA.x - vector.y * pointA.y - vector.z * pointA.z;

    return {vector,d};
}

Triple getPlaneLineIntersection(Plane plane, vectorLine line){
    Triple point = line.point;
    Triple vector = line.vector;
    Triple planeVector = plane.vector;
    double t = -(planeVector.x * point.x + planeVector.y * point.y + planeVector.z * point.z + plane.d)
            / (planeVector.x * vector.x + planeVector.y * vector.y + planeVector.z * vector.z);

    return {point.x + vector.x*t,point.y + vector.y*t, point.z + vector.z*t };
}

int changeCoordinatesGeometry(){
    //point: x,y,z where z is real-life height. The origin of the reference system is the bottom-left corner of the black
    //plate on the floor
    double pixelCm = 0.1; //1 pixel = 0.09774193548 cm
    Triple cameraPoint = {25.3, 32, 176.5}; // real-life camera position (cm)
    Triple pendulumPoint = {30.6,32, 14.2};// real-life pendulum position (cm)
    vectorLine focusLine = makeLine(cameraPoint,pendulumPoint);
    Plane focusPlane = getPerpendicularPlane(focusLine, pendulumPoint);


    ifstream input_txt ("../giugno.txt");
    if (input_txt.is_open())
        cout << "Opened input file.txt\n";

    ofstream output_txt ("../output3.txt");
    if (output_txt.is_open())
        cout << "Opened output file.txt\n";

    string line;
    while(getline(input_txt, line)) {

        // extract the coordinates
        int lBracket = line.find("(");
        int rBracket = line.find(")");
        int comma = line.find(",");

        string x = line.substr(lBracket+1, comma-lBracket-1);
        string y = line.substr(comma+1, rBracket-comma-1);
        x.erase(remove_if(x.begin(), x.end(), ::isspace),x.end());
        y.erase(remove_if(y.begin(), y.end(), ::isspace),y.end());

        double oldX = stoi(x);
        double oldY = stoi(y);

        double cmX = oldX * pixelCm;
        double cmY = oldY * pixelCm ;
        Triple coordinate = {cmX, cmY, 14.2};
        vectorLine pointLine = makeLine(cameraPoint,coordinate);
        Triple intersection = getPlaneLineIntersection(focusPlane, pointLine);

        output_txt <<line.substr(0,lBracket)<<"("<<intersection.x/pixelCm<<","<<intersection.y/pixelCm<<")\n";
        output_txt.flush();

    }

    // closing the txt files
    input_txt.close();
    output_txt.close();
    return 0;
}



int changeCoordinatesMatrix(){

    ifstream input_txt ("../giugno.txt");
    if (input_txt.is_open())
        cout << "Opened input file.txt\n";

    ofstream output_txt ("../output3.txt");
    if (output_txt.is_open())
        cout << "Opened output file.txt\n";

    string line;
    while(getline(input_txt, line)) {

        int lBracket = line.find("(");
        int rBracket = line.find(")");
        int comma = line.find(",");

        string x = line.substr(lBracket+1, comma-lBracket-1);
        string y = line.substr(comma+1, rBracket-comma-1);
        x.erase(remove_if(x.begin(), x.end(), ::isspace),x.end());
        y.erase(remove_if(y.begin(), y.end(), ::isspace),y.end());

        double oldX = stoi(x);
        double oldY = stoi(y);

        Point2d old = Point2d(oldX,oldY);
        Point2d nuovo = singlePointPerspective(old);


        output_txt <<line.substr(0,lBracket)<<"("<<nuovo.x<<","<<nuovo.y<<")\n";
        output_txt.flush();

    }

    // closing the txt files
    input_txt.close();
    output_txt.close();
    return 0;
}
