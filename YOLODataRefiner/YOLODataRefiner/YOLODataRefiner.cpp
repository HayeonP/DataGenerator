#include "YOLODataRefiner.h"
#include <fstream>
#include <sstream>

YOLODataRefiner::YOLODataRefiner(string _screen_name, string path, int _button_state, int _label_state)
	: screen_name(_screen_name), button_state(_button_state), label_state(_label_state), data_idx(START_IDX), data_num(0)
{
	
	// File list load
	ifstream in(path);
	string input_s;

	current_idx = 0;

	while (!in.eof()) {
		getline(in, input_s);

		ReplaceAll(input_s, "/", "\\\\");
		img_list.push_back(input_s);
		ReplaceAll(input_s, ".jpg", ".txt");
		txt_list.push_back(input_s);

		data_num++;
	}
	cout << "1" << endl;
	in.close();
	data_num -= 2;

	// Setup
	namedWindow(_screen_name);
	setMouseCallback(screen_name, CallBackFunc, this);
	move_flag = false;

	screen_width = 1650, screen_height = 1000, label_size = 70;
	cout << "2" << endl;
	// BUTTONS
	button_height = 100, button_width = 200;

	Rect move = Rect(Point(screen_width - button_width, 0), Point(screen_width, button_height));
	Rect erase = Rect(Point(screen_width - button_width, button_height + 10), Point(screen_width, button_height * 2 + 10));
	Rect create = Rect(Point(screen_width - button_width, button_height * 2 + 10 * 2), Point(screen_width, button_height * 3 + 10 * 2));
	Rect quit = Rect(Point(screen_width - button_width, button_height * 3 + 10 * 3), Point(screen_width, button_height * 4 + 10 * 3));
	button_locations.push_back(move);
	button_locations.push_back(erase);
	button_locations.push_back(create);
	button_locations.push_back(quit);
	// LABELS
	int button_lastY = button_height * 4 + 10 * 3;

	for (auto& p : std::experimental::filesystem::directory_iterator(LABEL_PATH)) {
		Mat label = imread(p.path().string());
		resize(label, label, Size(label_size, label_size));
		label_images.push_back(label);
	}

	for (int i = 0; i < LABEL_NUM; i++) {
		Rect label_location;
		int label_row = i / 4;
		int label_col = i % 4;
		label_location = Rect(
			screen_width - (4 - label_col) * label_size, button_lastY + label_row * label_size + 10,
			label_size, label_size);
		label_locations.push_back(label_location);
	}
	cout << "3" << endl;
	// Label & Button names
	label_names.push_back("Speed other");
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
	label_names.push_back("ALL");

	button_names.push_back("Move");
	button_names.push_back("Erase");
	button_names.push_back("Create");
	button_names.push_back("Quit");

	colors.push_back(Scalar(0, 0, 255));
	colors.push_back(Scalar(0, 255, 0));
	colors.push_back(Scalar(255, 0, 0));
	colors.push_back(Scalar(0, 120, 208));
	colors.push_back(Scalar(1, 207, 130));
	colors.push_back(Scalar(109, 208, 0));
	colors.push_back(Scalar(207, 156, 1));
	colors.push_back(Scalar(208, 0, 120));
	colors.push_back(Scalar(225, 27, 254));
	colors.push_back(Scalar(103, 177, 172));
	colors.push_back(Scalar(87, 103, 102));
	colors.push_back(Scalar(179, 102, 154));
	colors.push_back(Scalar(29, 60, 109));
	colors.push_back(Scalar(80, 10, 65));
	colors.push_back(Scalar(225, 150, 9));
	colors.push_back(Scalar(200, 92, 109));
	colors.push_back(Scalar(51, 151, 251));
	colors.push_back(Scalar(251, 151, 51));
	colors.push_back(Scalar(151, 251, 109));
	colors.push_back(Scalar(68, 68, 68));
	colors.push_back(Scalar(99, 99, 200));
	colors.push_back(Scalar(0, 0, 0));
	cout << "4" << endl;
}

void YOLODataRefiner::CallBackFunc(int event, int x, int y, int flags, void* userdata) {
	YOLODataRefiner* refiner = reinterpret_cast<YOLODataRefiner *>(userdata);
	if (event == EVENT_LBUTTONDOWN) {
		// MOVE BUTTON
		if ((x > refiner->button_locations[MOVE].x) &&
			(x < refiner->button_locations[MOVE].x + refiner->button_width) &&
			(y > refiner->button_locations[MOVE].y) &&
			(y < refiner->button_locations[MOVE].y + refiner->button_height)) {

			refiner->button_state = MOVE;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		// ERASE BUTTON
		else if ((x > refiner->button_locations[ERASE].x) &&
			(x < refiner->button_locations[ERASE].x + refiner->button_width) &&
			(y > refiner->button_locations[ERASE].y) &&
			(y < refiner->button_locations[ERASE].y + refiner->button_height)) {

			refiner->button_state = ERASE;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		// CREATE BUTTON
		else if ((x > refiner->button_locations[CREATE].x) &&
			(x < refiner->button_locations[CREATE].x + refiner->button_width) &&
			(y > refiner->button_locations[CREATE].y) &&
			(y < refiner->button_locations[CREATE].y + refiner->button_height)) {

			refiner->button_state = CREATE;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}

		// QUIT BUTTON
		else if ((x > refiner->button_locations[QUIT].x) &&
			(x < refiner->button_locations[QUIT].x + refiner->button_width) &&
			(y > refiner->button_locations[QUIT].y) &&
			(y < refiner->button_locations[QUIT].y + refiner->button_height)) {

			refiner->button_state = QUIT;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			exit(0);
		}

		// LABELS
		else if ((x > refiner->label_locations[CHILD_PROTECT].x) &&
			(x < refiner->label_locations[CHILD_PROTECT].x + refiner->label_size) &&
			(y > refiner->label_locations[CHILD_PROTECT].y) &&
			(y < refiner->label_locations[CHILD_PROTECT].y + refiner->label_size)) {

			refiner->label_state = CHILD_PROTECT;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->label_locations[CROSSWALK].x) &&
			(x < refiner->label_locations[CROSSWALK].x + refiner->label_size) &&
			(y > refiner->label_locations[CROSSWALK].y) &&
			(y < refiner->label_locations[CROSSWALK].y + refiner->label_size)) {

			refiner->label_state = CROSSWALK;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->label_locations[NO_LEFT].x) &&
			(x < refiner->label_locations[NO_LEFT].x + refiner->label_size) &&
			(y > refiner->label_locations[NO_LEFT].y) &&
			(y < refiner->label_locations[NO_LEFT].y + refiner->label_size)) {

			refiner->label_state = NO_LEFT;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->label_locations[NO_PARKING].x) &&
			(x < refiner->label_locations[NO_PARKING].x + refiner->label_size) &&
			(y > refiner->label_locations[NO_PARKING].y) &&
			(y < refiner->label_locations[NO_PARKING].y + refiner->label_size)) {

			refiner->label_state = NO_PARKING;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->label_locations[NO_RIGHT].x) &&
			(x < refiner->label_locations[NO_RIGHT].x + refiner->label_size) &&
			(y > refiner->label_locations[NO_RIGHT].y) &&
			(y < refiner->label_locations[NO_RIGHT].y + refiner->label_size)) {

			refiner->label_state = NO_RIGHT;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->label_locations[NO_STRAIGHT].x) &&
			(x < refiner->label_locations[NO_STRAIGHT].x + refiner->label_size) &&
			(y > refiner->label_locations[NO_STRAIGHT].y) &&
			(y < refiner->label_locations[NO_STRAIGHT].y + refiner->label_size)) {

			refiner->label_state = NO_STRAIGHT;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->label_locations[NO_UTURN].x) &&
			(x < refiner->label_locations[NO_UTURN].x + refiner->label_size) &&
			(y > refiner->label_locations[NO_UTURN].y) &&
			(y < refiner->label_locations[NO_UTURN].y + refiner->label_size)) {

			refiner->label_state = NO_UTURN;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->label_locations[ROTATE].x) &&
			(x < refiner->label_locations[ROTATE].x + refiner->label_size) &&
			(y > refiner->label_locations[ROTATE].y) &&
			(y < refiner->label_locations[ROTATE].y + refiner->label_size)) {

			refiner->label_state = ROTATE;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->label_locations[SLOW].x) &&
			(x < refiner->label_locations[SLOW].x + refiner->label_size) &&
			(y > refiner->label_locations[SLOW].y) &&
			(y < refiner->label_locations[SLOW].y + refiner->label_size)) {

			refiner->label_state = SLOW;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->label_locations[SPEED_BUST].x) &&
			(x < refiner->label_locations[SPEED_BUST].x + refiner->label_size) &&
			(y > refiner->label_locations[SPEED_BUST].y) &&
			(y < refiner->label_locations[SPEED_BUST].y + refiner->label_size)) {

			refiner->label_state = SPEED_BUST;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->label_locations[UNPROTECTED_LEFT].x) &&
			(x < refiner->label_locations[UNPROTECTED_LEFT].x + refiner->label_size) &&
			(y > refiner->label_locations[UNPROTECTED_LEFT].y) &&
			(y < refiner->label_locations[UNPROTECTED_LEFT].y + refiner->label_size)) {

			refiner->label_state = UNPROTECTED_LEFT;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->label_locations[UTURN].x) &&
			(x < refiner->label_locations[UTURN].x + refiner->label_size) &&
			(y > refiner->label_locations[UTURN].y) &&
			(y < refiner->label_locations[UTURN].y + refiner->label_size)) {

			refiner->label_state = UTURN;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->label_locations[SPEED_OTHER].x) &&
			(x < refiner->label_locations[SPEED_OTHER].x + refiner->label_size) &&
			(y > refiner->label_locations[SPEED_OTHER].y) &&
			(y < refiner->label_locations[SPEED_OTHER].y + refiner->label_size)) {

			refiner->label_state = SPEED_OTHER;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->label_locations[SPEED_20].x) &&
		(x < refiner->label_locations[SPEED_20].x + refiner->label_size) &&
		(y > refiner->label_locations[SPEED_20].y) &&
		(y < refiner->label_locations[SPEED_20].y + refiner->label_size)) {

		refiner->label_state = SPEED_20;
		refiner->select = Candidate();
		refiner->move_flag = false;
		refiner->create_flag = false;
		refiner->create_1 = Point();
		refiner->create_2 = Point();

		return;
		}
		else if ((x > refiner->label_locations[SPEED_30].x) &&
		(x < refiner->label_locations[SPEED_30].x + refiner->label_size) &&
		(y > refiner->label_locations[SPEED_30].y) &&
		(y < refiner->label_locations[SPEED_30].y + refiner->label_size)) {

		refiner->label_state = SPEED_30;
		refiner->select = Candidate();
		refiner->move_flag = false;
		refiner->create_flag = false;
		refiner->create_1 = Point();
		refiner->create_2 = Point();

		return;
		}
		else if ((x > refiner->label_locations[SPEED_40].x) &&
		(x < refiner->label_locations[SPEED_40].x + refiner->label_size) &&
		(y > refiner->label_locations[SPEED_40].y) &&
		(y < refiner->label_locations[SPEED_40].y + refiner->label_size)) {

		refiner->label_state = SPEED_40;
		refiner->select = Candidate();
		refiner->move_flag = false;
		refiner->create_flag = false;
		refiner->create_1 = Point();
		refiner->create_2 = Point();

		return;
		}
		else if ((x > refiner->label_locations[SPEED_50].x) &&
		(x < refiner->label_locations[SPEED_50].x + refiner->label_size) &&
		(y > refiner->label_locations[SPEED_50].y) &&
		(y < refiner->label_locations[SPEED_50].y + refiner->label_size)) {

		refiner->label_state = SPEED_50;
		refiner->select = Candidate();
		refiner->move_flag = false;
		refiner->create_flag = false;
		refiner->create_1 = Point();
		refiner->create_2 = Point();

		return;
		}
		else if ((x > refiner->label_locations[SPEED_60].x) &&
		(x < refiner->label_locations[SPEED_60].x + refiner->label_size) &&
		(y > refiner->label_locations[SPEED_60].y) &&
		(y < refiner->label_locations[SPEED_60].y + refiner->label_size)) {

		refiner->label_state = SPEED_60;
		refiner->select = Candidate();
		refiner->move_flag = false;
		refiner->create_flag = false;
		refiner->create_1 = Point();
		refiner->create_2 = Point();

		return;
		}
		else if ((x > refiner->label_locations[SPEED_70].x) &&
		(x < refiner->label_locations[SPEED_70].x + refiner->label_size) &&
		(y > refiner->label_locations[SPEED_70].y) &&
		(y < refiner->label_locations[SPEED_70].y + refiner->label_size)) {

		refiner->label_state = SPEED_70;
		refiner->select = Candidate();
		refiner->move_flag = false;
		refiner->create_flag = false;
		refiner->create_1 = Point();
		refiner->create_2 = Point();

		return;
		}
		else if ((x > refiner->label_locations[SPEED_70].x) &&
		(x < refiner->label_locations[SPEED_70].x + refiner->label_size) &&
		(y > refiner->label_locations[SPEED_70].y) &&
		(y < refiner->label_locations[SPEED_70].y + refiner->label_size)) {

		refiner->label_state = SPEED_70;
		refiner->select = Candidate();
		refiner->move_flag = false;
		refiner->create_flag = false;
		refiner->create_1 = Point();
		refiner->create_2 = Point();

		return;
		}
		else if ((x > refiner->label_locations[SPEED_80].x) &&
		(x < refiner->label_locations[SPEED_80].x + refiner->label_size) &&
		(y > refiner->label_locations[SPEED_80].y) &&
		(y < refiner->label_locations[SPEED_80].y + refiner->label_size)) {

		refiner->label_state = SPEED_80;
		refiner->select = Candidate();
		refiner->move_flag = false;
		refiner->create_flag = false;
		refiner->create_1 = Point();
		refiner->create_2 = Point();

		return;
		}
		else if ((x > refiner->label_locations[SPEED_90].x) &&
		(x < refiner->label_locations[SPEED_90].x + refiner->label_size) &&
		(y > refiner->label_locations[SPEED_90].y) &&
		(y < refiner->label_locations[SPEED_90].y + refiner->label_size)) {

		refiner->label_state = SPEED_90;
		refiner->select = Candidate();
		refiner->move_flag = false;
		refiner->create_flag = false;
		refiner->create_1 = Point();
		refiner->create_2 = Point();

		return;
		}
		else if ((x > refiner->label_locations[SPEED_100].x) &&
		(x < refiner->label_locations[SPEED_100].x + refiner->label_size) &&
		(y > refiner->label_locations[SPEED_100].y) &&
		(y < refiner->label_locations[SPEED_100].y + refiner->label_size)) {

		refiner->label_state = SPEED_100;
		refiner->select = Candidate();
		refiner->move_flag = false;
		refiner->create_flag = false;
		refiner->create_1 = Point();
		refiner->create_2 = Point();

		return;
		}
		else if ((x > refiner->label_locations[ALL].x) &&
			(x < refiner->label_locations[ALL].x + refiner->label_size) &&
			(y > refiner->label_locations[ALL].y) &&
			(y < refiner->label_locations[ALL].y + refiner->label_size)) {

			refiner->label_state = ALL;
			refiner->select = Candidate();
			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			return;
		}
		else if ((x > refiner->prev_location.x) &&
			(x < refiner->prev_location.x + refiner->label_size) &&
			(y > refiner->prev_location.y) &&
			(y < refiner->prev_location.y + refiner->label_size)) {

			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			if (refiner->data_idx == 0) {
				cout << "cannot go prev!" << endl;
				return;
			}

			refiner->label_state = ALL;
			refiner->data_idx--;
			refiner->select = Candidate();
			return;
		}
		else if ((x > refiner->next_location.x) &&
			(x < refiner->next_location.x + refiner->label_size) &&
			(y > refiner->next_location.y) &&
			(y < refiner->next_location.y + refiner->label_size)) {

			refiner->move_flag = false;
			refiner->create_flag = false;
			refiner->create_1 = Point();
			refiner->create_2 = Point();

			if ((refiner->data_idx + 1) >= refiner->data_num) {
				cout << "cannot go next!" << endl;
				return;
			}

			refiner->label_state = ALL;
			refiner->data_idx++;
			refiner->select = Candidate();
			return;
		}
	
		// MOVING 
		if (refiner->move_flag == true) {
			refiner->move_flag = false;

			vector<string> data;

			ifstream in_stream(refiner->txt_list[refiner->data_idx]);
			string input;


			while (!in_stream.eof()) {
				getline(in_stream, input);
				data.push_back(input);
			}
			in_stream.close();

			ofstream out_stream(refiner->txt_list[refiner->data_idx]);
			for (int idx = 0; idx < data.size(); idx++) {
				if (idx == refiner->select.line_num) {
					stringstream ss(data[idx]);
					int label;
					double x_center, y_center, width, height;
					ss >> label;
					width = (refiner->select.location.width) / (double)(refiner->src.cols);
					height = (refiner->select.location.height) / (double)(refiner->src.rows);
					x_center = (x / (double)(refiner->src.cols)) + 0.5 * width;
					y_center = (y / (double)(refiner->src.rows)) + 0.5 * height;

					out_stream << label << " " << x_center << " " << y_center << " ";
					out_stream << width << " " << height << endl;

					refiner->select = Candidate();

					continue;
				}
				out_stream << data[idx] << endl;
			}
			out_stream.close();

			refiner->select = Candidate();
			
			return;
		}

		// CREATING
		if (refiner->button_state == CREATE) {
			if (refiner->create_flag == false) {
				refiner->create_flag = true;
				refiner->create_1 = Point(x, y);
				refiner->create_2 = Point(x, y);
			}
			else if ( ( refiner->create_flag == true ) && ( refiner->label_state != ALL ) ){
				refiner->create_flag = false;
				
				Rect create_location(refiner->create_1, Point(x, y));
				
				vector<string> data;

				ifstream in_stream(refiner->txt_list[refiner->data_idx]);
				string input;


				while (!in_stream.eof()) {
					getline(in_stream, input);
					data.push_back(input);
				}
				in_stream.close();

				ofstream out_stream(refiner->txt_list[refiner->data_idx]);
				for (int idx = 0; idx < data.size(); idx++) {
					if (idx == 0) {
						int label;
						double x_center, y_center, width, height;
						label = refiner->label_state;
						width = (create_location.width) / (double)(refiner->src.cols);
						height = (create_location.height) / (double)(refiner->src.rows);
						x_center = (create_location.x / (double)(refiner->src.cols)) + 0.5 * width;
						y_center = (create_location.y / (double)(refiner->src.rows)) + 0.5 * height;

						out_stream << label << " " << x_center << " " << y_center << " ";
						out_stream << width << " " << height << endl;
					}
					out_stream << data[idx] << endl;
				}
				
				refiner->select = Candidate();
				out_stream.close();

				refiner->create_1 = Point();
				refiner->create_2 = Point();
			}
		}

		// LABEL IS ALL
		if (refiner->label_state == ALL) {
				for (int i = 0; i < LABEL_NUM; i++) {
					for (auto it = refiner->candidates[i].begin(); it != refiner->candidates[i].end(); ++it) {
						if ((x > (*it).location.x) && (x < ((*it).location.x + (*it).location.width)) &&
							(y > (*it).location.y) && (y < ((*it).location.y + (*it).location.height))) {
							(refiner->select).location = (*it).location;
							(refiner->select).line_num = (*it).line_num;
							if (refiner->button_state == MOVE) {
								if (refiner->move_flag == false) {
									refiner->move_flag = true;
									refiner->move.location = refiner->select.location;
									refiner->move.select = (*it);
								}
							}
							else if (refiner->button_state == ERASE) {
								vector<string> data;

								ifstream in_stream(refiner->txt_list[refiner->data_idx]);
								string input;


								while (!in_stream.eof()) {
									getline(in_stream, input);
									data.push_back(input);
								}
								in_stream.close();

								ofstream out_stream(refiner->txt_list[refiner->data_idx]);
								for (int idx = 0; idx < data.size(); idx++) {
									if (idx == refiner->select.line_num) continue;
									out_stream << data[idx] << endl;
								}
								out_stream.close();

								refiner->select = Candidate();
								return;
							}
							else if (refiner->button_state == CREATE) {

							}
							else if (refiner->button_state == QUIT) {
								//
							}

						}
					}
				}
			}

		// SPECIFIC LABEL
		else {
				for (auto it = refiner->candidates[refiner->label_state].begin(); it != refiner->candidates[refiner->label_state].end(); ++it) {
					if ((x > (*it).location.x) && (x < ((*it).location.x + (*it).location.width)) &&
						(y > (*it).location.y) && (y < ((*it).location.y + (*it).location.height))) {
						(refiner->select).location = (*it).location;
						(refiner->select).line_num = (*it).line_num;
						if (refiner->button_state == MOVE) {
							if (refiner->move_flag == false) {
								refiner->move_flag = true;
								refiner->move.location = refiner->select.location;
								refiner->move.select = (*it);
							}
						}
						else if (refiner->button_state == ERASE) {
							vector<string> data;

							ifstream in_stream(refiner->txt_list[refiner->data_idx]);
							string input;


							while (!in_stream.eof()) {
								getline(in_stream, input);
								data.push_back(input);
							}
							in_stream.close();

							ofstream out_stream(refiner->txt_list[refiner->data_idx]);
							for (int idx = 0; idx < data.size(); idx++) {
								if (idx == refiner->select.line_num) continue;
								out_stream << data[idx] << endl;
							}
							out_stream.close();

							refiner->select = Candidate();
							return;
						}
						else if (refiner->button_state == CREATE) {

						}
						else if (refiner->button_state == QUIT) {
							//
						}

					}
				}
			}
		
	}
	else if (event == EVENT_MOUSEMOVE) {
		if (refiner->move_flag == true) {
			refiner->move.location.x = x;
			refiner->move.location.y = y;
		}
		if (refiner->create_flag == true) {
			refiner->create_2 = Point(x, y);
		}
	}
	else if (event == EVENT_RBUTTONDBLCLK) {
		if (refiner->label_state == ALL) {
			for (int i = 0; i < LABEL_NUM; i++) {
				ofstream out_stream(refiner->txt_list[refiner->data_idx]);
				out_stream.close();
				
			}
		}
		else {
			vector<string> data;

			ifstream in_stream(refiner->txt_list[refiner->data_idx]);
			string input;

			while (!in_stream.eof()) {
				getline(in_stream, input);
				data.push_back(input);
			}
			in_stream.close();

			ofstream out_stream(refiner->txt_list[refiner->data_idx]);
			for (int idx = 0; idx < data.size(); idx++) {
				stringstream ss(data[idx]);
				int label;
				ss >> label;
				if(label == refiner->label_state)
					out_stream << data[idx] << endl;
				continue;
			}
			out_stream.close();
		}
		refiner->select = Candidate();
	}
	
}

void YOLODataRefiner::drawScreen() {
	src = imread(img_list[data_idx]);
	if (src.rows > screen_height || src.cols > screen_width)
		resize(src, src, Size(1000, src.rows * 1000 / src.cols));
	
	screen = Mat(screen_height, screen_width, CV_8UC3, Scalar(0, 0, 0));
	src.copyTo(screen(Rect(0,0,src.cols, src.rows)));
	ifstream in(txt_list[data_idx]);
	string input_s;
	
	int count = 0;
	while (!in.eof()) {
		getline(in, input_s);
		stringstream ss(input_s);
		
		string value;
		int label;
		double x_center, y_center, width, height;

		ss.seekg(0, ios::end);
		if (ss.tellg() <= 1) break;
		ss.seekg(0, 0);

		ss >> label;
		ss >> x_center;
		ss >> y_center;
		ss >> width;
		ss >> height;

		Candidate candidate;
		candidate.location = Rect(src.cols * (x_center - 0.5*width), src.rows* (y_center - 0.5*height),
			src.cols * width, src.rows * height);
		candidate.line_num = count;
		candidates[label].push_back(candidate);
		

		/*candidates[label].push_back(Rect(src.cols * (x_center - 0.5*width), src.rows* (y_center - 0.5*height),
			src.cols * width, src.rows * height));*/
		count++;
	}
	in.close();
	
	// Draw buttons
	rectangle(screen, button_locations[MOVE], Scalar(255, 255, 255), -1);
	rectangle(screen, button_locations[ERASE], Scalar(255, 255, 255), -1);
	rectangle(screen, button_locations[CREATE], Scalar(255, 255, 255), -1);
	rectangle(screen, button_locations[QUIT], Scalar(255, 255, 255), -1);

	putText(screen, "Move", Point(button_locations[MOVE].x + 25, button_locations[MOVE].y + 60), 2, 1.5, Scalar(0, 0, 255));
	putText(screen, "Erase", Point(button_locations[ERASE].x + 25, button_locations[ERASE].y + 60), 2, 1.5, Scalar(0, 0, 255));
	putText(screen, "Create", Point(button_locations[CREATE].x + 25, button_locations[CREATE].y + 60), 2, 1.5, Scalar(0, 0, 255));
	putText(screen, "Quit", Point(button_locations[QUIT].x + 25, button_locations[QUIT].y + 60), 2, 1.5, Scalar(0, 0, 255));
	
	// Draw labels
	int button_lastY = button_height * 4 + 10 * 3;
	for (int i = 0; i < LABEL_NUM; i++) {
		int label_row = i / 4;
		int label_col = i % 4;
		if (i == ALL) {
			Rect location = Rect(screen_width - (4 - label_col) * label_size, button_lastY + label_row * label_size + 10, label_size, label_size);
			rectangle(screen, location, Scalar(255, 255, 255), -1);
			putText(screen, "ALL", Point(location.x+ 5, location.y + 48), 2, 1, Scalar(0, 0, 255), 2);
		}
		else
			label_images[i].copyTo(screen(label_locations[i]));
		
		rectangle(screen, label_locations[i], Scalar(0, 0, 0), 7);
	}

	//// Next & Prev
	prev_location = Rect(screen_width - 4 * label_size, button_lastY + 6 * label_size + 10, label_size, label_size);
	rectangle(screen, prev_location, Scalar(255, 255, 255), -1);
	putText(screen, "Prev", Point(prev_location.x + 4, prev_location.y + 46), 2, 0.8, Scalar(0, 0, 255), 2);
	rectangle(screen, prev_location, Scalar(0, 0, 0), 7);

	next_location = Rect(screen_width - 3 * label_size, button_lastY + 6 * label_size + 10, label_size, label_size);
	rectangle(screen, next_location, Scalar(255, 255, 255), -1);
	putText(screen, "Next", Point(next_location.x + 4, next_location.y * 1 + 46), 2, 0.8, Scalar(0, 0, 255), 2);
	rectangle(screen, next_location, Scalar(0, 0, 0), 7);

	// Draw Candidates
	if (label_state == ALL) {
		for (int i = 0; i < LABEL_NUM; i++) {
			for (auto it = candidates[i].begin(); it != candidates[i].end(); ++it) {
				rectangle(screen, (*it).location, colors[i], 2);
				putText(screen, label_names[i], Point((*it).location.x - 5, (*it).location.y - 5), 1, 2, colors[i], 3);
			}
		}
	}
	else {
		for (auto it = candidates[label_state].begin(); it != candidates[label_state].end(); ++it) {
			rectangle(screen, (*it).location, colors[label_state], 2);
			putText(screen, label_names[label_state], Point((*it).location.x - 5, (*it).location.y - 5), 1, 2, colors[label_state], 3);
		}
	}


	// Select Button
	rectangle(screen, button_locations[button_state], Scalar(0, 0, 255), 7);
	// Select Label
	int label_row = label_state / 3;
	int label_col = label_state % 3;
	rectangle(screen, label_locations[label_state], Scalar(0, 0, 255), 7);
	// Draw selected
	rectangle(screen, select.location, Scalar(0, 0, 255), 7);

	// Draw Move
	if ( move_flag == true ){
		rectangle(screen, move.location, Scalar(200, 100, 100), 3, 5);
	}

	// Draw Create
	if (create_flag == true && label_state != ALL) {
		rectangle(screen, Rect(create_1, create_2), Scalar(200, 100, 100), 3, 5);
	}

	putText(screen, "Mode  : " + button_names[button_state], Point(screen_width - 320, screen_height - 50), 1, 1.5, Scalar(0, 0, 255));
	putText(screen, "Label : " + label_names[label_state], Point(screen_width - 320, screen_height - 30), 1, 1.5, Scalar(0,0,255));
	putText(screen, to_string(data_idx+1) + "/" + to_string(data_num), Point(10, screen_height - 10), 1, 1.5, Scalar(0, 0, 255));

	moveWindow(screen_name, 0, 0);
	imshow(screen_name, screen);
	int key = waitKey(10);
	// SPACE :: NEXT
	if (key == 32) {
		move_flag = false;
		create_flag = false;
		create_1 = Point();
		create_2 = Point();

		if ((data_idx + 1) >= data_num) {
			cout << "cannot go next!" << endl;
		}
		else{
			label_state = ALL;
			data_idx++;
			select = Candidate();
		}
	} 
	// Z :: LEFT
	else if (key == 122 || key == 90) {
		move_flag = false;
		create_flag = false;
		create_1 = Point();
		create_2 = Point();

		if (data_idx == 0) {
			cout << "cannot go prev!" << endl;
		}
		else {
			label_state = ALL;
			data_idx--;
			select = Candidate();
		}
	}
	// A :: selecet label All
	else if (key == 65 || key == 97) {
		label_state = ALL;
	}
	// E :: select button Erase
	else if (key == 69 || key == 101) {
		button_state = ERASE;
	}
	// C :: select button create
	else if (key == 67 || key == 99) {
		button_state = CREATE;
	}
	// M :: select button create
	else if (key == 77 || key == 109) {
		button_state = MOVE;
	}
	// ESC :: QUIT
	else if (key == 27) {
		exit(0);
	}


	for (int i = 0; i < LABEL_NUM; i++) {
		candidates[i].clear();
	}
	candidates->clear();
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





