#include "../include/Unit.h"
#include "../include/Utils.h"
#include <cstdlib> // For rand()
#include <ctime>   // For time()

bool isLegal(int x, int y, char** grid);

Unit::Unit(int startX, int startY, char generetOn){
	x = startX;
	y = startY;
	standOn = generetOn;
	currentDirection = rand() % 4;
}

int Unit::getX() const {
	return x;
}

int Unit::getY() const {
	return y;
}

void Unit::move(char** grid) {
	//(d%2)*(d<2?1:-1)
	//(!(d%2))*(d<2?1:-1)
	
	int newX = x + (currentDirection % 2) * (currentDirection < 2 ? 1 : -1);
	int newY = y + (1^(currentDirection % 2)) * (currentDirection < 2 ? 1 : -1);



	// 70% chance to continue in the same direction
	if (rand() % 100 < 80 && isLegal(newX, newY, grid)) {
		// Continue in the current direction
	}
	else {
		// Generate a new random direction
		int start = rand() % 3 + 1;
		int flag = 1;
		int newDirection;
		for (int i = 0; i < 3 && flag; i++)
		{
			newDirection = (currentDirection + start + i) % 4;
			newX = x + (newDirection % 2) * (newDirection < 2 ? 1 : -1);
			newY = y + (1 ^ (newDirection % 2)) * (newDirection < 2 ? 1 : -1);
			if (isLegal(newX, newY, grid)) {
				flag = 0;
				currentDirection = newDirection;
			}
		}
	}

	// Check if the new position is within the maze boundaries and is a space
	if (isLegal(newX, newY, grid)) {
		grid[x][y] = standOn;
		x = newX;
		y = newY;
		standOn = grid[x][y];
		grid[x][y] = '2';
	}
	// If the move is invalid, the unit stays in its current position and might try a new direction next step.
}

bool isLegal(int x, int y, char** grid) {
	return x > 0 && x < GRID_WIDTH - 1 && y > 0 && y < GRID_HEIGHT - 1 &&
		(grid[x][y] == ' ' || grid[x][y] == '@' || grid[x][y] == 'R');
}