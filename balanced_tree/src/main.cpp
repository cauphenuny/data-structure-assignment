/// @file main.cpp
/// @brief main function for testing the tree implementations (temporarily)

#include "avl.hpp"
#include "debug.hpp"
#include "doctest/doctest.h"
#include "tree.hpp"

#include <string>
#include <vector>
using namespace std;

void polymorphism_demo() {
    auto forest = vector<unique_ptr<TreeBase>>();

    auto tree0 = make_unique<Tree<int, string>>();
    tree0->insert(11, "tree");
    tree0->insert(45, "insert");
    tree0->insert(14, "demo");
    forest.push_back(std::move(tree0));

    auto tree1 = make_unique<Tree<int, double>>();
    tree1->insert(1, 2.34);
    tree1->insert(2, 3.45);
    tree1->insert(3, 4.56);
    tree1->insert(4, 5.67);
    forest.push_back(std::move(tree1));

    debug(forest);
}

void algorithm_demo() {
    auto tree = make_unique<Tree<int, string>>();

    const int N = 16;
    vector<int> values(N);
    for (int i = 1; i < N; i++) values[i] = values[i - 1] + rand() % 20 + 1;
    sort(values.begin(), values.end());
    for (int i = 1; i <= N; i++) {
        tree->insert(values[i - 1], "inserted on #" + to_string(i));
    }
    debug(tree->size());
    cout << "(basic)" << endl;
    tree->printCLI();

    tree = make_unique<AVLTree<int, string>>();
    for (int i = 1; i <= N; i++) {
        tree->insert(values[i - 1], "inserted on #" + to_string(i));
    }
    debug(tree->size());
    cout << "(AVL)" << endl;
    tree->printCLI();
}

int run_test(int argc, char* argv[]) {
    doctest::Context context;
    context.setOption("no-breaks", true);
    context.applyCommandLine(argc, argv);
    int res = context.run();
    if (context.shouldExit()) exit(res);
    return res;
}

int main(int argc, char* argv[]) {
    int ret = run_test(argc, argv);
    if (argc > 1 && strcmp(argv[1], "test") == 0) return ret;  // only run tests
    cout << "====================" << endl;
    polymorphism_demo();
    cout << "====================" << endl;
    algorithm_demo();
    return ret;
}
