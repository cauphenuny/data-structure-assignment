#include "avl.hpp"
#include "tree.hpp"

using namespace std;

int main() {
    auto node = Node<int, string>(1, "abc");
    node.lchild.reset(new Node<int, string>(2, "def", &node));
    debug(node);

    auto tree = Tree<int, int>::create();
    tree->insert(2, 114);
    tree->insert(1, 514);
    tree->insert(3, 1919);
    tree->insert(4, 810);
    debug(*tree);

    tree = AVLTree<int, int>::create();
    /*
    NOTE: not implemented yet
    avl->insert(2, 114);
    */
    debug(*tree);

    return 0;
}
