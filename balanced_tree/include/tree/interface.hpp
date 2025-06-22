#pragma once

#include "util.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

template <typename K, typename V> struct Pair {
    K key;
    V value;
};

struct NodeView {
    NodeView* parent;
    std::unique_ptr<NodeView> child[2];
    virtual ~NodeView() = default;
    // NOTE: id(): identification of the node, unique, invariant across operations
    virtual auto id() const -> const void* = 0;
    virtual auto content() const -> std::pair<std::string, std::string> = 0;
    virtual auto stringify() const -> std::string = 0;
};

using ForestView = std::vector<std::unique_ptr<NodeView>>;

struct TreeBase {
    virtual ~TreeBase() = default;
    virtual auto size() const -> size_t = 0;
    virtual void clear() = 0;
    virtual void print() const = 0;
    virtual auto view() const -> ForestView = 0;
    virtual auto trace() -> std::vector<ForestView> = 0;  // dump trace history
    // dump trace during a function execution
    virtual auto trace(const std::function<void()>& func) -> std::vector<ForestView> = 0;
    virtual void traceStart() = 0;
    virtual void traceStop() = 0;
    virtual void printCLI() const = 0;
    virtual auto stringify() const -> std::string = 0;
    virtual auto name() const -> std::string = 0;
};

template <typename K, typename V> struct Tree : TreeBase {
    virtual auto find(const K& key) -> Pair<const K, V>* = 0;
    /// findKth(rank): find the k-th element, 1-based index
    virtual auto findKth(size_t rank) -> Pair<const K, V>* = 0;
    virtual auto min() -> Pair<const K, V>* = 0;
    virtual auto max() -> Pair<const K, V>* = 0;
    virtual auto insert(const K& key, const V& value) -> Status = 0;
    virtual auto remove(const K& key) -> Status = 0;
    /// traverse(func): traverse the tree in key-increasing order
    virtual void traverse(const std::function<void(const K&, V&)>& func) = 0;
    virtual auto operator[](const K& key) -> V& = 0;
    virtual auto operator[](const K& key) const -> const V& = 0;
};

// NOTE: example:
// ```cpp
// auto tree = AVLTree<int, std::string>::create(); // std::unique_ptr<Tree<int, std::string>>
// tree.insert(1, "one");
// tree = BasicTree<int, std::string>::create();
// tree.insert(2, "two");
// ```

// bind implementation to the interface
template <typename K, typename V, template <typename, typename> typename Impl>
struct TreeAdapter : Tree<K, V> {
    friend struct Test;
    auto size() const -> size_t override { return impl->size(); }
    void clear() override { impl->clear(); }
    void print() const override { impl->print(); }
    auto view() const -> ForestView override { return impl->view(); }
    void printCLI() const override { impl->printCLI(); }
    auto stringify() const -> std::string override { return impl->stringify(); }
    auto name() const -> std::string override { return impl->name(); }
    auto insert(const K& k, const V& v) -> Status override { return impl->insert(k, v); }
    auto remove(const K& k) -> Status override { return impl->remove(k); }
    auto find(const K& k) -> Pair<const K, V>* override { return impl->find(k); }
    auto findKth(size_t rank) -> Pair<const K, V>* override { return impl->findKth(rank); }
    auto trace() -> std::vector<ForestView> override { return impl->trace(); }
    auto trace(const std::function<void()>& func) -> std::vector<ForestView> override {
        return impl->trace(func);
    }
    void traceStart() override { impl->traceStart(); }
    void traceStop() override { impl->traceStop(); }
    auto min() -> Pair<const K, V>* override { return impl->min(); }
    auto max() -> Pair<const K, V>* override { return impl->max(); }
    auto operator[](const K& k) -> V& override { return impl->operator[](k); }
    auto operator[](const K& k) const -> const V& override { return impl->operator[](k); }

    // NOTE:
    // No unified split/join/merge interface in Tree<K, V> for different algorithm, because these
    // functions need algorithm info, you can't join a BasicTree to AVLTree.
    // Use `std::make_unique<AVLTree<K, V>>()` instead of `AVLTree<K, V>::create()` if you want to
    // call split/join/merge because the latter will erase type info to Tree<K, V>.

    // NOTE: example:
    // ```cpp
    // auto tree = AVLTree<int, std::string>::create(); // erased to Tree<K, V>
    // tree->insert(...);
    // auto splited = tree->split(10); // failed!
    // auto avl = make_unique<AVLTree<int, std::string>>(); // retains original type AVLTree
    // avl->insert(...);
    // auto splited = avl->split(10); // success.
    // avl->join(std::move(splited)); // joins the splited tree back
    // ```

    auto split(const K& k) -> std::unique_ptr<TreeAdapter> {
        return std::make_unique<TreeAdapter>(impl->split(k));
    }

    // NOTE:
    // `join`: key-range not overlap, and this's keys must lesser than other's, O(log n)
    // `merge`: allow key overlapping, O(n log n)

    auto join(std::unique_ptr<TreeAdapter> other) -> Status {
        return impl->join(std::move(other->impl));
    }
    auto merge(std::unique_ptr<TreeAdapter> other) -> Status {
        return impl->merge(std::move(other->impl));
    }
    auto conflict(TreeAdapter* other) -> bool { return impl->conflict(other->impl.get()); }

    // NOTE:
    // Using unified iterator type for different algorithms requires virtual function call,
    // which is not efficient.
    // If you must traverse a abstract tree, use `traverse(func)` instead.

    auto begin() { return impl->begin(); }
    auto end() { return impl->end(); }
    auto iteratorOf(const K& k) { return impl->iteratorOf(k); }

    void traverse(const std::function<void(const K&, V&)>& func) override {
        return impl->traverse([&](auto&& node) { func(node->key, node->value); });
    }

    static auto create() -> std::unique_ptr<Tree<K, V>> {
        return std::make_unique<TreeAdapter>(std::make_unique<Impl<K, V>>());
    }

    TreeAdapter() : impl(std::make_unique<Impl<K, V>>()) {}
    TreeAdapter(std::unique_ptr<Impl<K, V>> impl) : impl(std::move(impl)) {}

private:
    std::unique_ptr<Impl<K, V>> impl;
};
