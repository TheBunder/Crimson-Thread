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