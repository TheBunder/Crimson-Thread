#include "HostageStation.h"

HostageStation::HostageStation(int x, int y, int subgrid, double hostageChance, int numHostages,
    double kidnapperChance, double soldierLossChance) :
    subgridAffiliation(subgrid), hostageChance(hostageChance),
    numHostages(numHostages), kidnapperChance(kidnapperChance),
    soldierLossChance(soldierLossChance), coords(x, y) {}

int HostageStation::getX() const { return coords.x; }
int HostageStation::getY() const { return coords.y; }
Point HostageStation::getCoords() const { return coords; }
int HostageStation::getSubgridAffiliation() const { return subgridAffiliation; }
double HostageStation::getHostageChance() const { return hostageChance; }
int HostageStation::getNumHostages() const { return numHostages; }
double HostageStation::getKidnapperChance() const { return kidnapperChance; }
double HostageStation::getSoldierLossChance() const { return soldierLossChance; }

void HostageStation::printInfo() const {
    printf("Hostage Station Info:\n");
    printf("  X: %d, Y: %d\n", coords.x, coords.y);
    printf("  Subgrid: %d\n", subgridAffiliation);
    printf("  Hostage Chance: %.2f\n", hostageChance);
    printf("  Number of Hostages: %d\n", numHostages);
    printf("  Kidnapper Chance: %.2f\n", kidnapperChance);
    printf("  Soldier Loss Chance: %.2f\n", soldierLossChance);
    printf("--------------------------\n");
}