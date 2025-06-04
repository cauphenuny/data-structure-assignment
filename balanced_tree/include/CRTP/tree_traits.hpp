#pragma once

#include <algorithm>
#include <memory>
#include <tuple>

namespace tree_trait {

template <typename Node> struct Maintain {
    static void maintain(Node* node) {
        while (node) {
            node->maintain();
            node = node->parent;
        }
    }
};

template <typename Tree> struct Detach {
    template <typename Node> auto detach(std::unique_ptr<Node>& node) -> std::unique_ptr<Node> {
        if (node->lchild && node->rchild) return nullptr;
        auto parent = node->parent;
        if (node->lchild) node->lchild->parent = parent;
        if (node->rchild) node->rchild->parent = parent;
        auto raw = node.release();
        if (!raw->lchild)
            node = std::move(raw->rchild);
        else
            node = std::move(raw->lchild);
        Tree::maintain(parent);
    }
};

template <typename Node> struct Rotate {
    static void rotateR(std::unique_ptr<Node>& root) {
        auto new_root = root->lchild.release();
        if (new_root->rchild) {
            root->bindL(std::move(new_root->rchild));
        }
        new_root->parent = root->parent;
        new_root->bindR(std::move(root));
        root.reset(new_root);
        root->rchild->maintain();
        root->maintain();
    }
    static void rotateL(std::unique_ptr<Node>& root) {
        auto new_root = root->rchild.release();
        if (new_root->lchild) {
            root->bindR(std::move(new_root->lchild));
        }
        new_root->parent = root->parent;
        new_root->bindL(std::move(root));
        root.reset(new_root);
        root->lchild->maintain();
        root->maintain();
    }
    static void rotateLR(std::unique_ptr<Node>& root) {
        rotateL(root->lchild);
        rotateR(root);
    }
    static void rotateRL(std::unique_ptr<Node>& root) {
        rotateR(root->rchild);
        rotateL(root);
    }
};

template <typename Tree> struct Search {
    auto find(auto& key) {
        auto& self = *(static_cast<Tree*>(this));
        return self.root ? self.root->find(key) : nullptr;
    }
    auto minimum() {
        auto& self = *(static_cast<Tree*>(this));
        return self.root ? self.root->minimum() : nullptr;
    }
    auto maximum() {
        auto& self = *(static_cast<Tree*>(this));
        return self.root ? self.root->maximum() : nullptr;
    }
};

template <typename Tree> struct Box {
    auto findBox(auto& node, auto& key) {
        auto find = [&key](
                        auto& self, auto parent,
                        auto& node) -> std::tuple<decltype(parent), decltype(node)> {
            if (!node || key == node->key) return {parent, node};
            if (key < node->key) {
                return self(self, node.get(), node->lchild);
            } else {
                return self(self, node.get(), node->rchild);
            }
        };
        return find(find, node ? node->parent : nullptr, node);
    }
    auto minBox(auto& node) {
        auto find = [](auto& self, auto& node) -> decltype(node) {
            if (!node || !node->lchild) return node;
            return self(self, node->lchild);
        };
        return find(find, node);
    }
    auto maxBox(auto& node) {
        auto find = [](auto& self, auto& node) -> decltype(node) {
            if (!node || !node->rchild) return node;
            return self(node->rchild);
        };
        return find(find, node);
    }
    auto& box(auto node_ptr) {
        auto& self = *(static_cast<Tree*>(this));
        if (!node_ptr->parent) return self.root;
        if (node_ptr->parent->lchild.get() == node_ptr) return node_ptr->parent->lchild;
        return node_ptr->parent->rchild;
    }
};
}  // namespace tree_trait
