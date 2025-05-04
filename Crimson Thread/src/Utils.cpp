//----INCLUDES--------------------------------------------------------
#include "include/Utils.h"

//----FUNCTIONS-------------------------------------------------------
char **allocateGrid() {
	char **grid = new char *[GRID_WIDTH];
	for (int i = 0; i < GRID_WIDTH; i++) {
		grid[i] = new char[GRID_HEIGHT]();
	}
	return grid;
}

void deallocateGrid(char** grid) {
	for (int i = 0; i < GRID_WIDTH; i++) {
		delete[] grid[i];
	}
	delete[] grid;
}

int IsInArrayBounds(int x, int y)
{
	// Returns "true" if x and y are both in-bounds.
	if (x < 0 || x >= GRID_WIDTH) return false;
	if (y < 0 || y >= GRID_HEIGHT) return false;
	return true;
}

PathKey makeKey(LocationID id1, LocationID id2) {
	return std::make_pair(std::min(id1, id2), std::max(id1, id2));
}