#include <iostream>
#include <fstream>
#include <vector>
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

char** Search(char** grid, std::array<int, 2> start, std::array<int, 2> goal);
void AStar(char** grid, char** path, std::array<int, 2> start, std::array<int, 2> goal);