#include "RoadSignDetector.h"

bool RoadSignDetector::loadCascade(string name) {
	CascadeClassifier cascade;

	cout << string(CASCADE_PATH) + name + ".xml" << endl;

	if (!cascade.load(string(CASCADE_PATH) + name + ".xml"))
	{
		cout << "[Error] Cannot open " << name <<".xml" << endl;

		//return false;
		exit(0);
	}

	_RoadSignClassifier.push_back(cascade);
	cout << "load_cascade success" << endl;
	return true;
}

RoadSignDetector::RoadSignDetector(void)
{
	using namespace placeholders;

	// Load speed number images for template matching
	int i = 0;
	for (auto& p1 : std::experimental::filesystem::directory_iterator(SPEED_NUN_PATH)) {
		_Number[i] = cvLoadImage(p1.path().string().c_str(), 0);
		i++;
	}

	// Initialize Road Sign Names
	_RoadSignNames.push_back("SPEED");
	_RoadSignNames.push_back("CHILD_PROTECT");
	_RoadSignNames.push_back("NO_LEFT");
	_RoadSignNames.push_back("NO_RIGHT");
	_RoadSignNames.push_back("NO_PARKING");
	_RoadSignNames.push_back("CROSSWALK");
	_RoadSignNames.push_back("NO_UTURN");
	_RoadSignNames.push_back("ROTATE");
	_RoadSignNames.push_back("SLOW");
	_RoadSignNames.push_back("SPEED_BUST");
	_RoadSignNames.push_back("UNPROTECTED_LEFT");
	_RoadSignNames.push_back("UTURN");
	_RoadSignNames.push_back("NO_STRAIGHT");

	// Initialize Detect Result
	for (int i = 0; i < ROADSIGN_NUM; i++)
		_DetectResult[i] = -1;

	// Cascade data load
	loadCascade("speed");
	loadCascade("child_protect");
	loadCascade("no_right"); //loadCascade("no_left");
	loadCascade("no_right");
	loadCascade("no_parking_잘안됨");
	loadCascade("child_protect"); //loadCascade("crosswalk");
	loadCascade("no_right"); //loadCascade("no_uturn");
	loadCascade("rotate");
	loadCascade("slow");
	loadCascade("speed_bust");
	loadCascade("unprotected_left");
	loadCascade("uturn");
	loadCascade("no_right"); //loadCascade("no_straight_잘안됨");

	_RoadSignFilter.push_back(std::bind(&RoadSignDetector::_Speed_Filter, this, _1, _2));
	_RoadSignFilter.push_back(std::bind(&RoadSignDetector::_Child_Protect_Filter, this, _1, _2));
	_RoadSignFilter.push_back(std::bind(&RoadSignDetector::_No_Left_Filter, this, _1, _2));
	_RoadSignFilter.push_back(std::bind(&RoadSignDetector::_No_Right_Filter, this, _1, _2));
	_RoadSignFilter.push_back(std::bind(&RoadSignDetector::_No_Parking_Filter, this, _1, _2));
	_RoadSignFilter.push_back(std::bind(&RoadSignDetector::_Crosswalk_Filter, this, _1, _2));
	_RoadSignFilter.push_back(std::bind(&RoadSignDetector::_No_Uturn_Filter, this, _1, _2));
	_RoadSignFilter.push_back(std::bind(&RoadSignDetector::_Rotate_Filter, this, _1, _2));
	_RoadSignFilter.push_back(std::bind(&RoadSignDetector::_Slow_Filter, this, _1, _2));
	_RoadSignFilter.push_back(std::bind(&RoadSignDetector::_Speed_Bust_Filter, this, _1, _2));
	_RoadSignFilter.push_back(std::bind(&RoadSignDetector::_Unprotected_Left_Filter, this, _1, _2));
	_RoadSignFilter.push_back(std::bind(&RoadSignDetector::_Uturn_Filter, this, _1, _2));
	_RoadSignFilter.push_back(std::bind(&RoadSignDetector::_No_Straight_Filter, this, _1, _2));
	
	vector<Rect> temp_vec;
	Mat temp_mat;
	for (int i = 0; i < ROADSIGN_NUM; i++) {
		_FilteredRoadSignLocations.push_back(temp_vec);
		_FilteredCandidates.push_back(temp_mat);
	}

}

RoadSignDetector::~RoadSignDetector(void)
{
}

//main 코드
Mat RoadSignDetector::RoadSignDetect(Mat& src, string filename)
{	
	//src 회전
	if (FLIP_FLAG)
		flip(src, src, -1);

	//White Balance
	autowb(src, src);
	Mat src_gray = Mat(src.size(), 8, 1);
	cv::cvtColor(src, src_gray, CV_BGR2GRAY);
	// Make Candidates
	Mat hist;
	equalizeHist(src_gray, hist);
	for (int i = 0; i < ROADSIGN_NUM; i++) {
		createCandidate(src, hist, i);
	}
	// Filtering
	for (int i = 1; i < ROADSIGN_NUM; i++) {
		Mat src_temp = src.clone();
		Mat gray_temp = src_gray.clone();
		cout << _RoadSignNames[i] << endl;
		_RoadSignFilter[i](src_temp, gray_temp);
		//show_filtered_candidates(src_temp, i);
	}
	// Create txt file
	vector<string> data;
	
	for (int i = 0; i < ROADSIGN_NUM; i++) {
		for (auto it = _FilteredRoadSignLocations[i].begin(); it != _FilteredRoadSignLocations[i].end(); ++it)
			data.push_back( normalize_location(src, i, (*it)) );
	}
	// Create datalist file
	string data_name;
	istringstream ss(filename);
	while (getline(ss, data_name, '\\')) {}
	data_list.push_back("data/img/" + data_name + "\n");

	filename = change_file_extention(filename, string("txt"));
	string txt_path = LOAD_PATH + string("\\") + filename;
	write_file(txt_path, data);
	
	_RoadSignLocations.clear();
	_FilteredRoadSignLocations.clear();
	_NoFilteredCandidates.clear();
	_FilteredCandidates.clear();
	
	cout << "done!" << endl;
	return src;
}

void RoadSignDetector::createCandidate(Mat src, Mat hist, int idx) {
	
	vector<Rect> found;
	Mat temp = src.clone();
	
	_RoadSignClassifier[idx].detectMultiScale(hist, found, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(20, 20), Size(200, 200));
	_RoadSignLocations.push_back(found);

	_NoFilteredCandidates.push_back(temp);

	for (int i = 0; i < found.size(); i++)
	{
		rectangle(_NoFilteredCandidates[idx], cvPoint(found[i].x, found[i].y), cvPoint(found[i].x + found[i].width, found[i].y + found[i].height), CV_RGB(0, 255, 0), 2, 8, 0);
	}
}

void RoadSignDetector::autowb(Mat& src, Mat& dst)
{
	double YSum = 0;
	double CbSum = 0;
	double CrSum = 0;
	double n = 0;

	double R, B, G, Y, Cb, Cr;
	double a11, a12, a21, a22, b1, b2, Ar, Ab;
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			int index = i * src.cols * 3 + j * 3;
			unsigned char R = (unsigned char)src.data[index + 2];
			unsigned char G = (unsigned char)src.data[index + 1];
			unsigned char B = (unsigned char)src.data[index];


			Y = 0.299*R + 0.587*G + 0.114*B;
			Cb = -0.1687*R - 0.3313*G + 0.5*B;
			Cr = 0.5*R - 0.4187*G - 0.0813*B;


			if (Y - fabs(Cb) - fabs(Cr) > 100)
			{
				YSum += Y;
				CbSum += Cb;
				CrSum += Cr;
				n++;
			}
		}
	}

	if (n == 0)
	{
		return;
	}

	YSum /= n;
	CbSum /= n;
	CrSum /= n;

	Y = YSum;
	Cb = CbSum;
	Cr = CrSum;

	a11 = -0.1687*Y - 0.2365*Cr;
	a12 = 0.5*Y + 0.866*Cb;
	a21 = 0.5*Y + 0.701*Cr;
	a22 = -0.0813*Y - 0.1441*Cb;

	b1 = 0.3313*Y - 0.114*Cb - 0.2366*Cr;
	b2 = 0.4187*Y - 0.1441*Cb - 0.299*Cr;


	Ar = (a22*b1 - a12*b2) / (a11*a22 - a12*a21);
	Ab = (a21*b1 - a11*b2) / (a21*a12 - a11*a22);

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			int index = i * src.cols * 3 + j * 3;
			float R = (unsigned char)src.data[index + 2];
			float G = (unsigned char)src.data[index + 1];
			float B = (unsigned char)src.data[index];

			R *= Ar;
			if (R > 255.)
			{
				R = 255.;
			}
			else if (R < 0.)
			{
				R = 0.;
			}

			if (G > 255.)
			{
				G = 255.;
			}
			else if (G < 0.)
			{
				G = 0.;
			}

			B *= Ab;
			if (B > 255.)
			{
				B = 255.;
			}
			else if (B < 0.)
			{
				B = 0.;
			}

			dst.data[i* dst.cols * 3 + j * 3 + 2] = (unsigned char)R;
			dst.data[i* dst.cols * 3 + j * 3 + 1] = (unsigned char)G;
			dst.data[i* dst.cols * 3 + j * 3] = (unsigned char)B;
		}
	}

	return;
}

void RoadSignDetector::showOnce(String name, Mat src) {
	namedWindow(name);
	moveWindow(name, 20, 20);
	imshow(name, src);
	waitKey();
	destroyAllWindows();
}

int RoadSignDetector::_Speed_Filter(Mat src, Mat grey) {
	bool th_Flg = false;
	bool tw_Flg = false;
	bool flg = false;

	bool maxflg = false;
	bool minflg = false;

	int nnum = 0;

	IplImage temp1 = src;
	IplImage temp2 = grey;


	IplImage* _src;
	IplImage* _box = cvCloneImage(&(IplImage)grey);
	int num = _RoadSignLocations[SPEED].size();
	vector<Rect> Temp = _RoadSignLocations[SPEED];
	int speed;
	float per = 60.f;


	for (int k = 0; k < num; k++)
	{
		//표지판 후보군 영역 110*110으로 크게 resize
		IplImage* roi = cvCreateImage(cvSize(Temp[k].width, Temp[k].height), 8, 1);
		IplImage* resize = cvCreateImage(cvSize(110, 110), 8, 1);
		IplImage* binary = cvCreateImage(cvGetSize(resize), 8, 1);

		cvSetZero(binary);

		cvSetImageROI(_box, cvRect(Temp[k].x, Temp[k].y, Temp[k].width, Temp[k].height));
		cvCopy(_box, roi);
		cvResize(roi, resize);

		cvAdaptiveThreshold(resize, binary, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 21, 8);
		cvThreshold(binary, binary, 50, 255, CV_THRESH_BINARY);

		//레이블링
		CBlobLabeling label;
		label.SetParam(binary, 50);
		label.DoLabeling();

		digit = 0;

		for (int i = 0; i < 3; i++)
		{
			resultNum[i][0] = 0;	//위치
			resultNum[i][1] = 0;	//자리수의 숫자
		}

		//레이블링영역 중 실제 숫자영역을 찾기 위한 조건들
		for (int i = 0; i < label.m_nBlobs; i++)
		{
			//			cvRectangle(color, cvPoint(label.m_recBlobs[i].x, label.m_recBlobs[i].y), cvPoint(label.m_recBlobs[i].x + label.m_recBlobs[i].width, label.m_recBlobs[i].y + label.m_recBlobs[i].height), CV_RGB(0, 255, 0), 1, 8, 0);
			if (label.m_recBlobs[i].x > 10 && label.m_recBlobs[i].y > 10 && ((label.m_recBlobs[i].x + label.m_recBlobs[i].width) < resize->width - 10) && ((label.m_recBlobs[i].y + label.m_recBlobs[i].height) < resize->height - 10))
			{
				if ((label.m_recBlobs[i].width >= 8) && (label.m_recBlobs[i].width < 40) && (label.m_recBlobs[i].height >= 20) && (label.m_recBlobs[i].height < 80))
				{
					if ((label.m_recBlobs[i].width <= label.m_recBlobs[i].height) && (label.m_recBlobs[i].width * 3 > label.m_recBlobs[i].height))
					{
						if ((label.m_recBlobs[i].height - label.m_recBlobs[i + 1].height) < 8 || (label.m_recBlobs[i].height - label.m_recBlobs[i + 1].height) > -8)
						{
							if (label.m_recBlobs[i].x < label.m_recBlobs[i + 1].x)
							{
								if (label.m_recBlobs[i].x + label.m_recBlobs[i].width < label.m_recBlobs[i + 1].x)
								{
									flg = true;
								}
							}

							if (label.m_recBlobs[i + 1].x < label.m_recBlobs[i].x)
							{
								if (label.m_recBlobs[i + 1].x + label.m_recBlobs[i + 1].width < label.m_recBlobs[i].x)
								{
									flg = true;
								}
							}

							if (flg == true)
							{
								float max = 0;
								float tmp[10];
								int matchingNum = 0;

								//templatematching (더 높은 성능을 얻기 위해서 직접 구현)
								for (int n = 0; n < 10; n++)
								{
									IplImage* num_temp = cvCreateImage(cvSize(label.m_recBlobs[i].width, label.m_recBlobs[i].height), 8, 1);
									IplImage* roi_temp = cvCreateImage(cvSize(label.m_recBlobs[i].width, label.m_recBlobs[i].height), 8, 1);

									cvResize(_Number[n], num_temp);
									cvSetImageROI(binary, label.m_recBlobs[i]);
									cvCopy(binary, roi_temp);

									cvThreshold(num_temp, num_temp, 50, 255, CV_THRESH_BINARY_INV);

									int count = 0;

									for (int i = 0; i < roi_temp->height; i++)
									{
										for (int j = 0; j < roi_temp->width; j++)
										{
											if (roi_temp->imageData[i*roi_temp->widthStep + j] == num_temp->imageData[i*roi_temp->widthStep + j])
											{
												count++;
											}
										}
									}

									float matchingPercent = 100 * (float)count / (float)(roi_temp->width * roi_temp->height);
									tmp[n] = matchingPercent;

									if (matchingPercent > max)
									{
										max = matchingPercent;
										matchingNum = n;
									}

									cvResetImageROI(binary);
									cvReleaseImage(&num_temp);
									cvReleaseImage(&roi_temp);
								}

								if (max > per && digit < 3)
								{
									resultNum[digit][0] = label.m_recBlobs[i].x;
									resultNum[digit][1] = matchingNum;
									digit++;
								}

								BubbleSorting(resultNum);	//버블정렬을 통해 십의 자리와 일의 자리의 숫자들 순서 안 바뀌게 함

															//여기서 부턴 속도값 조건
								if (digit == 3)
								{
									if (resultNum[0][1] == 1 && resultNum[2][1] == 0)
									{
										speed = 100 + resultNum[1][1] * 10;

										th_Flg = true;
									}
								}
								else if (digit == 2)	//두자리일 때
								{
									if (resultNum[0][1] == 1)
									{
										continue;
									}
									if (resultNum[1][1] == 0)
									{
										if (resultNum[0][1] == 0)
										{
											float max_tmp = 0;
											int matchingNum_tmp = 0;

											for (int q = 0; q < 10; q++)
											{
												if (tmp[q] == max)	continue;

												if (tmp[q] > max_tmp)
												{
													max_tmp = tmp[q];
													matchingNum_tmp = q;
												}
											}
											resultNum[0][1] = matchingNum_tmp;
										}

										speed = resultNum[0][1] * 10;

										tw_Flg = true;
									}
								}
								else
								{
									continue;
								}

							}

							if (th_Flg == true || tw_Flg == true)
							{
								_FilteredRoadSignLocations[SPEED].push_back(Temp[k]);
								/*candi[nnum].left = Temp[k].x;
								candi[nnum].top = Temp[k].y;
								candi[nnum].right = Temp[k].x + Temp[k].width;
								candi[nnum].bottom = Temp[k].y + Temp[k].height;*/
								nnum++;
							}
						}
					}
				}
			}
		}

		cvReleaseImage(&roi);
		cvReleaseImage(&binary);
		cvReleaseImage(&resize);
	}

	return speed;
}

int RoadSignDetector::_Child_Protect_Filter(Mat src, Mat grey) {
	
	vector<Rect> found = _RoadSignLocations[CHILD_PROTECT];
	double filter_threshold = 0.25;


	_FilteredRoadSignLocations[CHILD_PROTECT].clear();

	for (int k = 0; k < found.size(); k++)
	{
		Mat filter_src = src.clone();

		Mat mask;
		mask = hsv_filter(filter_src, found[k], 102, 130, 70, 255, 40, 255);

		double sum = 0;
		for (int i = 0; i < mask.rows * mask.cols; i++) {
			if (mask.data[i] > 0)
				sum++;
		}

		double color_rate = sum / (mask.rows * mask.cols);

		if (color_rate > filter_threshold) _FilteredRoadSignLocations[CHILD_PROTECT].push_back(found[k]);
	}

	return -1;
}

int RoadSignDetector::_No_Left_Filter(Mat src, Mat grey) {
	vector<Rect> found = _RoadSignLocations[NO_LEFT];
	double filter_threshold = NO_THRESHOLD;
	

	_FilteredRoadSignLocations[NO_LEFT].clear();

	for (int k = 0; k < found.size(); k++)
	{
		Mat filter_src = src.clone();

		Mat mask, mask1, mask2;
		
		mask1 = hsv_filter(filter_src, found[k], 150, 180, 130, 255, 0, 255);
		mask2 = hsv_filter(filter_src, found[k], 0, 10, 80, 255, 0, 255);
		mask = (mask1 ^ mask2);

		double sum = 0;
		for (int i = 0; i < mask.rows * mask.cols; i++) {
			if (mask.data[i] > 0)
				sum++;
		}

		double color_rate = sum / (mask.rows * mask.cols);

		if (color_rate > filter_threshold) _FilteredRoadSignLocations[NO_LEFT].push_back(found[k]);
	}
	
	return -1;
}

int RoadSignDetector::_No_Right_Filter(Mat src, Mat grey) {
	vector<Rect> found = _RoadSignLocations[NO_RIGHT];
	double filter_threshold = NO_THRESHOLD;


	_FilteredRoadSignLocations[NO_RIGHT].clear();

	for (int k = 0; k < found.size(); k++)
	{
		Mat filter_src = src.clone();
		Mat mask, mask1, mask2;

		mask1 = hsv_filter(filter_src, found[k], 150, 180, 130, 255, 0, 255);
		mask2 = hsv_filter(filter_src, found[k], 0, 10, 80, 255, 0, 255);
		mask = (mask1 ^ mask2);

		double sum = 0;
		for (int i = 0; i < mask.rows * mask.cols; i++) {
			if (mask.data[i] > 0)
				sum++;
		}

		double color_rate = sum / (mask.rows * mask.cols);

		if (color_rate > filter_threshold) _FilteredRoadSignLocations[NO_RIGHT].push_back(found[k]);
	}

	return -1;
}

int RoadSignDetector::_No_Parking_Filter(Mat src, Mat grey) {

	vector<Rect> found = _RoadSignLocations[NO_PARKING];
	double blue_threshold = 0.23;
	double red_threshold = 0.05;


	_FilteredRoadSignLocations[NO_PARKING].clear();

	for (int k = 0; k < found.size(); k++)
	{
		Mat filter_src = src.clone();

		Mat blue_mask, red_mask1, red_mask2, red_mask;
		blue_mask = hsv_filter(filter_src, found[k], 102, 130, 40, 255, 30, 255);
		
		red_mask1 = hsv_filter(filter_src, found[k], 150, 180, 80, 255, 0, 255);
		red_mask2 = hsv_filter(filter_src, found[k], 0, 10, 80, 255, 0, 255);
		red_mask = red_mask1 ^ red_mask2;

		double blue_sum = 0;
		double red_sum = 0;
		for (int i = 0; i < blue_mask.rows * blue_mask.cols; i++) {
			if (blue_mask.data[i] > 0)
				blue_sum++;
			if (red_mask.data[i] > 0)
				red_sum++;
		}

		double blue_rate = blue_sum / (blue_mask.rows * blue_mask.cols);
		double red_rate = red_sum / (red_mask.rows * red_mask.cols);

		if ( (blue_rate > blue_threshold) && (red_rate > red_threshold) )
			_FilteredRoadSignLocations[NO_PARKING].push_back(found[k]);
	}

	return -1;
}

int RoadSignDetector::_Crosswalk_Filter(Mat src, Mat grey) {

	vector<Rect> found = _RoadSignLocations[CROSSWALK];
	double filter_threshold = 0.25;


	_FilteredRoadSignLocations[CROSSWALK].clear();

	for (int k = 0; k < found.size(); k++)
	{
		Mat filter_src = src.clone();

		Mat mask;
		mask = hsv_filter(filter_src, found[k], 90, 130, 50, 255, 70, 255);

		double sum = 0;
		for (int i = 0; i < mask.rows * mask.cols; i++) {
			if (mask.data[i] > 0)
				sum++;
		}

		double color_rate = sum / (mask.rows * mask.cols);

		if (color_rate > filter_threshold) _FilteredRoadSignLocations[CROSSWALK].push_back(found[k]);
	}

	return -1;
}

int RoadSignDetector::_No_Uturn_Filter(Mat src, Mat grey) {

	vector<Rect> found = _RoadSignLocations[NO_UTURN];
	double red_threshold = 0.05;
	double black_threshold = 0.05;


	_FilteredRoadSignLocations[NO_UTURN].clear();

	for (int k = 0; k < found.size(); k++)
	{
		Mat filter_src = src.clone();

		Mat red_mask, red_mask1, red_mask2, black_mask;
		
		red_mask1 = hsv_filter(filter_src, found[k], 150, 180, 130, 255, 0, 255);
		red_mask2 = hsv_filter(filter_src, found[k], 0, 12, 130, 255, 0, 255);
		black_mask = hsv_filter(filter_src, found[k], 0, 180, 0, 255, 0, 100);

		red_mask = (red_mask1 ^ red_mask2);

		double red_sum = 0, black_sum=0;
		for (int i = 0; i < red_mask.rows * red_mask.cols; i++) {
			if (red_mask.data[i] > 0)
				red_sum++;
			if (black_mask.data[i] > 0)
				black_sum++;
		}

		double red_rate = red_sum / (red_mask.rows * red_mask.cols);
		double black_rate = black_sum / (black_mask.rows * black_mask.cols);
	
		if ( (red_rate > red_threshold) && (black_rate > black_threshold) )
			_FilteredRoadSignLocations[NO_UTURN].push_back(found[k]);
	}

	return -1;
}

int RoadSignDetector::_Rotate_Filter(Mat src, Mat grey) {

	vector<Rect> found = _RoadSignLocations[ROTATE];
	double filter_threshold = 0.25;

	_FilteredRoadSignLocations[ROTATE].clear();

	for (int k = 0; k < found.size(); k++)
	{
		Mat filter_src = src.clone();
	
		Mat mask;
		mask = hsv_filter(filter_src, found[k], 90, 130, 100, 255, 100, 255);

		double sum = 0;
		for (int i = 0; i < mask.rows * mask.cols; i++) {
			if (mask.data[i] > 0)
				sum++;
		}

		double color_rate = sum / (mask.rows * mask.cols);

		if (color_rate > filter_threshold) _FilteredRoadSignLocations[ROTATE].push_back(found[k]);
	}

	return -1;
}

int RoadSignDetector::_Slow_Filter(Mat src, Mat grey) {
	vector<Rect> found = _RoadSignLocations[SLOW];
	double red_threshold = 0.15;
	double black_threshold = 0.05;


	_FilteredRoadSignLocations[SLOW].clear();

	for (int k = 0; k < found.size(); k++)
	{
		Mat filter_src = src.clone();
		
		Mat red_mask, red_mask1, red_mask2, black_mask;

		red_mask1 = hsv_filter(filter_src, found[k], 150, 180, 130, 255, 0, 255);
		red_mask2 = hsv_filter(filter_src, found[k], 0, 10, 80, 255, 0, 255);
		black_mask = hsv_filter(filter_src, found[k], 0, 180, 0, 255, 0, 100);
		red_mask = (red_mask1 ^ red_mask2);

		double red_sum = 0, black_sum = 0;
		for (int i = 0; i < red_mask.rows * red_mask.cols; i++) {
			if (red_mask.data[i] > 0)
				red_sum++;
			if (black_mask.data[i] > 0)
				black_sum++;
		}

		double red_rate = red_sum / (red_mask.rows * red_mask.cols);
		double black_rate = black_sum / (black_mask.rows * black_mask.cols);

		if ( (red_rate > red_threshold) && (black_rate > black_threshold) )
			_FilteredRoadSignLocations[SLOW].push_back(found[k]);
	}

	return -1;
}

int RoadSignDetector::_Speed_Bust_Filter(Mat src, Mat grey) {

	vector<Rect> found = _RoadSignLocations[SPEED_BUST];
	double yellow_threshold = 0.2;
	double black_threshold = 0.12;

	_FilteredRoadSignLocations[SPEED_BUST].clear();

	for (int k = 0; k < found.size(); k++)
	{
		Mat filter_src = src.clone();

		Mat yellow_mask, black_mask;

		yellow_mask = hsv_filter(filter_src, found[k], 17, 26, 35, 255, 110, 255);
		black_mask = hsv_filter(filter_src, found[k], 0, 180, 0, 255, 0, 100);

		double black_sum = 0, yellow_sum = 0;
		for (int i = 0; i < yellow_mask.rows * yellow_mask.cols; i++) {
			if (yellow_mask.data[i] > 0)
				yellow_sum++;
			if (black_mask.data[i] > 0)
				black_sum++;
		}

		double yellow_rate = yellow_sum / (yellow_mask.rows * yellow_mask.cols);
		double black_rate = black_sum / (black_mask.rows * black_mask.cols);

		if ( (yellow_rate > yellow_threshold) && (black_rate > black_threshold) )
			_FilteredRoadSignLocations[SPEED_BUST].push_back(found[k]);
	}

	return -1;
}

int RoadSignDetector::_Unprotected_Left_Filter(Mat src, Mat grey) {

	vector<Rect> found = _RoadSignLocations[UNPROTECTED_LEFT];
	double filter_threshold = 0.25;


	_FilteredRoadSignLocations[UNPROTECTED_LEFT].clear();

	for (int k = 0; k < found.size(); k++)
	{
		Mat filter_src = src.clone();

		Mat mask;
		mask = hsv_filter(filter_src, found[k], 90, 130, 60, 255, 100, 255);

		double sum = 0;
		for (int i = 0; i < mask.rows * mask.cols; i++) {
			if (mask.data[i] > 0)
				sum++;
		}

		double color_rate = sum / (mask.rows * mask.cols);

		if (color_rate > filter_threshold) _FilteredRoadSignLocations[UNPROTECTED_LEFT].push_back(found[k]);
	}

	return -1;
}

int RoadSignDetector::_Uturn_Filter(Mat src, Mat grey) {

	vector<Rect> found = _RoadSignLocations[UTURN];
	double filter_threshold = 0.25;


	_FilteredRoadSignLocations[UTURN].clear();

	for (int k = 0; k < found.size(); k++)
	{
		Mat filter_src = src.clone();

		Mat mask;
		mask = hsv_filter(filter_src, found[k], 90, 130, 60, 255, 100, 255);

		double sum = 0;
		for (int i = 0; i < mask.rows * mask.cols; i++) {
			if (mask.data[i] > 0)
				sum++;
		}

		double color_rate = sum / (mask.rows * mask.cols);

		if (color_rate > filter_threshold) _FilteredRoadSignLocations[UTURN].push_back(found[k]);
	}

	return -1;
}

int RoadSignDetector::_No_Straight_Filter(Mat src, Mat grey){
	vector<Rect> found = _RoadSignLocations[NO_STRAIGHT];
	double filter_threshold = NO_THRESHOLD;


	_FilteredRoadSignLocations[NO_STRAIGHT].clear();

	for (int k = 0; k < found.size(); k++)
	{
		Mat filter_src = src.clone();

		Mat mask, mask1, mask2;

		mask1 = hsv_filter(filter_src, found[k], 150, 180, 130, 255, 0, 255);
		mask2 = hsv_filter(filter_src, found[k], 0, 10, 80, 255, 0, 255);
		mask = (mask1 ^ mask2);

		double sum = 0;
		for (int i = 0; i < mask.rows * mask.cols; i++) {
			if (mask.data[i] > 0)
				sum++;
		}

		double color_rate = sum / (mask.rows * mask.cols);

		if (color_rate > filter_threshold) _FilteredRoadSignLocations[NO_STRAIGHT].push_back(found[k]);
	}

	return -1;
}

Mat RoadSignDetector::hsv_filter(Mat src, Rect location, double minHue, double maxHue, double minSat, double maxSat, double minVal, double maxVal) {
	//표지판 후보군 영역 110*110으로 크게 resize
	Mat _roi = src(location);
	Mat _roi_resize;

	cv::resize(_roi, _roi_resize, Size(500, 500));

	Mat hsv;
	cvtColor(_roi_resize, hsv, CV_BGR2HSV);
	vector<Mat> channels;
	split(hsv, channels);

	Mat min_mask1, min_mask2, min_mask3;
	Mat hue_mask1, hue_mask2, hue_mask3;
	Mat val_mask1, val_mask2, val_mask3;

	threshold(channels[0], min_mask1, maxHue, 255, THRESH_BINARY_INV);
	threshold(channels[0], min_mask2, minHue, 255, THRESH_BINARY);
	Mat hueMask = min_mask1 & min_mask2;

	threshold(channels[1], hue_mask1, maxSat, 255, THRESH_BINARY_INV);
	threshold(channels[1], hue_mask2, minSat, 255, THRESH_BINARY);
	Mat satMask = hue_mask1 & hue_mask2;

	threshold(channels[2], val_mask1, maxVal, 255, THRESH_BINARY_INV);
	threshold(channels[2], val_mask2, minVal, 255, THRESH_BINARY);
	Mat valMask = val_mask1 & val_mask2;

	Mat mask = hueMask & satMask & valMask;
	Mat filtered(_roi_resize.size(), CV_8UC3, Scalar(0, 0, 0));

	return mask;
		
}

Mat RoadSignDetector::RGB_filter(Mat src, Rect location, double minR, double maxR, double minG, double maxG, double minB, double maxB) {
	//표지판 후보군 영역 110*110으로 크게 resize
	Mat _roi = src(location);
	Mat _roi_resize;

	resize(_roi, _roi_resize, Size(500, 500));

	Mat hsv;
	cvtColor(_roi_resize, hsv, CV_BGR2HSV);
	vector<Mat> channels;
	split(hsv, channels);

	Mat mask1;
	Mat mask2;

	threshold(channels[0], mask1, maxR, 255, THRESH_BINARY_INV);
	threshold(channels[0], mask2, minR, 255, THRESH_BINARY);
	Mat RMask = mask1 & mask2;

	threshold(channels[1], mask1, maxG, 255, THRESH_BINARY_INV);
	threshold(channels[1], mask2, minG, 255, THRESH_BINARY);
	Mat GMask = mask1 & mask2;

	threshold(channels[2], mask1, maxB, 255, THRESH_BINARY_INV);
	threshold(channels[2], mask2, minB, 255, THRESH_BINARY);
	Mat BMask = mask1 & mask2;

	Mat mask = RMask & GMask & BMask;
	Mat filtered(_roi_resize.size(), CV_8UC3, Scalar(0, 0, 0));

	return mask;
}

void RoadSignDetector::show_filtered_candidates(Mat src, int sign_type) {
	vector<Rect> filtered_found = _FilteredRoadSignLocations[sign_type];

	src.copyTo(_FilteredCandidates[sign_type]);

	for (int i = 0; i < filtered_found.size(); i++)
	{
		rectangle(_FilteredCandidates[sign_type],
			cvPoint(filtered_found[i].x, filtered_found[i].y), cvPoint(filtered_found[i].x
				+ filtered_found[i].width, filtered_found[i].y + filtered_found[i].height), CV_RGB(200, 200, 0), 2, 8, 0);
	}
	
	Mat show;
	_FilteredCandidates[sign_type].copyTo(show);

	putText(show, _RoadSignNames[sign_type], Point(10, 50), 2, 2, Scalar(0, 0, 255));

	imshow("no filtered", _NoFilteredCandidates[sign_type]);
	namedWindow("filtered");
	imshow("filtered", show);
	waitKey();
}

vector<string> RoadSignDetector::getRoadSignNames() {
	return _RoadSignNames;
}

vector<CascadeClassifier> RoadSignDetector::getRoadSignClassifier() {
	return _RoadSignClassifier;
}

vector< vector<Rect> > RoadSignDetector::getRoadSignLocations() {
	return _RoadSignLocations;
}

vector< vector<Rect> > RoadSignDetector::getFilteredRoadSignLocations() {
	return _FilteredRoadSignLocations;
}

vector< Mat > RoadSignDetector::getNoFilteredCandidates() {
	return _NoFilteredCandidates;
}

vector< Mat > RoadSignDetector::getFilteredCandidates() {
	return _FilteredCandidates;
}

void RoadSignDetector::BubbleSorting(int num[3][2])
{
	int temp = 0;
	int temp2 = 0;

	for (int i = 0; i < digit; i++)
	{
		for (int j = 0; j < digit - i - 1; j++)
		{
			if (num[j][0] > num[j + 1][0])
			{
				temp = num[j][0];
				temp2 = num[j][1];
				num[j][0] = num[j + 1][0];
				num[j][1] = num[j + 1][1];
				num[j + 1][0] = temp;
				num[j + 1][1] = temp2;
			}
		}
	}
}

string RoadSignDetector::normalize_location(Mat src, int label, Rect location) {	
	string result;
	double x_center, y_center, width, height;

	width = location.width / (double)(src.cols);
	height = location.height / (double)(src.rows);
	x_center = (location.x / (double)(src.cols)) + 0.5 * width;
	y_center = (location.y / (double)(src.rows)) + 0.5 * height;

	result = to_string(label) + " " + to_string(x_center) + " " + to_string(y_center) + " ";
	result += to_string(width) + " " + to_string(height) + "\n";
	return result;
}

string change_file_extention(string filename, string extention) {
	istringstream ss(filename);
	while (getline(ss, filename, '\\')) {}
	ss = istringstream(filename);
	getline(ss, filename, '.');
	filename = filename + "." + extention;
	return filename;
}

void read_file(string path, vector<string>& data) {
	ifstream in_stream(path);
	string input;

	while (!in_stream.eof()) {
		getline(in_stream, input);
		data.push_back(input);
	}
	in_stream.close();
}

void write_file(string path, vector<string> data) {
	ofstream out_stream(path);
	for (int idx = 0; idx < data.size(); idx++) {		
		out_stream << data[idx];
	}
	out_stream.close();
}

