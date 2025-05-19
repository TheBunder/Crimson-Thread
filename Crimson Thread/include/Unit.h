#ifndef UNIT_H
#define UNIT_H
#include <queue>
#include "Utils.h"

class Unit {
private:
    Point coords; // To hold x and y coordinates
    queue<Point> path{}; // The path the unit will take
    queue<Point> stationsCoords{}; // The location of each of the stations the unit will save
    bool finishedMission = false;
    char standOn[2];
    Point previousCoords;

    void SetStationsCoords(vector<LocationID> &operationOrder, map<PathKey, vector<Point> > &pathsBetweenStations);
    void SetPath(vector<LocationID> &operationOrder, map<PathKey, vector<Point> > &pathsBetweenStations);

public:
    queue<Point> GetPath();

    Unit(Point entrance, vector<LocationID> &OperationOrder,
               map<PathKey, vector<Point> > &pathsBetweenStations);

    int GetX() const;

    int GetY() const;

    Point GetCoords() const;

    void SetCoords(Point newPos);

    void Move(char **grid);

    bool IsFinished() const;

    char GetStoodOn() const;

    void SetStandOn(char standOn_);

    Point GetPreviousCoords() const;

    void SetPreviousCoords(Point previous_coords);
    void SetPreviousCoords(int x, int y);
};
#endif // UNIT_H
