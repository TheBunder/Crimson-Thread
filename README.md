## Crimson-Thread

This project simulates the exploration of a maze by multiple suldgiers, where each suldger group aims to people located at various points within the maze while keaping themself safe. 

This project is being developed as part of a 13th-grade academic undertaking. The current version is a work in progress, and significant development remains.

# Project Goals:

* Implement a multi-agent system for maze exploration.
* Utilize the A* search algorithm for efficient pathfinding.
* Implement a centralized task assignment mechanism.
* Evaluate the performance of the system in terms of total rewards collected and exploration efficiency.

# Key Features:

* **Maze Representation:** 
    * Utilizes a 2D array to represent the maze.
* **Pathfinding:** 
    * Implements the A* search algorithm for efficient pathfinding.
* **Reward Mechanism:** 
    * Each reward station has a probability of yielding a specific reward.
* **Centralized Control:** 
    * A central controller assigns tasks to agents based on reward probabilities and proximity.
* **Agent Coordination:** 
    * Agents operate independently, but the central controller coordinates their actions.

# Current Progress:

* **Maze Generation:** The maze generation component has been implemented.

**To-Do List:**

* [ ] Implement A* search algorithm.
* [ ] Implement reward mechanism.
* [ ] Implement central controller and task assignment logic.
* [ ] Implement agent behavior (pathfinding, reward collection).
* [ ] Conduct simulations and analyze results.

# Future Work:

* Explore alternative pathfinding algorithms (e.g., Dijkstra's algorithm).
* Implement more sophisticated agent coordination mechanisms.
* Investigate the impact of different reward distributions.
* Extend the simulation to handle dynamic environments.

# Screenshot

![MazeResized](https://github.com/user-attachments/assets/b4f763be-28fe-47c0-bb6d-e7e3e66a41de)
