//----INCLUDES--------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include <iostream>
#include <windows.h>
#include "MazeGenerator.h"
#include "HostageStation.h"
#include "Unit.h"
#include "AStar.h"

//----FUNCTION PROTOTYPES---------------------------------------------
char** allocateGrid();
void deallocateGrid(char** grid);
void deallocateHostageStations(HostageStation** HostageStations, int numOfSections);
void printHostageStationInfo(HostageStation** HostageStations, int numOfSections);

//----FUNCTIONS-------------------------------------------------------
int main()
{
	char** grid = allocateGrid();
	char** gridPath = allocateGrid();
	int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);
	HostageStation** HostageStations = new HostageStation * [numOfSections];

	generate(grid, HostageStations);
	//PrintGrid(grid);
	Point start = {15,5};

	vector<Point> path = AStar(grid, gridPath, HostageStations[0]->getCoords(), HostageStations[2]->getCoords());
	
	PrintGridWithPath(grid, gridPath, path);

	//printHostageStationInfo(HostageStations, numOfSections);

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