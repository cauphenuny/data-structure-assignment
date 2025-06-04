#pragma once
#include "avl.hpp"
#include "crtp_avl.hpp"
#include "tree.hpp"

#include <cassert>
#include <memory>
#include <random>

namespace crtp {

inline void benchmark() {
    auto old_tree = std::make_unique<::AVLTree<int, int>>();
    auto new_tree = std::make_unique<crtp::AVLTree<int, int>>();
    std::map<int, int> std_map;

    constexpr int N = 500000;

    std::cout << std::format("\n===== Tree Implementation Comparison =====\n");
    std::cout << std::format("Inserting and finding {} elements\n\n", N);

    // Create vector of keys
    std::vector<int> keys(N);
    for (int i = 0; i < N; i++) keys[i] = i;
    std::shuffle(keys.begin(), keys.end(), std::mt19937{std::random_device{}()});

    // Measure traditional AVLTree insertion time
    auto start = std::chrono::high_resolution_clock::now();
    for (int key : keys) {
        old_tree->insert(key, key);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto old_insert_time = std::chrono::duration<double, std::milli>(end - start).count();
    // debug(old_tree->height());

    // Measure CRTP AVLTree insertion time
    start = std::chrono::high_resolution_clock::now();
    for (int key : keys) {
        new_tree->insert(key, key);
    }
    end = std::chrono::high_resolution_clock::now();
    auto new_insert_time = std::chrono::duration<double, std::milli>(end - start).count();
    // debug(new_tree->root->height);

    // Measure std::map insertion time
    start = std::chrono::high_resolution_clock::now();
    for (int key : keys) {
        std_map[key] = key;
    }
    end = std::chrono::high_resolution_clock::now();
    auto std_insert_time = std::chrono::duration<double, std::milli>(end - start).count();

    // Measure traditional AVLTree find time
    start = std::chrono::high_resolution_clock::now();
    for (int key : keys) {
        volatile auto _ = old_tree->find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    auto old_find_time = std::chrono::duration<double, std::milli>(end - start).count();

    // Measure CRTP AVLTree find time
    start = std::chrono::high_resolution_clock::now();
    for (int key : keys) {
        volatile auto _ = new_tree->find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    auto new_find_time = std::chrono::duration<double, std::milli>(end - start).count();

    // Measure std::map find time
    start = std::chrono::high_resolution_clock::now();
    for (int key : keys) {
        volatile auto _ = std_map.find(key);
    }
    end = std::chrono::high_resolution_clock::now();
    auto std_find_time = std::chrono::duration<double, std::milli>(end - start).count();

    // Print results using std::format
    std::cout << std::format(
        "{:<12} {:<15} {:<15} {:<15} {:<15}\n", "Operation", "Traditional", "CRTP-based",
        "std::map", "CRTP Improvement");

    auto improvement = [](double old_time, double new_time) {
        return (old_time - new_time) / old_time * 100.0;
    };

    std::cout << std::format(
        "{:<12} {:<15.2f} {:<15.2f} {:<15.2f} {:<15.2f}%\n", "Insert (ms)", old_insert_time,
        new_insert_time, std_insert_time, improvement(old_insert_time, new_insert_time));

    std::cout << std::format(
        "{:<12} {:<15.2f} {:<15.2f} {:<15.2f} {:<15.2f}%\n", "Find (ms)", old_find_time,
        new_find_time, std_find_time, improvement(old_find_time, new_find_time));

    // Add comparison with std::map
    std::cout << std::format(
        "\n{:<12} {:<20} {:<20}\n", "Operation", "CRTP vs std::map", "Traditional vs std::map");

    std::cout << std::format(
        "{:<12} {:<20.2f}% {:<20.2f}%\n", "Insert", improvement(std_insert_time, new_insert_time),
        improvement(std_insert_time, old_insert_time));

    std::cout << std::format(
        "{:<12} {:<20.2f}% {:<20.2f}%\n", "Find", improvement(std_find_time, new_find_time),
        improvement(std_find_time, old_find_time));
}
}  // namespace crtp
