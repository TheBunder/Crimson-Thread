//----INCLUDES--------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include <iostream>
#include <windows.h>
#include <chrono>
#include <map>
#include "include/MazeGenerator.h"
#include "include/HostageStation.h"
#include "include/BFS.h"
#include "include/ThreadPool.h"
#include "include/GeneticAlgorithm.h"

//----FUNCTION PROTOTYPES---------------------------------------------
bool enable_ansi_escape_codes();
void deallocateHostageStations(HostageStation **HostageStations, int numOfSections);
void printHostageStationInfo(HostageStation **HostageStations, int numOfSections);
void fillImportantPoints(Point *importantPoints, HostageStation **HostageStations, int numberStations,
                         Point unitsStartingPosition);

int main() {
    // prep
    auto start_iteration = std::chrono::high_resolution_clock::now();
    enable_ansi_escape_codes(); // Used to enable
	srand(time(0)); // seed random number generator.

    // variables
    char **grid = allocateGrid();
    int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);
    int numOfUnits = (rand() % 3) + 3;

    HostageStation **HostageStations = new HostageStation *[numOfSections];
    map<PathKey, vector<Point> > pathsBetweenStations;

    // Generate maze and hostages (in the future, maybe units)
    Point unitsEntrance = generate(grid, HostageStations); // <---------------------------------------

    // Array that holds the points to all hostage station and the unit starting point.
    Point *importantPoints = new Point[numOfSections + 1];
    fillImportantPoints(importantPoints, HostageStations, numOfSections, unitsEntrance);

    // Add a mutex to protect the map from concurrent access
    std::mutex pathMapMutex;

    // Create a thread pool with hardware_concurrency threads
    ThreadPool pool(std::thread::hardware_concurrency());

    // Find the best path between each one of the important points
    for (LocationID i = 0; i < numOfSections + 1; i++) {
        // Capture i by value to avoid issues with the loop variables changing
        pool.enqueue([i, &grid, &importantPoints, &pathsBetweenStations, &pathMapMutex, numOfSections]() {
            // Calculate the path
            BFS(grid, i, importantPoints[i], importantPoints, numOfSections+1, pathsBetweenStations, pathMapMutex);
        });
    }

    // Wait for all tasks to complete before proceeding
    pool.wait_all();

    // End Path finding time and print it
    auto end_iteration = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_iteration = end_iteration - start_iteration;
    printf("Path finding execution time: %fl seconds\n", elapsed_iteration.count());

    // Main algorithm
    vector<vector<LocationID>> answer = mainAlgorithm(pathsBetweenStations, numOfSections, numOfUnits, HostageStations);
    // Print total Pvalue
    printf("Total PValue for the mission: %.2f\n", sumPValue(answer, HostageStations));

    ShowOperation(grid, numOfUnits, unitsEntrance, answer, pathsBetweenStations);

    // Deallocate space
    deallocateGrid(grid);
    deallocateHostageStations(HostageStations, numOfSections);
    delete[] importantPoints;
    // Print running time
    end_iteration = std::chrono::high_resolution_clock::now();
    elapsed_iteration = end_iteration - start_iteration;
    printf("\nTotal execution time: %fl seconds", elapsed_iteration.count());
}

//----FUNCTIONS-------------------------------------------------------

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

void fillImportantPoints(Point *importantPoints, HostageStation **HostageStations, int numberStations,
                         Point unitsStartingPosition) {
    importantPoints[0] = unitsStartingPosition;
    for (int i = 1; i <= numberStations; i++) {
        importantPoints[i] = HostageStations[i - 1]->getCoords();
    }
}

void deallocateHostageStations(HostageStation **HostageStations, int numOfSections) {
    for (int i = 0; i < numOfSections; i++) {
        delete HostageStations[i];
    }
    delete[] HostageStations;
}

void printHostageStationInfo(HostageStation **HostageStations, int numOfSections) {
    for (int i = 0; i < numOfSections; i++) {
        HostageStations[i]->printInfo();
    }
}