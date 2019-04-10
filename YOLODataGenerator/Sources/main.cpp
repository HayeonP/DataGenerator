#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <opencv2\opencv.hpp>

#define SAVE_LOAD_PATH	"D:\\Users\\VisionWork\\Desktop\\박하연\\Projects\\TrainingDataGenerator\\YOLODataRefiner\\YOLODataRefiner\\data\\img"
//#define SAVE_LOAD_PATH	"D:\\Users\\VisionWork\\Desktop\\박하연\\DB\\미수행\\speed_bust_2_20190312"
#define VIDEO_PATH	"D:\\Users\\VisionWork\\Desktop\\박하연\\DB\\final\\rotate\\"
//#define VIDEO_PATH	"D:\\Users\\VisionWork\\Desktop\\박하연\\Projects\\TrainingDataGenerator\\YOLODataRefiner\\YOLODataRefiner\\data\\img"
#define TRAIN_DIR_PATH "D:\\Users\\VisionWork\\Desktop\\박하연\\DB\\txt"
#define TRAIN_FILE_PATH "D:\\Users\\VisionWork\\Desktop\\박하연\\DB\\txt\\train.txt"

#define TARGET_FPS	3
#define SEQ_FLAG 0
#define TXT_FLAG 0
#define TRAIN_FLAG 0

// Devide train & valid data
#define DEVIDE_TRAIN_VALID 1
#define DEVIDE_TRAIN_VALID_DIR "D:\\Users\\VisionWork\\Desktop\\박하연\\DB\\final"
#define TRAIN_RATE_MT_2500 0.8
#define TRAIN_RATE_LT_2500 0.9



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
	
	if (TRAIN_FLAG) {
		vector<string> final_data;
		for (auto& p1 : std::experimental::filesystem::directory_iterator(TRAIN_DIR_PATH)) {
			string filepath = p1.path().string();

			ifstream in_stream(filepath);

			string dirname, final_string;
			string front("data/img/");
			istringstream dirss(filepath);
			while (getline(dirss, dirname, '\\')) {}
			dirss = istringstream(dirname);
			getline(dirss, dirname, '.');

			while (!in_stream.eof()) {
				string data, prev, last;
				getline(in_stream, data);
				istringstream ss(data);
				while(getline(ss, last, '/')){}
				istringstream ss2(last);
				string temp;
				getline(ss2, temp, '.');
				getline(ss2, temp, '.');
				if (temp != string("jpg")) 
					continue;
				

				final_string = front + dirname + string("/") + last+"\n";
				final_data.push_back(final_string);
			}

			ofstream out_stream(TRAIN_FILE_PATH);
			for (auto it = final_data.begin(); it != final_data.end(); ++it) {
				out_stream << (*it);
			}

		}
	}

	if (DEVIDE_TRAIN_VALID) {
		vector<int> label_num(24);
		vector<int> train_num;
		int data_count[24];
		int train_data_count[24];
		int valid_data_count[24];

		for (int i = 0; i < 23; i++) {
			data_count[i] = 0;
			train_data_count[i] = 0;
			valid_data_count[i] = 0;
		}

		vector<string> label_names;

		label_names.push_back("Speed OTher");
		label_names.push_back("Child Protect");
		label_names.push_back("No Left");
		label_names.push_back("No Right");
		label_names.push_back("No Parking");
		label_names.push_back("Crosswalk");
		label_names.push_back("No Uturn");
		label_names.push_back("Rotate");
		label_names.push_back("Slow");
		label_names.push_back("Speed Bust");
		label_names.push_back("Unprotected Left");
		label_names.push_back("Uturn");
		label_names.push_back("No Straight");
		label_names.push_back("Speed 20");
		label_names.push_back("Speed 30");
		label_names.push_back("Speed 40");
		label_names.push_back("Speed 50");
		label_names.push_back("Speed 60");
		label_names.push_back("Speed 70");
		label_names.push_back("Speed 80");
		label_names.push_back("Speed 90");
		label_names.push_back("Speed 100");
		label_names.push_back("Traffic Sign");
		label_names.push_back("ALL");

		int a = 0;

		for (auto& p : std::experimental::filesystem::directory_iterator(DEVIDE_TRAIN_VALID_DIR)) {
			string dirname = p.path().string();
			string filename_1;
			string trainname = p.path().string() + string("\\train.txt");
			cout << dirname << endl;
			vector<string> data;

			stringstream ss_1(dirname);
			while (getline(ss_1, filename_1, '\\'));
			//if((filename_1.find("19") == std::string::npos)) continue;

			ifstream in_stream(trainname);
			string input;

			while (!in_stream.eof()) {
				getline(in_stream, input);
				data.push_back(input);
			}
			in_stream.close();

			for (int i = 0; i < data.size() - 1; i++) {
				istringstream ss(data[i]);
				string filename;

				while (getline(ss, filename, '/')) {}
				ss = istringstream(filename);
				getline(ss, filename, '.');
				filename += ".txt";
				filename = dirname + "\\" + filename;

				vector<string> bbdata;

				ifstream in_stream(filename);
				string input;

				while (!in_stream.eof()) {
					getline(in_stream, input);
					bbdata.push_back(input);
				}
				in_stream.close();


				for (int i = 0; i < bbdata.size(); i++) {
					stringstream ss(bbdata[i]);
					int label;
					string s;
					ss >> s;
					if (s == string("")) {
						continue;
					}
					label = stoi(s);
					label_num[label]++;
				}
				label_num.back() += 1;
			}
			cout << "OK!" << endl;
		}

		cout << "[ Total data num ]" << endl;
		for (int i = 0; i < label_num.size(); i++) {
			cout << label_names[i] << " : " << label_num[i] << endl;
		};

		// Set number of training data
		for (auto it = label_num.begin(); it != label_num.end(); ++it) {
			if (*it >= 2500) 
				train_num.push_back(*it * TRAIN_RATE_MT_2500);
			else 
				train_num.push_back(*it * TRAIN_RATE_LT_2500);
		}

		cout << "[ Train num ]" << endl;
		for (int i = 0; i <train_num.size(); i++) {
			cout << label_names[i] << " : " << train_num[i] << endl;
		};


		string train_name("devided_train.txt");
		string valid_name("devided_valid.txt");

		vector<string> train_list;
		vector<string> valid_list;

		for (auto& p : std::experimental::filesystem::directory_iterator(DEVIDE_TRAIN_VALID_DIR)) {
			
			
			string dirname = p.path().string();
			string filename_1;
			string trainname = p.path().string() + string("\\train.txt");
			vector<string> data;

			cout <<"Check : " << dirname << endl;

			stringstream ss_1(dirname);
			while (getline(ss_1, filename_1, '\\'));

			ifstream in_stream(trainname);
			string input;

			while (!in_stream.eof()) {
				getline(in_stream, input);
				data.push_back(input);
			}
			in_stream.close();

			for (int i = 0; i < data.size() - 1; i++) {
				istringstream ss(data[i]);
				istringstream ss2(dirname);
				string filename;
				string savename;
				string dir;
				vector<string> prev;
				
				while (getline(ss, filename, '/')) {}

				int flag = 0;
				while (getline(ss2, dir, '\\')) {
					if (prev.size() < 2) prev.push_back(dir);
					else {
						prev[0] = prev[1];
						prev[1] = dir;
					}
					
				}
				dir = prev[1];

				ss = istringstream(filename);
				savename = filename;

				getline(ss, filename, '.');
				filename += ".txt";
				filename = dirname + "\\" + filename;

				

				vector<string> bbdata;

				ifstream in_stream(filename);
				string input;

				while (!in_stream.eof()) {
					getline(in_stream, input);
					bbdata.push_back(input);
				}
				in_stream.close();

				bool save_flag = false;
				for (int i = 0; i < bbdata.size(); i++) {
					stringstream ss(bbdata[i]);
					int label;
					string s;
					ss >> s;
					if (s == string("")) {
						continue;
					}
					label = stoi(s);
					data_count[label]++;
					if (data_count[label] >= train_num[label]) {
						valid_data_count[label]++;
						save_flag = true;
					}
					else {
						train_data_count[label]++;
					}
				}

				savename = std::string("data/img/") + dir + string("/") + savename;

				if (save_flag) valid_list.push_back(savename);
				else train_list.push_back(savename);
			}

			std::cout << train_list.size() << "  " << valid_list.size() << endl;

		}

		ofstream writeTrain(DEVIDE_TRAIN_VALID_DIR + string("\\") + train_name);
		ofstream writeValid(DEVIDE_TRAIN_VALID_DIR + string("\\") + valid_name);

		for (auto it = train_list.begin(); it != train_list.end(); ++it) {
			writeTrain << *it << endl;
		}

		for (auto it = valid_list.begin(); it != valid_list.end(); ++it) {
			writeValid << *it << endl;
		}

		writeTrain.close();
		writeValid.close();

		cout << "[ Train label num ]" << endl;
		for (int i = 0; i < label_names.size(); i++) {
			cout << label_names[i] << " : " << train_data_count[i] << endl;
		};

		cout << "[ Valid label num ]" << endl;
		for (int i = 0; i < label_names.size(); i++) {
			cout << label_names[i] << " : " << valid_data_count[i] << endl;
		};

		cout << "Result : " << train_list.size() << " " << valid_list.size() << endl;
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
	size_t start_pos = 0; //string처음부터 검사
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)  //from을 찾을 수 없을 때까지
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // 중복검사를 피하고 from.length() > to.length()인 경우를 위해서
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
