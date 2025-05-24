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
// Free all the space HS took.
void DeallocateHostageStations(HostageStation **hostageStations, int numOfSections);

// Print the info about all the HS
void printHostageStationInfo(HostageStation **hostageStations, int numOfSections);

void FillImportantPoints(vector<pair<LocationID, Point>> &importantPoints, HostageStation **hostageStations, int numberStations,
                         Point unitsEntrance); // Insert all location and ID of valuable HS and the unit entrance.

// Remove all points that are not reachable from the entrance.
void RemoveUnreachablePoints(vector<pair<LocationID, Point>> &importantPoints,
                        const map<PathKey, vector<Point> > &pathsBetweenStations);

// Show each unit HS
void ShowPlan(vector<vector<LocationID> > plan, HostageStation **hostageStations);

// Show the plan the units will fallow
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
    if (grid == nullptr) {
        PrintError("Error: Failed to allocate maze grid. Exiting.\n");
        getchar();
        return -1;
    }
    int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);
    int numOfUnits = (rand() % 3) + 3;

    HostageStation **hostageStations = new HostageStation *[numOfSections];
    map<PathKey, vector<Point> > pathsBetweenStations;

    // Generate simulation environment with the stations and units entrance.
    Point unitsEntrance = GenerateSimulationEnvironment(grid, hostageStations);
    if (unitsEntrance == Point(-1, -1)) {
        PrintError("Error: Failed to generate simulation environment. Exiting.\n");
        DeallocateGrid(grid);
        DeallocateHostageStations(hostageStations, numOfSections);
        getchar();
        return -1;
    }

    // Array that holds the points to all hostage station and the unit starting point.
    vector<pair<LocationID, Point>> importantPoints;
    FillImportantPoints(importantPoints, hostageStations, numOfSections, unitsEntrance);
    if (importantPoints.empty()) {
        PrintError("Error: Failed to generate important points.\n");
        DeallocateGrid(grid);
        DeallocateHostageStations(hostageStations, numOfSections);
        getchar();
        return -1;
    }
    if (importantPoints.size() == 1) {
        printf("There are no station worth the risk.");
        DeallocateGrid(grid);
        DeallocateHostageStations(hostageStations, numOfSections);
        getchar();
        return -1;
    }

    // Add a mutex to protect the map from concurrent access
    std::mutex pathMapMutex;

    // Create a thread pool with hardware_concurrency threads (number of cores in CPU)
    ThreadPool pool(std::thread::hardware_concurrency());

    // Find the best path between each one of the important points
    for (LocationID i = 0; i < importantPoints.size(); i++) {
        // Capture i by value to avoid issues with the loop variables changing
        pool.Enqueue([i, &grid, &importantPoints, &pathsBetweenStations, &pathMapMutex]() {
            // Calculate the path
            BFS(grid, importantPoints[i].first, importantPoints[i].second, importantPoints,  pathsBetweenStations, pathMapMutex);
        });
    }

    // Wait for all tasks to complete before proceeding
    pool.WaitAll();

    // End Path finding time and print it
    auto endPathFinding = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedIteration = endPathFinding - startProgram;
    printf("Simulation environment creation & Path finding execution time: %f seconds\n", elapsedIteration.count());

    // Main algorithm
    RemoveUnreachablePoints(importantPoints, pathsBetweenStations);
    auto startGA = std::chrono::high_resolution_clock::now();
    vector<vector<LocationID> > answer = MainAlgorithm(pathsBetweenStations, importantPoints, numOfUnits, hostageStations);
    if (answer.empty()) {
        PrintError("Error: Failed to creat an answer using the GA. Exiting.\n");
        getchar();
    }

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
    printf("Operation finished successfully, please press enter to finish the program");
    getchar();

    // Deallocate space
    DeallocateGrid(grid);
    DeallocateHostageStations(hostageStations, numOfSections);
}

// Insert all the important points into the array and resize it.
void FillImportantPoints(vector<pair<LocationID, Point>> &importantPoints, HostageStation **hostageStations, int numberStations,
                         Point unitsEntrance) {
    if (hostageStations == nullptr) {
        PrintError("Error: FillImportantPoints received null hostageStations.\n");
        return;
    }

    // Insert the units' starting position at the start.
    importantPoints.emplace_back(-1, unitsEntrance);

    // Iterate through the hostage stations and add their coordinates from index 1.
    for (int i = 0; i < numberStations; i++) {
        // Stor the station with valid locations and non-negative PValue
        if (hostageStations[i]->GetCoords() != Point(-1, -1) && hostageStations[i]->GetPValue() >= 0) {
            importantPoints.emplace_back(hostageStations[i]->GetSubgridAffiliation(),hostageStations[i]->GetCoords());
        }
    }
}

void RemoveUnreachablePoints(vector<pair<LocationID, Point>> &importantPoints, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (importantPoints.empty()) {
        return;
    }

    std::vector<std::pair<LocationID, Point>> reachablePoints;

    reachablePoints.push_back(importantPoints.at(0));
    LocationID unitsEntranceID = importantPoints.at(0).first;

    for (int i = 1; i < importantPoints.size(); i++) {
        int distance = GetPathCost(importantPoints[0].first, importantPoints[i].first, pathsBetweenStations);
        // Insert only reachable stations
        if (distance && distance <= UNIT_STEP_BUDGET) {
            reachablePoints.push_back(importantPoints.at(i));
        }
    }

    importantPoints.swap(reachablePoints);
}

void ShowPlan(const vector<vector<LocationID> > plan, HostageStation **hostageStations) {
    for (int u = 0; u < plan.size(); ++u) {
        UnitColor();
        printf("------Unit number #%d plan:------\n", u);
        if (plan[u].size() == 1) {
            ResetFG();
            printf("No stations assigned\n");
        } else {
            // Print info about each of the stations
            HostagesColor();
            for (int s = 1; s < plan[u].size(); ++s) {
                hostageStations[plan[u][s]]->PrintInfo();
            }
        }
    }

    ResetFG();

    printf("\nTo continue to the visualization proses, please press enter");
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
    printf("Stations that are next to be collected or were already collected will be marked with: ");
    NextStationColor();
    printf("              \n");
    ResetBG();

    printf("\nPlease press enter to start the visualization of the best plan the genetic algorithm found\n");
    getchar();
}

// Deallocate the memory used by an array of HostageStation objects and the array itself.
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
