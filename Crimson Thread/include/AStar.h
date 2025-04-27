//----INCLUDES--------------------------------------------------------
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include "Utils.h"

//----NAMESPACES------------------------------------------------------
using std::abs;
using std::cout;
using std::istringstream;
using std::sort;
using std::string;
using std::vector;

//----FUNCTION DECLARATIONS-------------------------------------
vector<Point> AStar(char** grid,  Point start, Point goal);