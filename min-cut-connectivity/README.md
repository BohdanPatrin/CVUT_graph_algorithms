# Network Flow: Global Min-Cut Connectivity

## Overview
This project solves a highly complex network resilience problem: determining the absolute edge connectivity between a central node (a source) and every other node (sinks) in an undirected graph. Given a variable threshold of "blocked" edges (policemen guarding roads), the algorithm calculates in $O(1)$ time per query whether a node is disconnected from the source. To achieve this, it constructs a global minimum-cut tree during initialization.

## Key Features
*   **Gusfield’s Algorithm (Gomory-Hu Tree):** Constructs an equivalent cut-tree for the graph, caching the minimum cut between all pairs of nodes. This reduces the problem from calculating max-flow for every query to simply traversing a pre-built bottleneck tree.
*   **Dinic’s Algorithm for Max-Flow:** Solves the core Max-Flow / Min-Cut problem using an optimized level graph generation (BFS) and a custom iterative Depth-First Search (DFS) for path augmentation.
*   **Memory-Optimized Graph (CSR):** To meet strict automated memory and time limits, the standard `std::vector<std::vector<int>>` adjacency list was replaced with a flattened Compressed Sparse Row (CSR) structure (`head`, `to`, `rev`, `res`), significantly improving CPU cache locality.
*   **Iterative State Machine:** The standard recursive DFS used in Dinic's algorithm was flattened into a manual stack (`path`, `edge_path`) to prevent stack overflow limits and speed up execution time.

## Code Highlight: Fast Network Reset
Because Gusfield's algorithm requires running Dinic's max-flow up to $N-1$ times on the same graph, resetting the network's edge residues (capacities) using a full $O(E)$ loop every time is too slow. The engine tracks exactly which edges were saturated and rolls them back in $O(\text{Modified})$, leading to massive performance gains.

```cpp
int compute_max_flow(int s, int t) {
    int flow = 0;
    int tail = 0;

    // Standard Dinic's Level Graph Loop
    while (bfs(s, t, tail)) {
        for (int i = 0; i < tail; ++i)
            ptr[q[i]] = head[q[i]]; // Reset dynamic edge pointers

        flow += dfs_iterative(s, t); // Pushes flow and logs modifications
    }

    // FAST RESET: Only reset edges that were actually touched
    for (int e : modified_edges)
        res[e] = 1;

    modified_edges.clear();
    return flow;
}
```

## System Architecture
1.  **Graph Construction:** Takes a list of bidirectional roads and flattens them into a CSR format. Edges are paired with their reverse counterparts for residual flow tracking.
2.  **Tree Building:** Iterates through $N-1$ nodes, computing the maximum flow from the current node $s$ to its designated parent $t$ in the evolving cut-tree.
3.  **Cut Caching:** Once the Gusfield cut-tree is generated, a final Breadth-First Search traverses the tree from Node $0$ (the central source). It records the absolute minimum edge weight (the bottleneck cut) along the path to every other node.
4.  **$O(1)$ Resolution:** When queried with a number of blocked edges (`policemen`), the `solve()` method simply compares the pre-calculated minimum cut against the threshold, returning a fast boolean vector.

## Tech Stack
*   **Language:** C++20
*   **Concepts:** Graph Theory, Max-Flow / Min-Cut, Dinic's Algorithm, Gusfield's Algorithm, Gomory-Hu Trees, Cache Optimization
