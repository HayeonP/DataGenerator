#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

#define SAVE_PATH	"D:\\Users\\VisionWork\\Desktop\\박하연\\DB\\SequenceImage\\190103_2"
#define VIDEO_PATH	"D:\\Users\\VisionWork\\Desktop\\박하연\\DB\\BlackBox\\190103_2"
#define TARGET_FPS	1

vector<string> get_filepaths_in_directory(string path);

int main() {

	vector<string> video_paths = get_filepaths_in_directory(VIDEO_PATH);

	for (auto it = video_paths.begin(); it != video_paths.end(); it++) {
		string filepath = (*it);

		string filename;

		istringstream ss(filepath);
		while (getline(ss, filename, '\\')) {}
		ss = istringstream(filename);
		getline(ss, filename, '.');

		VideoCapture vc(filepath);
		if (!vc.isOpened()) {
			cout << "Cannot open video file" << endl;
			return -1;
		}

		double fps_origin = vc.get(CV_CAP_PROP_FPS);

		Mat frame;

		int skip_count = 0;
		int save_count = 0;
		while (1) {
			vc >> frame;
			if (frame.empty())
				break;
			else
			{
				skip_count++;
				waitKey(1000 / fps_origin);
				if (skip_count == fps_origin / TARGET_FPS) {
					imshow("video", frame);
					cout << SAVE_PATH + string("\\") + filename + "_" + to_string(save_count) + string(".jpg") << endl;
					cout<<imwrite(SAVE_PATH + string("\\") + filename + "_" + to_string(save_count) + string(".jpg"), frame)<<endl;
					skip_count = 0;
					save_count++;
				}
			}
		}
	}

	return 0;

}

vector<string> get_filepaths_in_directory(string path) {
	vector<string> paths;

	for (auto& p1 : std::experimental::filesystem::directory_iterator(path)) {
		paths.push_back(p1.path().string());
	}

	return paths;
}