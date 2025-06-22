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

template <typename K, typename V> using BasicTree = TreeAdapter<K, V, BasicTreeImpl>;

// ============================== Implementation ================================

template <typename K, typename V>
struct BasicNode : Pair<const K, V>,
                   trait::node::TypeTraits<K, V>,
                   trait::node::Link<BasicNode<K, V>>,
                   trait::node::View<BasicNode<K, V>>,
                   trait::node::Maintain<trait::node::Size<BasicNode<K, V>>>,
                   trait::node::Search<BasicNode<K, V>> {

    BasicNode(const K& k, const V& v, BasicNode* parent = nullptr)
        : Pair<const K, V>(k, v), trait::node::Link<BasicNode<K, V>>(parent) {
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
    : trait::Mixin<BasicNode<K, V>, trait::TypeTraits, trait::Maintain>,
      trait::Mixin<
          BasicTreeImpl<K, V>, trait::InsertRemove, trait::Search, trait::Clear, trait::Size,
          trait::Print, trait::Traverse, trait::Merge, trait::Subscript, trait::Conflict,
          trait::Box, trait::Detach, trait::View, trait::Trace, trait::TracedBind,
          trait::TracedConstruct, trait::Iterate> {
    friend struct Test;

    std::unique_ptr<BasicNode<K, V>> root{nullptr};
    BasicTreeImpl() = default;
    BasicTreeImpl(std::unique_ptr<BasicNode<K, V>> root) : root(std::move(root)) {
        if (this->root) this->root->parent = nullptr;
    }

    auto split(const K& key) -> std::unique_ptr<BasicTreeImpl> {
        auto divide = [&key, this](auto self, std::unique_ptr<BasicNode<K, V>> node)
            -> std::tuple<std::unique_ptr<BasicNode<K, V>>, std::unique_ptr<BasicNode<K, V>>> {
            if (!node) return {nullptr, nullptr};
            if (key <= node->key) {
                auto [lchild, rchild] = this->unbind(node);
                node->maintain();
                auto [lchild_l, lchild_r] = self(self, std::move(lchild));
                this->bind(node, L, std::move(lchild_r));
                this->bind(node, R, std::move(rchild));
                node->maintain();
                return {std::move(lchild_l), std::move(node)};
            } else {
                auto [lchild, rchild] = this->unbind(node);
                node->maintain();
                auto [rchild_l, rchild_r] = self(self, std::move(rchild));
                this->bind(node, L, std::move(lchild));
                this->bind(node, R, std::move(rchild_l));
                node->maintain();
                return {std::move(node), std::move(rchild_r)};
            }
        };
        auto [left, right] = divide(divide, std::move(this->root));
        this->root = std::move(left);
        this->tracedUntrack(right);
        return std::make_unique<BasicTreeImpl>(std::move(right));
    }

    auto join(std::unique_ptr<BasicTreeImpl> other) -> Status {
        if (!other) return Status::FAILED;
        if (!other->root || !this->root) {
            this->root = std::move(other->root ? other->root : this->root);
            return Status::SUCCESS;
        }
        this->tracedTrack(other->root);
        auto& max_node = this->maxBox(this->root);
        this->bind(max_node, R, std::move(other->root));
        this->maintain(max_node.get());
        return Status::SUCCESS;
    }

    auto name() const -> std::string { return "BasicTree"; }
    auto stringify() const -> std::string { return serializeClass("BasicTree", root); }
};
