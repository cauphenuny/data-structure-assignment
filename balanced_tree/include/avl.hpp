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

private:
    static void rotateR(std::unique_ptr<Node>& root);
    static void rotateL(std::unique_ptr<Node>& root);
    static void rotateLR(std::unique_ptr<Node>& root);
    static void rotateRL(std::unique_ptr<Node>& root);
    void balance(const Key& key);
    auto concat(std::unique_ptr<AVLTree> other) -> Status;
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

    void refresh() override {
        this->Node::refresh();
        this->height = 1 + std::max(
                               this->lchild ? this->avlLeft()->height : 0,
                               this->rchild ? this->avlRight()->height : 0);
    }

    AVLNode* avlLeft() const { return static_cast<AVLNode*>(this->lchild.get()); }
    AVLNode* avlRight() const { return static_cast<AVLNode*>(this->rchild.get()); }
    AVLNode* avlParent() const { return static_cast<AVLNode*>(this->parent); }
    int factor() const {
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

template <typename K, typename V> void AVLTree<K, V>::balance(const K& key) {
    auto find_imbalance =
        [&key](auto self, std::unique_ptr<Node>& node) -> std::tuple<bool, std::unique_ptr<Node>&> {
        if (!node || key == node->key) return {false, node};
        auto avl_node = static_cast<AVLNode*>(node.get());
        auto [found, rotate_node] = self(self, key < node->key ? node->lchild : node->rchild);
        if (found) return {true, rotate_node};
        if (avl_node->factor() > 1 || avl_node->factor() < -1) {
            return {true, node};
        }
        return {false, node};
    };
    auto [need_rotate, node] = find_imbalance(find_imbalance, this->root);

    if (need_rotate) {
        auto avl_node = static_cast<AVLNode*>(node.get());
        auto avl_lchild = avl_node->avlLeft(), avl_rchild = avl_node->avlRight();
        if (avl_node->factor() > 1) {
            if (avl_lchild->factor() >= 0) {
                rotateR(node);
            } else {
                rotateLR(node);
            }
        } else if (avl_node->factor() < -1) {
            if (avl_rchild->factor() <= 0) {
                rotateL(node);
            } else {
                rotateRL(node);
            }
        }
    }
}

template <typename K, typename V> Status AVLTree<K, V>::insert(const K& key, const V& value) {
    auto [parent, node] = this->container(key);
    if (node) return Status::FAILED;  // key already exists
    node = std::make_unique<AVLNode>(key, value, parent);
    Tree::refresh(parent);
    balance(key);
    return Status::SUCCESS;
}

template <typename K, typename V> Status AVLTree<K, V>::remove(const K& key) {
    auto [parent, node] = this->container(key);
    if (!node) return Status::FAILED;
    auto left_tree = std::make_unique<AVLTree>(node->lchild.release());
    auto right_tree = std::make_unique<AVLTree>(node->rchild.release());
    left_tree->concat(std::move(right_tree));
    if (left_tree->root) left_tree->root->parent = node->parent;
    node.reset(left_tree->root.release());
    Tree::refresh(parent);
    balance(key);
    return Status::FAILED;
}

template <typename K, typename V> auto AVLTree<K, V>::split(const K&) -> std::unique_ptr<Tree> {
    // TODO:
    return nullptr;
}

template <typename K, typename V> Status AVLTree<K, V>::concat(std::unique_ptr<AVLTree>) {
    return Status::FAILED;
}

template <typename K, typename V> Status AVLTree<K, V>::merge(std::unique_ptr<Tree> other) {
    if (!other) return Status::FAILED;         // tree does not exist
    if (!other->root) return Status::SUCCESS;  // nothing to merge
    auto avl_other = dynamic_cast<AVLTree<K, V>*>(other.get());
    if (!avl_other || (this->minimum()->key <= other->maximum()->key &&
                       other->minimum()->key <= this->maximum()->key)) {
        return this->mixin(std::move(other));
    }
    other.release();
    return this->concat(std::unique_ptr<AVLTree>(avl_other));
}
