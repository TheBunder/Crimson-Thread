//----INCLUDES--------------------------------------------------------
#include <map>
#include <utility>
#include "include/BFS.h"
#include "include/Utils.h"

//----FUNCTIONS-------------------------------------------------------
Point **allocateParentGrid() {
    Point **grid = new Point *[GRID_WIDTH];
    for (int i = 0; i < GRID_WIDTH; i++) {
        grid[i] = new Point[GRID_HEIGHT];
    }
    return grid;
}

void deallocateParentGrid(Point ** grid) {
    for (int i = 0; i < GRID_WIDTH; i++) {
        delete grid[i];
    }
    delete grid;
}

bool CheckValidCell(int x, int y, char **grid) {
    //Check point is on grid and empty
    return IsInArrayBounds(x, y) && (grid[x][y] == State::kEmpty);
}

void AddToOpen(int x, int y, queue<Point> &open_Points, char **grid) {
    Point point{x, y};

    open_Points.push(point);
    grid[x][y] = State::kClosed;
}

void ExpandNeighbors(const Point current_point, queue<Point> &uncheckedPoints, char **grid, Point **parentGrid) {
    vector<Point> possibleMovements{{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

    for (Point movement: possibleMovements) {
        int x_test = current_point.x + movement.x;
        int y_test = current_point.y + movement.y;
        if (CheckValidCell(x_test, y_test, grid)) {
            // Store parent relationship before adding to open list
            parentGrid[x_test][y_test] = current_point;
            AddToOpen(x_test, y_test, uncheckedPoints, grid);
        }
    }
}

vector<Point> ReconstructPath(Point **parentGrid, Point start, Point goal) {
    vector<Point> path;
    Point current = {goal.x, goal.y};

    // Add goal to path
    path.push_back(current);

    // Work backwards from goal to start
    while (current != start) {
        current = parentGrid[current.x][current.y];
        path.push_back(current);
    }

    // Reverse to get path from start to goal
    std::reverse(path.begin(), path.end());

    return path;
}

void ReconstructPaths(Point **parentGrid, LocationID ID, Point start, Point *importantPoints, int importantPointsSize,
                      map<PathKey, vector<Point> > &pathsBetweenStations, mutex &pathMapMutex) {
    for (LocationID i = ID; i < importantPointsSize; i++) {
        vector<Point> path = ReconstructPath(parentGrid, start, importantPoints[i]);

        // Acquire the lock before accessing the shared map
        std::lock_guard<std::mutex> lock(pathMapMutex);

        // Insert the calculated path into the map
        pathsBetweenStations[{ID, i}] = path;

        // The lock is automatically released when it goes out of scope
    }
}

void Search(char **grid, Point start, Point **parentGrid) {
    queue<Point> uncheckedPoints{};

    int x = start.x;
    int y = start.y;
    AddToOpen(x, y, uncheckedPoints, grid);

    while (!uncheckedPoints.empty()) {
        Point currentPoint = uncheckedPoints.front();
        uncheckedPoints.pop();

        grid[currentPoint.x][currentPoint.y] = State::kSearched;

        ExpandNeighbors(currentPoint, uncheckedPoints, grid, parentGrid);
    }
}

void BFS(char **grid, LocationID ID, Point start, Point *importantPoints, int importantPointsSize,
         map<PathKey, vector<Point> > &pathsBetweenStations, mutex &pathMapMutex) {
    Point **parentGrid = allocateParentGrid();
    char **navGrid = allocateGrid();

    // Copy obstacle and empty cell information from the main grid to the navigation grid
    for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if ((unsigned char) grid[x][y] > 100) {
                navGrid[x][y] = kObstacle;
            } else {
                navGrid[x][y] = kEmpty;
            }
        }
    }
    Search(navGrid, start, parentGrid);

    ReconstructPaths(parentGrid, ID, start, importantPoints, importantPointsSize, pathsBetweenStations, pathMapMutex);

    deallocateGrid(navGrid);
    deallocateParentGrid(parentGrid);
}
