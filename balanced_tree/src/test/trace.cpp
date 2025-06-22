#include "doctest/doctest.h"
#include "tree/avl.hpp"
#include "tree/basic.hpp"
#include "tree/splay.hpp"
#include "tree/treap.hpp"

#include <format>
#include <iostream>
#include <random>
#include <vector>

static void printNodeViewCLI(const NodeView* node, int depth) {
    if (!node) return;
    printNodeViewCLI(node->child[R].get(), depth + 1);
    auto [key, value] = node->content();
    std::cout << std::format(
        "{}{{{}: {}}} at {}\n", std::string(depth * 4, ' '), key, value, node->id());
    printNodeViewCLI(node->child[L].get(), depth + 1);
}

static void printForestCLI(const ForestView& forest) {
    for (size_t i = 0; auto& node : forest) {
        if (node) {
            if (i++) std::cout << "--\n";
            printNodeViewCLI(node.get(), 0);
        }
    }
}

static void printTraceCLI(std::string_view title, const std::vector<ForestView>& trace) {
    std::cout << title << ":\n";
    for (size_t i = 0; i < trace.size(); ++i) {
        std::cout << "Trace #" << i + 1 << ":\n";
        printForestCLI(trace[i]);
        std::cout << "-----------------------\n";
    }
}

TEST_CASE("Trace") {
    auto test = [](auto tree, bool echo) {
        tree->traceStart();
        for (int i = 0; i < 8; i++) tree->insert(i, i * 10);
        if (echo) printTraceCLI("After inserting 8 elements", tree->trace());

        if (echo) std::cout << "\n\n--------------------------------\n";
        auto tree2 = tree->split(4);
        if (echo) printTraceCLI("After splitting at key 4", tree->trace());

        if (echo) std::cout << "\n\n--------------------------------\n";
        auto trace = tree->trace([&] {
            tree2->clear();
            for (int i = 10; i < 13; i++) {
                tree2->insert(i, i * 10);
            }
            tree->join(std::move(tree2));
        });
        if (echo) printTraceCLI("After joining another tree", trace);

        if (echo) std::cout << "\n\n--------------------------------\n";
        trace = tree->trace([&] {
            auto keys = std::vector<int>();
            tree->traverse([&](auto&& node) { keys.push_back(node->key); });
            std::shuffle(keys.begin(), keys.end(), std::mt19937{std::random_device{}()});
            if (echo) debug(keys);
            for (auto k : keys) {
                tree->remove(k);
            }
        });
        if (echo) printTraceCLI("Random deleting all node", trace);
        // debug(tree);
        // tree->remove(0);
        // debug(tree->getRecord());
    };
    test(std::make_unique<AVLTree<int, int>>(), true);
    test(std::make_unique<BasicTree<int, int>>(), false);
    test(std::make_unique<Treap<int, int>>(), false);
    test(std::make_unique<SplayTree<int, int>>(), false);
}
