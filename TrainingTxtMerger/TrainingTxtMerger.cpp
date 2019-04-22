#include "TrainingTxtMerger.h"

#define DB_PATH "C:\\SSD_DB"
#define TRAIN_NAME "final_train.txt"
#define VALID_NAME "final_valid.txt"

using namespace PHY;
namespace fs = std::experimental::filesystem;

int label_to_idx(int label) {
	int idx;
	if (label == 9) idx = 22;
	else idx = label - 80;
	return idx;
}

struct Data {
	string txt_path;
	vector<int> label_num = vector<int>(23);
};


int main(int argc, char** argv) {
	
	vector<Data> dataset;
	vector<int> total_label_num(23);
	vector<int> total_valid_num(23);

	// Collect data info
	for (auto& p : fs::directory_iterator(DB_PATH)){
		if (!fs::is_directory(p.status())) continue;
		
		string dirname = p.path().string();
		string add_string;
		istringstream is(dirname);
		while (getline(is, add_string, '\\')) {}
		cout << add_string << endl;

		for (auto& q : fs::directory_iterator(dirname)) {
			string filename = q.path().string();
			if (filename.find("txt") == string::npos) continue;
			if (filename.find("train.txt") != string::npos) continue;
			if (filename.find("train2.txt") != string::npos) continue;
			
			TextLabelEditor editor(filename);			
			vector<nodelist> node_data = editor.get_node_data();
			Data temp_data;

			// Get path for train.txt
			istringstream is(filename);
			string temp;
			nodelist path_node;
			while (getline(is, temp, '\\')) {
				path_node.push_back(temp);
			}
			is.clear();
			is = istringstream(path_node.back());
			string file_lastname;
			getline(is, file_lastname, '.');
			string path = "data/img/" + add_string + "/" + file_lastname + ".jpg";
			temp_data.txt_path = path;
			// Add label num
			for (auto it = node_data.begin(); it != node_data.end(); ++it) {				
				auto node = *it;
				int label = atoi(node[0].c_str());
				int idx = label_to_idx(label);
				total_label_num[idx]++;
				temp_data.label_num[idx]++;
			}
			dataset.push_back(temp_data);
		}

		
	}

	// Calculate valid data ratio
	int total = 0;
	for (int i = 0; i < total_label_num.size(); i++) {
		int num = total_label_num[i];
		total += num;
		float valid_ratio;
		if (num <= 2000) valid_ratio = 0.2;
		else valid_ratio = 0.3;
		total_valid_num[i] = total_label_num[i] * valid_ratio;
	}
	
	cout << " total : " << total << endl;

	ofstream train_out(string(DB_PATH) + "\\" + TRAIN_NAME);
	ofstream valid_out(string(DB_PATH) + "\\" + VALID_NAME);
	int train_num = 0, valid_num = 0;
	
	srand((unsigned int)time(NULL));
	while (!dataset.empty()) {
		int idx = rand() / dataset.size();
		int valid_sum = 0;
		for (int i = 0; i < LABEL_NUM; i++) {
			int num = dataset[idx].label_num[i];
			total_label_num[i] -= num;
			valid_sum += total_label_num[i];
		}
		if (valid_sum > 0) {
			valid_out << dataset[idx].txt_path << endl;
			valid_num++;
		}
		else {
			train_out << dataset[idx].txt_path << endl;
			train_num++;
		}

		dataset.erase(dataset.begin() + idx);
		cout << "size : " << dataset.size() << endl;
		if ((dataset.size() % 10000) == 0) cout << "10000" << endl;
	}
	cout << valid_num << endl;
	cout << train_num << endl;
}
