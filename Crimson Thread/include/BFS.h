#ifndef BFS_H
#define BFS_H
//----INCLUDES--------------------------------------------------------
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <queue>
#include "Utils.h"
#include "include/ThreadPool.h"

//----NAMESPACES------------------------------------------------------
using std::abs;
using std::cout;
using std::istringstream;
using std::string;
using std::vector;
using std::queue;

//----FUNCTION DECLARATIONS-------------------------------------
void BFS(char **grid, LocationID startID, Point start, Point *importantPoints, int importantPointsSize,
         map<PathKey, vector<Point> > &pathsBetweenStations, mutex &pathMapMutex);

#endif //BFS_H
