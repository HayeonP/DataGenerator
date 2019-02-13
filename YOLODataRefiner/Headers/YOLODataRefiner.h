#pragma once
#include "YOLODataRefinerDef.h"

using namespace std;
using namespace cv;

struct Candidate {
	Rect location;
	int line_num;
};

struct Move{
	Rect location = Rect();
	Candidate select = Candidate();
};

class YOLODataRefiner {
public:
	YOLODataRefiner(string _screen_name, string path,int _button_state, int _label_state);
	static void CallBackFunc(int event, int x, int y, int flags, void* userdata);
	void drawScreen();
private:
	int button_state;
	int label_state;
	int screen_width;
	int screen_height;
	int button_width;
	int button_height;
	int label_size;
	int data_idx;
	int data_num;

	int current_idx;

	string screen_name;
	Mat src, screen;
	vector<Mat> label_images;
	vector<Rect> label_locations;
	vector<Rect> button_locations;
	vector<Candidate> candidates[LABEL_NUM];
	vector<Scalar> colors;
	vector<string> label_names;
	vector<string> button_names;
	vector<string> img_list;
	vector<string> txt_list;
	Rect next_location;
	Rect prev_location;
	
	Candidate select;
	Move move;
	Point create_1, create_2;
	
	bool move_flag;
	bool create_flag;

};

string ReplaceAll(std::string &str, const std::string& from, const std::string& to);

