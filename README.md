Training Data Generator
===============

RoadSignDetector
----------------
It detects Koeran traffic roadsigns on image and create data satisfying YOLO's training data format.


YOLODataRefiner
---------------
It is kind of labeler for refining or creating ground truth on image.
It targets to satisfy YOLO's training data foramt.

CascadeDataExpander
-------------------
Diversify the training data for cascade classifying by change the brightness.


SaveFixedFpsVideoFrames
-----------------------
Save video frames at fixed fps.


YOLODataGenerator
-----------------
Generate sequence data and empty {image name}.txt, train.txt files


YOLODataNumChecker
------------------
Count the number of labels in dataset


BoundingBoxSizeChanger
----------------------
Change the bounding boxes size by same ratio
