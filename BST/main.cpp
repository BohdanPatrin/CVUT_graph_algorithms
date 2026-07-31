/*
 * # Statically Optimal BST
 *
 * Your task is to implement (template of) the function
 * `optimal_tree` which calculates optimal BST for
 * given data:
 *
 * - Its argument is a sequence of `std::pair<T, uint32_t>` where
 *   `first` is a key of the tree and `second` is relative frequency
 *   of searching for this element.
 * - The probability of searching for a fixed key is its relative
 *   frequency divided by sum of relative frequencies of all keys.
 * - The keys are in the same order as they should appear in the
 *   resulting tree. Comparing the keys should not be needed at all.
 * - The return value is the optimal BST, i.e., BST minimizing
 *   the average number of steps to find a key.
 *
 * The time limits are 9, 5, 6 and 3.5 seconds.
 *
 */

#ifndef __PROGTEST__
#include <cassert>
#include <cstdarg>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <array>
#include <algorithm>
#include <functional>
#include <deque>
#include <queue>
#include <random>
#include <ranges>
#include <type_traits>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <compare>
#include <ranges>


struct TestFailed : std::runtime_error {
    using std::runtime_error::runtime_error;
};

std::string fmt(const char *f, ...) {
    va_list args1;
    va_list args2;
            va_start(args1, f);
    va_copy(args2, args1);

    std::string buf(vsnprintf(nullptr, 0, f, args1), '\0');
            va_end(args1);

    vsnprintf(buf.data(), buf.size() + 1, f, args2);
            va_end(args2);

    return buf;
}

#define CHECK(succ, ...) do { \
    if (!(succ)) throw TestFailed(fmt(__VA_ARGS__)); \
  } while (0)

template < typename T >
struct Tree {
    Tree(T key, std::unique_ptr<Tree> left = nullptr, std::unique_ptr<Tree> right = nullptr)
            : key(std::move(key)), left(std::move(left)), right(std::move(right)) {}

    T key;
    std::unique_ptr<Tree> left, right;
};

#endif


template < typename T >
std::unique_ptr<Tree<T>> optimal_tree(const std::vector<std::pair<T, uint32_t>>& data) {
    if (data.empty())
        return nullptr;

    size_t n = data.size();

    std::vector<uint64_t> dp(n * n, 0);
    std::vector<uint32_t> opt(n * n, 0);

    std::vector<uint64_t> w_acc(n + 1, 0);
    for (size_t i = 0; i < n; ++i)
        w_acc[i + 1] = w_acc[i] + data[i].second;



    auto get_weight = [&](size_t i, size_t j) {
        return w_acc[j + 1] - w_acc[i];
    };

    auto get_dp = [&](size_t i, size_t j) { return dp[i * n + j]; };
    auto set_dp = [&](size_t i, size_t j, uint64_t val) { dp[i * n + j] = val; };
    auto get_opt = [&](size_t i, size_t j) { return opt[i * n + j]; };
    auto set_opt = [&](size_t i, size_t j, uint32_t val) { opt[i * n + j] = val; };


    for (size_t i = 0; i < n; ++i)
        set_opt(i, i, i);

    for (size_t len = 2; len <= n; ++len) {
        for (size_t i = 0; i <= n - len; ++i) {
            size_t j = i + len - 1;

            size_t k_min = get_opt(i, j - 1);
            size_t k_max = get_opt(i + 1, j);

            uint64_t min_cost = std::numeric_limits<uint64_t>::max();
            uint32_t best_k = k_min;

            uint64_t range_weight = get_weight(i, j);
            for (size_t k = k_min; k <= k_max; ++k) {
                uint64_t current_cost = range_weight - data[k].second;
                if (k > i) current_cost += get_dp(i, k - 1);
                if (k < j) current_cost += get_dp(k + 1, j);
                if (current_cost < min_cost) {
                    min_cost = current_cost;
                    best_k = k;
                }
            }
            set_dp(i, j, min_cost);
            set_opt(i, j, best_k);
        }
    }

    std::function<std::unique_ptr<Tree<T>>(size_t, size_t)> build_tree =
            [&](size_t i, size_t j) -> std::unique_ptr<Tree<T>> {
                if (i > j) return nullptr;
                size_t k = get_opt(i, j);
                auto node = std::make_unique<Tree<T>>(data[k].first);
                if (k > i) node->left = build_tree(i, k - 1);
                if (k < j) node->right = build_tree(k + 1, j);
                return node;
            };

    return build_tree(0, n - 1);
}


#ifndef __PROGTEST__

std::pair<uint64_t, std::vector<std::pair<std::string, uint32_t>>> S_TESTS[] = {
        { 15, { { "bar", 13 }, { "baz", 1 }, { "foo", 300 } } },
        { 29, { { "bar", 1300 }, { "baz", 1 }, { "foo", 3 }, { "goo", 20 } } },
        { 4, { { "bar", 1 }, { "baz", 1 }, { "foo", 1 }, { "goo", 1 } } },
        { 5, { { "bar", 1 }, { "baz", 1 }, { "foo", 1 }, { "goo", 100 } } },
};

std::pair<uint64_t, std::vector<std::pair<int, uint32_t>>> I_TESTS[] = {
        { 872836, {
                          { 164, 636135 }, { 257, 264 }, { 260, 438976 }, { 297, 50666 },
                          { 318, 140667 },
                  } },
        { 398616, {
                          { 788, 8054 }, { 823, 27034 }, { 843, 227058 }, { 851, 46664 },
                          { 926, 287553 },
                  } },
        { 951743, {
                          { 477, 8065 }, { 568, 512023 }, { 600, 41 }, { 664, 389113 },
                          { 760, 421962 }, { 844, 773 },
                  } },
        { 366268, {
                          { 432, 314434 }, { 447, 39325 }, { 546, 299 }, { 578, 64054 },
                          { 620, 9293 }, { 661, 97394 },
                  } },
        { 2022549, {
                          { 720, 493114 }, { 794, 681569 }, { 854, 373266 }, { 875, 21953 },
                          { 916, 790 }, { 938, 32805 }, { 956, 592726 },
                  } },
        { 3014384, {
                          { 307, 262191 }, { 386, 85211 }, { 450, 439074 }, { 484, 1050 },
                          { 579, 531540 }, { 630, 970334 }, { 638, 729065 },
                  } },
        { 2057927, {
                          { 953, 132745 }, { 1021, 46702 }, { 1109, 157495 }, { 1197, 884753 },
                          { 1251, 97369 }, { 1266, 421896 }, { 1342, 52 }, { 1400, 439023 },
                  } },
        { 526083, {
                          { 393, 19683 }, { 485, 54947 }, { 525, 148922 }, { 527, 64093 },
                          { 606, 27075 }, { 657, 103918 }, { 662, 32840 }, { 693, 17587 },
                  } },
        { 463486, {
                          { 736, 2296 }, { 770, 5901 }, { 826, 29850 }, { 876, 15693 },
                          { 942, 9296 }, { 982, 46684 }, { 1026, 39312 }, { 1032, 64008 },
                          { 1057, 614187 },
                  } },
        { 1831803, {
                          { 686, 175682 }, { 751, 288 }, { 821, 39368 }, { 860, 804406 },
                          { 891, 614163 }, { 959, 140684 }, { 965, 205463 }, { 1020, 161 },
                          { 1034, 42912 },
                  } },
        { 4404438, {
                          { 259, 59400 }, { 349, 778770 }, { 421, 681516 }, { 451, 274709 },
                          { 524, 5 }, { 559, 405311 }, { 563, 884759 }, { 577, 262223 },
                          { 580, 125022 }, { 632, 21957 },
                  } },
        { 3568403, {
                          { 666, 175696 }, { 702, 456582 }, { 732, 704978 }, { 800, 32858 },
                          { 807, 474604 }, { 855, 753590 }, { 939, 420 }, { 960, 103831 },
                          { 998, 195133 }, { 1021, 85206 },
                  } },
        { 3128395, {
                          { 520, 85217 }, { 604, 238343 }, { 661, 40 }, { 680, 830615 },
                          { 712, 32793 }, { 749, 512040 }, { 780, 85236 }, { 854, 54939 },
                          { 951, 157492 }, { 1013, 24411 }, { 1066, 636056 },
                  } },
        { 1240100, {
                          { 607, 42963 }, { 649, 4144 }, { 707, 343002 }, { 727, 8024 },
                          { 827, 148973 }, { 903, 32824 }, { 956, 274686 }, { 980, 2292 },
                          { 1062, 68963 }, { 1099, 9331 }, { 1192, 205455 },
                  } },
        { 4127533, {
                          { 843, 804382 }, { 917, 357975 }, { 1006, 195139 }, { 1098, 300849 },
                          { 1161, 32856 }, { 1250, 778689 }, { 1289, 78 }, { 1315, 27079 },
                          { 1348, 9336 }, { 1374, 415 }, { 1382, 157548 }, { 1458, 439047 },
                  } },
};


template < typename T >
void check_sol(
        uint64_t expected_score,
        const std::vector<std::pair<T, uint32_t>>& data,
        const std::unique_ptr<Tree<T>>& solution
) {

    using NodePtr = const std::unique_ptr<Tree<T>>;

    std::vector<std::pair<T, uint64_t>> seen;
    auto go = [&](NodePtr& node, uint64_t depth, auto&& self) {
        if (!node) return;

        self(node->left, depth + 1, self);
        seen.emplace_back(node->key, depth);
        self(node->right, depth + 1, self);
    };

    go(solution, 0, go);

    CHECK(seen.size() == data.size(),
          "Wrong number of nodes: expected %zu but go %zu.\n",
          data.size(), seen.size());

    uint64_t score = 0;
    for (size_t i = 0; i < seen.size(); i++) {
        CHECK(seen[i].first == data[i].first, "Element mismatch at index %zu\n", i);
        score += seen[i].second * data[i].second;
    }

    CHECK(score == expected_score, "Wrong score: expected %llu but got %llu\n",
          (unsigned long long)expected_score, (unsigned long long)score);
}

void run_tests(auto&& tests) {
    for (auto&& [ ref_sol, data ] : tests) {
        auto tree = optimal_tree(data);
        check_sol(ref_sol, data, tree);
    }
}

int main() {
    run_tests(S_TESTS);
    run_tests(I_TESTS);

    std::cout << "All tests passed." << std::endl;
}

#endif

