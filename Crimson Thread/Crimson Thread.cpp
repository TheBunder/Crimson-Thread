#include <stdio.h>
#include <stdlib.h>
#include "MazeGenerator.h"
//----GLOBAL VARIABLES------------------------------------------------
char* grid[GRID_WIDTH];
int main()
{
	for (int i = 0; i < GRID_WIDTH; i++)
	{
		grid[i] = (char*)malloc(GRID_HEIGHT * sizeof(char));
	}
    generate(grid);

	//char a[6][6] = { {221,} }
}