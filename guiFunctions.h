#ifndef FOUCAULT_GUIFUNCTIONS_H
#define FOUCAULT_GUIFUNCTIONS_H

#include <string>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>



inline bool fileExist(const std::string &name);


void usage();

void drawGraph();


//calibration.cpp functions below

int calibrateCamera();

void updateSize();

void changePointFocus(int status, void *data);

void toggleDrag(int status, void *data);

void mouseCallBack(int event, int x, int y, int flags, void *userdata);

void savePoints(int status, void *data);

cv::Mat drawPerspectivePoint(cv::Mat img, cv::Mat matrix);

cv::Mat calibrationMatching(cv::Mat img, const cv::Mat &templ);

#endif //FOUCAULT_GUIFUNCTIONS_H
