//----INCLUDES--------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include <iostream>
#include <windows.h>
#include <chrono>
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

	auto start_iteration = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < numOfSections; i++)
	{
		for (int j = i+1; j < numOfSections; j++)
		{
			AStar(grid, gridPath, HostageStations[i]->getCoords(), HostageStations[j]->getCoords());
		}
	}
	auto end_iteration = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed_iteration = end_iteration - start_iteration;

	std::cout << "Total execution time: " << elapsed_iteration.count() << " seconds" << std::endl;

	//PrintGridWithPath(grid, gridPath, path);

	//printHostageStationInfo(HostageStations, numOfSections);

	deallocateGrid(grid);
	deallocateGrid(gridPath);
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