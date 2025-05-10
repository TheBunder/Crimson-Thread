#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H
//----INCLUDES--------------------------------------------------------
#include "Utils.h"
#include "HostageStation.h"

//----CONSTANTS------------------------------------------------------
const int UNIT_STEP_BUDGET = 180;
const int POPULATION_SIZE = 200;
const int GENERATIONS = 400;
const int TOURNAMENT_SIZE = 5;
const int CROSSOVER_RATE = 80; // In precents
const int MUTATION_RATE = 02;  // In precents
const int NUM_OF_ELITS = 1;  // Number of elit chromosomes

//----STRUCT------------------------------------------------------
struct Chromosome {
    vector<vector<LocationID>> unitPaths; // Outer vector size = num_units
    // Inner vector holds ordered station IDs for that unit
    vector<int> unitSteps; // Holds how much steps each unit takes in here current plan
    double fitness = 0.0; // Stores the calculated fitness (total PValue)
    bool isValid = false; // Flag indicating if constraints are met
    bool needsFitnessEvaluation  = true; // Flag indicating if we need to pass through fitness check
};

//----FUNCTION DECLARATIONS------------------------------------------
int GetPathCost(LocationID id1, LocationID id2, const map<PathKey, vector<Point> > &pathsBetweenStations); // Helper to get cost (length - 1), returns -1 or throws if path not found
double SumPValue(vector<vector<LocationID>> plan, HostageStation **HostageStations); // Get the total PValue from the plan
vector<vector<LocationID>> MainAlgorithm(const map<PathKey, vector<Point> > &pathsBetweenStations, int numOfHostageStations,
                    int numOfUnits, HostageStation **HostageStations); //initialize chromosome population
#endif //GENETIC_ALGORITHM_H
