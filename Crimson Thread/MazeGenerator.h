//----INCLUDES--------------------------------------------------------
#include <iostream>
#include "MazeChars.h"
#include "HostageStation.h"

//----FUNCTION DECLARATIONS-------------------------------------
void generate(char** grid, HostageStation** HostageStations); // Generate the maze and insert the people and the GPS stations
void PrintGrid(char** grid); // Print the array