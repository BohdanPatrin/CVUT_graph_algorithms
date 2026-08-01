Advanced C++ Algorithms & Data Structures

This repository contains highly optimized C++ solutions to complex algorithmic problems, developed to run under strict memory and execution time limits (evaluated via the Progtest platform). 

Each project demonstrates the application of advanced data structures, graph theory, dynamic programming and network flow to solve multi-layered problems efficiently.

## Projects

*   **[State-Space Search (Shortest Path)](./state-space-search)**
    *   **Concepts:** Breadth-First Search (BFS), State-Space Modeling, Graph Traversal.
    *   **Description:** An optimized pathfinding algorithm that navigates a dynamic state-space graph, calculating combat mechanics, equipment combinations, and stealth states to find the absolute shortest survival path.

*   **[Lazy Propagation Tree (Range Queries)](./lazy-propagation-tree)**
    *   **Concepts:** Custom Balanced Binary Search Tree, Lazy Propagation, Range Minimum Queries (RMQ).
    *   **Description:** A highly efficient data structure built from scratch to process dynamic range updates and enforce strict negative-value constraints across grouped entities in logarithmic time.

*   **[Knapsack & Graph Components (Vault Heist)](./knapsack-and-graphs)**
    *   **Concepts:** Connected Components (BFS), Subset Sum (DP), 0/1 Knapsack (DP).
    *   **Description:** A multi-stage algorithm that isolates graph components to solve a partition problem, followed by a time-constrained 0/1 Knapsack optimization to maximize extracted value.
 
*   **[Network Flow: Global Min-Cut Connectivity](./min-cut-connectivity)**
    *   **Concepts:** Max-Flow / Min-Cut, Dinic's Algorithm, Gusfield's Algorithm, Compressed Sparse Row (CSR).
    *   **Description:** An optimized network flow engine that constructs a global minimum-cut tree (Gomory-Hu) using a memory-optimized graph structure. It flattens recursive depth-first searches to determine absolute edge connectivity between nodes, allowing for $O(1)$ query resolution.

## Tech Stack
*   **Language:** C++20
*   **Build System:** CMake / Make
