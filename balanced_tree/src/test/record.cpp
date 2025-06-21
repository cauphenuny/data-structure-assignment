#include "doctest/doctest.h"
#include "tree/avl.hpp"

TEST_CASE("Recording `AVLTree`") {
    auto tree = std::make_unique<AVLTree<int, int>>();
    tree->startRecording();
    for (int i = 0; i < 4; i++) tree->insert(i, i * 10);
    debug(tree->getRecord());
    tree->remove(0);
    debug(tree->getRecord());
}
