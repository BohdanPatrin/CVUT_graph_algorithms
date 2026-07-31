/*
 * # Edit Distance (Levenshtein distance)
 *
 * Your task is to implement function
 * `edit_distance(std::string_view a, std::string_view b)` which
 * calculates the shortest sequence of operations to transform `a`
 * into `b`. The return value is sequence of the operations of
 * the type `std::vector<EditOp>` where `EditOp` is variant of
 * possible operations:
 *
 * - `struct Insert { size_t index; char new_value; }`: Insert
 *   character `new_value` at index `index`.
 * - `struct Rewrite { size_t index; char new_value; }`: Rewrite
 *   character at index `index` to value `new_value`.
 * - `struct Delete { size_t index; }`: Erase character at index
 *   `index`.
 *
 * The time limits are 9 seconds for the small test and 6.5 seconds
 * for the large test and 3 seconds for bonus points. The big test
 * awards partial points to slightly slower solutions.
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
#include <variant>


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

#define OK() do {} while (0)
#define CHECK(succ, ...) do { \
    if (!(succ)) throw TestFailed(fmt(__VA_ARGS__)); \
  } while (0)


struct Insert { size_t index; char new_value; };
struct Erase { size_t index; };
struct Rewrite { size_t index; char new_value; };

using EditOp = std::variant<Insert, Erase, Rewrite>;

#endif


std::vector<EditOp> edit_distance(std::string_view a, std::string_view b) {
    size_t prefix = 0;
    while (prefix < a.size() && prefix < b.size() && a[prefix] == b[prefix])
        prefix++;

    size_t a_len = a.size();
    size_t b_len = b.size();
    size_t suffix = 0;
    while (suffix < (a_len - prefix) && suffix < (b_len - prefix) &&
           a[a_len - 1 - suffix] == b[b_len - 1 - suffix])
        suffix++;

    const char* s1 = a.data() + prefix;
    const char* s2 = b.data() + prefix;
    size_t n = a_len - prefix - suffix;
    size_t m = b_len - prefix - suffix;

    std::vector<EditOp> ops;
    if (n == 0) {
        for (size_t k = 0; k < m; ++k)
            ops.emplace_back(Insert{prefix, s2[m - 1 - k]});

        return ops;
    }
    if (m == 0) {
        for (size_t k = 0; k < n; ++k)
            ops.emplace_back(Erase{prefix + n - 1 - k});

        return ops;
    }

    std::vector<uint8_t> dir((n + 1) * (m + 1));
    std::vector<uint32_t> prev(m + 1);
    std::vector<uint32_t> curr(m + 1);

    for (size_t j = 0; j <= m; ++j) {
        prev[j] = j;
        if (j > 0) dir[j] = 1;
    }

    for (size_t i = 1; i <= n; ++i) {
        curr[0] = i;
        dir[i * (m + 1)] = 2;

        const char c1 = s1[i - 1];

        for (size_t j = 1; j <= m; ++j) {
            const char c2 = s2[j - 1];

            uint32_t cost_diag = prev[j - 1] + (c1 == c2 ? 0 : 1);
            uint32_t cost_up = prev[j] + 1;
            uint32_t cost_left = curr[j - 1] + 1;

            if (cost_diag <= cost_up && cost_diag <= cost_left) {
                curr[j] = cost_diag;
                dir[i * (m + 1) + j] = 0;
            } else if (cost_left <= cost_up) {
                curr[j] = cost_left;
                dir[i * (m + 1) + j] = 1;
            } else {
                curr[j] = cost_up;
                dir[i * (m + 1) + j] = 2;
            }
        }
        std::swap(prev, curr);
    }

    size_t i = n;
    size_t j = m;

    while (i > 0 || j > 0) {
        uint8_t d = dir[i * (m + 1) + j];

        if (d == 0) {
            if (s1[i - 1] != s2[j - 1])
                ops.emplace_back(Rewrite{prefix + i - 1, s2[j - 1]});
            i--; j--;
        } else if (d == 1) {
            ops.emplace_back(Insert{prefix + i, s2[j - 1]});
            j--;
        } else {
            ops.emplace_back(Erase{prefix + i - 1});
            i--;
        }
    }
    return ops;
}


#ifndef __PROGTEST__


struct Test {
    size_t ops;
    std::string a, b;
};

Test tests[] = {
        { 0, "a", "a" },
        { 2, "a", "aaa" },
        { 2, "aaa", "a" },
        { 1, "a", "b" },
        { 3, "kitten", "sitting" },
        { 5, "intention", "execution" },
        { 171,
             "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.",
                "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
        },
        { 175,
             "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.",
                "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur."
        },
        { 654,
             "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?",
                "At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos dolores et quas molestias excepturi sint occaecati cupiditate non provident, similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et dolorum fuga. Et harum quidem rerum facilis est et expedita distinctio. Nam libero tempore, cum soluta nobis est eligendi optio cumque nihil impedit quo minus id quod maxime placeat facere possimus, omnis voluptas assumenda est, omnis dolor repellendus. Temporibus autem quibusdam et aut officiis debitis aut rerum necessitatibus saepe eveniet ut et voluptates repudiandae sint et molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus asperiores repellat."
        },
};


std::pair<std::string, std::string> gen_test(size_t a_len, size_t b_len, uint32_t seed) {
    std::mt19937 R(seed);

    std::pair<std::string, std::string> ret;
    auto& [ a, b ] = ret;
    a.resize(a_len);
    b.resize(b_len);

    for (char& c : a) c = 'a' + (R() % 26);
    for (char& c : b) c = 'a' + (R() % 26);

    return ret;
}

struct GenTest {
    size_t ops;
    size_t a_len, b_len;
    uint32_t seed;
};
GenTest small_tests[] =  {
        { 52, 53, 57, 3695321631 },
        { 61, 66, 65, 4054054267 },
        { 60, 69, 54, 1538242923 },
        { 65, 69, 68, 342186463 },
        { 56, 61, 64, 4003122640 },
        { 56, 51, 64, 371633339 },
        { 59, 68, 56, 3748193064 },
        { 56, 59, 66, 877682033 },
        { 61, 66, 69, 1235502787 },
        { 57, 64, 57, 4252431350 },
        { 57, 60, 68, 153021563 },
        { 52, 57, 56, 433274506 },
        { 62, 69, 61, 1412579772 },
        { 60, 54, 69, 1412218467 },
        { 55, 64, 59, 1731381075 },
        { 64, 74, 57, 2384538489 },
        { 55, 60, 57, 3161140608 },
        { 61, 65, 66, 1866868037 },
        { 56, 59, 58, 290152419 },
        { 54, 63, 58, 293619771 },
};

GenTest mid_tests[] =  {
        { 446, 502, 504, 2758883949 },
        { 461, 516, 516, 1603420692 },
        { 456, 516, 508, 3424078807 },
        { 449, 500, 511, 1400742931 },
        { 456, 510, 512, 42003445 },
        { 448, 502, 503, 2937871243 },
        { 452, 505, 504, 95045563 },
        { 458, 519, 515, 2492242928 },
        { 456, 514, 518, 1512283434 },
        { 443, 501, 503, 1194203670 },
        { 466, 519, 514, 3176616540 },
        { 456, 516, 510, 4153101820 },
        { 466, 517, 518, 3213795491 },
        { 447, 505, 508, 103606739 },
        { 444, 503, 503, 1576375291 },
        { 449, 511, 502, 2693591265 },
        { 458, 504, 518, 3446607440 },
        { 455, 506, 516, 3301160724 },
        { 449, 500, 513, 1049853860 },
        { 462, 518, 515, 4007899828 },
};

GenTest unbalanced_tests[] =  {
        { 19982, 20012, 30, 3923047546 },
        { 19991, 20013, 22, 1710299462 },
        { 19983, 21, 20004, 4127769854 },
        { 19980, 36, 20016, 1403057010 },
        { 19991, 20016, 25, 3498969622 },
        { 19969, 20000, 31, 178647418 },
        { 19981, 20017, 36, 2062866459 },
        { 19987, 20015, 28, 4161422258 },
        { 19976, 20000, 24, 1284960027 },
        { 19991, 26, 20017, 1534173412 },
        { 19990, 27, 20017, 2457990664 },
        { 19970, 34, 20004, 2545167258 },
        { 19983, 25, 20008, 1728202415 },
        { 19975, 20003, 28, 4004710963 },
        { 19973, 20004, 31, 3426728456 },
        { 19974, 31, 20005, 1188541451 },
        { 19983, 20013, 30, 3314724422 },
        { 19979, 31, 20010, 2969900722 },
        { 19996, 22, 20018, 3208041741 },
        { 19968, 20007, 39, 1476156445 },
};

GenTest big_tests[] =  {
        { 3529, 4000, 4005, 499485525 },
        { 3533, 4002, 4009, 4192265898 },
        { 3524, 4006, 4004, 1052144362 },
        { 3542, 4013, 4015, 157476329 },
        { 3520, 4002, 4008, 450830566 },
        { 3536, 4017, 4016, 3936328337 },
        { 3540, 4000, 4006, 2285217052 },
        { 3538, 4012, 4006, 3868456983 },
        { 3526, 4000, 4018, 3630566721 },
        { 3521, 4003, 4016, 2939227091 },
        { 3536, 4017, 4018, 2714194049 },
        { 3527, 4004, 4007, 2761018657 },
        { 3540, 4018, 4002, 3826515968 },
        { 3530, 4001, 4007, 2209372478 },
        { 3543, 4015, 4015, 3747307430 },
        { 3545, 4016, 4016, 4278571502 },
        { 3532, 4006, 4016, 818449387 },
        { 3525, 4011, 4001, 2518204739 },
        { 3536, 4004, 4009, 3951709741 },
        { 3524, 4009, 4005, 3183660940 },
};

// replace with avl-as-array for better performance
template < typename T >
struct Array {
    bool empty() const { return _data.empty(); }
    size_t size() const { return _data.size(); }

    const T& operator [] (size_t index) const { return _data.at(index); }
    T& operator [] (size_t index) { return _data.at(index); }

    void insert(size_t index, T value) {
        if (index > _data.size()) throw std::out_of_range("oops");
        _data.insert(_data.begin() + index, std::move(value));
    }

    void erase(size_t index) {
        _data.erase(_data.begin() + index);
    }

private:
    std::vector<T> _data;
};


void check_sol(
        size_t expected_ops,
        std::string_view a,
        std::string_view b,
        const std::vector<EditOp>& solution
) {
    Array<char> str;

    for (char c : a) str.insert(str.size(), c);

    for (auto&& op : solution) {
        if (auto ins = std::get_if<Insert>(&op)) {
            CHECK(ins->index <= str.size(), "Insert index out of range.\n");
            str.insert(ins->index, ins->new_value);
        } else if (auto rew = std::get_if<Rewrite>(&op)) {
            CHECK(rew->index < str.size(), "Rewrite index out of range.\n");
            str[rew->index] = rew->new_value;
        } else {
            auto del = std::get<Erase>(op);
            CHECK(del.index < str.size(), "Erase index out of range.\n");
            str.erase(del.index);
        }
    }

    CHECK(str.size() == b.size(),
          "Wrong size: edited a has length %zu but b has %zu.\n",
          str.size(), b.size());

    for (size_t i = 0; i < b.size(); i++)
        CHECK(str[i] == b[i],
              "Character mismatch at index %zu: expected '%c' but got '%c'.\n",
              i, b[i], str[i]);

    CHECK(solution.size() == expected_ops,
          "Wrong number of operations: expected %zu but got %zu\n",
          expected_ops, solution.size());

    OK();
}

void run_gen_test(const GenTest& t) {
    auto [ a, b ] = gen_test(t.a_len, t.b_len, t.seed);
    check_sol(t.ops, a, b, edit_distance(a, b));
}

int main() {
    for (auto&& t : tests)
        check_sol(t.ops, t.a, t.b, edit_distance(t.a, t.b));

    for (auto&& t : mid_tests) run_gen_test(t);
    for (auto&& t : unbalanced_tests) run_gen_test(t);
    for (auto&& t : big_tests) run_gen_test(t);

    std::cout << "All tests passed." << std::endl;
}

#endif


