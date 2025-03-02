#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include "MazeGenerator.h"
#include "HostageStation.h"

int main()
{
	char* grid[GRID_WIDTH];
	for (int i = 0; i < GRID_WIDTH; i++)
	{
		grid[i] = (char*)malloc(GRID_HEIGHT * sizeof(char));
	}
	HostageStation* HostageStations = (HostageStation*)malloc((GRID_WIDTH / SUBGRID_SIZE) * (GRID_HEIGHT / SUBGRID_SIZE)
		*sizeof(HostageStation));
    generate(grid, HostageStations);
	PrintGrid(grid);
}