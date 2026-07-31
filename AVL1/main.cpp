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
#include <random>
#include <type_traits>

// We use std::set as a reference to check our implementation.
// It is not available in progtest :)
#include <set>

template < typename T >
struct Ref {
    size_t size() const { return _data.size(); }
    const T* find(const T& value) const {
        auto it = _data.find(value);
        if (it == _data.end()) return nullptr;
        return &*it;
    }
    bool insert(const T& value) { return _data.insert(value).second; }
    bool erase(const T& value) { return _data.erase(value); }

    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end(); }

private:
    std::set<T> _data;
};

#endif


namespace config {
    // Enable to check that the tree is AVL balanced.
    inline constexpr bool CHECK_DEPTH = true;

    // Disable if your implementation does not have parent pointers
    inline constexpr bool PARENT_POINTERS = true;
}

// TODO implement
template < typename T >
struct Tree {
    struct Node {
        T value;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
        int height = 1;

        Node(T v, Node* p) : value(std::move(v)), parent(p) {}
    };

    Node* m_root = nullptr;
    size_t m_size = 0;

    Tree() = default;

    ~Tree() {
        freeTree(m_root);
    }

    void freeTree(Node* node) {
        if (!node) return;
        freeTree(node->left);
        freeTree(node->right);
        delete node;
    }

    size_t size() const {
        return m_size;
    }

    const T* find(const T& value) const {
        Node* current = m_root;
        while (current) {
            if (value < current->value)
                current = current->left;
            else if (current->value < value)
                current = current->right;
            else
                return &current->value;
        }
        return nullptr;
    }

    bool insert(T value) {
        bool success = false;
        m_root = insertImpl(m_root, std::move(value), nullptr, success);
        if (success) m_size++;
        return success;
    }

    bool erase(const T& value) {
        bool success = false;
        m_root = eraseImpl(m_root, value, success);
        if (success) m_size--;
        return success;
    }



    int getHeight(Node* n) {
        return n ? n->height : 0;
    }

    void updateHeight(Node* n) {
        if (n) n->height = 1 + std::max(getHeight(n->left), getHeight(n->right));
    }

    int getBalance(Node* n) {
        return n ? getHeight(n->left) - getHeight(n->right) : 0;
    }

    Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;

        x->right = y;
        y->left = T2;

        x->parent = y->parent;
        y->parent = x;
        if (T2) T2->parent = y;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;

        y->left = x;
        x->right = T2;

        y->parent = x->parent;
        x->parent = y;
        if (T2) T2->parent = x;

        updateHeight(x);
        updateHeight(y);

        return y;
    }

    Node* balanceNode(Node* node) {
        updateHeight(node);
        int balance = getBalance(node);

        if (balance > 1) {
            if (getBalance(node->left) < 0)
                node->left = rotateLeft(node->left);

            return rotateRight(node);
        }

        if (balance < -1) {
            if (getBalance(node->right) > 0)
                node->right = rotateRight(node->right);

            return rotateLeft(node);
        }

        return node;
    }

    Node* insertImpl(Node* node, T value, Node* p, bool& success) {
        if (!node) {
            success = true;
            return new Node(std::move(value), p);
        }

        if (value < node->value) 
            node->left = insertImpl(node->left, std::move(value), node, success);
        else if (node->value < value)
            node->right = insertImpl(node->right, std::move(value), node, success);
        else {
            success = false;
            return node;
        }

        return balanceNode(node);
    }

    Node* findMin(Node* node) {
        while (node->left) node = node->left;
        return node;
    }

    Node* eraseImpl(Node* node, const T& value, bool& success) {
        if (!node) {
            success = false;
            return nullptr;
        }

        if (value < node->value)
            node->left = eraseImpl(node->left, value, success);
        else if (node->value < value)
            node->right = eraseImpl(node->right, value, success);
        else {
            success = true;


            if (!node->left) {
                Node* rightChild = node->right;
                if (rightChild) rightChild->parent = node->parent;
                delete node;
                return rightChild;
            }
            else if (!node->right) {
                Node* leftChild = node->left;
                if (leftChild) leftChild->parent = node->parent;
                delete node;
                return leftChild;
            }
            else {
                Node* temp = findMin(node->right);
                node->value = std::move(temp->value);

                bool subSuccess = false;
                node->right = eraseImpl(node->right, node->value, subSuccess);
            }
        }

        if (node) {
            if (node->left) node->left->parent = node;
            if (node->right) node->right->parent = node;

            return balanceNode(node);
        }
        return nullptr;
    }

    struct TesterInterface {
        static const Node *root(const Tree *t) { return t->m_root; }
        static const Node *parent(const Node *n) { return n->parent; }
        static const Node *right(const Node *n) { return n->right; }
        static const Node *left(const Node *n) { return n->left; }
        static const T& value(const Node *n) { return n->value; }
    };
};
#ifndef __PROGTEST__

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

template < typename T >
struct Tester {
    Tester() = default;

    void size() const {
        size_t r = ref.size();
        size_t t = tested.size();
        if (r != t) throw TestFailed(fmt("Size: got %zu but expected %zu.", t, r));
    }

    void find(const T& x) const {
        auto r = ref.find(x);
        auto t = tested.find(x);
        bool found_r = r != nullptr;
        bool found_t = t != nullptr;

        if (found_r != found_t) _throw("Find mismatch", found_r);
        if (found_r && *t != x) throw TestFailed("Find: found different value");
    }

    void insert(const T& x, bool check_tree_ = false) {
        auto succ_r = ref.insert(x);
        auto succ_t = tested.insert(x);
        if (succ_r != succ_t) _throw("Insert mismatch", succ_r);
        size();
        if (check_tree_) check_tree();
    }

    void erase(const T& x, bool check_tree_ = false) {
        bool succ_r = ref.erase(x);
        auto succ_t = tested.erase(x);
        if (succ_r != succ_t) _throw("Erase mismatch", succ_r);
        size();
        if (check_tree_) check_tree();
    }

    struct NodeCheckResult {
        const T* min = nullptr;
        const T* max = nullptr;
        int depth = -1;
        size_t size = 0;
    };

    void check_tree() const {
        using TI = typename Tree<T>::TesterInterface;
        auto ref_it = ref.begin();
        bool check_value_failed = false;
        auto check_value = [&](const T& v) {
            if (check_value_failed) return;
            check_value_failed = (ref_it == ref.end() || *ref_it != v);
            if (!check_value_failed) ++ref_it;
        };

        auto r = check_node(TI::root(&tested), decltype(TI::root(&tested))(nullptr), check_value);
        size_t t_size = tested.size();

        if (t_size != r.size) throw TestFailed(
                    fmt("Check tree: size() reports %zu but expected %zu.", t_size, r.size));

        if (check_value_failed) throw TestFailed(
                    "Check tree: element mismatch");

        size();
    }

    template < typename Node, typename F >
    NodeCheckResult check_node(const Node* n, const Node* p, F& check_value) const {
        if (!n) return {};

        using TI = typename Tree<T>::TesterInterface;
        if constexpr(config::PARENT_POINTERS) {
            if (TI::parent(n) != p) throw TestFailed("Parent mismatch.");
        }

        auto l = check_node(TI::left(n), n, check_value);
        check_value(TI::value(n));
        auto r = check_node(TI::right(n), n, check_value);

        if (l.max && !(*l.max < TI::value(n)))
            throw TestFailed("Max of left subtree is too big.");
        if (r.min && !(TI::value(n) < *r.min))
            throw TestFailed("Min of right subtree is too small.");

        if (config::CHECK_DEPTH && abs(l.depth - r.depth) > 1) throw TestFailed(fmt(
                    "Tree is not avl balanced: left depth %i and right depth %i.",
                    l.depth, r.depth
            ));

        return {
                l.min ? l.min : &TI::value(n),
                r.max ? r.max : &TI::value(n),
                std::max(l.depth, r.depth) + 1, 1 + l.size + r.size
        };
    }

    static void _throw(const char *msg, bool s) {
        throw TestFailed(fmt("%s: ref %s.", msg, s ? "succeeded" : "failed"));
    }

    Tree<T> tested;
    Ref<T> ref;
};


void test_insert() {
    Tester<int> t;

    for (int i = 0; i < 10; i++) t.insert(i, true);
    for (int i = -10; i < 20; i++) t.find(i);

    for (int i = 0; i < 10; i++) t.insert((1 + i * 7) % 17, true);
    for (int i = -10; i < 20; i++) t.find(i);
}

void test_erase() {
    Tester<int> t;

    for (int i = 0; i < 10; i++) t.insert((1 + i * 7) % 17, true);
    for (int i = -10; i < 20; i++) t.find(i);

    for (int i = 3; i < 22; i += 2) t.erase(i, true);
    for (int i = -10; i < 20; i++) t.find(i);

    for (int i = 0; i < 10; i++) t.insert((1 + i * 13) % 17 - 8, true);
    for (int i = -10; i < 20; i++) t.find(i);

    for (int i = -4; i < 10; i++) t.erase(i, true);
    for (int i = -10; i < 20; i++) t.find(i);
}

enum RandomTestFlags : unsigned {
    SEQ = 1, NO_ERASE = 2, CHECK_TREE = 4
};

void test_random(size_t size, unsigned flags = 0) {
    Tester<size_t> t;
    std::mt19937 my_rand(24707 + size);

    bool seq = flags & SEQ;
    bool erase = !(flags & NO_ERASE);
    bool check_tree = flags & CHECK_TREE;

    for (size_t i = 0; i < size; i++)
        t.insert(seq ? 2*i : my_rand() % (3*size), check_tree);

    t.check_tree();

    for (size_t i = 0; i < 3*size + 1; i++) t.find(i);

    for (size_t i = 0; i < 30*size; i++) switch (my_rand() % 5) {
            case 1: t.insert(my_rand() % (3*size), check_tree);
                break;
            case 2: if (erase) t.erase(my_rand() % (3*size), check_tree);
                break;
            default:
                t.find(my_rand() % (3*size));
        }

    t.check_tree();
}

int main() {
    try {
        std::cout << "Insert test..." << std::endl;
        test_insert();

        std::cout << "Erase test..." << std::endl;
        test_erase();

        std::cout << "Tiny random test..." << std::endl;
        test_random(20, CHECK_TREE);

        std::cout << "Small random test..." << std::endl;
        test_random(200, CHECK_TREE);

        std::cout << "Big random test..." << std::endl;
        test_random(50'000);

        std::cout << "Big sequential test..." << std::endl;
        test_random(50'000, SEQ);

        std::cout << "All tests passed." << std::endl;
    } catch (const TestFailed& e) {
        std::cout << "Test failed: " << e.what() << std::endl;
    }
}

#endif


