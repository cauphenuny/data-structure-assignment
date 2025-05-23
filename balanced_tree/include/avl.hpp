/// @file avl.hpp
/// @brief AVL tree implementation

#include "tree.hpp"

/****************************** Definition ********************************/

template <typename Key, typename Value> struct AVLNode;

template <typename Key, typename Value> struct AVLTree : Tree<Key, Value> {
    using Node = AVLNode<Key, Value>;
    using Tree = Tree<Key, Value>;

    auto stringify() const -> std::string override;
    void insert(const Key& key, const Value& value) override;
    void remove(const Key& key) override;
    auto split(const Key& key) -> Tree* override;
    void merge(Tree* other) override;
};

/****************************** Implementation ********************************/

template <typename Key, typename Value> struct AVLNode : Node<Key, Value> {
    int height;
    AVLNode(const Key& key, const Value& value, Node<Key, Value>* parent = nullptr)
        : Node<Key, Value>(key, value, parent), height(1) {}
    auto stringify() const -> std::string override;
};

template <typename Key, typename Value> auto AVLNode<Key, Value>::stringify() const -> std::string {
    return serializeClass("AVLNode", height) + " | " + this->Node<Key, Value>::stringify();
}

template <typename Key, typename Value> auto AVLTree<Key, Value>::stringify() const -> std::string {
    return this->Tree::stringify();
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::insert(const Key& key, const Value& value) {
    // TODO:
}

template <typename Key, typename Value> void AVLTree<Key, Value>::remove(const Key& key) {
    // TODO:
}

template <typename Key, typename Value> auto AVLTree<Key, Value>::split(const Key& key) -> Tree* {
    // TODO:
    return nullptr;
}

template <typename Key, typename Value> void AVLTree<Key, Value>::merge(Tree* other) {
    // TODO:
}
