#ifndef FOUCAULT_OURFUNCTIONS_H
#define FOUCAULT_OURFUNCTIONS_H

#include <string>

int oldMain();

int meanShiftTest();

int circleDetector();

int colorTracking(std::string chosenColor);

int changeCoordinates();

int changeCoordinates2();

int changeCoordinates3();

void disegnaParallasse();

cv::Point2f rotate_and_translate(cv::Point2f point);

int drawPoints();

std::pair<double, double> lineLineIntersection(std::pair<double, double> A, std::pair<double, double> B,
                                               std::pair<double, double> C, std::pair<double, double> D);


#endif //FOUCAULT_OURFUNCTIONS_H
