#include "YOLODataRefiner.h"
#include <fstream>
#include <iostream>
#include <string>





int main() {
	




	YOLODataRefiner _YOLODataRefiner("Screen", MOVE, ALL);

	Mat src = imread(IMAGE_PATH);

	while (1) {
		_YOLODataRefiner.drawScreen(src);
	}


	
	

	return 0;
}


