#ifndef FOUCAULT_OURFUNCTIONS_H
#define FOUCAULT_OURFUNCTIONS_H

#include <string>

int oldMain();

int meanShiftTest();

int circleDetector();

int colorTracking(std::string chosenColor);

int changeCoordinates();

int changeCoordinates2();

cv::Point2f rotate_and_translate(cv::Point2f point);

int drawPoints();


#endif //FOUCAULT_OURFUNCTIONS_H
