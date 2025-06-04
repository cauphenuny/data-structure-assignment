#pragma once
#include "node_traits.hpp"
#include "tree_traits.hpp"
#include "util.hpp"

#include <string>

namespace crtp {

template <typename K, typename V>
struct AVLNode : TypeTraits<K, V>,
                 trait::Edit<AVLNode<K, V>>,
                 trait::Maintain<trait::Size<AVLNode<K, V>>, trait::Height<AVLNode<K, V>>>,
                 trait::Search<AVLNode<K, V>> {
    K key;
    V value;
    AVLNode* parent{nullptr};
    std::unique_ptr<AVLNode<K, V>> lchild{nullptr}, rchild{nullptr};

    AVLNode(const K& k, const V& v, AVLNode* parent = nullptr) : key(k), value(v), parent(parent) {}

    auto stringify() const -> std::string {
        return serializeClass(
            "AVLNode", key, value, this, this->height, this->size, this->parent, lchild, rchild);
    }
};

template <typename K, typename V>
struct AVLTree : TypeTraits<K, V>,
                 tree_trait::Search<AVLTree<K, V>>,
                 private tree_trait::Box<AVLTree<K, V>>,
                 private tree_trait::Maintain<AVLNode<K, V>>,
                 private tree_trait::Rotate<AVLNode<K, V>>,
                 private tree_trait::Detach<AVLTree<K, V>> {

    std::unique_ptr<AVLNode<K, V>> root{nullptr};

    bool balance(std::unique_ptr<AVLNode<K, V>>& node) {
        int prev = node->height;
        if (node->balanceFactor() > 1) {
            if (node->lchild->balanceFactor() >= 0) {
                this->rotateR(node);
            } else {
                this->rotateLR(node);
            }
        } else if (node->balanceFactor() < -1) {
            if (node->rchild->balanceFactor() <= 0) {
                this->rotateL(node);
            } else {
                this->rotateRL(node);
            }
        }
        this->maintain(node.get());
        return prev != node->height;
    }

    void checkBalance(AVLNode<K, V>* node) {
        while (node) {
            if (node->balanceFactor() > 1 || node->balanceFactor() < -1) {
                if (!this->balance(this->box(node))) return;
            }
            node = node->parent;
        }
    }

    Status insert(const K& key, const V& value) {
        auto [parent, node] = this->findBox(key);
        if (node) return Status::FAILED;  // key already exists
        node = std::make_unique<AVLNode<K, V>>(key, value, parent);
        this->maintain(parent);
        this->checkBalance(parent);
        return Status::SUCCESS;
    }

    Status remove(const K& key) {
        auto [parent, node] = this->findBox(key);
        if (!node) return Status::FAILED;
        if (!node->lchild || !node->rchild) {
            this->detach(node);
            this->maintain(parent);
            this->checkBalance(parent);
        } else {
            auto detached = this->detach(this->maxBox(node->lchild));
            detached->bindL(std::move(node->lchild));
            detached->bindR(std::move(node->rchild));
            node = std::move(detached);
            node->parent = parent;
            this->maintain(node.get());
            this->checkBalance(parent);
        }
        return Status::SUCCESS;
    }

    auto stringify() const -> std::string { return serializeClass("AVLTree", root); }
};
}  // namespace crtp
