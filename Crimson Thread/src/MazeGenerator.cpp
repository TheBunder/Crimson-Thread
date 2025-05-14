//----INCLUDES--------------------------------------------------------
#include <algorithm>
#include <windows.h>
#include "include/MazeGenerator.h"
#include "include/Unit.h"

//----CONSTANTS-------------------------------------------------------
const int DOWN = 0;
const int RIGHT = 1;
const int UP = 2;
const int LEFT = 3;

// Lookup table for wall characters based on surrounding value
constexpr unsigned char WallTypeByPattern[16] = {
    MazeChar::HorizontalWall, // 0: No walls around
    MazeChar::HorizontalWall, // 1: Wall to the right
    MazeChar::VerticalWall, // 2: Wall below
    MazeChar::BottomLeftCorner, // 3: Wall to right and below
    MazeChar::HorizontalWall, // 4: Wall to the left
    MazeChar::HorizontalWall, // 5: Walls to right and left
    MazeChar::BottomRightCorner, // 6: Walls to left and below
    MazeChar::TopTee, // 7: Walls to right, left, and below
    MazeChar::VerticalWall, // 8: Wall above
    MazeChar::TopLeftCorner, // 9: Walls to right and above
    MazeChar::VerticalWall, // 10: Walls above and below
    MazeChar::RightTee, // 11: Walls to right, below, and above
    MazeChar::TopRightCorner, // 12: Walls to left and above
    MazeChar::BottomTee, // 13: Walls to right, left, and above
    MazeChar::LeftTee, // 14: Walls to left, below, and above
    MazeChar::Cross // 15: Walls in all directions
};

//----FUNCTION PROTOTYPES---------------------------------------------
void ResetGrid(char **grid); //Fill the array with the WALL sign
int IsInArrayBounds(int x, int y); // Check if the x and y points are in the array
void Visit(int x, int y, char **grid); // Move in the array and make a path (The main method to creat the maze)
void BreakWalls(char **grid); // After the maze was made, it breaks additional paths
void RedoWalls(char **grid); // Convert the walls from the default version to a better looking tiles
void InsertHostages(char **grid,
                    HostageStation **HostageStations); // Add people (Hostages and\or kidnappers) to the maze
void PrintGrid(char **grid); // Print the array
Point InsertUnitEntrance(char **grid); // Find a good position for the units to start.

//----FUNCTIONS-------------------------------------------------------
// Function to generate the maze and place important features like hostages and the unit entrance.
// This acts as the main orchestrator for the maze creation process.
Point Generate(char **grid, HostageStation **HostageStations) {
    // Start by filling the grid with walls.
    ResetGrid(grid);
    // Use a recursive backtracking to carve out the main paths of the maze, starting from (1, 1).
    Visit(1, 1, grid);
    // Break some additional walls to create more complex paths in the maze.
    BreakWalls(grid);
    // Refine the visual representation of the walls.
    RedoWalls(grid);
    // Insert and creat the HostageStations entities into the generated maze grid.
    InsertHostages(grid, HostageStations);
    // Find and insert a suitable starting position for the units within the maze.
    // Return the coordinates of this starting position.
    return InsertUnitEntrance(grid);
}

// Fills the grid with walls.
void ResetGrid(char **grid) {
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            grid[y][x] = WALL;
        }
    }
}

// Returns "true" if x and y are both in-bounds.
int IsInMaze(int x, int y) {
    if (x < 1 || x >= GRID_WIDTH - 1) return false;
    if (y < 1 || y >= GRID_HEIGHT - 1) return false;
    return true;
}

void Visit(int x, int y, char **grid) {
    // Set my current location to be an empty passage.
    grid[x][y] = PATH;

    // Create a local array containing the 4 directions and shuffle their order.
    int dirs[4] = {DOWN, RIGHT, UP, LEFT};
    for (int i = 0; i < 4; ++i) {
        int r = rand() % 4;
        dirs[r] = dirs[r] ^ dirs[i]; // Switch between two random elements
        dirs[i] = dirs[r] ^ dirs[i];
        dirs[r] = dirs[r] ^ dirs[i];
    }

    // Loop through every direction and attempt to Visit that direction.
    for (int i = 0; i < 4; ++i) {
        int dx = 0, dy = 0;
        switch (dirs[i]) {
            case DOWN: dy = -1;
                break;
            case UP: dy = 1;
                break;
            case RIGHT: dx = 1;
                break;
            case LEFT: dx = -1;
                break;
        }

        int x2 = x + (dx * 2);
        int y2 = y + (dy * 2);

        if (IsInArrayBounds(x2, y2)) {
            // In-bounds check
            if (grid[x2][y2] == WALL) {
                grid[x + dx][y + dy] = PATH; // Knock down the wall
                Visit(x2, y2, grid);
            }
        }
    }
}

// Check if a wall is breakable
int IsBreakable(int x, int y, char **grid) {
    if (!IsInMaze(x, y)) {
        return false;
    }
    return ((grid[x + 1][y] == WALL && grid[x - 1][y] == WALL
             && !(grid[x][y + 1] == WALL || grid[x][y - 1] == WALL)) ||
            (grid[x][y + 1] == WALL && grid[x][y - 1] == WALL && !
             (grid[x + 1][y] == WALL || grid[x - 1][y] == WALL)));
}

void BreakWalls(char **grid) {
    // Define the number of walls to attempt to break
    int numOfWallsBroken = GRID_SIZE * 2;
    // Variable to store a random linear index in the grid.
    int location;
    // Flag to indicate if a breakable wall has been found for the current attempt.
    int brokeWall;
    // Variables to store the 2D coordinates
    int x, y;

    for (int i = 0; i < numOfWallsBroken; i++) {
        // Reset the flag for this wall-breaking attempt.
        brokeWall = 0;
        // Get an initial random number.
        location = rand();

        // Loop until a breakable wall location is found.
        while (!brokeWall) {
            // Convert the random number into a linear index within the grid bounds.
            location %= (GRID_WIDTH * GRID_HEIGHT);
            // Convert the linear index into 2D grid coordinates (x, y).
            x = location % GRID_WIDTH;
            y = location / GRID_WIDTH;

            // Check if the current grid cell is a WALL AND if it meets the criteria to be breakable.
            if (grid[x][y] == WALL && IsBreakable(x, y, grid)) {
                // If it's a breakable wall, set the flag to exit the while loop.
                brokeWall = 1;
            }

            // Increment the location to check the next potential spot in the grid
            // if the current one wasn't a breakable wall.
            location++;
        }

        // Break wall
        grid[x][y] = PATH;
    }
}

void InsertHostages(char **grid, HostageStation **HostageStations) {
    // Calculate the total number of sections based on grid size and subgrid size.
    int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);
    // Flag to track if a station was successfully placed in the current section.
    bool placed;
    // Variables for initial random position within a subgrid.
    int x, y;
    // Variables for checking neighboring positions.
    int newX, newY;

    // Arrays to define relative coordinates for checking 8 neighbors + the center (9 total).
    int dx[] = {0, -1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {0, -1, 0, 1, -1, 1, -1, 0, 1};

    for (int i = 0; i < numOfSections; i++) {
        // Reset the placed flag for the new section.
        placed = false;

        // Calculate subgrid coordinates
        int subgrid_x = i % (GRID_WIDTH / SUBGRID_SIZE);
        int subgrid_y = i / (GRID_WIDTH / SUBGRID_SIZE);

        // Generate a random position within the subgrid
        x = subgrid_x * SUBGRID_SIZE + rand() % SUBGRID_SIZE;
        y = subgrid_y * SUBGRID_SIZE + rand() % SUBGRID_SIZE;


        // Check if the potential position is within the maze bounds AND if it's a valid path cell.
        for (int j = 0; j < 9 && !placed; j++) {
            newX = x + dx[j];
            newY = y + dy[j];

            if (IsInMaze(newX, newY) && grid[newX][newY] == PATH) {
                // If valid and a path, place the HOSTAGES character on the grid.
                grid[newX][newY] = HOSTAGES;
                placed = true;
                x = newX;
                y = newY;
            }
        }

        // Create a new HostageStation object and store it in the array
        if (placed) {
            // If successfully placed, create the object with the placed coordinates and random attributes.
            HostageStations[i] = new HostageStation(x, y, i, (double) (rand() % 101) / 100,
                                                    rand() % 10 + 1, (double) (rand() % 71) / 100,
                                                    (double) (rand() % 41) / 100);
        } else {
            // If not placed (no valid spot found in the section/neighbors),
            // create a dummy HostageStation with invalid coordinates (-1, -1) and default values.
            HostageStations[i] = new HostageStation(-1, -1, i, 0.0, 0, 0.0, 0.0); // if not placed put -1,-1
        }
    }
}

Point InsertUnitEntrance(char **grid) {
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
    return {-1, 1};
}

int GetWallSurroundingValue(int x, int y, char **grid) {
    return (grid[x + 1][y] != PATH) * 1 + // Right neighbor
           (grid[x][y + 1] != PATH) * 2 + // Top neighbor
           (grid[x - 1][y] != PATH) * 4 + // Left neighbor
           (grid[x][y - 1] != PATH) * 8; // Bottom neighbor
}

void RedoInnerWalls(char **grid) {
    for (int y = 1; y < GRID_HEIGHT - 1; y++) {
        for (int x = 1; x < GRID_WIDTH - 1; x++) {
            if (grid[x][y] == WALL) {
                grid[x][y] = WallTypeByPattern[GetWallSurroundingValue(x, y, grid)];
            }
        }
    }
}

void RedoTopAndBottomWalls(char **grid) {
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
    // Apply visual refinement to the inner walls of the maze.
    RedoInnerWalls(grid);
    // Apply visual refinement to the outer walls, including borders and corners.
    RedoOuterWalls(grid);
}

// All colors that are used are from the 256-color color table
void HostagesColor() {
    // Color name: OrangeRed1 - 202
    printf("\033[38;5;202m");
}

void UnitColor() {
    // Color name: Yellow1 - 226
    printf("\033[38;5;226m");
}

void ResetFG() {
    // Reset FG back to black
    printf("\033[0m");
}

void SearchedColor() {
    // Color name: PaleTurquoise1 - 159
    printf("\033[48;5;159m");
}

void PathColor() {
    // Color name: DeepSkyBlue1 - 39
    printf("\033[48;5;39m");
}

void FinishedPathColor() {
    // Color name: BlueViolet  - 57
    printf("\033[48;5;57m");
}


void ResetBG() {
    // Reset BG back to black
    printf("\033[48;5;0m");
}

void PrintXAxis() {
    printf("   ");
    for (int i = 0; i < GRID_WIDTH; i++) {
        putchar(i % 10 + '0');
    }
    putchar('\n');
}

void PrintGrid(char **grid) {
    // Print top X axis
    PrintXAxis();

    for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
        // Print left Y axis
        printf("%02d ", y % 100); // Print Y coordinate (mod 100)

        for (int x = 0; x < GRID_WIDTH; x++) {
            // Check if wall or path
            if ((unsigned char) grid[x][y] > 100 || grid[x][y] == PATH) {
                putchar(grid[x][y]);
            } else {
                // Print with specific color
                if (grid[x][y] == HOSTAGES) {
                    HostagesColor();
                } else {
                    UnitColor();
                }
                putchar(grid[x][y]);
                ResetFG();
            }
        }

        // Print right Y axis
        printf(" %02d\n", y % 100);
    }

    // Print bottom X axis
    PrintXAxis();
}

void PrintGridWithPath(char **grid, char **navGrid, vector<Point> path) {
    for (Point coord: path) {
        navGrid[coord.x][coord.y] = kPath;
    }
    char fBGChanged = 0;
    for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
        // Print left Y axis
        printf("%02d ", y % 100); // Print Y coordinate (mod 100)

        for (int x = 0; x < GRID_WIDTH; x++) {
            if (navGrid[x][y] == kPath) {
                // Mark path by changing BG color
                PathColor();
                fBGChanged = 1;
            }

            // Check if wall or path
            if ((unsigned char) grid[x][y] > 100 || grid[x][y] == PATH) {
                putchar(grid[x][y]);
            } else {
                // Print with specific color
                if (grid[x][y] == HOSTAGES) {
                    HostagesColor();
                } else {
                    UnitColor();
                }
                putchar(grid[x][y]);
                ResetFG();
            }
            if (fBGChanged) {
                ResetBG();
                fBGChanged >>= 1; // make zero
            }
        }

        // Print right Y axis
        printf(" %02d\n", y % 100);
    }


    // Reset the path mark
    for (Point coord: path) {
        navGrid[coord.x][coord.y] = kEmpty;
    }

    printf("\n");
}

void PrintGridWithPath(char **grid, char **navGrid) {
    bool fBGChanged = false;
    for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
        // Print left Y axis
        printf("%02d ", y % 100); // Print Y coordinate (mod 100)

        for (int x = 0; x < GRID_WIDTH; x++) {
            if (navGrid[x][y] == -1) {
                // Mark finished path by changing BG color
                FinishedPathColor();
                fBGChanged = true;
            } else if (navGrid[x][y] > 0) {
                // Mark path by changing BG color
                PathColor();
                fBGChanged = true;
            }

            // Check if wall or path
            if ((unsigned char) grid[x][y] > 100 || grid[x][y] == PATH) {
                putchar(grid[x][y]);
            } else {
                // Print with specific color
                if (grid[x][y] == HOSTAGES) {
                    HostagesColor();
                } else {
                    UnitColor();
                }
                putchar(grid[x][y]);
                ResetFG();
            }
            if (fBGChanged) {
                ResetBG();
                fBGChanged = false; // make zero
            }
        }

        // Print right Y axis
        printf(" %02d\n", y % 100);
    }

    printf("\n");
}

void PrintCharInGrid(int x, int y, char **navGrid, char c) {
    // Set background color
    if (navGrid[x][y] == -1) {
        // Mark finished path by changing BG color
        FinishedPathColor();
    } else if (navGrid[x][y] > 0) {
        // Mark path by changing BG color
        PathColor();
    }

    // Print with specific color
    if (c!=PATH) {
        if (c == HOSTAGES) {
            HostagesColor();
        } else {
            UnitColor();
        }
    }

    // Put the char
    putchar(c);

    // Reset colors
    ResetFG();
    ResetBG();
}

void MarkPath(vector<Unit> units, char **navGrid) {
    for (Unit unit: units) {
        queue<Point> q = unit.GetPath();
        while (!q.empty()) {
            navGrid[q.front().x][q.front().y]++;
            q.pop();
        }
    }
}

void PrintGridWithUnits(char **grid, vector<Unit> units, char **navGrid) {
    // Add units
    for (Unit unit: units) {
        grid[unit.GetX()][unit.GetY()] = UNIT;
    }

    // Print
    PrintGridWithPath(grid, navGrid);

    // Remove units and their path mark
    for (Unit unit: units) {
        grid[unit.GetX()][unit.GetY()] = PATH;
        if (--navGrid[unit.GetX()][unit.GetY()] == 0) {
            // If no more units will wolk there mark as empty
            navGrid[unit.GetX()][unit.GetY()] = -1;
        }
    }
}

void ShowNextFrame(char **grid, vector<Unit> units, char **navGrid, HANDLE hConsole) {
    // Add units
    for (Unit unit: units) {
        // Get position to change
        int unitX = unit.GetX();
        int unitY = unit.GetY();

        // Set cursor in the correct position
        COORD coord = {(short)(unitX+3), (short)(GRID_HEIGHT-unitY-1)};
        SetConsoleCursorPosition(hConsole, coord);

        // Print the unit
        PrintCharInGrid(unitX, unitY, navGrid, UNIT);
    }

    // Remove units and their path mark
    for (Unit unit: units) {
        // Get position to change
        Point previousCoords = unit.GetPreviousCoords();
        int unitX = previousCoords.x;
        int unitY = previousCoords.y;

        // Set cursor in the correct position
        COORD coord = {(short)(unitX+3), (short)(GRID_HEIGHT-unitY-1)};
        SetConsoleCursorPosition(hConsole, coord);

        // Print the unit
        PrintCharInGrid(unitX, unitY, navGrid, PATH);
        if (--navGrid[unit.GetX()][unit.GetY()] == 0) {
            // If no more units will wolk there mark as empty
            navGrid[unit.GetX()][unit.GetY()] = -1;
        }
    }
}

void CreatUnits(vector<Unit> &units, int numOfUnits, Point unitsEntrance, vector<vector<LocationID> > &OperationOrder,
                map<PathKey, vector<Point> > &pathsBetweenStations) {
    for (int i = 0; i < numOfUnits; i++) {
        units.emplace_back(unitsEntrance, OperationOrder[i], pathsBetweenStations);
    }
}

void ShowOperation(char **grid, int numOfUnits, Point unitsEntrance, vector<vector<LocationID> > &OperationOrder,
                   map<PathKey, vector<Point> > &pathsBetweenStations) {
    // Get console handle
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Creat units
    vector<Unit> units{};
    CreatUnits(units, numOfUnits, unitsEntrance, OperationOrder, pathsBetweenStations);

    // Allocate and fill grid
    char **navGrid = AllocateGrid();

    // Set default navigation grid
    for (int i = 0; i < GRID_WIDTH; ++i) {
        for (int j = 0; j < GRID_HEIGHT; ++j) {
            navGrid[i][j] = kEmpty;
        }
    }

    // Fill the path the units will take
    MarkPath(units, navGrid);

    // Move to a good location to print the simulation
    COORD coord = {0, 0};
    SetConsoleCursorPosition(hConsole, coord);

    // Print the frame
    PrintGridWithUnits(grid, units, navGrid);

    while (!units.empty()) {
        // Move all units and remove those that finished their operation.
        for (int u = units.size() - 1; u >= 0; --u) {
            if (units[u].IsFinished()) {
                // Remove the station using swap and pop.
                swap(units[u], units.back());
                units.pop_back();
            } else {
                // Remember where the units were to remove from the grid
                units[u].SetPreviousCoords(units[u].GetCoords());

                // Move those that didn't finish.
                units[u].Move(grid);
            }
        }

        ShowNextFrame(grid, units, navGrid, hConsole);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    // deallocate and fill grid
    DeallocateGrid(navGrid);
}
