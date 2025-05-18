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
#include "include/ConsoleManager.h"

//----FUNCTION PROTOTYPES---------------------------------------------
bool EnableAnsiEscapeCodes();
void DeallocateHostageStations(HostageStation **HostageStations, int numOfSections);
void printHostageStationInfo(HostageStation **HostageStations, int numOfSections);
void FillImportantPoints(Point *importantPoints, HostageStation **HostageStations, int numberStations,
                         Point unitsStartingPosition);

int main() {
    // prep
    system("CLS"); // Clear console
    auto startProgram = std::chrono::high_resolution_clock::now();
	srand(time(0)); // seed random number generator.

    // Set the console on a separate thread
    thread consoleThread(SetConsole);

    // variables
    char **grid = AllocateGrid();
    int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);
    int numOfUnits = (rand() % 3) + 3;

    HostageStation **HostageStations = new HostageStation *[numOfSections];
    map<PathKey, vector<Point> > pathsBetweenStations;

    // Generate maze and hostages (in the future, maybe units)
    Point unitsEntrance = Generate(grid, HostageStations);

    // Array that holds the points to all hostage station and the unit starting point.
    Point *importantPoints = new Point[numOfSections + 1];
    FillImportantPoints(importantPoints, HostageStations, numOfSections, unitsEntrance);

    // Add a mutex to protect the map from concurrent access
    std::mutex pathMapMutex;

    // Create a thread pool with hardware_concurrency threads
    ThreadPool pool(std::thread::hardware_concurrency());

    // Find the best path between each one of the important points
    for (LocationID i = 0; i < numOfSections + 1; i++) {
        // Capture i by value to avoid issues with the loop variables changing
        pool.Enqueue([i, &grid, &importantPoints, &pathsBetweenStations, &pathMapMutex, numOfSections]() {
            // Calculate the path
            BFS(grid, i, importantPoints[i], importantPoints, numOfSections+1, pathsBetweenStations, pathMapMutex);
        });
    }

    // Wait for all tasks to complete before proceeding
    pool.WaitAll();

    // End Path finding time and print it
    auto endPathFinding = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_iteration = endPathFinding - startProgram;
    printf("Path finding execution time: %fl seconds\n", elapsed_iteration.count());

    // Main algorithm
    auto startGA = std::chrono::high_resolution_clock::now();
    vector<vector<LocationID>> answer = MainAlgorithm(pathsBetweenStations, numOfUnits, numOfSections , HostageStations);

    // Print GA running execution time
    auto endGA = std::chrono::high_resolution_clock::now();
    elapsed_iteration = endGA - startGA;
    printf("\nGenetic algorithm execution time: %fl seconds\n", elapsed_iteration.count());

    // Print total Pvalue
    printf("Total PValue for the mission: %.2f\n", SumPValue(answer, HostageStations));

    // Wait for the console thread to finish
    consoleThread.join();

    // Show the best operation found
    ShowOperation(grid, numOfUnits, unitsEntrance, answer, pathsBetweenStations);

    // Deallocate space
    DeallocateGrid(grid);
    DeallocateHostageStations(HostageStations, numOfSections);
    delete[] importantPoints;

    // Print running time
    endPathFinding = std::chrono::high_resolution_clock::now();
    elapsed_iteration = endPathFinding - startProgram;
    printf("\nTotal execution time: %fl seconds", elapsed_iteration.count());
}

//----FUNCTIONS-------------------------------------------------------

// Function to populate an array with the coordinates of important points,
// including the units' starting position and all hostage stations.
void FillImportantPoints(Point *importantPoints, HostageStation **HostageStations, int numberStations,
                         Point unitsStartingPosition) {
    // Assign the units' starting position as the first important point (at index 0).
    importantPoints[0] = unitsStartingPosition;

    // Iterate through the hostage stations and add their coordinates from index 1.
    for (int i = 1; i <= numberStations; i++) {
        // Stor the station location
        importantPoints[i] = HostageStations[i - 1]->GetCoords();
    }
}

// Deallocate the memory used by an array of HostageStation objects
// and the array of pointers itself.
void DeallocateHostageStations(HostageStation **HostageStations, int numOfSections) {
    for (int i = 0; i < numOfSections; i++) {
        delete HostageStations[i];
    }
    delete[] HostageStations;
}

// Print the info about each one of the HostageStation
void PrintHostageStationInfo(HostageStation **HostageStations, int numOfSections) {
    for (int i = 0; i < numOfSections; i++) {
        HostageStations[i]->PrintInfo();
    }
}