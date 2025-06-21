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
struct AVLTreeImpl : trait::Mixin<AVLNode<K, V>, trait::TypeTraits, trait::Maintain>,
                     trait::Mixin<
                         AVLTreeImpl<K, V>, trait::Search, trait::Clear, trait::Size, trait::Height,
                         trait::Print, trait::Traverse, trait::Merge, trait::Subscript,
                         trait::Conflict, trait::Box, trait::Detach, trait::View, trait::Trace,
                         trait::TracedBind, trait::TracedConstruct, trait::Rotate> {
    friend struct Test;

    std::unique_ptr<AVLNode<K, V>> root{nullptr};

    AVLTreeImpl() = default;
    AVLTreeImpl(std::unique_ptr<AVLNode<K, V>> root) : root(std::move(root)) {
        if (this->root) this->root->parent = nullptr;
    }

    Status insert(const K& key, const V& value) {
        auto [parent, node] = this->findBox(this->root, key);
        if (node) return Status::FAILED;  // key already exists
        this->constructNode(node, key, value, parent);
        this->checkBalance(parent);
        return Status::SUCCESS;
    }

    Status remove(const K& key) {
        auto [parent, node] = this->findBox(this->root, key);
        if (!node) return Status::FAILED;
        if (!node->child[L] || !node->child[R]) {
            auto detached = this->detach(node);
            this->tracedUntrack(detached);
            this->checkBalance(parent);
        } else {
            auto detached = this->detach(this->maxBox(node->child[L]));
            auto lchild = this->unbind(node, L);
            auto rchild = this->unbind(node, R);
            this->overwriteNode(node, std::move(detached));
            this->bind(node, L, std::move(lchild));
            this->bind(node, R, std::move(rchild));
            this->checkBalance(node.get());
        }
        return Status::SUCCESS;
    }

    std::unique_ptr<AVLTreeImpl<K, V>> split(const K& key) {
        auto divide = [&](auto self, std::unique_ptr<AVLNode<K, V>> node)
            -> std::tuple<std::unique_ptr<AVLNode<K, V>>, std::unique_ptr<AVLNode<K, V>>> {
            if (!node) return {nullptr, nullptr};
            auto [lchild, rchild] = this->unbind(node);
            node->maintain();
            if (key <= node->key) {
                auto [left, mid] = self(self, std::move(lchild));
                this->join(mid, std::move(node), std::move(rchild));
                return {std::move(left), std::move(mid)};
            } else {
                auto [mid, right] = self(self, std::move(rchild));
                auto left = std::move(lchild);
                this->join(left, std::move(node), std::move(mid));
                return {std::move(left), std::move(right)};
            }
        };
        auto [left, right] = divide(divide, std::move(this->root));
        this->root = std::move(left);
        this->tracedUntrack(right);
        return std::make_unique<AVLTreeImpl<K, V>>(std::move(right));
    }

    Status join(std::unique_ptr<AVLTreeImpl> other) {
        if (!other) return Status::FAILED;
        if (!other->root || !this->root) {
            if (other->root)
                this->moveNode(this->root, std::move(other->root), (AVLNode<K, V>*)nullptr);
            return Status::SUCCESS;
        }
        auto other_root = std::move(other->root);
        this->tracedTrack(other_root);
        if (this->height() >= other->height()) {
            auto mid = this->detach(this->minBox(other_root));
            this->join(this->root, std::move(mid), std::move(other_root));
        } else {
            auto mid = this->detach(this->maxBox(this->root));
            this->join(this->root, std::move(mid), std::move(other_root));
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

    Status join(
        std::unique_ptr<AVLNode<K, V>>& left, std::unique_ptr<AVLNode<K, V>> mid,
        std::unique_ptr<AVLNode<K, V>> right) {
        auto find = [](auto self, bool is_right, int height, AVLNode<K, V>* parent,
                       std::unique_ptr<AVLNode<K, V>>& node)
            -> std::tuple<AVLNode<K, V>*, std::unique_ptr<AVLNode<K, V>>&> {
            if (!node) return {parent, node};
            if (node->height <= height) return {parent, node};
            return self(
                self, is_right, height, node.get(), is_right ? node->child[R] : node->child[L]);
        };
        AVLNode<K, V>* insert_pos = nullptr;
        auto left_height = left ? left->height : 0;
        auto right_height = right ? right->height : 0;
        if (left_height >= right_height) {
            auto [parent, cut] = find(find, true, right_height + 1, nullptr, left);
            this->bind(mid, L, std::move(cut));
            this->bind(mid, R, std::move(right));
            this->moveNode(cut, std::move(mid), parent);
            insert_pos = cut.get();
        } else {
            auto [parent, cut] = find(find, false, left_height + 1, nullptr, right);
            this->bind(mid, L, std::move(left));
            this->bind(mid, R, std::move(cut));
            this->moveNode(cut, std::move(mid), parent);
            insert_pos = cut.get();
            left = std::move(right);
        }
        this->checkBalance(insert_pos);
        return Status::SUCCESS;
    }
};
