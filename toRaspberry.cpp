//
// Created by francis on 12/05/20.
//

/*
Usiamo questo file per inserire il main e la funzione da voler eseguire sul raspberry, così che il main
che usiamo sul nostro pc sia pulito e tutte le funzioni siano ben separate. Questo va modificato solo quando vogliamo
caricare qualcosa sul rasp. Inoltre questo file è RIMOSSO dal Cmake, così da non avere conflitti per la funzione main
*/


#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <opencv2/imgproc.hpp>

#include "ourFunctions.h"

using namespace cv;
using namespace std;



int main(int, char**){
    //oldMain();
    //meanShiftTest();

    //string color("green");
    string color("red");
    colorTracking(color);
    //circleDetector();
    //colorTrackingAndCircleDetection(color);
    return 0;
}
