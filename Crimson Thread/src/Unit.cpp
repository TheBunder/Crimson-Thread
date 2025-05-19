#include <utility>
#include "../include/Unit.h"

#include <iostream>

#include "../include/Utils.h"

void Unit::SetPath(vector<LocationID> &operationOrder, map<PathKey, vector<Point> > &pathsBetweenStations) {
    // If the operation order contains only one location (just the starting point),
    // the mission is considered finished immediately.
    if (operationOrder.size() == 1) {
        finishedMission = true;
        return;
    }

    PathKey key;
    // Iterate through the operation order, considering pairs of adjacent location IDs
    for (int i = 1; i < operationOrder.size(); ++i) {
        // Create a key to look up the pre-calculated path segment between locations
        key = MakeKey(operationOrder[i - 1], operationOrder[i]);
        // Get a reference to the vector of points for this path segment
        vector<Point> &currentPath = pathsBetweenStations[key];

        // Determine the direction to traverse the path vector based on the order of IDs
        if (operationOrder[i - 1] < operationOrder[i]) {
            // Traverse path forwards,
            // For the first path segment, include the starting point
            int startIndex = (i == 1) ? 0 : 1;

            for (int j = startIndex; j < currentPath.size(); ++j) {
                path.push(currentPath[j]);
            }
        }
        else {
            // Traverse path backwards
            for (int j = currentPath.size() - 2; j >= 0; --j) {
                path.push(currentPath[j]);
            }
        }
    }
}

queue<Point> Unit::GetPath() {
    return path;
}

void Unit::SetStationsCoords(vector<LocationID> &operationOrder, map<PathKey, vector<Point> > &pathsBetweenStations) {
    PathKey key;
    // Iterate through the operation order, considering pairs of adjacent location IDs
    for (int i = 1; i < operationOrder.size(); ++i) {
        // Create a key to look up the pre-calculated path segment between locations
        key = MakeKey(operationOrder[i - 1], operationOrder[i]);
        // Get a reference to the vector of points for this path segment
        vector<Point> &currentPath = pathsBetweenStations[key];

        // Determine the direction to traverse the path vector based on the order of IDs
        if (operationOrder[i - 1] < operationOrder[i]) {
            // Our next station is at the end of the path
            // Note: we don't need to remember the entrance
            stationsCoords.push(currentPath[currentPath.size() - 1]);
        }
        else {
            // Because we traverse back, our next station is in the start
            stationsCoords.push(currentPath[0]);
        }
    }
}

Unit::Unit(Point entrance, vector<LocationID> &OperationOrder,
           map<PathKey, vector<Point> > &pathsBetweenStations): coords(entrance) {
    SetPath(OperationOrder, pathsBetweenStations);
    SetStationsCoords(OperationOrder, pathsBetweenStations);
    SetStandOn('E');
}

int Unit::GetX() const { return coords.x; }
int Unit::GetY() const { return coords.y; }
Point Unit::GetCoords() const { return coords; }

void Unit::SetCoords(Point newPos) {
    coords = newPos;
}

void Unit::Move(char** grid) {
    // Remember where the unit was to remove from the grid
    SetPreviousCoords(GetCoords());

    // Check if the unit finished her operation
    if (!path.empty()) {
        // Update the location of the unit
        Point newPos = path.front();
        if (newPos != stationsCoords.front()) {
            SetStandOn(grid[newPos.x][newPos.y]);
        }
        else {
            SetStandOn(PATH);
            stationsCoords.pop();
        }
        SetCoords(newPos);
        path.pop();
    } else {
        finishedMission = true;
    }
}

bool Unit::IsFinished() const {
    return finishedMission;
}

char Unit::GetStoodOn() const {
    return standOn[1];
}

void Unit::SetStandOn(char standOn_) {
    standOn[1] = standOn[0];
    standOn[0] = standOn_;
}

Point Unit::GetPreviousCoords() const {
    return previousCoords;
}

void Unit::SetPreviousCoords(Point previous_coords) {
    previousCoords = previous_coords;
}
void Unit::SetPreviousCoords(int x, int y) {
    previousCoords = Point(x, y);
}