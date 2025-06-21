#include "doctest/doctest.h"
#include "tree/avl.hpp"

static void printNodeViewCLI(const NodeView* node, int depth) {
    if (!node) return;
    printNodeViewCLI(node->child[R].get(), depth + 1);
    std::cout << std::string(depth * 4, ' ') << serialize(node->content()) << "\n";
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

static void printTraceCLI(std::string_view title, const std::vector<ForestView>& record) {
    std::cout << title << ":\n";
    for (size_t i = 0; i < record.size(); ++i) {
        std::cout << "Record " << i + 1 << ":\n";
        printForestCLI(record[i]);
        std::cout << "-----------------------\n";
    }
}

TEST_CASE("Recording `AVLTree`") {
    auto tree = std::make_unique<AVLTree<int, int>>();
    tree->traceStart();
    for (int i = 0; i < 8; i++) tree->insert(i, i * 10);
    printTraceCLI("After inserting 5 elements", tree->trace());

    std::cout << "\n\n--------------------------------\n";
    tree->split(4);
    printTraceCLI("After splitting at key 4", tree->trace());

    std::cout << "\n\n--------------------------------\n";
    auto trace = tree->trace([&] {
        auto tree2 = std::make_unique<AVLTree<int, int>>();
        for (int i = 10; i < 13; i++) {
            tree2->insert(i, i * 10);
        }
        tree->join(std::move(tree2));
    });
    printTraceCLI("After joining another tree", trace);
    // tree->remove(0);
    // debug(tree->getRecord());
}
