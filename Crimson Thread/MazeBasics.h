#ifndef MAZE_BASICS
#define MAZE_BASICS
//----INCLUDES-------------------------------------------------------
#include <array>
#include <vector>
#include <stdio.h>

using std::vector;
//----CONSTANTS------------------------------------------------------

const int GRID_SIZE = 101;
const int GRID_WIDTH = GRID_SIZE;
const int GRID_HEIGHT = GRID_SIZE;
const int SUBGRID_SIZE = GRID_SIZE / 4;
const char	WALL = 219;			// █
const char	SPACE = 32;			// | |<- Space
const char	HOSTAGES = 64;		// @
const char	RoutePoint = 82;    // R

//----ENUM-----------------------------------------------------------
enum MazeChar : char {
    TopLeftCorner = 201/*╔*/,
    TopRightCorner = 187/*╗*/,
    BottomLeftCorner = 200/*╚*/,
    BottomRightCorner = 188/*╝*/,
    HorizontalWall = 205/*═*/,
    VerticalWall = 186/*║*/,
    RightTee = 204/*╠*/,
    BottomTee = 203/*╦*/,
    TopTee = 202/*╩*/,
    LeftTee = 185/*╣*/,
    Cross = 206/*╬*/
};

enum State : char
{
    kEmpty,
    kObstacle,
    kClosed,
    kSearched,
    kStart,
    kFinish,
    kPath
};

//----FUNCTION DECLARATIONS------------------------------------------
int IsInArrayBounds(int x, int y);

#endif MAZE_BASICS