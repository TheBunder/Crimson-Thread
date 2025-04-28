#ifndef HOSTAGESTATION_H
#define HOSTAGESTATION_H

#include<array>
#include <stdio.h>
#include "Utils.h"

class HostageStation {
private:
    Point coords; // To hold x and y coordinates
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
    Point getCoords() const;
    int getSubgridAffiliation() const;
    double getHostageChance() const;
    int getNumHostages() const;
    double getKidnapperChance() const;
    double getSoldierLossChance() const;
    double getPValue() const;

    // Print Info Method
    void printInfo() const;
};

#endif // HOSTAGESTATION_H