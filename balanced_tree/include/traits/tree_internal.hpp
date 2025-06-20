#pragma once

#include "util.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace trait {

template <typename N> struct TypeTraits {
    using KeyType = typename N::KeyType;
    using ValueType = typename N::ValueType;
    using PairType = typename N::PairType;
    using NodeType = N;
};

template <typename Type, template <typename> class... Traits> struct Mixin : Traits<Type>... {};

template <typename Node> struct Maintain {
    static void maintain(Node* node) {
        while (node) {
            node->maintain();
            node = node->parent;
        }
    }
};

template <typename Node> struct Rotate {
    static void rotate(int dir, std::unique_ptr<Node>& root) {
        auto new_root = root->unbind(dir ^ 1);
        if (new_root->child[dir]) {
            root->bind(dir ^ 1, std::move(new_root->child[dir]));
        }
        new_root->parent = root->parent;
        new_root->bind(dir, std::move(root));
        root = std::move(new_root);
        root->child[dir]->maintain();
        root->maintain();
    }
    static void rotateL(std::unique_ptr<Node>& root) { return rotate(L, root); }
    static void rotateR(std::unique_ptr<Node>& root) { return rotate(R, root); }
    static void rotateLR(std::unique_ptr<Node>& root) {
        rotateL(root->child[L]);
        rotateR(root);
    }
    static void rotateRL(std::unique_ptr<Node>& root) {
        rotateR(root->child[R]);
        rotateL(root);
    }
};

template <typename Tree> struct Search {
    auto find(auto&& key) {
        auto&& root = static_cast<const Tree*>(this)->root;
        return root ? root->find(key) : nullptr;
    }
    auto min() {
        auto&& root = static_cast<Tree*>(this)->root;
        return root ? root->findMin() : nullptr;
    }
    auto max() {
        auto&& root = static_cast<Tree*>(this)->root;
        return root ? root->findMax() : nullptr;
    }
};

template <typename Tree> struct Detach {
    template <typename Node> auto detach(std::unique_ptr<Node>& node) -> std::unique_ptr<Node> {
        if (node->child[L] && node->child[R]) return nullptr;
        auto parent = node->parent;
        if (node->child[L]) node->child[L]->parent = parent;
        if (node->child[R]) node->child[R]->parent = parent;
        auto raw = node.release();
        raw->parent = nullptr;
        node = std::move(raw->child[raw->child[L] ? L : R]);
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
                return self(self, node.get(), node->child[L]);
            } else {
                return self(self, node.get(), node->child[R]);
            }
        };
        return find(find, node ? node->parent : nullptr, node);
    }
    auto& minBox(auto& node) {
        auto find = [](auto& self, auto& node) -> decltype(node) {
            if (!node || !node->child[L]) return node;
            return self(self, node->child[L]);
        };
        return find(find, node);
    }
    auto& maxBox(auto& node) {
        auto find = [](auto& self, auto& node) -> decltype(node) {
            if (!node || !node->child[R]) return node;
            return self(self, node->child[R]);
        };
        return find(find, node);
    }
    auto& box(auto node_ptr) {
        auto& self = *(static_cast<Tree*>(this));
        assert(node_ptr);
        if (node_ptr == self.root.get()) return self.root;
        assert(node_ptr->parent);
        if (node_ptr->parent->child[L].get() == node_ptr) return node_ptr->parent->child[L];
        assert(node_ptr->parent->child[R].get() == node_ptr);
        return node_ptr->parent->child[R];
    }
};

template <typename Tree> struct Clear {
    void clear() {
        auto& root = static_cast<Tree*>(this)->root;
        root.reset();
    }
};

template <typename Tree> struct Size {
    auto size() const {
        auto& root = static_cast<const Tree*>(this)->root;
        return root ? root->size : 0;
    }
};

template <typename Tree> struct Height {
    auto height() const {
        auto& root = static_cast<const Tree*>(this)->root;
        return root ? root->height : 0;
    }
};

template <typename Tree> struct Traverse {
    void traverse(auto&& func) {
        auto& root = static_cast<Tree*>(this)->root;
        voidTraverse(root, func);
    }
    auto traverse(auto&& func, auto&& reduction) {
        auto& root = static_cast<Tree*>(this)->root;
        return typedTraverse(root, func, reduction);
    }

private:
    void voidTraverse(auto& node, auto&& func) {
        if (!node) return;
        voidTraverse(node->child[L], func);
        func(node);
        voidTraverse(node->child[R], func);
    }
    auto typedTraverse(auto& node, auto&& func, auto&& reduction) {
        if (!node) return;
        auto ret = typedTraverse(node->child[L], func);
        ret = reduction(ret, func(node));
        ret = reduction(ret, typedTraverse(node->child[R], func));
        return ret;
    }
};

template <typename Tree> struct Conflict {
    bool conflict(Tree* other) {
        std::vector<typename Tree::NodeType*> vec1, vec2;
        auto& self = *(static_cast<Tree*>(this));
        self.traverse([&](auto& node) { vec1.push_back(node.get()); });
        other->traverse([&](auto& node) { vec2.push_back(node.get()); });
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
        this_min = self.min()->key, this_max = self.max()->key;
        other_min = other->min()->key, other_max = other->max()->key;
        if (this_min <= other_max && other_min <= this_max) {
            return self.inject(std::move(other));
        } else {
            if (this_min > other_max) std::swap(self.root, other->root);
            return self.join(std::move(other));
        }
    }

private:
    Status inject(std::unique_ptr<Tree> other) {
        auto& self = *(static_cast<Tree*>(this));
        other->traverse([&self](auto& node) { self.insert(node->key, node->value); });
        return Status::SUCCESS;
    }
};

template <typename Tree> struct Subscript {
    const auto& operator[](auto&& key) const {
        auto node = static_cast<const Tree*>(this)->find(key);
        if (!node) throw std::out_of_range("Key not found in the tree.");
        return node->value;
    }
    auto& operator[](auto&& key) {
        auto& self = *(static_cast<Tree*>(this));
        auto pair = self.find(key);
        if (pair) return pair->value;
        self.insert(key, typename Tree::ValueType{});
        return self.find(key)->value;
    }
};

}  // namespace trait
