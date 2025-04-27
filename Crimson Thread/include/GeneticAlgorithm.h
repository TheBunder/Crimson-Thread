#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H
//----INCLUDES--------------------------------------------------------
#include "Utils.h"

//----CONSTANTS------------------------------------------------------
const int UNIT_STEP_BUDGET = 180;
const int POPULATION_SIZE = 200;

//----STRUCT------------------------------------------------------
struct Chromosome {
    vector<vector<LocationID>> unitPaths; // Outer vector size = num_units
    // Inner vector holds ordered station IDs for that unit
    vector<int> unitSteps; // Holds how much steps each unit takes in here current plan
    double fitness = 0.0; // Stores the calculated fitness (total PValue)
    bool isValid = false; // Flag indicating if constraints are met
};

//----FUNCTION DECLARATIONS------------------------------------------
PathKey makeKey(LocationID id1, LocationID id2); // Function to ensure consistent key ordering
int getPathCost(LocationID id1, LocationID id2, const map<PathKey, vector<Point> > &pathsBetweenStations); // Helper to get cost (length - 1), returns -1 or throws if path not found
void initialization(const map<PathKey, vector<Point> > &pathsBetweenStations, int numOfHostageStations, int numOfUnits); //initialize chromosome population
#endif //GENETIC_ALGORITHM_H
