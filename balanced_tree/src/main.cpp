/// @file main.cpp
/// @brief main function for testing the tree implementations (temporarily)

#include "avl.hpp"
#include "debug.hpp"
#include "doctest/doctest.h"
#include "tree.hpp"

#include <string>
#include <vector>
using namespace std;

void demo() {
    auto forest = vector<unique_ptr<TreeBase>>();

    using TreeType0 = Tree<int, string>;
    using TreeType1 = Tree<int, double>;

    forest.push_back(make_unique<TreeType0>());

    auto tree0 = dynamic_cast<TreeType0*>(forest[0].get());
    tree0->insert(114, "test");
    tree0->insert(514, "tree");
    debug(forest);

    forest.push_back(make_unique<AVLTree<int, string>>());
    tree0 = dynamic_cast<TreeType0*>(forest[1].get());
    tree0->insert(0, "not implemented yet");
    debug(forest);

    forest.push_back(make_unique<TreeType1>());
    auto tree1 = dynamic_cast<TreeType1*>(forest[2].get());
    tree1->insert(123, 4.56);
    debug(forest);
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
    int res = run_test(argc, argv);
    if (argc > 1 && strcmp(argv[1], "test") == 0) return res;
    demo();
    return 0;
}
