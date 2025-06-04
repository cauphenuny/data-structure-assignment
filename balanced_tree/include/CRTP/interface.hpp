#pragma once

#include "node_traits.hpp"
#include "tree.hpp"
#include "util.hpp"

namespace crtp {

template <typename K, typename V> struct Tree : TreeBase {
    using PairType = TypeTraits<K, V>::PairType;
    virtual auto find(const K& key) -> PairType* = 0;
    virtual auto min() -> PairType* = 0;
    virtual auto max() -> PairType* = 0;
    virtual auto insert(const K& key, const V& value) -> Status = 0;
    virtual auto remove(const K& key) -> Status = 0;
    virtual auto split(const K& key) -> std::unique_ptr<Tree> = 0;
    virtual auto join(std::unique_ptr<Tree> other) -> Status = 0;
};

}  // namespace crtp
