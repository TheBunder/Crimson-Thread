#include <iostream>
#include "mazeChars.h"
#include "HostageStation.h"

const int GRID_SIZE = 101;
const int GRID_WIDTH = GRID_SIZE;
const int GRID_HEIGHT = GRID_SIZE;
const int SUBGRID_SIZE = GRID_SIZE / 4;
void generate(char** grid, HostageStation* HostageStations); // Generate the maze and insert the people and the GPS stations
void PrintGrid(char** grid); // Print the array