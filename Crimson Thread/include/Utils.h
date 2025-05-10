#ifndef MAZE_BASICS
#define MAZE_BASICS
//----INCLUDES-------------------------------------------------------
#include <array>
#include <vector>
#include <map>
//----NAMESPACES------------------------------------------------------
using namespace std;
using std::vector;
using std::map;
using std::pair;

// Key: Pair of IDs (Station/Start Point). Ensure consistent ordering (e.g., smaller ID first).
// Special ID for start points (units IDs are index*-1 ; station ID are index+1)
using LocationID = int;
using PathKey = pair<LocationID, LocationID>;

//----CONSTANTS------------------------------------------------------
const int GRID_SIZE = 101;
const int GRID_WIDTH = GRID_SIZE;
const int GRID_HEIGHT = GRID_SIZE;
const int NUM_SUBGRIDS_PER_DIMENSION = 4;
const int SUBGRID_SIZE = GRID_SIZE / NUM_SUBGRIDS_PER_DIMENSION;
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
    Point() : x(0), y(0) {} // Initialize x and y to some default values

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
char** AllocateGrid();
void DeallocateGrid(char** grid);
int IsInArrayBounds(int x, int y);
PathKey MakeKey(LocationID id1, LocationID id2); // Function to ensure consistent key ordering

#endif