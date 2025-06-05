#pragma once

#include "debug.hpp"
#include "interface.hpp"
#include "node_traits.hpp"
#include "tree_traits.hpp"
#include "util.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>

namespace crtp {  // test

// ============================== Definition ================================

template <typename K, typename V> struct BasicTreeImpl;

template <typename K, typename V> using BasicTree = TreeImpl<K, V, BasicTreeImpl<K, V>>;

// ============================== Implementation ================================

template <typename K, typename V>
struct BasicNode : TypeTraits<K, V>,
                   Pair<const K, V>,
                   trait::Link<BasicNode<K, V>>,
                   trait::Maintain<trait::Size<BasicNode<K, V>>>,
                   trait::Search<BasicNode<K, V>> {

    BasicNode(const K& k, const V& v, BasicNode* parent = nullptr)
        : Pair<const K, V>(k, v), trait::Link<BasicNode<K, V>>(parent) {}

    auto stringify() const -> std::string {
        return serializeClass(
            "BasicNode", this->key, this->value, this, this->size, this->parent, this->lchild,
            this->rchild);
    }
};

template <typename K, typename V>
struct BasicTreeImpl : TypeTraits<K, V>,
                       tree_trait::Search<BasicTreeImpl<K, V>>,
                       tree_trait::Clear<BasicTreeImpl<K, V>>,
                       tree_trait::Size<BasicTreeImpl<K, V>>,
                       tree_trait::Print<BasicTreeImpl<K, V>>,
                       tree_trait::Traverse<BasicTreeImpl<K, V>>,
                       tree_trait::Merge<BasicTreeImpl<K, V>>,
                       private tree_trait::Box<BasicTreeImpl<K, V>>,
                       private tree_trait::Maintain<BasicNode<K, V>>,
                       private tree_trait::Detach<BasicTreeImpl<K, V>> {
    friend struct tree_trait::Box<BasicTreeImpl<K, V>>;
    friend struct tree_trait::Subscript<BasicTreeImpl<K, V>>;

    std::unique_ptr<BasicNode<K, V>> root{nullptr};

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
        if (!node->lchild || !node->rchild) {
            this->detach(node);
            this->maintain(parent);
            return Status::SUCCESS;
        }
        auto detached = this->detach(this->maxBox(node->lchild));
        detached->bindL(std::move(node->lchild));
        detached->bindR(std::move(node->rchild));
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
                node->bindL(std::move(lchild_r));
                node->bindR(std::move(rchild));
                node->maintain();
                return {std::move(lchild_l), std::move(node)};
            } else {
                auto [lchild, rchild] = node->unbind();
                auto [rchild_l, rchild_r] = self(self, std::move(rchild));
                node->bindL(std::move(lchild));
                node->bindR(std::move(rchild_l));
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
        max_node->bindR(std::move(other->root));
        this->maintain(max_node.get());
        return Status::SUCCESS;
    }

    auto stringify() const -> std::string { return serializeClass("BasicTree", root); }
};

}  // namespace crtp
