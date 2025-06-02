/// @file tree.hpp
/// @brief Base class for binary search tree

#pragma once
#include "debug.hpp"
#include "util.hpp"

#include <cctype>
#include <cstdlib>
#include <memory>
#include <string>

/****************************** Definition ********************************/

struct TreeBase {
    virtual ~TreeBase() = default;
    virtual auto size() const -> size_t = 0;
    virtual void clear() = 0;
    virtual void print() const = 0;
    virtual void printCLI() const = 0;
    virtual auto stringify() const -> std::string = 0;
};

template <typename Key, typename Value> struct Tree : TreeBase {
    struct Node;
    friend struct Test;

    std::unique_ptr<Node> root;

    Tree() = default;
    explicit Tree(Node* root);
    explicit Tree(std::unique_ptr<Node> root);

    auto size() const -> size_t override;
    void clear() override;
    void print() const override;
    void printCLI() const override;
    auto find(const Key& key) const -> const Node*;
    auto maximum() const -> const Node*;
    auto minimum() const -> const Node*;
    auto conflict(Tree* other) -> bool;  // time complexity: O(n)

    Value& operator[](const Key& key);
    const Value& operator[](const Key& key) const;

    virtual ~Tree() = default;
    virtual auto stringify() const -> std::string override;
    virtual auto insert(const Key& key, const Value& value) -> Status;
    virtual auto remove(const Key& key) -> Status;
    virtual auto split(const Key& key) -> std::unique_ptr<Tree>;  // split out nodes that >= key
    virtual auto merge(std::unique_ptr<Tree> other) -> Status;    // auto invokes concat/mixin

    // NOTE:
    // conflict:
    //   check if other tree have nodes that conflict with this tree
    // merge:
    //   merge other tree to this tree, auto invokes concat/mixin
    // mixin:
    //   requires:
    //     none
    //   notes:
    //     - time complexity: O(m log(n)), m = other->size(), n = this->size()
    //     - If key conflicts, the conflicted node would not be inserted, the rest of the nodes
    //     would be inserted normally
    // concat:
    //   requires:
    //     - key-range not overlapping
    //     - tree type compatible
    //   notes:
    //     - time complexity: O(log (n + m))
    //     - You can't merge a basic tree to AVL tree
    //     - Merging an AVL tree to basic tree is acceptable, however losts the balance feature

    // NOTE:
    // When calling merge/mixin/concat, the ownership is passed to function, so whether the merge is
    // successful or not, the original tree is destroyed.
    // Thus, you should check conflict before calling merge/mixin/concat, by code logic or calling
    // conflict().

    // TODO:
    // Is it better to change interface to use reference instead of take ownership?

protected:
    static void maintain(Node* start);  // refresh nodes info upwards from start to root
    static auto detach(std::unique_ptr<Node>& node) -> std::unique_ptr<Node>;  // detach a node
    auto container(const Key& key) -> std::tuple<Node*, std::unique_ptr<Node>&>;
    auto container(const Key& key) const -> std::tuple<const Node*, const std::unique_ptr<Node>&>;
    auto box(Node* node) -> std::unique_ptr<Node>&;

    auto mixin(std::unique_ptr<Tree> other) -> Status;

    // NOTE: detach would return nullptr when the node have two children (not trivial)
    // NOTE: mixin calls insert() in implement, so have polymorphic behavior to auto maintain
    // variant tree node data, can be called by derived class object

private:
    auto concat(std::unique_ptr<Tree> other) -> Status;
    // NOTE: derived classes should not call this, implement their own cancat() instead.
};

template <typename K, typename V> struct AVLTree;

/****************************** Implementation ********************************/

template <typename K, typename V> struct Tree<K, V>::Node {
    using Key = K;
    using Value = V;
    friend struct Test;  // let `Test` class have access to non-public members
    friend struct Tree<K, V>;
    friend struct AVLTree<K, V>;

    Key key;
    Value value;
    size_t size;

    Node(const Key& key, const Value& value, Node* parent = nullptr)
        : key(key), value(value), size(1), lchild(nullptr), rchild(nullptr), parent(parent) {}
    virtual ~Node() = default;

    virtual auto stringify() const -> std::string {
        return serializeClass("Node", key, value, size, this, parent, lchild, rchild);
    }
    virtual void maintain() {
        this->size =
            1 + (this->lchild ? this->lchild->size : 0) + (this->rchild ? this->rchild->size : 0);
    }

    const Node* left() const { return lchild.get(); }
    const Node* right() const { return rchild.get(); }

protected:
    std::unique_ptr<Node> lchild, rchild;
    Node* parent;
    enum { L, R };
    void bindL(std::unique_ptr<Node> node) {
        lchild = std::move(node);
        if (lchild) lchild->parent = this;
    }
    void bindR(std::unique_ptr<Node> node) {
        rchild = std::move(node);
        if (rchild) rchild->parent = this;
    }
};

template <typename K, typename V>
Tree<K, V>::Tree(std::unique_ptr<Node> root) : root(std::move(root)) {
    root->parent = nullptr;
}

template <typename K, typename V>
Tree<K, V>::Tree(Node* root) : root(std::unique_ptr<Node>(root)) {}

template <typename K, typename V> auto Tree<K, V>::stringify() const -> std::string {
    return serializeClass("Tree", root);
}

template <typename K, typename V> auto Tree<K, V>::size() const -> size_t {
    return root ? root->size : 0;
}

template <typename K, typename V> void Tree<K, V>::clear() { root.reset(); }

template <typename K, typename V> void Tree<K, V>::print() const {
    // TODO:
}

template <typename K, typename V> void Tree<K, V>::printCLI() const {
    if (!root) {
        std::cout << "<empty>" << std::endl;
        return;
    }
    auto print_node = [&](auto self, const Node* node, int depth) {
        if (!node) return;
        self(self, node->lchild.get(), depth + 1);
        std::cout << std::string(depth * 4, ' ') << node->key << ": " << node->value << "\n";
        self(self, node->rchild.get(), depth + 1);
    };
    print_node(print_node, root.get(), 0);
}

template <typename K, typename V> auto Tree<K, V>::maximum() const -> const Node* {
    Node* cur = root.get();
    while (cur && cur->rchild) {
        cur = cur->rchild.get();
    }
    return cur;
}

template <typename K, typename V> auto Tree<K, V>::minimum() const -> const Node* {
    Node* cur = root.get();
    while (cur && cur->lchild) {
        cur = cur->lchild.get();
    }
    return cur;
}

template <typename K, typename V>
auto Tree<K, V>::detach(std::unique_ptr<Node>& node) -> std::unique_ptr<Node> {
    if (node->lchild && node->rchild) return std::unique_ptr<Node>(nullptr);
    auto parent = node->parent;
    if (node->lchild) node->lchild->parent = parent;
    if (node->rchild) node->rchild->parent = parent;
    auto raw = node.release();
    if (!raw->lchild)
        node = std::move(raw->rchild);
    else
        node = std::move(raw->lchild);
    Tree::maintain(parent);
    return std::unique_ptr<Node>(raw);
}

template <typename K, typename V>
auto Tree<K, V>::container(const K& key) -> std::tuple<Node*, std::unique_ptr<Node>&> {
    using T = std::unique_ptr<Node>;
    auto search = [&key](auto self, Node* parent, T& current) -> std::tuple<Node*, T&> {
        if (!current || key == current->key) return {parent, current};
        if (key < current->key)
            return self(self, current.get(), current->lchild);
        else
            return self(self, current.get(), current->rchild);
    };
    return search(search, nullptr, this->root);
}

template <typename K, typename V>
auto Tree<K, V>::container(const K& key) const
    -> std::tuple<const Node*, const std::unique_ptr<Node>&> {
    auto search = [&key](auto self, const Node* parent, const std::unique_ptr<Node>& current)
        -> std::tuple<const Node*, const std::unique_ptr<Node>&> {
        if (!current || key == current->key) return {parent, current};
        if (key < current->key)
            return self(self, current.get(), current->lchild);
        else
            return self(self, current.get(), current->rchild);
    };
    return search(search, nullptr, this->root);
}

template <typename K, typename V> auto Tree<K, V>::box(Node* node) -> std::unique_ptr<Node>& {
    if (!node) throw std::invalid_argument("node is nullptr");
    if (node == this->root.get()) return this->root;
    if (!node->parent) throw std::invalid_argument("invalid node: no parent");
    if (node->parent->lchild.get() == node) return node->parent->lchild;
    if (node->parent->rchild.get() == node) return node->parent->rchild;
    throw std::invalid_argument("invalid node");
}

template <typename K, typename V> auto Tree<K, V>::find(const K& key) const -> const Node* {
    auto [_, node] = this->container(key);
    return node.get();
}

template <typename K, typename V> auto Tree<K, V>::operator[](const K& key) -> V& {
    auto [_, node] = this->container(key);
    if (!node) this->insert(key, V{});
    return node->value;
}

template <typename K, typename V> auto Tree<K, V>::operator[](const K& key) const -> const V& {
    auto [_, node] = this->container(key);
    if (!node) throw std::out_of_range(std::format("key {} not found", key));
    return node->value;
}

template <typename K, typename V> void Tree<K, V>::maintain(Node* start) {
    while (start) {
        start->maintain();
        start = start->parent;
    }
}

template <typename K, typename V> Status Tree<K, V>::insert(const K& key, const V& value) {
    auto [parent, node] = this->container(key);
    if (node) return Status::FAILED;
    node = std::make_unique<Node>(key, value, parent);
    maintain(parent);
    return Status::SUCCESS;
}

template <typename K, typename V> Status Tree<K, V>::remove(const K& key) {
    auto [parent, node] = this->container(key);
    if (!node) return Status::FAILED;
    if (!node->lchild || !node->rchild) {
        Tree::detach(node);
        Tree::maintain(parent);
        return Status::SUCCESS;
    }
    auto find_max = [](auto self, auto& node) -> decltype(node) {
        if (!node || !node->rchild) return node;
        return self(self, node->rchild);
    };
    auto detached = Tree::detach(find_max(find_max, node->lchild));
    detached->bindL(std::move(node->lchild));
    detached->bindR(std::move(node->rchild));
    detached->parent = parent;
    node = std::move(detached);
    Tree::maintain(node.get());
    return Status::SUCCESS;
}

template <typename K, typename V> auto Tree<K, V>::split(const K& key) -> std::unique_ptr<Tree> {
    auto new_tree = std::make_unique<Tree>();
    auto dfs = [&](auto self, std::unique_ptr<Node>& node) {
        if (!node) return;
        if (node->rchild) self(self, node->rchild);
        if (node->lchild) self(self, node->lchild);
        if (node->key >= key) {
            new_tree->insert(node->key, node->value);
            remove(node->key);
        }
    };
    dfs(dfs, root);
    return new_tree;
}

template <typename K, typename V> auto Tree<K, V>::conflict(Tree* other) -> bool {
    std::vector<Node*> vec1, vec2;
    vec1.reserve(this->size()), vec2.reserve(other->size());
    auto dfs = [&](auto self, Node* node, std::vector<Node*>& vec) {
        if (!node) return;
        self(self, node->lchild.get(), vec);
        vec.push_back(node);
        self(self, node->rchild.get(), vec);
    };
    dfs(dfs, this->root.get(), vec1);
    dfs(dfs, other->root.get(), vec2);
    for (auto it1 = vec1.begin(), it2 = vec2.begin(); it1 != vec1.end() && it2 != vec2.end();) {
        if ((*it1)->key < (*it2)->key) {
            ++it1;
        } else if ((*it1)->key > (*it2)->key) {
            ++it2;
        } else {
            return true;
        }
    }
    return false;
}

// NOTE: when key conflicts, conflicted nodes will not be inserted, others will be inserted normally
template <typename K, typename V> auto Tree<K, V>::merge(std::unique_ptr<Tree> other) -> Status {
    if (!other) return Status::FAILED;         // tree does not exist
    if (!other->root) return Status::SUCCESS;  // nothing to merge
    if (!this->root) {
        this->root = std::move(other->root);
        return Status::SUCCESS;  // merging into empty tree
    }
    if (this->minimum()->key > other->maximum()->key ||
        other->minimum()->key > this->maximum()->key) {
        return this->concat(std::move(other));
    }
    return this->mixin(std::move(other));
}

template <typename K, typename V> auto Tree<K, V>::mixin(std::unique_ptr<Tree> other) -> Status {
    if (!other) return Status::FAILED;
    if (!other->root) return Status::SUCCESS;
    if (!this->root) {
        this->root = std::move(other->root);
        return Status::SUCCESS;
    }
    auto copy = [&](auto self, Tree* dest, std::unique_ptr<Node>& node) -> Status {
        Status result = Status::SUCCESS;
        if (node->rchild)
            if (self(self, dest, node->rchild) != Status::SUCCESS) result = Status::FAILED;
        if (node->lchild)
            if (self(self, dest, node->lchild) != Status::SUCCESS) result = Status::FAILED;
        if (dest->insert(node->key, node->value) != Status::SUCCESS) result = Status::FAILED;
        return result;
    };
    return copy(copy, this, other->root);
}

// NOTE: when key-range overlapping, the merge will fail, lost all nodes in `other`
template <typename K, typename V> Status Tree<K, V>::concat(std::unique_ptr<Tree> other) {
    if (!other) return Status::FAILED;
    if (!other->root) return Status::SUCCESS;
    if (!this->root) {
        this->root = std::move(other->root);
        return Status::SUCCESS;
    }
    if (this->size() && other->size()) {
        if ((this->minimum()->key <= other->maximum()->key) &&
            (other->minimum()->key <= this->maximum()->key)) {
            return Status::FAILED;  // Overlapping keys
        }
        if (this->minimum()->key > other->maximum()->key) {
            std::swap(this->root, other->root);
        }
    }
    Node* rightmost = this->root.get();
    while (rightmost->rchild) {
        rightmost = rightmost->rchild.get();
    }
    rightmost->bindR(std::move(other->root));
    maintain(rightmost);
    return Status::SUCCESS;
}
