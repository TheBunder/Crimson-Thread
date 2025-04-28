//----INCLUDES--------------------------------------------------------
#include <stdlib.h>
# include "include/GeneticAlgorithm.h"

#include <cstdio>

//----FUNCTIONS-------------------------------------------------------
PathKey makeKey(LocationID id1, LocationID id2) {
    return std::make_pair(std::min(id1, id2), std::max(id1, id2));
}

int getPathCost(LocationID id1, LocationID id2, const map<PathKey, vector<Point> > &pathsBetweenStations) {
    PathKey key = makeKey(id1, id2);
    auto it = pathsBetweenStations.find(key);
    if (it == pathsBetweenStations.end() || it->second.empty()) {
        // Handle error: path not found or empty path (shouldn't happen if pre-calculation is complete)
        printf("\033[38;5;202m");
        printf("Opps!!! id1: %d id2: %d\n",id1,id2);
        printf("\033[0m");
        return -1; // Or some indicator of error
    }
    // Cost is the number of steps, which is path length (number of cells) - 1
    return static_cast<int>(it->second.size()) - 1;
}

double sumPValue(vector<vector<LocationID>> plan, HostageStation **HostageStations) {
    double sum = 0;
    for (int i = 0; i < plan.size(); i++) {
        for (int j = 1; j < plan[i].size(); j++) { // Skip the units starting position
            sum += HostageStations[plan[i][j] - 1]->getPValue(); // removed one to turn back to index
        }
    }

    return sum;
}

Chromosome *getFittestChromosome(Chromosome **chromosomeArray, HostageStation **HostageStations, int population) {
    Chromosome *fittest = chromosomeArray[0];
    double fittestPValue = sumPValue(fittest->unitPaths, HostageStations);
    double chromosomePValue;
    for (int i = 0; i < population; ++i) {
        chromosomePValue = sumPValue(chromosomeArray[i]->unitPaths, HostageStations);
        if (chromosomePValue >fittestPValue) {
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

Chromosome **allocateChromosome(int numOfUnits) {
    Chromosome **chromosomeArray = new Chromosome *[POPULATION_SIZE];
    for (int i = 0; i < POPULATION_SIZE; i++) {
        chromosomeArray[i] = new Chromosome;
        chromosomeArray[i]->unitPaths.resize(numOfUnits);
        chromosomeArray[i]->unitSteps.resize(numOfUnits, 0);
    }
    return chromosomeArray;
}

void deallocateChromosome(Chromosome **chromosomeArray) {
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

// Function to print Chromosome information using printf
void printChromosomeInfo(const Chromosome& chromosome, int chromosomeIndex) {
    printf("Chromosome %d:\n", chromosomeIndex); // Add an index for readability
    printf("Valid: %s\n", chromosome.isValid ? "true" : "false");
    printf("Fitness: %.2f\n", chromosome.fitness);

    // Print unit paths
    printf("Unit Paths:\n");
    for (size_t unitIndex = 0; unitIndex < chromosome.unitPaths.size(); ++unitIndex) {
        printf("Unit %zu: ", unitIndex);
        if (chromosome.unitPaths[unitIndex].empty()) {
            printf("[]\n"); // Empty path
        } else {
            for (int locationId : chromosome.unitPaths[unitIndex]) {
                printf("%d ", locationId);
            }
            printf("\n");
        }
    }

    // Print unit steps
    printf("  Unit Steps:\n");
    for (size_t unitIndex = 0; unitIndex < chromosome.unitSteps.size(); ++unitIndex) {
        printf("    Unit %zu: %d\n", unitIndex, chromosome.unitSteps[unitIndex]);
    }
}

void initialization(Chromosome **chromosomeArray, const map<PathKey, vector<Point> > &pathsBetweenStations, int numOfHostageStations,
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

vector<vector<LocationID>> mainAlgorithm(const map<PathKey, vector<Point> > &pathsBetweenStations, int numOfHostageStations,
                    int numOfUnits, HostageStation **HostageStations) {
    // Allocate population
    Chromosome **chromosomeArray = allocateChromosome(numOfUnits);

    initialization(chromosomeArray, pathsBetweenStations, numOfHostageStations, numOfUnits);

    vector<vector<LocationID>> bestPath = getFittestChromosome(chromosomeArray, HostageStations, POPULATION_SIZE)->unitPaths;

    // Deallocate population
    deallocateChromosome(chromosomeArray);

    return bestPath;
}
