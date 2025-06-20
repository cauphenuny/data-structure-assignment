#pragma once

#include "debug.hpp"
#include "interface.hpp"
#include "traits/node_traits.hpp"
#include "traits/tree_traits.hpp"
#include "util.hpp"

#include <cassert>
#include <memory>
#include <string>

// ============================== Definition ================================

template <typename K, typename V> struct AVLTreeImpl;

template <typename K, typename V> using AVLTree = TreeAdapter<K, V, AVLTreeImpl>;

// ============================== Implementation ================================

template <typename K, typename V>
struct AVLNode
    : Pair<const K, V>,
      trait::node::TypeTraits<K, V>,
      trait::node::Link<AVLNode<K, V>>,
      trait::node::View<AVLNode<K, V>>,
      trait::node::Maintain<trait::node::Size<AVLNode<K, V>>, trait::node::Height<AVLNode<K, V>>>,
      trait::node::Search<AVLNode<K, V>> {

    AVLNode(const K& k, const V& v, AVLNode* parent = nullptr)
        : Pair<const K, V>(k, v), trait::node::Link<AVLNode<K, V>>(parent) {
        this->maintain();
    }

    auto stringify() const -> std::string {
        return serializeClass(
            "AVLNode", this->key, this->value, this, this->height, this->size, this->parent,
            this->child[L], this->child[R]);
    }
};

template <typename K, typename V>
struct AVLTreeImpl
    : trait::Mixin<
          AVLTreeImpl<K, V>, trait::Search, trait::Clear, trait::Size, trait::Height, trait::Print,
          trait::Traverse, trait::Merge, trait::Subscript, trait::Conflict, trait::Box,
          trait::Detach, trait::View, trait::Record, trait::BindRecord, trait::Rotate>,
      trait::Mixin<AVLNode<K, V>, trait::TypeTraits, trait::Maintain> {
    friend struct Test;

    std::unique_ptr<AVLNode<K, V>> root{nullptr};

    AVLTreeImpl() = default;
    AVLTreeImpl(std::unique_ptr<AVLNode<K, V>> root) : root(std::move(root)) {
        if (this->root) this->root->parent = nullptr;
    }

    Status insert(const K& key, const V& value) {
        auto [parent, node] = this->findBox(this->root, key);
        if (node) return Status::FAILED;  // key already exists
        node = std::make_unique<AVLNode<K, V>>(key, value, parent);
        this->checkBalance(parent);
        return Status::SUCCESS;
    }

    Status remove(const K& key) {
        auto [parent, node] = this->findBox(this->root, key);
        if (!node) return Status::FAILED;
        if (!node->child[L] || !node->child[R]) {
            this->detach(node);
            this->checkBalance(parent);
        } else {
            auto detached = this->detach(this->maxBox(node->child[L]));
            this->bind(detached, L, std::move(node->child[L]));
            this->bind(detached, R, std::move(node->child[R]));
            node = std::move(detached);
            node->parent = parent;
            this->checkBalance(node.get());
        }
        return Status::SUCCESS;
    }

    std::unique_ptr<AVLTreeImpl<K, V>> split(const K& key) {
        auto tree = [](std::unique_ptr<AVLNode<K, V>> node) {
            return std::make_unique<AVLTreeImpl<K, V>>(std::move(node));
        };
        auto divide = [&](auto self, std::unique_ptr<AVLNode<K, V>> node)
            -> std::tuple<std::unique_ptr<AVLTreeImpl<K, V>>, std::unique_ptr<AVLTreeImpl<K, V>>> {
            if (!node) return {tree(nullptr), tree(nullptr)};
            auto [lchild, rchild] = this->unbind(node);
            node->maintain();
            if (key <= node->key) {
                auto [left, mid] = self(self, std::move(lchild));
                mid->join(std::move(node), tree(std::move(rchild)));
                return {std::move(left), std::move(mid)};
            } else {
                auto [mid, right] = self(self, std::move(rchild));
                auto left = tree(std::move(lchild));
                left->join(std::move(node), std::move(mid));
                return {std::move(left), std::move(right)};
            }
        };
        auto [left, right] = divide(divide, std::move(this->root));
        this->root = std::move(left->root);
        return std::move(right);
    }

    Status join(std::unique_ptr<AVLTreeImpl> other) {
        if (!other) return Status::FAILED;
        if (!other->root || !this->root) {
            this->root = std::move(other->root ? other->root : this->root);
            return Status::SUCCESS;
        }
        if (this->height() >= other->height()) {
            auto mid = other->detach(other->minBox(other->root));
            this->join(std::move(mid), std::move(other));
        } else {
            auto mid = this->detach(this->maxBox(this->root));
            this->join(std::move(mid), std::move(other));
        }
        return Status::SUCCESS;
    }

    auto name() const -> std::string { return "AVLTree"; }
    auto stringify() const -> std::string { return serializeClass("AVLTree", root); }

private:
    bool balance(std::unique_ptr<AVLNode<K, V>>& node) {
        int prev = node->height;
        if (node->balanceFactor() > 1) {
            if (node->child[L]->balanceFactor() >= 0) {
                this->rotateR(node);
            } else {
                this->rotateLR(node);
            }
        } else if (node->balanceFactor() < -1) {
            if (node->child[R]->balanceFactor() <= 0) {
                this->rotateL(node);
            } else {
                this->rotateRL(node);
            }
        }
        return prev == node->height;
    }

    void checkBalance(AVLNode<K, V>* node) {
        while (node) {
            node->maintain();
            if (node->balanceFactor() > 1 || node->balanceFactor() < -1) {
                if (this->balance(this->box(node))) break;
            }
            node = node->parent;
        }
        this->maintain(node);
    }

    Status join(std::unique_ptr<AVLNode<K, V>> mid, std::unique_ptr<AVLTreeImpl> right) {
        auto find = [](auto self, bool is_right, int height, AVLNode<K, V>* parent,
                       std::unique_ptr<AVLNode<K, V>>& node)
            -> std::tuple<AVLNode<K, V>*, std::unique_ptr<AVLNode<K, V>>&> {
            if (!node) return {parent, node};
            if (node->height <= height) return {parent, node};
            return self(
                self, is_right, height, node.get(), is_right ? node->child[R] : node->child[L]);
        };
        AVLNode<K, V>* insert_pos = nullptr;
        if (this->height() >= right->height()) {
            auto [parent, cut] = find(find, true, right->height() + 1, nullptr, this->root);
            this->bind(mid, L, std::move(cut));
            this->bind(mid, R, std::move(right->root));
            cut = std::move(mid);
            cut->parent = parent;
            insert_pos = cut.get();
        } else {
            auto [parent, cut] = find(find, false, this->height() + 1, nullptr, right->root);
            this->bind(mid, L, std::move(this->root));
            this->bind(mid, R, std::move(cut));
            cut = std::move(mid);
            cut->parent = parent;
            insert_pos = cut.get();
            this->root = std::move(right->root);
        }
        this->checkBalance(insert_pos);
        return Status::SUCCESS;
    }
};
