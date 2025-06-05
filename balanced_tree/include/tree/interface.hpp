#pragma once

#include "traits/node_traits.hpp"
#include "util.hpp"

#include <string>

template <typename K, typename V> struct Pair {
    K key;
    V value;
};

struct TreeBase {
    virtual ~TreeBase() = default;
    virtual auto size() const -> size_t = 0;
    virtual void clear() = 0;
    virtual void print() const = 0;
    virtual void printCLI() const = 0;
    virtual auto stringify() const -> std::string = 0;
};

template <typename K, typename V> struct Tree : TreeBase {
    virtual auto find(const K& key) -> Pair<const K, V>* = 0;
    virtual auto min() -> Pair<const K, V>* = 0;
    virtual auto max() -> Pair<const K, V>* = 0;
    virtual auto insert(const K& key, const V& value) -> Status = 0;
    virtual auto remove(const K& key) -> Status = 0;
    virtual auto split(const K& key) -> std::unique_ptr<Tree> = 0;
    virtual auto join(std::unique_ptr<Tree> other) -> Status = 0;
    virtual auto merge(std::unique_ptr<Tree> other) -> Status = 0;
    virtual auto operator[](const K& key) -> V& = 0;
    virtual auto operator[](const K& key) const -> const V& = 0;
};

template <typename K, typename V, typename Impl> struct TreeImpl : Tree<K, V> {
    friend struct Test;
    auto size() const -> size_t override { return impl->size(); }
    void clear() override { impl->clear(); }
    void print() const override { impl->print(); }
    void printCLI() const override { impl->printCLI(); }
    auto stringify() const -> std::string override { return impl->stringify(); }
    auto insert(const K& k, const V& v) -> Status override { return impl->insert(k, v); }
    auto remove(const K& k) -> Status override { return impl->remove(k); }
    auto find(const K& k) -> Pair<const K, V>* override { return impl->find(k); }
    auto min() -> Pair<const K, V>* override { return impl->min(); }
    auto max() -> Pair<const K, V>* override { return impl->max(); }
    auto operator[](const K& k) -> V& override { return impl->operator[](k); }
    auto operator[](const K& k) const -> const V& override { return impl->operator[](k); }
    auto split(const K& k) -> std::unique_ptr<Tree<K, V>> override {
        auto split_tree = impl->split(k);
        return std::make_unique<TreeImpl>(std::move(split_tree));
    }
    auto merge(std::unique_ptr<Tree<K, V>> other) -> Status override {
        if (auto tree_impl = dynamic_cast<TreeImpl<K, V, Impl>*>(other.get())) {
            return impl->merge(std::move(tree_impl->impl));
        }
        return Status::FAILED;  // cannot merge with non-compatible tree
    }
    auto join(std::unique_ptr<Tree<K, V>> other) -> Status override {
        if (auto tree_impl = dynamic_cast<TreeImpl<K, V, Impl>*>(other.get())) {
            return impl->join(std::move(tree_impl->impl));
        }
        return Status::FAILED;  // cannot join with non-compatible tree
    }
    static auto create() -> std::unique_ptr<Tree<K, V>> {
        return std::make_unique<TreeImpl>(std::make_unique<Impl>());
    }

    TreeImpl() : impl(std::make_unique<Impl>()) {}
    TreeImpl(std::unique_ptr<Impl> impl) : impl(std::move(impl)) {}

private:
    std::unique_ptr<Impl> impl;
};

// example:
// auto tree = AVLTree<int, std::string>::create(); // std::unique_ptr<Tree<int, std::string>>
// tree.insert(1, "one");
// tree = BasicTree<int, std::string>::create();
// tree.insert(2, "two");
