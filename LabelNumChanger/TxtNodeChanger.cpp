#include "TextLabelEditor.h"
#include <filesystem>
#include <vector>
#include <sstream>
#include <fstream>

#define PATH "C:\\SSD_DB"
#define LABEL_INDEX 0
#define LABEL_OFFSET 80

using namespace PHY;
namespace fs = std::experimental::filesystem;

int main() {
	for (auto& p : fs::directory_iterator(PATH)) {
		string dirname = p.path().string();
		int num = 0;
		cout << dirname << endl;
		for (auto& q : fs::directory_iterator(dirname)) {
			
			string filename = q.path().string();
			if (filename.find("txt") == std::string::npos) continue;
			num++;

			if (filename.find("train2.txt") != std::string::npos) continue;

			TextLabelEditor editor(filename);
			
			vector<nodelist> node_data = editor.get_node_data();
			
			if (node_data.empty()) continue;
			
			for (auto it = node_data.begin(); it != node_data.end(); ++it) {
				if ((*it).empty()) continue;
				int label = atoi((*it)[LABEL_INDEX].c_str());
				if (label == 22) label = 9;
				else label += LABEL_OFFSET;
				(*it)[LABEL_INDEX] = to_string(label);
			}

			editor.write_data(node_data);
		}

		cout << "Done!" << endl;
	}
}