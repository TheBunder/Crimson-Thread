#ifndef UNIT_H
#define UNIT_H
#include "Utils.h"

class Unit {
private:
    Point coords; // To hold x and y coordinates
    // char standOn; // Which char he is standing on. saved to reput the char he runover.
    map<PathKey, vector<Point> > &pathsBetweenStations;
    vector<LocationID> &unitOperationOrder;
    int currentStepInPath = 0;
    PathKey currentPathKey;
    int currentStationIndex = 0;
    bool finishedMission = false;

public:
    Unit(Point entrance, vector<LocationID> unitOperationOrder, map<PathKey, vector<Point>> &pathsBetweenStations);

    int getX() const;
    int getY() const;
    Point getCoords() const;
    void setCoords(Point newPos);

    void move();

    int getCurrentStepInPath() const;
    void SetCurrentStepInPath(int current_step_in_path);

    PathKey GetCurrentPathKey() const;
    void SetCurrentPathKey(const PathKey &current_path_key);

    int GetCurrentStationIndex() const;
    void SetCurrentStationIndex(int current_station_index);

    bool isFinished();

    void swapUnits(Unit &other);
};
#endif // UNIT_H