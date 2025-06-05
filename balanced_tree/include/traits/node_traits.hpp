#pragma once

#include "tree/interface.hpp"

#include <algorithm>
#include <memory>

namespace trait {

template <typename K, typename V> struct TypeTraits {
    using KeyType = K;
    using ValueType = V;
    using PairType = Pair<const K, V>;
    enum Dir { L, R };
};

template <typename Node> struct Link {
    Node* parent{nullptr};
    std::unique_ptr<Node> lchild{nullptr}, rchild{nullptr};
    Link(Node* parent = nullptr) : parent(parent) {}

    void bind(int direction, std::unique_ptr<Node> node) {
        auto& self = *(static_cast<Node*>(this));
        auto& child = (direction == Node::R) ? self.rchild : self.lchild;
        child = std::move(node);
        if (child) child->parent = &self;
    }
    auto unbind(int direction) -> std::unique_ptr<Node> {
        auto& self = *(static_cast<Node*>(this));
        auto child = std::move(direction == Node::R ? self.rchild : self.lchild);
        if (child) child->parent = nullptr;
        self.maintain();
        return child;
    }
    auto unbind() -> std::tuple<std::unique_ptr<Node>, std::unique_ptr<Node>> {
        auto& self = *(static_cast<Node*>(this));
        auto l = std::move(self.lchild);
        auto r = std::move(self.rchild);
        if (l) l->parent = nullptr;
        if (r) r->parent = nullptr;
        self.maintain();
        return {std::move(l), std::move(r)};
    }
};

template <typename... Ts> struct Maintain : Ts... {
    void maintain() { (Ts::_maintain(), ...); }
};

template <typename Node> struct Height {
    int height{1};
    int balanceFactor() {
        auto& self = *(static_cast<Node*>(this));
        auto l = self.lchild ? self.lchild->height : 0;
        auto r = self.rchild ? self.rchild->height : 0;
        return l - r;
    }
    void _maintain() {
        auto& self = *(static_cast<Node*>(this));
        auto l = self.lchild ? self.lchild->height : 0;
        auto r = self.rchild ? self.rchild->height : 0;
        self.height = 1 + std::max(l, r);
    }
};

template <typename Node> struct Size {
    int size{1};
    void _maintain() {
        auto& self = *(static_cast<Node*>(this));
        auto l = self.lchild ? self.lchild->size : 0;
        auto r = self.rchild ? self.rchild->size : 0;
        self.size = 1 + l + r;
    }
};

template <typename Node, typename Key> struct MinMax {
    Key min_key, max_key;
    void _maintain() {
        auto& self = *(static_cast<Node*>(this));
        min_key = self.key, max_key = self.key;
        if (self.lchild) {
            min_key = std::min(min_key, self.lchild->min_key);
            max_key = std::max(max_key, self.lchild->max_key);
        }
        if (self.rchild) {
            min_key = std::min(min_key, self.rchild->min_key);
            max_key = std::max(max_key, self.rchild->max_key);
        }
    }
};

template <typename Node> struct Search {
    auto find(auto& key) {
        auto node = static_cast<Node*>(this);
        while (node) {
            if (key == node->key) break;
            if (key < node->key)
                node = node->lchild.get();
            else
                node = node->rchild.get();
        }
        return static_cast<Node::PairType*>(node);
    }
    auto findMin() {
        auto node = static_cast<Node*>(this);
        while (node && node->lchild) node = node->lchild.get();
        return static_cast<Node::PairType*>(node);
    }
    auto findMax() {
        auto node = static_cast<Node*>(this);
        while (node && node->rchild) node = node->rchild.get();
        return static_cast<Node::PairType*>(node);
    }
};

}  // namespace trait
