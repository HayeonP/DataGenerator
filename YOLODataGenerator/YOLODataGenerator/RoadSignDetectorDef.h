#pragma once

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>

// RESOLUTION
#define IMG_WIDTH				1280		//1280
#define IMG_HEIGHT				672			//672
#define DEF_AVI_SPEED           40
#define DEF_AVI_SPEED_MIN       1
#define DEF_AVI_SPEED_MAX       100
#define DEF_AVI_SPEED_SCALE     2
#define VIDEO_INPUT_WIDTH_SIZE  720
#define VIDEO_INPUT_HEIGHT_SIZE 480

// ROAD SIGNS
#define SPEED					0
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
#define ROADSIGN_NUM			13

// FLAG & PARAMETER
#define PI						3.14
#define FLIP_FLAG				0
#define HAAR_THRESHOLD			900
#define NO_THRESHOLD			0.18

// FUNCTIONS
#define INDEX_3CH(X, Y, W)		(Y) * W  + (X) * 3
#define INDEX_1CH(X, Y, W)		(Y) * W + (X)

#define max(x,y) ((x) > (y) ? (x) : (y))
#define min(x,y) ((x) < (y) ? (x) : (y))

#define CASCADE_PATH ".\\cascade\\"
#define SPEED_NUN_PATH "speed_num_image"
#define LOAD_PATH "D:\\Users\\VisionWork\\Desktop\\박하연\\DB\\SequenceImage\\190110-2"
#define LOAD_NAME "child_protect_"


// DEBUG
#define MASK_FLAG				0


struct sPoint {
	double x, y;
};

struct sCircle {
	double cx, cy;   // 원의 중심
	double r;      // 원의 반지름
};