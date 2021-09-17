# Foucault pendulum tracking system
![GitHub last commit](https://img.shields.io/github/last-commit/FrancescoScandiffio/Foucault-pendulum-tracking-system)

## About the project
The purpose of this project is to develop a program capable of tracking a Foucault pendulum with the aim of collecting motion data that can be compared with the expected theoretical trend. Below we describe the main features of the system, the execution modes and the structure of the repository. For more information refer to the content of the ```\documentation``` folder.

### Assets deployed
The hardware consists of:
- A Foucault pendulum.
- An intermittent electromagnetic coil placed under the center of the base of the pendulum. The magnetic field counteracts the damping of oscillations and keeps the pendulum moving.
- A <a href="https://www.raspberrypi.org/products/raspberry-pi-4-model-b/specifications/">Raspberry Pi 4</a>.
- A camera placed over the pendulum and connected to the Raspberry Pi. The camera frames the pendulum from above but not perpendicularly to the base of the structure: the camera is therefore shifted horizontally and tilted. To eliminate the distortion that occurs due to camera tilting, we developed a [remote camera calibration system](#perspective-correction).



### Main requirements
The system needs to comply with the following requirements:
- Locate the center of the pendulum in real-time for each frame captured by the camera.
- Collect at least 5 coordinates per second. 
- Apply a perspective correction strategy.
- Accurate extraction of the coordinates: maximum error under 10 pixels per axis
- Store the coordinates in a CSV file.
- Show a 2D graph of the pendulum trajectory, real-time.

## Implementation specs
Here below are reported some aspects regarding the techniques adopted in the realisation of the software.

### Object detection
The first operation to be performed on the frames extracted from the camera is to locate the pendulum in the scene. This is done using the Template Matching algorithm, in particular the **Zero-normalised cross correlation function** (ZNCC). This algorithm requires an image of the object to be detected, called Template. Since no pre-processing is applied to the images, the object should have the same orientation and size in both Template and source frames: to obtain a good Template it is sufficient to cut a frame obtained from the camera. Attaching a marker to the object in a colour not present in the rest of the image improves accuracy: in our case we used red tape.

<p float="left" align="center">
  <img src="./images/TemplateAndDetection.png" width="35%"  />
</p>
<p align="center">
    <i>Fig 1. Template matching applied to a frame.</i>
</p>

### Multithreading
A multi-threading strategy has been applied in order to increase the number of frames analyzed per second, hence the number of object coordinates per second. Since the raspberry Pi-4 has 4 cores, a 4-thread structure has been employed. Through experimental tests we have verified that a greater number of threads causes the device to heat up, thermal throttling issues, performance degradation and eventually a device failure.
The threads are arranged in the following 3-tier processing chain which takes inspiration from the Producer-Consumer pattern:
1. The Main thread initializes the program and extracts frames from the camera.
2. Two threads apply the matching algorithm to the frames (one frame per thread) and forward the detected coordinates to the third layer.
3. One thread sorts the received points by the id of the frames, append them to a CSV file and displays real-time information about the motion of the tracked object.

<p float="left" align="center">
  <img src="./images/threadFlowChart.png" width="50%"  />
</p>
<p align="center">
    <i>Fig 2. Multi-threading flowchart of threads.</i>
</p>


**QUI DA FINIRE**
The threads have been organized as two cascading producer-consumers. The <i>main</i> thread that produces a frame extracted from the camera and that is sent to one of the two threads for processing. 
Alternately a frame is inserted in one of the two queues consumed by their respective thread for pendulum detection. 
The result is then sent to a queue waiting for the writing thread to insert it in a CSV document.

### Perspective correction
By the definition of <i> parallax effect </i> the position of an object seems different with respect to the background when it is viewed along two
different lines of sights.
As the camera was not positioned perfectly above the pendulum, but slightly tilted, it was necessary to devise a system for adjusting the perspective.
The idea was then to apply a perspective transformation to the acquired frames so as to remove the effects of inclination, thus obtaining images projected on the plane perpendicular to the focal axis.

The first time the user decides to apply the perspective correction, in the calibration window it is required to visually position the vertices of a quadrilateral on which to apply the perspective.
A further window, on the side, shows the warped frame. The points chosen by the user are stored in a file and used by the tracking system to perform real-time perspective correction. In later uses of the application the user can decide to re-position the points, use existing points, or start the program without perspective correction. An example of the calibration file created by the program can be found in ```\release\```.

<p align="center">
    <i>Calibration interface:</i>
</p>
<p float="left" align="center">
  <img src="./images/unifiedCalibration.png" width="50%"  />
</p>

### Output file
The coordinates extracted by the system are stored in a CSV file. Each entry of the file is in this format: ```Time;X;Y``` where:
- Time is expressed in seconds since the start of the computation. 
- (X,Y) coordinates in pixel. The axis origin is placed in the top-left corner of the image.

### Graphical interface
The main goal of the GUI is to allow the user to visualise at a glance the real-time stream from the camera showing a feedback of the detection as we can observe from the figure below. 

<p align="center">
    <i>Tracking interface:</i>
</p>
<p float="left" align="center">
  <img src="./images/template_interfaccia.png" width="30%"  />
</p>

From this interface can also be shown a graph of points detected in real-time (up to 1000). As new points come, the older coordinates are deleted from the graph.

It is also made available an offline version of the graph. This is very similar to the online version, but showing coordinates read from a CSV of choice. However, it has some additional features:
- The appearing of new coordinates can be stopped and played.
- The speed of visualisations of new points can be changed.
- The number of points displayed at a time can be changed as well (30 by default) with no upper limit.

<p align="center">
    <i>Offline graph:</i>
</p>
<p float="left" align="center">
  <img src="./images/graph_offline.png" width="50%"  />
</p>

On the upper left corner the time in seconds of the last point inserted. 

## Repository structure
- The ```\documentation``` folder contains further information about the project. 
- The ```\images``` folder contains generic images. 
- The folder ```\oldScripts``` contains previous versions of the features.
- The folder ```\release``` contains the updated version of the program.
- The file ```\release\template.png``` is the template image used for detection.
- The file ```\release\calibration.txt``` is an example of calibration file generated by the program after configuring the perspective correction with the proper tool.
- Inside ```\release\source\```:
    - ```\main.cpp``` executes the main part of the program, that is: frame extraction, pendulum recognition, application of the perspective if requested, drawing of the online graph if requested and finally writing of the results to a file.
    - ```\calibration.cpp``` contains the code for the calibration interface.
    - ```\offlineGraph.cpp``` realizes the tool for drawing the offline graph of points.

## How to run the program
In order to use the program it first needs to be compiled. We first move to the directory where it is located, for example:

```cd Desktop/ProjectFolder/release/source```

Then to compile:
```
g++ calibration.cpp main.cpp offlineGraph.cpp −o tracciamentoPendolo −I/usr/local/include/opencv4/ −L/usr/local/lib −I/home/pi/Desktop/ProjectFolder/headers −lopencv core −lopencv imgproc −lopencv highgui −lopencv ml −lopencv video −lopencv features2d −lopencv calib3d −lopencv objdetect −lopencv flann −lopencv videoio −lopencv imgcodecs −lpthread
```

The program is ready to be executed with the simple command ```./tracciamentoPendolo```, or with further optional arguments:
- The option ```-c``` or ```-calibrate``` to execute the program in calibration mode.
- The option ```-g``` or ```-graph``` to execute the program in offline mode to draw the graph.
- The option ```-h``` or ```-help``` to obtain info about the command line arguments available.

If the program is executed with no arguments it will start the tracking. The user will be asked if they want to apply perspective correction or not. To interrupt the execution of the program it is sufficient to type ```Ctrl + c```. 

For further information upon the usage of the program with the other configuration, please refer to the user manual that can be found in the documentation. 


## Authors
- [Claudia Raffaelli](https://github.com/ClaudiaRaffaelli)
- [Francesco Scandiffio](https://github.com/FraScandiffio)

## Acknowledgments
Laboratory of automatic control project - Computer Engineering Master Degree @[University of Florence](https://www.unifi.it/changelang-eng.html)
