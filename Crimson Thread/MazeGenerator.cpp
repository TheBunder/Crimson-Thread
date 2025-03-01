#include <iostream>
#include "MazeGenerator.h"
#include "mazeChars.h"
using namespace std;
//----CONSTANTS-------------------------------------------------------
const int DOWN = 0;
const int RIGHT = 1;
const int UP = 2;
const int LEFT = 3;
const char WALL = 219; // █
const char SPACE = 32; // | |<- Space
const char PEOPLE = 64; // @
const int SUBGRID_SIZE = GRID_SIZE/4;

//----GLOBAL VARIABLES------------------------------------------------
char** grid;
//----FUNCTION PROTOTYPES---------------------------------------------
void ResetGrid(); //Fill the array with the WALL sign
int	 IsInArrayBounds(int x, int y); // Check if the x and y points are in the array
void Visit(int x, int y); // Move in the array and make a peath (The main method to creat the maze)
void BreakWalls(); // After the maze was made it breaks aditional paths
void RedoWalls(); // Convert the walls from the difult version to a better loking tiles
void InsertPeople(); // Add people (Hostages and\or kidnappers) to the maze
void PrintGrid(); // Print the array
//----FUNCTIONS-------------------------------------------------------
void generate(char** gridI)
{
	grid = gridI;
	// Starting point and top-level control.
	srand(time(0)); // seed random number generator.
	ResetGrid();
	Visit(1, 1);
	BreakWalls();
	RedoWalls();
	InsertPeople();
	PrintGrid();
}

void ResetGrid() {
	// Fills the grid with walls.
	for (int y = 0; y < GRID_HEIGHT; ++y) {
		for (int x = 0; x < GRID_WIDTH; ++x) {
			grid[y][x] = WALL;
		}
	}
}

int IsInArrayBounds(int x, int y)
{
	// Returns "true" if x and y are both in-bounds.
	if (x < 0 || x >= GRID_WIDTH) return false;
	if (y < 0 || y >= GRID_HEIGHT) return false;
	return true;
}

int IsInMaze(int x, int y)
{
	// Returns "true" if x and y are both in-bounds.
	if (x < 1 || x >= GRID_WIDTH - 1) return false;
	if (y < 1 || y >= GRID_HEIGHT - 1) return false;
	return true;
}

void Visit(int x, int y) {
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
				Visit(x2, y2);
			}
		}
	}
}

int IsBrakeble(int x, int y) {
	if (!IsInMaze(x, y)) {
		return false;
	}
	return ((grid[x + 1][y] == WALL && grid[x - 1][y] == WALL
		&& !(grid[x][y + 1] == WALL || grid[x][y - 1] == WALL)) ||
		(grid[x][y + 1] == WALL && grid[x][y - 1] == WALL && !
			(grid[x + 1][y] == WALL || grid[x - 1][y] == WALL)));
}

void BreakWalls() {
	int numOfWallsBroken = GRID_SIZE*2;
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
			if (grid[x][y] == WALL && IsBrakeble(x, y))
			{
				brokeWall = 1;
			}

			location++;
		}

		// Break wall
		grid[x][y] = SPACE;
	}
}

void InsertPeople() {
	int numOfSections = (GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE); // (101/25)^2=16
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
			grid[x][y] = PEOPLE;
			placed = true;
		}
		else {
			// Try surrounding positions
			for (int j = 0; j < 8 && !placed; j++) {
				newX = x + dx[j];
				newY = y + dy[j];

				if (IsInMaze(newX, newY) && grid[newX][newY] == SPACE) {
					grid[newX][newY] = PEOPLE;
					placed = true;
				}
			}
		}
		
	}
}


int GetWallPositionValue(int x, int y) {
	return (grid[x + 1][y] != SPACE) * 1 + (grid[x][y + 1] != SPACE) * 2
		+ (grid[x - 1][y] != SPACE) * 4 + (grid[x][y - 1] != SPACE) * 8;
}

void RedoInnerWalls() {
	for (int y = 1; y < GRID_HEIGHT - 1; y++) {
		for (int x = 1; x < GRID_WIDTH - 1; x++) {
			if (grid[x][y] == WALL) {
				switch (GetWallPositionValue(x, y)) {
				case 0: case 1: case 4: case 5:
					grid[x][y] = MazeChar::HorizontalWall;
					break;
				case 2: case 8: case 10:
					grid[x][y] = MazeChar::VerticalWall;
					break;
				case 3:
					grid[x][y] = MazeChar::BottomLeftCorner;
					break;
				case 6:
					grid[x][y] = MazeChar::BottomRightCorner;
					break;
				case 7:
					grid[x][y] = MazeChar::TopTee;
					break;
				case 9:
					grid[x][y] = MazeChar::TopLeftCorner;
					break;
				case 11:
					grid[x][y] = MazeChar::RightTee;
					break;
				case 12:
					grid[x][y] = MazeChar::TopRightCorner;
					break;
				case 13:
					grid[x][y] = MazeChar::BottomTee;
					break;
				case 14:
					grid[x][y] = MazeChar::LeftTee;
					break;
				case 15:
					grid[x][y] = MazeChar::Cross;
					break;
				default:
					break;
				}
			}
		}
	}
}

void RedoOuterWalls() {
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

void RedoWalls() {
	RedoInnerWalls();
	RedoOuterWalls();
}

void PrintGrid() {
	for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
		for (int x = 0; x < GRID_WIDTH; x++) {
			putchar(grid[x][y]);
		}
		putchar('\n');
	}
}