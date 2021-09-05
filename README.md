# Foucault pendulum tracking software
![GitHub last commit](https://img.shields.io/github/last-commit/FraScandiffio/Foucault)

## About the project
The purpose of this project is to develop a program capable of tracking a Foucault pendulum with the goal of investigating its behavior. 

### Assets deployed
The hardware consists of:
- A Foucault pendulum placed on an electromagnetic coil, so as to make it swing indefinitely.
- A Raspberry Pi 4.
- A camera, placed over the pendulum and connected to the Raspberry Pi. The camera frames the pendulum from above, and it is slightly toed in, 
resulting in a captured scene mildly skewed. This issue will be addressed by applying a perspective correction on the frames. Moreover the camera is of type
Rolling Shutter: the frame pixels are not acquired at the same time and this can lead to some distortions. 
This is especially true when dealing with moving objects within a frame. 

### Main requirements
The system needs to comply with the following requirements:
- Locate the center of the pendulum in real-time for each frame captured by the camera.
- Accurate extraction of the coordinates.
- Fast enough to collect real-time data of an adequate number of points for each oscillation.
- Ability to store the positions collected in a file, taking care of applying the mentioned perspective correction.
- Have an utility to draw a 2D graph of the pendulum trajectory.
- A graphical user interface that allows to easily perform all the desired tasks.

## Implementation specs
Here below are reported some aspects regarding the techniques adopted in the realisation of the software.

### Tracking algorithm: template matching
With this technique the recognition of the center of the pendulum is achieved by searching within each frame captured by the camera, for the group of pixels that is most similar to the content of an another image used as reference, the Template.
The template used for this project can be found in the folder ```\images```.
<p align="center">
    <i>Template matching applied to a frame:</i>
</p>
<p float="left" align="center">
  <img src="./images/TemplateAndDetection.png" width="35%"  />
</p>

The developed program does not implement any kind of pre-processing on the Source image nor to the Template, such as rotation or size normalization. 
The object to be identified needs to have the same dimensions and orientation in both Template and Source.
Also the Template holds, besides the marker, a portion of the near area for better performances.

### Multithreading
In order to improve the performances of the system that was capable of handling 5 FPS with the template matching algorithm has been introduced a multi-threading solution.
Threads are arranged in a consumer producer pattern. In particular the threads involved are:
- A main thread that extracts frames from the camera.
- Two computing threads that consume frames, and produce results.
- A writer thread that handles the writing to the CSV file.

A greater number of threads on the 4 cores Raspberry Pi would have decreased the performances rather than increase them. 
The number of FPS the system can handle is 9. In fact, a higher number did not allow the system to process frames fast enough,
causing queues to fill up without being consumed fast enough.

<p align="center">
    <i>Multi-threading flowchart of threads:</i>
</p>
<p float="left" align="center">
  <img src="./images/threadFlowChart.png" width="50%"  />
</p>

The threads have been organized as two cascading producer-consumers. The <i>main</i> thread that produces a frame extracted from the camera and that is sent to one of the two threads for processing. 
Alternately a frame is inserted in one of the two queues consumed by their respective thread for pendulum detection. 
The result is then sent to a queue waiting for the writing thread to insert it in a CSV document.

### Perspective correction
By the definition of <i> parallax effect </i> the position of an object seems different with respect to the background when it is viewed along two
different lines of sights.
As the camera was not positioned perfectly above the pendulum, but slightly tilted, it was necessary to devise a system for adjusting the perspective.
The idea was then to apply a perspective transformation to the acquired frames so as to remove the effects of inclination, 
thus obtaining images projected on the plane perpendicular to the focal axis.

The first time the user decides to apply the perspective correction, in the calibration window the user is required to visually position the vertices of a quadrilateral
on which to apply the perspective.
A further window, on the side, shows the warped frame.
The points chosen by the user are stored in a file and used by the tracking system to perform real-time perspective correction. 
In later uses of the application the user can decide to re-position the points, use existing points, or start the program without perspective correction.

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

## How to run the program

## Authors
- [Claudia Raffaelli](https://github.com/ClaudiaRaffaelli)
- [Francesco Scandiffio](https://github.com/FraScandiffio)

## Acknowledgments
Laboratory of automatic control project - Computer Engineering Master Degree @[University of Florence](https://www.unifi.it/changelang-eng.html)
