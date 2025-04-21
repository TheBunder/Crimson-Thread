//----INCLUDES--------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include <iostream>
#include <windows.h>
#include <chrono>
#include <map>
#include "MazeGenerator.h"
#include "HostageStation.h"
#include "AStar.h"
#include "ThreadPool.h"

using std::map;
using std::pair;

//----FUNCTION PROTOTYPES---------------------------------------------
bool enable_ansi_escape_codes();
void deallocateHostageStations(HostageStation** HostageStations, int numOfSections);
void printHostageStationInfo(HostageStation** HostageStations, int numOfSections);

//----FUNCTIONS-------------------------------------------------------
int main()
{
	// prep
	auto start_iteration = std::chrono::high_resolution_clock::now();
	enable_ansi_escape_codes(); // Used to enable colors

	// variables
	char** grid = allocateGrid();
	char** navGrid = allocateGrid();
	int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);
	HostageStation** HostageStations = new HostageStation * [numOfSections];
	map<pair<int, int>, vector<Point>> pathsBetweenStations;

	generate(grid, HostageStations);

	std::mutex pathMapMutex; // Add a mutex to protect the map from concurrent access

	// Create a thread pool with hardware_concurrency threads (usually better than creating too many threads)
	ThreadPool pool(std::thread::hardware_concurrency());

	for (int i = 0; i < numOfSections; i++)
	{
		for (int j = i+1; j < numOfSections; j++)
		{
			// Capture i and j by value to avoid issues with the loop variables changing
			pool.enqueue([i, j, &grid, &HostageStations, &pathsBetweenStations, &pathMapMutex]() {
				// Calculate the path
				auto path = AStar(grid, HostageStations[i]->getCoords(), HostageStations[j]->getCoords());

				// Safely store the result in the map using a mutex
				std::lock_guard<std::mutex> lock(pathMapMutex);
				pathsBetweenStations[{i, j}] = path;

			});
		}
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	for (int i = 0; i < numOfSections; i++)
	{
		for (int j = i+1; j < numOfSections; j++)
		{
			PrintGridWithPath(grid, navGrid, pathsBetweenStations[{i,j}]);
		}
	}

	//printHostageStationInfo(HostageStations, numOfSections);

	deallocateGrid(grid);
	deallocateGrid(navGrid);
	deallocateHostageStations(HostageStations, numOfSections);

	// Print running time
	auto end_iteration = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed_iteration = end_iteration - start_iteration;

	printf("Total execution time: %d seconds",elapsed_iteration.count());
}

//  Enables ANSI escape code processing on Windows console.
// Returns true if successful, false otherwise.
bool enable_ansi_escape_codes() {
	// Get a handle to the standard output device (the console)
	HANDLE stdOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdOutputHandle == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Error getting standard output handle.\n");
		return false;
	}

	// Get the current console mode flags
	DWORD consoleMode = 0;
	if (!GetConsoleMode(stdOutputHandle, &consoleMode)) {
		fprintf(stderr, "Error getting console mode.\n");
		return false;
	}

	// Check if ANSI processing is already enabled
	if (consoleMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) {
		// Already enabled, nothing to do
		return true;
	}

	// Set the ENABLE_VIRTUAL_TERMINAL_PROCESSING flag
	consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

	// Set the new console mode
	if (!SetConsoleMode(stdOutputHandle, consoleMode)) {
		fprintf(stderr, "Error setting console mode. This may require Windows 10 or newer.\n");
		return false;
	}

	return true;
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