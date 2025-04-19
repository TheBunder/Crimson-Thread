//----INCLUDES--------------------------------------------------------
#include "MazeBasics.h"

//----FUNCTIONS-------------------------------------------------------
int IsInArrayBounds(int x, int y)
{
	// Returns "true" if x and y are both in-bounds.
	if (x < 0 || x >= GRID_WIDTH) return false;
	if (y < 0 || y >= GRID_HEIGHT) return false;
	return true;
}