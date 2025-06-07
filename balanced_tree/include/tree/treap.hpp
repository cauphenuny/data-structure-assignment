#pragma once

#include "interface.hpp"
#include "traits/node_traits.hpp"
#include "traits/tree_traits.hpp"
#include "util.hpp"

#include <memory>
#include <random>
#include <string>

// ============================== Definition ================================

template <typename K, typename V> struct TreapImpl;

template <typename K, typename V> using Treap = TreeAdapter<K, V, TreapImpl<K, V>>;

// ============================== Implementation ================================

template <typename K, typename V>
struct TreapNode : Pair<const K, V>,
                   trait::TypeTraits<K, V>,
                   trait::Link<TreapNode<K, V>>,
                   trait::Maintain<trait::Size<TreapNode<K, V>>>,
                   trait::Search<TreapNode<K, V>> {
    int priority;  // Random priority for treap property

    TreapNode(const K& k, const V& v, TreapNode* parent = nullptr)
        : Pair<const K, V>(k, v), trait::Link<TreapNode<K, V>>(parent) {
        this->maintain();
        this->priority = randomPriority();
    }

    static int randomPriority() {
        static thread_local std::mt19937 gen{std::random_device{}()};
        static thread_local std::uniform_int_distribution<int> dist(1, 1 << 30);
        return dist(gen);
    }

    auto stringify() const -> std::string {
        return serializeClass(
            "TreapNode", this->key, this->value, this->priority, this, this->size, this->parent,
            this->child[L], this->child[R]);
    }
};

template <typename K, typename V>
struct TreapImpl
    : trait::Dispatch<
          TreapImpl<K, V>, tree_trait::Search, tree_trait::Clear, tree_trait::Size,
          tree_trait::Print, tree_trait::Traverse, tree_trait::Merge, tree_trait::Subscript,
          tree_trait::Conflict, tree_trait::Box, tree_trait::Detach>,
      trait::Dispatch<
          TreapNode<K, V>, tree_trait::TypeTraits, tree_trait::Maintain, tree_trait::Rotate> {
    friend struct Test;

    std::unique_ptr<TreapNode<K, V>> root{nullptr};

    TreapImpl() = default;
    TreapImpl(std::unique_ptr<TreapNode<K, V>> root) : root(std::move(root)) {
        if (this->root) this->root->parent = nullptr;
    }

    auto stringify() const -> std::string { return serializeClass("Treap", root); }

    auto insert(const K& key, const V& value) -> Status {
        auto [parent, box] = this->findBox(this->root, key);
        if (box) return Status::FAILED;  // Key already exists
        box = std::make_unique<TreapNode<K, V>>(key, value, parent);
        this->maintain(parent);
        auto node = box.get();
        while (parent && node->priority > parent->priority) {
            node = this->pushup(node);
            parent = node->parent;
        }
        return Status::SUCCESS;
    }

    auto remove(const K& key) -> Status {
        auto [left, mid, right] = split(std::move(this->root), key);
        auto ret = Status::SUCCESS;
        if (!mid) ret = Status::FAILED;
        this->root = join(std::move(left), std::move(right));
        return ret;
    }

    auto split(const K& key) -> std::unique_ptr<TreapImpl<K, V>> {
        auto [left, mid, right] = split(std::move(this->root), key);
        this->root = std::move(left);
        return std::make_unique<TreapImpl<K, V>>(join(std::move(mid), std::move(right)));
    }

    auto join(std::unique_ptr<TreapImpl<K, V>> other) -> Status {
        if (!other) return Status::FAILED;
        this->root = join(std::move(this->root), std::move(other->root));
        return Status::SUCCESS;
    }

private:
    TreapNode<K, V>* pushup(TreapNode<K, V>* node) {
        auto& box = this->box(node->parent);
        this->rotate(!node->which(), box);
        return box.get();
    }

    auto split(std::unique_ptr<TreapNode<K, V>> node, const K& key) -> std::tuple<
        std::unique_ptr<TreapNode<K, V>>, std::unique_ptr<TreapNode<K, V>>,
        std::unique_ptr<TreapNode<K, V>>> {
        if (!node) return {nullptr, nullptr, nullptr};
        if (key == node->key) {
            auto [lchild, rchild] = node->unbind();
            return {std::move(lchild), std::move(node), std::move(rchild)};
        } else if (key < node->key) {
            auto [left, mid, right] = split(std::move(node->child[L]), key);
            node->bind(L, std::move(right));
            node->maintain();
            return {std::move(left), std::move(mid), std::move(node)};
        } else {
            auto [left, mid, right] = split(std::move(node->child[R]), key);
            node->bind(R, std::move(left));
            node->maintain();
            return {std::move(node), std::move(mid), std::move(right)};
        }
    }

    auto join(std::unique_ptr<TreapNode<K, V>> left, std::unique_ptr<TreapNode<K, V>> right)
        -> std::unique_ptr<TreapNode<K, V>> {
        if (!left) return right;
        if (!right) return left;
        if (left->priority > right->priority) {
            left->bind(R, join(std::move(left->child[R]), std::move(right)));
            left->maintain();
            return left;
        } else {
            right->bind(L, join(std::move(left), std::move(right->child[L])));
            right->maintain();
            return right;
        }
    }
};
