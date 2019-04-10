#include <iostream>
#include <vector>
#include <filesystem>
#include <sstream>
#include <fstream>

using namespace std;

//#define PATH "D:\\Users\\VisionWork\\Desktop\\박하연\\Projects\\TrainingDataGenerator\\YOLODataRefiner\\YOLODataRefiner\\data\\img"
#define PATH "D:\\Users\\VisionWork\\Desktop\\박하연\\DB\\final\\a"
#define TOTAL 0

int main() {
	vector<int> label_num(24);
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

	for (auto& p : std::experimental::filesystem::directory_iterator(PATH)) {
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

		if (TOTAL == 0) {
			cout << "[ " << filename_1 << " ]" << endl;
			for (int i = 0; i < label_num.size(); i++) {
				cout << label_names[i] << " : " << label_num[i] << endl;
				label_num[i] = 0;
			}
		}
		
	}

	if (TOTAL == 1) {
		cout << "[ Total ]" << endl;
		for (int i = 0; i < label_num.size(); i++) {
			cout << label_names[i] << " : " << label_num[i] << endl;
		};
	}
	

	return 0;
}