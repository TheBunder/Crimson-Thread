//----INCLUDES--------------------------------------------------------
#include <algorithm>
#include "include/MazeGenerator.h"
#include "include/Unit.h"
#include "include/Visualizer.h"

//----CONSTANTS-------------------------------------------------------
// 4 direction array (up, right, down, left)
const int dx[] = {0, 1, 0, -1};
const int dy[] = {-1, 0, 1, 0};

// Lookup table for wall characters based on surrounding value
constexpr unsigned char WALL_TYPE_BY_PATTERN[16] = {
    MazeChar::HorizontalWall, // 0: No walls around
    MazeChar::HorizontalWall, // 1: Wall to the right
    MazeChar::VerticalWall, // 2: Wall above
    MazeChar::BottomLeftCorner, // 3: Wall to right and above
    MazeChar::HorizontalWall, // 4: Wall to the left
    MazeChar::HorizontalWall, // 5: Walls to right and left
    MazeChar::BottomRightCorner, // 6: Walls to left and above
    MazeChar::TopTee, // 7: Walls to right, left, and above
    MazeChar::VerticalWall, // 8: Wall below
    MazeChar::TopLeftCorner, // 9: Walls to right and below
    MazeChar::VerticalWall, // 10: Walls below and above
    MazeChar::RightTee, // 11: Walls to right, above, and below
    MazeChar::TopRightCorner, // 12: Walls to left and below
    MazeChar::BottomTee, // 13: Walls to right, left, and below
    MazeChar::LeftTee, // 14: Walls to left, above, and below
    MazeChar::Cross // 15: Walls in all directions
};

//----FUNCTION PROTOTYPES---------------------------------------------
void ResetGrid(char **grid); //Fill the array with the WALL sign
void CarveMaze(char** grid, int currentX, int currentY); // Move in the array and make a path (The main method to creat the maze)
void BreakWalls(char **grid); // After the maze was made, it breaks additional paths
void RedoWalls(char **grid); // Convert the walls from the default version to a better looking tiles
void FillWithDefaultStations(HostageStation **hostageStation); // fill the entire hostages array with default values
void InsertHostages(char **grid,
                    HostageStation **hostageStations); // Add people (Hostages and\or kidnappers) to the maze
Point InsertUnitEntrance(char **grid); // Find a good position for the units to start.

//----FUNCTIONS-------------------------------------------------------
// Generate te entire simulation environment including the maze, the hostage stations and the unit entrance.
Point GenerateSimulationEnvironment(char **grid, HostageStation **hostageStations) {
    if (grid == nullptr) {
        PrintError("Error: GenerateSimulationEnvironment received null grid.\n");
        FillWithDefaultStations(hostageStations);
        return {-1, -1};
    }
    if (hostageStations == nullptr) {
        PrintError("Error: GenerateSimulationEnvironment received null hostageStations.\n");
        FillWithDefaultStations(hostageStations);
        return {-1, -1};;
    }

    // 1. Start by filling the grid with walls.
    ResetGrid(grid);
    // 2. Use a recursive backtracking to carve out the main paths of the maze, starting from (1, 1).
    CarveMaze(grid, 1, 1);
    // 3. Break some additional walls to create more ways to go between two points in the maze.
    BreakWalls(grid);
    // 4. Refine the visual representation of the walls.
    RedoWalls(grid);
    // 5. Creat and insert the hostageStations into the grid.
    InsertHostages(grid, hostageStations);
    // 6. Find and insert a fitting entrance position for the units within the maze, and return it.
    return InsertUnitEntrance(grid);
}

// Fills the grid with walls.
void ResetGrid(char **grid) {
    // Check if we got a valid pointer to a grid.
    if (grid == nullptr) {
        PrintError("Error: ResetGrid received a null grid pointer. Cannot reset grid.\n");
        return; // Cannot proceed without a grid
    }

    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            grid[x][y] = WALL;
        }
    }
}

// Check if a cell is unvisited (surrounded by walls)
bool IsUnvisited(char** grid, int x, int y) {
    if (grid == nullptr) {
        PrintError("Error: IsUnvisited received a null grid pointer.\n");
        return false;
    }

    if (!IsInArrayBounds(x, y)) {
        return false;
    }

    // Check if this position and all around it are walls
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int newX = x + i;
            int newY = y + j;
            if (IsInArrayBounds(newX, newY) && grid[newX][newY] != WALL) {
                return false;
            }
        }
    }

    // All the surroundings are walls, so it is truly unvisited.
    return true;
}

// Get unvisited neighbors that are 2 cells away
std::vector<Point> GetUnvisitedNeighbors(char** grid, int x, int y) {
    std::vector<Point> neighbors;
    if (grid == nullptr) {
        PrintError("Error: GetUnvisitedNeighbors received a null grid pointer.\n");
        return neighbors;
    }

    for (int i = 0; i < 4; i++) {
        int newX = x + dx[i] * 2;  // Move 2 Points in each direction
        int newY = y + dy[i] * 2;

        if (IsUnvisited(grid, newX, newY)) { // only unvisited tiles can be neighbors
            neighbors.emplace_back(newX, newY);
        }
    }

    return neighbors;
}

// Create a path between two Points
void CreatePath(char** grid, int x1, int y1, int x2, int y2) {
    if (grid == nullptr) {
        PrintError("Error: CreatePath received a null grid pointer.\n");
        return;
    }

    grid[x1][y1] = PATH;
    grid[x2][y2] = PATH;

    // Create path in the Point between them
    int midX = x1 + (x2 - x1) / 2;
    int midY = y1 + (y2 - y1) / 2;
    grid[midX][midY] = PATH;
}

// Shuffle vector using Fisher-Yates algorithm
void ShuffleVector(std::vector<Point>& vec) {
    for (int i = vec.size() - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        std::swap(vec[i], vec[j]);
    }
}

// Generate maze using true recursive backtracking
void CarveMaze(char** grid, int currentX, int currentY) {
    if (grid == nullptr) {
        PrintError("Error: CarveMaze received a null grid pointer.\n");
        return;
    }
    if (!IsInArrayBounds(currentX, currentY)) {
        PrintError("Error: CarveMaze called with out-of-bounds initial coordinates (%d, %d). Aborting execution.\n", currentX, currentY);
        return;
    }

    grid[currentX][currentY] = PATH; // Mark the current cell as a path

    std::vector<Point> neighbors = GetUnvisitedNeighbors(grid, currentX, currentY);
    ShuffleVector(neighbors); // Randomize the order of neighbors

    for (const Point& next : neighbors) {
        // Check that noun of the recursive calls changed the neighbor
        if (IsUnvisited(grid, next.x, next.y)) {
            CreatePath(grid, currentX, currentY, next.x, next.y);
            // Recursive call to explore from the new cell
            CarveMaze(grid, next.x, next.y);
        }
    }
}

// Returns true if x and y are both in the internal bounds of the maze.
bool IsInMaze(int x, int y) {
    if (x < 1 || x >= GRID_WIDTH - 1) return false;
    if (y < 1 || y >= GRID_HEIGHT - 1) return false;
    return true;
}

// Check if a wall is breakable
int IsBreakable(int x, int y, char **grid) {
    if (grid == nullptr) {
        PrintError("Error: IsBreakable received a null grid pointer.\n");
        return false;
    }
    if (!IsInMaze(x, y) || grid[x][y] != WALL) {
        return false;
    }

    bool isHorizontalSegment = grid[x + 1][y] == WALL && grid[x - 1][y] == WALL;
    bool isVerticalSegment = grid[x][y + 1] == WALL && grid[x][y - 1] == WALL;
    bool hasNoHorizontalWallConnection  = !(grid[x + 1][y] == WALL || grid[x - 1][y] == WALL);
    bool hasNoVerticalWallConnectio = !(grid[x][y + 1] == WALL || grid[x][y - 1] == WALL);

    // Check if it is a continued of a wall and not an intersection.
    return (isHorizontalSegment && hasNoVerticalWallConnectio) || (isVerticalSegment && hasNoHorizontalWallConnection);
}

void BreakWalls(char **grid) {
    if (grid == nullptr) {
        PrintError("Error: BreakWalls received a null grid pointer.\n");
        return;
    }

    // Define the number of walls to break
    int numOfWallsBroken = GRID_HEIGHT + GRID_WIDTH;
    // Holds the location in the maze if it was one denominational.
    int location;
    // Flags to indicate stats.
    bool brokeWall;
    bool noMoreBreakableWalls = false;
    // Store the coords
    int x, y;

    for (int i = 0; i < numOfWallsBroken && !noMoreBreakableWalls; i++) {
        // Reset the flag for this attempt.
        brokeWall = false;
        // Get an initial random number to indicate a place.
        int startLocation = rand() % (GRID_WIDTH * GRID_HEIGHT);
        location = startLocation + 1;

        // Loop until we find a suitable wall to break.
        while (!brokeWall && !noMoreBreakableWalls) {
            // Make sure the random number will be in the grid bounds.
            location %= (GRID_WIDTH * GRID_HEIGHT);
            if (location == startLocation) {
                noMoreBreakableWalls = true;
            }

            // Convert the number into a 2D location (x, y).
            x = location / GRID_HEIGHT;
            y = location % GRID_HEIGHT;

            // Check if the current grid cell is a WALL and if it is suitable for breaking.
            if (grid[x][y] == WALL && IsBreakable(x, y, grid)) {
                // If it's a breakable wall, set the flag to exit the loop and break.
                brokeWall = true;
                grid[x][y] = PATH;
            }

            // Continue moving in the grid forward until we find a suitable wall.
            location++;
        }
    }
}

// Try to place hostage station at random position or its 8 neighbors
bool TryPlaceAtRandomPosition(char **grid, int leftBound, int bottomBound, int* finalX, int* finalY) {
    if (grid == nullptr) {
        PrintError("Error: TryPlaceAtRandomPosition received null grid.\n");
        return false;
    }

    // Generate random position within subgrid
    int randomX = leftBound + rand() % SUBGRID_SIZE;
    int randomY = bottomBound + rand() % SUBGRID_SIZE;

    // Check center position + 8 surrounding neighbors
    int offsetX[] = {0, -1, -1, -1, 0, 0, 1, 1, 1};
    int offsetY[] = {0, -1, 0, 1, -1, 1, -1, 0, 1};

    for (int i = 0; i < 9; i++) {
        int candidateX = randomX + offsetX[i];
        int candidateY = randomY + offsetY[i];

        if (IsInMaze(candidateX, candidateY) && grid[candidateX][candidateY] == PATH) {
            grid[candidateX][candidateY] = HOSTAGES;
            *finalX = candidateX;
            *finalY = candidateY;
            return true;
        }
    }

    // Store the random position for modular search starting point
    *finalX = randomX;
    *finalY = randomY;
    return false;
}

// Search entire subgrid using modular approach starting from given position
bool SearchSubgridModular(char **grid, int leftBound, int bottomBound, int startX, int startY, int* finalX, int* finalY) {
    if (grid == nullptr) {
        PrintError("Error: SearchSubgridModular received null grid.\n");
        return false;
    }

    int startOffsetX = startX - leftBound;
    int startOffsetY = startY - bottomBound;

    for (int rowOffset = 0; rowOffset < SUBGRID_SIZE; rowOffset++) {
        for (int colOffset = 0; colOffset < SUBGRID_SIZE; colOffset++) {
            // Calculate new positions using modulo
            int candidateX = leftBound + ((startOffsetX + colOffset) % SUBGRID_SIZE);
            int candidateY = bottomBound + ((startOffsetY + rowOffset) % SUBGRID_SIZE);

            if (IsInMaze(candidateX, candidateY) && grid[candidateX][candidateY] == PATH) {
                grid[candidateX][candidateY] = HOSTAGES;
                *finalX = candidateX;
                *finalY = candidateY;
                return true;
            }
        }
    }

    // There is not a single PATH in the entire subgird.
    return false;
}

void FillWithDefaultStations(HostageStation **hostageStations) {
    if (hostageStations == nullptr) {
        PrintError("Error: FillWithDefaultStations received null hostageStations.\n");
        return;
    }

    int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);

    for (int sectionIndex = 0; sectionIndex < numOfSections; sectionIndex++) {
        hostageStations[sectionIndex] = new HostageStation(-1, -1, sectionIndex, 0.0,
                                                                    0, 0.0, 0.0);
    }
}

void InsertHostages(char **grid, HostageStation **hostageStations) {
    if (grid == nullptr) {
        PrintError("Error: InsertHostages received null grid.\n");
        FillWithDefaultStations(hostageStations);
        return;
    }
    if (hostageStations == nullptr) {
        PrintError("Error: InsertHostages received null hostageStations.\n");
        return;
    }
    if (SUBGRID_SIZE <= 0) {
        PrintError("Error: SUBGRID_SIZE must be greater then 0.\n");
        return;
    }
    if (SUBGRID_SIZE > GRID_WIDTH || SUBGRID_SIZE > GRID_HEIGHT) {
        PrintError("Error: SUBGRID_SIZE must be smaller then the GRID_WIDTH and GRID_HEIGHT.\n");
        return;
    }

    int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);

    for (int sectionIndex = 0; sectionIndex < numOfSections; sectionIndex++) {
        // Calculate subgrid boundaries
        int subgridCol = sectionIndex % (GRID_WIDTH / SUBGRID_SIZE);
        int subgridRow = sectionIndex / (GRID_WIDTH / SUBGRID_SIZE);
        int leftBound = subgridCol * SUBGRID_SIZE;
        int bottomBound = subgridRow * SUBGRID_SIZE;

        int finalX = -1, finalY = -1;
        bool placed = false;

        // First try: Insert to a random position or to it's 8 surrounding neighbors.
        placed = TryPlaceAtRandomPosition(grid, leftBound, bottomBound, &finalX, &finalY);

        // Second attempt: Search entire subgrid modularly if the first attempt didn't find a place.
        if (!placed) {
            placed = SearchSubgridModular(grid, leftBound, bottomBound, finalX, finalY, &finalX, &finalY);
        }

        // Create HostageStation
        if (placed) {
            hostageStations[sectionIndex] = new HostageStation(finalX, finalY, sectionIndex, (double)(rand() % 101) / 100,
                                                    rand() % 10 + 1, (double)(rand() % 71) / 100,
                                                    (double)(rand() % 41) / 100);
        } else {
            // This subgrid has no PATH cells at all
            hostageStations[sectionIndex] = new HostageStation(-1, -1, sectionIndex, 0.0,
                                                                    0, 0.0, 0.0);
        }
    }
}

Point InsertUnitEntrance(char **grid) {
    if (grid == nullptr) {
        PrintError("Error: InsertUnitEntrance received a null grid pointer.\n");
        return {-1, 1}; // Error value
    }

    // Calculate the horizontal center of the grid.
    int widthCenter = GRID_WIDTH / 2;

    // Search outwards from the horizontal center towards the left and right.
    for (int x = 0; x < GRID_WIDTH / 2; x++) {
        if (grid[widthCenter + x][1] == PATH) {
            grid[widthCenter + x][1] = 'E';
            return {widthCenter + x, 1};
        }
        if (grid[widthCenter - x][1] == PATH) {
            grid[widthCenter - x][1] = 'E';
            return {widthCenter - x, 1};
        }
    }

    // Handling the case where no entrance point is found. Even tho {1,1} will always be path.
    PrintError("Error: No suitable entrance location was found.\n");
    return {-1, 1};
}

int GetWallSurroundingValue(int x, int y, char **grid) {
    if (grid == nullptr) {
        PrintError("Error: GetWallSurroundingValue received a null grid pointer.\n");
        return -1; // Error value
    }
    if (!IsInMaze(x,y)) {
        PrintError("Error: GetWallSurroundingValue got out of maze bounds coords (%d, %d).\n", x, y);
        return -1; // Error value
    }

    return ((grid[x + 1][y] != PATH) << 0) +    // Right neighbor (Bit 0)
           ((grid[x][y + 1] != PATH) << 1) +    // Top neighbor (Bit 1)
           ((grid[x - 1][y] != PATH) << 2) +    // Left neighbor (Bit 2)
           ((grid[x][y - 1] != PATH) << 3);     // Bottom neighbor (Bit 3)
}

void RedoInnerWalls(char **grid) {
    if (grid == nullptr) {
        PrintError("Error: RedoInnerWalls received a null grid pointer.\n");
        return;
    }

    for (int y = 1; y < GRID_HEIGHT - 1; y++) {
        for (int x = 1; x < GRID_WIDTH - 1; x++) {
            if (grid[x][y] == WALL) {
                int wallSurroundingValue = GetWallSurroundingValue(x, y, grid);
                if (wallSurroundingValue != -1) {
                    grid[x][y] = WALL_TYPE_BY_PATTERN[wallSurroundingValue];
                }
            }
        }
    }
}

void RedoTopAndBottomWalls(char **grid) {
    if (grid == nullptr) {
        PrintError("Error: RedoTopAndBottomWalls received a null grid pointer.\n");
        return;
    }

    for (int x = 1; x < GRID_WIDTH - 1; x++) {
        //Top walls
        if (grid[x][1] == PATH) {
            grid[x][0] = MazeChar::HorizontalWall;
        } else {
            grid[x][0] = MazeChar::TopTee;
        }
        //Bottom walls
        if (grid[x][GRID_HEIGHT - 2] == PATH) {
            grid[x][GRID_HEIGHT - 1] = MazeChar::HorizontalWall;
        } else {
            grid[x][GRID_HEIGHT - 1] = MazeChar::BottomTee;
        }
    }
}

void RedoLeftAndRightWalls(char **grid) {
    if (grid == nullptr) {
        PrintError("Error: RedoLeftAndRightWalls received a null grid pointer.\n");
        return;
    }

    for (int y = 1; y < GRID_HEIGHT - 1; y++) {
        //Left walls
        if (grid[1][y] == PATH) {
            grid[0][y] = MazeChar::VerticalWall;
        } else {
            grid[0][y] = MazeChar::RightTee;
        }
        //Right walls
        if (grid[GRID_WIDTH - 2][y] == PATH) {
            grid[GRID_WIDTH - 1][y] = MazeChar::VerticalWall;
        } else {
            grid[GRID_WIDTH - 1][y] = MazeChar::LeftTee;
        }
    }
}

void RedoOuterWalls(char **grid) {
    if (grid == nullptr) {
        PrintError("Error: RedoOuterWalls received a null grid pointer.\n");
        return;
    }

    // Refine the visual representation of the top and bottom border walls.
    RedoTopAndBottomWalls(grid);
    // Refine the visual representation of the left and right border walls.
    RedoLeftAndRightWalls(grid);

    // Explicitly set the characters for the four corner cells of the grid.
    grid[0][0] = MazeChar::BottomLeftCorner;
    grid[GRID_WIDTH - 1][0] = MazeChar::BottomRightCorner;
    grid[GRID_WIDTH - 1][GRID_HEIGHT - 1] = MazeChar::TopRightCorner;
    grid[0][GRID_HEIGHT - 1] = MazeChar::TopLeftCorner;
}

void RedoWalls(char **grid) {
    if (grid == nullptr) {
        PrintError("Error: RedoWalls received a null grid pointer.\n");
        return;
    }

    // Apply visual refinement to the inner walls of the maze.
    RedoInnerWalls(grid);
    // Apply visual refinement to the outer walls, including borders and corners.
    RedoOuterWalls(grid);
}