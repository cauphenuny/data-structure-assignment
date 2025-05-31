/// @file avl.hpp
/// @brief AVL tree implementation

#include "tree.hpp"
#include "util.hpp"

#include <algorithm>

/****************************** Definition ********************************/

template <typename Key, typename Value> struct AVLNode;

template <typename Key, typename Value> struct AVLTree : Tree<Key, Value> {
    using Node = Node<Key, Value>;
    using AVLNode = AVLNode<Key, Value>;
    using Tree = Tree<Key, Value>;

    auto stringify() const -> std::string override;
    auto insert(const Key& key, const Value& value) -> Status override;
    auto remove(const Key& key) -> Status override;
    auto split(const Key& key) -> std::unique_ptr<Tree> override;
    auto merge(std::unique_ptr<Tree> other) -> Status override;

private:
    static void rotateR(std::unique_ptr<Node>& root);
    static void rotateL(std::unique_ptr<Node>& root);
    static void rotateLR(std::unique_ptr<Node>& root);
    static void rotateRL(std::unique_ptr<Node>& root);
};

/****************************** Implementation ********************************/

template <typename Key, typename Value> struct AVLNode : Node<Key, Value> {
    using Node = Node<Key, Value>;
    int height;

    AVLNode(const Key& key, const Value& value, Node* parent = nullptr)
        : Node(key, value, parent), height(1) {}

    auto stringify() const -> std::string override {
        return serializeClass("AVLNode", height) + " : " + this->Node::stringify();
    }
    void refresh() override {
        this->Node::refresh();
        this->height = 1 + std::max(
                               this->lchild ? this->avlLeft()->height : 0,
                               this->rchild ? this->avlRight()->height : 0);
    }

    AVLNode* avlLeft() { return static_cast<AVLNode*>(this->lchild.get()); }
    AVLNode* avlRight() { return static_cast<AVLNode*>(this->rchild.get()); }
    AVLNode* avlParent() { return static_cast<AVLNode*>(this->parent); }
    int factor() {
        auto left = this->avlLeft(), right = this->avlRight();
        return (left ? left->height : 0) - (right ? right->height : 0);
    }
};

template <typename K, typename V> auto AVLTree<K, V>::stringify() const -> std::string {
    return "AVLTree : " + this->Tree::stringify();
}

template <typename K, typename V> void AVLTree<K, V>::rotateR(std::unique_ptr<Node>& root) {
    auto new_root = root->lchild.release();
    if (new_root->rchild) {
        new_root->rchild->parent = root.get();
        root->lchild.reset(new_root->rchild.release());
    }
    new_root->parent = root->parent;
    root->parent = new_root;
    new_root->rchild.reset(root.release());
    root.reset(new_root);
    Tree::refresh(root->rchild.get());
}

template <typename K, typename V> void AVLTree<K, V>::rotateL(std::unique_ptr<Node>& root) {
    auto new_root = root->rchild.release();
    if (new_root->lchild) {
        new_root->lchild->parent = root.get();
        root->rchild.reset(new_root->lchild.release());
    }
    new_root->parent = root->parent;
    root->parent = new_root;
    new_root->lchild.reset(root.release());
    root.reset(new_root);
    Tree::refresh(root->lchild.get());
}

template <typename K, typename V> void AVLTree<K, V>::rotateLR(std::unique_ptr<Node>& root) {
    rotateL(root->lchild);
    rotateR(root);
}

template <typename K, typename V> void AVLTree<K, V>::rotateRL(std::unique_ptr<Node>& root) {
    rotateR(root->rchild);
    rotateL(root);
}

template <typename K, typename V> Status AVLTree<K, V>::insert(const K& key, const V& value) {
    if (!this->root) {
        this->root = std::make_unique<AVLNode>(key, value);
        return Status::SUCCESS;
    }

    auto search = [&key](auto self, std::unique_ptr<Node>& parent)
        -> std::tuple<bool, std::unique_ptr<Node>&, std::unique_ptr<Node>&> {
        if (key == parent->key) return {true, parent, parent};
        if (key < parent->key) {
            if (parent->lchild) return self(self, parent->lchild);
            return {false, parent, parent->lchild};
        } else {
            if (parent->rchild) return self(self, parent->rchild);
            return {false, parent, parent->rchild};
        }
    };
    auto [exist, parent, node] = search(search, this->root);
    if (exist) {
        return Status::FAILED;
    }
    node = std::make_unique<AVLNode>(key, value, parent.get());
    Tree::refresh(node.get());

    auto find_imbalance =
        [&key](auto self, std::unique_ptr<Node>& node) -> std::tuple<bool, std::unique_ptr<Node>&> {
        if (key == node->key) return {false, node};
        auto avl_node = static_cast<AVLNode*>(node.get());
        auto [found, rotate_node] = self(self, key < node->key ? node->lchild : node->rchild);
        if (found) return {true, rotate_node};
        if (avl_node->factor() > 1 || avl_node->factor() < -1) {
            return {true, node};
        }
        return {false, node};
    };

    auto [need_rotate, rotate_node] = find_imbalance(find_imbalance, this->root);

    if (need_rotate) {
        auto avl_node = static_cast<AVLNode*>(rotate_node.get());
        auto avl_lchild = avl_node->avlLeft(), avl_rchild = avl_node->avlRight();
        if (avl_node->factor() > 1) {
            if (avl_lchild->factor() >= 0) {
                rotateR(rotate_node);
            } else {
                rotateLR(rotate_node);
            }
        } else if (avl_node->factor() < -1) {
            if (avl_rchild->factor() <= 0) {
                rotateL(rotate_node);
            } else {
                rotateRL(rotate_node);
            }
        }
    }

    return Status::SUCCESS;
}

template <typename K, typename V> Status AVLTree<K, V>::remove(const K&) {
    // TODO:
    return Status::FAILED;
}

template <typename K, typename V> auto AVLTree<K, V>::split(const K&) -> std::unique_ptr<Tree> {
    // TODO:
    return nullptr;
}

template <typename K, typename V> Status AVLTree<K, V>::merge(std::unique_ptr<Tree>) {
    // TODO:
    return Status::FAILED;
}
