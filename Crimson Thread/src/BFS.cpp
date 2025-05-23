//----INCLUDES--------------------------------------------------------
#include <map>
#include <utility>
#include "include/BFS.h"
#include "include/Utils.h"
#include "include/Visualizer.h"

//----FUNCTIONS-------------------------------------------------------
Point **AllocateParentGrid() {
    try {
        Point **grid = new Point *[GRID_WIDTH];
        for (int i = 0; i < GRID_WIDTH; i++) {
            grid[i] = new Point[GRID_HEIGHT];
        }
        return grid;
    }catch (const std::bad_alloc &e) {
        PrintError("Error: Failed to allocate parentGrid memory.\n");
        return nullptr;
    }
}

void DeallocateParentGrid(Point ** grid) {
    if (grid == nullptr) {
        return;
    }

    for (int i = 0; i < GRID_WIDTH; i++) {
        delete grid[i];
    }
    delete grid;
}

bool CheckValidCell(int x, int y, char **grid) {
    if (grid == nullptr) {
        PrintError("Error: CheckValidCell received a null grid.\n");
        return false;
    }

    //Check point is on grid and empty
    return IsInArrayBounds(x, y) && (grid[x][y] == State::kEmpty);
}

void AddToOpen(int x, int y, queue<Point> &openPoints, char **grid) {
    if (grid == nullptr) {
        PrintError("Error: AddToOpen received a null grid.\n");
        return;
    }
    if (!IsInArrayBounds(x,y)) {
        PrintError("Error: AddToOpen received an out-of-bound point.\n");
        return;
    }

    Point point{x, y};

    openPoints.push(point);
    grid[x][y] = kClosed;
}

void ExpandNeighbors(const Point currentPoint, queue<Point> &uncheckedPoints, char **grid, Point **parentGrid) {
    if (grid == nullptr) {
        PrintError("Error: ExpandNeighbors received a null grid.\n");
        return;
    }
    if (parentGrid == nullptr) {
        PrintError("Error: ExpandNeighbors received a null parentGrid.\n");
        return;
    }
    if (!IsInArrayBounds(currentPoint)) {
        PrintWarning("Warning: Search received an out-of-bound point.\n");
        return;
    }

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

    if (parentGrid == nullptr) {
        PrintError("Error: ReconstructPaths received a null parentGrid.\n");
        return path;
    }
    if (!IsInArrayBounds(start) || !IsInArrayBounds(goal)) {
        PrintWarning("Warning: ReconstructPath received an out-of-bound start or goal.\n");
        return path;
    }

    if (parentGrid[goal.x][goal.y] == Point(-1, -1)) {
        //received an unreachable goal.
        return path;
    }

    Point current = {goal.x, goal.y};

    // Add goal to the path
    path.push_back(current);

    int maxAttempts = GRID_WIDTH * GRID_HEIGHT;

    // Work backwards from goal to start
    while (current != start && maxAttempts) {
        if (IsInArrayBounds(current)) {
            current = parentGrid[current.x][current.y];
            path.push_back(current);
        }
        else {
            PrintError("Error: ReconstructPath received an out-of-bound location from parentGrid.\n");
        }
        --maxAttempts;
    }

    if (!maxAttempts) {
        PrintError("Error: ReconstructPaths couldn't reach to start from goal.A defective parent grid was provided.\n");
        return vector<Point>();
    }

    // Reverse to get the path from start to goal
    std::reverse(path.begin(), path.end());

    return path;
}

void ReconstructPaths(Point **parentGrid, LocationID startID, Point start, vector<pair<LocationID, Point>> importantPoints,
                      map<PathKey, vector<Point> > &pathsBetweenStations, mutex &pathMapMutex) {
    if (parentGrid == nullptr) {
        PrintError("Error: ReconstructPaths received a null parentGrid.\n");
        return;
    }
    if (!importantPoints.size()) {
        PrintError("Error: ReconstructPaths received empty importantPoints.\n");
        return;
    }

    for (int i = 0; i < importantPoints.size(); i++) {
        if (startID < importantPoints.at(i).first) {
            vector<Point> path = ReconstructPath(parentGrid, start, importantPoints[i].second);

            // Activate lock before using the shared map
            std::lock_guard<std::mutex> lock(pathMapMutex);

            // Insert the calculated path into the map
            pathsBetweenStations[{startID, importantPoints[i].first}] = path;

            // The lock is automatically released when it goes out of scope
        }
    }
}

void Search(char **grid, Point start, Point **parentGrid) {
    if (grid == nullptr) {
        PrintError("Error: Search received a null grid.\n");
        return;
    }
    if (parentGrid == nullptr) {
        PrintError("Error: Search received a null parentGrid.\n");
        return;
    }
    if (!IsInArrayBounds(start)) {
        PrintError("Error: Search received an out-of-bound start point.\n");
        return;
    }

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

        // Insert unvisited neighbors to queue
        ExpandNeighbors(currentPoint, uncheckedPoints, grid, parentGrid);
    }
}

void BFS(char **grid, LocationID startID, Point start, vector<pair<LocationID, Point>> importantPoints,
         map<PathKey, vector<Point> > &pathsBetweenStations, mutex &pathMapMutex) {
    if (grid == nullptr) {
        PrintError("Error: BFS received a null grid.\n");
        return;
    }
    if (!importantPoints.size()) {
        PrintError("Error: BFS received empty importantPoints.\n");
        return;
    }
    if (!IsInArrayBounds(start)) {
        PrintError("Error: BFS received an out-of-bound start point.\n");
        return;
    }

    // Allocate memory for the parent grid, used to reconstruct paths after the search.
    Point **parentGrid = AllocateParentGrid();
    if (parentGrid == nullptr) {
        PrintError("Error: Failed to allocate parentGrid for BFS.\n");
        return;
    }

    // Allocate memory for a navigation grid, used by the search algorithm.
    char **navGrid = AllocateGrid();
    if (navGrid == nullptr) {
        PrintError("Error: Failed to allocate navGrid for BFS.\n");
        DeallocateParentGrid(parentGrid);
        return;
    }

    // Make the navGrid by separation between walkable and unwalkable tiles
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

    // Execute BFS search from start and fill parentGrid
    Search(navGrid, start, parentGrid);

    // Reconstruct paths from start to all important points with higher ID.
    ReconstructPaths(parentGrid, startID, start, importantPoints, pathsBetweenStations, pathMapMutex);

    // Deallocate the memory used by navGrid and parentGrid.
    DeallocateGrid(navGrid);
    DeallocateParentGrid(parentGrid);
}
