#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <opencv2\opencv.hpp>

#define SAVE_LOAD_PATH	"D:\\Users\\VisionWork\\Desktop\\���Ͽ�\\DB\\SequenceImage\\190124_b"
//#define SAVE_LOAD_PATH	"D:\\Users\\VisionWork\\Desktop\\���Ͽ�\\Projects\\TrainingDataGenerator\\YOLODataRefiner\\YOLODataRefiner\\data\\img"
#define VIDEO_PATH	"D:\\Users\\VisionWork\\Desktop\\���Ͽ�\\DB\\BlackBox\\190124_b"
//#define VIDEO_PATH	"D:\\Users\\VisionWork\\Desktop\\���Ͽ�\\Projects\\TrainingDataGenerator\\YOLODataRefiner\\YOLODataRefiner\\data\\img"
#define TARGET_FPS	3
#define SEQ_FLAG 1
#define TXT_FLAG 1


using namespace std;
using namespace cv;

vector<string> get_filepaths_in_directory(string path);
string ReplaceAll(std::string &str, const std::string& from, const std::string& to);
string change_file_extention(string filename, string extention);
void write_file(string path, vector<string> data);

int main(int argc, char** argv) {

	if (SEQ_FLAG) {
		// Create Sequence Images
		vector<string> video_paths = get_filepaths_in_directory(VIDEO_PATH);

		for (auto it = video_paths.begin(); it != video_paths.end(); it++) {
			string filepath = (*it);

			string filename;

			istringstream ss(filepath);
			while (getline(ss, filename, '\\')) {}
			ss = istringstream(filename);
			getline(ss, filename, '.');
			cout << filename << endl;


			VideoCapture vc;
			vc.open(filepath);

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
					//cout << skip_count << endl;
					waitKey((int)(1000 / fps_origin));
					if (skip_count == (int)fps_origin / TARGET_FPS) {
						// imshow("video", frame);
						string save_file_path = SAVE_LOAD_PATH + string("\\") + filename + "_" + to_string(save_count) + string(".jpg");
						ReplaceAll(save_file_path, " ", "_");
						cout << imwrite(save_file_path, frame) << endl;
						cout << save_file_path << endl;
						skip_count = 0;
						save_count++;
					}
				}
			}
		}
	}
	
	if (TXT_FLAG) {
		int i = 0;

		vector<string> data_list;
		for (auto& p1 : std::experimental::filesystem::directory_iterator(SAVE_LOAD_PATH)) {
			string filename = p1.path().string();
			string data_name;
			istringstream ss(filename);
			while (getline(ss, data_name, '\\')) {}
			data_list.push_back("data/img/" + data_name + "\n");
			filename = change_file_extention(filename, string("txt"));
			string txt_path = SAVE_LOAD_PATH + string("\\") + filename;
			ofstream of(txt_path);

		}

		write_file(SAVE_LOAD_PATH + string("\\train.txt"), data_list);
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

string ReplaceAll(std::string &str, const std::string& from, const std::string& to) {
	size_t start_pos = 0; //stringó������ �˻�
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)  //from�� ã�� �� ���� ������
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // �ߺ��˻縦 ���ϰ� from.length() > to.length()�� ��츦 ���ؼ�
	}
	return str;
}

string change_file_extention(string filename, string extention) {
	istringstream ss(filename);
	while (getline(ss, filename, '\\')) {}
	ss = istringstream(filename);
	getline(ss, filename, '.');
	filename = filename + "." + extention;
	return filename;
}

void write_file(string path, vector<string> data) {
	ofstream out_stream(path);
	for (int idx = 0; idx < data.size(); idx++) {
		out_stream << data[idx];
	}
	out_stream.close();
}
