/// @file tree.hpp
/// @brief Base class for binary search tree

#pragma once
#include "debug.hpp"
#include "util.hpp"

#include <SFML/Window/Joystick.hpp>
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

template <typename Key, typename Value> struct Node;
template <typename Key, typename Value> struct Tree;

template <typename Key, typename Value> using TreeObject = std::unique_ptr<Tree<Key, Value>>;

template <typename Key, typename Value> struct Tree : TreeBase {
    using Node = Node<Key, Value>;

    auto size() const -> size_t override;
    void clear() override;
    void print() const override;
    void printCLI() const override;
    auto find(const Key& key) const -> Node*;

    virtual auto stringify() const -> std::string override;
    virtual auto insert(const Key& key, const Value& value) -> Status;
    virtual auto remove(const Key& key) -> Status;
    virtual auto split(const Key& key) -> Tree*;
    virtual void merge(Tree* other);

    virtual ~Tree() = default;

    std::unique_ptr<Node> root;

protected:
    static void refresh(Node* start);  // refresh info in node range [start, root]
};

/****************************** Implementation ********************************/

template <typename Key, typename Value> struct Node {
    std::unique_ptr<Node> lchild, rchild;
    Node* parent;
    size_t size;
    Key key;
    Value value;
    Node(const Key& key, const Value& value, Node* parent = nullptr)
        : lchild(nullptr), rchild(nullptr), parent(parent), size(1), key(key), value(value) {}
    virtual auto stringify() const -> std::string {
        return serializeClass("Node", key, value, size, this, parent, lchild, rchild);
    }
    virtual void refresh() {
        this->size =
            1 + (this->lchild ? this->lchild->size : 0) + (this->rchild ? this->rchild->size : 0);
    }
    virtual ~Node() = default;
};

template <typename Node> struct NodeTraits;
template <typename K, typename V> struct NodeTraits<Node<K, V>> {
    using Key = K;
    using Value = V;
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
    std::function<void(const Node*, int)> printNode = [&](const Node* node, int depth) {
        if (!node) return;
        printNode(node->lchild.get(), depth + 1);
        std::cout << std::string(depth * 2, ' ') << node->key << ": " << node->value << "\n";
        printNode(node->rchild.get(), depth + 1);
    };
    printNode(root.get(), 0);
}

template <typename K, typename V> auto Tree<K, V>::find(const K& key) const -> Node* {
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
    // 更新size
    cur = parent;
    while (cur) {
        cur->size = 1;
        if (cur->lchild) cur->size += cur->lchild->size;
        if (cur->rchild) cur->size += cur->rchild->size;
        cur = cur->parent;
    }
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

template <typename K, typename V> void Tree<K, V>::merge(Tree* other) {
    // 简单实现：将other的所有节点插入到当前树
    if (!other) return;
    auto dfs = [&](auto self, const std::unique_ptr<Node>& node) {
        if (!node) return;
        insert(node->key, node->value);
        self(self, node->lchild);
        self(self, node->rchild);
    };
    dfs(dfs, other->root);
}

template <typename K, typename V> auto Tree<K, V>::split(const K& key) -> Tree* {
    // 简单实现：将所有key > 给定key的节点移到新树
    auto new_tree = new Tree();
    auto dfs = [&](auto self, std::unique_ptr<Node>& node) {
        if (!node) return;
        if (node->key > key) {
            new_tree->insert(node->key, node->value);
            if (node->lchild) self(self, node->lchild);
            if (node->rchild) self(self, node->rchild);
            remove(node->key);
        } else {
            if (node->rchild) self(self, node->rchild);
        }
    };
    dfs(dfs, root);
    return new_tree;
}
