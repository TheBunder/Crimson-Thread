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
    void setPath(vector<LocationID> &operationOrder, map<PathKey, vector<Point> > &pathsBetweenStations);

    queue<Point> getPath();

    Unit(Point entrance, vector<LocationID> &OperationOrder,
               map<PathKey, vector<Point> > &pathsBetweenStations);

    int getX() const;

    int getY() const;

    Point getCoords() const;

    void setCoords(Point newPos);

    void move();

    bool isFinished() const;
};
#endif // UNIT_H
