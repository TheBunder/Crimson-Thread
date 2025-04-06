//----INCLUDES--------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include <iostream>
#include <windows.h>
#include "MazeGenerator.h"
#include "HostageStation.h"
#include "Unit.h"

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
	
	// Initialize random seed
	srand(time(0));

	// Create the Unit at a starting position
	Unit* Unit2 = new Unit(1, 1, grid[1][1]);
	Unit* Unit4 = new Unit(20, 20, grid[20][20]);
	Unit* Unit8 = new Unit(30, 40, grid[30][40]);
	Sleep(5000);
	// Simulate a few moves
	for (int i = 0; i < 1000; ++i) {
		Unit2->move(grid);
		Unit4->move(grid);
		Unit8->move(grid);
		Sleep(1000);
		//Print from top left - removes stater
		COORD coord = { 0, 0 };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

		//printf("Unit's current position: (%d, %d)\n", playerUnit->getX(), playerUnit->getY());
		PrintGrid(grid);
		fflush(stdout);
	}


	//printHostageStationInfo(HostageStations, numOfSections);

	deallocateGrid(grid);
	deallocateHostageStations(HostageStations, numOfSections);
	delete Unit2;
	delete Unit4;
	delete Unit8;
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