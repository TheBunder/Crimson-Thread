#ifndef UNIT_H
#define UNIT_H
#include <queue>
#include "Utils.h"

class Unit {
private:
    Point coords; // To hold x and y coordinates
    queue<Point> path{};
    bool finishedMission = false;
    char standOn;
    Point previousCoords;

public:
    void SetPath(vector<LocationID> &operationOrder, map<PathKey, vector<Point> > &pathsBetweenStations);

    queue<Point> GetPath();

    Unit(Point entrance, vector<LocationID> &OperationOrder,
               map<PathKey, vector<Point> > &pathsBetweenStations);

    int GetX() const;

    int GetY() const;

    Point GetCoords() const;

    void SetCoords(Point newPos);

    void Move(char **grid);

    bool IsFinished() const;

    char GetStandOn() const;

    void SetStandOn(char stand_on);

    Point GetPreviousCoords() const;

    void SetPreviousCoords(const Point &previous_coords);
    void SetPreviousCoords(int x, int y);
};
#endif // UNIT_H
