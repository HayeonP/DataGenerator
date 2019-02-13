#include <iostream>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>

using namespace std;
using namespace cv;

#define PATH "D:\\Users\\VisionWork\\Desktop\\¹ÚÇÏ¿¬\\DB\\SequenceImage"
#define CHANGE_RATE 1.2

int main() {
	for (auto& q : std::experimental::filesystem::directory_iterator(PATH)) {
		string _path = q.path().string();
		
		for (auto& p : std::experimental::filesystem::directory_iterator(_path)) {
			string filepath = p.path().string();
			string extension;
			istringstream ss(filepath);
			while (getline(ss, extension, '\\')) {}
			ss = istringstream(extension);

			if (extension == "train.txt") continue;

			getline(ss, extension, '.');
			getline(ss, extension, '.');
			if (extension != "txt") continue;

			vector<string> data;

			ifstream in_stream(filepath);
			string input;

			while (!in_stream.eof()) {
				getline(in_stream, input);
				if ((int)(input.c_str()[0]) == 0) continue;
				data.push_back(input);
			}
			in_stream.close();

			ofstream out_stream(filepath);
			for (int idx = 0; idx < data.size(); idx++) {
				stringstream ss(data[idx]);
				int label;
				double x_center, y_center, width, height;
				ss >> label;
				ss >> x_center;
				ss >> y_center;
				ss >> width;
				ss >> height;
				width = width * CHANGE_RATE;
				height = height * CHANGE_RATE;
				out_stream << label << " " << x_center << " " << y_center << " ";
				out_stream << width << " " << height << endl;
			}
			out_stream.close();

			cout << filepath << " complete!" << endl;
		}
	}
	
}