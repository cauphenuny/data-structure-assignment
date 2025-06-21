#include "doctest/doctest.h"
#include "tree/avl.hpp"

TEST_CASE("Recording `AVLTree`") {
    auto tree = std::make_unique<AVLTree<int, int>>();
    for (int i = 0; i < 8; i++) tree->insert(i, i * 10);
    tree->startRecording();
    auto splited = tree->split(4);
    auto record1 = tree->getRecord();
    auto record2 = splited->getRecord();
    debug(record1, record2);
}
