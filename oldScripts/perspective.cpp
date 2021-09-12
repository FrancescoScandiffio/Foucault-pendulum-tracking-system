//
// Created by francis on 23/06/20.
//
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <bits/stdc++.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <opencv2/videoio.hpp>

using namespace cv;
using namespace std;


void perspective(){
    VideoCapture video(0); // open the default camera
    video.set(CAP_PROP_FPS, int(20));
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
        if (waitKey(5) >= 0)
            break;

        // the camera will be de-initialized automatically in VideoCapture destructor

        Point p1 = Point(32, 0);
        Point p2 = Point(610, 0);
        Point p3 = Point(23, 478);
        Point p4 = Point(600, 475);

        circle(frame, p1, 5, (255, 0, 255)); // top left
        circle(frame, p2, 5, (255, 0, 255)); // top right
        circle(frame, p3, 5, (255, 0, 255)); //bottom left
        circle(frame, p4, 5, (255, 0, 255)); //bottom right

        imshow("Frame", frame);

        int resultHeight = 480;
        int resultWidth = 640;

        Point p5 = Point(0, 0);
        Point p6 = Point(resultWidth+20, 0);
        Point p7 = Point(0, resultHeight);
        Point p8 = Point(resultWidth+20, resultHeight);


        Point2f v1[] = {p1,p2,p3,p4};
        Point2f v2[] = {p5,p6,p7,p8};

        Mat matrix = getPerspectiveTransform(v1, v2);
        Mat matrix2;
        invert(matrix,matrix2);
        cout<<matrix2.row(0)<<endl;
        cout<<matrix2.row(1)<<endl;
        cout<<matrix2.row(2)<<endl;

        Mat result;
        Size dsize = Size(resultWidth, resultHeight);
        warpPerspective(frame, result, matrix, dsize);
        imshow("transform", result);
        waitKey(1);


    }

}

/*

 LA MATRTICE È LA SEGUENTE

{{1.099921451562581, 0.02070981812565529, -35.19748645000259},
    {0, 1.001988233837735, 0},
    {-1.087449025826923e-05, -4.051472511817295e-06, 1}};


L'inversa che a noi serve è

{{0.9094723113199955, -0.01866819716529685, 32.01113935433808},
    {-0, 0.9980157113920193, -0},
    {9.890047789614893e-06, 3.840426092853079e-06, 1.000348104823065}};

 */

Point2d singlePointPerspective(Point2d src){

    double array[3][3] = {{0.9094723113199955, -0.01866819716529685, 32.01113935433808},
                          {-0, 0.9980157113920193, -0},
                          {9.890047789614893e-06, 3.840426092853079e-06, 1.000348104823065}};


    //Mat inverted= Mat(3,3,CV_32FC1, &array);
    //cout<<inverted.row(0)<<endl;
    //cout<<inverted.row(1)<<endl;
    //cout<<inverted.row(2)<<endl;

    Point destination = Point(0,0);

    double num = array[0][0]*src.x+array[0][1]*src.y+array[0][2];
    double dem = array[2][0]*src.x+array[2][1]*src.y+array[2][2];
    destination.x= num/dem;

    num = array[1][0]*src.x+array[1][1]*src.y+array[1][2];
    dem = array[2][0]*src.x+array[2][1]*src.y+array[2][2];
    destination.y= num/dem;

    return destination;

}
