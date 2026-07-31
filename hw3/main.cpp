#ifndef __PROGTEST__
#include <cassert>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <algorithm>
#include <functional>
#include <bitset>
#include <list>
#include <array>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>
#include <compare>

struct Vault {
    unsigned value;
    unsigned pebbles;
    std::vector<std::pair<unsigned, unsigned>> missing_connections;
};

struct UnlockingSequence {
    unsigned vault_id;
    std::vector<bool> moved_pebbles;
};

#endif

// --- helper structures ---

struct ValidVault {
    unsigned original_id;
    unsigned value;
    unsigned cost; // pebbles/2
    std::vector<int> solution_components; // component indices that form the solution
    std::vector<std::vector<int>> components; // component mapping to reconstruct the full boolean vector later
    unsigned total_pebbles;
};

std::vector<UnlockingSequence> plan_heist(
        const std::vector<Vault>& vaults,
        unsigned transition_time,
        unsigned max_time
) {
    std::vector<ValidVault> valid_vaults;

    // go through each vault
    for (size_t i = 0; i < vaults.size(); ++i) {
        const auto& v = vaults[i];

        // pebbles must be even to be split exactly in half
        if (v.pebbles % 2 != 0) continue;
        unsigned target = v.pebbles / 2;

        // build adjacency for missing connections
        std::vector<std::vector<unsigned>> adj(v.pebbles);
        for (const auto& pair : v.missing_connections) {
            adj[pair.first].push_back(pair.second);
            adj[pair.second].push_back(pair.first);
        }

        // find connected components
        std::vector<std::vector<int>> components;
        std::vector<bool> visited(v.pebbles, false);
        for (unsigned p = 0; p < v.pebbles; ++p) {
            if (visited[p]) continue;
            std::vector<int> component;
            std::queue<unsigned> q;
            q.push(p);
            visited[p] = true;
            component.push_back(p);
            while(!q.empty()) {
                unsigned curr = q.front();
                q.pop();
                for (unsigned neighbor : adj[curr])
                    if (!visited[neighbor]) {
                        visited[neighbor] = true;
                        q.push(neighbor);
                        component.push_back(neighbor);
                    }
            }
            components.push_back(std::move(component));
        }

        // solving subset sum
        std::vector<int> ss_parent(target + 1, -2); // -2: unreachable, -1: start
        ss_parent[0] = -1;

        for (size_t comp_idx = 0; comp_idx < components.size(); ++comp_idx) {
            size_t size = components[comp_idx].size();
            for (int w = target; w >= (int)size; --w)
                if (ss_parent[w - size] != -2 && ss_parent[w] == -2)
                    ss_parent[w] = comp_idx;
        }

        // target is reachable -> reconstruct the component list
        if (ss_parent[target] != -2) {
            std::vector<int> sol_comps;
            int curr = target;
            while (curr > 0) {
                int comp_idx = ss_parent[curr];
                sol_comps.push_back(comp_idx);
                curr -= components[comp_idx].size();
            }
            valid_vaults.push_back({
                                           (unsigned)i,
                                           v.value,
                                           target,
                                           std::move(sol_comps),
                                           std::move(components),
                                           v.pebbles
                                   });
        }
    }

    // knapsack problem
    unsigned long long effective_capacity = (unsigned long long)max_time + transition_time;

    std::vector<long long> dp(effective_capacity + 1, -1);
    dp[0] = 0;

    std::vector<std::vector<bool>> keep(valid_vaults.size(), std::vector<bool>(effective_capacity + 1, false));

    for (size_t i = 0; i < valid_vaults.size(); ++i) {
        unsigned weight = valid_vaults[i].cost + transition_time;
        unsigned value = valid_vaults[i].value;

        for (unsigned long long w = effective_capacity; w >= weight; --w)
            if (dp[w - weight] != -1) {
                long long new_val = dp[w - weight] + value;
                if (new_val > dp[w]) {
                    dp[w] = new_val;
                    keep[i][w] = true;
                }
            }
    }

    // reconstruct solution
    long long max_val = -1;
    unsigned long long best_w = 0;
    for (unsigned long long w = 0; w <= effective_capacity; ++w)
        if (dp[w] > max_val) {
            max_val = dp[w];
            best_w = w;
        }


    std::vector<UnlockingSequence> result;
    if (max_val <= 0) return result;

    // backtrack to find chosen vaults
    for (int i = valid_vaults.size() - 1; i >= 0; --i)
        if (keep[i][best_w]) {
            const auto& vv = valid_vaults[i];

            // reconstruct boolean vector for this vault
            std::vector<bool> moved(vv.total_pebbles, false);
            for (int comp_idx : vv.solution_components)
                for (int pebble_idx : vv.components[comp_idx])
                    moved[pebble_idx] = true;

            result.push_back({ vv.original_id, std::move(moved) });
            best_w -= (vv.cost + transition_time);
        }

    // reverse because knapsack backtrack returns solution in reverse order
    std::reverse(result.begin(), result.end());

    return result;
}

#ifndef __PROGTEST__

struct TestFailed : std::runtime_error {
    using std::runtime_error::runtime_error;
};

#define CHECK(cond, msg) do { \
    if (!(cond)) throw TestFailed(msg); \
  } while (0)

void check_unlocking_sequence(
        unsigned pebbles,
        const std::vector<std::pair<unsigned, unsigned>>& missing_connections,
        const std::vector<bool>& moved
) {
    CHECK(moved.size() == pebbles, "Solution has wrong size.\n");

    size_t moved_cnt = 0;
    for (bool p : moved) moved_cnt += p;
    CHECK(2*moved_cnt == pebbles,
          "Exactly half of the pebbles must be moved.\n");

    for (auto [ u, v ] : missing_connections) CHECK(moved[u] == moved[v],
                                                    "Pebble not connected with all on other side.\n");
}

void check_solution(
        const std::vector<UnlockingSequence>& solution,
        unsigned expected_value,
        const std::vector<Vault>& vaults,
        unsigned transition_time,
        unsigned max_time
) {
    unsigned time = 0, value = 0;
    std::vector<bool> robbed(vaults.size(), false);

    for (size_t i = 0; i < solution.size(); i++) {
        const auto& [ id, moved ] = solution[i];

        CHECK(id < vaults.size(), "Id is out of range.\n");
        CHECK(!robbed[id], "Robbed same vault twice.\n");
        robbed[id] = true;

        const auto& vault = vaults[id];
        value += vault.value;

        if (i != 0) time += transition_time;
        time += vault.pebbles / 2;
        CHECK(time <= max_time, "Run out of time.\n");

        check_unlocking_sequence(vault.pebbles, vault.missing_connections, moved);
    }

    CHECK(value == expected_value, "Total value mismatch.\n");
}


struct Test {
    unsigned expected_value;
    unsigned max_time;
    unsigned transition_time;
    std::vector<Vault> vaults;
};

inline const std::vector<Test> TESTS = {
        Test{
                .expected_value = 3010, .max_time = 3, .transition_time = 8,
                .vaults = {
                        { .value = 3010, .pebbles = 6, .missing_connections = { {3,4}, {0,1}, {4,5}, {5,3}, } },
                        { .value = 3072, .pebbles = 6, .missing_connections = { {2,1}, {1,3}, {0,1}, {0,3}, {4,5}, {2,3}, } },
                        { .value = 5069, .pebbles = 10, .missing_connections = { {7,2}, {3,4}, {0,1}, {8,4}, {1,2}, {8,3}, {7,0}, {5,6}, {9,5}, {9,6}, } },
                        { .value = 2061, .pebbles = 4, .missing_connections = { {3,0}, {2,1}, {0,2}, {1,3}, } },
                }
        },
        Test{
                .expected_value = 6208, .max_time = 13, .transition_time = 12,
                .vaults = {
                        { .value = 6011, .pebbles = 12, .missing_connections = { {1,5}, {2,4}, {5,10}, {1,10}, {0,3}, {8,3}, {8,0}, {9,8}, {2,6}, {3,9}, {0,9}, {4,6}, {11,7}, } },
                        { .value = 2056, .pebbles = 4, .missing_connections = { {1,0}, {2,0}, {2,1}, } },
                        { .value = 5885, .pebbles = 12, .missing_connections = { {1,6}, {3,7}, {1,0}, {2,9}, {9,8}, {2,8}, {5,7}, {11,4}, {10,1}, {5,3}, {0,10}, } },
                        { .value = 5818, .pebbles = 12, .missing_connections = { {9,0}, {7,1}, {6,4}, {8,6}, {4,2}, {11,5}, {5,3}, {9,7}, {8,4}, {2,8}, {10,11}, {5,10}, {10,3}, {9,1}, } },
                        { .value = 4880, .pebbles = 10, .missing_connections = { {7,3}, {4,1}, {9,2}, {6,9}, {2,6}, {5,0}, {8,4}, } },
                        { .value = 5233, .pebbles = 10, .missing_connections = { {0,2}, {4,5}, {8,3}, {9,7}, {7,1}, {6,3}, {6,8}, } },
                        { .value = 6208, .pebbles = 12, .missing_connections = { {1,7}, {3,4}, {10,7}, {0,3}, {8,2}, {5,1}, {9,11}, {0,6}, {6,3}, {10,1}, {0,4}, } },
                        { .value = 4182, .pebbles = 8, .missing_connections = { {5,7}, {7,4}, {4,5}, {1,0}, {5,6}, {3,1}, {6,4}, {0,3}, {6,7}, } },
                }
        },
        Test{
                .expected_value = 1, .max_time = 100, .transition_time = 8,
                .vaults = {
                        { .value = 1, .pebbles = 14, .missing_connections = {
                                {0,1}, {0,2}, {3,4}, {4,5}, {6,7}, {8,9}, {10,11}, {12,13},
                        } },
                }
        },
};

int main() {
    int ok = 0, fail = 0;

    for (auto t : TESTS) {
        try {
            auto sol = plan_heist(t.vaults, t.transition_time, t.max_time);
            check_solution(sol, t.expected_value, t.vaults, t.transition_time, t.max_time);
            ok++;
        } catch (const TestFailed&) {
            fail++;
        }
    }

    if (!fail) std::cout << "Passed all " << ok << " tests!" << std::endl;
    else std::cout << "Failed " << fail << " of " << (ok + fail) << " tests." << std::endl;
}

#endif


