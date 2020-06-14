#ifndef FOUCAULT_OURFUNCTIONS_H
#define FOUCAULT_OURFUNCTIONS_H

#include <string>

int changeCoordinates2();

int changeCoordinates3();

void drawParallax();

int drawPoints();

std::pair<double, double> lineLineIntersection(std::pair<double, double> A, std::pair<double, double> B,
                                               std::pair<double, double> C, std::pair<double, double> D);


#endif //FOUCAULT_OURFUNCTIONS_H
