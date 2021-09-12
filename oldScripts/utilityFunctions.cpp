#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <thread>
#include <zconf.h>

using namespace cv;
using namespace std;
#define point pair<double, double>
Mat coord_image = Mat::zeros( 7000, 1200, CV_8UC3);

int drawPoints(){

    ifstream input_txt ("../output3.txt");
    if (input_txt.is_open())
        cout << "Opened input file\n";

    /// Create white empty image
    Mat coord_image = Mat::zeros( 600, 600, CV_8UC3);
    coord_image = cv::Scalar(255, 255, 255);

    string line;
    int iter=0;
    Point old_pt;
    Point new_pt;

    int height=480;

    while(getline(input_txt, line)) {
        // extract the coordinates
        size_t pos_first_bracket = line.find("(");
        size_t pos_second_bracket = line.find(")");
        string coordinates = line.substr(pos_first_bracket + 1, pos_second_bracket - pos_first_bracket - 1);

        // detect position of the comma to separate x from y
        size_t pos_comma = coordinates.find(",");
        string x = coordinates.substr(0, pos_comma);
        //note: use pos_comma + 2 in txt files where there is a blank spot between comma and number
        string y = coordinates.substr(pos_comma + 1);
        //cout<<"x: "<<x<<", y: "<<y<<endl;

        if (iter==0){
            // taking the last two points extracted from the txt
            old_pt = Point(stoi(x), height-stoi(y));
            new_pt = old_pt;
        } else if (iter > 5000) {
            // discarding the first 5000 points in order to remove possible anomalies while starting the pendulum
            old_pt = new_pt;
            new_pt = Point(stoi(x), height-stoi(y));

            // printing a line between the two points
            cv::line(coord_image, old_pt, new_pt, cv::Scalar(0,0,0), 2);
        }
        iter++;
    }

    imshow("Coordinates:", coord_image);
    int k = waitKey(0); // Wait for a keystroke in the window, press "s" to save the image
    if(k == 's') {
        imwrite("../outputs/coordinates2.png", coord_image);
    }

    return 0;
}

int changeCoordinates(){

    // frame height of raspberry
    int height=480;

    ifstream input_txt ("../Wed May 13 10:40:40 2020.txt");
    if (input_txt.is_open())
        cout << "Opened input file.txt\n";

    ofstream output_txt ("../output.txt");
    if (output_txt.is_open())
        cout << "Opened output file.txt\n";

    string line;
    int count_frame=0;
    string date;
    string old_date="";
    while(getline(input_txt, line)) {
        size_t pos_first_bracket = line.find("(");

        // if there is no ")" in this line, it means that the line contains the timestamp
        if (pos_first_bracket==string::npos){
            // extracting just the time
            size_t pos_colon = line.find(":");
            size_t pos_2020 = line.find("2020");
            date = line.substr (pos_colon-2,pos_2020-pos_colon+1);
            //cout<<"Date: "<<date<<endl;
            if(old_date==date){
                count_frame++;
            } else
                count_frame=0;
            old_date=date;

        } else{
            // extract the coordinates
            size_t pos_second_bracket = line.find(")");
            string coordinates = line.substr (pos_first_bracket+1, pos_second_bracket-pos_first_bracket-1);
            //cout<<coordinates<<endl;

            // detect position of the comma to separate x from y
            size_t pos_comma = coordinates.find(",");
            string x=coordinates.substr (0,pos_comma);
            string y=coordinates.substr (pos_comma+2);
            //cout<<"x: "<<x<<", y: "<<y<<endl;

            // updating with proper value. At the moment y indicates the distance from the point to the top of the image
            // now y is the distance from point to image bottom
            int new_y=height-stoi(y);
            //cout<<"Old y: "<<y<<", New y: "<<new_y<<endl;

            // find frame number
            size_t pos_frame_number = line.find("frame ");
            size_t pos_position = line.find("position");
            string frame_number=line.substr (pos_frame_number,pos_position-2);
            cout<<"Frame number: "<<frame_number<<endl;

            // saving to txt
            output_txt <<date<< "_"<< count_frame << " "<< frame_number<<" position ("<<x<<","<<new_y<<")\n";
            output_txt.flush();
        }
    }

    // closing the txt files
    input_txt.close();
    output_txt.close();
    return 0;
}

int changeCoordinates2(){
    // frame height of raspberry
    int height=480;

    // using sin of the angle (1.87036) in degrees
    double sin_angle_degrees = 0.0326;

    // camera center
    double camera_center = 310;

    ifstream input_txt ("../Fri May 29 14-02-27 2020.txt");
    if (input_txt.is_open())
        cout << "Opened input file.txt\n";

    ofstream output_txt ("../output3.txt");
    if (output_txt.is_open())
        cout << "Opened output file.txt\n";

    string line;
    while(getline(input_txt, line)) {

        // extract the coordinates
        size_t pos_first_bracket = line.find("(");
        size_t pos_second_bracket = line.find(")");
        string coordinates = line.substr (pos_first_bracket+1, pos_second_bracket-pos_first_bracket-1);
        //cout<<coordinates<<endl;

        // detect position of the comma to separate x from y
        size_t pos_comma = coordinates.find(",");
        string x=coordinates.substr (0,pos_comma);
        string y=coordinates.substr (pos_comma+2);
        //cout<<"x: "<<x<<", y: "<<y<<endl;

        // updating with proper value. At the moment y indicates the distance from the point to the top of the image
        // now y is the distance from point to image bottom
        int new_y=height-stoi(y);

        /// correcting the parallax:
        // translating the system to camera coordinates
        double translated_x = stoi(x) - camera_center ;

        // finding new x
        double rotated_x = translated_x+ translated_x * sin_angle_degrees;
        //cout<<new_x<<endl;

        double new_x = rotated_x + camera_center;

        string start_string = line.substr (0,pos_first_bracket);
        // saving to txt
        output_txt <<start_string<<"("<<new_x<<","<<new_y<<")\n";
        output_txt.flush();
    }

    // closing the txt files
    input_txt.close();
    output_txt.close();
    return 0;
}

/// Utility functions for drawParallax from now on
void drawParallax() {

    // Declaring functions for later use
    int changeCoordinates3();
    void draw();

    changeCoordinates3();
    ifstream input_txt ("../output4.txt");
    if (input_txt.is_open())
        cout << "Opened input file.txt\n";

    /// Create white empty image
    coord_image = cv::Scalar(255, 255, 255);

    string line;
    int iter=0;
    Point new_pt;

    std::thread first(draw);
    first.detach();

    while(getline(input_txt, line) && iter < 50000) {
        // extract the coordinates
        size_t pos_first_bracket = line.find("(");
        size_t pos_second_bracket = line.find(")");
        string coordinates = line.substr(pos_first_bracket + 1, pos_second_bracket - pos_first_bracket - 1);

        // detect position of the comma to separate x from y
        size_t pos_comma = coordinates.find(",");
        string x = coordinates.substr(0, pos_comma);
        string y = coordinates.substr(pos_comma + 1);
        //cout<<"x: "<<stod(x)<<", y: "<<stod(y)<<endl;

        // taking the last two points extracted from the txt

        new_pt = Point(stod(x), stod(y));

        // printing a line between the two points
        cv::line(coord_image, new_pt, new_pt, cv::Scalar(0,0,0), 2);
        unsigned int t = 50;
        usleep(t);
        iter++;
    }
    sleep(100);
    return;
}

void draw() {
    while (true) {
        imshow("Coordinates:", coord_image);
        int k = waitKey(1); // Wait for a keystroke in the window, press "s" to save the image
    }
}

int changeCoordinates3(){

    // declaring function to use later on
    point getPointOnTilted(double, point, double);
    point twoLineIntersection(point, point, point, point);

    double pixelCm = 0.2; //un pixel corrisponde a 0.2258 cm per come è il nostro sistema //todo controlla anche questo
    point camera = make_pair(130,140); //TODO controllale. Stima molto grossolana
    point pendulum = make_pair(150,50);
    //coefficiente angolare della retta passante per il pendolo e la fotocamera. Chiamiamola retta r
    double angularCoeff = (pendulum.second - camera.second)/(pendulum.first - camera.first);

    //non potendo usare l'equazione esplicita della retta perpendicolare alla retta r nel Pendolo, seleziono due punti
    //che si trovano su tale retta. Questi due punti permettono di rappresentare la retta in un altro modo a noi utile
    point tiltedA = getPointOnTilted(angularCoeff, pendulum, 0);
    point tiltedB = getPointOnTilted(angularCoeff, pendulum, 10);


    // frame height of raspberry
    int height=480;


    ifstream input_txt ("../Fri May 29 14-02-27 2020.txt");
    if (input_txt.is_open())
        cout << "Opened input file.txt\n";

    ofstream output_txt ("../output4.txt");
    if (output_txt.is_open())
        cout << "Opened output file.txt\n";

    string line;
    while(getline(input_txt, line)) {

        // extract the coordinates
        size_t pos_first_bracket = line.find("(");
        size_t pos_second_bracket = line.find(")");
        string coordinates = line.substr (pos_first_bracket+1, pos_second_bracket-pos_first_bracket-1);
        //cout<<coordinates<<endl;

        // detect position of the comma to separate x from y
        size_t pos_comma = coordinates.find(",");
        string x=coordinates.substr (0,pos_comma);
        string y=coordinates.substr (pos_comma+2);
        //cout<<"x: "<<x<<", y: "<<y<<endl;

        // updating with proper value. At the moment y indicates the distance from the point to the top of the image
        // now y is the distance from point to image bottom
        int new_y=height-stoi(y);

        /// correcting the parallax:
        // translating the system to camera coordinates
        double oldX = stoi(x);


        //uso i punti tilted estratti per calcolare l'interesezione tra la retta e la retta che passa tra la camera ed il
        //generico punto P

        point oldPoint = make_pair(oldX * pixelCm,0);
        point newPoint = twoLineIntersection(tiltedA, tiltedB, camera, oldPoint);

        string start_string = line.substr (0,pos_first_bracket);
        // saving to txt
        output_txt <<start_string<<"("<<newPoint.first/pixelCm<<","<<new_y<<")\n";
        output_txt.flush();
    }

    // closing the txt files
    input_txt.close();
    output_txt.close();
    return 0;
}

//lines camera,pendulum --> y = x * dx/dy - xp * dx/dy + yp     // m = dx/dy
//lines perpendicular -->   y = -1/m *x + (xp/m + yp)

//equazione della retta perpendicolare nel punto givenPoinat ad un'altra retta con coefficiente angolare noto.
point getPointOnTilted(double angularCoeff, point givenPoint, double x){
    double y = -x/angularCoeff + givenPoint.first/angularCoeff + givenPoint.second;
    return make_pair(x,y);
}

point twoLineIntersection(point A, point B, point C, point D) {
    // Line AB represented as a1x + b1y = c1
    double a1 = B.second - A.second;
    double b1 = A.first - B.first;
    double c1 = a1*(A.first) + b1*(A.second);

    // Line CD represented as a2x + b2y = c2
    double a2 = D.second - C.second;
    double b2 = C.first - D.first;
    double c2 = a2*(C.first)+ b2*(C.second);

    double determinant = a1*b2 - a2*b1;

    if (determinant == 0)
    {
        // The lines are parallel. This is simplified
        // by returning a pair of FLT_MAX
        std::cerr<<"Lines does NOT intersect"<<endl;
        return make_pair(FLT_MAX, FLT_MAX);
    }
    else
    {
        double x = (b2*c1 - b1*c2)/determinant;
        double y = (a1*c2 - a2*c1)/determinant;
        return make_pair(x, y);
    }
}
