#include <stdio.h>
#include "include/HostageStation.h"

HostageStation::HostageStation(int x, int y, int subgrid, double hostageChance, int numHostages,
                               double kidnapperChance, double soldierLossChance) : subgridAffiliation(subgrid),
    hostageChance(hostageChance),
    numHostages(numHostages), kidnapperChance(kidnapperChance),
    soldierLossChance(soldierLossChance), coords(x, y) {
}

int HostageStation::GetX() const { return coords.x; }
int HostageStation::GetY() const { return coords.y; }
Point HostageStation::GetCoords() const { return coords; }
int HostageStation::GetSubgridAffiliation() const { return subgridAffiliation; }
double HostageStation::GetHostageChance() const { return hostageChance; }
int HostageStation::GetNumHostages() const { return numHostages; }
double HostageStation::GetKidnapperChance() const { return kidnapperChance; }
double HostageStation::GetSoldierLossChance() const { return soldierLossChance; }
double HostageStation::GetPValue() const { return numHostages * hostageChance - kidnapperChance * soldierLossChance; }

void HostageStation::PrintInfo() const {
    printf("Hostage Station Info:\n");
    printf("  X: %d, Y: %d\n", coords.x, coords.y);
    printf("  Subgrid: %d\n", subgridAffiliation);
    printf("  Hostage Chance: %.2f\n", hostageChance);
    printf("  Number of Hostages: %d\n", numHostages);
    printf("  Kidnapper Chance: %.2f\n", kidnapperChance);
    printf("  Soldier Loss Chance: %.2f\n", soldierLossChance);
    printf("  PValue: %.2f\n", GetPValue());
    printf("--------------------------\n");
}
