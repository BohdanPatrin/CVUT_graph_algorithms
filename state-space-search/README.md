# State-Space Search: Dynamic Pathfinding & Combat Simulation

## Overview
This project implements an advanced shortest-path algorithm that navigates a complex, multi-dimensional state space. Rather than a simple graph traversal (where nodes are physical locations), a "state" in this engine encompasses the player's physical room, current inventory, stealth status, and objective completion. The algorithm evaluates deterministic combat mechanics and dynamic inventory permutations to find the absolute shortest survival path through the graph.

## Key Features
*   **State-Space BFS:** Utilizes a `std::deque` to perform a highly optimized Breadth-First Search, guaranteeing the shortest sequence of actions (Move, Pickup, Drop) to retrieve the objective and return safely.
*   **Deterministic Combat Engine:** Replaces cycle-heavy combat loops with $O(1)$ mathematical evaluations to determine survivability and turn counts based on base stats, stacking damage, and armor modifiers.
*   **Custom Hashing for Nested Data:** Implements robust custom hash functions (`operator()`) for complex structs (`State`, `Inventory`, `Item`) using bitwise XOR and bit-shifting, allowing them to be efficiently tracked inside a `std::unordered_map`.
*   **Dynamic Inventory Permutations:** The algorithm dynamically evaluates the opportunity cost of swapping items (Weapons, Armor, Stealth Modifiers) by branching new states for every valid item pick-up or drop.

## Code Highlight: Custom State Hashing
To efficiently track visited states and reconstruct the shortest path, the engine relies on a `std::unordered_map`. This required building custom, collision-resistant hash functions for heavily nested structures.

```cpp
struct InventoryHash {
    size_t operator()(const Inventory& inv) const noexcept {
        size_t h1 = std::hash<int>()(inv.weapon ? *inv.weapon : -1);
        size_t h2 = std::hash<int>()(inv.armor  ? *inv.armor  : -1);
        size_t h3 = std::hash<int>()(inv.duck   ? *inv.duck   : -1);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

struct StateHash {
    size_t operator()(const State& s) const noexcept {
        size_t h1 = std::hash<int>()(s.room);
        size_t h2 = InventoryHash()(s.inv);
        size_t h3 = std::hash<bool>()(s.has_treasure);
        size_t h4 = std::hash<bool>()(s.is_sneaking);
        
        // Bitwise shifts and XORs to combine hashes and prevent collisions
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};
```

## System Architecture
## System Architecture
1.  **Environment Graph:** Rooms are represented as nodes connected by bidirectional edges (corridors), populated with optional enemies and loot.
2.  **State Evaluation:** At each node, the BFS generates adjacent states by:
    *   Evaluating adjacent rooms (Move).
    *   Evaluating available items to loot (Pickup).
    *   Evaluating current inventory to discard (Drop).
3.  **Combat Resolution:** Before moving into a room with a monster, the engine calculates if the current inventory stats combined with the hero's base stats result in a victory. If not, the branch is culled unless the hero possesses a stealth modifier.
4.  **Path Reconstruction:** Once a winning state reaches an exit, the sequence of `std::variant<Move, Pickup, Drop>` actions is reconstructed in reverse using the parent tracking map.

## Tech Stack
*   **Language:** C++20
*   **Standard Library Usage:** `<deque>`, `<unordered_map>`, `<variant>`, `<optional>`
*   **Concepts:** Graph Theory, State-Space Search, Deterministic Simulation
