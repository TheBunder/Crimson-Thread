//----INCLUDES--------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include "MazeGenerator.h"
#include "HostageStation.h"

//----FUNCTION PROTOTYPES---------------------------------------------
char** allocateGrid();
void deallocateGrid(char** grid);
void printHostageStationInfo(HostageStation* HostageStations, int numOfSections);

//----FUNCTIONS-------------------------------------------------------
int main()
{
	char** grid = allocateGrid();
	int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);
	HostageStation* HostageStations = (HostageStation*)malloc(numOfSections * sizeof(HostageStation));
	generate(grid, HostageStations);
	PrintGrid(grid);

	printHostageStationInfo(HostageStations, numOfSections);

	deallocateGrid(grid);
	free(HostageStations);
}

char** allocateGrid() {
	char** grid = new char* [GRID_WIDTH];
	for (int i = 0; i < GRID_WIDTH; i++) {
		grid[i] = new char[GRID_HEIGHT];
	}
	return grid;
}

void deallocateGrid(char** grid) {
	for (int i = 0; i < GRID_WIDTH; i++) {
		delete[] grid[i];
	}
	delete[] grid;
}

void printHostageStationInfo(HostageStation* HostageStations, int numOfSections) {
	for (int i = 0; i < numOfSections; i++) {
		HostageStations[i].printInfo();
	}
}