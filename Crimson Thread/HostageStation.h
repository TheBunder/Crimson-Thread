#ifndef HOSTAGESTATION_H
#define HOSTAGESTATION_H

#include <stdio.h> // Included for printing

class HostageStation {
private:
    int x;
    int y;
    int subgridAffiliation;
    double hostageChance;
    int numHostages;
    double kidnapperChance;
    double soldierLossChance;

public:
    // Constructor
    HostageStation(int x, int y, int subgrid, double hostageChance, int numHostages,
        double kidnapperChance, double soldierLossChance);

    // Getters
    int getX() const;
    int getY() const;
    int getSubgridAffiliation() const;
    double getHostageChance() const;
    int getNumHostages() const;
    double getKidnapperChance() const;
    double getSoldierLossChance() const;

    // Print Info Method
    void printInfo() const;
};

#endif // HOSTAGESTATION_H