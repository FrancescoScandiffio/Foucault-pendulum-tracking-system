#include <iostream>
#include <cmath>
#include <opencv2/core.hpp>
#include "main.h"

using namespace cv;
using namespace std;

#define PI 3.14159265

int main() {

    string path = "/path/test.csv";
    fstream input_csv;
    input_csv.open(path); // e.g. "../PendulumCsv/prova.csv"

    std::vector< float > tt;
    std::vector< float > xx;
    std::vector< float > yy;
    std::vector< float > rr;
    float xCenter= 332;
    float yCenter= 243;

    // Helper vars
    std::string line, colname;
    // the first line is the header line, we discard it
    std::getline(input_csv, line);

    double x;
    double y;
    String time;
    // Read data, line by line
    while(std::getline(input_csv, line)) {

        // Create a stringstream of the current line
        std::istringstream ss(line);
        std::string token;
        x = 99999999;
        y = 99999999;
        time = " ";

        while (std::getline(ss, token, ';')) {
            if (time == " ")
                time = token;
            else if (x == 99999999)
                x = std::stod(token);
            else
                y = std::stod(token);
        }
        // current line of the file
        std::cout << "time " << std::stof(time) << ", x: " << x << ", y: " << y << std::endl;

        tt.push_back(std::stof(time));
        xx.push_back(x-xCenter);
        yy.push_back(y-yCenter);
        rr.push_back(sqrt(pow((x-xCenter), 2) + pow((y-yCenter), 2)));
    }

    // Close file
    input_csv.close();

    int result = computeDegree(tt, xx, yy, rr);

    return 0;
}



// --------------------------------------------

int computeDegree( std::vector< float > tt, std::vector< float > xx, std::vector< float > yy, std::vector< float > rr){
    // - tt is an array of elapsed times, one for each coordinate sampled
    // - xx is an array of x coordinates obtained from (x coord - xCenter)
    // - yy is an array of y coordinates obtained from (y coord - yCenter)
    // - rr is an array of radius obtained from sqrt(xx^2+yy^2)
    // - npts is the total number of points

    int npts = tt.size();
    std::cout << "number of points: " << npts << std::endl;

    float Rgood = 100;
    // maximum number
    int Nmassi = 0;
    // maximum indices
    std::vector< float > imassi;

    for (int i=1; i<npts-1; i++) {
        if ((rr[i] > rr[i - 1]) && (rr[i] >= rr[i + 1]) && (rr[i] > Rgood)){
            Nmassi = Nmassi + 1;
            imassi.push_back(i);
        }
    }

    // if the number of maximum is odd, we make it even discarding the last one
    Nmassi = Nmassi - Nmassi % 2;

    float th3s = 0;
    float th3s2 = 0;

    for (int j=0; j<Nmassi-1; j+=2) {
        // k and l are two consecutive maximum
        int k = imassi[j];
        int l = imassi[j + 1];

        // use the consecutive maximum coordinates to obtain theta: arctan((yl-yk)/(xl-xk))
        int th3now = atan((yy[l] - yy[k])/(xx[l] - xx[k])) *180 / PI; // arctan in degrees
        if (th3now < 0){
            th3now = th3now + 180;
        }
        th3s = th3s + th3now;
        th3s2 = th3s2 + pow(th3now,2);
        std::cout << "theta3 " << j << ", "<< th3now <<std::endl;
    }

    int teta3 = th3s/Nmassi*2;
    int erteta3 = sqrt(th3s2 - pow(th3s,2)/Nmassi*2.0)/Nmassi*2.0;

    std::cout << "teta3: " << teta3 << ", erteta3: "<< erteta3 <<std::endl;
    return 0;
}

