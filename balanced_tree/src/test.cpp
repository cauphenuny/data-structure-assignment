#include "avl.hpp"
#include "tree.hpp"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

bool traverse(auto& node, auto func) {
    bool result = func(node);
    if (node->lchild) result = result && traverse(node->lchild, func);
    if (node->rchild) result = result && traverse(node->rchild, func);
    return result;
}

auto check_size = [](auto& node) {
    return node->size ==
           1 + (node->lchild ? node->lchild->size : 0) + (node->rchild ? node->rchild->size : 0);
};

auto check_parent = [](auto& node) {
    if (node->lchild) {
        if (node->lchild->parent != node.get()) return false;
    }
    if (node->rchild) {
        if (node->rchild->parent != node.get()) return false;
    }
    return true;
};

TEST_CASE("`Tree` insertion, find") {
    auto tree = std::make_unique<Tree<int, std::string>>();

    SUBCASE("Empty tree operations") {
        CHECK(tree->size() == 0);
        CHECK(tree->find(10) == nullptr);
        CHECK(tree->remove(10) == Status::FAILED);
    }

    SUBCASE("Insertion and find") {
        // Insert nodes
        CHECK(tree->insert(50, "fifty") == Status::SUCCESS);
        CHECK(tree->insert(30, "thirty") == Status::SUCCESS);
        CHECK(tree->insert(70, "seventy") == Status::SUCCESS);
        CHECK(tree->insert(20, "twenty") == Status::SUCCESS);
        CHECK(tree->insert(40, "forty") == Status::SUCCESS);
        CHECK(tree->insert(60, "sixty") == Status::SUCCESS);
        CHECK(tree->insert(80, "eighty") == Status::SUCCESS);

        // Check size
        CHECK(tree->size() == 7);

        // Find nodes
        auto node = tree->find(50);
        CHECK(node != nullptr);
        CHECK(node->key == 50);
        CHECK(node->value == "fifty");

        node = tree->find(20);
        CHECK(node != nullptr);
        CHECK(node->key == 20);
        CHECK(node->value == "twenty");

        node = tree->find(80);
        CHECK(node != nullptr);
        CHECK(node->key == 80);
        CHECK(node->value == "eighty");

        // Find non-existent node
        CHECK(tree->find(55) == nullptr);

        // Duplicate insertion
        CHECK(tree->insert(50, "FIFTY") == Status::FAILED);
    }
}

TEST_CASE("`Tree` removal, split, merge") {
    auto tree = std::make_unique<Tree<int, std::string>>();
    // Setup tree
    tree->insert(50, "fifty");
    tree->insert(30, "thirty");
    tree->insert(70, "seventy");
    tree->insert(20, "twenty");
    tree->insert(40, "forty");
    tree->insert(60, "sixty");
    tree->insert(80, "eighty");

    REQUIRE(tree->size() == 7);

    SUBCASE("Removal") {
        SUBCASE("leaf node") {
            // Remove leaf node
            CHECK(tree->remove(20) == Status::SUCCESS);
            CHECK(tree->find(20) == nullptr);
            CHECK(tree->size() == 6);
            SUBCASE("one child") {
                // Remove node with one child
                CHECK(tree->remove(30) == Status::SUCCESS);
                CHECK(tree->find(30) == nullptr);
                CHECK(tree->size() == 5);
                CHECK(tree->find(40) != nullptr);
            }
            CHECK(traverse(tree->root, check_size));
            CHECK(traverse(tree->root, check_parent));
        }

        SUBCASE("two children") {
            CHECK(tree->remove(50) == Status::SUCCESS);
            CHECK(tree->find(50) == nullptr);
            CHECK(tree->size() == 6);
            CHECK(traverse(tree->root, check_size));
            CHECK(traverse(tree->root, check_parent));
        }

        SUBCASE("remove all") {
            int size = tree->size();
            for (int i = size - 1; i >= 0; i--) {
                CHECK(tree->remove(tree->root->key) == Status::SUCCESS);
                CHECK(tree->size() == i);
            }
        }
    }

    SUBCASE("Split and merge") {
        // Split at 50
        auto other = tree->split(50);
        CHECK(other != nullptr);
        CHECK(tree->size() + other->size() == 7);

        CHECK(traverse(tree->root, check_size));
        CHECK(traverse(tree->root, check_parent));
        CHECK(traverse(other->root, check_size));
        CHECK(traverse(other->root, check_parent));

        // Verify split worked correctly
        CHECK(tree->find(30) != nullptr);
        CHECK(tree->find(20) != nullptr);
        CHECK(tree->find(40) != nullptr);
        CHECK(tree->find(50) == nullptr);
        CHECK(tree->find(70) == nullptr);

        CHECK(other->find(50) != nullptr);
        CHECK(other->find(70) != nullptr);
        CHECK(other->find(60) != nullptr);
        CHECK(other->find(80) != nullptr);

        // Merge back
        tree->concat(std::move(other));
        CHECK(tree->size() == 7);
        CHECK(tree->find(50) != nullptr);
        CHECK(tree->find(70) != nullptr);
        CHECK(tree->find(60) != nullptr);
        CHECK(tree->find(80) != nullptr);

        CHECK(traverse(tree->root, check_size));
        CHECK(traverse(tree->root, check_parent));
    }
}

auto check_height = [](auto& node) {
    using Type = std::decay_t<decltype(*node)>;
    using KeyType = typename NodeTraits<Type>::Key;
    using ValueType = typename NodeTraits<Type>::Value;
    auto avl_node = dynamic_cast<AVLNode<KeyType, ValueType>*>(node.get());
    int left_height = avl_node->avlLeft() ? avl_node->avlLeft()->height : 0;
    int right_height = avl_node->avlRight() ? avl_node->avlRight()->height : 0;
    return avl_node->height == std::max(left_height, right_height) + 1;
};

auto check_balance = [](auto& node) {
    using Type = std::decay_t<decltype(*node)>;
    using KeyType = typename NodeTraits<Type>::Key;
    using ValueType = typename NodeTraits<Type>::Value;
    auto avl_node = dynamic_cast<AVLNode<KeyType, ValueType>*>(node.get());
    int factor = avl_node->factor();
    return factor >= -1 && factor <= 1;
};

TEST_CASE("`AVLTree` insertion") {
    auto tree = std::make_unique<AVLTree<int, std::string>>();

    SUBCASE("Empty tree operations") {
        CHECK(tree->size() == 0);
        CHECK(tree->find(10) == nullptr);
    }

    SUBCASE("Basic insertion") {
        // Insert nodes
        CHECK(tree->insert(50, "fifty") == Status::SUCCESS);
        CHECK(tree->insert(30, "thirty") == Status::SUCCESS);
        CHECK(tree->insert(70, "seventy") == Status::SUCCESS);

        // Check size
        CHECK(tree->size() == 3);

        // Find nodes
        auto node = tree->find(50);
        CHECK(node != nullptr);
        CHECK(node->key == 50);
        CHECK(node->value == "fifty");

        // Check AVL specific properties
        auto avl_node = static_cast<AVLNode<int, std::string>*>(node);
        CHECK(avl_node->height == 2);
        CHECK(avl_node->factor() == 0);
    }

    SUBCASE("Right-Right rotation (LL)") {
        // Create a right-heavy tree that needs LL rotation
        CHECK(tree->insert(30, "thirty") == Status::SUCCESS);
        CHECK(tree->insert(20, "twenty") == Status::SUCCESS);
        CHECK(tree->insert(10, "ten") == Status::SUCCESS);

        // Verify rotation occurred
        CHECK(tree->find(20) == tree->root.get());
        CHECK(tree->find(10) == tree->root->lchild.get());
        CHECK(tree->find(30) == tree->root->rchild.get());

        // Check balance factors
        auto root = static_cast<AVLNode<int, std::string>*>(tree->root.get());
        CHECK(root->factor() == 0);
        CHECK(root->height == 2);
    }

    SUBCASE("Left-Left rotation (RR)") {
        // Create a left-heavy tree that needs RR rotation
        CHECK(tree->insert(10, "ten") == Status::SUCCESS);
        CHECK(tree->insert(20, "twenty") == Status::SUCCESS);
        CHECK(tree->insert(30, "thirty") == Status::SUCCESS);

        // Verify rotation occurred
        CHECK(tree->find(20) == tree->root.get());
        CHECK(tree->find(10) == tree->root->lchild.get());
        CHECK(tree->find(30) == tree->root->rchild.get());

        // Check balance factors
        auto root = static_cast<AVLNode<int, std::string>*>(tree->root.get());
        CHECK(root->factor() == 0);
        CHECK(root->height == 2);
    }

    SUBCASE("Left-Right rotation (LR)") {
        // Create a tree that needs LR rotation
        CHECK(tree->insert(30, "thirty") == Status::SUCCESS);
        CHECK(tree->insert(10, "ten") == Status::SUCCESS);
        CHECK(tree->insert(20, "twenty") == Status::SUCCESS);

        // Verify rotation occurred
        CHECK(tree->find(20) == tree->root.get());
        CHECK(tree->find(10) == tree->root->lchild.get());
        CHECK(tree->find(30) == tree->root->rchild.get());

        // Check balance factors
        auto root = static_cast<AVLNode<int, std::string>*>(tree->root.get());
        CHECK(root->factor() == 0);
        CHECK(root->height == 2);
    }

    SUBCASE("Right-Left rotation (RL)") {
        // Create a tree that needs RL rotation
        CHECK(tree->insert(10, "ten") == Status::SUCCESS);
        CHECK(tree->insert(30, "thirty") == Status::SUCCESS);
        CHECK(tree->insert(20, "twenty") == Status::SUCCESS);

        // Verify rotation occurred
        CHECK(tree->find(20) == tree->root.get());
        CHECK(tree->find(10) == tree->root->lchild.get());
        CHECK(tree->find(30) == tree->root->rchild.get());

        // Check balance factors
        auto root = static_cast<AVLNode<int, std::string>*>(tree->root.get());
        CHECK(root->factor() == 0);
        CHECK(root->height == 2);
    }

    SUBCASE("Complex insertions and tree balance") {
        // Insert multiple values that trigger various rotations
        int N = 15;
        for (int i = 1; i <= N; i++) {
            CHECK(tree->insert(i, std::to_string(i)) == Status::SUCCESS);
            // debug(tree);
            CHECK(traverse(tree->root, check_height));
            CHECK(traverse(tree->root, check_balance));
        }
    }

    SUBCASE("More complex insertions and tree balance") {
        tree->clear();
        // Insert multiple values that trigger various rotations
        int N = 2000;
        for (int i = 1; i <= N; i++) {
            tree->insert(i, std::to_string(i));
        }
        CHECK(tree->size() == N);
        CHECK(traverse(tree->root, check_balance));
        CHECK(traverse(tree->root, check_height));
        auto root = static_cast<AVLNode<int, std::string>*>(tree->root.get());
        CHECK(root->height <= std::ceil(std::sqrt(2) * std::log2(N)));
    }
}
