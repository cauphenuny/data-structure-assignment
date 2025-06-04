#pragma once
#include "interface.hpp"
#include "node_traits.hpp"
#include "tree_traits.hpp"
#include "util.hpp"

#include <string>

namespace crtp {

namespace _avl_impl {
template <typename K, typename V> struct AVLTree;
}

template <typename K, typename V> struct AVLTree : Tree<K, V> {
    auto size() const -> size_t override { return tree->size(); }
    void clear() override { tree->clear(); }
    void print() const override { tree->printCLI(); }
    void printCLI() const override { tree->printCLI(); }
    auto stringify() const -> std::string override { return tree->stringify(); }
    auto insert(const K& key, const V& value) -> Status override {
        return tree->insert(key, value);
    }
    auto remove(const K& key) -> Status override { return tree->remove(key); }
    auto find(const K& key) -> typename Tree<K, V>::PairType* override { return tree->find(key); }
    auto min() -> typename Tree<K, V>::PairType* override { return tree->min(); }
    auto max() -> typename Tree<K, V>::PairType* override { return tree->max(); }
    auto split(const K& key) -> std::unique_ptr<Tree<K, V>> override {
        auto split_tree = tree->split(key);
        return std::make_unique<AVLTree>(std::move(split_tree));
    }
    auto join(std::unique_ptr<Tree<K, V>> other) -> Status override {
        if (auto avl_tree = dynamic_cast<AVLTree<K, V>*>(other.get())) {
            return tree->join(std::move(avl_tree->tree));
        }
        return Status::FAILED;  // cannot join with non-AVL tree
    }
    AVLTree() : tree(std::make_unique<_avl_impl::AVLTree<K, V>>()) {}
    AVLTree(std::unique_ptr<_avl_impl::AVLTree<K, V>> tree) : tree(std::move(tree)) {}

private:
    std::unique_ptr<_avl_impl::AVLTree<K, V>> tree;
};

namespace _avl_impl {

template <typename K, typename V>
struct AVLNode : TypeTraits<K, V>,
                 Pair<const K, V>,
                 trait::Edit<AVLNode<K, V>>,
                 trait::Maintain<trait::Size<AVLNode<K, V>>, trait::Height<AVLNode<K, V>>>,
                 trait::Search<AVLNode<K, V>> {
    AVLNode* parent{nullptr};
    std::unique_ptr<AVLNode<K, V>> lchild{nullptr}, rchild{nullptr};

    AVLNode(const K& k, const V& v, AVLNode* parent = nullptr)
        : Pair<const K, V>(k, v), parent(parent) {}

    auto stringify() const -> std::string {
        return serializeClass(
            "AVLNode", this->key, this->value, this, this->height, this->size, this->parent, lchild,
            rchild);
    }
};

template <typename K, typename V>
struct AVLTree : TypeTraits<K, V>,
                 tree_trait::Search<AVLTree<K, V>>,
                 tree_trait::Clear<AVLTree<K, V>>,
                 tree_trait::Size<AVLTree<K, V>>,
                 tree_trait::Height<AVLTree<K, V>>,
                 tree_trait::PrintCLI<AVLTree<K, V>>,
                 private tree_trait::Box<AVLTree<K, V>>,
                 private tree_trait::Maintain<AVLNode<K, V>>,
                 private tree_trait::Rotate<AVLNode<K, V>>,
                 private tree_trait::Detach<AVLTree<K, V>> {
    friend struct tree_trait::Box<AVLTree<K, V>>;
    friend struct tree_trait::Detach<AVLTree<K, V>>;

    std::unique_ptr<AVLNode<K, V>> root{nullptr};

    AVLTree() = default;
    AVLTree(std::unique_ptr<AVLNode<K, V>> root) : root(std::move(root)) {
        if (this->root) this->root->parent = nullptr;
    }

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
        auto [parent, node] = this->findBox(this->root, key);
        if (node) return Status::FAILED;  // key already exists
        node = std::make_unique<AVLNode<K, V>>(key, value, parent);
        this->maintain(parent);
        this->checkBalance(parent);
        return Status::SUCCESS;
    }

    Status remove(const K& key) {
        auto [parent, node] = this->findBox(this->root, key);
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

    std::unique_ptr<AVLTree<K, V>> split(const K& key) {
        auto tree = [](std::unique_ptr<AVLNode<K, V>> node) {
            return std::make_unique<AVLTree<K, V>>(std::move(node));
        };
        auto divide = [&](auto self, std::unique_ptr<AVLNode<K, V>> node)
            -> std::tuple<std::unique_ptr<AVLTree<K, V>>, std::unique_ptr<AVLTree<K, V>>> {
            if (!node) return {tree(nullptr), tree(nullptr)};
            auto [lchild, rchild] = node->unbind();
            if (key <= node->key) {
                auto [left, right] = self(self, std::move(lchild));
                right->join(std::move(node), tree(std::move(rchild)));
                return {std::move(left), std::move(right)};
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

    Status join(std::unique_ptr<AVLNode<K, V>> mid, std::unique_ptr<AVLTree> right) {
        auto find = [](auto self, bool is_right, int height, AVLNode<K, V>* parent,
                       std::unique_ptr<AVLNode<K, V>>& node)
            -> std::tuple<AVLNode<K, V>*, std::unique_ptr<AVLNode<K, V>>&> {
            if (!node) return {parent, node};
            if (node->height <= height) return {parent, node};
            return self(self, is_right, height, node.get(), is_right ? node->rchild : node->lchild);
        };
        AVLNode<K, V>* insert_pos = nullptr;
        if (this->height() >= right->height()) {
            auto [parent, cut] = find(find, true, right->height() + 1, nullptr, this->root);
            mid->bindL(std::move(cut));
            mid->bindR(std::move(right->root));
            cut = std::move(mid);
            cut->parent = parent;
            insert_pos = cut.get();
        } else {
            auto [parent, cut] = find(find, false, this->height() + 1, nullptr, right->root);
            mid->bindL(std::move(this->root));
            mid->bindR(std::move(cut));
            cut = std::move(mid);
            cut->parent = parent;
            insert_pos = cut.get();
            this->root = std::move(right->root);
        }
        this->maintain(insert_pos);
        this->checkBalance(insert_pos);
        return Status::SUCCESS;
    }

    Status join(std::unique_ptr<AVLTree> other) {
        if (!other) return Status::FAILED;
        if (!other->root || !this->root) {
            this->root = std::move(other->root ? other->root : this->root);
            return Status::SUCCESS;
        }
        if (this->height() >= other->height()) {
            auto mid = other->detach(other->minBox(other->root));
            this->join(std::move(mid), std::move(other));
        } else {
            auto mid = this->detach(this->minBox(this->root));
            this->join(std::move(mid), std::move(other));
        }
        return Status::SUCCESS;
    }

    auto stringify() const -> std::string { return serializeClass("AVLTree", root); }
};
}  // namespace _avl_impl

}  // namespace crtp
