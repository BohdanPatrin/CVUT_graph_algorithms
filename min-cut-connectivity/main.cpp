#ifndef __PROGTEST__
#include <cassert>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <algorithm>
#include <bitset>
#include <list>
#include <array>
#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>
#include <compare>
#include <ranges>
#include <optional>
#include <variant>

using House = size_t;
using Road = std::pair<House, House>;

namespace student_namespace {
#endif

    struct DeliverySolver {
        int n;
        std::vector<int> head, to, rev, res;
        std::vector<int> level, visited;
        int current_gen = 0;
        std::vector<int> ptr, q, path, edge_path;
        std::vector<int> modified_edges;
        std::vector<int> min_cuts;

        DeliverySolver(unsigned houses, const std::vector<Road>& roads) {
            n = houses;

            // buigcsr graph
            head.assign(n + 1, 0);
            for (const auto& r : roads) {
                if (r.first == r.second) continue;
                head[r.first + 1]++;
                head[r.second + 1]++;
            }
            for (int i = 0; i < n; ++i)
                head[i + 1] += head[i];

            int E = head.back();
            to.resize(E);
            rev.resize(E);
            res.assign(E, 1);

            std::vector<int> cur_head = head;
            for (const auto& r : roads) {
                int u = r.first, v = r.second;
                if (u == v) continue;
                int e1 = cur_head[u]++;
                int e2 = cur_head[v]++;
                to[e1] = v; rev[e1] = e2;
                to[e2] = u; rev[e2] = e1;
            }

            // preallocate dfs/bfs memory pools
            level.resize(n);
            visited.assign(n, 0);
            ptr.resize(n);
            q.resize(n);
            path.reserve(n);
            edge_path.reserve(n);
            min_cuts.assign(n, 0);

            build_gomory_hu();
        }

        bool bfs(int s, int t, int& tail) {
            if (++current_gen == 2000000000) {
                std::fill(visited.begin(), visited.end(), 0);
                current_gen = 1;
            }

            visited[s] = current_gen;
            level[s] = 0;
            int head_idx = 0;
            tail = 0;
            q[tail++] = s;

            while (head_idx < tail) {
                int u = q[head_idx++];
                for (int e = head[u]; e < head[u + 1]; ++e)
                    if (res[e] > 0) {
                        int v = to[e];
                        if (visited[v] != current_gen) {
                            visited[v] = current_gen;
                            level[v] = level[u] + 1;
                            q[tail++] = v;
                        }
                    }

            }
            return visited[t] == current_gen;
        }

        int dfs_iterative(int s, int t) {
            path.clear();
            edge_path.clear();
            int curr = s;
            int pushed = 0;

            while (true) {
                if (curr == t) {
                    // saturated path - update residues and track modification for fast reset
                    for (int e : edge_path) {
                        res[e] -= 1;
                        res[rev[e]] += 1;
                        modified_edges.push_back(e);
                        modified_edges.push_back(rev[e]);
                    }
                    pushed += 1;
                    curr = s; // backtrack to source
                    path.clear();
                    edge_path.clear();
                    continue;
                }

                bool advanced = false;
                while (ptr[curr] < head[curr + 1]) {
                    int e = ptr[curr];
                    if (res[e] > 0) {
                        int v = to[e];
                        if (visited[v] == current_gen && level[v] == level[curr] + 1) {
                            path.push_back(curr);
                            edge_path.push_back(e);
                            curr = v;
                            advanced = true;
                            break;
                        }
                    }
                    ptr[curr]++;
                }

                if (!advanced) {
                    if (curr == s) break; // network completely saturated
                    curr = path.back(); // dead end
                    path.pop_back();
                    edge_path.pop_back();
                    ptr[curr]++;
                }
            }
            return pushed;
        }

        int compute_max_flow(int s, int t) {
            int flow = 0;
            int tail = 0;

            while (bfs(s, t, tail)) {
                for (int i = 0; i < tail; ++i)
                    ptr[q[i]] = head[q[i]];

                flow += dfs_iterative(s, t);
            }

            // only reset edges that were actually touched
            for (int e : modified_edges)
                res[e] = 1;

            modified_edges.clear();

            return flow;
        }

        void build_gomory_hu() {
            if (n <= 1) return;

            std::vector<int> p(n, 0);
            std::vector<int> cap(n, 0);

            // Gusfield's algorithm
            for (int s = 1; s < n; ++s) {
                int t = p[s];
                int flow = compute_max_flow(s, t);
                cap[s] = flow;

                for (int i = s + 1; i < n; ++i)
                    if (p[i] == t && visited[i] == current_gen) p[i] = s;

            }

            // build cut-tree
            std::vector<std::vector<std::pair<int, int>>> tree_adj(n);
            for (int i = 1; i < n; ++i) {
                tree_adj[i].push_back({p[i], cap[i]});
                tree_adj[p[i]].push_back({i, cap[i]});
            }

            min_cuts[0] = std::numeric_limits<int>::max();
            std::queue<int> tq;
            tq.push(0);

            std::vector<bool> tree_visited(n, false);
            tree_visited[0] = true;

            // bfs on the cut-tree
            while (!tq.empty()) {
                int u = tq.front();
                tq.pop();
                for (auto& edge : tree_adj[u]) {
                    int v = edge.first;
                    int w = edge.second;
                    if (!tree_visited[v]) {
                        tree_visited[v] = true;
                        min_cuts[v] = std::min(min_cuts[u], w);
                        tq.push(v);
                    }
                }
            }
        }

        std::vector<bool> solve(unsigned policemen) const {
            std::vector<bool> result(n);
            for (int i = 0; i < n; ++i)
                result[i] = (min_cuts[i] > (int)policemen);

            return result;
        }
    };

#ifndef __PROGTEST__
}



struct Test {
  unsigned houses;
  std::vector<Road> roads;
  std::vector<std::pair<unsigned, std::vector<bool>>> solution;
};

const std::vector<Test> examples = {
  Test{
    6,
    { {0,1}, {0,2}, {1,2}, {2,3}, {3,4}, {0,5}, {1,5} },
    {
      { 0, { 1, 1, 1, 1, 1, 1 } },
      { 1, { 1, 1, 1, 0, 0, 1 } },
      { 2, { 1, 1, 0, 0, 0, 0 } },
      { 3, { 1, 0, 0, 0, 0, 0 } },
      { 100, { 1, 0, 0, 0, 0, 0 } },
    }
  },
//{ {: gen/examples-gen.inc }}
};


int main() {
  for (const auto& [ h, r, l ] : examples) {
    student_namespace::DeliverySolver solver(h, r);
    for (const auto& [ p, ref ] : l) {
      std::vector<bool> sol = solver.solve(p);
      assert(sol == ref);
    }
  }

  std::cout << "All tests passes" << std::endl;
}

#endif
