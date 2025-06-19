/// @file treap.hpp
/// @brief Treap implementation

#pragma once
#include "tree.hpp"
#include "util.hpp"

#include <memory>
#include <random>

namespace legacy {

/****************************** Definition ********************************/

template <typename Key, typename Value> struct Treap : Tree<Key, Value> {
    struct TreapNode;
    using Node = Tree<Key, Value>::Node;
    using Tree = Tree<Key, Value>;

    Treap() = default;
    explicit Treap(Node* root) : Tree(std::unique_ptr<Node>(root)) {}
    explicit Treap(std::unique_ptr<Node> root) : Tree(std::move(root)) {}

    auto stringify() const -> std::string override;
    auto name() const -> std::string override;
    auto insert(const Key& key, const Value& value) -> Status override;
    auto remove(const Key& key) -> Status override;
    auto split(const Key& key) -> std::unique_ptr<Tree> override;
    auto merge(std::unique_ptr<Tree> other) -> Status override;

protected:
    static auto treap(const std::unique_ptr<Node>& node) -> TreapNode*;
    static auto treap(Node* node) -> TreapNode*;
    static auto split(std::unique_ptr<Node> node, const Key& key)
        -> std::tuple<std::unique_ptr<Node>, std::unique_ptr<Node>, std::unique_ptr<Node>>;
    static auto join(std::unique_ptr<Node> left, std::unique_ptr<Node> right)
        -> std::unique_ptr<Node>;

    static int randomPriority() {
        static thread_local std::mt19937 gen{std::random_device{}()};
        static thread_local std::uniform_int_distribution<int> dist(1, 1 << 30);
        return dist(gen);
    }
};

/****************************** Implementation ********************************/

template <typename Key, typename Value> struct Treap<Key, Value>::TreapNode : Tree::Node {
    using Node = Tree::Node;
    friend struct Treap<Key, Value>;

    int priority;

    TreapNode(const Key& key, const Value& value, int priority, Node* parent = nullptr)
        : Node(key, value, parent), priority(priority) {}

    auto stringify() const -> std::string override {
        return serializeClass("TreapNode", priority) + " : " + this->Node::stringify();
    }
};

template <typename K, typename V>
auto Treap<K, V>::treap(const std::unique_ptr<Node>& node) -> TreapNode* {
    return static_cast<TreapNode*>(node.get());
}
template <typename K, typename V> auto Treap<K, V>::treap(Node* node) -> TreapNode* {
    return static_cast<TreapNode*>(node);
}

template <typename K, typename V> auto Treap<K, V>::stringify() const -> std::string {
    return "Treap : " + this->Tree::stringify();
}

template <typename K, typename V> auto Treap<K, V>::name() const -> std::string {
    return "legacy::Treap";
}

template <typename K, typename V>
auto Treap<K, V>::split(std::unique_ptr<Node> node, const K& key)
    -> std::tuple<std::unique_ptr<Node>, std::unique_ptr<Node>, std::unique_ptr<Node>> {
    if (!node) return {nullptr, nullptr, nullptr};
    if (key == node->key) {
        auto [lchild, rchild] = node->unbind();
        return {std::move(lchild), std::move(node), std::move(rchild)};
    } else if (key < node->key) {
        auto [left, mid, right] = split(node->unbindL(), key);
        node->bindL(std::move(right));
        node->maintain();
        return {std::move(left), std::move(mid), std::move(node)};
    } else {
        auto [left, mid, right] = split(node->unbindR(), key);
        node->bindR(std::move(left));
        node->maintain();
        return {std::move(node), std::move(mid), std::move(right)};
    }
}

template <typename K, typename V>
auto Treap<K, V>::join(std::unique_ptr<Node> left, std::unique_ptr<Node> right)
    -> std::unique_ptr<Node> {
    if (!left) return std::move(right);
    if (!right) return std::move(left);
    if (treap(left)->priority > treap(right)->priority) {
        left->bindR(join(std::move(left->rchild), std::move(right)));
        left->maintain();
        return std::move(left);
    } else {
        right->bindL(join(std::move(left), std::move(right->lchild)));
        right->maintain();
        return std::move(right);
    }
}

template <typename K, typename V> auto Treap<K, V>::split(const K& key) -> std::unique_ptr<Tree> {
    auto [left, mid, right] = split(std::move(this->root), key);
    this->root = std::move(left);
    return std::make_unique<Treap>(join(std::move(mid), std::move(right)));
}

template <typename K, typename V> auto Treap<K, V>::merge(std::unique_ptr<Tree> other) -> Status {
    if (!other) return Status::FAILED;  // tree does not exist
    if (!this->root || !other->root) {
        this->root = std::move(other->root ? other->root : this->root);
        return Status::SUCCESS;
    }
    auto treap_other = dynamic_cast<Treap<K, V>*>(other.get());
    auto this_min = this->minimum()->key, this_max = this->maximum()->key;
    auto other_min = other->minimum()->key, other_max = other->maximum()->key;
    if (!treap_other || (this_min <= other_max && other_min <= this_max)) {
        return this->mixin(std::move(other));
    }
    if (other_max < this_min) std::swap(this->root, other->root);
    this->root = join(std::move(this->root), std::move(other->root));
    return Status::SUCCESS;
}

template <typename K, typename V> auto Treap<K, V>::insert(const K& key, const V& value) -> Status {
    auto [left, mid, right] = split(std::move(this->root), key);
    auto ret = Status::SUCCESS;
    if (mid) {
        ret = Status::FAILED;
    } else {
        mid = std::make_unique<TreapNode>(key, value, randomPriority());
    }
    this->root = join(std::move(left), join(std::move(mid), std::move(right)));
    return ret;
}

template <typename K, typename V> auto Treap<K, V>::remove(const K& key) -> Status {
    auto [left, mid, right] = split(std::move(this->root), key);
    auto ret = Status::SUCCESS;
    if (!mid) ret = Status::FAILED;
    this->root = join(std::move(left), std::move(right));
    return ret;
}

}  // namespace legacy
