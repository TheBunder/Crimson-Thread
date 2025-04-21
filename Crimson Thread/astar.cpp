//----INCLUDES--------------------------------------------------------
#include <map>
#include <utility>  // for std::pair
#include "AStar.h"

//----FUNCTIONS-------------------------------------------------------
/*
Compare f-values of two cells.
*/
bool Compare(const vector<int> node1, const vector<int> node2)
{
    return (node1[2] + node1[3]) > (node2[2] + node2[3]); // is node1 greater than node2
}

/*
Sort the two-dimensional vector of ints in descending order.
*/
void CellSort(vector<vector<int>>* v)
{
    sort(v->begin(), v->end(), Compare);
}

int Heuristic(int x1, int y1, int x2, int y2)
{
    return abs(x2 - x1) + abs(y2 - y1);
}

bool CheckValidCell(int x, int y, char** grid)
{
    //Check point is on grid and empty
    return IsInArrayBounds(x, y) && (grid[x][y] == State::kEmpty);
}

void AddToOpen(int x, int y, int g, int h, vector<vector<int>>& open_nodes, char** grid)
{
    vector<int> node{ x, y, g, h };

    open_nodes.push_back(node);
    grid[x][y] = State::kClosed;
}

void ExpandNeighbors(const vector<int>& current_node, Point goal,
    vector<vector<int>>& open_nodes, char** grid, std::map<Point, Point>& parent_map)
{
    int x = current_node[0];
    int y = current_node[1];
    int g = current_node[2];
    Point current_point = {x, y};

    vector<Point> possibleMovements{ {1, 0}, {0, 1}, {-1, 0}, {0, -1} };

    for (Point movement : possibleMovements)
    {
        int x_test = x + movement.x;
        int y_test = y + movement.y;
        if (CheckValidCell(x_test, y_test, grid))
        {
            int g_test = g + 1;
            int h = Heuristic(x_test, y_test, goal.x, goal.y);
            // Store parent relationship before adding to open list
            parent_map[Point(x_test, y_test)] = current_point;
            AddToOpen(x_test, y_test, g_test, h, open_nodes, grid);
        }
    }
}

vector<Point> ReconstructPath(std::map<Point, Point>& parent_map,
    Point start,
    Point goal)
{
    vector<Point> path;
    Point current = {goal.x, goal.y};
    Point start_point = {start.x, start.y};

    // Add goal to path
    path.push_back({ current.x, current.y });

    // Work backwards from goal to start
    while (current != start_point && parent_map.find(current) != parent_map.end())
    {
        current = parent_map[current];
        path.push_back({ current.x, current.y });
    }

    // Reverse to get path from start to goal
    std::reverse(path.begin(), path.end());

    return path;
}

vector<Point> Search(char** grid, Point start, Point goal)
{
    vector<vector<int>> open_nodes{};
    std::map<Point, Point> parent_map;

    int x = start.x;
    int y = start.y;
    int g = 0;
    int h = Heuristic(x, y, goal.x, goal.y);
    AddToOpen(x, y, g, h, open_nodes, grid);

    while (!open_nodes.empty())
    {
        CellSort(&open_nodes);
        vector<int> current_node = open_nodes.back();

        open_nodes.pop_back(); // remove last element from vector
        x = current_node[0];
        y = current_node[1];
        grid[x][y] = State::kSearched;

        if (x == goal.x && y == goal.y)
        {
            // Path found, reconstruct it
            return ReconstructPath(parent_map, start, goal);
        }
        ExpandNeighbors(current_node, goal, open_nodes, grid, parent_map);
    }

    // We've run out of new nodes to explore and haven't found a path.
    printf("No path found!\n");

    return vector<Point>(); // Return empty vector if no path found
}

vector<Point> AStar(char** grid, Point start, Point goal)
{
    char **navGrid = allocateGrid();

    // convert grid to navGrid
    for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if ((unsigned char)grid[x][y] > 100) {
                navGrid[x][y] = kObstacle;
            }
            else {
                navGrid[x][y] = kEmpty;
            }
        }
    }
    vector<Point> path = Search(navGrid, start, goal);

    deallocateGrid(navGrid);

    // Return the path as a vector of points
    return path;
}