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

// Define a key type for our parent map
typedef std::pair<int, int> Point;

void ExpandNeighbors(const vector<int>& current_node, std::array<int, 2> goal,
    vector<vector<int>>& open_nodes, char** grid, std::map<Point, Point>& parent_map)
{
    int x = current_node[0];
    int y = current_node[1];
    int g = current_node[2];
    Point current_point(x, y);

    vector<vector<int>> possibleMovements{ {1, 0}, {0, 1}, {-1, 0}, {0, -1} };

    for (vector<int> movement : possibleMovements)
    {
        int x_test = x + movement[0];
        int y_test = y + movement[1];
        if (CheckValidCell(x_test, y_test, grid))
        {
            int g_test = g + 1;
            int h = Heuristic(x_test, y_test, goal[0], goal[1]);
            // Store parent relationship before adding to open list
            parent_map[Point(x_test, y_test)] = current_point;
            AddToOpen(x_test, y_test, g_test, h, open_nodes, grid);
        }
    }
}

vector<std::array<int, 2>> ReconstructPath(std::map<Point, Point>& parent_map,
    std::array<int, 2> start,
    std::array<int, 2> goal)
{
    vector<std::array<int, 2>> path;
    Point current(goal[0], goal[1]);
    Point start_point(start[0], start[1]);

    // Add goal to path
    path.push_back({ current.first, current.second });

    // Work backwards from goal to start
    while (current != start_point && parent_map.find(current) != parent_map.end())
    {
        current = parent_map[current];
        path.push_back({ current.first, current.second });
    }

    // Reverse to get path from start to goal
    std::reverse(path.begin(), path.end());

    return path;
}

vector<std::array<int, 2>> Search(char** grid, std::array<int, 2> start, std::array<int, 2> goal)
{
    vector<vector<int>> open_nodes{};
    std::map<Point, Point> parent_map;

    int x = start[0];
    int y = start[1];
    int g = 0;
    int h = Heuristic(x, y, goal[0], goal[1]);
    AddToOpen(x, y, g, h, open_nodes, grid);

    while (!open_nodes.empty())
    {
        CellSort(&open_nodes);
        vector<int> current_node = open_nodes.back();

        open_nodes.pop_back(); // remove last element from vector
        x = current_node[0];
        y = current_node[1];
        grid[x][y] = State::kSearched;

        if (x == goal[0] && y == goal[1])
        {
            // Path found, reconstruct it
            return ReconstructPath(parent_map, start, goal);
        }
        ExpandNeighbors(current_node, goal, open_nodes, grid, parent_map);
    }

    // We've run out of new nodes to explore and haven't found a path.
    printf("No path found!\n");

    return vector<std::array<int, 2>>(); // Return empty vector if no path found
}

vector<std::array<int, 2>> AStar(char** grid, char** path, std::array<int, 2> start, std::array<int, 2> goal)
{
    // convert grid to path
    for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if ((unsigned char)grid[x][y] > 100) {
                path[x][y] = kObstacle;
            }
            else {
                path[x][y] = kEmpty;
            }
        }
    }

    // Return the path as a vector of points
    return Search(path, start, goal);
}