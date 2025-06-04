#pragma once

#include <algorithm>
#include <memory>

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
    auto find(this auto&& self, auto& key) { return self.root ? self.root->find(key) : nullptr; }
    auto minimum(this auto&& self) { return self.root ? self.root->minimum() : nullptr; }
    auto maximum(this auto&& self) { return self.root ? self.root->maximum() : nullptr; }
};

template <typename Tree> struct Box {
    auto findBox(this auto& self, auto& key) {
        auto find = [&key](
                        this auto& self, auto parent,
                        auto& node) -> std::tuple<decltype(parent), decltype(node)> {
            if (!node || key == node->key) return {parent, node};
            if (key < node->key) {
                return self(node.get(), node->lchild);
            } else {
                return self(node.get(), node->rchild);
            }
        };
        return find(self.root ? self.root->parent : nullptr, self.root);
    }
    auto minBox(this auto& self) {
        auto find = [](this auto& self, auto& node) -> decltype(node) {
            if (!node || !node->lchild) return node;
            return self(node->lchild);
        };
        return find(self.root);
    }
    auto maxBox(this auto& self) {
        auto find = [](this auto& self, auto& node) -> decltype(node) {
            if (!node || !node->rchild) return node;
            return self(node->rchild);
        };
        return find(self.root);
    }
    auto& box(this auto& self, auto node_ptr) {
        if (!node_ptr->parent) return self.root;
        if (node_ptr->parent->lchild.get() == node_ptr) return node_ptr->parent->lchild;
        return node_ptr->parent->rchild;
    }
};
}  // namespace tree_trait
