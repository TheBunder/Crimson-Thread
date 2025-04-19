//----INCLUDES--------------------------------------------------------
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include "MazeBasics.h"

using std::abs;
using std::cout;
using std::istringstream;
using std::sort;
using std::string;
using std::vector;

//----FUNCTION DECLARATIONS-------------------------------------
vector<std::array<int, 2>> Search(char** grid, std::array<int, 2> start, std::array<int, 2> goal);
vector<std::array<int, 2>> AStar(char** grid, char** path, std::array<int, 2> start, std::array<int, 2> goal);