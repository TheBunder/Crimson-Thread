//----INCLUDES--------------------------------------------------------
#include <iostream>
#include "Utils.h"
#include "HostageStation.h"

//----FUNCTION DECLARATIONS-------------------------------------
Point Generate(char** grid, HostageStation** HostageStations); // Generate the maze and insert the people and the GPS stations
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