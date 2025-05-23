//----INCLUDES--------------------------------------------------------
#include "include/Utils.h"

#include "include/Visualizer.h"

//----FUNCTIONS-------------------------------------------------------
char **AllocateGrid() {
	try {
		char **grid = new char *[GRID_WIDTH];
		for (int i = 0; i < GRID_WIDTH; i++) {
			grid[i] = new char[GRID_HEIGHT]();
		}
		return grid;
	}catch (const std::bad_alloc &e) {
		PrintError("Error: Failed to allocate grid memory.\n");
		return nullptr;
	}
}

void DeallocateGrid(char** grid) {
	if (grid == nullptr) {
		return;
	}

	for (int i = 0; i < GRID_WIDTH; i++) {
		delete[] grid[i];
	}
	delete[] grid;
}

int IsInArrayBounds(Point p)
{
	// Returns "true" if x and y are both in-bounds.
	return IsInArrayBounds(p.x, p.y);
}

int IsInArrayBounds(int x, int y)
{
	// Returns "true" if x and y are both in-bounds.
	if (x < 0 || x >= GRID_WIDTH) return false;
	if (y < 0 || y >= GRID_HEIGHT) return false;
	return true;
}

PathKey MakeKey(LocationID id1, LocationID id2) {
	return std::make_pair(std::min(id1, id2), std::max(id1, id2));
}