﻿#ifndef UTILS
#define UTILS
//----INCLUDES-------------------------------------------------------
#include <array>
#include <vector>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
//----NAMESPACES------------------------------------------------------
// using namespace std;
using std::vector;
using std::swap;
using std::thread;
using std::mutex;
using std::map;
using std::pair;
using std::queue;

// Key: A pair of IDs (Station/Start Point). Ensure consistent ordering (e.g., smaller ID first).
// Special ID for start points (units IDs are index*-1 ; station ID are index+1)
using LocationID = int;
using PathKey = pair<LocationID, LocationID>;

//----CONSTANTS------------------------------------------------------
const int GRID_SIZE = 101;
const int GRID_WIDTH = 201;
const int GRID_HEIGHT = 51;
const int SUBGRID_SIZE = 25;
const char	WALL = 219;			// █
const char	PATH = 32;			// | |<- Space
const char	HOSTAGES = 64;		// @
const char	UNIT = 85;		    // U

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
    Point() : x(-1), y(-1) {} // Initialize x and y to some default values

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
char** AllocateGrid(); // Allocate the grid used for the simulation
void DeallocateGrid(char** grid); // Deallocate the grid used for the simulation
int IsInArrayBounds(Point p); // Check if the point is in the array
int IsInArrayBounds(int x, int y); // Check if the x and y are in the array
PathKey MakeKey(LocationID id1, LocationID id2); // Function to ensure consistent key ordering

#endif // UTILS