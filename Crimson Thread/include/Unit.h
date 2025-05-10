#ifndef UNIT_H
#define UNIT_H
#include <queue>
#include "Utils.h"

class Unit {
private:
    Point coords; // To hold x and y coordinates
    queue<Point> path{};
    bool finishedMission = false;

public:
    void SetPath(vector<LocationID> &operationOrder, map<PathKey, vector<Point> > &pathsBetweenStations);

    queue<Point> GetPath();

    Unit(Point entrance, vector<LocationID> &OperationOrder,
               map<PathKey, vector<Point> > &pathsBetweenStations);

    int GetX() const;

    int GetY() const;

    Point GetCoords() const;

    void SetCoords(Point newPos);

    void Move();

    bool IsFinished() const;
};
#endif // UNIT_H
