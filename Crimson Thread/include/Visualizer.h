#ifndef VISUALIZER_H
#define VISUALIZER_H
//----INCLUDES--------------------------------------------------------
#include "Utils.h"

//----FUNCTION DECLARATIONS-------------------------------------
void PrintGrid(char** grid); // Print the maze
void PrintGridWithPath(char** grid, char** navGrid, vector<Point> path); // Print the array with the A* search
void ShowOperation(char **grid, int numOfUnits, Point unitsEntrance, vector<vector<LocationID> > &OperationOrder,
                   map<PathKey, vector<Point> > &pathsBetweenStations);

void HostagesColor();
void UnitColor();
void ResetFG();
void PathColor();
void FinishedPathColor();
void ResetBG();

#endif //VISUALIZER_H
