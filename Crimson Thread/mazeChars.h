const int GRID_SIZE = 51;
const int GRID_WIDTH = GRID_SIZE;
const int GRID_HEIGHT = GRID_SIZE;
const int SUBGRID_SIZE = GRID_SIZE / 4;

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