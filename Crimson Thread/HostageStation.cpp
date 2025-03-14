#include "HostageStation.h"

HostageStation::HostageStation(int x, int y, int subgrid, double hostageChance, int numHostages,
    double kidnapperChance, double soldierLossChance) :
    x(x), y(y), subgridAffiliation(subgrid), hostageChance(hostageChance),
    numHostages(numHostages), kidnapperChance(kidnapperChance),
    soldierLossChance(soldierLossChance) {}

int HostageStation::getX() const { return x; }
int HostageStation::getY() const { return y; }
int HostageStation::getSubgridAffiliation() const { return subgridAffiliation; }
double HostageStation::getHostageChance() const { return hostageChance; }
int HostageStation::getNumHostages() const { return numHostages; }
double HostageStation::getKidnapperChance() const { return kidnapperChance; }
double HostageStation::getSoldierLossChance() const { return soldierLossChance; }

void HostageStation::printInfo() const {
    printf("Hostage Station Info:\n");
    printf("  X: %d, Y: %d\n", x, y);
    printf("  Subgrid: %d\n", subgridAffiliation);
    printf("  Hostage Chance: %.2f\n", hostageChance);
    printf("  Number of Hostages: %d\n", numHostages);
    printf("  Kidnapper Chance: %.2f\n", kidnapperChance);
    printf("  Soldier Loss Chance: %.2f\n", soldierLossChance);
    printf("--------------------------\n");
}