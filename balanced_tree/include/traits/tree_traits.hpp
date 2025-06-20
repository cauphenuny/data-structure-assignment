#pragma once

#include "tree/interface.hpp"
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

template <typename Tree> struct Rotate {
    void rotate(int dir, auto& root) {
        auto& self = *static_cast<Tree*>(this);
        auto new_root = self.unbind(root, dir ^ 1);
        if (new_root->child[dir]) {
            self.bind(root, dir ^ 1, std::move(new_root->child[dir]));
        }
        new_root->parent = root->parent;
        self.bind(new_root, dir, std::move(root));
        root = std::move(new_root);
        root->child[dir]->maintain();
        root->maintain();
    }
    void rotateL(auto& root) { return rotate(L, root); }
    void rotateR(auto& root) { return rotate(R, root); }
    void rotateLR(auto& root) {
        rotateL(root->child[L]);
        rotateR(root);
    }
    void rotateRL(auto& root) {
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

template <typename Tree> struct Print {
    void printCLI() const {
        auto& self = *(static_cast<const Tree*>(this));
        if (!self.root) {
            std::cout << "Tree is empty." << '\n';
            return;
        }
        auto print_node = [&](auto self, auto node, int depth) {
            if (!node) return;
            self(self, node->child[L].get(), depth + 1);
            std::cout << std::string(depth * 4, ' ') << node->key << ": " << node->value << "\n";
            self(self, node->child[R].get(), depth + 1);
        };
        print_node(print_node, self.root.get(), 0);
    }
    void print() const {
        // auto& self = *(static_cast<const Tree*>(this));
        // TODO:
    }
};

template <typename Tree> struct View {
    auto view() const -> ForestView {
        auto& root = (static_cast<const Tree*>(this))->root;
        ForestView forest_view;
        forest_view.push_back(create(root.get()));
        return forest_view;
    }
    auto view(auto& node) const -> std::unique_ptr<NodeView> {
        auto root = node.get();
        if (!root) return nullptr;
        while (root->parent) root = root->parent;
        return create(root);
    }

private:
    auto create(auto* node) const -> std::unique_ptr<NodeView> {
        if (!node) return nullptr;
        auto view = node->view();
        view->child[L] = create(node->child[L].get());
        view->child[R] = create(node->child[R].get());
        if (view->child[L]) view->child[L]->parent = view.get();
        if (view->child[R]) view->child[R]->parent = view.get();
        return view;
    }
};

template <typename Tree> struct Record {
    void startRecording() { recording = true; }
    void stopRecording() { recording = false; }
    auto getRecord() -> std::vector<ForestView> {
        auto ret = std::move(records);
        records = std::vector<ForestView>();
        return ret;
    }
    void record(auto&... nodes) {
        if (!recording) return;
        auto& self = *static_cast<Tree*>(this);
        ForestView view;
        (view.push_back(self.view(nodes)), ...);
        ForestView filtered;
        for (auto& v : view) {
            if (v) filtered.push_back(std::move(v));
        }
        records.push_back(std::move(filtered));
    }

private:
    bool recording{false};
    std::vector<ForestView> records;
};

template <typename Tree> struct Bind {
    static void bind(auto& parent, size_t which, auto node) { parent.bind(which, std::move(node)); }
    static auto unbind(auto& parent, size_t which) { parent.unbind(which); }
    static auto unbind(auto& parent) { return std::make_tuple(parent.unbind(L), parent.unbind(R)); }
};

template <typename Tree> struct BindRecord {
    void bind(auto& parent, size_t which, auto node) {
        auto& self = *(static_cast<Tree*>(this));
        if (!node) {
            parent->bind(which, std::move(node));
        } else {
            self.record(parent, node);
            parent->bind(which, std::move(node));
            self.record(parent);
        }
    }
    auto unbind(auto& parent, size_t which) {
        auto& self = *(static_cast<Tree*>(this));
        auto node = parent->unbind(which);
        self.record(parent, node);
        return node;
    }
    auto unbind(auto& parent) {
        auto& self = *(static_cast<Tree*>(this));
        auto [lchild, rchild] = parent->unbind();
        self.record(parent, lchild, rchild);
        return std::make_tuple(std::move(lchild), std::move(rchild));
    }
};

}  // namespace trait
