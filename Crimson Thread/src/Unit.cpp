#include <utility>
#include "../include/Unit.h"
#include "../include/Utils.h"

bool isLegal(int x, int y, char **grid);

void Unit::setPath(vector<LocationID> &operationOrder, map<PathKey, vector<Point> > &pathsBetweenStations) {
    if (operationOrder.size() == 1) {
        finishedMission = true;
    } else {
        PathKey key;
        for (int i = 1; i < operationOrder.size(); ++i) {
            key = makeKey(operationOrder[i - 1], operationOrder[i]);
            vector<Point> &currentPath = pathsBetweenStations[key];
            if (operationOrder[i - 1] < operationOrder[i]) {
                // If the starting ID is less than the ending ID, traverse the path vector forwards
                for (int j = 1; j < currentPath.size(); ++j) {
                    path.push(currentPath[j]); // Push a copy of the element onto the queue
                }
            }
            else {
                // If the starting ID is greater than to the ending ID, traverse the path vector backwards
                for (int j = currentPath.size() - 1; j > 0; --j) {
                    path.push(currentPath[j]); // Push a copy of the element onto the queue
                }
            }
        }
    }
}

queue<Point> Unit::getPath() {
    return path;
}

Unit::Unit(Point entrance, vector<LocationID> &OperationOrder,
           map<PathKey, vector<Point> > &pathsBetweenStations): coords(entrance) {
    setPath(OperationOrder, pathsBetweenStations);
}

int Unit::getX() const { return coords.x; }
int Unit::getY() const { return coords.y; }
Point Unit::getCoords() const { return coords; }

void Unit::setCoords(Point newPos) {
    coords = newPos;
}

void Unit::move() {
    if (!path.empty()) {
        setCoords(path.front());
        path.pop();
    } else {
        finishedMission = true;
    }
}

bool Unit::isFinished() const {
    return finishedMission;
}
