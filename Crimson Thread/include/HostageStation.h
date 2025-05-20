#ifndef HOSTAGESTATION_H
#define HOSTAGESTATION_H

#include "Utils.h"

class HostageStation {
private:
    Point coords; // Hold x and y coordinates
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
    int GetX() const;
    int GetY() const;
    Point GetCoords() const;
    int GetSubgridAffiliation() const;
    double GetHostageChance() const;
    int GetNumHostages() const;
    double GetKidnapperChance() const;
    double GetSoldierLossChance() const;
    double GetPValue() const;

    // Print Info Method
    void PrintInfo() const;
};

#endif // HOSTAGESTATION_H