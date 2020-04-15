#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(int, char**) {

    // Initialization of VideoCapture
    const String my_path="/Users/claudia/Desktop/test.mp4";
    VideoCapture cap(my_path);
    //TODO uncomment the following line on Raspberry to get the camera
    //VideoCapture cap(0); // open the default camera
    if(!cap.isOpened()){
        // check if we succeeded
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    // Start grabbing frames
    Mat frame;
    cout << "Start grabbing" << endl
         << "Press any key to terminate" << endl;

    for (;;) {
        // wait for a new frame from camera and store it into 'frame'
        cap.read(frame);
        // check if we succeeded
        if (frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }
        // show live and wait for a key with timeout long enough to show images
        imshow("Live", frame);
        if (waitKey(5) >= 0)
            break;
    }
    // the camera will be de-initialized automatically in VideoCapture destructor
    return 0;
}
