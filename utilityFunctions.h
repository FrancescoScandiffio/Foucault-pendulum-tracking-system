#ifndef FOUCAULT_OURFUNCTIONS_H
#define FOUCAULT_OURFUNCTIONS_H

#include <string>
#include <opencv2/core/types.hpp>

void perspective();

cv::Point2d singlePointPerspective(cv::Point2d);

int changeCoordinatesGeometry();

int changeCoordinatesMatrix();

int changeCoordinates2();

int changeCoordinates3();

void drawParallax(int val);

int drawPoints();

std::pair<double, double> lineLineIntersection(std::pair<double, double> A, std::pair<double, double> B,
                                               std::pair<double, double> C, std::pair<double, double> D);


#endif //FOUCAULT_OURFUNCTIONS_H
