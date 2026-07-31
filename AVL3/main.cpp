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
#include <utility>

// We use std::map as a reference to check our implementation.
// It is not available in progtest :)
#include <map>
#include <string>

template < typename K, typename V, typename Op = std::plus<V> >
struct Ref {
    bool insert(const K& key, const V& value) {
        if (map.contains(key)) return false;
        map.emplace(key, value);
        return true;
    }

    std::optional<V> erase(const K& key) {
        auto ret = value(key);
        map.erase(key);
        return ret;
    }

    std::optional<V> value(const K& key) const {
        if (!map.contains(key)) return {};
        return map.at(key);
    }

    void update(const K& first, const K& last, const V& by) {
        auto end = map.end();
        auto it = map.lower_bound(first);
        while (it != end && it->first <= last) {
            it->second = op(it->second, by);
            ++it;
        }
    }

    auto begin() const { return map.begin(); }
    auto end() const { return map.end(); }

private:
    std::map<K, V> map;
    Op op{};
};

#endif


namespace config {
    inline constexpr bool PARENT_POINTERS = true;
    inline constexpr bool CHECK_DEPTH = true;
}


template < typename K, typename V, typename Op = std::plus<V> >
struct LazyUpdate {
    struct Node {
        K key;
        V value;
        V pending_update;

        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
        int height = 1;

        Node(K k, V v, Node* p)
                : key(std::move(k))
                , value(std::move(v))
                , pending_update(V{})
                , parent(p)
        {}
    };

    Node* root = nullptr;

    LazyUpdate() = default;

    ~LazyUpdate() {
        freeTree(root);
    }

    void freeTree(Node* n) {
        if (!n) return;
        freeTree(n->left);
        freeTree(n->right);
        delete n;
    }


    void applyLazy(Node* n, const V& v) {
        if (!n) return;
        n->pending_update = Op{}(n->pending_update, v);
    }

    void push(Node* n) {
        if (!n || n->pending_update == V{}) return;

        applyLazy(n->left, n->pending_update);
        applyLazy(n->right, n->pending_update);

        n->value = Op{}(n->value, n->pending_update);
        n->pending_update = V{};
    }

    bool insert(const K& key, const V& value) {
        bool success = false;
        root = insertImpl(root, key, value, nullptr, success);
        return success;
    }

    std::optional<V> erase(const K& key) {
        std::optional<V> result;
        root = eraseImpl(root, key, result);
        return result;
    }

    std::optional<V> value(const K& key) const {
        Node* current = root;
        V accumulated_lazy = V{};

        while (current) {
            accumulated_lazy = Op{}(accumulated_lazy, current->pending_update);
            if (key < current->key)
                current = current->left;
            else if (current->key < key)
                current = current->right;
            else
                return Op{}(current->value, accumulated_lazy);
        }
        return std::nullopt;
    }

    void update(const K& first, const K& last, const V& by) {
        Node* cur = root;
        while (cur) {
            push(cur);
            if (cur->key > last) cur = cur->left;
            else if (cur->key < first) cur = cur->right;
            else break;
        }

        if (!cur) return;

        cur->value = Op{}(cur->value, by);

        Node* temp = cur->left;
        while (temp) {
            push(temp);
            if (temp->key >= first) {
                temp->value = Op{}(temp->value, by);
                applyLazy(temp->right, by);
                temp = temp->left;
            }
            else
                temp = temp->right;

        }

        temp = cur->right;
        while (temp) {
            push(temp);
            if (temp->key <= last) {
                temp->value = Op{}(temp->value, by);
                applyLazy(temp->left, by);
                temp = temp->right;
            }
            else
                temp = temp->left;

        }
    }


    int getHeight(Node* n) { return n ? n->height : 0; }
    void updateHeight(Node* n) { if (n) n->height = 1 + std::max(getHeight(n->left), getHeight(n->right)); }
    int getBalance(Node* n) { return n ? getHeight(n->left) - getHeight(n->right) : 0; }

    Node* rotateRight(Node* y) {
        Node* x = y->left;
        push(x);
        Node* T2 = x->right;
        x->right = y; y->left = T2;
        x->parent = y->parent; y->parent = x;
        if (T2) T2->parent = y;
        updateHeight(y); updateHeight(x);
        return x;
    }

    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        push(y);
        Node* T2 = y->left;
        y->left = x; x->right = T2;
        y->parent = x->parent; x->parent = y;
        if (T2) T2->parent = x;
        updateHeight(x); updateHeight(y);
        return y;
    }

    Node* balanceNode(Node* node) {
        updateHeight(node);
        int bal = getBalance(node);
        if (bal > 1) {
            if (getBalance(node->left) < 0) {
                push(node->left);
                node->left = rotateLeft(node->left);
            }
            return rotateRight(node);
        }
        if (bal < -1) {
            if (getBalance(node->right) > 0) {
                push(node->right);
                node->right = rotateRight(node->right);
            }
            return rotateLeft(node);
        }
        return node;
    }

    Node* insertImpl(Node* node, const K& key, const V& val, Node* p, bool& success) {
        if (!node) {
            success = true;
            return new Node(key, val, p);
        }
        push(node);
        if (key < node->key) node->left = insertImpl(node->left, key, val, node, success);
        else if (node->key < key) node->right = insertImpl(node->right, key, val, node, success);
        else { success = false; return node; }
        return balanceNode(node);
    }

    Node* deleteMin(Node* n, std::optional<K>& outKey, std::optional<V>& outVal) {
        if (!n) return nullptr;
        push(n);

        if (!n->left) {
            outKey = std::move(n->key);
            outVal = std::move(n->value);
            Node* rightChild = n->right;
            if (rightChild) rightChild->parent = n->parent;
            delete n;
            return rightChild;
        }

        n->left = deleteMin(n->left, outKey, outVal);
        if (n->left) n->left->parent = n;
        return balanceNode(n);
    }

    Node* eraseImpl(Node* node, const K& key, std::optional<V>& result) {
        if (!node) return nullptr;
        push(node);

        if (key < node->key)
            node->left = eraseImpl(node->left, key, result);
        else if (node->key < key)
            node->right = eraseImpl(node->right, key, result);
        else {
            result = node->value;
            if (!node->left) {
                Node* rightChild = node->right;
                if (rightChild) rightChild->parent = node->parent;
                delete node;
                return rightChild;
            }
            if (!node->right) {
                Node* leftChild = node->left;
                if (leftChild) leftChild->parent = node->parent;
                delete node;
                return leftChild;
            }

            std::optional<K> minKey;
            std::optional<V> minVal;
            node->right = deleteMin(node->right, minKey, minVal);
            if (node->right) node->right->parent = node;

            node->key = std::move(*minKey);
            node->value = std::move(*minVal);
        }

        if (node) {
            if (node->left) node->left->parent = node;
            if (node->right) node->right->parent = node;
            return balanceNode(node);
        }
        return nullptr;
    }

    struct TesterInterface {
        static const Node *root(const LazyUpdate *t) { return t->root; }
        static const Node *parent(const Node *n) { return n->parent; }
        static const Node *right(const Node *n) { return n->right; }
        static const Node *left(const Node *n) { return n->left; }
        static const K& key(const Node *n) { return n->key; }
        static const V& value(const Node *n) { return n->value; }
        static const V& pending_update(const Node *n) { return n->pending_update; }
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

template < typename K, typename V, typename Op = std::plus<V> >
struct Tester {
    void insert(const K& key, const V& value, bool check_tree_ = false) {
        bool r = ref.insert(key, value);
        bool s = stud.insert(key, value);
        if (r != s) _throw("insert mismatch", r);

        if (check_tree_) check_tree();
    }

    void erase(const K& key, bool check_tree_ = false) {
        std::optional<V> r = ref.erase(key);
        std::optional<V> s = stud.erase(key);
        if (r.has_value() != s.has_value())
            _throw("Erase mismatch", !!r);

        if (r != s)
            throw TestFailed("Erase value mismatch");

        if (check_tree_) check_tree();
    }

    void value(const K& key, bool check_tree_ = false) const {
        std::optional<V> r = ref.value(key);
        std::optional<V> s = stud.value(key);
        if (r.has_value() != s.has_value())
            _throw("Value mismatch", !!r);

        if (r != s)
            throw TestFailed("Value value mismatch: expected");

        if (check_tree_) check_tree();
    }

    void update(const K& first, const K& last, const V& by, bool check_tree_ = false) {
        ref.update(first, last, by);
        stud.update(first, last, by);

        if (check_tree_) check_tree();
    }

    void check_tree() const {
        using TI = typename LazyUpdate<K, V, Op>::TesterInterface;
        auto ref_it = ref.begin();

        auto check_value = [&](const K& key, const V& value, const V& lazy) {
            if (ref_it == ref.end())
                throw TestFailed("Reference elements exhausted.");

            const auto& [ k, v ] = *ref_it;

            if (k != key)
                throw TestFailed("Key mismatch.");

            if (v != Op{}(value, lazy))
                throw TestFailed("Value mismatch.");

            ++ref_it;
        };

        check_node(
                TI::root(&stud),
                decltype(TI::root(&stud))(nullptr),
                check_value,
                V{}
        );
    }

private:
    template < typename Node, typename F >
    int check_node(const Node* n, const Node* p, F& check_value, const V& acc_lazy) const {
        if (!n) return -1;

        using TI = typename LazyUpdate<K, V, Op>::TesterInterface;
        if constexpr(config::PARENT_POINTERS) {
            if (TI::parent(n) != p)
                throw TestFailed("Parent mismatch.");
        }

        V new_lazy = Op{}(acc_lazy, TI::pending_update(n));
        int l_depth = check_node(TI::left(n), n, check_value, new_lazy);
        check_value(TI::key(n), TI::value(n), new_lazy);
        int r_depth = check_node(TI::right(n), n, check_value, new_lazy);

        if (config::CHECK_DEPTH && abs(l_depth - r_depth) > 1) throw TestFailed(fmt(
                    "Tree is not avl balanced: left depth %i and right depth %i.",
                    l_depth, r_depth
            ));

        return std::max(l_depth, r_depth) + 1;
    }

    static void _throw(const char *msg, bool s) {
        throw TestFailed(fmt("%s: ref %s.", msg, s ? "succeeded" : "failed"));
    }

    LazyUpdate<K, V, Op> stud;
    Ref<K, V, Op> ref;
};


void test_insert() {
    Tester<int, int, std::plus<>> t;

    for (int k = 1; k <= 10; k++) {
        t.insert(k, k * 10, true);
        t.value(k, true);
    }

    t.insert(5, 999, true);
    t.value(5, true);
    t.update(3, 7, 5, true);

    for (int k = 11; k <= 20; k++) t.insert(k, k * 10, true);
    t.update(1, 20, -3, true);
    t.update(21, 30, 10, true);

    for (int k = -10; k <= 40; k++) t.value(k);
    t.check_tree();

    t.update(5, 5, 2, true);
    t.update(5, 5, -10, true);
    t.update(5, 5, 0, true);

    t.value(5);
    t.check_tree();
}

void test_erase() {
    Tester<int, int, std::plus<>> t;

    for (int k = 1; k <= 15; k++) t.insert(k, k * 2, true);

    t.erase(10, true);
    t.erase(100, true);
    t.erase(10, true);

    t.update(5, 15, 3, true);
    t.value(10, true);

    for (int k = 1; k <= 15; k++) t.erase(k, true);
    for (int k = 1; k <= 15; k++) t.value(k, true);

    t.insert(1, 100, true);
    t.insert(1, 999, true);
}


enum RandomTestFlags : unsigned {
    SEQ = 1, NO_ERASE = 2, CHECK_TREE = 4
};


struct Foo {
    unsigned x = 0;
    double y = 1;

    friend bool operator == (Foo a, Foo b) {
        if (a.x != b.x) return false;
        return std::abs(a.y - b.y) <= 1e-12*(std::abs(a.y) + std::abs(b.y));
    }

    struct Bar {
        Foo operator() (Foo a, Foo b) const {
            return { a.x + b.x, a.y * b.y };
        }
    };
};

template < typename T >
T wrap(uint32_t x) {
    if constexpr(std::is_same_v<T, std::string>)
        return std::to_string(x);
    else if constexpr(std::is_same_v<T, Foo>)
        return { unsigned(x / 10), x % 10 / 4.5 };
    else
        return x;
}

template < typename Key = int, typename Value = int, typename Plus = std::plus<Value> >
void test_random(size_t size, unsigned flags = 0) {
    Tester<Key, Value, Plus> t;
    std::mt19937 my_rand(24607 + size);

    bool seq = flags & SEQ;
    bool erase = !(flags & NO_ERASE);
    bool check_tree = flags & CHECK_TREE;

    for (size_t i = 0; i < size; i++) {
        auto k = wrap<Key>(seq ? i : my_rand() % (3*size));
        t.insert(k, wrap<Value>(my_rand() % (3*size)), check_tree);
    }

    t.check_tree();

    for (size_t i = 0; i < 3*size; i++)
        t.value(wrap<Key>(i), check_tree);

    size_t max = size;
    for (size_t i = 0; i < 30*size; i++) switch (my_rand() % 7) {
            case 1: {
                auto k = wrap<Key>(seq ? max++ : my_rand() % (3*size));
                t.insert(k, wrap<Value>(my_rand() % 1'000), check_tree);
                break;
            }
            case 2:
                if (erase) t.erase(wrap<Key>(my_rand() % (3*size)), check_tree);
                break;
            case 3: {
                auto f = wrap<Key>(my_rand() % (3*size));
                auto l = wrap<Key>(my_rand() % (3*size));
                if (f > l) std::swap(f, l);
                t.update(f, l, wrap<Value>(my_rand() % 20), check_tree);
                break;
            }
            default:
                t.value(wrap<Key>(my_rand() % (3*size)));
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

        std::cout << "Tiny random test (different types)..." << std::endl;
        test_random<std::string, int>(20, CHECK_TREE);
        test_random<int, Foo, Foo::Bar>(20, CHECK_TREE);
        test_random<std::string, Foo, Foo::Bar>(20, CHECK_TREE);

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


