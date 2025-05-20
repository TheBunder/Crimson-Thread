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

    bool Move(char **grid);

    bool IsFinished() const;

    Point GetPreviousCoords() const;

    void SetPreviousCoords(Point previous_coords);
    void SetPreviousCoords(int x, int y);

    Point GetNextStationCoords() const;
};
#endif // UNIT_H
