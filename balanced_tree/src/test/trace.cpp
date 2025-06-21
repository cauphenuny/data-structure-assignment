#include "doctest/doctest.h"
#include "tree/avl.hpp"
#include "tree/basic.hpp"
#include "tree/splay.hpp"
#include "tree/treap.hpp"

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

TEST_CASE("Recording `AVLTree`") {
    using T = AVLTree<int, int>;
    auto tree = std::make_unique<T>();
    tree->traceStart();
    for (int i = 0; i < 8; i++) tree->insert(i, i * 10);
    printTraceCLI("After inserting 8 elements", tree->trace());

    std::cout << "\n\n--------------------------------\n";
    tree->split(4);
    printTraceCLI("After splitting at key 4", tree->trace());

    std::cout << "\n\n--------------------------------\n";
    auto trace = tree->trace([&] {
        auto tree2 = std::make_unique<T>();
        for (int i = 10; i < 13; i++) {
            tree2->insert(i, i * 10);
        }
        tree->join(std::move(tree2));
    });
    printTraceCLI("After joining another tree", trace);
    // debug(tree);
    // tree->remove(0);
    // debug(tree->getRecord());
}

TEST_CASE("Recording `Treap`") {
    using T = Treap<int, int>;
    auto tree = std::make_unique<T>();
    tree->traceStart();
    for (int i = 0; i < 8; i++) tree->insert(i, i * 10);
    tree->split(4);
    auto trace = tree->trace([&] {
        auto tree2 = std::make_unique<T>();
        for (int i = 10; i < 13; i++) {
            tree2->insert(i, i * 10);
        }
        tree->join(std::move(tree2));
    });
}
