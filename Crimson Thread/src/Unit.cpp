#include <utility>
#include "../include/Unit.h"
#include "../include/Utils.h"

bool IsLegal(int x, int y, char **grid);

void Unit::SetPath(vector<LocationID> &operationOrder, map<PathKey, vector<Point> > &pathsBetweenStations) {
    // If the operation order contains only one location (just the starting point),
    // the mission is considered finished immediately.
    if (operationOrder.size() == 1) {
        finishedMission = true;
    } else {
        PathKey key;
        // Iterate through the operation order, considering pairs of adjacent location IDs
        // to represent path segments (from operationOrder[i-1] to operationOrder[i]).
        for (int i = 1; i < operationOrder.size(); ++i) {
            // Create a key to look up the pre-calculated path segment between the two location IDs.
            key = MakeKey(operationOrder[i - 1], operationOrder[i]);
            // Get a reference to the vector of points for this path segment from the map.
            vector<Point> &currentPath = pathsBetweenStations[key];

            // Determine the direction to traverse the path vector based on the order of IDs in the key.
            if (operationOrder[i - 1] < operationOrder[i]) {
                // If the starting ID is less than the ending ID, traverse the path vector forwards
                for (int j = 1; j < currentPath.size(); ++j) {
                    path.push(currentPath[j]); // Push a copy of the element onto the queue
                }
            }
            else {
                // If the starting ID is greater than to the ending ID, traverse the path vector backwards
                for (int j = currentPath.size() - 1; j >= 0; --j) {
                    path.push(currentPath[j]); // Push a copy of the element onto the queue
                }
            }
        }
    }
}

queue<Point> Unit::GetPath() {
    return path;
}

Unit::Unit(Point entrance, vector<LocationID> &OperationOrder,
           map<PathKey, vector<Point> > &pathsBetweenStations): coords(entrance) {
    SetPath(OperationOrder, pathsBetweenStations);
}

int Unit::GetX() const { return coords.x; }
int Unit::GetY() const { return coords.y; }
Point Unit::GetCoords() const { return coords; }

void Unit::SetCoords(Point newPos) {
    coords = newPos;
}

void Unit::Move(char** grid) {
    if (!path.empty()) {
        Point newPos = path.front();
        SetStandOn(grid[newPos.x][newPos.y]);
        SetCoords(newPos);
        path.pop();
    } else {
        finishedMission = true;
    }
}

bool Unit::IsFinished() const {
    return finishedMission;
}

char Unit::GetStandOn() const {
    return standOn;
}

void Unit::SetStandOn(char stand_on) {
    standOn = stand_on;
}

Point Unit::GetPreviousCoords() const {
    return previousCoords;
}

void Unit::SetPreviousCoords(const Point &previous_coords) {
    previousCoords = previous_coords;
}
void Unit::SetPreviousCoords(int x, int y) {
    previousCoords = Point(x, y);
}