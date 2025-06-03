/// @file avl.hpp
/// @brief AVL tree implementation

#include "tree.hpp"
#include "util.hpp"

#include <algorithm>

/****************************** Definition ********************************/

template <typename Key, typename Value> struct AVLTree : Tree<Key, Value> {
    struct AVLNode;
    using Tree = Tree<Key, Value>;
    using Node = Tree::Node;

    AVLTree() = default;
    explicit AVLTree(Node* root) : Tree(std::unique_ptr<Node>(root)) {}
    explicit AVLTree(std::unique_ptr<Node> root) : Tree(std::move(root)) {}

    auto stringify() const -> std::string override;
    auto insert(const Key& key, const Value& value) -> Status override;
    auto remove(const Key& key) -> Status override;
    auto split(const Key& key) -> std::unique_ptr<Tree> override;
    auto merge(std::unique_ptr<Tree> other) -> Status override;

    auto height() const -> int;

private:
    static void rotateR(std::unique_ptr<Node>& root);
    static void rotateL(std::unique_ptr<Node>& root);
    static void rotateLR(std::unique_ptr<Node>& root);
    static void rotateRL(std::unique_ptr<Node>& root);
    static auto avl(const std::unique_ptr<Node>& node) -> AVLNode*;
    static auto avl(Node*) -> AVLNode*;
    void balance(Node* node);
    void balanceNode(std::unique_ptr<Node>& node);
    auto join(std::unique_ptr<AVLTree> other) -> Status;
};

/****************************** Implementation ********************************/

template <typename Key, typename Value> struct AVLTree<Key, Value>::AVLNode : Tree::Node {
    using Node = Tree::Node;
    friend struct AVLTree<Key, Value>;
    int height;

    AVLNode(const Key& key, const Value& value, Node* parent = nullptr)
        : Node(key, value, parent), height(1) {}

    auto stringify() const -> std::string override {
        return serializeClass("AVLNode", height) + " : " + this->Node::stringify();
    }

    void maintain() override {
        this->Node::maintain();
        this->height = 1 + std::max(
                               this->lchild ? this->avlLeft()->height : 0,
                               this->rchild ? this->avlRight()->height : 0);
    }

    auto avlLeft() const { return static_cast<AVLNode*>(this->lchild.get()); }
    auto avlRight() const { return static_cast<AVLNode*>(this->rchild.get()); }
    auto avlParent() const { return static_cast<AVLNode*>(this->parent); }
    auto factor() const {
        auto left = this->avlLeft(), right = this->avlRight();
        return (left ? left->height : 0) - (right ? right->height : 0);
    }
};

template <typename K, typename V> auto AVLTree<K, V>::stringify() const -> std::string {
    return "AVLTree : " + this->Tree::stringify();
}

template <typename K, typename V> auto AVLTree<K, V>::height() const -> int {
    return static_cast<AVLNode*>(this->root.get())->height;
}

template <typename K, typename V>
auto AVLTree<K, V>::avl(const std::unique_ptr<Node>& node) -> AVLNode* {
    return static_cast<AVLNode*>(node.get());
}

template <typename K, typename V> auto AVLTree<K, V>::avl(Node* node) -> AVLNode* {
    return static_cast<AVLNode*>(node);
}

template <typename K, typename V> void AVLTree<K, V>::rotateR(std::unique_ptr<Node>& root) {
    auto new_root = root->lchild.release();
    if (new_root->rchild) {
        root->bindL(std::move(new_root->rchild));
    }
    new_root->parent = root->parent;
    new_root->bindR(std::move(root));
    root.reset(new_root);
    Tree::maintain(root->rchild.get());
}

template <typename K, typename V> void AVLTree<K, V>::rotateL(std::unique_ptr<Node>& root) {
    auto new_root = root->rchild.release();
    if (new_root->lchild) {
        root->bindR(std::move(new_root->lchild));
    }
    new_root->parent = root->parent;
    new_root->bindL(std::move(root));
    root.reset(new_root);
    Tree::maintain(root->lchild.get());
}

template <typename K, typename V> void AVLTree<K, V>::rotateLR(std::unique_ptr<Node>& root) {
    rotateL(root->lchild);
    rotateR(root);
}

template <typename K, typename V> void AVLTree<K, V>::rotateRL(std::unique_ptr<Node>& root) {
    rotateR(root->rchild);
    rotateL(root);
}

template <typename K, typename V> void AVLTree<K, V>::balanceNode(std::unique_ptr<Node>& node) {
    if (!node) return;
    auto avl_node = avl(node);
    if (avl_node->factor() > 1) {
        if (avl_node->avlLeft()->factor() >= 0) {
            rotateR(node);
        } else {
            rotateLR(node);
        }
    } else if (avl_node->factor() < -1) {
        if (avl_node->avlRight()->factor() <= 0) {
            rotateL(node);
        } else {
            rotateRL(node);
        }
    }
}

template <typename K, typename V> void AVLTree<K, V>::balance(Node* node) {
    while (node) {
        auto avl_node = avl(node);
        if (avl_node->factor() > 1 || avl_node->factor() < -1) {
            this->balanceNode(this->box(node));
            return;
        }
        node = node->parent;
    }
}

template <typename K, typename V> Status AVLTree<K, V>::insert(const K& key, const V& value) {
    auto [parent, node] = this->findBox(key);
    if (node) return Status::FAILED;  // key already exists
    node = std::make_unique<AVLNode>(key, value, parent);
    Tree::maintain(parent);
    this->balance(parent);
    return Status::SUCCESS;
}

template <typename K, typename V> Status AVLTree<K, V>::remove(const K& key) {
    auto [parent, node] = this->findBox(key);
    if (!node) return Status::FAILED;
    if (!node->lchild || !node->rchild) {
        Tree::detach(node);
        Tree::maintain(parent);
        this->balance(parent);
    } else {
        auto detached = Tree::detach(Tree::max(node->lchild));
        detached->bindL(std::move(node->lchild));
        detached->bindR(std::move(node->rchild));
        node = std::move(detached);
        node->parent = parent;
        Tree::maintain(node.get());
        this->balance(node.get());
    }
    return Status::SUCCESS;
}

template <typename K, typename V> auto AVLTree<K, V>::split(const K&) -> std::unique_ptr<Tree> {
    // TODO:
    return nullptr;
}

template <typename K, typename V> Status AVLTree<K, V>::join(std::unique_ptr<AVLTree> other) {
    if (!other) return Status::FAILED;  // tree does not exist
    if (!this->root || !other->root) {
        this->root = std::move(other->root ? other->root : this->root);
        return Status::SUCCESS;
    }
    auto find = [](auto self, bool is_right, auto height, AVLNode* node) -> Node* {
        if (!node) return nullptr;
        auto next = is_right ? node->avlRight() : node->avlLeft();
        if (node->height <= height) return next ? next : node;
        return self(self, is_right, height, next);
    };
    Node* insert_pos = nullptr;
    if (this->height() >= other->height()) {
        auto mid = other->detach(Tree::min(other->root));
        auto& cut = this->box(find(find, true, other->height() + 1, avl(this->root)));
        auto parent = cut->parent;
        mid->bindL(std::move(cut));
        mid->bindR(std::move(other->root));
        cut = std::move(mid);
        cut->parent = parent;
        insert_pos = cut.get();
    } else {
        auto mid = this->detach(Tree::max(this->root));
        auto& cut = other->box(find(find, false, this->height() + 1, avl(other->root)));
        auto parent = cut->parent;
        mid->bindL(std::move(this->root));
        mid->bindR(std::move(cut));
        cut = std::move(mid);
        cut->parent = parent;
        insert_pos = cut.get();
        this->root = std::move(other->root);
    }
    Tree::maintain(insert_pos);
    this->balance(insert_pos);
    return Status::SUCCESS;
}

template <typename K, typename V> Status AVLTree<K, V>::merge(std::unique_ptr<Tree> other) {
    if (!other) return Status::FAILED;  // tree does not exist
    if (!this->root || !other->root) {
        this->root = std::move(other->root ? other->root : this->root);
        return Status::SUCCESS;
    }
    auto avl_other = dynamic_cast<AVLTree<K, V>*>(other.get());
    auto this_min = this->minimum()->key, this_max = this->maximum()->key;
    auto other_min = other->minimum()->key, other_max = other->maximum()->key;
    if (!avl_other || (this_min <= other_max && other_min <= this_max)) {
        return this->mixin(std::move(other));
    }
    if (other_max < this_min) std::swap(this->root, other->root);
    other.release();
    return this->join(std::unique_ptr<AVLTree>(avl_other));
}
