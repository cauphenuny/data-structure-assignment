#pragma once

#include <algorithm>
#include <memory>

template <typename K, typename V> struct TypeTraits {
    using Key = K;
    using Value = V;
};

namespace trait {

template <typename Node> struct Edit {
    void bindL(this auto& self, std::unique_ptr<Node> node) {
        self.lchild = std::move(node);
        if (self.lchild) self.lchild->parent = &self;
    }
    void bindR(this auto& self, std::unique_ptr<Node> node) {
        self.rchild = std::move(node);
        if (self.rchild) self.rchild->parent = &self;
    }
    auto unbindL(this auto&& self) -> std::unique_ptr<Node> {
        auto l = std::move(self.lchild);
        if (l) l->parent = nullptr;
        self.maintain();
        return l;
    }
    auto unbindR(this auto&& self) -> std::unique_ptr<Node> {
        auto r = std::move(self.rchild);
        if (r) r->parent = nullptr;
        self.maintain();
        return r;
    }
    auto unbind(this auto&& self) -> std::tuple<std::unique_ptr<Node>, std::unique_ptr<Node>> {
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

template <typename Node> struct Search {
    auto find(this auto&& self, auto& key) {
        auto node = &self;
        while (node) {
            if (key == node->key) break;
            if (key < node->key)
                node = node->lchild.get();
            else
                node = node->rchild.get();
        }
        return node;
    }
    auto minimum(this auto&& self) {
        auto node = &self;
        while (node && node->lchild) node = node->lchild.get();
        return node.get();
    }
    auto maximum(this auto&& self) {
        auto node = &self;
        while (node && node->rchild) node = node->rchild.get();
        return node.get();
    }
};

}  // namespace trait
