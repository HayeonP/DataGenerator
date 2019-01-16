#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <filesystem>
#include <sstream>
#include <istream>

using namespace std;
using namespace cv;

#define LABEL_PATH "Labels"
#define IMAGE_PATH "data\\img"
#define TRAIN_PATH "data\\img\\train.txt"

#define SPEED_OTHER				0
#define CHILD_PROTECT			1
#define NO_LEFT					2
#define NO_RIGHT				3
#define NO_PARKING				4
#define CROSSWALK				5
#define NO_UTURN				6
#define ROTATE					7
#define SLOW					8
#define SPEED_BUST				9
#define UNPROTECTED_LEFT		10
#define UTURN					11
#define NO_STRAIGHT				12
#define SPEED_20				13
#define SPEED_30				14
#define SPEED_40				15
#define SPEED_50				16
#define SPEED_60				17
#define SPEED_70				18
#define SPEED_80				19
#define SPEED_90				20
#define SPEED_100				21
#define ALL						22
#define LABEL_NUM				23

#define MOVE				0
#define ERASE				1
#define CREATE				2
#define QUIT				3

#define START_IDX			1144
