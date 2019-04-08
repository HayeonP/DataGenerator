#include "YOLODataRefiner.h"
#include <fstream>
#include <iostream>
#include <string>





int main() {
	YOLODataRefiner _YOLODataRefiner("Screen", TRAIN_PATH, CREATE, TRAFFIC_SIGN);
	
	while (1) {
		_YOLODataRefiner.drawScreen();
	}

	return 0;
}


