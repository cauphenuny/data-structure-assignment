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

    auto size() const -> size_t override;
    void clear() override;
    void print() const override;
    void printCLI() const override;
    auto find(const Key& key) const -> const Node*;
    auto maximum() const -> const Node*;
    auto minimum() const -> const Node*;
    auto conflict(Tree* other) -> bool;  // time complexity: O(n)

    virtual auto stringify() const -> std::string override;
    virtual auto insert(const Key& key, const Value& value) -> Status;
    virtual auto remove(const Key& key) -> Status;
    virtual auto split(const Key& key) -> std::unique_ptr<Tree>;  // split out nodes that >= key
    virtual auto merge(std::unique_ptr<Tree> other) -> Status;    // auto invokes concat/mix
    virtual auto mixin(std::unique_ptr<Tree> other) -> Status;    // O(n log n) merge, allow overlap
    virtual auto concat(std::unique_ptr<Tree> other) -> Status;   // O(log n) merge, no overlap

    virtual ~Tree() = default;

    std::unique_ptr<Node> root;

protected:
    static void refresh(Node* start);  // refresh info in node range [start, root]
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
    virtual void refresh() {
        this->size =
            1 + (this->lchild ? this->lchild->size : 0) + (this->rchild ? this->rchild->size : 0);
    }

    const Node* leftChild() const { return lchild.get(); }
    const Node* rightChild() const { return rchild.get(); }

protected:
    std::unique_ptr<Node> lchild, rchild;
    Node* parent;
};

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

template <typename K, typename V> auto Tree<K, V>::find(const K& key) const -> const Node* {
    Node* cur = root.get();
    while (cur) {
        if (key < cur->key) {
            cur = cur->lchild.get();
        } else if (key > cur->key) {
            cur = cur->rchild.get();
        } else {
            return cur;
        }
    }
    return nullptr;
}

template <typename K, typename V> void Tree<K, V>::refresh(Node* start) {
    while (start) {
        start->refresh();
        start = start->parent;
    }
}

template <typename K, typename V> Status Tree<K, V>::insert(const K& key, const V& value) {
    if (!root) {
        root = std::make_unique<Node>(key, value, nullptr);
        return Status::SUCCESS;
    }
    Node* cur = root.get();
    Node* parent = nullptr;
    while (cur) {
        parent = cur;
        if (key < cur->key) {
            if (cur->lchild) {
                cur = cur->lchild.get();
            } else {
                cur->lchild = std::make_unique<Node>(key, value, parent);
                break;
            }
        } else if (key > cur->key) {
            if (cur->rchild) {
                cur = cur->rchild.get();
            } else {
                cur->rchild = std::make_unique<Node>(key, value, parent);
                break;
            }
        } else {
            return Status::FAILED;
        }
    }
    refresh(parent);
    return Status::SUCCESS;
}

template <typename K, typename V> Status Tree<K, V>::remove(const K& key) {
    Node* cur = root.get();
    Node* parent = nullptr;
    while (cur && cur->key != key) {
        parent = cur;
        if (key < cur->key)
            cur = cur->lchild.get();
        else
            cur = cur->rchild.get();
    }
    if (!cur) return Status::FAILED;  // Node not found

    auto transplant = [&](std::unique_ptr<Node>& u, std::unique_ptr<Node>& v) {
        if (v) v->parent = u->parent;  // Set parent before moving

        if (!u->parent) {
            // Root node case
            root = std::move(v);
        } else if (u->parent->lchild.get() == u.get()) {
            u->parent->lchild = std::move(v);
        } else {
            u->parent->rchild = std::move(v);
        }
    };

    std::unique_ptr<Node>* cur_ptr = nullptr;
    if (!parent)
        cur_ptr = &root;
    else if (parent->lchild.get() == cur)
        cur_ptr = &(parent->lchild);
    else
        cur_ptr = &(parent->rchild);

    // Track the node that needs size updates after removal
    Node* update_node = parent;

    if (!cur->lchild) {
        // Case 1: Node has no left child
        transplant(*cur_ptr, cur->rchild);
    } else if (!cur->rchild) {
        // Case 2: Node has no right child
        transplant(*cur_ptr, cur->lchild);
    } else {
        // Case 3: Node has two children - find successor
        Node* succ = cur->rchild.get();
        Node* succ_parent = cur;

        // Find leftmost node in right subtree
        while (succ->lchild) {
            succ_parent = succ;
            succ = succ->lchild.get();
        }

        // If successor is not direct right child, update path
        if (succ_parent != cur) {
            update_node = succ_parent;  // For refreshing sizes

            // Move successor's right child to successor's position
            auto succ_right = std::move(succ->rchild);
            succ_parent->lchild = std::move(succ_right);
            if (succ_parent->lchild) {
                succ_parent->lchild->parent = succ_parent;
            }

            // Set up successor with removed node's children
            auto successor = std::make_unique<Node>(succ->key, succ->value);
            successor->parent = cur->parent;

            // Move removed node's children to successor
            successor->rchild = std::move(cur->rchild);
            if (successor->rchild) {
                successor->rchild->parent = successor.get();
            }

            successor->lchild = std::move(cur->lchild);
            if (successor->lchild) {
                successor->lchild->parent = successor.get();
            }

            // Replace removed node with successor
            *cur_ptr = std::move(successor);
        } else {
            // Successor is direct right child
            auto successor = std::move(cur->rchild);
            successor->parent = cur->parent;

            // Move left child to successor
            successor->lchild = std::move(cur->lchild);
            if (successor->lchild) {
                successor->lchild->parent = successor.get();
            }

            // Replace removed node with successor
            *cur_ptr = std::move(successor);
        }
    }

    // Update sizes/heights after removal
    if (update_node) {
        refresh(update_node);
    } else if (root) {
        // If we removed the root, refresh from the new root
        refresh(root.get());
    }

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
        this->root.reset(other->root.release());
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
        this->root.reset(other->root.release());
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
        this->root.reset(other->root.release());
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
    rightmost->rchild.reset(other->root.release());
    rightmost->rchild->parent = rightmost;
    refresh(rightmost);
    return Status::SUCCESS;
}
