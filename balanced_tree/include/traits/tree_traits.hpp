#pragma once

#include "util.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <tuple>

namespace tree_trait {

template <typename N> struct TypeTraits {
    using KeyType = typename N::KeyType;
    using ValueType = typename N::ValueType;
    using PairType = typename N::PairType;
    using Dir = typename N::Dir;
    using NodeType = N;
};

template <typename Node> struct Maintain {
    static void maintain(Node* node) {
        while (node) {
            node->maintain();
            node = node->parent;
        }
    }
};

template <typename Node> struct Rotate {
    static void rotateR(std::unique_ptr<Node>& root) {
        auto new_root = root->lchild.release();
        if (new_root->rchild) {
            root->bind(Node::L, std::move(new_root->rchild));
        }
        new_root->parent = root->parent;
        new_root->bind(Node::R, std::move(root));
        root.reset(new_root);
        root->rchild->maintain();
        root->maintain();
    }
    static void rotateL(std::unique_ptr<Node>& root) {
        auto new_root = root->rchild.release();
        if (new_root->lchild) {
            root->bind(Node::R, std::move(new_root->lchild));
        }
        new_root->parent = root->parent;
        new_root->bind(Node::L, std::move(root));
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
    auto find(auto&& key) {
        auto&& self = *(static_cast<const Tree*>(this));
        return self.root ? self.root->find(key) : nullptr;
    }
    auto min() {
        auto&& self = *(static_cast<Tree*>(this));
        return self.root ? self.root->findMin() : nullptr;
    }
    auto max() {
        auto&& self = *(static_cast<Tree*>(this));
        return self.root ? self.root->findMax() : nullptr;
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
        return std::unique_ptr<Node>(raw);
    }
};

template <typename Tree> struct Box {
    auto findBox(auto&& node, auto&& key) {
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
    auto& minBox(auto& node) {
        auto find = [](auto& self, auto& node) -> decltype(node) {
            if (!node || !node->lchild) return node;
            return self(self, node->lchild);
        };
        return find(find, node);
    }
    auto& maxBox(auto& node) {
        auto find = [](auto& self, auto& node) -> decltype(node) {
            if (!node || !node->rchild) return node;
            return self(self, node->rchild);
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

template <typename Tree> struct Clear {
    void clear() {
        auto& self = *(static_cast<Tree*>(this));
        self.root.reset();
    }
};

template <typename Tree> struct Size {
    auto size() const {
        auto& self = *(static_cast<const Tree*>(this));
        return self.root ? self.root->size : 0;
    }
};

template <typename Tree> struct Height {
    auto height() const {
        auto& self = *(static_cast<const Tree*>(this));
        return self.root ? self.root->height : 0;
    }
};

template <typename Tree> struct Traverse {
    void traverse(auto&& func) {
        auto& self = *(static_cast<Tree*>(this));
        void_traverse(self.root, func);
    }
    auto traverse(auto&& func, auto&& reduction) {
        auto& self = *(static_cast<Tree*>(this));
        return typed_traverse(self.root, func, reduction);
    }

private:
    void void_traverse(auto& node, auto&& func) {
        if (!node) return;
        void_traverse(node->lchild, func);
        func(node);
        void_traverse(node->rchild, func);
    }
    auto typed_traverse(auto& node, auto&& func, auto&& reduction) {
        if (!node) return;
        auto ret = typed_traverse(node->lchild, func);
        ret = reduction(ret, func(node));
        ret = reduction(ret, typed_traverse(node->rchild, func));
        return ret;
    }
};

template <typename Tree> struct Conflict {
    bool conflict(Tree* other) {
        std::vector<typename Tree::NodeType*> vec1, vec2;
        auto& self = *(static_cast<Tree*>(this));
        self.traverse([&](auto& node) { vec1.push_back(node); });
        other->traverse([&](auto& node) { vec2.push_back(node); });
        for (auto it1 = vec1.begin(), it2 = vec2.begin(); it1 != vec1.end() && it2 != vec2.end();) {
            if ((*it1)->key < (*it2)->key) {
                ++it1;
            } else if ((*it1)->key > (*it2)->key) {
                ++it2;
            } else {
                return true;
            }
        }
        return false;
    }
};

template <typename Tree> struct Merge {
    Status merge(std::unique_ptr<Tree> other) {
        if (!other) return Status::FAILED;
        if (!other->root) return Status::SUCCESS;
        auto& self = *(static_cast<Tree*>(this));
        if (!self.root) {
            self.root = std::move(other->root);
            return Status::SUCCESS;
        }
        typename Tree::KeyType this_min, this_max, other_min, other_max;
        if constexpr (requires { self.root->min_key, self.root->max_key; }) {
            this_min = self.root->min_key;
            this_max = self.root->max_key;
            other_min = other->root->min_key;
            other_max = other->root->max_key;
        } else {
            this_min = self.min()->key;
            this_max = self.max()->key;
            other_min = other->min()->key;
            other_max = other->max()->key;
        }
        if (this_min <= other_max && other_min <= this_max) {
            return self.mixin(std::move(other));
        } else {
            if (this_min > other_max) std::swap(self.root, other->root);
            return self.join(std::move(other));
        }
    }

private:
    Status mixin(std::unique_ptr<Tree> other) {
        auto& self = *(static_cast<Tree*>(this));
        other->traverse([&self](auto& node) { self.insert(node->key, node->value); });
        return Status::SUCCESS;
    }
};

template <typename Tree> struct Subscript {
    const auto& operator[](auto&& key) const {
        auto& self = *(static_cast<const Tree*>(this));
        auto node = self.find(key);
        if (!node) throw std::out_of_range("Key not found in the tree.");
        return node->value;
    }
    auto& operator[](auto&& key) {
        auto& self = *(static_cast<Tree*>(this));
        auto [parent, node] = self.findBox(self.root, key);
        if (!node) self.insert(key, typename Tree::ValueType{});
        return node->value;
    }
};

template <typename Tree> struct Print {
    void printCLI() const {
        auto& self = *(static_cast<const Tree*>(this));
        if (!self.root) {
            std::cout << "Tree is empty." << std::endl;
            return;
        }
        auto print_node = [&](auto self, auto node, int depth) {
            if (!node) return;
            self(self, node->lchild.get(), depth + 1);
            std::cout << std::string(depth * 4, ' ') << node->key << ": " << node->value << "\n";
            self(self, node->rchild.get(), depth + 1);
        };
        print_node(print_node, self.root.get(), 0);
    }
    void print() const {
        // auto& self = *(static_cast<const Tree*>(this));
        // TODO:
    }
};

}  // namespace tree_trait
