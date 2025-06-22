#pragma once

#include "debug.hpp"
#include "tree/interface.hpp"
#include "util.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <tuple>

namespace trait::node {

template <typename K, typename V> struct TypeTraits {
    using KeyType = K;
    using ValueType = V;
    using PairType = Pair<const K, V>;
};

template <typename Node> struct Link {
    Node* parent{nullptr};
    std::unique_ptr<Node> child[2]{nullptr, nullptr};
    Link(Node* parent = nullptr) : parent(parent) {}

    auto& lchild() { return child[L]; }
    auto& rchild() { return child[R]; }

    void bind(size_t which, std::unique_ptr<Node> node) {
        auto& self = *(static_cast<Node*>(this));
        auto& child = self.child[which];
        child = std::move(node);
        if (child) child->parent = &self;
    }
    auto unbind(size_t which) -> std::unique_ptr<Node> {
        auto& self = *(static_cast<Node*>(this));
        auto child = std::move(self.child[which]);
        if (child) child->parent = nullptr;
        return child;
    }
    auto unbind() -> std::tuple<std::unique_ptr<Node>, std::unique_ptr<Node>> {
        return {unbind(L), unbind(R)};
    }
    auto which() const -> int {
        auto& self = *(static_cast<const Node*>(this));
        if (self.parent) {
            return self.parent->child[L].get() == &self ? L : R;
        }
        return -1;
    }
};

template <typename Node> struct View {
    struct Wrapper : NodeView {
        const void* addr;
        std::string key, value;
        Wrapper(const Node* node)
            : addr(node), key(serialize(node->key)), value(serialize(node->value)) {}
        auto id() const -> const void* override { return addr; }
        auto content() const -> std::pair<std::string, std::string> override {
            return {key, value};
        }
        auto stringify() const -> std::string override {
            return serializeClass("NodeView", addr, key, value, this->child[L], this->child[R]);
        }
    };
    auto view() const -> std::unique_ptr<NodeView> {
        return std::make_unique<Wrapper>(static_cast<const Node*>(this));
    }
};

template <typename... Ts> struct Maintain : Ts... {
    void maintain() { (Ts::maintain(), ...); }
};

template <typename Node> struct Height {
    int height{1};
    int balanceFactor() {
        auto& self = *(static_cast<Node*>(this));
        auto l = self.child[L] ? self.child[L]->height : 0;
        auto r = self.child[R] ? self.child[R]->height : 0;
        return l - r;
    }
    void maintain() {
        auto& self = *(static_cast<Node*>(this));
        auto l = self.child[L] ? self.child[L]->height : 0;
        auto r = self.child[R] ? self.child[R]->height : 0;
        self.height = 1 + std::max(l, r);
    }
};

template <typename Node> struct Size {
    size_t size{1};
    void maintain() {
        auto& self = *(static_cast<Node*>(this));
        auto l = self.child[L] ? self.child[L]->size : 0;
        auto r = self.child[R] ? self.child[R]->size : 0;
        self.size = 1 + l + r;
    }
};

template <typename Node, typename Key> struct MinMax {
    Key min_key, max_key;
    void maintain() {
        auto& self = *(static_cast<Node*>(this));
        min_key = self.key, max_key = self.key;
        if (self.child[L]) {
            min_key = std::min(min_key, self.child[L]->min_key);
            max_key = std::max(max_key, self.child[L]->max_key);
        }
        if (self.child[R]) {
            min_key = std::min(min_key, self.child[R]->min_key);
            max_key = std::max(max_key, self.child[R]->max_key);
        }
    }
};

template <typename Node> struct Search {
    auto find(auto&& key) {
        auto node = static_cast<Node*>(this);
        while (node) {
            if (key == node->key) break;
            if (key < node->key)
                node = node->child[L].get();
            else
                node = node->child[R].get();
        }
        return node;
    }
    auto findKth(size_t rank) {
        auto node = static_cast<Node*>(this);
        if (rank > node->size) return static_cast<Node*>(nullptr);
        size_t lsize = node->child[L] ? node->child[L]->size : 0;
        if (rank <= lsize) return node->child[L]->findKth(rank);
        if (rank > lsize + 1) return node->child[R]->findKth(rank - lsize - 1);
        return node;
    }
    auto findMin() {
        auto node = static_cast<Node*>(this);
        while (node && node->child[L]) node = node->child[L].get();
        return node;
    }
    auto findMax() {
        auto node = static_cast<Node*>(this);
        while (node && node->child[R]) node = node->child[R].get();
        return node;
    }
};

}  // namespace trait::node
