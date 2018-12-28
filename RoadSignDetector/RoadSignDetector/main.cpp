#include <iostream>
#include <opencv2\opencv.hpp>
#include "RoadSignDetector.h"

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
	RoadSignDetector roadSignDetector;
	
	int i = 0;

	for (auto& p1 : std::experimental::filesystem::directory_iterator(LOAD_PATH)) {
		Mat src = imread(p1.path().string());
		if ( src.empty() ) continue;
		roadSignDetector.RoadSignDetect(src, p1.path().string());
	}

	write_file(LOAD_PATH + string("\\train.txt"), roadSignDetector.getDataList());

	return 0;
}