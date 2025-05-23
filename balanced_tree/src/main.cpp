/// @file main.cpp
/// @brief main function for testing the tree implementations (temporarily)

#include "avl.hpp"
#include "debug.hpp"
#include "tree.hpp"

#include <string>
#include <vector>
using namespace std;

int main() {
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

    return 0;
}
