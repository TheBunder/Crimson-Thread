//----INCLUDES--------------------------------------------------------
#include <stdlib.h>
#include <cstdio>
#include <set>
#include <algorithm>
# include "include/GeneticAlgorithm.h"
#include "include/ThreadPool.h"
#include "include/Visualizer.h"

//----FUNCTIONS-------------------------------------------------------
int GetPathCost(LocationID id1, LocationID id2, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    PathKey key = MakeKey(id1, id2);
    auto it = pathsBetweenStations.find(key);
    if (it == pathsBetweenStations.end() || it->second.empty()) {
        // Handle error: path not found or empty path (shouldn't happen if pre-calculation is complete)
        PrintWarning("Warning: Tried finding the path length between id1: %d id2: %d\n", id1, id2);
        return -1; // An indicator of error
    }
    // Cost is the number of steps, which is path length (number of cells) - 1
    return static_cast<int>(it->second.size()) - 1;
}

double SumPValue(const vector<vector<LocationID> > plan, HostageStation **hostageStations) {
    if (hostageStations == nullptr) {
        PrintError("Error: SumPValue received null hostageStations");
        return 0.0;
    }

    double sum = 0;
    for (int i = 0; i < plan.size(); i++) {
        for (int j = 1; j < plan[i].size(); j++) {
            if (plan[i][j] < 0) {
                PrintError("Error: SumPValue tried to visite HS with negative index");
            } else {
                // Skip the units starting position
                sum += hostageStations[plan[i][j]]->GetPValue(); // removed one to turn back to index
            }
        }
    }

    return sum;
}

int PathDistance(vector<LocationID> path, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (path.empty()) {
        PrintWarning("Warning: PathDistance received an empty path");
    }

    int pathLength = 0;

    // Sum the total distance between each Point in the unit path
    for (int s = 1; s < path.size(); ++s) {
        int segmentLength = GetPathCost(path[s - 1], path[s], pathsBetweenStations);
        if (segmentLength == -1) {
            PrintError("Error: PathDistance searched invalid path segment from %d to %d", s - 1, s);
            return -1;
        }
        pathLength += segmentLength;
    }

    return pathLength;
}

bool IsValidPath(vector<LocationID> &unitPath, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (unitPath.empty()) {
        return false;
    }
    // A set to store encountered LocationIDs
    std::set<LocationID> encounteredStations;
    int pathLength = 0;

    for (int s = 1; s < unitPath.size(); ++s) {
        if (!encounteredStations.insert(unitPath[s]).second) {
            // If insert second is false, it means the station was in the set before.
            return false;
        }

        // Sum path
        int segmentLength = GetPathCost(unitPath[s - 1], unitPath[s], pathsBetweenStations);
        if (segmentLength == -1) {
            PrintError("Error: IsValidPath searched invalid path segment from %d to %d", s - 1, s);
            return false;
        }
        pathLength += segmentLength;

        if (pathLength > UNIT_STEP_BUDGET) {
            return false; // The unit has passed the budget
        }
    }

    return true;
}

bool IsValidChromosome(Chromosome *chromosome, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (!chromosome) {
        PrintError("Error: IsValid received null chromosome\n");
        return false;
    }
    if (chromosome->unitPaths.size() != chromosome->unitSteps.size()) {
        PrintError("Error: IsValidChromosome received chromosome with mismatch unitPaths and unitSteps size");
    }

    // A set to store encountered LocationIDs
    std::set<LocationID> encounteredStations;
    int pathLength = 0;

    int i = 0;

    for (vector<LocationID> unitPath: chromosome->unitPaths) {
        for (int s = 1; s < unitPath.size(); ++s) {
            if (!encounteredStations.insert(unitPath[s]).second) {
                // If insert second is false, it means the station was in the set before.
                return false;
            }

            // Sum path
            int segmentLength = GetPathCost(unitPath[s - 1], unitPath[s], pathsBetweenStations);
            if (segmentLength == -1) {
                PrintError("Error: IsValidPath searched invalid path segment from %d to %d", s - 1, s);
                return false;
            }
            pathLength += segmentLength;

            if (pathLength > UNIT_STEP_BUDGET) {
                return false; // One of the units has passed the budget
            }
        }

        // Set unit path length
        chromosome->unitSteps[i++] = pathLength;

        // Reset the sum
        pathLength = 0;
    }

    return true;
}

Chromosome *GetFittestChromosome(Chromosome **chromosomeArray, HostageStation **hostageStations, int population) {
    if (!chromosomeArray || !hostageStations || population <= 0) {
        PrintError("Error: GetFittestChromosome received invalid parameters\n");
        return nullptr;
    }

    // Set the first as the best we found
    Chromosome *fittest = chromosomeArray[0];
    if (!fittest) {
        PrintError("Error: GetFittestChromosome first chromosome in array is null\n");
        return nullptr;
    }

    // Get the fitness of the best
    double fittestPValue = SumPValue(fittest->unitPaths, hostageStations);
    double chromosomePValue;
    for (int i = 0; i < population; ++i) {
        if (!chromosomeArray[i]) {
            PrintWarning("Warning: GetFittestChromosome received a null chromosome in index %d\n", i);
        } else {
            // Get the fittness of the chromosome we currently check
            chromosomePValue = SumPValue(chromosomeArray[i]->unitPaths, hostageStations);

            // Check if we found a better chromosome
            if (chromosomePValue > fittestPValue) {
                // Set the chromosome as the current best and save its fitness
                fittest = chromosomeArray[i];
                fittestPValue = chromosomePValue;
            }
        }
    }

    return fittest;
}

Chromosome *GetFittestChromosome(Chromosome **chromosomeArray, int population) {
    if (!chromosomeArray || population <= 0) {
        PrintError("Error: GetFittestChromosome received invalid parameters\n");
        return nullptr;
    }

    // Set the first as the best we found
    Chromosome *fittest = chromosomeArray[0];
    if (!fittest) {
        PrintError("Error: GetFittestChromosome first chromosome in array is null\n");
        return nullptr;
    }

    for (int i = 0; i < population; ++i) {
        if (!chromosomeArray[i]) {
            PrintWarning("Warning: GetFittestChromosome received a null chromosome in index %d\n", i);
        } else {
            // Check if we found a better chromosome
            if (chromosomeArray[i]->fitness > fittest->fitness) {
                // Set the chromosome as the current best
                fittest = chromosomeArray[i];
            }
        }
    }

    return fittest;
}

Chromosome *GetFittestChromosome(Chromosome **chromosomeArray) {
    return GetFittestChromosome(chromosomeArray, POPULATION_SIZE);
}

// Creat new empty chromosome
Chromosome *AllocateChromosome(int numOfUnits) {
    if (numOfUnits <= 0) {
        PrintError("Error: AllocateChromosome received nun-positive amount of units\n");
        return nullptr;
    }

    Chromosome *chromosome = nullptr;
    try {
        Chromosome *chromosome = new Chromosome;
        chromosome->unitPaths.resize(numOfUnits);
        chromosome->unitSteps.resize(numOfUnits, 0);
        chromosome->needsFitnessEvaluation = true;
        return chromosome;
    } catch (const std::bad_alloc &e) {
        PrintError("Error: AllocateChromosome received bad memory allocation\n");
        delete chromosome;
        return nullptr;
    }
}

// Allocate the full population of chromosomes
Chromosome **AllocateChromosomePopulation(int numOfUnits) {
    if (numOfUnits <= 0) {
        PrintError("Error: AllocateChromosome received nun-positive amount of units\n");
        return nullptr;
    }

    try {
        Chromosome **chromosomeArray = new Chromosome *[POPULATION_SIZE];
        for (int i = 0; i < POPULATION_SIZE; i++) {
            chromosomeArray[i] = AllocateChromosome(numOfUnits);
            if (chromosomeArray[i] == nullptr) {
                PrintError("Error: AllocateChromosome failed to allocate chromosome in the array\n");
                for (int j = 0; j < i; ++j) {
                    delete chromosomeArray[j];
                }
                delete[] chromosomeArray;

                return nullptr;
            }
        }
        return chromosomeArray;
    } catch (const std::bad_alloc &e) {
        PrintError("Error: AllocateChromosome received bad memory allocation at creating population array\n");
        return nullptr;
    }
}

// Deallocate the full population of chromosomes
void DeallocateChromosomePopulation(Chromosome **chromosomeArray) {
    if (chromosomeArray == nullptr) {
        PrintWarning("Warning: DeallocateChromosomePopulation received a null chromosomeArray\n");
        return;
    }

    for (int i = 0; i < POPULATION_SIZE; i++) {
        delete chromosomeArray[i];
    }
    delete[] chromosomeArray;
}

void InsertEntranceToPath(Chromosome *chromosome, int unit, const vector<pair<LocationID, Point> > &importantPoints) {
    if (chromosome == nullptr || importantPoints.empty()) {
        PrintError("Error: InsertEntranceToPath received invalid parameters\n");
        return;
    }

    if (unit >= chromosome->unitPaths.size() || unit < 0) {
        PrintError("Error: InsertEntranceToPath tried to insert station to invalid unit\n");
        return;
    }

    chromosome->unitPaths[unit].push_back(importantPoints.at(0).first);
}

void InsertStationToPath(Chromosome *chromosome, int unit, LocationID station,
                         const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (chromosome == nullptr || station < 0 || pathsBetweenStations.empty()) {
        PrintError("Error: InsertStationToPath received invalid parameters\n");
        return;
    }

    if (unit >= chromosome->unitPaths.size() || unit < 0) {
        PrintError("Error: InsertStationToPath tried to insert station to invalid unit\n");
        return;
    }

    if (chromosome->unitPaths[unit].empty()) {
        PrintError("Error: InsertStationToPath tried to insert HS to a path with not a set entrance\n");
    }

    int segmentLength = GetPathCost(chromosome->unitPaths[unit].back(), station, pathsBetweenStations);
    if (segmentLength == -1) {
        PrintError("Error: IsValidPath searched invalid path segment from %d to %d", chromosome->unitPaths[unit].back(),
                   station);
        return;
    }

    chromosome->unitSteps[unit] += segmentLength;
    chromosome->unitPaths[unit].push_back(station);
}

void ResetAvailable(vector<int> *availableStations, const vector<pair<LocationID, Point> > &importantPoints) {
    if (availableStations == nullptr) {
        PrintError("Error: ResetAvailable received null availableStations\n");
        return;
    }

    // Clear the trash elements that in the vector
    availableStations->clear();

    // Insert each of the IDs of the HS.
    for (int i = 1; i < importantPoints.size(); ++i) {
        availableStations->push_back(importantPoints.at(i).first);
    }
}

// Check if we can reach the Point within the budget limit.
bool IsReachable(Chromosome *chromosome, int unit, LocationID station,
                 const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (chromosome == nullptr) {
        PrintError("Error: IsReachable received null chromosome\n");
        return false;
    }
    if (unit >= chromosome->unitPaths.size() || unit < 0) {
        PrintError("Error: IsReachable received invalid unit\n");
        return false;
    }
    if (chromosome->unitPaths[unit].empty()) {
        PrintError("Error: IsReachable received a path with not a set entrance\n");
    }

    // Get current path length.
    int pathCost = GetPathCost(chromosome->unitPaths[unit].back(), station, pathsBetweenStations);

    // Check if getting to the Point will exceed the budget
    return (chromosome->unitSteps[unit] + pathCost) <= UNIT_STEP_BUDGET && pathCost != -1;
}

// Function to print unit paths
void PrintUnitPaths(const vector<vector<LocationID> > &unitPaths) {
    // Print a header for the unit paths section.
    printf("Unit Paths:\n");

    for (size_t u = 0; u < unitPaths.size(); ++u) {
        printf("Unit %zu: ", u);
        if (unitPaths[u].empty()) {
            printf("[]\n"); // Empty path
        } else {
            // Print the IDs of the unit
            for (int locationId: unitPaths[u]) {
                printf("%d ", locationId);
            }
            printf("\n");
        }
    }
}

// Function to print unit steps
void PrintUnitSteps(const vector<int> &unitSteps) {
    // Print unit steps
    printf("  Unit Steps:\n");
    for (size_t unitIndex = 0; unitIndex < unitSteps.size(); ++unitIndex) {
        printf("\tUnit %zu: %d\n", unitIndex, unitSteps[unitIndex]);
    }
}

// Function to print Chromosome information
void PrintChromosomeInfo(const Chromosome *chromosome, int chromosomeIndex) {
    if (chromosome == nullptr) {
        PrintError("Error: PrintChromosomeInfo received null chromosome\n");
        return;
    }

    printf("Chromosome %d:\n", chromosomeIndex); // Add an index for readability
    printf("Valid: %s\n", chromosome->isValid ? "true" : "false");
    printf("Fitness: %.2f\n", chromosome->fitness);

    // Print unit paths
    PrintUnitPaths(chromosome->unitPaths);

    // Print unit steps
    PrintUnitSteps(chromosome->unitSteps);
}

bool Initialization(Chromosome **chromosomeArray, const map<PathKey, vector<Point> > &pathsBetweenStations,
                    const vector<pair<LocationID, Point> > &importantPoints, int numOfUnits) {
    if (pathsBetweenStations.empty() || importantPoints.empty() || numOfUnits < 1) {
        PrintError("Error: Initialization received in valid input");
        return false;
    }

    // Initialize available stations
    vector<int> availableStations;

    for (int c = 0; c < POPULATION_SIZE; c++) {
        if (chromosomeArray[c] == nullptr) {
            PrintError("Error: Initialization received null chromosome at index: %d\n", c);
        }

        // Reset valid stations
        ResetAvailable(&availableStations, importantPoints);

        // Insert the station starting point as their first location
        for (int u = 0; u < numOfUnits; u++) {
            InsertEntranceToPath(chromosomeArray[c], u, importantPoints);
        }

        bool allStationsAssigned = false;

        // Add stations to each unit
        int numAttemptsToAdd = 20;
        for (int i = 0; i < numAttemptsToAdd && !allStationsAssigned; i++) {
            int u = rand() % numOfUnits;
            int randomIndex = rand() % availableStations.size();
            int randomStation = availableStations[randomIndex];
            if (IsReachable(chromosomeArray[c], u, randomStation, pathsBetweenStations)) {
                InsertStationToPath(chromosomeArray[c], u, randomStation, pathsBetweenStations);

                // Remove the station from the list of available once, using swap and pop
                swap(availableStations[randomIndex], availableStations.back());
                availableStations.pop_back();

                // Check if all stations were assigned
                if (availableStations.empty()) {
                    allStationsAssigned = true;
                }
            }
        }
    }

    return true;
}

void CalculateFitness(Chromosome *chromosome, const map<PathKey, vector<Point> > &pathsBetweenStations,
                      HostageStation **hostageStations) {
    if (chromosome == nullptr || hostageStations == nullptr) {
        PrintError("Error: CalculateFitness received null parameters\n");
        return;
    }

    // Check if valid chromosome
    bool valid = IsValidChromosome(chromosome, pathsBetweenStations);
    chromosome->isValid = valid;
    // If not valid set a penalty fitness
    if (!valid) {
        chromosome->fitness = -1;
    } else {
        // set the real fitness
        chromosome->fitness = SumPValue(chromosome->unitPaths, hostageStations);
    }

    // Mark the chromosome as evaluated
    chromosome->needsFitnessEvaluation = false;
}

void EvaluatePopulationFitness(Chromosome **chromosomeArray, const map<PathKey, vector<Point> > &pathsBetweenStations,
                               HostageStation **hostageStations, ThreadPool &pool) {
    if (chromosomeArray == nullptr || hostageStations == nullptr) {
        PrintError("Error: EvaluatePopulationFitness received null parameters\n");
        return;
    }

    // Iterate through each chromosome in the population.
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        if (chromosomeArray[i] == nullptr) {
            PrintWarning("Warning: EvaluatePopulationFitness recived null chromosme at index: %d", i);
        } else {
            // Check if the chromosome needs fitness evaluation.
            if (chromosomeArray[i]->needsFitnessEvaluation) {
                // Use the thread pool to calculate to multiple chromosome their fitness.
                pool.Enqueue([i, chromosomeArray, &pathsBetweenStations, hostageStations]() {
                    CalculateFitness(chromosomeArray[i], pathsBetweenStations, hostageStations);
                });
            }
        }
    }

    // Wait for all submitted tasks to complete.
    pool.WaitAll();
}

void Selection(Chromosome **chromosomeArray, Chromosome **matingPool) {
    if (chromosomeArray == nullptr || matingPool == nullptr) {
        PrintError("Error: Selection received null parameters\n");
        return;
    }

    // Creat an arena to preform the tournament
    Chromosome **arena = nullptr;
    try {
        arena = new Chromosome *[TOURNAMENT_SIZE];
    } catch (const std::bad_alloc &e) {
        PrintError("Error: Selection received bad memory allocation\n");
        return;
    }

    for (int i = 0; i < POPULATION_SIZE; ++i) {
        // Insert TOURNAMENT_SIZE random chromosomes into the arena
        for (int j = 0; j < TOURNAMENT_SIZE; ++j) {
            arena[j] = chromosomeArray[rand() % POPULATION_SIZE];
        }
        // Insert into the mating pool the fittest in the arena
        matingPool[i] = GetFittestChromosome(arena, TOURNAMENT_SIZE);

        if (matingPool[i] == nullptr) {
            PrintError("Error: Selection received null chromosome from tournament selection\n");
            delete[] arena;
            return;
        }
    }

    // Deallocate arena from memorey
    delete arena;
}

void Crossover(Chromosome **matingPool, Chromosome **nextGeneration, int numOfUnits) {
    if (!matingPool || !nextGeneration || numOfUnits < 1) {
        PrintError("Error: Crossover received invalid parameters\n");
        return;
    }
    for (int i = 0; i < POPULATION_SIZE - 1; i += 2) {
        // Save a pointer to two parent chromosomes
        Chromosome *parent1 = matingPool[i];
        Chromosome *parent2 = matingPool[i + 1];
        if (parent1 == nullptr || parent2 == nullptr) {
            PrintWarning("Error: Crossover received null chromosome in matingPool\n");
        } else {
            // Creat new chromosomes to represent their offsprings
            Chromosome *child1 = AllocateChromosome(numOfUnits);
            Chromosome *child2 = AllocateChromosome(numOfUnits);
            if (child1 == nullptr || child2 == nullptr) {
                PrintWarning("Error: Crossover received null chromosome during child allocation\n");
            } else {
                // Set the children paths and step count like their parents
                child1->unitPaths = parent1->unitPaths;
                child2->unitPaths = parent2->unitPaths;
                child1->unitSteps = parent1->unitSteps;
                child2->unitSteps = parent2->unitSteps;

                if (rand() % 100 < CROSSOVER_RATE) {
                    // Crossover occurs: Swap one paths' steps
                    int randUnitIndex = rand() % numOfUnits;

                    // Swap one unitPath and step count
                    child1->unitPaths[randUnitIndex] = parent2->unitPaths[randUnitIndex];
                    child1->unitSteps[randUnitIndex] = parent2->unitSteps[randUnitIndex];
                    child2->unitPaths[randUnitIndex] = parent1->unitPaths[randUnitIndex];
                    child2->unitSteps[randUnitIndex] = parent1->unitSteps[randUnitIndex];

                    // Mark for fitness recalculation
                    child1->needsFitnessEvaluation = true;
                    child2->needsFitnessEvaluation = true;
                } else {
                    // No crossover: Simply copy the parents' entire data
                    child1->needsFitnessEvaluation = parent1->needsFitnessEvaluation;
                    child2->needsFitnessEvaluation = parent2->needsFitnessEvaluation;
                    child1->isValid = parent1->isValid;
                    child2->isValid = parent2->isValid;
                    child1->fitness = parent1->fitness;
                    child2->fitness = parent2->fitness;
                }

                // Assign the newly created and populated children to the next generation
                nextGeneration[i] = child1;
                nextGeneration[i + 1] = child2;
            }
        }
    }
}

// Function to find a random LocationID not used in the chromosome's paths
LocationID FindRandomUnusedStation(const Chromosome *chromosome,
                                   const vector<pair<LocationID, Point> > &importantPoints) {
    if (chromosome == nullptr || importantPoints.empty()) {
        PrintError("Error: FindRandomUnusedStation received invalid parameters\n");
        return -1;
    }

    // Collect all used LocationIDs in the chromosome
    std::set<LocationID> usedStations;
    for (const vector<LocationID> &path: chromosome->unitPaths) {
        for (const LocationID stationID: path) {
            usedStations.insert(stationID);
        }
    }

    // Collect all unused LocationIDs from the list of all possible stations
    std::vector<LocationID> availableStations;
    for (int i = 1; i < importantPoints.size(); ++i) {
        // Check if the possible_station is NOT in the set of used_stations
        if (usedStations.find(importantPoints[i].first) == usedStations.end()) {
            // It's not in the used set, it's available
            availableStations.push_back(importantPoints[i].first);
        }
    }

    // Pick a random station from the available ones
    if (availableStations.empty()) {
        // No unused stations available in this chromosome
        return -1;
    }

    // Generate a random index within the bounds of the available_stations vector
    int randomIndex = rand() % availableStations.size();

    // Return the station at that random index
    return availableStations[randomIndex];
}

bool AddStationToRandomUnitPath(Chromosome *chromosome, const vector<pair<LocationID, Point> > &importantPoints,
                                int numOfUnits, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (chromosome == nullptr || importantPoints.empty() || numOfUnits < 1 || pathsBetweenStations.empty()) {
        PrintError("Error: AddStationToRandomUnitPath received invalid parameters\n");
        return false;
    }
    if (chromosome->unitPaths.size() < numOfUnits) {
        PrintError("Error: AddStationToRandomUnitPath received numOfUnits to large\n");
        return false;
    }
    // Chose a random unit.
    int randUnitIndex = rand() % numOfUnits;

    // Generate a random station ID that isn't assigned.
    int randomStation = FindRandomUnusedStation(chromosome, importantPoints);

    // Check if found and if so, is reachable.
    if (randomStation != -1 && IsReachable(chromosome, randUnitIndex, randomStation, pathsBetweenStations)) {
        InsertStationToPath(chromosome, randUnitIndex, randomStation, pathsBetweenStations);
        // Return that the chromosome was mutated
        return true;
    }

    // Return that the chromosome wasn't mutated due to budget constraints
    return false;
}

bool RemoveStationFromRandomUnitPath(Chromosome *chromosome, int numOfUnits) {
    if (chromosome == nullptr || numOfUnits < 1) {
        PrintError("Error: RemoveStationFromRandomUnitPath received invalid parameters\n");
    }
    if (chromosome->unitPaths.size() < numOfUnits) {
        PrintError("Error: RemoveStationFromRandomUnitPath received numOfUnits to large\n");
        return false;
    }
    // Chose a random unit.
    int randUnitIndex = rand() % numOfUnits;

    // Get a pointer to the unit assigned stations.
    vector<LocationID> &selectedPath = chromosome->unitPaths[randUnitIndex];
    int numberOfStops = selectedPath.size();

    // Check if the unit has assigned stations.
    if (numberOfStops > 1) {
        // Chose a random station and remove it from the plan.
        int randomStation = rand() % (numberOfStops - 1) + 1;
        selectedPath.erase(selectedPath.begin() + randomStation);

        // Return that the chromosome was mutated.
        return true;
    }

    // Return that the chromosome wasn't mutated due to not having eligible units.
    return false;
}

bool SwapStationFromRandomUnitPath(Chromosome *chromosome,
                                   int numOfUnits, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (chromosome == nullptr || numOfUnits < 1 || pathsBetweenStations.empty()) {
        PrintError("Error: SwapStationFromRandomUnitPath received invalid parameters\n");
    }
    if (chromosome->unitPaths.size() < numOfUnits) {
        PrintError("Error: SwapStationFromRandomUnitPath received numOfUnits to large\n");
        return false;
    }

    int randomIndex1, randomIndex2;

    // Find all eligible units (those with at least 2 stops to swap)
    vector<int> eligibleUnits;
    for (int i = 0; i < numOfUnits; i++) {
        if (chromosome->unitPaths[i].size() >= 3) {
            // Start + at least 2 stops
            eligibleUnits.push_back(i);
        }
    }

    // Need at least 1 eligible unit
    if (eligibleUnits.empty()) {
        return false;
    }

    // Select a random eligible unit
    int randUnitIndex = eligibleUnits[rand() % eligibleUnits.size()];
    vector<LocationID> &selectedPath = chromosome->unitPaths[randUnitIndex];

    int numberOfStops = selectedPath.size();

    // Generate 2 random stations.
    randomIndex1 = rand() % (numberOfStops - 1) + 1;
    do {
        randomIndex2 = rand() % (numberOfStops - 1) + 1;
    } while (randomIndex1 == randomIndex2);

    // Swap the order of arrival.
    swap(selectedPath[randomIndex1], selectedPath[randomIndex2]);

    // Check if the plan is executable under the step restriction.
    if (IsValidPath(selectedPath, pathsBetweenStations)) {
        // Return that the chromosome was mutated
        return true;
    }

    // If not in budget revert change
    swap(selectedPath[randomIndex1], selectedPath[randomIndex2]);

    // Return that the chromosome wasn't mutated due to budget constraints
    return false;
}

bool SwapStationBetweenRandomUnitsPath(Chromosome *chromosome,
                                       int numOfUnits, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (chromosome == nullptr || numOfUnits < 1 || pathsBetweenStations.empty()) {
        PrintError("Error: SwapStationBetweenRandomUnitsPath received invalid parameters\n");
    }
    if (chromosome->unitPaths.size() < numOfUnits) {
        PrintError("Error: SwapStationBetweenRandomUnitsPath received numOfUnits to large\n");
        return false;
    }

    // Find all eligible units (those with at least 2 stops)
    vector<int> eligibleUnits;
    for (int i = 0; i < numOfUnits; i++) {
        if (chromosome->unitPaths[i].size() >= 3) {
            // Start + at least 2 stops
            eligibleUnits.push_back(i);
        }
    }

    // Need at least 2 eligible units to perform a swap
    if (eligibleUnits.size() < 2) {
        return false;
    }

    // Select two different units randomly
    int randIndex1 = rand() % eligibleUnits.size();
    int randIndex2;
    do {
        randIndex2 = rand() % eligibleUnits.size();
    } while (randIndex1 == randIndex2);

    int randUnitIndex1 = eligibleUnits[randIndex1];
    int randUnitIndex2 = eligibleUnits[randIndex2];

    // Make a copy of their path plan
    vector<LocationID> testPath1 = chromosome->unitPaths[randUnitIndex1];
    vector<LocationID> testPath2 = chromosome->unitPaths[randUnitIndex2];

    // Save the length of the plan of each one of theme
    int numberOfStops1 = testPath1.size();
    int numberOfStops2 = testPath2.size();

    // Get random station positions (skip the first position which is the start)
    int randomIndex1 = rand() % (numberOfStops1 - 1) + 1;
    int randomIndex2 = rand() % (numberOfStops2 - 1) + 1;

    // Swap and check if in step budget range.
    swap(testPath1[randomIndex1], testPath2[randomIndex2]);

    if (IsValidPath(testPath1, pathsBetweenStations) && IsValidPath(testPath2, pathsBetweenStations)) {
        // If in budget, make the change on the real thing
        vector<LocationID> &selectedPath1 = chromosome->unitPaths[randUnitIndex1];
        vector<LocationID> &selectedPath2 = chromosome->unitPaths[randUnitIndex2];
        swap(selectedPath1[randomIndex1], selectedPath2[randomIndex2]);

        // Return that the chromosome was mutated
        return true;
    }

    // Return that the chromosome wasn't mutated due to budget constraints
    return false;
}

bool Mutate(Chromosome *chromosome, const vector<pair<LocationID, Point> > &importantPoints, int numOfUnits,
            const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (chromosome == nullptr || importantPoints.empty() || numOfUnits < 1 || pathsBetweenStations.empty()) {
        PrintError("Error: Mutate received invalid parameters\n");
        return false;
    }

    switch (rand() % 4) {
        // Choose mutation type
        case 0:
            return AddStationToRandomUnitPath(chromosome, importantPoints, numOfUnits,
                                              pathsBetweenStations);
        case 1:
            return RemoveStationFromRandomUnitPath(chromosome, numOfUnits);
        case 2:
            return SwapStationFromRandomUnitPath(chromosome, numOfUnits, pathsBetweenStations);
        case 3:
            return SwapStationBetweenRandomUnitsPath(chromosome, numOfUnits, pathsBetweenStations);
        default:
            return false;
    }
}

void Mutation(Chromosome **nextGeneration, const vector<pair<LocationID, Point> > &importantPoints, int numOfUnits,
              const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (nextGeneration == nullptr || importantPoints.empty() || numOfUnits < 1 || pathsBetweenStations.empty()) {
        PrintError("Error: Mutation received invalid parameters\n");
        return;
    }

    for (int i = 0; i < POPULATION_SIZE; ++i) {
        if (rand() % 100 < MUTATION_RATE) {
            if (nextGeneration[i] == nullptr) {
                PrintWarning("Warning: Mutation received a null chromosome in nextGeneration at index: %d\n", i);
            } else {
                // Mutate, and if any mutation type reported a change mark in chromosome
                if (Mutate(nextGeneration[i], importantPoints, numOfUnits,
                           pathsBetweenStations)) {
                    nextGeneration[i]->needsFitnessEvaluation = true; // Mark for re-evaluation
                }
            }
        }
    }
}

bool compareChromosomePtrsByFitnessDesc (Chromosome* a, Chromosome* b) {
    if (a == nullptr && b == nullptr) return false; // Equal if both null
    if (a == nullptr) return false; // b must be not null
    if (b == nullptr) return true;  // a must be not null

    return a->fitness > b->fitness; // For descending order (highest fitness first)
};

void PerformElitismAndReplacement(Chromosome **currentPopulation, Chromosome **offspringPopulation) {
    if (currentPopulation == nullptr || offspringPopulation == nullptr) {
        PrintError("Error: PerformElitismAndReplacement received invalid parameters\n");
        return;
    }

    // Partition current population: fittest elites at the start.
    std::nth_element(currentPopulation, currentPopulation + NUM_OF_ELITS, currentPopulation + POPULATION_SIZE,
                     compareChromosomePtrsByFitnessDesc);

    // Number of offspring needed to fill the rest of the next generation.
    int numOffspringToKeep = POPULATION_SIZE - NUM_OF_ELITS;

    // Partition offspring population: fittest to replace non-elites at the start.
    std::nth_element(offspringPopulation, offspringPopulation + numOffspringToKeep, offspringPopulation + POPULATION_SIZE,
                     compareChromosomePtrsByFitnessDesc);


    // Replace the non-elite chromosomes in currentPopulation with the selected offspring.
    for (int i = NUM_OF_ELITS; i < POPULATION_SIZE; ++i) {
        if (offspringPopulation[i] != nullptr) {
            delete currentPopulation[i]; // Delete the old, less fit chromosome.
        }
        currentPopulation[i] = offspringPopulation[i - NUM_OF_ELITS]; // Replace with a fitter offspring.
        offspringPopulation[i - NUM_OF_ELITS] = nullptr; // Nullify offspring pointer to prevent double deletion.
    }

    // Delete the offspring chromosomes that were not selected for the next generation.
    for (int i = numOffspringToKeep; i < POPULATION_SIZE; ++i) {
        // Safety check
        if (offspringPopulation[i] != nullptr) {
            delete offspringPopulation[i];
            offspringPopulation[i] = nullptr;
        }
    }
}

void OrderPathBruteForce(vector<LocationID> &path, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (path.empty()) {
        PrintWarning("Warning: OrderPathBruteForce received an empty path\n");
        return;
    }
    // If there are no other locations to visit, the path is just the entrance.
    if (path.size() == 1) {
        return;
    }
    if (pathsBetweenStations.empty()) {
        PrintError("Error: OrderPathBruteForce received an empty pathsBetweenStations");
    }

    vector<LocationID> stations;
    for (int i = 1; i < path.size(); ++i) {
        stations.push_back(path.at(i));
    }

    int minTotalDistance = PathDistance(path, pathsBetweenStations);
    if (minTotalDistance == -1) {
        PrintError("Error: OrderPathBruteForce origin path has distance -1");
        return;
    }

    // Need to sure to use next_permutation()
    sort(stations.begin(), stations.end());

    do {
        vector<LocationID> currentPath;
        currentPath.push_back(path.front()); // Always start at the units entrance
        for (LocationID id: stations) {
            currentPath.push_back(id); // Add current permutation
        }

        int currentTotalDistance = PathDistance(currentPath, pathsBetweenStations);

        // Update if found a better plan
        if (currentTotalDistance < minTotalDistance) {
            minTotalDistance = currentTotalDistance;
            path = currentPath;
        }
    } while (next_permutation(stations.begin(), stations.end())); // Run until we have covered all possible permutations
}


// Order a path in the shortest way in number of steps using Brute Force for each of the untis
void FindBestPathInPlanBruteForce(vector<vector<LocationID> > &fullPlan,
                                  const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (pathsBetweenStations.empty()) {
        PrintError("Error: pathsBetweenStations received an empty pathsBetweenStations");
    }
    for (vector<LocationID> &plan: fullPlan) {
        OrderPathBruteForce(plan, pathsBetweenStations);
    }
}

vector<vector<LocationID> > MainAlgorithm(const map<PathKey, vector<Point> > &pathsBetweenStations,
                                          const vector<pair<LocationID, Point> > &importantPoints,
                                          int numOfUnits, HostageStation **hostageStations) {
    if (pathsBetweenStations.empty() || importantPoints.empty() || numOfUnits < 1 || hostageStations == nullptr) {
        PrintError("Error: MainAlgorithm received in valid input");
        return vector<vector<LocationID> >();
    }

    // Create thread pool with hardware_concurrency threads
    ThreadPool pool(thread::hardware_concurrency());

    // Allocate memory for population
    Chromosome **currentPopulation = AllocateChromosomePopulation(numOfUnits);
    Chromosome **matingPool = (Chromosome **) malloc(sizeof(Chromosome *) * POPULATION_SIZE);
    Chromosome **offspringPopulation = (Chromosome **) malloc(sizeof(Chromosome *) * POPULATION_SIZE);
    if (currentPopulation == nullptr || matingPool == nullptr || offspringPopulation == nullptr) {
        PrintError("Error: MainAlgorithm couldn't allocate array of pointers to chromosomes.");
        return vector<vector<LocationID> >();
    }

    // Create and evaluate Generation 0
    bool GASucceed = Initialization(currentPopulation, pathsBetweenStations, importantPoints, numOfUnits);
    if (!GASucceed) {
        PrintError("Error: MainAlgorithm couldn't initialize chromosomes.");
        return vector<vector<LocationID> >();
    }

    EvaluatePopulationFitness(currentPopulation, pathsBetweenStations, hostageStations, pool);

    for (int G = 0; G < GENERATIONS; ++G) {
        // 1. Selection: Choose parents from currentPopulation based on fitness, fill matingPool
        Selection(currentPopulation, matingPool);

        // 2. Crossover: Create new offspring from matingPool.
        Crossover(matingPool, offspringPopulation, numOfUnits);

        // // 3. Mutation: Apply mutations to some of the newly created offspring (in offspringPopulation)
        Mutation(offspringPopulation, importantPoints, numOfUnits, pathsBetweenStations);

        // 4. Evaluate Fitness of New Offspring using the thread pool
        // Only evaluates offspring marked as needing evaluation by Crossover/Mutation.
        EvaluatePopulationFitness(offspringPopulation, pathsBetweenStations, hostageStations, pool);

        // 5. Creat the real next generation
        PerformElitismAndReplacement(currentPopulation, offspringPopulation);
    }

    vector<vector<LocationID> > bestPlan = GetFittestChromosome(currentPopulation)->unitPaths;

    // Deallocate population
    DeallocateChromosomePopulation(currentPopulation);
    free(matingPool);
    free(offspringPopulation);

    // Improve any imperfections in the order of actions.
    FindBestPathInPlanBruteForce(bestPlan, pathsBetweenStations);

    // Return best plan found
    return bestPlan;
}
