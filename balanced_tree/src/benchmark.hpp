#pragma once
#include "_legacy/avl.hpp"
#include "_legacy/tree.hpp"
#include "tree/avl.hpp"
#include "tree/basic.hpp"
#include "tree/splay.hpp"
#include "tree/treap.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <format>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

using namespace std;

inline void crtpBenchmark() {
    using Key = int;
    using Value = int;
    constexpr int N = 200000;

    vector<Key> keys(N);
    for (int i = 0; i < N; ++i) keys[i] = i;
    shuffle(keys.begin(), keys.end(), mt19937{random_device{}()});

    struct Metrics {
        double insert_time;
        double find_time;
        double remove_time;
    };

    auto bench = [&](auto&& tree, string_view name) -> Metrics {
        // Insert
        auto start = chrono::high_resolution_clock::now();
        for (auto k : keys) tree->insert(k, k);
        auto end = chrono::high_resolution_clock::now();
        double insert_time = chrono::duration<double, milli>(end - start).count();

        // Find
        start = chrono::high_resolution_clock::now();
        for (auto k : keys) volatile auto _ = tree->find(k);
        end = chrono::high_resolution_clock::now();
        double find_time = chrono::duration<double, milli>(end - start).count();

        // Remove
        start = chrono::high_resolution_clock::now();
        for (auto k : keys) {
            if constexpr (requires { tree->remove(k); }) {
                tree->remove(k);
            } else {
                tree->erase(k);
            }
        }
        end = chrono::high_resolution_clock::now();
        double remove_time = chrono::duration<double, milli>(end - start).count();

        cout << format(
            "{:<20} {:>12.2f} {:>12.2f} {:>12.2f}\n", name, insert_time, find_time, remove_time);

        return {insert_time, find_time, remove_time};
    };

    cout << format("\n===== Tree Implementation Comparison =====\n");
    cout << format("{:<20} {:>12} {:>12} {:>12}\n", "Tree", "Insert", "Find", "Remove");

    struct StdMapWrapper {
        void insert(const Key& key, const Value& value) { m[key] = value; }
        void remove(const Key& key) { m.erase(key); }
        auto find(const Key& key) { return m.find(key); }
        map<Key, Value> m;
    };

    auto old_time = bench(make_unique<legacy::AVLTree<Key, Value>>(), "legacy::AVLTree(ms)");
    auto new_time = bench(make_unique<AVLTree<Key, Value>>(), "AVLTree(ms)");
    bench(make_unique<StdMapWrapper>(), "std::map(ms)");

    auto relative = [](double old_time, double new_time) {
        return (old_time - new_time) / old_time * 100.0;
    };

    cout << format(
        "\n{:<20} {:>12.2f} {:>12.2f} {:>12.2f}\n", "CRTP Improvement(%)",
        relative(old_time.insert_time, new_time.insert_time),
        relative(old_time.find_time, new_time.find_time),
        relative(old_time.remove_time, new_time.remove_time));
}

inline void algorithmBenchmark() {
    cout << format("\n===== Tree Algorithm Comparison =====\n");
    using Key = int;
    using Value = int;
    constexpr int N = 200000;
    vector<Key> keys(N);
    for (int i = 0; i < N; ++i) keys[i] = i;
    shuffle(keys.begin(), keys.end(), mt19937{random_device{}()});

    auto duration = [](auto func) {
        auto start = chrono::high_resolution_clock::now();
        func();
        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double, milli>(end - start).count();
    };
    auto micro_duration = [](auto func) {
        auto start = chrono::high_resolution_clock::now();
        func();
        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double, micro>(end - start).count();
    };

    auto bench = [&](auto tree, string_view name, bool include_sequential = true) {
        // Insert
        double insert_time = duration([&] {
            for (auto k : keys) tree->insert(k, k);
        });

        // Find
        double find_time = duration([&] {
            for (auto k : keys) volatile auto _ = tree->find(k);
        });

        // Remove
        double remove_time = duration([&] {
            for (auto k : keys) tree->remove(k);
        });

        // Re-insert for split/merge
        for (auto k : keys) tree->insert(k, k);

        // Split
        decltype(tree->split(N / 2)) split_result;
        double split_time = micro_duration([&] { split_result = tree->split(N / 2); });

        // Merge
        double merge_time = micro_duration([&] { tree->merge(std::move(split_result)); });

        if (include_sequential) {
            tree->clear();
            double seq_insert_time = duration([&] {
                for (int i = 0; i < N; i++) tree->insert(i, i);
            });
            double seq_find_time = duration([&] {
                for (int i = 0; i < N; i++) volatile auto _ = tree->find(i);
            });
            cout << format(
                "{:<10} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>16.2f} {:>16.2f}\n",
                name, insert_time, find_time, remove_time, split_time, merge_time, seq_insert_time,
                seq_find_time);
        } else {
            cout << format(
                "{:<10} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>16} {:>16}\n", name,
                insert_time, find_time, remove_time, split_time, merge_time, "N/A", "N/A");
        }
    };

    cout << format(
        "{:<10} {:>10} {:>10} {:>10} {:>10} {:>10} {:>16} {:>16}\n", "Tree", "Insert(ms)",
        "Find(ms)", "Remove(ms)", "Split(us)", "Merge(us)", "SeqInsert(ms)", "SeqFind(ms)");

    bench(make_unique<BasicTree<Key, Value>>(), "Basic", false);
    bench(make_unique<AVLTree<Key, Value>>(), "AVL");
    bench(make_unique<Treap<Key, Value>>(), "Treap");
    bench(make_unique<SplayTree<Key, Value>>(), "Splay");
}
inline void benchmark() {
    crtpBenchmark();
    algorithmBenchmark();
}
