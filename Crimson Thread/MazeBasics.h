#ifndef MAZE_BASICS
#define MAZE_BASICS
//----INCLUDES-------------------------------------------------------
#include <array>
#include <vector>
#include <stdio.h>
//----NAMESPACES------------------------------------------------------
using std::vector;
using namespace std;

//----CONSTANTS------------------------------------------------------

const int GRID_SIZE = 101;
const int GRID_WIDTH = GRID_SIZE;
const int GRID_HEIGHT = GRID_SIZE;
const int SUBGRID_SIZE = GRID_SIZE / 4;
const char	WALL = 219;			// █
const char	SPACE = 32;			// | |<- Space
const char	HOSTAGES = 64;		// @

//----ENUM-----------------------------------------------------------
enum MazeChar : unsigned char {
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

//----STRUCT------------------------------------------------------

typedef struct Point {
    int x;
    int y;

    // Your existing constructor
    Point(int x_val, int y_val) : x(x_val), y(y_val) {}

    // Default constructor (no arguments)
    Point() : x(0), y(0) {} // Initialize x and y to some default values (e.g., 0)

    // Overloaded equality operator (==)
    bool operator==(const Point& other) const {
        return (x == other.x && y == other.y);
    }

    // Overloaded inequality operator (!=)
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }

    // Overloaded less than operator (<)
    bool operator<(const Point& other) const {
        if (x != other.x) {
            return x < other.x;
        }
        return y < other.y;
    }
} Point;

//----FUNCTION DECLARATIONS------------------------------------------
char** allocateGrid();
void deallocateGrid(char** grid);
int IsInArrayBounds(int x, int y);

#endif