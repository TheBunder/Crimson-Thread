//----INCLUDES--------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include "MazeGenerator.h"
#include "HostageStation.h"

//----FUNCTION PROTOTYPES---------------------------------------------
char** allocateGrid();
void deallocateGrid(char** grid);
void deallocateHostageStations(HostageStation** HostageStations, int numOfSections);
void printHostageStationInfo(HostageStation** HostageStations, int numOfSections);

//----FUNCTIONS-------------------------------------------------------
int main()
{
	char** grid = allocateGrid();
	int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);
	HostageStation** HostageStations = new HostageStation * [numOfSections];

	generate(grid, HostageStations);
	PrintGrid(grid);

	printHostageStationInfo(HostageStations, numOfSections);

	deallocateGrid(grid);
	deallocateHostageStations(HostageStations, numOfSections);
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

void deallocateHostageStations(HostageStation** HostageStations, int numOfSections) {
	for (int i = 0; i < numOfSections; i++) {
		delete HostageStations[i];
	}
	delete[] HostageStations;
}

void printHostageStationInfo(HostageStation** HostageStations, int numOfSections) {
	for (int i = 0; i < numOfSections; i++) {
		HostageStations[i]->printInfo();
	}
}