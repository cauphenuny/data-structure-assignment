#pragma once

#include "tree/interface.hpp"
#include "util.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace trait {

/// @struct TypeTraits
/// @brief type info for metaprogramming
template <typename N> struct TypeTraits {
    using KeyType = typename N::KeyType;
    using ValueType = typename N::ValueType;
    using PairType = typename N::PairType;
    using NodeType = N;
};

/// @struct Mixin
/// @brief simply mixin multiple traits into a single type, Mixin<T, A, B> serves as A<T>, B<T>
template <typename Type, template <typename> class... Traits> struct Mixin : Traits<Type>... {};

/// @struct Maintain
/// @brief provide a maintain() method which maintain info upwards starts from {node}
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
        auto parent = root->parent;
        self.bind(new_root, dir, std::move(root));
        self.moveNode(root, std::move(new_root), parent);
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
    auto findKth(size_t rank) {
        auto&& root = static_cast<const Tree*>(this)->root;
        return root ? root->findKth(rank) : nullptr;
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

template <typename Tree> struct Iterate {
    auto begin() { return wrap(static_cast<Tree*>(this)->min()); }
    auto end() {
        using NodeType = typename Tree::NodeType;
        return wrap(static_cast<NodeType*>(nullptr));
    }
    auto iteratorOf(auto&& key) { return wrap(static_cast<Tree*>(this)->find(key)); }

private:
    auto wrap(auto* node) {
        using PairType = typename Tree::PairType;
        using NodeType = typename Tree::NodeType;
        struct Iterator {
            PairType* content;

            Iterator(NodeType* node) : content(node) {}
            PairType* operator->() { return content; }
            const PairType* operator->() const { return content; }
            PairType& operator*() { return *content; }
            PairType const& operator*() const { return *content; }

            auto next() -> Iterator {
                auto node = static_cast<NodeType*>(content);
                if (node->child[R]) return node->child[R]->findMin();
                while (node->parent && node->which() == R) node = node->parent;
                return Iterator(node->parent);
            }
            auto prev() -> Iterator {
                auto node = static_cast<NodeType*>(content);
                if (node->child[L]) return node->child[L]->findMax();
                while (node->parent && node->which() == L) node = node->parent;
                return Iterator(node->parent);
            }

            Iterator& operator++() {
                content = next().content;
                return *this;
            }
            Iterator& operator--() {
                content = prev().content;
                return *this;
            }
            bool operator==(const Iterator& other) const { return content == other.content; }
            bool operator!=(const Iterator& other) const { return content != other.content; }
            operator bool() const { return content != nullptr; }
        };
        return Iterator(node);
    }
};

/// @struct Detach
/// @brief detach a leaf node or semi-leaf node from the tree
template <typename Tree> struct Detach {
    template <typename Node> auto detach(std::unique_ptr<Node>& node) -> std::unique_ptr<Node> {
        auto& self = *(static_cast<Tree*>(this));
        assert(!(node->child[L] && node->child[R]));
        if (!node->child[L] && !node->child[R]) {
            auto parent = node->parent;
            node->parent = nullptr;
            auto detached = std::move(node);
            self.tracedTrack(detached);
            Tree::maintain(parent);
            return detached;
        } else {
            auto child = std::move(node->child[L] ? node->child[L] : node->child[R]);
            child->parent = node->parent;
            node->parent = nullptr;
            auto detached = std::move(node);
            self.tracedTrack(detached);
            node = std::move(child);
            self.snapshot();
            Tree::maintain(node->parent);
            return detached;
        }
    }
};

/// @struct Box
/// @brief get the std::unique_ptr container of specific node
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

/// @struct Traverse
/// @brief provide tree traversal methods
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

/// @struct Conflict
/// @brief check if two trees have conflict keys
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

/// @struct Merge
/// @brief merge two trees, auto call join/inject
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
            self(self, node->child[R].get(), depth + 1);
            std::cout << std::string(depth * 4, ' ') << node->key << ": " << node->value << "\n";
            self(self, node->child[L].get(), depth + 1);
        };
        print_node(print_node, self.root.get(), 0);
    }
    void print() const {
        // auto& self = *(static_cast<const Tree*>(this));
        // TODO:
    }
};

/// @struct View
/// @brief create a view of root or specific node
template <typename Tree> struct View {
    auto view() const -> ForestView {
        auto& root = (static_cast<const Tree*>(this))->root;
        ForestView forest_view;
        forest_view.push_back(create(root.get()));
        return forest_view;
    }

    /// @func view
    /// @brief create a view of the connected component of {node}
    auto view(auto* node) const -> std::unique_ptr<NodeView> {
        auto root = node;
        if (!root) return nullptr;
        while (root->parent) root = root->parent;
        return create(root);
    }

private:
    auto create(auto* node) const -> std::unique_ptr<NodeView> {
        if (!node) return nullptr;
        auto view = node->view();
        view->parent = nullptr;
        view->child[L] = create(node->child[L].get());
        view->child[R] = create(node->child[R].get());
        if (view->child[L]) view->child[L]->parent = view.get();
        if (view->child[R]) view->child[R]->parent = view.get();
        return view;
    }
};

/// @struct Trace
/// @brief functions that control tracing
template <typename Tree> struct Trace {
    void traceClear() {
        entries.clear();
        record.clear();
    }
    void traceStart() {
        auto& self = *static_cast<Tree*>(this);
        tracing = true;
        tracedTrack(self.root);
    }
    void traceStop() {
        tracing = false;
        entries.clear();
    }
    auto trace() -> std::vector<ForestView> {
        auto ret = std::move(record);
        record = std::vector<ForestView>();
        return ret;
    }
    auto trace(auto&& func, auto&&... args) -> std::vector<ForestView> {
        traceClear();
        traceStart();
        func(std::forward<decltype(args)>(args)...);
        traceStop();
        return trace();
    }

    /// @func tracedTrack
    /// @brief start tracking {nodes}, and snapshot the current state
    void tracedTrack(auto&&... nodes) {
        if (track(nodes...)) snapshot();
    }

    /// @func tracedUntrack
    /// @brief stop tracking {nodes}, and snapshot the current state
    void tracedUntrack(auto&&... nodes) {
        if (untrack(nodes...)) snapshot();
    }

    /// @func snapshot
    /// @brief create a snapshot by entries
    void snapshot() {
        if (!tracing) return;
        auto& self = *static_cast<Tree*>(this);
        using T = typename Tree::NodeType;
        ForestView view;
        for (auto entry : entries) {
            view.push_back(self.view(static_cast<T*>(entry)));
        }
        record.push_back(std::move(view));
    }

    /// @func track
    /// @brief start tracking {nodes} by adding roots to entries
    bool track(auto&&... nodes) {
        if (!tracing) return false;
        bool success = false;
        auto add = [&](auto&& node) {
            if (!node) return;
            success = true;
            auto ptr = node.get();
            while (ptr->parent) ptr = ptr->parent;
            entries.insert(ptr);
        };
        (add(nodes), ...);
        return success;
    }

    /// @func untrack
    /// @brief stop tracking {nodes} by removing roots from entries
    bool untrack(auto&&... nodes) {
        if (!tracing) return false;
        bool success = false;
        auto remove = [&](auto&& node) {
            if (!node) return;
            success = true;
            auto ptr = node.get();
            while (ptr->parent) ptr = ptr->parent;
            entries.erase(ptr);
        };
        (remove(nodes), ...);
        return success;
    }

private:
    bool tracing{false};
    std::vector<ForestView> record;
    std::set<void*> entries;
};

template <typename Tree> struct Bind {
    static void bind(auto& parent, size_t which, auto node) { parent.bind(which, std::move(node)); }
    static auto unbind(auto& parent, size_t which) { parent.unbind(which); }
    static auto unbind(auto& parent) { return std::make_tuple(parent.unbind(L), parent.unbind(R)); }
};

template <typename Tree> struct TracedConstruct {
    /// @func constructNode
    /// @brief construct a node with {args} at {node}
    void constructNode(auto& node, auto&&... args) {
        auto& self = *(static_cast<Tree*>(this));
        using Node = typename Tree::NodeType;
        node = std::make_unique<Node>(std::forward<decltype(args)>(args)...);
        self.tracedTrack(node);
    }

    /// @func overwriteNode
    /// @brief move node from {src} to {dest}, where the info in dest is valid, like
    /// operator=(auto&&)
    auto overwriteNode(auto& dest, auto src) {
        auto& self = *(static_cast<Tree*>(this));
        auto parent = dest ? dest->parent : nullptr;
        self.untrack(src), self.untrack(dest);
        dest = std::move(src);
        dest->parent = parent;
        self.tracedTrack(dest);
    }

    /// @func moveNode
    /// @brief move node from {src} to {dest}, where dest is invalid before, like
    /// constructor(auto&&)
    auto moveNode(auto& dest, auto src, auto* parent) {
        auto& self = *(static_cast<Tree*>(this));
        self.untrack(src);
        dest = std::move(src);
        dest->parent = parent;
        self.tracedTrack(dest);
    }
};

/// @struct TracedBind
/// @brief bind/unbind nodes with tracing
template <typename Tree> struct TracedBind {
    void bind(auto& parent, size_t which, auto node) {
        auto& self = *(static_cast<Tree*>(this));
        if (!node) {
            parent->bind(which, std::move(node));
        } else {
            node->parent = nullptr;
            self.untrack(node);
            parent->bind(which, std::move(node));
            self.snapshot();
        }
    }
    auto unbind(auto& parent, size_t which) {
        auto& self = *(static_cast<Tree*>(this));
        auto node = parent->unbind(which);
        self.tracedTrack(node);
        return node;
    }
    auto unbind(auto& parent) {
        auto& self = *(static_cast<Tree*>(this));
        auto [lchild, rchild] = parent->unbind();
        self.tracedTrack(lchild, rchild);
        return std::make_tuple(std::move(lchild), std::move(rchild));
    }
};

}  // namespace trait
