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

// We use std::vector as a reference to check our implementation.
// It is not available in progtest :)
#include <vector>

template < typename T >
struct Ref {
    bool empty() const { return _data.empty(); }
    size_t size() const { return _data.size(); }

    const T& operator [] (size_t index) const { return _data.at(index); }
    T& operator [] (size_t index) { return _data.at(index); }

    void insert(size_t index, T value) {
        if (index > _data.size()) throw std::out_of_range("oops");
        _data.insert(_data.begin() + index, std::move(value));
    }

    T erase(size_t index) {
        T ret = std::move(_data.at(index));
        _data.erase(_data.begin() + index);
        return ret;
    }

    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end(); }

private:
    std::vector<T> _data;
};

#endif


namespace config {
    inline constexpr bool PARENT_POINTERS = true;
    inline constexpr bool CHECK_DEPTH = true;
}

template < typename T >
struct Array {
    struct Node {
        T value;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
        int height = 1;
        size_t size = 1;

        Node(T v, Node* p) : value(std::move(v)), parent(p) {}
    };

    Node* m_root = nullptr;

    Array() = default;

    ~Array() {
        freeTree(m_root);
    }

    void freeTree(Node* n) {
        if (!n) return;
        freeTree(n->left);
        freeTree(n->right);
        delete n;
    }


    size_t getSubtreeSize(Node* n) const {
        return n ? n->size : 0;
    }

    int getHeight(Node* n) const {
        return n ? n->height : 0;
    }


    void updateStats(Node* n) {
        if (n) {
            n->height = 1 + std::max(getHeight(n->left), getHeight(n->right));
            n->size = 1 + getSubtreeSize(n->left) + getSubtreeSize(n->right);
        }
    }

    int getBalance(Node* n) const {
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

        updateStats(y);
        updateStats(x);

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

        updateStats(x);
        updateStats(y);

        return y;
    }

    Node* balanceNode(Node* node) {
        updateStats(node);
        int bal = getBalance(node);

        if (bal > 1) {
            if (getBalance(node->left) < 0)
                node->left = rotateLeft(node->left);

            return rotateRight(node);
        }
        if (bal < -1) {
            if (getBalance(node->right) > 0)
                node->right = rotateRight(node->right);

            return rotateLeft(node);
        }
        return node;
    }

    bool empty() const {
        return m_root == nullptr;
    }

    size_t size() const {
        return getSubtreeSize(m_root);
    }

    Node* getNodeAt(size_t index) const {
        Node* cur = m_root;
        while (cur) {
            size_t leftSize = getSubtreeSize(cur->left);
            if (index < leftSize)
                cur = cur->left;
            else if (index > leftSize) {
                index -= (leftSize + 1);
                cur = cur->right;
            } else
                return cur;

        }
        return nullptr;
    }

    const T& operator [] (size_t index) const {
        if (index >= size()) throw std::out_of_range("Index out of range");
        return getNodeAt(index)->value;
    }

    T& operator [] (size_t index) {
        if (index >= size()) throw std::out_of_range("Index out of range");
        return getNodeAt(index)->value;
    }

    void insert(size_t index, T value) {
        if (index > size()) throw std::out_of_range("Index out of range");
        m_root = insertImpl(m_root, index, std::move(value), nullptr);
    }

    Node* insertImpl(Node* node, size_t index, T value, Node* p) {
        if (!node)
            return new Node(std::move(value), p);


        size_t leftSize = getSubtreeSize(node->left);

        if (index <= leftSize)
            node->left = insertImpl(node->left, index, std::move(value), node);
        else
            node->right = insertImpl(node->right, index - (leftSize + 1), std::move(value), node);


        return balanceNode(node);
    }

    T erase(size_t index) {
        if (index >= size()) throw std::out_of_range("Index out of range");
        T removedValue;
        m_root = eraseImpl(m_root, index, removedValue);
        return removedValue;
    }

    Node* findMin(Node* node) {
        while (node->left) node = node->left;
        return node;
    }

    Node* deleteMin(Node* node, Node*& outNode) {
        if (!node->left) {
            outNode = node;
            Node* rightChild = node->right;
            if (rightChild) rightChild->parent = node->parent;
            return rightChild;
        }
        node->left = deleteMin(node->left, outNode);
        if (node->left) node->left->parent = node;
        return balanceNode(node);
    }

    Node* eraseImpl(Node* node, size_t index, T& outValue) {
        if (!node) return nullptr;

        size_t leftSize = getSubtreeSize(node->left);

        if (index < leftSize)
            node->left = eraseImpl(node->left, index, outValue);
        else if (index > leftSize)
            node->right = eraseImpl(node->right, index - (leftSize + 1), outValue);
        else {
            outValue = std::move(node->value);

            if (!node->left || !node->right) {
                Node* temp = node->left ? node->left : node->right;
                if (!temp) {
                    delete node;
                    return nullptr;
                } else {
                    temp->parent = node->parent;
                    delete node;
                    return temp;
                }
            } else {
                Node* successor = nullptr;
                node->right = deleteMin(node->right, successor);
                if (node->right) node->right->parent = node;

                node->value = std::move(successor->value);
                delete successor;
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
        static const Node *root(const Array *t) { return t->m_root; }
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

    size_t size() const {
        bool te = tested.empty();
        size_t r = ref.size();
        size_t t = tested.size();
        if (te != !t) throw TestFailed(fmt("Size: size %zu but empty is %s.",
                                           t, te ? "true" : "false"));
        if (r != t) throw TestFailed(fmt("Size: got %zu but expected %zu.", t, r));
        return r;
    }

    const T& operator [] (size_t index) const {
        const T& r = ref[index];
        const T& t = tested[index];
        if (r != t) throw TestFailed("Op [] const mismatch.");
        return t;
    }

    void assign(size_t index, T x) {
        ref[index] = x;
        tested[index] = std::move(x);
        operator[](index);
    }

    void insert(size_t i, T x, bool check_tree_ = false) {
        ref.insert(i, x);
        tested.insert(i, std::move(x));
        size();
        if (check_tree_) check_tree();
    }

    T erase(size_t i, bool check_tree_ = false) {
        T r = ref.erase(i);
        T t = tested.erase(i);
        if (r != t) throw TestFailed(fmt("Erase mismatch at %zu.", i));
        size();
        if (check_tree_) check_tree();
        return t;
    }

    void check_tree() const {
        using TI = typename Array<T>::TesterInterface;
        auto ref_it = ref.begin();
        bool check_value_failed = false;
        auto check_value = [&](const T& v) {
            if (check_value_failed) return;
            check_value_failed = (ref_it == ref.end() || *ref_it != v);
            if (!check_value_failed) ++ref_it;
        };

        size();

        check_node(TI::root(&tested), decltype(TI::root(&tested))(nullptr), check_value);

        if (check_value_failed) throw TestFailed(
                    "Check tree: element mismatch");
    }

    template < typename Node, typename F >
    int check_node(const Node* n, const Node* p, F& check_value) const {
        if (!n) return -1;

        using TI = typename Array<T>::TesterInterface;
        if constexpr(config::PARENT_POINTERS) {
            if (TI::parent(n) != p) throw TestFailed("Parent mismatch.");
        }

        auto l_depth = check_node(TI::left(n), n, check_value);
        check_value(TI::value(n));
        auto r_depth = check_node(TI::right(n), n, check_value);

        if (config::CHECK_DEPTH && abs(l_depth - r_depth) > 1) throw TestFailed(fmt(
                    "Tree is not avl balanced: left depth %i and right depth %i.",
                    l_depth, r_depth
            ));

        return std::max(l_depth, r_depth) + 1;
    }

    static void _throw(const char *msg, bool s) {
        throw TestFailed(fmt("%s: ref %s.", msg, s ? "succeeded" : "failed"));
    }

    Array<T> tested;
    Ref<T> ref;
};


void test_insert() {
    Tester<int> t;

    for (int i = 0; i < 10; i++) t.insert(i, i, true);
    for (int i = 0; i < 10; i++) t.insert(i, -i, true);
    for (size_t i = 0; i < t.size(); i++) t[i];

    for (int i = 0; i < 5; i++) t.insert(15, (1 + i * 7) % 17, true);
    for (int i = 0; i < 10; i++) t.assign(2*i, 3*t[2*i]);
    for (size_t i = 0; i < t.size(); i++) t[i];
}

void test_erase() {
    Tester<int> t;

    for (int i = 0; i < 10; i++) t.insert(i, i, true);
    for (int i = 0; i < 10; i++) t.insert(i, -i, true);

    for (size_t i = 3; i < t.size(); i += 2) t.erase(i, true);
    for (size_t i = 0; i < t.size(); i++) t[i];

    for (int i = 0; i < 5; i++) t.insert(3, (1 + i * 7) % 17, true);
    for (size_t i = 1; i < t.size(); i += 3) t.erase(i, true);

    for (int i = 0; i < 20; i++) t.insert(3, 100 + i, true);

    for (int i = 0; i < 5; i++) t.erase(t.size() - 1, true);
    for (int i = 0; i < 5; i++) t.erase(0, true);

    for (int i = 0; i < 4; i++) t.insert(i, i, true);
    for (size_t i = 0; i < t.size(); i++) t[i];
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

    for (size_t i = 0; i < size; i++) {
        size_t pos = seq ? 0 : my_rand() % (i + 1);
        t.insert(pos, my_rand() % (3*size), check_tree);
    }

    t.check_tree();

    for (size_t i = 0; i < t.size(); i++) t[i];

    for (size_t i = 0; i < 30*size; i++) switch (my_rand() % 7) {
            case 1: {
                if (!erase && i % 3 == 0) break;
                size_t pos = seq ? 0 : my_rand() % (t.size() + 1);
                t.insert(pos, my_rand() % 1'000'000, check_tree);
                break;
            }
            case 2:
                if (erase) t.erase(my_rand() % t.size(), check_tree);
                break;
            case 3:
                t.assign(my_rand() % t.size(), 155 + i);
                break;
            default:
                t[my_rand() % t.size()];
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

        std::cout << "Bigger random test..." << std::endl;
        test_random(5'000);

        std::cout << "Bigger sequential test..." << std::endl;
        test_random(5'000, SEQ);

        std::cout << "All tests passed." << std::endl;
    } catch (const TestFailed& e) {
        std::cout << "Test failed: " << e.what() << std::endl;
    }
}

#endif


