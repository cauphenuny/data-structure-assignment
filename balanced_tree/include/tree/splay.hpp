/// @file splay.hpp
/// @brief Splay tree implementation
#pragma once

#include "debug.hpp"
#include "interface.hpp"
#include "traits/node_traits.hpp"
#include "traits/tree_traits.hpp"

#include <memory>
#include <string>

// ============================== Definition ================================

template <typename K, typename V> struct SplayTreeImpl;

template <typename K, typename V> using SplayTree = TreeAdapter<K, V, SplayTreeImpl>;

// ============================== Implementation ================================

template <typename K, typename V>
struct SplayNode : Pair<const K, V>,
                   trait::node::TypeTraits<K, V>,
                   trait::node::Link<SplayNode<K, V>>,
                   trait::node::View<SplayNode<K, V>>,
                   trait::node::Maintain<trait::node::Size<SplayNode<K, V>>>,
                   trait::node::Search<SplayNode<K, V>> {
    SplayNode(const K& k, const V& v, SplayNode* parent = nullptr)
        : Pair<const K, V>(k, v), trait::node::Link<SplayNode<K, V>>(parent) {
        this->maintain();
    }

    auto stringify() const -> std::string {
        return serializeClass(
            "SplayNode", this->key, this->value, this, this->size, this->parent, this->child[L],
            this->child[R]);
    }
};

template <typename K, typename V>
struct SplayTreeImpl
    : trait::Mixin<SplayNode<K, V>, trait::TypeTraits, trait::Maintain>,
      trait::Mixin<
          SplayTreeImpl<K, V>, trait::Search, trait::Size, trait::Print, trait::Traverse,
          trait::Merge, trait::Subscript, trait::Conflict, trait::Box, trait::View, trait::Trace,
          trait::TracedBind, trait::Rotate, trait::TracedConstruct, trait::Iterate> {
    friend struct Test;

    std::unique_ptr<SplayNode<K, V>> root{nullptr};

    SplayTreeImpl() = default;
    SplayTreeImpl(std::unique_ptr<SplayNode<K, V>> root) : root(std::move(root)) {
        if (this->root) this->root->parent = nullptr;
    }

    auto name() const -> std::string { return "SplayTree"; }
    auto stringify() const -> std::string { return serializeClass("SplayTree", root); }

    auto insert(const K& key, const V& value) -> Status {
        auto [parent, node] = this->findBox(this->root, key);
        if (node) return Status::FAILED;  // Key already exists
        this->constructNode(node, key, value, parent);
        this->splay(node.get());
        return Status::SUCCESS;
    }

    auto remove(const K& key) -> Status {
        if (!this->find(key)) return Status::FAILED;
        auto [left, right] = this->unbind(this->root);
        this->tracedUntrack(this->root);
        this->root = std::move(left);
        this->join(std::make_unique<SplayTreeImpl<K, V>>(std::move(right)));
        return Status::SUCCESS;
    }

    auto split(const K& key) -> std::unique_ptr<SplayTreeImpl<K, V>> {
        std::unique_ptr<SplayNode<K, V>> other;
        if (this->find(key)) {
            other = std::move(this->root);
            this->root = std::move(this->unbind(other, L));
            other->maintain();
        } else {
            other = this->unbind(this->root, R);
            this->root->maintain();
        }
        return std::make_unique<SplayTreeImpl<K, V>>(std::move(other));
    }

    auto join(std::unique_ptr<SplayTreeImpl<K, V>> other) -> Status {
        if (!other) return Status::FAILED;
        if (!this->root || !other->root) {
            this->root = std::move(other->root ? other->root : this->root);
            return Status::SUCCESS;
        }
        this->splay(this->maxBox(this->root).get());
        this->bind(root, R, std::move(other->root));
        this->root->maintain();
        return Status::SUCCESS;
    }

    auto min() -> Pair<const K, V>* {
        if (!this->root) return nullptr;
        this->splay(this->root->findMin());
        return this->root.get();
    }

    auto max() -> Pair<const K, V>* {
        if (!this->root) return nullptr;
        this->splay(this->root->findMax());
        return this->root.get();
    }

    auto findNode(const K& key) -> SplayNode<K, V>* {
        if (!this->root) return nullptr;
        auto [parent, node] = this->findBox(this->root, key);
        if (!node) {
            this->splay(parent);
            return nullptr;
        }
        this->splay(node.get());
        return this->root.get();
    }

    auto find(const K& key) -> Pair<const K, V>* { return findNode(key); }

    /// NOTE: splay has a relatively more imbalanced structure, which would cause stack overflow in
    /// calling default destructor recursively. So I implement a custom non-recursive clear method.
    void clear() {
        std::vector<std::unique_ptr<SplayNode<K, V>>> stack;
        if (this->root) stack.push_back(std::move(this->root));
        while (!stack.empty()) {
            auto node = std::move(stack.back());
            stack.pop_back();
            if (node->lchild()) stack.push_back(std::move(this->unbind(node, L)));
            if (node->rchild()) stack.push_back(std::move(this->unbind(node, R)));
        }
    }

    ~SplayTreeImpl() { this->clear(); }

private:
    void pushup(SplayNode<K, V>* node) { this->rotate(!node->which(), this->box(node->parent)); }

    void splay(SplayNode<K, V>* node) {
        if (!node) return;
        node->maintain();
        while (node->parent) {
            node->parent->maintain();
            auto parent = node->parent;
            if (parent->parent) {
                node->parent->parent->maintain();
                pushup(node->which() == parent->which() ? parent : node);
            }
            pushup(node);
        }
    }
};
