#include <iostream>
#include <string>
#include <deque>
#include <filesystem>
#include <opencv2\opencv.hpp>
#include <sstream>

#define LOAD_PATH "C:\\Users\\vision\\Desktop\\표지판데이터2\\Origin"
#define SAVE_PATH "C:\\Users\\vision\\Desktop\\표지판데이터2\\Final"
#define DEBUG_FLAG 0
#define BRIGHT_ALPHA 1.2
#define BRIGHT_BETA 30
#define DARK_ALPHA 0.9
#define DARK_BETA -30

using namespace std;
using namespace cv;

deque<string> label_name;

void label_init();
void modify_brightness(Mat src, Mat& dst, double alpha, int beta);

int main() {
	label_init();

	for (auto& p1 : std::experimental::filesystem::directory_iterator(LOAD_PATH)) {
		int cnt = 1001;

		if (label_name.empty()) break;
		cout << "label name : " << label_name.front()<<endl;
		cout << "dir name : " << p1.path().string()<<endl;


		for (auto& p2 : std::experimental::filesystem::directory_iterator(p1.path())) {			
			Mat src = imread(p2.path().string());
			Mat resize_dst;
			Mat bright_dst;
			Mat dark_dst;
			
			stringstream ss1, ss2, ss3;
			
			// Resizing to 24 x 24			
			ss1 << SAVE_PATH << "\\" << label_name.front() 
				<< "\\" << label_name.front()
				<< "_" << cnt << ".jpg";
			cnt++;			

			resize(src, resize_dst, Size(64, 64));
			imwrite(ss1.str(), resize_dst);

			if (DEBUG_FLAG) cout << "1 " << ss1.str() << endl;

			// Brighter
			ss2 << SAVE_PATH << "\\" << label_name.front()
				<< "\\" << label_name.front()
				<< "_" << cnt << ".jpg";
			cnt++;

			modify_brightness(resize_dst, bright_dst, 1.2, 30);
			imwrite(ss2.str(), bright_dst);

			if (DEBUG_FLAG) cout << "2 " << ss2.str() << endl;

			// Darker
			ss3 << SAVE_PATH << "\\" << label_name.front()
				<< "\\" << label_name.front()
				<< "_" << cnt << ".jpg";
			cnt++;

			modify_brightness(resize_dst, dark_dst, 0.9, -30);
			imwrite(ss3.str(), dark_dst);		

			if (DEBUG_FLAG) cout << "3 " << ss3.str() << endl;

			ss3 << SAVE_PATH << "\\" << label_name.front()
				<< "\\" << label_name.front()
				<< "_" << cnt << ".jpg";
			cnt++;

			modify_brightness(resize_dst, dark_dst, 0.9, -30 * 2);
			imwrite(ss3.str(), dark_dst);

			ss3 << SAVE_PATH << "\\" << label_name.front()
				<< "\\" << label_name.front()
				<< "_" << cnt << ".jpg";
			cnt++;

			modify_brightness(resize_dst, dark_dst, 1.3, 30 * 2);
			imwrite(ss3.str(), dark_dst);
		}

		label_name.pop_front();

	}

	return 0;
}

void label_init() {
	label_name.push_back("child_protection");
	label_name.push_back("crosswalk");
	label_name.push_back("no_left");
	label_name.push_back("no_parking");
	label_name.push_back("no_right");
	label_name.push_back("no_straight");
	label_name.push_back("no_uturn");
	label_name.push_back("slow");
	label_name.push_back("speed_bust");
	label_name.push_back("unprotected_left");
	label_name.push_back("uturn");

	return;
}

void modify_brightness(Mat src, Mat& dst, double alpha, int beta) {
	dst = Mat::zeros(src.size(), src.type());

	for (int y = 0; y < src.rows; y++) {
		for (int x = 0; x < src.cols; x++) {
			for (int c = 0; c < 3; c++) {
				dst.at<Vec3b>(y,x)[c] =
					saturate_cast<uchar>( alpha * (src.at<Vec3b>(y, x)[c]) + beta );
			}
		}
	}
}