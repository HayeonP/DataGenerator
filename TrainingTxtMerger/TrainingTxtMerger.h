#include "TextLabelEditor.h"
#include <filesystem>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdlib>

#define SPEED_OTHER				80
#define CHILD_PROTECT			81
#define NO_LEFT					82
#define NO_RIGHT				83
#define NO_PARKING				84
#define CROSSWALK				85
#define NO_UTURN				86
#define ROTATE					87
#define SLOW					88
#define SPEED_BUST				89
#define UNPROTECTED_LEFT		90
#define UTURN					91
#define NO_STRAIGHT				92
#define SPEED_20				93
#define SPEED_30				94
#define SPEED_40				95
#define SPEED_50				96
#define SPEED_60				97
#define SPEED_70				98
#define SPEED_80				99
#define SPEED_90				100
#define SPEED_100				101
#define TRAFFIC_SIGN			9
#define LABEL_NUM				23

#define SPEED_OTHER_IDX 0			
#define CHILD_PROTECT_IDX 1
#define NO_LEFT_IDX 2			
#define NO_RIGHT_IDX 3				
#define NO_PARKING_IDX 4				
#define CROSSWALK_IDX 5		
#define NO_UTURN_IDX 6		
#define ROTATE_IDX 7		
#define SLOW_IDX 8			
#define SPEED_BUST_IDX 9				
#define UNPROTECTED_LEFT_IDX 10		
#define UTURN_IDX 11
#define NO_STRAIGHT_IDX 12			
#define SPEED_20_IDX 13	
#define SPEED_30_IDX 14		
#define SPEED_40_IDX 15		
#define SPEED_50_IDX 16		
#define SPEED_60_IDX 17		
#define SPEED_70_IDX 18		
#define SPEED_80_IDX 19		
#define SPEED_90_IDX 20		
#define SPEED_100_IDX 21		
#define TRAFFIC_SIGN_IDX 22

