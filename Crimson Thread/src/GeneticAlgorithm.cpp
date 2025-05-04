//----INCLUDES--------------------------------------------------------
#include <stdlib.h>
#include <cstdio>
#include <set>
# include "include/GeneticAlgorithm.h"


//----FUNCTIONS-------------------------------------------------------
int getPathCost(LocationID id1, LocationID id2, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    PathKey key = makeKey(id1, id2);
    auto it = pathsBetweenStations.find(key);
    if (it == pathsBetweenStations.end() || it->second.empty()) {
        // Handle error: path not found or empty path (shouldn't happen if pre-calculation is complete)
        printf("\033[38;5;202m");
        printf("Opps!!! id1: %d id2: %d\n", id1, id2);
        printf("\033[0m");
        return -1; // Or some indicator of error
    }
    // Cost is the number of steps, which is path length (number of cells) - 1
    return static_cast<int>(it->second.size()) - 1;
}

double sumPValue(vector<vector<LocationID> > plan, HostageStation **HostageStations) {
    double sum = 0;
    for (int i = 0; i < plan.size(); i++) {
        for (int j = 1; j < plan[i].size(); j++) {
            // Skip the units starting position
            sum += HostageStations[plan[i][j] - 1]->getPValue(); // removed one to turn back to index
        }
    }

    return sum;
}

int pathDistance(vector<LocationID> path, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    int pathLength = 0;

    for (int s = 1; s < path.size(); ++s) {
        pathLength += getPathCost(path[s - 1], path[s], pathsBetweenStations);
    }

    return pathLength;
}

bool isValid(Chromosome *chromosome, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (!chromosome) {
        return false;
    }

    // A set to store encountered LocationIDs
    std::set<LocationID> encountered_stations;
    int pathLength = 0;

    int i = 0;

    for (vector<LocationID> unitPath: chromosome->unitPaths) {
        for (int s = 1; s < unitPath.size(); ++s) {
            pathLength += getPathCost(unitPath[s - 1], unitPath[s], pathsBetweenStations);

            if (pathLength > UNIT_STEP_BUDGET) {
                return false; // One of the units have passed the budget
            }
            if (!encountered_stations.insert(unitPath[s]).second) {
                // Insertion failed because the station_id was already in the set
                // This means we found a duplicate
                return false;
            }
        }
        chromosome->unitSteps[i++] = pathLength;
        pathLength = 0;
    }

    return true;
}

Chromosome *getFittestChromosome(Chromosome **chromosomeArray, HostageStation **HostageStations, int population) {
    Chromosome *fittest = chromosomeArray[0];
    double fittestPValue = sumPValue(fittest->unitPaths, HostageStations);
    double chromosomePValue;
    for (int i = 0; i < population; ++i) {
        chromosomePValue = sumPValue(chromosomeArray[i]->unitPaths, HostageStations);
        if (chromosomePValue > fittestPValue) {
            fittest = chromosomeArray[i];
            fittestPValue = chromosomePValue;
        }
    }

    return fittest;
}

Chromosome *getFittestChromosome(Chromosome **chromosomeArray, int population) {
    Chromosome *fittest = chromosomeArray[0];
    for (int i = 0; i < population; ++i) {
        if (chromosomeArray[i]->fitness > fittest->fitness) {
            fittest = chromosomeArray[i];
        }
    }

    return fittest;
}

Chromosome *getFittestChromosome(Chromosome **chromosomeArray) {
    return getFittestChromosome(chromosomeArray, POPULATION_SIZE);
}

Chromosome *getListFittestChromosome(Chromosome **chromosomeArray) {
    Chromosome *lessFittest = chromosomeArray[0];
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        if (chromosomeArray[i]->fitness < lessFittest->fitness) {
            lessFittest = chromosomeArray[i];
        }
    }

    return lessFittest;
}

Chromosome *allocateChromosome(int numOfUnits) {
    Chromosome *chromosome = new Chromosome;
    chromosome->unitPaths.resize(numOfUnits);
    chromosome->unitSteps.resize(numOfUnits, 0);
    return chromosome;
}

Chromosome **allocateChromosomePopulation(int numOfUnits) {
    Chromosome **chromosomeArray = new Chromosome *[POPULATION_SIZE];
    for (int i = 0; i < POPULATION_SIZE; i++) {
        chromosomeArray[i] = allocateChromosome(numOfUnits);
    }
    return chromosomeArray;
}

void deallocateChromosomePopulation(Chromosome **chromosomeArray) {
    for (int i = 0; i < POPULATION_SIZE; i++) {
        delete chromosomeArray[i];
    }
    delete chromosomeArray;
}

void insertStationToPath(Chromosome *chromosome, int unit) {
    chromosome->unitPaths[unit].push_back(0);
}

void insertStationToPath(Chromosome *chromosome, int unit, LocationID station,
                         const map<PathKey, vector<Point> > &pathsBetweenStations) {
    chromosome->unitSteps[unit] += getPathCost(chromosome->unitPaths[unit].back(), station, pathsBetweenStations);
    chromosome->unitPaths[unit].push_back(station);
}

void resetAvailable(vector<int> *availableStations, int numOfHostageStations) {
    availableStations->clear();
    for (int i = 0; i < numOfHostageStations; ++i) {
        availableStations->push_back(i + 1);
    }
}

bool isReachable(Chromosome *chromosome, int unit, LocationID station,
                 const map<PathKey, vector<Point> > &pathsBetweenStations) {
    int pathCost = getPathCost(chromosome->unitPaths[unit].back(), station, pathsBetweenStations);
    return (chromosome->unitSteps[unit] + pathCost) <= UNIT_STEP_BUDGET && pathCost != -1;
}

// Function to print unit paths
void PrintUnitPaths(const vector<vector<LocationID>> &unitPaths) {
    printf("Unit Paths:\n");
    for (size_t unitIndex = 0; unitIndex < unitPaths.size(); ++unitIndex) {
        printf("Unit %zu: ", unitIndex);
        if (unitPaths[unitIndex].empty()) {
            printf("[]\n"); // Empty path
        } else {
            for (int locationId: unitPaths[unitIndex]) {
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
        printf("    Unit %zu: %d\n", unitIndex, unitSteps[unitIndex]);
    }
}

// Function to print Chromosome information
void printChromosomeInfo(const Chromosome &chromosome, int chromosomeIndex) {
    printf("Chromosome %d:\n", chromosomeIndex); // Add an index for readability
    printf("Valid: %s\n", chromosome.isValid ? "true" : "false");
    printf("Fitness: %.2f\n", chromosome.fitness);

    // Print unit paths
    PrintUnitPaths(chromosome.unitPaths);
    printf("Unit Paths:\n");
    for (size_t unitIndex = 0; unitIndex < chromosome.unitPaths.size(); ++unitIndex) {
        printf("Unit %zu: ", unitIndex);
        if (chromosome.unitPaths[unitIndex].empty()) {
            printf("[]\n"); // Empty path
        } else {
            for (int locationId: chromosome.unitPaths[unitIndex]) {
                printf("%d ", locationId);
            }
            printf("\n");
        }
    }

    // Print unit steps
    PrintUnitSteps(chromosome.unitSteps);
}

void Initialization(Chromosome **chromosomeArray, const map<PathKey, vector<Point> > &pathsBetweenStations,
                    int numOfHostageStations,
                    int numOfUnits) {
    // Initialize available stations (0 to 16)
    vector<int> availableStations;

    for (int c = 0; c < POPULATION_SIZE; c++) {
        // Reset valid stations
        resetAvailable(&availableStations, numOfHostageStations);

        // Insert the station starting point as their first location
        for (int u = 0; u < numOfUnits; u++) {
            insertStationToPath(chromosomeArray[c], u);
        }

        // Add stations to each unit
        int numAttemptsToAdd = 20;
        for (int i = 0; i < numAttemptsToAdd; i++) {
            int u = rand() % numOfUnits;
            int randomIndex = rand() % availableStations.size();
            int randomStation = availableStations[randomIndex];
            if (isReachable(chromosomeArray[c], u, randomStation, pathsBetweenStations)) {
                insertStationToPath(chromosomeArray[c], u, randomStation, pathsBetweenStations);

                // Remove the station using swap and pop
                swap(availableStations[randomIndex], availableStations.back());
                availableStations.pop_back();
            }
        }
    }
}

void CalculateFitness(Chromosome *chromosome, const map<PathKey, vector<Point> > &pathsBetweenStations,
                    HostageStation **HostageStations) {
    bool valid = isValid(chromosome, pathsBetweenStations);
    chromosome->isValid = valid;
    if (!valid) {
        chromosome->fitness = -1;
    } else {
        chromosome->fitness = sumPValue(chromosome->unitPaths, HostageStations);
    }
    chromosome->needsFitnessEvaluation = false;
}

void CalculatePopulationFitness(Chromosome **chromosomeArray, const map<PathKey, vector<Point> > &pathsBetweenStations,
                                HostageStation **HostageStations) {
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        if (chromosomeArray[i]->needsFitnessEvaluation) {
            CalculateFitness(chromosomeArray[i], pathsBetweenStations, HostageStations);
        }
    }
}

void Selection(Chromosome **chromosomeArray, Chromosome **matingPool) {
    Chromosome **arena = allocateChromosomePopulation(TOURNAMENT_SIZE);
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        for (int j = 0; j < TOURNAMENT_SIZE; ++j) {
            arena[j] = chromosomeArray[rand() % POPULATION_SIZE];
        }
        matingPool[i] = getFittestChromosome(arena, TOURNAMENT_SIZE);
    }
}

void Crossover(Chromosome **matingPool, Chromosome **nextGeneration, int numOfUnits) {
    for (int i = 0; i < POPULATION_SIZE; i += 2) {
        Chromosome *parent1 = matingPool[i];
        Chromosome *parent2 = matingPool[i + 1];

        Chromosome *child1 = allocateChromosome(numOfUnits);
        Chromosome *child2 = allocateChromosome(numOfUnits);

        if (rand() % 100 < CROSSOVER_RATE) {
            // Crossover occurs: Copy parents' paths, then swap one path
            child1->unitPaths = parent1->unitPaths;
            child2->unitPaths = parent2->unitPaths;

            // Crossover occurs: Copy parents' steps, then swap one paths' steps
            child1->unitSteps = parent1->unitSteps;
            child2->unitSteps = parent2->unitSteps;

            int randUnitIndex = rand() % numOfUnits;

            // Swap one unitPath
            child1->unitPaths[randUnitIndex] = parent2->unitPaths[randUnitIndex];
            child1->unitSteps[randUnitIndex] = parent2->unitSteps[randUnitIndex];
            child2->unitPaths[randUnitIndex] = parent1->unitPaths[randUnitIndex];
            child2->unitSteps[randUnitIndex] = parent1->unitSteps[randUnitIndex];

            // Mark for fitness recalculation
            child1->needsFitnessEvaluation = true;
            child2->needsFitnessEvaluation = true;
        } else {
            // No crossover: Simply copy the parents' entire path structure
            child1->unitPaths = parent1->unitPaths;
            child2->unitPaths = parent2->unitPaths;

            // Fitness is the same as parents', no recalculation needed
            child1->needsFitnessEvaluation = false;
            child2->needsFitnessEvaluation = false;
            child1->fitness = parent1->fitness;
            child2->fitness = parent2->fitness;
        }

        // Assign the newly created and populated children to the next generation
        // Ensure you handle memory if nextGeneration pointers were pre-existing and allocated
        // (e.g., delete nextGeneration[i] and nextGeneration[i+1] before assigning if needed)
        nextGeneration[i] = child1;
        nextGeneration[i + 1] = child2;
    }
    // After this function, nextGeneration should contain POPULATION_SIZE valid Chromosome* pointers,
    // pointing to newly allocated objects (or objects copied via elitism, if handled before/after).
    // These objects are now owned by whoever owns the nextGeneration array/vector.
}

// Function to find a random LocationID not used in the chromosome's paths
LocationID findRandomUnusedStation(const Chromosome *chromosome, const int numOfHostageStations) {
    if (!chromosome) {
        // Handle null chromosome pointer case
        return -1;
    }

    // 1. Collect all used LocationIDs in the chromosome
    std::set<LocationID> used_stations;
    for (const vector<LocationID> path: chromosome->unitPaths) {
        // Assuming path starts at index 0, include all stations in the path
        for (const LocationID station_id: path) {
            used_stations.insert(station_id);
        }
    }

    // 2. Collect all unused LocationIDs from the list of all possible stations
    std::vector<LocationID> available_stations;
    for (int station_id = 1; station_id <= numOfHostageStations; ++station_id) {
        // Check if the possible_station is NOT in the set of used_stations
        if (used_stations.find(station_id) == used_stations.end()) {
            // It's not in the used set, so it's available
            available_stations.push_back(station_id);
        }
    }

    // 3. Pick a random station from the available ones
    if (available_stations.empty()) {
        // No unused stations available in this chromosome
        return -1;
    } else {
        // Generate a random index within the bounds of the available_stations vector
        int randomIndex = rand() % available_stations.size();

        // Return the station at that random index
        return available_stations[randomIndex];
    }
}

void AddStationToRandomUnitPath(Chromosome *chromosome, int numOfUnits, int numOfHostageStations,
                                const map<PathKey, vector<Point> > &pathsBetweenStations) {
    int randUnitIndex = rand() % numOfUnits;
    int randomStation = findRandomUnusedStation(chromosome, numOfHostageStations);
    if (randomStation != -1 && isReachable(chromosome, randUnitIndex, randomStation, pathsBetweenStations)) {
        insertStationToPath(chromosome, randUnitIndex, randomStation, pathsBetweenStations);
    }
}

void RemoveStationFromRandomUnitPath(Chromosome *chromosome,
                                     int numOfUnits, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    int randUnitIndex = rand() % numOfUnits;
    vector<LocationID> &selectedPath = chromosome->unitPaths[randUnitIndex];
    int numberOfStops = selectedPath.size();
    if (numberOfStops) {
        int randomStation = rand() % numberOfStops;
        selectedPath.erase(selectedPath.begin() + randomStation);
        pathDistance(selectedPath, pathsBetweenStations);
    }
}

void SwapStationFromRandomUnitPath(Chromosome *chromosome,
                                   int numOfUnits, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    int randomIndex1, randomIndex2;
    int randUnitIndex = rand() % numOfUnits;
    vector<LocationID> testPath = chromosome->unitPaths[randUnitIndex];
    int numberOfStops = testPath.size();
    if (numberOfStops >= 3) {
        do {
            randomIndex1 = rand() % (numberOfStops - 1) + 1;
            randomIndex2 = rand() % (numberOfStops - 1) + 1;
        } while (randomIndex1 == randomIndex2);
        swap(testPath[randomIndex1], testPath[randomIndex2]);
        int testPathDistance = pathDistance(testPath, pathsBetweenStations);
        if (testPathDistance < UNIT_STEP_BUDGET) {
            vector<LocationID> &selectedPath = chromosome->unitPaths[randUnitIndex];
            swap(selectedPath[randomIndex1], selectedPath[randomIndex2]);
            chromosome->unitSteps[randUnitIndex] = testPathDistance;
            chromosome->needsFitnessEvaluation = true;
        }
    }
}

void SwapStationBetweenRandomUnitsPath(Chromosome *chromosome,
                                       int numOfUnits, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    int randUnitIndex1, randUnitIndex2;

    do {
        randUnitIndex1 = rand() % numOfUnits;
        randUnitIndex2 = rand() % numOfUnits;
    } while (randUnitIndex1 == randUnitIndex2);

    vector<LocationID> testPath1 = chromosome->unitPaths[randUnitIndex1];
    vector<LocationID> testPath2 = chromosome->unitPaths[randUnitIndex2];

    int numberOfStops1 = testPath1.size();
    int numberOfStops2 = testPath2.size();

    int randomIndex1 = rand() % (numberOfStops1 - 1) + 1;
    int randomIndex2 = rand() % (numberOfStops2 - 1) + 1;

    swap(testPath1[randomIndex1], testPath2[randomIndex2]);
    int testPathDistance1 = pathDistance(testPath1, pathsBetweenStations);
    int testPathDistance2 = pathDistance(testPath2, pathsBetweenStations);
    if (testPathDistance1 < UNIT_STEP_BUDGET && testPathDistance2 < UNIT_STEP_BUDGET) {
        vector<LocationID> &selectedPath1 = chromosome->unitPaths[randUnitIndex1];
        vector<LocationID> &selectedPath2 = chromosome->unitPaths[randUnitIndex2];
        swap(selectedPath1[randomIndex1], selectedPath2[randomIndex2]);
        chromosome->unitSteps[randUnitIndex1] = testPathDistance1;
        chromosome->unitSteps[randUnitIndex2] = testPathDistance2;
        chromosome->needsFitnessEvaluation = true;
    }
}

void Mutation(Chromosome **nextGeneration, int numOfUnits, int numOfHostageStations,
              const map<PathKey, vector<Point> > &pathsBetweenStations) {
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        if (rand() % 100 < MUTATION_RATE) {
            switch (rand() % 4) {
                case 0:
                    AddStationToRandomUnitPath(nextGeneration[i], numOfUnits, numOfHostageStations,
                                               pathsBetweenStations);
                    break;
                case 1:
                    RemoveStationFromRandomUnitPath(nextGeneration[i], numOfUnits, pathsBetweenStations);
                    break;
                case 2:
                    SwapStationFromRandomUnitPath(nextGeneration[i], numOfUnits, pathsBetweenStations);
                    break;
                case 3:
                    SwapStationBetweenRandomUnitsPath(nextGeneration[i], numOfUnits, pathsBetweenStations);
                    break;
            }
        }
    }
}

vector<vector<LocationID> > mainAlgorithm(const map<PathKey, vector<Point> > &pathsBetweenStations,
                                          int numOfUnits,
                                          int numOfHostageStations, HostageStation **HostageStations) {
    // Allocate memory for population
    Chromosome **currentPopulation = allocateChromosomePopulation(numOfUnits);
    Chromosome **matingPool = (Chromosome **) malloc(sizeof(Chromosome *) * POPULATION_SIZE);
    Chromosome **offspringPopulation = (Chromosome **) malloc(sizeof(Chromosome *) * POPULATION_SIZE);

    // Create and evaluate Generation 0
    Initialization(currentPopulation, pathsBetweenStations, numOfHostageStations, numOfUnits);
    CalculatePopulationFitness(currentPopulation, pathsBetweenStations, HostageStations);

    for (int G = 0; G < GENERATIONS; ++G) {
        // 1. Selection: Choose parents from currentPopulation based on fitness, fill matingPool
        Selection(currentPopulation, matingPool);

        // 2. Crossover: Create new offspring from matingPool.
        // Offspring chromosomes are newly allocated. Pointers stored in offspringPopulation.
        Crossover(matingPool, offspringPopulation, numOfUnits);

        // 3. Mutation: Apply mutations to some of the newly created offspring (in offspringPopulation)
        Mutation(offspringPopulation, numOfUnits, numOfHostageStations, pathsBetweenStations);

        // 4. Evaluate Fitness of New Offspring
        // Only evaluates offspring marked as needing evaluation by Crossover/Mutation.
        CalculatePopulationFitness(currentPopulation, pathsBetweenStations, HostageStations);
        currentPopulation[0] = getFittestChromosome(currentPopulation);
        Chromosome *ListFittest = getFittestChromosome(currentPopulation);

        for (int i = 1; i < POPULATION_SIZE; ++i) {
            if(offspringPopulation[i-1] != ListFittest) {
                currentPopulation[i] = offspringPopulation[i-1];
            }
        }

    }

    vector<vector<LocationID> > bestPlan = getFittestChromosome(currentPopulation)->unitPaths;

    // Deallocate population
    deallocateChromosomePopulation(currentPopulation);
    free(matingPool);
    free(offspringPopulation);
    return bestPlan;
}