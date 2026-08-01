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
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>
#include <utility>

struct Hobbit {
    std::string name;
    int hp, off, def;

    friend bool operator == (const Hobbit&, const Hobbit&) = default;
};

std::ostream& operator << (std::ostream& out, const Hobbit& h) {
    return out
            << "Hobbit{\"" << h.name << "\", "
            << ".hp=" << h.hp << ", "
            << ".off=" << h.off << ", "
            << ".def=" << h.def << "}";
}

template < typename T >
std::ostream& operator << (std::ostream& out, const std::optional<T>& x) {
    if (!x) return out << "EMPTY_OPTIONAL";
    return out << "Optional{" << *x << "}";
}

#endif


struct HobbitArmy {
    static constexpr bool CHECK_NEGATIVE_HP = true;

    struct Node {
        std::string name;
        int hp, off, def;
        int min_hp;

        int l_hp = 0;
        int l_off = 0;
        int l_def = 0;

        Node *left = nullptr;
        Node *right = nullptr;
        Node *parent = nullptr;
        int height = 1;

        Node(const Hobbit& h, Node* p)
                : name(h.name), hp(h.hp), off(h.off), def(h.def), min_hp(h.hp), parent(p) {}
    };

    Node* root = nullptr;

    HobbitArmy() = default;

    ~HobbitArmy() {
        freeTree(root);
    }

    void freeTree(Node* n) {
        if (!n) return;
        freeTree(n->left);
        freeTree(n->right);
        delete n;
    }


    void apply(Node* n, int dh, int doff, int dd) {
        if (!n) return;
        n->hp += dh;
        n->off += doff;
        n->def += dd;

        n->l_hp += dh;
        n->l_off += doff;
        n->l_def += dd;

        n->min_hp += dh;
    }

    void push(Node* n) {
        if (!n) return;
        if (n->l_hp == 0 && n->l_off == 0 && n->l_def == 0) return;

        apply(n->left, n->l_hp, n->l_off, n->l_def);
        apply(n->right, n->l_hp, n->l_off, n->l_def);

        n->l_hp = 0;
        n->l_off = 0;
        n->l_def = 0;
    }

    int getHeight(Node* n) const { return n ? n->height : 0; }

    void updateStats(Node* n) {
        if (!n) return;
        n->height = 1 + std::max(getHeight(n->left), getHeight(n->right));

        n->min_hp = n->hp;
        if (n->left) n->min_hp = std::min(n->min_hp, n->left->min_hp);
        if (n->right) n->min_hp = std::min(n->min_hp, n->right->min_hp);
    }

    int getBalance(Node* n) const {
        return n ? getHeight(n->left) - getHeight(n->right) : 0;
    }

    Node* rotateRight(Node* y) {
        Node* x = y->left;
        push(x);
        Node* T2 = x->right;

        x->right = y;
        y->left = T2;

        x->parent = y->parent;
        y->parent = x;
        if (T2) T2->parent = y;

        updateStats(y);
        updateStats(x);
        return x;
    }

    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        push(y);
        Node* T2 = y->left;

        y->left = x;
        x->right = T2;

        y->parent = x->parent;
        x->parent = y;
        if (T2) T2->parent = x;

        updateStats(x);
        updateStats(y);
        return y;
    }

    Node* balanceNode(Node* n) {
        updateStats(n);
        int bal = getBalance(n);
        if (bal > 1) {
            if (getBalance(n->left) < 0) {
                push(n->left);
                n->left = rotateLeft(n->left);
            }
            return rotateRight(n);
        }
        if (bal < -1) {
            if (getBalance(n->right) > 0) {
                push(n->right);
                n->right = rotateRight(n->right);
            }
            return rotateLeft(n);
        }
        return n;
    }


    bool add(const Hobbit& hobbit) {
        if (hobbit.hp <= 0) return false;
        bool success = false;
        root = insertImpl(root, hobbit, nullptr, success);
        return success;
    }

    Node* insertImpl(Node* node, const Hobbit& h, Node* p, bool& success) {
        if (!node) {
            success = true;
            return new Node(h, p);
        }
        push(node);
        if (h.name < node->name) node->left = insertImpl(node->left, h, node, success);
        else if (node->name < h.name) node->right = insertImpl(node->right, h, node, success);
        else { success = false; return node; }
        return balanceNode(node);
    }

    std::optional<Hobbit> erase(const std::string& hobbit_name) {
        std::optional<Hobbit> result;
        root = eraseImpl(root, hobbit_name, result);
        return result;
    }

    Node* deleteMin(Node* n, Hobbit& outH) {
        push(n);
        if (!n->left) {
            outH = {n->name, n->hp, n->off, n->def};
            Node* r = n->right;
            if (r) r->parent = n->parent;
            delete n;
            return r;
        }
        n->left = deleteMin(n->left, outH);
        if (n->left) n->left->parent = n;
        return balanceNode(n);
    }

    Node* eraseImpl(Node* node, const std::string& name, std::optional<Hobbit>& result) {
        if (!node) return nullptr;
        push(node);
        if (name < node->name) node->left = eraseImpl(node->left, name, result);
        else if (node->name < name) node->right = eraseImpl(node->right, name, result);
        else {
            result = Hobbit{node->name, node->hp, node->off, node->def};
            if (!node->left || !node->right) {
                Node* temp = node->left ? node->left : node->right;
                if (!temp) { delete node; return nullptr; }
                temp->parent = node->parent;
                delete node;
                return temp;
            }
            Hobbit minData{"",0,0,0};
            node->right = deleteMin(node->right, minData);
            if (node->right) node->right->parent = node;
            node->name = std::move(minData.name);
            node->hp = minData.hp; node->off = minData.off; node->def = minData.def;
        }
        if (node) {
            if (node->left) node->left->parent = node;
            if (node->right) node->right->parent = node;
            return balanceNode(node);
        }
        return nullptr;
    }

    std::optional<Hobbit> stats(const std::string& hobbit_name) const {
        Node* cur = root;
        int dh = 0, doff = 0, dd = 0;
        while (cur) {
            if (cur->name == hobbit_name)
                return Hobbit{cur->name, cur->hp + dh, cur->off + doff, cur->def + dd};

            dh += cur->l_hp; doff += cur->l_off; dd += cur->l_def;
            if (hobbit_name < cur->name) cur = cur->left;
            else cur = cur->right;
        }
        return std::nullopt;
    }


    int getRangeMin(Node* n, const std::string& first, const std::string& last) {
        while (n) {
            push(n);
            if (n->name > last) n = n->left;
            else if (n->name < first) n = n->right;
            else break;
        }
        if (!n) return std::numeric_limits<int>::max();

        int min_val = n->hp;

        Node* t = n->left;
        while (t) {
            push(t);
            if (t->name >= first) {
                min_val = std::min(min_val, t->hp);
                if (t->right) min_val = std::min(min_val, t->right->min_hp);
                t = t->left;
            } else t = t->right;
        }

        t = n->right;
        while (t) {
            push(t);
            if (t->name <= last) {
                min_val = std::min(min_val, t->hp);
                if (t->left) min_val = std::min(min_val, t->left->min_hp);
                t = t->right;
            } else t = t->left;
        }
        return min_val;
    }

    bool enchant(const std::string& first, const std::string& last, int hp_diff, int off_diff, int def_diff) {
        if (first > last) return true;

        if (hp_diff < 0) {
            int min_val = getRangeMin(root, first, last);
            if (min_val != std::numeric_limits<int>::max())
                if ((long long)min_val + hp_diff <= 0) return false;

        }

        updateRangeRecursive(root, first, last, hp_diff, off_diff, def_diff);
        return true;
    }


    void updateRangeRecursive(Node* n, const std::string& first, const std::string& last, int dh, int doff, int dd) {
        if (!n) return;


        if (n->name > last) {
            push(n);
            updateRangeRecursive(n->left, first, last, dh, doff, dd);
            updateStats(n);
        }
        else if (n->name < first) {
            push(n);
            updateRangeRecursive(n->right, first, last, dh, doff, dd);
            updateStats(n);
        }
        else {
            push(n);
            n->hp += dh; n->off += doff; n->def += dd;

            if (n->left) updateLeftPath(n->left, first, dh, doff, dd);
            if (n->right) updateRightPath(n->right, last, dh, doff, dd);

            updateStats(n);
        }
    }

    void updateLeftPath(Node* n, const std::string& first, int dh, int doff, int dd) {
        if (!n) return;
        push(n);
        if (n->name >= first) {
            n->hp += dh; n->off += doff; n->def += dd;
            apply(n->right, dh, doff, dd);
            updateLeftPath(n->left, first, dh, doff, dd);
        }
        else
            updateLeftPath(n->right, first, dh, doff, dd);

        updateStats(n);
    }

    void updateRightPath(Node* n, const std::string& last, int dh, int doff, int dd) {
        if (!n) return;
        push(n);
        if (n->name <= last) {
            n->hp += dh; n->off += doff; n->def += dd;
            apply(n->left, dh, doff, dd);
            updateRightPath(n->right, last, dh, doff, dd);
        } 
        else
            updateRightPath(n->left, last, dh, doff, dd);

        updateStats(n);
    }

    void for_each(auto&& fun) const {
        for_each_impl(root, 0, 0, 0, fun);
    }

private:
    void for_each_impl(Node *node, int acc_h, int acc_o, int acc_d, auto& fun) const {
        if (!node) return;
        int next_h = acc_h + node->l_hp;
        int next_o = acc_o + node->l_off;
        int next_d = acc_d + node->l_def;

        for_each_impl(node->left, next_h, next_o, next_d, fun);
        Hobbit h{node->name, node->hp + acc_h, node->off + acc_o, node->def + acc_d};
        fun(h);
        for_each_impl(node->right, next_h, next_o, next_d, fun);
    }
};

#ifndef __PROGTEST__

////////////////// Dark magic, ignore ////////////////////////

template < typename T >
auto quote(const T& t) { return t; }

std::string quote(const std::string& s) {
    std::string ret = "\"";
    for (char c : s) if (c != '\n') ret += c; else ret += "\\n";
    return ret + "\"";
}

#define STR_(a) #a
#define STR(a) STR_(a)

#define CHECK_(a, b, a_str, b_str) do { \
    auto _a = (a); \
    decltype(a) _b = (b); \
    if (_a != _b) { \
      std::cout << "Line " << __LINE__ << ": Assertion " \
        << a_str << " == " << b_str << " failed!" \
        << " (lhs: " << quote(_a) << ")" << std::endl; \
      fail++; \
    } else ok++; \
  } while (0)

#define CHECK(a, b) CHECK_(a, b, #a, #b)


////////////////// End of dark magic ////////////////////////


void check_army(const HobbitArmy& A, const std::vector<Hobbit>& ref, int& ok, int& fail) {
    size_t i = 0;

    A.for_each([&](const Hobbit& h) {
        CHECK(i < ref.size(), true);
        CHECK(h, ref[i]);
        i++;
    });

    CHECK(i, ref.size());
}

void test1(int& ok, int& fail) {
    HobbitArmy A;
    check_army(A, {}, ok, fail);

    CHECK(A.add({"Frodo", 100, 10, 3}), true);
    CHECK(A.add({"Frodo", 200, 10, 3}), false);
    CHECK(A.erase("Frodo"), std::optional(Hobbit("Frodo", 100, 10, 3)));
    CHECK(A.add({"Frodo", 200, 10, 3}), true);

    CHECK(A.add({"Sam", 80, 10, 4}), true);
    CHECK(A.add({"Pippin", 60, 12, 2}), true);
    CHECK(A.add({"Merry", 60, 15, -3}), true);
    CHECK(A.add({"Smeagol", 0, 100, 100}), false);

    if constexpr(HobbitArmy::CHECK_NEGATIVE_HP)
    CHECK(A.add({"Smeagol", -100, 100, 100}), false);

    CHECK(A.add({"Smeagol", 200, 100, 100}), true);

    CHECK(A.enchant("Frodo", "Frodo", 10, 1, 1), true);
    CHECK(A.enchant("Sam", "Frodo", -1000, 1, 1), true); // empty range
    CHECK(A.enchant("Bilbo", "Bungo", 1000, 0, 0), true); // empty range

    if constexpr(HobbitArmy::CHECK_NEGATIVE_HP)
    CHECK(A.enchant("Frodo", "Sam", -60, 1, 1), false);

    CHECK(A.enchant("Frodo", "Sam", 1, 0, 0), true);
    CHECK(A.enchant("Frodo", "Sam", -60, 1, 1), true);

    CHECK(A.stats("Gandalf"), std::optional<Hobbit>{});
    CHECK(A.stats("Frodo"), std::optional(Hobbit("Frodo", 151, 12, 5)));
    CHECK(A.stats("Merry"), std::optional(Hobbit("Merry", 1, 16, -2)));

    check_army(A, {
            {"Frodo", 151, 12, 5},
            {"Merry", 1, 16, -2},
            {"Pippin", 1, 13, 3},
            {"Sam", 21, 11, 5},
            {"Smeagol", 200, 100, 100},
    }, ok, fail);
}

int main() {
    int ok = 0, fail = 0;
    test1(ok, fail);

    if (!fail) std::cout << "Passed all " << ok << " tests!" << std::endl;
    else std::cout << "Failed " << fail << " of " << (ok + fail) << " tests." << std::endl;
}

#endif


