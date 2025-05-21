//----INCLUDES--------------------------------------------------------
#include <map>
#include <utility>
#include "include/BFS.h"
#include "include/Utils.h"

//----FUNCTIONS-------------------------------------------------------
Point **AllocateParentGrid() {
    Point **grid = new Point *[GRID_WIDTH];
    for (int i = 0; i < GRID_WIDTH; i++) {
        grid[i] = new Point[GRID_HEIGHT];
    }
    return grid;
}

void DeallocateParentGrid(Point ** grid) {
    for (int i = 0; i < GRID_WIDTH; i++) {
        delete grid[i];
    }
    delete grid;
}

bool CheckValidCell(int x, int y, char **grid) {
    //Check point is on grid and empty
    return IsInArrayBounds(x, y) && (grid[x][y] == State::kEmpty);
}

void AddToOpen(int x, int y, queue<Point> &openPoints, char **grid) {
    Point point{x, y};

    openPoints.push(point);
    grid[x][y] = kClosed;
}

void ExpandNeighbors(const Point currentPoint, queue<Point> &uncheckedPoints, char **grid, Point **parentGrid) {
    vector<Point> possibleMovements{{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

    for (Point movement: possibleMovements) {
        int xTest = currentPoint.x + movement.x;
        int yTest = currentPoint.y + movement.y;
        if (CheckValidCell(xTest, yTest, grid)) {
            // Store parent relationship before adding to open list
            parentGrid[xTest][yTest] = currentPoint;
            AddToOpen(xTest, yTest, uncheckedPoints, grid);
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

void ReconstructPaths(Point **parentGrid, LocationID startID, Point start, Point *importantPoints, int importantPointsSize,
                      map<PathKey, vector<Point> > &pathsBetweenStations, mutex &pathMapMutex) {
    for (LocationID i = startID; i < importantPointsSize; i++) {
        vector<Point> path = ReconstructPath(parentGrid, start, importantPoints[i]);

        // Acquire the lock before accessing the shared map
        std::lock_guard<std::mutex> lock(pathMapMutex);

        // Insert the calculated path into the map
        pathsBetweenStations[{startID, i}] = path;

        // The lock is automatically released when it goes out of scope
    }
}

void Search(char **grid, Point start, Point **parentGrid) {
    queue<Point> uncheckedPoints{};

    int x = start.x;
    int y = start.y;

    // Add the starting point to the queue and mark it as visited in the grid.
    AddToOpen(x, y, uncheckedPoints, grid);

    // Continue the search as long as there are points left in the queue.
    while (!uncheckedPoints.empty()) {
        // Get the next point from the front of the queue and remove it.
        Point currentPoint = uncheckedPoints.front();
        uncheckedPoints.pop();

        // Mark the current point in the grid as fully searched.
        grid[currentPoint.x][currentPoint.y] = State::kSearched;

        // Explore the neighbors of the current point.
        // Add valid, unvisited neighbors to the queue and record the current point as their parent.
        ExpandNeighbors(currentPoint, uncheckedPoints, grid, parentGrid);
    }
}

void BFS(char **grid, LocationID startID, Point start, Point *importantPoints, int importantPointsSize,
         map<PathKey, vector<Point> > &pathsBetweenStations, mutex &pathMapMutex) {
    // Allocate memory for the parent grid, used to reconstruct paths after the search.
    Point **parentGrid = AllocateParentGrid();
    // Allocate memory for a navigation grid, used by the search algorithm.
    char **navGrid = AllocateGrid();

    // Prepare the navigation grid by copying obstacle and empty cell information
    // from the main grid. The BFS will operate on this navGrid.
    for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            // Check if wall
            if ((unsigned char) grid[x][y] > 100) {
                navGrid[x][y] = kObstacle;
            } else {
                navGrid[x][y] = kEmpty;
            }
        }
    }

    // Perform the actual Breadth-First Search starting from the 'start' point
    // on the prepared navigation grid. Populates the parentGrid for path reconstruction.
    Search(navGrid, start, parentGrid);

    // Reconstruct paths from the 'start' point (identified by 'ID') to all points in the 'importantPoints' array.
    ReconstructPaths(parentGrid, startID, start, importantPoints, importantPointsSize, pathsBetweenStations, pathMapMutex);

    // Deallocate the memory used by the temporary navigation grid.
    DeallocateGrid(navGrid);
    // Deallocate the memory used by the parent grid.
    DeallocateParentGrid(parentGrid);
}
