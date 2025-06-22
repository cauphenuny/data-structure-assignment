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

template <typename K, typename V> using Treap = TreeAdapter<K, V, TreapImpl>;

// ============================== Implementation ================================

template <typename K, typename V>
struct TreapNode : Pair<const K, V>,
                   trait::node::TypeTraits<K, V>,
                   trait::node::Link<TreapNode<K, V>>,
                   trait::node::View<TreapNode<K, V>>,
                   trait::node::Maintain<trait::node::Size<TreapNode<K, V>>>,
                   trait::node::Search<TreapNode<K, V>> {
    int priority;  // Random priority for treap property

    TreapNode(const K& k, const V& v, TreapNode* parent = nullptr)
        : Pair<const K, V>(k, v), trait::node::Link<TreapNode<K, V>>(parent) {
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
    : trait::Mixin<TreapNode<K, V>, trait::TypeTraits, trait::Maintain>,
      trait::Mixin<
          TreapImpl<K, V>, trait::Search, trait::Clear, trait::Size, trait::Print, trait::Traverse,
          trait::Merge, trait::Subscript, trait::Conflict, trait::Box, trait::Detach, trait::View,
          trait::Trace, trait::TracedBind, trait::Rotate, trait::TracedConstruct, trait::Iterate> {
    friend struct Test;

    std::unique_ptr<TreapNode<K, V>> root{nullptr};

    TreapImpl() = default;
    TreapImpl(std::unique_ptr<TreapNode<K, V>> root) : root(std::move(root)) {
        if (this->root) this->root->parent = nullptr;
    }

    auto name() const -> std::string { return "Treap"; }
    auto stringify() const -> std::string { return serializeClass("Treap", root); }

    auto insert(const K& key, const V& value) -> Status {
        auto [parent, node] = this->findBox(this->root, key);
        if (node) return Status::FAILED;  // Key already exists
        this->constructNode(node, key, value, parent);
        this->maintain(parent);
        auto ptr = node.get();
        while (parent && ptr->priority > parent->priority) {
            ptr = this->pushup(ptr);
            parent = ptr->parent;
        }
        return Status::SUCCESS;
    }

    auto remove(const K& key) -> Status {
        auto [left, mid, right] = split(std::move(this->root), key);
        auto ret = Status::SUCCESS;
        if (!mid) ret = Status::FAILED;
        this->tracedUntrack(mid);
        this->root = join(std::move(left), std::move(right));
        return ret;
    }

    auto split(const K& key) -> std::unique_ptr<TreapImpl<K, V>> {
        auto [left, mid, right] = split(std::move(this->root), key);
        this->root = std::move(left);
        auto joined = this->join(std::move(mid), std::move(right));
        this->tracedUntrack(joined);
        return std::make_unique<TreapImpl<K, V>>(std::move(joined));
    }

    auto join(std::unique_ptr<TreapImpl<K, V>> other) -> Status {
        if (!other) return Status::FAILED;
        this->tracedTrack(other->root);
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
            auto [lchild, rchild] = this->unbind(node);
            node->maintain();
            return {std::move(lchild), std::move(node), std::move(rchild)};
        } else if (key < node->key) {
            auto [left, mid, right] = split(this->unbind(node, L), key);
            this->bind(node, L, std::move(right));
            node->maintain();
            return {std::move(left), std::move(mid), std::move(node)};
        } else {
            auto [left, mid, right] = split(this->unbind(node, R), key);
            this->bind(node, R, std::move(left));
            node->maintain();
            return {std::move(node), std::move(mid), std::move(right)};
        }
    }

    auto join(std::unique_ptr<TreapNode<K, V>> left, std::unique_ptr<TreapNode<K, V>> right)
        -> std::unique_ptr<TreapNode<K, V>> {
        if (!left) return right;
        if (!right) return left;
        if (left->priority > right->priority) {
            this->bind(left, R, join(this->unbind(left, R), std::move(right)));
            left->maintain();
            return left;
        } else {
            this->bind(right, L, join(std::move(left), this->unbind(right, L)));
            right->maintain();
            return right;
        }
    }
};
