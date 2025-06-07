#pragma once

#include "debug.hpp"
#include "interface.hpp"
#include "traits/node_traits.hpp"
#include "traits/tree_traits.hpp"
#include "util.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>

// ============================== Definition ================================

template <typename K, typename V> struct BasicTreeImpl;

template <typename K, typename V> using BasicTree = TreeAdapter<K, V, BasicTreeImpl<K, V>>;

// ============================== Implementation ================================

template <typename K, typename V>
struct BasicNode : Pair<const K, V>,
                   trait::TypeTraits<K, V>,
                   trait::Link<BasicNode<K, V>>,
                   trait::Maintain<trait::Size<BasicNode<K, V>>>,
                   trait::Search<BasicNode<K, V>> {

    BasicNode(const K& k, const V& v, BasicNode* parent = nullptr)
        : Pair<const K, V>(k, v), trait::Link<BasicNode<K, V>>(parent) {
        this->maintain();
    }

    auto stringify() const -> std::string {
        return serializeClass(
            "BasicNode", this->key, this->value, this, this->size, this->parent, this->child[L],
            this->child[R]);
    }
};

template <typename K, typename V>
struct BasicTreeImpl
    : trait::Dispatch<
          BasicTreeImpl<K, V>, tree_trait::Search, tree_trait::Clear, tree_trait::Size,
          tree_trait::Print, tree_trait::Traverse, tree_trait::Merge, tree_trait::Subscript,
          tree_trait::Conflict, tree_trait::Box, tree_trait::Detach>,
      trait::Dispatch<
          BasicNode<K, V>, tree_trait::TypeTraits, tree_trait::Maintain, tree_trait::Rotate> {
    friend struct Test;

    std::unique_ptr<BasicNode<K, V>> root{nullptr};
    BasicTreeImpl() = default;
    BasicTreeImpl(std::unique_ptr<BasicNode<K, V>> root) : root(std::move(root)) {
        if (this->root) this->root->parent = nullptr;
    }

    auto insert(const K& key, const V& value) -> Status {
        auto [parent, node] = this->findBox(this->root, key);
        if (node) return Status::FAILED;
        node = std::make_unique<BasicNode<K, V>>(key, value, parent);
        this->maintain(parent);
        return Status::SUCCESS;
    }

    auto remove(const K& key) -> Status {
        auto [parent, node] = this->findBox(this->root, key);
        if (!node) return Status::FAILED;
        if (!node->child[L] || !node->child[R]) {
            this->detach(node);
            this->maintain(parent);
            return Status::SUCCESS;
        }
        auto detached = this->detach(this->maxBox(node->child[L]));
        detached->bind(L, std::move(node->child[L]));
        detached->bind(R, std::move(node->child[R]));
        node = std::move(detached);
        node->parent = parent;
        this->maintain(node.get());
        return Status::SUCCESS;
    }

    auto split(const K& key) -> std::unique_ptr<BasicTreeImpl> {
        auto divide = [&key](auto self, std::unique_ptr<BasicNode<K, V>> node)
            -> std::tuple<std::unique_ptr<BasicNode<K, V>>, std::unique_ptr<BasicNode<K, V>>> {
            if (!node) return {nullptr, nullptr};
            if (key <= node->key) {
                auto [lchild, rchild] = node->unbind();
                auto [lchild_l, lchild_r] = self(self, std::move(lchild));
                node->bind(L, std::move(lchild_r));
                node->bind(R, std::move(rchild));
                node->maintain();
                return {std::move(lchild_l), std::move(node)};
            } else {
                auto [lchild, rchild] = node->unbind();
                auto [rchild_l, rchild_r] = self(self, std::move(rchild));
                node->bind(L, std::move(lchild));
                node->bind(R, std::move(rchild_l));
                node->maintain();
                return {std::move(node), std::move(rchild_r)};
            }
        };
        auto [left, right] = divide(divide, std::move(this->root));
        this->root = std::move(left);
        return std::make_unique<BasicTreeImpl>(std::move(right));
    }

    auto join(std::unique_ptr<BasicTreeImpl> other) -> Status {
        if (!other) return Status::FAILED;
        if (!other->root || !this->root) {
            this->root = std::move(other->root ? other->root : this->root);
            return Status::SUCCESS;
        }
        auto& max_node = this->maxBox(this->root);
        max_node->bind(R, std::move(other->root));
        this->maintain(max_node.get());
        return Status::SUCCESS;
    }

    auto stringify() const -> std::string { return serializeClass("BasicTree", root); }
};
