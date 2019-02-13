#pragma once

#include "RoadSignDetectorDef.h"
#include "BlobLabeling.h"
#include "MatrixAlgebra.h"
#include "CompressiveTracker.h"
#include <opencv2/opencv.hpp>
#include <fstream>
#include <functional>
#include <vector>

using namespace std;
using namespace cv;

class RoadSignDetector {
public:
	RoadSignDetector();
	~RoadSignDetector();
	bool loadCascade(string name);
private:
	IplImage* _Number[10];
	char _numName[128];
	int resultNum[3][2];
	int digit;
	bool _loadFlg;
	string filename;
	vector<string> data_list;
	
	vector<string> _RoadSignNames;
	vector<CascadeClassifier> _RoadSignClassifier;
	vector< vector<Rect> > _RoadSignLocations;
	vector< vector<Rect> > _FilteredRoadSignLocations;
	vector< Mat > _NoFilteredCandidates;
	vector< Mat > _FilteredCandidates;
	vector< function<int(Mat, Mat)> > _Filters;
	int _DetectResult[ROADSIGN_NUM];

	void createCandidate(Mat src, Mat hist, int idx);
	void autowb(Mat& src, Mat& dst);

	int _Speed_Filter(Mat src, Mat grey);
	int _Child_Protect_Filter(Mat src, Mat grey);
	int _No_Left_Filter(Mat src, Mat grey);
	int _No_Right_Filter(Mat src, Mat grey);
	int _No_Parking_Filter(Mat src, Mat grey);
	int _Crosswalk_Filter(Mat src, Mat grey);
	int _No_Uturn_Filter(Mat src, Mat grey);
	int _Rotate_Filter(Mat src, Mat grey);
	int _Slow_Filter(Mat src, Mat grey);
	int _Speed_Bust_Filter(Mat src, Mat grey);
	int _Unprotected_Left_Filter(Mat src, Mat grey);
	int _Uturn_Filter(Mat src, Mat grey);
	int _No_Straight_Filter(Mat src, Mat grey);
		
	void showOnce(String name, Mat src);
	Mat hsv_filter(Mat src, Rect location, double minHue, double maxHue, double minSat, double maxSat, double minVal, double maxVal);
	Mat RGB_filter(Mat src, Rect location, double minR, double maxR, double minG, double maxG, double minB, double maxB);
	void show_filtered_candidates(Mat src, int sign_type);

	vector<string> getRoadSignNames();
	vector<CascadeClassifier> getRoadSignClassifier();
	vector< vector<Rect> > getRoadSignLocations();
	vector< vector<Rect> > getFilteredRoadSignLocations();
	vector< Mat > getNoFilteredCandidates();
	vector< Mat > getFilteredCandidates();

	void BubbleSorting(int num[3][2]);

public:
	vector < function<int(Mat, Mat)> > _RoadSignFilter;
	Mat RoadSignDetect(Mat& src, string filename);
	string normalize_location(Mat src, int label, Rect location);
	vector< string > getDataList() { return data_list; };
};

string change_file_extention(string filename, string extention);
void write_file(string path, vector<string> contents);