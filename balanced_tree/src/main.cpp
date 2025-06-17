/// @file main.cpp
/// @brief main function for testing the tree implementations (temporarily)

#include "benchmark.hpp"
#include "debug.hpp"
#include "doctest/doctest.h"
#include "tree/avl.hpp"
#include "tree/basic.hpp"
#include "tree/interface.hpp"
#include "tree/treap.hpp"
#include "gui/gui.hpp"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
using namespace std;

void polymorphismDemo() {
    auto forest = vector<unique_ptr<TreeBase>>();

    auto tree0 = BasicTree<int, string>::create();
    tree0->insert(11, "tree");
    tree0->insert(45, "insert");
    tree0->insert(14, "demo");
    forest.push_back(std::move(tree0));

    auto tree1 = BasicTree<int, double>::create();
    tree1->insert(1, 2.34);
    tree1->insert(2, 3.45);
    tree1->insert(3, 4.56);
    tree1->insert(4, 5.67);
    forest.push_back(std::move(tree1));

    debug(forest);

    auto view = forest[0]->view();
    debug(view);
}

void algorithmDemo() {
    auto tree = BasicTree<int, int>::create();

    const int N = 16;
    vector<int> values(N);
    for (int i = 1; i < N; i++) values[i] = values[i - 1] + rand() % 20 + 1;
    sort(values.begin(), values.end());
    auto insert = [&values](auto& tree) {
        for (size_t i = 0; auto v : values) tree->insert(v, ++i);
    };

    insert(tree);
    debug(tree->size());
    cout << "(basic)" << '\n', tree->printCLI();

    tree = Treap<int, int>::create();
    insert(tree);
    debug(tree->size());
    cout << "(Treap)" << '\n', tree->printCLI();

    tree = AVLTree<int, int>::create();
    insert(tree);
    debug(tree->size());
    cout << "(AVL)" << '\n', tree->printCLI();
}

int runTest(int argc, char* argv[], bool run_perf) {
    doctest::Context context;
    context.setOption("no-breaks", true);
    context.applyCommandLine(argc, argv);
    if (!run_perf) context.addFilter("test-case-exclude", "*PERF*");
    int res = context.run();
    if (context.shouldExit()) exit(res);
    return res;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && strcmp(argv[1], "gui") == 0) {
        GUIBase gui;
        gui.run();
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "test") == 0) return runTest(argc, argv, true);
    int ret = runTest(argc, argv, false);
    cout << "====================" << '\n';
    polymorphismDemo();
    cout << "====================" << '\n';
    algorithmDemo();
    cout << "====================" << '\n';
    benchmark();
    return ret;
}
