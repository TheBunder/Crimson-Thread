//----INCLUDES--------------------------------------------------------
#include <iostream>
#include "MazeBasics.h"
#include "HostageStation.h"

//----FUNCTION DECLARATIONS-------------------------------------
void generate(char** grid, HostageStation** HostageStations); // Generate the maze and insert the people and the GPS stations
void PrintGrid(char** grid); // Print the maze
void PrintGridWithPath(char** grid, char** gridPath, vector<Point> path); // Print the array with the A* search