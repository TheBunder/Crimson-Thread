#include <utility>
#include "../include/Unit.h"
#include "../include/Utils.h"

bool isLegal(int x, int y, char **grid);

Unit::Unit(Point entrance, vector<LocationID> unitOperationOrder,
           map<PathKey, vector<Point> > &pathsBetweenStations): coords(entrance),
                                                                pathsBetweenStations(pathsBetweenStations),
                                                                unitOperationOrder(unitOperationOrder) {
    if (unitOperationOrder.size() >= 2) {
        currentPathKey = make_pair(unitOperationOrder[0], unitOperationOrder[1]);
    }
    else {
        finishedMission = true;
    }
}

int Unit::getX() const { return coords.x; }
int Unit::getY() const { return coords.y; }
Point Unit::getCoords() const { return coords; }

void Unit::setCoords(Point newPos) {
    coords = newPos;
}

void Unit::move() {
    if (currentStepInPath < pathsBetweenStations[currentPathKey].size()) {
        setCoords(pathsBetweenStations[currentPathKey][currentStepInPath]);
        currentStepInPath++;
    } else {
        if (currentStationIndex + 1 < unitOperationOrder.size()) {
            currentStepInPath = 1;
            currentStationIndex++;
            currentPathKey = makeKey(unitOperationOrder[currentStationIndex], unitOperationOrder[currentStationIndex + 1]);
            move();
        }
        else {
            finishedMission = true;
        }
    }
}

int Unit::getCurrentStepInPath() const {
    return currentStepInPath;
}

void Unit::SetCurrentStepInPath(int current_step_in_path) {
    currentStepInPath = current_step_in_path;
}

PathKey Unit::GetCurrentPathKey() const {
    return currentPathKey;
}

void Unit::SetCurrentPathKey(const PathKey &current_path_key) {
    currentPathKey = current_path_key;
}

int Unit::GetCurrentStationIndex() const {
    return currentStationIndex;
}

void Unit::SetCurrentStationIndex(int current_station_index) {
    currentStationIndex = current_station_index;
}

bool Unit::isFinished() {
    return finishedMission;
}

void Unit::swapUnits(Unit& other)
{
    using std::swap;
    swap(coords, other.coords);
    swap(currentStepInPath, other.currentStepInPath);
    swap(currentPathKey, other.currentPathKey);
    swap(currentStationIndex, other.currentStationIndex);
    swap(finishedMission, other.finishedMission);
}