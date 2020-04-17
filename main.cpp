#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(int, char**) {

    // Initialization of VideoCapture
    // Test done with video 1920x1080
    const String my_path="../test.mp4";
    VideoCapture video(my_path);

    //TODO uncomment the following lines on Raspberry to get the camera and set fps rate
    //VideoCapture video(0); // open the default camera
    //setting fps rate of video to grab, it should work with the raspberry camera
    //video.set(CAP_PROP_FPS, int(30));

    if(!video.isOpened()){
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
        video.read(frame);
        // check if we succeeded
        if (frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        // store frame in folder
        std::stringstream ss;
        ss << "../frames/frame_" << video.get(0) << ".jpg";
        std::string filename = ss.str();
        imwrite(filename, frame);

        // show live and wait for a key with timeout long enough to show images
        imshow("Live", frame);
        if (waitKey(5) >= 0)
            break;
    }
    // the camera will be de-initialized automatically in VideoCapture destructor
    return 0;
}