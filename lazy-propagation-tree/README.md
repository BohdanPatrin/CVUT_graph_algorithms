# Lazy Propagation Tree: Dynamic Range Updates & Queries

## Overview
This project implements a highly optimized, custom-built balanced Binary Search Tree (BST) from scratch to manage a dynamic collection of entities. Designed to handle strict time complexities, the data structure efficiently processes range updates and range minimum queries (RMQ) in logarithmic time. It strictly enforces system constraints, validating that massive batch updates never cause any individual entity's resource pool to drop below zero.

## Key Features
*   **Custom Balanced BST:** Implements a self-balancing tree structure (similar to an AVL tree) using calculated subtree heights and dynamic left/right rotations to ensure $O(\log N)$ insertion, deletion, and search operations.
*   **Lazy Propagation:** Range updates (e.g., buffing or debuffing a contiguous alphabetical subset of entities) are processed in $O(\log N)$ time by storing modifier tags at higher-level nodes and pushing them down to children only when necessary.
*   **Range Minimum Queries (RMQ):** The tree dynamically tracks the absolute minimum health (`min_hp`) within every subtree. This allows the engine to predict if a batch debuff will kill any entity in $O(\log N)$ time, safely aborting the operation before any invalid modifications are made.
*   **Zero Memory Leaks:** Implements strict manual memory management utilizing custom destructors and subtree recursive deletion to handle raw pointers without relying on smart pointers.

## Code Highlight: Lazy Evaluation & State Tracking
To achieve logarithmic time complexity for batch updates, the tree nodes store "lazy" variables (`l_hp`, `l_off`, `l_def`). These values are only applied to child nodes when the tree is actively traversed, preventing the need for $O(N)$ full-tree iterations.

```cpp
struct Node {
    std::string name;
    int hp, off, def;
    
    // Tracks the minimum HP in this subtree for O(log N) validation
    int min_hp; 

    // Lazy propagation modifiers
    int l_hp = 0;
    int l_off = 0;
    int l_def = 0;

    Node *left = nullptr;
    Node *right = nullptr;
    Node *parent = nullptr;
    int height = 1;
};

// Pushes lazy modifiers down to children before traversing
void push(Node* n) {
    if (!n) return;
    if (n->l_hp == 0 && n->l_off == 0 && n->l_def == 0) return;

    apply(n->left, n->l_hp, n->l_off, n->l_def);
    apply(n->right, n->l_hp, n->l_off, n->l_def);

    // Reset current node's tags after pushing
    n->l_hp = 0;
    n->l_off = 0;
    n->l_def = 0;
}
```

## System Architecture
1.  **Node Maintenance:** Every modification (insertion, deletion, or range update) triggers a recursive `updateStats()` call to recalculate tree height and the `min_hp` of the affected branches.
2.  **Tree Balancing:** The `balanceNode()` function evaluates the balance factor of subtrees. If a branch becomes too heavy (balance > 1 or < -1), it resolves the imbalance through targeted `rotateLeft()` or `rotateRight()` pointer reassignment.
3.  **Constraint Enforcement:** Before applying a negative health modifier across a range, `getRangeMin()` isolates the target range and evaluates the lowest `min_hp` value. If the operation would drop this value to 0 or below, the entire update is cleanly rejected.
4.  **Traversal Routing:** Tree parsing relies on a custom `for_each` lambda function injector, accumulating localized lazy variables on-the-fly to reconstruct accurate states without modifying the tree.

## Tech Stack
*   **Language:** C++20
*   **Concepts:** Binary Search Trees, AVL Balancing, Lazy Propagation, Range Minimum Query (RMQ), Raw Memory Management
