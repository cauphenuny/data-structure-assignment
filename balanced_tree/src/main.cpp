/// @file main.cpp
/// @brief main function for testing the tree implementations (temporarily)

#include "avl.hpp"
#include "tree.hpp"

using namespace std;

int main() {
    auto node = Node<int, string>(1, "abc");
    node.lchild.reset(new Node<int, string>(2, "def", &node));
    debug(node);

    auto tree = Tree<int, string>::create();
    tree->insert(114, "test");
    tree->insert(514, "tree");
    tree->insert(1919, "insert");
    tree->insert(810, "function");
    debug(*tree);

    tree = AVLTree<int, string>::create();
    tree->insert(2, "not implemented yet");
    debug(*tree);

    return 0;
}
