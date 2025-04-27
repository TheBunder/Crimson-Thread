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
#include "include/AStar.h"
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
    enable_ansi_escape_codes(); // Used to enable colors

    // variables
    char **grid = allocateGrid();
    char **navGrid = allocateGrid();
    int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);
    int numOfUnits = (rand() % 3) + 3;

    HostageStation **HostageStations = new HostageStation *[numOfSections];
    map<PathKey, vector<Point> > pathsBetweenStations;

    // Generate maze and hostages (in the future, maybe units)
    Point unitsStartingPosition = generate(grid, HostageStations); // <---------------------------------------

    // Array that holds the points to all hostage station and the unit starting point.
    Point *importantPoints = new Point[numOfSections + 1];
    fillImportantPoints(importantPoints, HostageStations, numOfSections, unitsStartingPosition);

    // Add a mutex to protect the map from concurrent access
    std::mutex pathMapMutex;

    // Create a thread pool with hardware_concurrency threads
    ThreadPool pool(std::thread::hardware_concurrency());

    // Find the best path between each one of the important points
    for (LocationID i = 0; i < numOfSections + 1; i++) {
        for (LocationID j = i + 1; j < numOfSections + 1; j++) {
            // Capture i and j by value to avoid issues with the loop variables changing
            pool.enqueue([i, j, &grid, &importantPoints, &pathsBetweenStations, &pathMapMutex]() {
                // Calculate the path
                vector<Point> path = AStar(grid, importantPoints[i], importantPoints[j]);

                // Safely store the result in the map using a mutex
                std::lock_guard<std::mutex> lock(pathMapMutex);
                pathsBetweenStations[{i, j}] = path;
            });
        }
    }

    // Wait for all tasks to complete before proceeding
    pool.wait_all();

    // End Path finding time and print it
    auto end_iteration = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_iteration = end_iteration - start_iteration;
    printf("Path finding execution time: %fl seconds\n", elapsed_iteration.count());

    // this_thread::sleep_for(chrono::seconds(1));

    // for (LocationID i = 0; i < numOfSections + 1; i++) {
    //     for (LocationID j = i + 1; j < numOfSections + 1; j++) {
    //         printf("Distance from %d to %d is: %d\n",i,j, pathsBetweenStations[{i,j}].size()-1);
    //     }
    // }

    // Genetic Algorithm
    initialization(pathsBetweenStations, numOfSections, numOfUnits);

    /*
    // Print each one of the paths for debug
    for (LocationID i = 0; i < numOfSections; i++)
    {
        for (LocationID j = i+1; j < numOfSections; j++)
        {
            PrintGridWithPath(grid, navGrid, pathsBetweenStations[{i,j}]);
        }
    }
    */

    // Print the info about each of the HS for debug
    //printHostageStationInfo(HostageStations, numOfSections);

    // Deallocate space
    deallocateGrid(grid);
    deallocateGrid(navGrid);
    deallocateHostageStations(HostageStations, numOfSections);

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
