#pragma once

#include "debug.hpp"
#include "node_traits.hpp"
#include "tree_traits.hpp"
#include "util.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>

namespace crtp {  // test

template <typename K, typename V>
struct BasicNode : TypeTraits<K, V>,
                   trait::Edit<BasicNode<K, V>>,
                   trait::Maintain<trait::Size<BasicNode<K, V>>>,
                   trait::Search<BasicNode<K, V>> {
    K key;
    V value;
    BasicNode* parent{nullptr};
    std::unique_ptr<BasicNode<K, V>> lchild{nullptr}, rchild{nullptr};

    BasicNode(const K& k, const V& v, BasicNode* parent = nullptr)
        : key(k), value(v), parent(parent) {}

    auto stringify() const -> std::string {
        return serializeClass(
            "BasicNode", key, value, this, this->size, this->parent, lchild, rchild);
    }
};

template <typename K, typename V>
struct BasicTree : TypeTraits<K, V>,
                   tree_trait::Search<BasicTree<K, V>>,
                   private tree_trait::Box<BasicTree<K, V>>,
                   private tree_trait::Maintain<BasicNode<K, V>>,
                   private tree_trait::Detach<BasicTree<K, V>> {

    std::unique_ptr<BasicNode<K, V>> root{nullptr};

    auto insert(const K& key, const V& value) -> Status {
        auto [parent, node] = this->findBox(key);
        if (node) return Status::FAILED;
        node = std::make_unique<BasicNode<K, V>>(key, value, parent);
        this->maintain(parent);
        return Status::SUCCESS;
    }

    auto remove(const K& key) -> Status {
        auto [parent, node] = this->findBox(key);
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

    auto stringify() const -> std::string { return serializeClass("BasicTree", root); }
};

}  // namespace crtp
