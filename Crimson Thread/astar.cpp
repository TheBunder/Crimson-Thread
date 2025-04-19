#include <array>
#include "AStar.h"

/*
Compare f-values of two cells.
*/
bool Compare(const vector<int> node1, const vector<int> node2)
{
	return (node1[2] + node1[3]) > (node2[2] + node2[3]); // is node1 greater then node2
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

void ExpandNeighbors(const vector<int>& current_node, std::array<int, 2> goal,
	vector<vector<int>>& open_nodes, char** grid)
{
	int x = current_node[0];
	int y = current_node[1];
	int g = current_node[2];

	vector<vector<int>> possibleMovements{ {1, 0}, {0, 1}, {-1, 0}, {0, -1} };

	for (vector<int> movement : possibleMovements)
	{
		int x_test = x + movement[0];
		int y_test = y + movement[1];
		if (CheckValidCell(x_test, y_test, grid))
		{
			int g_test = g + 1;
			int h = Heuristic(x_test, y_test, goal[0], goal[1]);
			AddToOpen(x_test, y_test, g_test, h, open_nodes, grid);
		}
	}
}

char** Search(char** grid, std::array<int, 2> start, std::array<int, 2> goal)
{
	vector<vector<int>> open_nodes{};

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
		grid[x][y] = State::kPath;

		if (x == goal[0] && y == goal[1])
		{
			return grid;
		}
		ExpandNeighbors(current_node, goal, open_nodes, grid);
	}

	// We've run out of new nodes to explore and haven't found a path.
	printf("No path found!\n");

	return NULL;
}

void AStar(char** grid, char** path, std::array<int, 2> start, std::array<int, 2> goal)
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

	Search(path, start, goal); // return a grod with the best path
}