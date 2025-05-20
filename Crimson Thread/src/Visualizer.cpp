//----INCLUDES--------------------------------------------------------
#include <windows.h>
#include <iostream>
#include "include/Visualizer.h"
#include "include/Unit.h"

//----FUNCTIONS-------------------------------------------------------
// All colors that are used are from the 256-color table
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

void PathColor() {
    // Color name: DeepSkyBlue1 - 39
    printf("\033[48;5;39m");
}

void FinishedPathColor() {
    // Color name: BlueViolet  - 57
    printf("\033[48;5;57m");
}

void NextStationColor() {
    // Color name: Gold3  - 178
    printf("\033[48;5;178m");
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
            } else if (navGrid[x][y] <= -2) {
                // Mark The next station the unit will go to
                NextStationColor();
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
                fBGChanged = false;
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
    } else if (navGrid[x][y] <= -2) {
        // Mark The next station the unit will go to
        NextStationColor();
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

    // Mark each unit first objective
    for (Unit unit : units) {
        Point unitFirstStation = unit.GetNextStationCoords();
        navGrid[unitFirstStation.x][unitFirstStation.y] = -2;
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

        if (--navGrid[unit.GetX()][unit.GetY()] == 0) {
            // If no more units will wolk there mark as empty
            navGrid[unit.GetX()][unit.GetY()] = -1;
        }

        // Print the item under the unit and update the grid
        PrintCharInGrid(unitX, unitY, navGrid, grid[unitX][unitY]);
    }
}

void CreatUnits(vector<Unit> &units, int numOfUnits, Point unitsEntrance, vector<vector<LocationID> > &OperationOrder,
                map<PathKey, vector<Point> > &pathsBetweenStations) {
    for (int i = 0; i < numOfUnits; i++) {
        units.emplace_back(unitsEntrance, OperationOrder[i], pathsBetweenStations);
    }
}

void MarkNextStation(char **grid, HANDLE hConsole, Point newStationLocation, char **navGrid) {
    navGrid[newStationLocation.x][newStationLocation.y] = -2; // Mark on the navGrid

    // Mark on the screen
    COORD coord = {(short)(newStationLocation.x+3), (short)(GRID_HEIGHT-newStationLocation.y-1)};
    SetConsoleCursorPosition(hConsole, coord);

    // Print the station
    PrintCharInGrid(newStationLocation.x, newStationLocation.y, navGrid, HOSTAGES);
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

    // Print the very first frame
    PrintGridWithPath(grid, navGrid);

    while (!units.empty()) {
        // Move all units and remove those that finished their operation.
        for (int u = units.size() - 1; u >= 0; --u) {
            if (units[u].IsFinished()) {
                // Remove the station using swap and pop.
                swap(units[u], units.back());
                units.pop_back();
            } else {
                // Move those that didn't finish.
                bool markNewStation = units[u].Move(grid);
                if (markNewStation) {
                    MarkNextStation(grid, hConsole, units[u].GetNextStationCoords(), navGrid);
                }
            }
        }

        ShowNextFrame(grid, units, navGrid, hConsole);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    // Move the pointer to the bottom of the grid
    coord = {0, GRID_HEIGHT};
    SetConsoleCursorPosition(hConsole, coord);

    // Deallocate grid
    DeallocateGrid(navGrid);
}