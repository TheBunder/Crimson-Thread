//----INCLUDES--------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include <iostream>
#include <windows.h>
#include <chrono>
#include "include/MazeGenerator.h"
#include "include/HostageStation.h"
#include "include/BFS.h"
#include "include/ThreadPool.h"
#include "include/GeneticAlgorithm.h"
#include "include/ConsoleManager.h"
#include "include/Visualizer.h"

//----FUNCTION PROTOTYPES---------------------------------------------
bool EnableAnsiEscapeCodes();
void DeallocateHostageStations(HostageStation **hostageStations, int numOfSections);
void printHostageStationInfo(HostageStation **hostageStations, int numOfSections);
void FillImportantPoints(Point *importantPoints, HostageStation **hostageStations, int numberStations,
                         Point unitsStartingPosition);
void ShowPlan(vector<vector<LocationID>> plan, HostageStation **hostageStations); // Show the plan the units will fallow
void ExplainSigns(); // Explain the various marks and signs in the simulation

//----FUNCTIONS-------------------------------------------------------
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

    HostageStation **hostageStations = new HostageStation *[numOfSections];
    map<PathKey, vector<Point> > pathsBetweenStations;

    // Generate simulation environment with the stations and units entrance.
    Point unitsEntrance = GenerateSimulationEnvironment(grid, hostageStations);

    // Array that holds the points to all hostage station and the unit starting point.
    Point *importantPoints = new Point[numOfSections + 1];
    FillImportantPoints(importantPoints, hostageStations, numOfSections, unitsEntrance);

    // Add a mutex to protect the map from concurrent access
    std::mutex pathMapMutex;

    // Create a thread pool with hardware_concurrency threads (amount of cores in CPU)
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
    std::chrono::duration<double> elapsedIteration = endPathFinding - startProgram;
    printf("Simulation environment creation & Path finding execution time: %f seconds\n", elapsedIteration.count());

    // Main algorithm
    auto startGA = std::chrono::high_resolution_clock::now();
    vector<vector<LocationID>> answer = MainAlgorithm(pathsBetweenStations, numOfUnits, numOfSections , hostageStations);

    // Print GA running execution time
    auto endGA = std::chrono::high_resolution_clock::now();
    elapsedIteration = endGA - startGA;
    printf("\nGenetic algorithm execution time: %f seconds\n", elapsedIteration.count());

    // Print total PValue
    printf("Total PValue for the mission: %.2f\n", SumPValue(answer, hostageStations));

    // Wait for the console thread to finish
    consoleThread.join();

    // Show the best operation found
    printf("\n\nBest plan found: \n");
    ShowPlan(answer, hostageStations);

    // Explaining the visualization
    system("CLS"); // Clear console
    ExplainSigns();

    // Visualize operation found
    system("CLS"); // Clear console
    ShowOperation(grid, numOfUnits, unitsEntrance, answer, pathsBetweenStations);
    getchar();

    // Deallocate space
    DeallocateGrid(grid);
    DeallocateHostageStations(hostageStations, numOfSections);
    delete[] importantPoints;
}

// Function to populate an array with the coordinates of important points,
// including the units' starting position and all hostage stations.
void FillImportantPoints(Point *importantPoints, HostageStation **hostageStations, int numberStations,
                         Point unitsStartingPosition) {
    // Assign the units' starting position as the first important point (at index 0).
    importantPoints[0] = unitsStartingPosition;

    // Iterate through the hostage stations and add their coordinates from index 1.
    for (int i = 1; i <= numberStations; i++) {
        // Stor the station location
        importantPoints[i] = hostageStations[i - 1]->GetCoords();
    }
}

void ShowPlan(const vector<vector<LocationID>> plan, HostageStation **hostageStations) {
    for (int u = 0; u < plan.size(); ++u) {
        UnitColor();
        printf("------Unit number #%d plan:------\n", u);
        if (plan[u].size() == 1) {
            ResetFG();
            printf("No stations assigned\n");
        }
        else {
            // Print info about each of the stations
            HostagesColor();
            for (int s = 1; s < plan[u].size(); ++s) {
                hostageStations[plan[u][s]-1]->PrintInfo();
            }
        }
    }

    ResetFG();

    printf("\nTo continue to the visualization proses, please press any button");
    getchar();
}

void ExplainSigns() {
    printf("Before we start visualizing the simulation, here are all the marks and signs you need to know: \n\n");

    // Explain signs
    UnitColor();
    printf("The Units will be represented using this ASCII sign: %c and in this very color.\n", UNIT);
    HostagesColor();
    printf("The Hostage Stations will be represented using this ASCII sign: %c and in this very color.\n", HOSTAGES);

    ResetFG();

    // Explain marks
    printf("\nThe path the unit will take will be marked by this color: ");
    PathColor();
    printf("              \n");
    ResetBG();
    printf("The path that the unit toke and will no longer go over will be marked by this color: ");
    FinishedPathColor();
    printf("              \n");
    ResetBG();

    printf("\nPlease press any key to start the visualization of the best plan the genetic algorithm found\n");
    getchar();
}

// Deallocate the memory used by an array of HostageStation objects
// and the array of pointers itself.
void DeallocateHostageStations(HostageStation **hostageStations, int numOfSections) {
    for (int i = 0; i < numOfSections; i++) {
        delete hostageStations[i];
    }
    delete[] hostageStations;
}

// Print the info about each one of the HostageStation
void PrintHostageStationInfo(HostageStation **hostageStations, int numOfSections) {
    for (int i = 0; i < numOfSections; i++) {
        hostageStations[i]->PrintInfo();
    }
}