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
Point generate(char **grid, HostageStation **HostageStations) {
    // Starting point and top-level control.
    ResetGrid(grid);
    Visit(1, 1, grid);
    BreakWalls(grid);
    RedoWalls(grid);
    InsertHostages(grid, HostageStations);
    return InsertUnitEntrance(grid);
}

void ResetGrid(char **grid) {
    // Fills the grid with walls.
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            grid[y][x] = WALL;
        }
    }
}

int IsInMaze(int x, int y) {
    // Returns "true" if x and y are both in-bounds.
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
    int numOfWallsBroken = GRID_SIZE * 2;
    int location;
    int brokeWall;
    int x, y;

    for (int i = 0; i < numOfWallsBroken; i++) {
        brokeWall = 0;
        location = rand(); // Random location

        while (!brokeWall) {
            location %= (GRID_WIDTH * GRID_HEIGHT); // Keeps in the grid at all the iterations
            x = location % GRID_WIDTH;
            y = location / GRID_WIDTH;
            if (grid[x][y] == WALL && IsBreakable(x, y, grid)) {
                brokeWall = 1;
            }

            location++;
        }

        // Break wall
        grid[x][y] = PATH;
    }
}

void InsertHostages(char **grid, HostageStation **HostageStations) {
    int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);
    bool placed = false;
    int x, y;
    int newX, newY;

    int dx[] = {0, -1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {0, -1, 0, 1, -1, 1, -1, 0, 1};

    for (int i = 0; i < numOfSections; i++) {
        placed = false;

        // Calculate subgrid coordinates
        int subgrid_x = i % (GRID_WIDTH / SUBGRID_SIZE);
        int subgrid_y = i / (GRID_WIDTH / SUBGRID_SIZE);

        // Generate a random position within the subgrid
        x = subgrid_x * SUBGRID_SIZE + rand() % SUBGRID_SIZE;
        y = subgrid_y * SUBGRID_SIZE + rand() % SUBGRID_SIZE;


        // Try to insert
        for (int j = 0; j < 9 && !placed; j++) {
            newX = x + dx[j];
            newY = y + dy[j];

            if (IsInMaze(newX, newY) && grid[newX][newY] == PATH) {
                grid[newX][newY] = HOSTAGES;
                placed = true; // It still might not be placed
                x = newX;
                y = newY;
            }
        }

        // Create a new HostageStation object and store it in the array
        if (placed) {
            HostageStations[i] = new HostageStation(x, y, i, (double) (rand() % 101) / 100,
                                                    rand() % 10 + 1, (double) (rand() % 71) / 100,
                                                    (double) (rand() % 41) / 100);
        } else {
            HostageStations[i] = new HostageStation(-1, -1, i, 0.0, 0, 0.0, 0.0); // if not placed put -1,-1
        }
    }
}

Point InsertUnitEntrance(char **grid) {
    int widthCenter = GRID_WIDTH / 2;
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
    RedoTopAndBottomWalls(grid);
    RedoLeftAndRightWalls(grid);

    //Corners
    grid[0][0] = MazeChar::BottomLeftCorner;
    grid[GRID_WIDTH - 1][0] = MazeChar::BottomRightCorner;
    grid[GRID_WIDTH - 1][GRID_HEIGHT - 1] = MazeChar::TopRightCorner;
    grid[0][GRID_HEIGHT - 1] = MazeChar::TopLeftCorner;
}

void RedoWalls(char **grid) {
    RedoInnerWalls(grid);
    RedoOuterWalls(grid);
}

// All colors that are used are from the 256-color color table
void HostagesColor() {
    // Color name: OrangeRed1 - 202
    printf("\033[38;5;202m");
}

void UnitColor() {
    // Color name: Green3 - 40
    printf("\033[38;5;40m");
}

void resetFG() {
    // Reset FG back to black
    printf("\033[0m");
}

void SearchedColor() {
    // Color name: PaleTurquoise1 - 159
    printf("\033[48;5;159m");
}

void PathColor() {
    // Color name: GreenYellow - 154
    printf("\033[48;5;154m");
}

void resetBG() {
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
            if ((unsigned char) grid[x][y] > 100 || grid[x][y] == PATH) {
                putchar(grid[x][y]);
            } else {
                if (grid[x][y] == HOSTAGES) {
                    HostagesColor();
                } else {
                    UnitColor();
                }
                putchar(grid[x][y]);
                resetFG();
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
                PathColor();
                fBGChanged = 1;
            }

            if ((unsigned char) grid[x][y] > 100 || grid[x][y] == PATH) {
                putchar(grid[x][y]);
            } else {
                if (grid[x][y] == HOSTAGES) {
                    HostagesColor();
                } else {
                    UnitColor();
                }
                putchar(grid[x][y]);
                resetFG();
            }
            if (fBGChanged) {
                resetBG();
                fBGChanged >>= 1; // make zero
            }
        }

        // Print right Y axis
        printf(" %02d\n", y % 100);
    }

    for (Point coord: path) {
        navGrid[coord.x][coord.y] = kEmpty;
    }

    printf("\n");
}

void PrintGridWithUnits(char **grid, vector<Unit> units) {
    // Add units
    for (Unit unit : units){
        grid[unit.getX()][unit.getY()] = UNIT;
    }

    // Print
    PrintGrid(grid);

    // Remove units
    for (Unit unit : units){
        grid[unit.getX()][unit.getY()] = PATH;
    }
}

void creatUnits(vector<Unit> &units, int numOfUnits, Point unitsEntrance, vector<vector<LocationID>> &OperationOrder,
                   map<PathKey, vector<Point> > &pathsBetweenStations) {
    for (int i = 0; i < numOfUnits; i++) {
        units.emplace_back(unitsEntrance, OperationOrder[i], pathsBetweenStations);
    }
}

void ShowOperation(char **grid, int numOfUnits, Point unitsEntrance, vector<vector<LocationID> > &OperationOrder,
                   map<PathKey, vector<Point> > &pathsBetweenStations) {
    vector<Unit> units{};
    creatUnits(units, numOfUnits, unitsEntrance, OperationOrder, pathsBetweenStations);
    while (!units.empty()) {
        for (int u = units.size()-1; u >= 0; --u) {
            if (units[u].isFinished()) {
                // Remove the station using swap and pop
                units[u].swapUnits(units.back());
                units.pop_back();
            }
            else {
                units[u].move();
            }
        }
        COORD coord = { 0, 1 };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
        PrintGridWithUnits(grid, units);
    }
}
