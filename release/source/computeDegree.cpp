#include <iostream>
#include <cmath>
#include <opencv2/core.hpp>
#include "main.h"
#include <array>

using namespace cv;
using namespace std;

#define PI 3.14159265

int main() {

    string path = "/path/test.csv";
    fstream input_csv;
    input_csv.open(path); // e.g. "../PendulumCsv/prova.csv"

    float xCenter= 332;
    float yCenter= 243;

    // Helper vars
    std::string line, colname;
    // the first line is the header line, we discard it
    std::getline(input_csv, line);

    double x;
    double y;
    String time;
    // npts is the total number of points
    int npts=0;

    // last, current and next elements to evaluate. Each array has the form [x, y, radius].
    String lastTime;
    double last[3];
    String currentTime;
    double current[3];
    String nextTime;
    double next[3];

    double* lastMax = nullptr;

    float th3s = 0;
    float th3s2 = 0;

    // number of maximum
    int Nmassi = 0;


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
        //std::cout << "time " << time << ", x: " << x << ", y: " << y << std::endl;

        // - time is the current time
        // - xx is the x coordinate obtained from (x coord - xCenter)
        // - yy is the y coordinate obtained from (y coord - yCenter)
        // - rr is the radius obtained from sqrt(xx^2+yy^2)
        double xx = x-xCenter;
        double yy = y-yCenter;
        double rr = sqrt(pow(xx, 2) + pow(yy, 2));

        float Rgood = 100;

        if(npts == 0){
            nextTime=time;
            next[0]=xx;
            next[1]=yy;
            next[2]=rr;
        }else if(npts==1){
            currentTime=nextTime;
            current[0]=next[0];
            current[1]=next[1];
            current[2]=next[2];
            nextTime=time;
            next[0]=xx;
            next[1]=yy;
            next[2]=rr;
        }else{
            lastTime=currentTime;
            last[0]=current[0];
            last[1]=current[1];
            last[2]=current[2];
            currentTime=nextTime;
            current[0]=next[0];
            current[1]=next[1];
            current[2]=next[2];
            nextTime=time;
            next[0]=xx;
            next[1]=yy;
            next[2]=rr;

            if ((current[2] > last[2]) && (current[2] >= next[2]) && (current[2] > Rgood)){
                // found a new maximum
                Nmassi++;
                //std::cout << "Max: time " << currentTime << ", xx: " << current[0]<< ", yy: " << current[1] << ", r: " << current[2] <<std::endl;

                // we evaluate the maximums two by two
                if(lastMax == nullptr){ //checks the size of the array. If the size is zero
                    lastMax = new double[3];
                    // this is the first of the couple to evaluate
                    lastMax[0]=current[0];
                    lastMax[1]=current[1];
                    lastMax[2]=current[2];
                    continue;
                }

                // use the consecutive maximum coordinates to obtain theta:
                // arctan((y_current - y_last)/(x_current - x_last))
                int th3now = atan((current[1] - lastMax[1])/(current[0] - lastMax[0])) *180 / PI; // arctan in degrees
                if (th3now < 0){
                    th3now = th3now + 180;
                }
                th3s = th3s + th3now;
                th3s2 = th3s2 + pow(th3now,2);
                std::cout << "Num max: " << Nmassi << ", theta3: "<< th3now <<std::endl;

                // we evaluated two max
                lastMax = nullptr;
            }
        }

        npts++;
    }

    // Close file
    input_csv.close();

    int teta3 = th3s/Nmassi*2;
    int erteta3 = sqrt(th3s2 - pow(th3s,2)/Nmassi*2.0)/Nmassi*2.0;

    std::cout << "teta3: " << teta3 << ", erteta3: "<< erteta3 <<std::endl;


    return 0;
}

