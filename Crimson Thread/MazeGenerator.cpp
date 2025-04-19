//----INCLUDES--------------------------------------------------------
#include <stdio.h>
#include "MazeGenerator.h"

//----CONSTANTS-------------------------------------------------------
const int	DOWN		 = 0;
const int	RIGHT		 = 1;
const int	UP			 = 2;
const int	LEFT		 = 3;

//----FUNCTION PROTOTYPES---------------------------------------------
void ResetGrid(char** grid);			//Fill the array with the WALL sign
int	 IsInArrayBounds(int x, int y);		// Check if the x and y points are in the array
void Visit(int x, int y, char** grid);	// Move in the array and make a peath (The main method to creat the maze)
void BreakWalls(char** grid);			// After the maze was made it breaks aditional paths
void RedoWalls(char** grid);			// Convert the walls from the difult version to a better loking tiles
void InsertHostages(char** grid,
	HostageStation** HostageStations);	// Add people (Hostages and\or kidnappers) to the maze
void InsertRoutePoints(char** grid);	// Add route points to the maze
void PrintGrid(char** grid);			// Print the array

//----FUNCTIONS-------------------------------------------------------
void generate(char** grid, HostageStation** HostageStations)
{
	// Starting point and top-level control.
	srand(time(0)); // seed random number generator.
	ResetGrid(grid);
	Visit(1, 1, grid);
	BreakWalls(grid);
	RedoWalls(grid);
	InsertHostages(grid, HostageStations);
	InsertRoutePoints(grid);
}

void ResetGrid(char** grid) {
	// Fills the grid with walls.
	for (int y = 0; y < GRID_HEIGHT; ++y) {
		for (int x = 0; x < GRID_WIDTH; ++x) {
			grid[y][x] = WALL;
		}
	}
}

int IsInMaze(int x, int y)
{
	// Returns "true" if x and y are both in-bounds.
	if (x < 1 || x >= GRID_WIDTH - 1) return false;
	if (y < 1 || y >= GRID_HEIGHT - 1) return false;
	return true;
}

void Visit(int x, int y, char** grid) {
	// Set my current location to be an empty passage.
	grid[y][x] = SPACE;

	// Create a local array containing the 4 directions and shuffle their order.
	int dirs[4] = { DOWN, RIGHT, UP, LEFT };
	for (int i = 0; i < 4; ++i) {
		int r = rand() % 4;
		dirs[r] = dirs[r] ^ dirs[i]; // Switch betwin two random elements
		dirs[i] = dirs[r] ^ dirs[i];
		dirs[r] = dirs[r] ^ dirs[i];
	}

	// Loop through every direction and attempt to Visit that direction.
	for (int i = 0; i < 4; ++i) {
		int dx = 0, dy = 0;
		switch (dirs[i]) {
		case DOWN: dy = -1; break;
		case UP: dy = 1; break;
		case RIGHT: dx = 1; break;
		case LEFT: dx = -1; break;
		}

		int x2 = x + (dx * 2);
		int y2 = y + (dy * 2);

		if (x2 >= 0 && x2 < GRID_WIDTH && y2 >= 0 && y2 < GRID_HEIGHT) { // In-bounds check
			if (grid[y2][x2] == WALL) {
				grid[y + dy][x + dx] = SPACE; // Knock down the wall
				Visit(x2, y2, grid);
			}
		}
	}
}

int IsBrakeble(int x, int y, char** grid) {
	if (!IsInMaze(x, y)) {
		return false;
	}
	return ((grid[x + 1][y] == WALL && grid[x - 1][y] == WALL
		&& !(grid[x][y + 1] == WALL || grid[x][y - 1] == WALL)) ||
		(grid[x][y + 1] == WALL && grid[x][y - 1] == WALL && !
			(grid[x + 1][y] == WALL || grid[x - 1][y] == WALL)));
}

void BreakWalls(char** grid) {
	int numOfWallsBroken = GRID_SIZE * 2;
	int location;
	int brokeWall;
	int x, y;

	for (int i = 0; i < numOfWallsBroken; i++) {
		brokeWall = 0;
		location = rand(); // Random location

		while (!brokeWall) {
			location %= (GRID_WIDTH * GRID_HEIGHT); // Keaps in the grid at all the iterations
			x = location % GRID_WIDTH;
			y = location / GRID_WIDTH;
			if (grid[x][y] == WALL && IsBrakeble(x, y, grid))
			{
				brokeWall = 1;
			}

			location++;
		}

		// Break wall
		grid[x][y] = SPACE;
	}
}

void InsertHostages(char** grid, HostageStation** HostageStations) {
	int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);
	bool placed = false;
	int x, y;
	int newX, newY;

	int dx[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int dy[] = { -1, 0, 1, -1, 1, -1, 0, 1 };

	for (int i = 0; i < numOfSections; i++) {
		placed = false;

		// Calculate subgrid coordinates
		int subgrid_x = i % (GRID_WIDTH / SUBGRID_SIZE);
		int subgrid_y = i / (GRID_WIDTH / SUBGRID_SIZE);

		// Generate a random position within the subgrid
		x = subgrid_x * SUBGRID_SIZE + rand() % SUBGRID_SIZE;
		y = subgrid_y * SUBGRID_SIZE + rand() % SUBGRID_SIZE;

		if (grid[x][y] == SPACE) {
			grid[x][y] = HOSTAGES;
			placed = true;
		}
		else {
			// Try surrounding positions
			for (int j = 0; j < 8 && !placed; j++) {
				newX = x + dx[j];
				newY = y + dy[j];

				if (IsInMaze(newX, newY) && grid[newX][newY] == SPACE) {
					grid[newX][newY] = HOSTAGES;
					placed = true; // It still might not be placed
					x = newX;
					y = newY;
				}
			}
		}

		// Create a new HostageStation object and store it in the array
		if (placed) {
			HostageStations[i] = new HostageStation(x, y, i, (double)(rand() % 101) / 100,
				rand() % 10 + 1, (double)(rand() % 71) / 100, (double)(rand() % 41) / 100);
		}
		else {
			HostageStations[i] = new HostageStation(-1, -1, i, 0.0, 0, 0.0, 0.0); // if not placed put -1,-1
		}
	}
}

void InsertRoutePoints(char** grid) {
	int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE);
	bool placed = false;
	int x, y;
	int newX, newY;

	int dx[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int dy[] = { -1, 0, 1, -1, 1, -1, 0, 1 };

	for (int i = 0; i < numOfSections; i++) {
		placed = false;

		// Calculate subgrid coordinates
		int subgrid_x = i % (GRID_WIDTH / SUBGRID_SIZE);
		int subgrid_y = i / (GRID_WIDTH / SUBGRID_SIZE);

		// Calculate the middle of the subgrid
		x = subgrid_x * SUBGRID_SIZE + SUBGRID_SIZE / 2;
		y = subgrid_y * SUBGRID_SIZE + SUBGRID_SIZE / 2;

		if (IsInMaze(x, y) && grid[x][y] == SPACE) {
			grid[x][y] = RoutePoint; // Use the RoutePoint character
			placed = true;
		}
		else {
			// Try surrounding positions
			for (int j = 0; j < 8 && !placed; j++) {
				newX = x + dx[j];
				newY = y + dy[j];

				if (IsInMaze(newX, newY) && grid[newX][newY] == SPACE) {
					grid[newX][newY] = RoutePoint; // Use the RoutePoint character
					placed = true; // It still might not be placed
				}
			}
		}
	}
}

int GetWallSurroundingValue(int x, int y, char** grid) {
	return (grid[x + 1][y] != SPACE) * 1 + (grid[x][y + 1] != SPACE) * 2
		+ (grid[x - 1][y] != SPACE) * 4 + (grid[x][y - 1] != SPACE) * 8;
}

void RedoInnerWalls(char** grid) {
	char walls[] = {
		HorizontalWall,     // 0
		HorizontalWall,     // 1
		VerticalWall,       // 2
		BottomLeftCorner,   // 3
		HorizontalWall,     // 4
		HorizontalWall,     // 5
		BottomRightCorner,  // 6
		TopTee,             // 7
		VerticalWall,       // 8
		TopLeftCorner,      // 9
		VerticalWall,       // 10
		RightTee,           // 11
		TopRightCorner,     // 12
		BottomTee,          // 13
		LeftTee,            // 14
		Cross               // 15
	};
	for (int y = 1; y < GRID_HEIGHT - 1; y++) {
		for (int x = 1; x < GRID_WIDTH - 1; x++) {
			if (grid[x][y] == WALL) {
				grid[x][y] = walls[GetWallSurroundingValue(x, y, grid)];
			}
		}
	}
}

void RedoOuterWalls(char** grid) {
	for (int y = 1; y < GRID_HEIGHT - 1; y++) {
		//Left walls
		if (grid[1][y] == SPACE) {
			grid[0][y] = MazeChar::VerticalWall;
		}
		else {
			grid[0][y] = MazeChar::RightTee;
		}
		//Right walls
		if (grid[GRID_WIDTH - 2][y] == SPACE) {
			grid[GRID_WIDTH - 1][y] = MazeChar::VerticalWall;
		}
		else {
			grid[GRID_WIDTH - 1][y] = MazeChar::LeftTee;
		}
	}
	for (int x = 1; x < GRID_WIDTH - 1; x++) {
		//Top walls
		if (grid[x][1] == SPACE) {
			grid[x][0] = MazeChar::HorizontalWall;
		}
		else {
			grid[x][0] = MazeChar::TopTee;
		}
		//Bottom walls
		if (grid[x][GRID_HEIGHT - 2] == SPACE) {
			grid[x][GRID_HEIGHT - 1] = MazeChar::HorizontalWall;
		}
		else {
			grid[x][GRID_HEIGHT - 1] = MazeChar::BottomTee;
		}
	}

	//Corners
	grid[0][0] = MazeChar::BottomLeftCorner;
	grid[GRID_WIDTH - 1][0] = MazeChar::BottomRightCorner;
	grid[GRID_WIDTH - 1][GRID_HEIGHT - 1] = MazeChar::TopRightCorner;
	grid[0][GRID_HEIGHT - 1] = MazeChar::TopLeftCorner;
}

void RedoWalls(char** grid) {
	RedoInnerWalls(grid);
	RedoOuterWalls(grid);
}

// All colors that are used are from the 256-color color table
void HostagesColor() {
	// Color name: MediumPurple1 - 141
	printf("\033[38;5;141m");
}

void RoutePointsColor() {
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
	for (int i = 0; i < GRID_WIDTH; i++)
	{
		putchar(i % 10 + '0');
	}
	putchar('\n');
}

void PrintGrid(char** grid) {
	// Print top X axis
	PrintXAxis();

	for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
		// Print left Y axis
		printf("%02d ", y % 100); // Print Y coordinate (mod 100)

		for (int x = 0; x < GRID_WIDTH; x++) {
			if ((unsigned char)grid[x][y] > 100 || grid[x][y] == ' ') {
				putchar(grid[x][y]);
			}
			else {
				if (grid[x][y] == RoutePoint) {
					RoutePointsColor();
				}
				else {
					if (grid[x][y] == HOSTAGES) {
						HostagesColor();
					}
					else {
						UnitColor();
					}
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

void PrintGridWithPath(char** grid, char** path) {
	char fBGChanged = 0;
	for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
		// Print left Y axis
		printf("%02d ", y % 100); // Print Y coordinate (mod 100)

		for (int x = 0; x < GRID_WIDTH; x++) {
			if (path[x][y] == kClosed) {
				SearchedColor();
				fBGChanged = 1;
			}
			else
			{
				if (path[x][y] == kPath)
				{
					PathColor();
					fBGChanged = 1;
				}
			}
			if ((unsigned char)grid[x][y] > 100 || grid[x][y] == ' ') {
				putchar(grid[x][y]);
			}
			else {
				if (grid[x][y] == RoutePoint) {
					RoutePointsColor();
				}
				else {
					if (grid[x][y] == HOSTAGES) {
						HostagesColor();
					}
					else {
						UnitColor();
					}
				}
				putchar(grid[x][y]);
				resetFG();
			}
			if (fBGChanged) {
				resetBG();
				fBGChanged>>1; // make zero
			}
		}

		// Print right Y axis
		printf(" %02d\n", y % 100);
	}
	printf("\n");
}