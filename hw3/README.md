# Knapsack & Graph Components: Vault Heist Optimization

## Overview
This project solves a complex, multi-stage resource allocation and constraint satisfaction problem. The algorithm determines the optimal sequence of vaults to unlock to maximize value within a strict time limit. Unlocking a vault is not guaranteed; each vault requires solving a graph partitioning problem to satisfy unique, localized locking constraints before it can even be considered for the final time-optimization phase.

## Key Features
*   **Graph Partitioning (BFS):** Translates physical lock constraints ("missing connections") into an adjacency list, utilizing a Breadth-First Search queue to isolate independent connected components.
*   **Subset Sum (Dynamic Programming):** Evaluates if the isolated graph components can be perfectly partitioned to meet the vault's $50/50$ unlocking requirement. Solved in $O(N \cdot W)$ time with full parent-state tracking for reconstruction.
*   **0/1 Knapsack (Dynamic Programming):** Maximizes the total extracted value bounded by a global time limit (factoring in mandatory transition times between vaults) using a highly optimized, bottom-up DP array.
*   **State Backtracking:** Successfully reconstructs not just the optimal sequence of vaults to visit, but the precise internal configuration (boolean vectors) required to bypass each specific lock.

## Code Highlight: 0/1 Knapsack with Transition Costs
After filtering out unsolvable vaults via BFS and Subset Sum, the system executes a Knapsack DP algorithm. It uses an `effective_capacity` to elegantly handle variable transition costs between nodes, tracking decisions in a 2D boolean matrix to allow for rapid backtracking.

```cpp
// effective_capacity accounts for the final transition time 
unsigned long long effective_capacity = (unsigned long long)max_time + transition_time;
std::vector<long long> dp(effective_capacity + 1, -1);
dp[0] = 0;

// keep[][] tracks the DP decisions for O(N) backtracking
std::vector<std::vector<bool>> keep(valid_vaults.size(), std::vector<bool>(effective_capacity + 1, false));

for (size_t i = 0; i < valid_vaults.size(); ++i) {
    unsigned weight = valid_vaults[i].cost + transition_time;
    unsigned value = valid_vaults[i].value;

    // Bottom-up 0/1 Knapsack evaluation
    for (unsigned long long w = effective_capacity; w >= weight; --w) {
        if (dp[w - weight] != -1) {
            long long new_val = dp[w - weight] + value;
            if (new_val > dp[w]) {
                dp[w] = new_val;
                keep[i][w] = true;
            }
        }
    }
}
```

## System Architecture
1.  **Component Mapping:** Iterates through every available vault. If a vault has an odd number of locks (making a 50/50 split mathematically impossible), it is immediately discarded. Otherwise, BFS groups the internal mechanisms into rigid components.
2.  **Solvability Check:** Runs a Subset Sum DP to check if the components can be combined to reach exactly `target = pebbles / 2`. Unsolvable vaults are discarded.
3.  **Value Optimization:** The remaining `valid_vaults` are passed into a 0/1 Knapsack algorithm, prioritizing high-value, low-time-cost targets.
4.  **Reconstruction:** The algorithm iterates backward through the `keep` matrix, extracting the chosen vaults and translating the mapped components back into the specific boolean sequence required by the test environment.

## Tech Stack
*   **Language:** C++20
*   **Standard Library Usage:** `<vector>`, `<queue>`, `<algorithm>`
*   **Concepts:** Graph Theory, Connected Components, Breadth-First Search (BFS), Dynamic Programming (Subset Sum, 0/1 Knapsack)
