/// @file tree.hpp
/// @brief Base class for binary search tree

#pragma once
#include "debug.hpp"

#include <cctype>
#include <cstdlib>
#include <memory>
#include <string>

/****************************** Definition ********************************/

template <typename Key, typename Value> struct Node;
template <typename Key, typename Value> struct Tree;

template <typename Key, typename Value> using TreeObject = std::unique_ptr<Tree<Key, Value>>;

template <typename Key, typename Value> struct Tree {
    using Node = Node<Key, Value>;
    using TreeObject = TreeObject<Key, Value>;

    static auto create() -> TreeObject;

    void clear();
    auto size() const -> size_t;
    auto find(const Key& key) const -> Node*;

    virtual auto stringify() const -> std::string;
    virtual void insert(const Key& key, const Value& value);
    virtual void remove(const Key& key);
    virtual auto split(const Key& key) -> TreeObject;
    virtual void merge(const TreeObject& other);

    Tree() = default;
    Tree(const Tree&) = delete;
    Tree(Tree&&) = default;
    Tree& operator=(const Tree&) = delete;
    Tree& operator=(Tree&&) = default;
    virtual ~Tree() = default;

protected:
    std::unique_ptr<Node> root;
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
};
template <typename Key, typename Value> auto Tree<Key, Value>::stringify() const -> std::string {
    return serializeClass("Tree", root);
}

template <typename Key, typename Value> auto Tree<Key, Value>::create() -> TreeObject {
    return std::make_unique<Tree<Key, Value>>();
}

template <typename Key, typename Value> auto Tree<Key, Value>::size() const -> size_t {
    return root ? root->size : 0;
}

template <typename Key, typename Value> void Tree<Key, Value>::clear() { root.reset(); }

template <typename Key, typename Value>
void Tree<Key, Value>::insert(const Key& key, const Value& value) {
    if (!root) {
        root = std::make_unique<Node>(key, value, nullptr);
        return;
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
            cur->value = value;  // 更新已存在节点
            return;
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
}

template <typename Key, typename Value> void Tree<Key, Value>::remove(const Key& key) {
    Node* cur = root.get();
    Node* parent = nullptr;
    while (cur && cur->key != key) {
        parent = cur;
        if (key < cur->key)
            cur = cur->lchild.get();
        else
            cur = cur->rchild.get();
    }
    if (!cur) return;  // 未找到

    auto transplant = [&](std::unique_ptr<Node>& u, std::unique_ptr<Node>& v) {
        if (!u->parent) {
            root = std::move(v);
            if (root) root->parent = nullptr;
        } else if (u->parent->lchild.get() == u.get()) {
            u->parent->lchild = std::move(v);
            if (u->parent->lchild) u->parent->lchild->parent = u->parent;
        } else {
            u->parent->rchild = std::move(v);
            if (u->parent->rchild) u->parent->rchild->parent = u->parent;
        }
    };

    std::unique_ptr<Node>* cur_ptr = nullptr;
    if (!cur->parent)
        cur_ptr = &root;
    else if (cur->parent->lchild.get() == cur)
        cur_ptr = &(cur->parent->lchild);
    else
        cur_ptr = &(cur->parent->rchild);

    if (!cur->lchild) {
        transplant(*cur_ptr, cur->rchild);
    } else if (!cur->rchild) {
        transplant(*cur_ptr, cur->lchild);
    } else {
        // 找到右子树最小节点
        Node* succ = cur->rchild.get();
        std::unique_ptr<Node>* succ_ptr = &(cur->rchild);
        while (succ->lchild) {
            succ_ptr = &(succ->lchild);
            succ = succ->lchild.get();
        }
        if (succ->parent != cur) {
            transplant(*succ_ptr, succ->rchild);
            succ->rchild = std::move(cur->rchild);
            if (succ->rchild) succ->rchild->parent = succ;
        }
        auto wrapper = std::unique_ptr<Node>(succ);
        transplant(*cur_ptr, wrapper);
        succ->lchild = std::move(cur->lchild);
        if (succ->lchild) succ->lchild->parent = succ;
        succ->parent = cur->parent;
        // 注意：succ的unique_ptr已被转移，cur已无效
    }
    // 更新size
    Node* p = parent;
    while (p) {
        p->size = 1;
        if (p->lchild) p->size += p->lchild->size;
        if (p->rchild) p->size += p->rchild->size;
        p = p->parent;
    }
}

template <typename Key, typename Value> void Tree<Key, Value>::merge(const TreeObject& other) {
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

template <typename Key, typename Value> auto Tree<Key, Value>::split(const Key& key) -> TreeObject {
    // 简单实现：将所有key > 给定key的节点移到新树
    auto new_tree = Tree<Key, Value>::create();
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
