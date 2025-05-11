//----INCLUDES--------------------------------------------------------
#include <stdlib.h>
#include <cstdio>
#include <set>
# include "include/GeneticAlgorithm.h"
#include "include/ThreadPool.h"

//----FUNCTIONS-------------------------------------------------------
int GetPathCost(LocationID id1, LocationID id2, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    PathKey key = MakeKey(id1, id2);
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

double SumPValue(vector<vector<LocationID> > plan, HostageStation **HostageStations) {
    double sum = 0;
    for (int i = 0; i < plan.size(); i++) {
        for (int j = 1; j < plan[i].size(); j++) {
            // Skip the units starting position
            sum += HostageStations[plan[i][j] - 1]->GetPValue(); // removed one to turn back to index
        }
    }

    return sum;
}

int PathDistance(vector<LocationID> path, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    int pathLength = 0;

    // Sum the total distance between each Point in the unit path
    for (int s = 1; s < path.size(); ++s) {
        pathLength += GetPathCost(path[s - 1], path[s], pathsBetweenStations);
    }

    return pathLength;
}

bool IsValid(Chromosome *chromosome, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (!chromosome) {
        return false;
    }

    // A set to store encountered LocationIDs
    std::set<LocationID> encountered_stations;
    int pathLength = 0;

    int i = 0;

    for (vector<LocationID> unitPath: chromosome->unitPaths) {
        for (int s = 1; s < unitPath.size(); ++s) {
            // Sum path
            pathLength += GetPathCost(unitPath[s - 1], unitPath[s], pathsBetweenStations);

            if (pathLength > UNIT_STEP_BUDGET) {
                return false; // One of the units have passed the budget
            }
            if (!encountered_stations.insert(unitPath[s]).second) {
                // Insertion failed because the station_id was already in the set
                // This means we found a duplicate
                return false;
            }
        }

        // Set unit path length
        chromosome->unitSteps[i++] = pathLength;

        // Reset the sum
        pathLength = 0;
    }

    return true;
}

Chromosome *GetFittestChromosome(Chromosome **chromosomeArray, HostageStation **HostageStations, int population) {
    // Set the first as the best we found
    Chromosome *fittest = chromosomeArray[0];
    // Get the fitness of the best
    double fittestPValue = SumPValue(fittest->unitPaths, HostageStations);
    double chromosomePValue;
    for (int i = 0; i < population; ++i) {
        // Get the fittness of the chromosome we currently check
        chromosomePValue = SumPValue(chromosomeArray[i]->unitPaths, HostageStations);

        // Check if we found a better chromosome
        if (chromosomePValue > fittestPValue) {
            // Set the chromosome as the current best and save its fitness
            fittest = chromosomeArray[i];
            fittestPValue = chromosomePValue;
        }
    }

    return fittest;
}

Chromosome *GetFittestChromosome(Chromosome **chromosomeArray, int population) {
    // Set the first as the best we found
    Chromosome *fittest = chromosomeArray[0];
    for (int i = 0; i < population; ++i) {
        // Check if we found a better chromosome
        if (chromosomeArray[i]->fitness > fittest->fitness) {
            // Set the chromosome as the current best
            fittest = chromosomeArray[i];
        }
    }

    return fittest;
}

Chromosome *GetFittestChromosome(Chromosome **chromosomeArray) {
    return GetFittestChromosome(chromosomeArray, POPULATION_SIZE);
}

// Creat new empty chromosome
Chromosome *AllocateChromosome(int numOfUnits) {
    Chromosome *chromosome = new Chromosome;
    chromosome->unitPaths.resize(numOfUnits);
    chromosome->unitSteps.resize(numOfUnits, 0);
    chromosome->needsFitnessEvaluation  = true;
    return chromosome;
}

// Allocate the full population of chromosomes
Chromosome **AllocateChromosomePopulation(int numOfUnits) {
    Chromosome **chromosomeArray = new Chromosome *[POPULATION_SIZE];
    for (int i = 0; i < POPULATION_SIZE; i++) {
        chromosomeArray[i] = AllocateChromosome(numOfUnits);
    }
    return chromosomeArray;
}

// Deallocate the full population of chromosomes
void DeallocateChromosomePopulation(Chromosome **chromosomeArray) {
    for (int i = 0; i < POPULATION_SIZE; i++) {
        delete chromosomeArray[i];
    }
    delete[] chromosomeArray;
}

void InsertStationToPath(Chromosome *chromosome, int unit) {
    chromosome->unitPaths[unit].push_back(0);
}

void InsertStationToPath(Chromosome *chromosome, int unit, LocationID station,
                         const map<PathKey, vector<Point> > &pathsBetweenStations) {
    chromosome->unitSteps[unit] += GetPathCost(chromosome->unitPaths[unit].back(), station, pathsBetweenStations);
    chromosome->unitPaths[unit].push_back(station);
}

void ResetAvailable(vector<int> *availableStations, int numOfHostageStations) {
    // Clear the trash elements that in the vector
    availableStations->clear();

    // Insert each of the IDs of the station.
    for (int i = 0; i < numOfHostageStations; ++i) {
        availableStations->push_back(i + 1);
    }
}

// Check if we can reach the Point within the budget limit.
bool IsReachable(Chromosome *chromosome, int unit, LocationID station,
                 const map<PathKey, vector<Point> > &pathsBetweenStations) {
    // Get current path length.
    int pathCost = GetPathCost(chromosome->unitPaths[unit].back(), station, pathsBetweenStations);

    // Check if getting to the Point will exceed the budget
    return (chromosome->unitSteps[unit] + pathCost) <= UNIT_STEP_BUDGET && pathCost != -1;
}

// Function to print unit paths
void PrintUnitPaths(const vector<vector<LocationID>> &unitPaths) {
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
        printf("    Unit %zu: %d\n", unitIndex, unitSteps[unitIndex]);
    }
}

// Function to print Chromosome information
void PrintChromosomeInfo(const Chromosome *chromosome, int chromosomeIndex) {
    printf("Chromosome %d:\n", chromosomeIndex); // Add an index for readability
    printf("Valid: %s\n", chromosome->isValid ? "true" : "false");
    printf("Fitness: %.2f\n", chromosome->fitness);

    // Print unit paths
    PrintUnitPaths(chromosome->unitPaths);

    // Print unit steps
    PrintUnitSteps(chromosome->unitSteps);
}

void Initialization(Chromosome **chromosomeArray, const map<PathKey, vector<Point> > &pathsBetweenStations,
                    int numOfUnits,
                    int numOfHostageStations
                    ) {
    // Initialize available stations (0 to 16)
    vector<int> availableStations;

    for (int c = 0; c < POPULATION_SIZE; c++) {
        // Reset valid stations
        ResetAvailable(&availableStations, numOfHostageStations);

        // Insert the station starting point as their first location
        for (int u = 0; u < numOfUnits; u++) {
            InsertStationToPath(chromosomeArray[c], u);
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

                // Remove the station using swap and pop
                swap(availableStations[randomIndex], availableStations.back());
                availableStations.pop_back();

                // Check if all stesions were assigned
                if (availableStations.empty()) {
                    allStationsAssigned = true;
                }
            }
        }

        // PrintChromosomeInfo(chromosomeArray[c], c);
    }
}

void CalculateFitness(Chromosome *chromosome, const map<PathKey, vector<Point> > &pathsBetweenStations,
                    HostageStation **HostageStations) {
    // Check if valid chromosome
    bool valid = IsValid(chromosome, pathsBetweenStations);
    chromosome->isValid = valid;
    // If not valid set a penalty fitness
    if (!valid) {
        chromosome->fitness = -1;
    } else { // set the real fitness
        chromosome->fitness = SumPValue(chromosome->unitPaths, HostageStations);
    }

    // Mark the chromosome as evaluated
    chromosome->needsFitnessEvaluation = false;
}

void EvaluatePopulationFitness(Chromosome **chromosomeArray, const map<PathKey, vector<Point> > &pathsBetweenStations,
                                HostageStation **HostageStations, ThreadPool &pool) {
    // Iterate through each chromosome in the population.
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        // Check if the chromosome needs fitness evaluation.
        if (chromosomeArray[i]->needsFitnessEvaluation) {
            // // Submit a task to the thread pool to calculate fitness for this chromosome.
            // // Capture i by value and use references for the constant data
            // pool.Enqueue([i, chromosomeArray, &pathsBetweenStations, HostageStations]() {
            //     CalculateFitness(chromosomeArray[i], pathsBetweenStations, HostageStations);
            // });

            // Calculate fitness without threading
            CalculateFitness(chromosomeArray[i], pathsBetweenStations, HostageStations);
        }
    }

    // Wait for all submitted tasks to complete.
    pool.WaitAll();
}

void Selection(Chromosome **chromosomeArray, Chromosome **matingPool) {
    // Creat an arena to preform the tournament
    Chromosome **arena = new Chromosome *[TOURNAMENT_SIZE];
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        // Insert TOURNAMENT_SIZE random chromosomes into the arena
        for (int j = 0; j < TOURNAMENT_SIZE; ++j) {
            arena[j] = chromosomeArray[rand() % POPULATION_SIZE];
        }
        // Insert into the mating pool the fittest in the arena
        matingPool[i] = GetFittestChromosome(arena, TOURNAMENT_SIZE);
    }

    // Deallocate arena from memorey
    delete arena;
}

void Crossover(Chromosome **matingPool, Chromosome **nextGeneration, int numOfUnits) {
    for (int i = 0; i < POPULATION_SIZE - 1; i += 2) {
        // Save a pointer to two parent chromosomes
        Chromosome *parent1 = matingPool[i];
        Chromosome *parent2 = matingPool[i + 1];

        // Creat new chromosomes to represent their offsprings
        Chromosome *child1 = AllocateChromosome(numOfUnits);
        Chromosome *child2 = AllocateChromosome(numOfUnits);

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
            // No crossover: Simply copy the parents' entire path structure
            // Fitness is the same as parents', no recalculation needed
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

// Function to find a random LocationID not used in the chromosome's paths
LocationID FindRandomUnusedStation(const Chromosome *chromosome, const int numOfHostageStations) {
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

bool AddStationToRandomUnitPath(Chromosome *chromosome, int numOfUnits, int numOfHostageStations,
                                const map<PathKey, vector<Point> > &pathsBetweenStations) {
    // Chose a random unit.
    int randUnitIndex = rand() % numOfUnits;

    // Generate a random station ID that isn't assigned.
    int randomStation = FindRandomUnusedStation(chromosome, numOfHostageStations);

    // Check if found and if so, is reachable.
    if (randomStation != -1 && IsReachable(chromosome, randUnitIndex, randomStation, pathsBetweenStations)) {
        InsertStationToPath(chromosome, randUnitIndex, randomStation, pathsBetweenStations);
        // Return that the chromosome was mutated
        return  true;
    }

    // Return that the chromosome wasn't mutated due to budget constraints
    return false;
}

bool RemoveStationFromRandomUnitPath(Chromosome *chromosome,
                                     int numOfUnits, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    // Chose a random unit.
    int randUnitIndex = rand() % numOfUnits;

    // Get a pointer to the unit assigned stations.
    vector<LocationID> &selectedPath = chromosome->unitPaths[randUnitIndex];
    int numberOfStops = selectedPath.size();

    // Check if unit have assigned stations.
    if (numberOfStops > 1) {
        // Chose a random station and remove it from the plan.
        int randomStation = rand() % (numberOfStops - 1) + 1;
        selectedPath.erase(selectedPath.begin() + randomStation);

        // Reevaluate the unit path.
        chromosome->unitSteps[randUnitIndex] = PathDistance(selectedPath, pathsBetweenStations);

        // Return that the chromosome was mutated.
        return  true;
    }

    // Return that the chromosome wasn't mutated due to not having eligible units.
    return false;
}

bool SwapStationFromRandomUnitPath(Chromosome *chromosome,
                                   int numOfUnits, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    int randomIndex1, randomIndex2;

    // Find all eligible units (those with at least 2 stops to swap)
    vector<int> eligibleUnits;
    for (int i = 0; i < numOfUnits; i++) {
        if (chromosome->unitPaths[i].size() >= 3) { // Start + at least 2 stops
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
    int testPathDistance = PathDistance(selectedPath, pathsBetweenStations);
    if (testPathDistance < UNIT_STEP_BUDGET) {
        chromosome->unitSteps[randUnitIndex] = testPathDistance;

        // Return that the chromosome was mutated
        return  true;
    }

    // If not in budget
    // Revert change
    swap(selectedPath[randomIndex1], selectedPath[randomIndex2]);

    // Return that the chromosome wasn't mutated due to budget constraints
    return false;
}

bool SwapStationBetweenRandomUnitsPath(Chromosome *chromosome,
                                       int numOfUnits, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    // Find all eligible units (those with at least 2 stops)
    vector<int> eligibleUnits;
    for (int i = 0; i < numOfUnits; i++) {
        if (chromosome->unitPaths[i].size() >= 3) { // Start + at least 2 stops
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
    int testPathDistance1 = PathDistance(testPath1, pathsBetweenStations);
    int testPathDistance2 = PathDistance(testPath2, pathsBetweenStations);

    if (testPathDistance1 < UNIT_STEP_BUDGET && testPathDistance2 < UNIT_STEP_BUDGET) {
        // If in budget, make the change on the real thing
        vector<LocationID> &selectedPath1 = chromosome->unitPaths[randUnitIndex1];
        vector<LocationID> &selectedPath2 = chromosome->unitPaths[randUnitIndex2];
        swap(selectedPath1[randomIndex1], selectedPath2[randomIndex2]);
        chromosome->unitSteps[randUnitIndex1] = testPathDistance1;
        chromosome->unitSteps[randUnitIndex2] = testPathDistance2;

        // Return that the chromosome was mutated
        return  true;
    }

    // Return that the chromosome wasn't mutated due to budget constraints
    return false;
}

bool Mutate(Chromosome *chromosome, int numOfUnits, int numOfHostageStations,
              const map<PathKey, vector<Point> > &pathsBetweenStations) {
    switch (rand() % 4) { // Choose mutation type
        case 0:
            return AddStationToRandomUnitPath(chromosome, numOfUnits, numOfHostageStations,
                                       pathsBetweenStations);
        case 1:
            return RemoveStationFromRandomUnitPath(chromosome, numOfUnits, pathsBetweenStations);
        case 2:
            return SwapStationFromRandomUnitPath(chromosome, numOfUnits, pathsBetweenStations);
        case 3:
            return SwapStationBetweenRandomUnitsPath(chromosome, numOfUnits, pathsBetweenStations);
    }
}

void Mutation(Chromosome **nextGeneration, int numOfUnits, int numOfHostageStations,
              const map<PathKey, vector<Point> > &pathsBetweenStations) {
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        if (rand() % 100 < MUTATION_RATE) {

            if (Mutate(nextGeneration[i], numOfUnits, numOfHostageStations,
                                       pathsBetweenStations)) { // Mutate, and if any mutation type reported a change
                nextGeneration[i]->needsFitnessEvaluation = true; // Mark for re-evaluation
            }
        }
    }
}

int Partition(Chromosome** population, int low, int high) {
    // Choose the last element as the pivot
    Chromosome* pivot = population[high];

    // Index of smaller element
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        // If current element is greater than or equal to pivot (for descending order)
        if (population[j]->fitness > pivot->fitness) {
            i++; // increment index of smaller element
            swap(population[i], population[j]);
        }
    }

    swap(population[i + 1], population[high]);
    return (i + 1);
}

void QuickSelect(Chromosome** population, int low, int high, int k) {
    // If k is more than number of elements in array
    if (k > 0 && k <= high - low + 1) {
        // Partition the array around a pivot and get the pivot position
        int pi = Partition(population, low, high);

        // If pivot is the k-th largest element
        if (pi - low == k - 1) {
            return; // Found the k-th element
        }

        // If pivot is greater than k-th largest, search in the left sub-array
        if (pi - low > k - 1) {
            QuickSelect(population, low, pi - 1, k);
        }

        // If pivot is less than k-th largest, search in the right sub-array
        // The k-th element in the right sub-array is the (k - (pi - low + 1))-th element
        QuickSelect(population, pi + 1, high, k - (pi - low + 1));
    }
}

void PerformElitismAndReplacement(Chromosome **currentPopulation, Chromosome **offspringPopulation) {
    // Partition current population: fittest elites at the start.
    QuickSelect(currentPopulation, 0, POPULATION_SIZE - 1, NUM_OF_ELITS);
    // Partition offspring population: fittest to replace non-elites at the start.
    QuickSelect(offspringPopulation, 0, POPULATION_SIZE - 1, POPULATION_SIZE - NUM_OF_ELITS);

    // Number of offspring needed to fill the rest of the next generation.
    int numOffspringToKeep = POPULATION_SIZE - NUM_OF_ELITS;

    // Replace the non-elite chromosomes in currentPopulation with the selected offspring.
    for (int i = NUM_OF_ELITS; i < POPULATION_SIZE; ++i) {
        delete currentPopulation[i]; // Delete the old, less fit chromosome.
        currentPopulation[i] = offspringPopulation[i-NUM_OF_ELITS];  // Replace with a fitter offspring.
        offspringPopulation[i-NUM_OF_ELITS] = nullptr; // Nullify offspring pointer to prevent double deletion.
    }

    // Delete the offspring chromosomes that were not selected for the next generation.
    for (int i = numOffspringToKeep; i < POPULATION_SIZE; ++i) {
        if (offspringPopulation[i] != nullptr) { // Safety check
            delete offspringPopulation[i];
            offspringPopulation[i] = nullptr;
        }
    }
}

vector<vector<LocationID> > MainAlgorithm(const map<PathKey, vector<Point> > &pathsBetweenStations,
                                          int numOfUnits,
                                          int numOfHostageStations, HostageStation **HostageStations) {
    // Create thread pool with hardware_concurrency threads
    ThreadPool pool(thread::hardware_concurrency());

    // Allocate memory for population
    Chromosome **currentPopulation = AllocateChromosomePopulation(numOfUnits);
    Chromosome **matingPool = (Chromosome **) malloc(sizeof(Chromosome *) * POPULATION_SIZE);
    Chromosome **offspringPopulation = (Chromosome **) malloc(sizeof(Chromosome *) * POPULATION_SIZE);

    // Create and evaluate Generation 0
    Initialization(currentPopulation, pathsBetweenStations, numOfUnits, numOfHostageStations);
    EvaluatePopulationFitness(currentPopulation, pathsBetweenStations, HostageStations, pool);

    for (int G = 0; G < GENERATIONS; ++G) {
        // 1. Selection: Choose parents from currentPopulation based on fitness, fill matingPool
        Selection(currentPopulation, matingPool);

        // 2. Crossover: Create new offspring from matingPool.
        // Offspring chromosomes are newly allocated. Pointers stored in offspringPopulation.
        Crossover(matingPool, offspringPopulation, numOfUnits);

        // // 3. Mutation: Apply mutations to some of the newly created offspring (in offspringPopulation)
        Mutation(offspringPopulation, numOfUnits, numOfHostageStations, pathsBetweenStations);

        // 4. Evaluate Fitness of New Offspring using the thread pool
        // Only evaluates offspring marked as needing evaluation by Crossover/Mutation.
        EvaluatePopulationFitness(offspringPopulation, pathsBetweenStations, HostageStations, pool);

        // 5. Creat the real next generation
        PerformElitismAndReplacement(currentPopulation, offspringPopulation);
    }

    vector<vector<LocationID> > bestPlan = GetFittestChromosome(currentPopulation)->unitPaths;

    // Deallocate population
    DeallocateChromosomePopulation(currentPopulation);
    free(matingPool);
    free(offspringPopulation);

    // Return best plan found
    return bestPlan;
}