#include "debug.hpp"
#include "tree/avl.hpp"
#include "tree/basic.hpp"
#include "tree/splay.hpp"
#include "tree/treap.hpp"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

struct Test {  // extract private/protected members from class
    static bool traverse(auto& node, auto func) {
        if (!node) return true;
        bool result = func(node);
        if (!result) {
            debug(node);
        }
        result = result && traverse(node->child[L], func);
        result = result && traverse(node->child[R], func);
        return result;
    }

    static bool sorted(auto& node) {
        std::vector<std::decay_t<decltype(node->key)>> ret;
        auto dfs = [&ret](auto self, auto& node) {
            if (!node) return;
            self(self, node->child[L]);
            ret.push_back(node->key);
            self(self, node->child[R]);
        };
        dfs(dfs, node);
        for (int i = 0; i < (int)ret.size() - 1; i++) {
            if (ret[i] > ret[i + 1]) {
                // debug(ret);
                debug(i, ret[i], ret[i + 1]);
                return false;
            }
        }
        return true;
    }

    constexpr static auto CHECK_SIZE = [](auto& node) {
        return node->size == 1 + (node->child[L] ? node->child[L]->size : 0) +
                                 (node->child[R] ? node->child[R]->size : 0);
    };

    constexpr static auto CHECK_PARENT = [](auto& node) {
        if (node->child[L]) {
            if (node->child[L]->parent != node.get()) return false;
        }
        if (node->child[R]) {
            if (node->child[R]->parent != node.get()) return false;
        }
        return true;
    };

    constexpr static auto CHECK_HEIGHT = [](auto& node) {
        int left_height = node->child[L] ? node->child[L]->height : 0;
        int right_height = node->child[R] ? node->child[R]->height : 0;
        return node->height == std::max(left_height, right_height) + 1;
    };

    constexpr static auto CHECK_BALANCE = [](auto& node) {
        int factor = node->balanceFactor();
        return factor >= -1 && factor <= 1;
    };

    constexpr static auto CHECK_PRIORITY = [](auto& node) {
        if (!node->parent) return true;
        return node->parent->priority >= node->priority;
    };

    // static auto& extractTree(auto& container) { return container->impl; }

    static void check(auto& tree) {
        // auto& tree = extractTree(container);
        CHECK(sorted(tree->root));
        CHECK(traverse(tree->root, CHECK_PARENT));
        CHECK(traverse(tree->root, CHECK_SIZE));
    }

    static void checkAVL(auto& avl_tree) {
        CHECK(sorted(avl_tree->root));
        CHECK(traverse(avl_tree->root, CHECK_PARENT));
        CHECK(traverse(avl_tree->root, CHECK_SIZE));
        CHECK(traverse(avl_tree->root, CHECK_HEIGHT));
        CHECK(traverse(avl_tree->root, CHECK_BALANCE));
    }

    static void checkTreap(auto& treap_tree) {
        CHECK(sorted(treap_tree->root));
        CHECK(traverse(treap_tree->root, CHECK_PARENT));
        CHECK(traverse(treap_tree->root, CHECK_SIZE));
        CHECK(traverse(treap_tree->root, CHECK_PRIORITY));
    }

    static auto& findNode(auto& tree, auto&& key) {
        auto [_, node] = tree->findBox(tree->root, key);
        return node;
    }
};

TEST_CASE("`Tree` insertion, find") {
    auto tree = std::make_unique<BasicTree<int, std::string>>();

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
    auto tree = std::make_unique<BasicTreeImpl<int, std::string>>();
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
            Test::check(tree);
        }

        SUBCASE("two children") {
            CHECK(tree->remove(50) == Status::SUCCESS);
            CHECK(tree->find(50) == nullptr);
            CHECK(tree->size() == 6);
            // debug(tree);
            // tree->printCLI();
            Test::check(tree);
        }

        SUBCASE("remove all") {
            int size = tree->size();
            for (int i = size - 1; i >= 0; i--) {
                CHECK(tree->remove(tree->root->key) == Status::SUCCESS);
                CHECK(tree->size() == i);
                Test::check(tree);
            }
        }
    }

    SUBCASE("Split and merge") {
        // Split at 50
        auto other = tree->split(50);
        CHECK(other != nullptr);
        CHECK(tree->size() + other->size() == 7);

        Test::check(tree);
        Test::check(other);

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
        tree->merge(std::move(other));
        CHECK(tree->size() == 7);
        CHECK(tree->find(50) != nullptr);
        CHECK(tree->find(70) != nullptr);
        CHECK(tree->find(60) != nullptr);
        CHECK(tree->find(80) != nullptr);

        Test::check(tree);
    }
}

TEST_CASE("`Tree` removal, split, merge") {
    auto tree = std::make_unique<BasicTreeImpl<int, std::string>>();
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
            Test::check(tree);
        }

        SUBCASE("two children") {
            CHECK(tree->remove(50) == Status::SUCCESS);
            CHECK(tree->find(50) == nullptr);
            CHECK(tree->size() == 6);
            // debug(tree);
            // tree->printCLI();
            Test::check(tree);
        }

        SUBCASE("remove all") {
            int size = tree->size();
            for (int i = size - 1; i >= 0; i--) {
                CHECK(tree->remove(tree->root->key) == Status::SUCCESS);
                CHECK(tree->size() == i);
                Test::check(tree);
            }
        }
    }

    SUBCASE("Split and merge") {
        // Split at 50
        auto other = tree->split(50);
        CHECK(other != nullptr);
        CHECK(tree->size() + other->size() == 7);

        CHECK(Test::sorted(tree->root));
        CHECK(Test::traverse(tree->root, Test::CHECK_SIZE));
        CHECK(Test::traverse(tree->root, Test::CHECK_PARENT));
        CHECK(Test::sorted(other->root));
        CHECK(Test::traverse(other->root, Test::CHECK_SIZE));
        CHECK(Test::traverse(other->root, Test::CHECK_PARENT));

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
        tree->merge(std::move(other));
        CHECK(tree->size() == 7);
        CHECK(tree->find(50) != nullptr);
        CHECK(tree->find(70) != nullptr);
        CHECK(tree->find(60) != nullptr);
        CHECK(tree->find(80) != nullptr);

        Test::check(tree);
    }
}

TEST_CASE("`AVLTree` insertion") {
    auto tree = std::make_unique<AVLTreeImpl<int, std::string>>();

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
        auto& node = Test::findNode(tree, 50);
        CHECK(node);
        CHECK(node->key == 50);
        CHECK(node->value == "fifty");
        CHECK(node->height == 2);
        CHECK(node->balanceFactor() == 0);
    }

    SUBCASE("Right-Right rotation (LL)") {
        // Create a right-heavy tree that needs LL rotation
        CHECK(tree->insert(30, "thirty") == Status::SUCCESS);
        CHECK(tree->insert(20, "twenty") == Status::SUCCESS);
        CHECK(tree->insert(10, "ten") == Status::SUCCESS);

        // Verify rotation occurred
        CHECK(Test::findNode(tree, 20) == tree->root);
        CHECK(Test::findNode(tree, 10) == tree->root->child[L]);
        CHECK(Test::findNode(tree, 30) == tree->root->child[R]);

        // Check balance factors
        CHECK(tree->root->balanceFactor() == 0);
        CHECK(tree->root->height == 2);
    }

    SUBCASE("Left-Left rotation (RR)") {
        // Create a left-heavy tree that needs RR rotation
        CHECK(tree->insert(10, "ten") == Status::SUCCESS);
        CHECK(tree->insert(20, "twenty") == Status::SUCCESS);
        CHECK(tree->insert(30, "thirty") == Status::SUCCESS);

        // Verify rotation occurred
        CHECK(Test::findNode(tree, 20) == tree->root);
        CHECK(Test::findNode(tree, 10) == tree->root->child[L]);
        CHECK(Test::findNode(tree, 30) == tree->root->child[R]);

        // Check balance factors
        CHECK(tree->root->balanceFactor() == 0);
        CHECK(tree->root->height == 2);
    }

    SUBCASE("Left-Right rotation (LR)") {
        // Create a tree that needs LR rotation
        CHECK(tree->insert(30, "thirty") == Status::SUCCESS);
        CHECK(tree->insert(10, "ten") == Status::SUCCESS);
        CHECK(tree->insert(20, "twenty") == Status::SUCCESS);

        // Verify rotation occurred
        CHECK(Test::findNode(tree, 20) == tree->root);
        CHECK(Test::findNode(tree, 10) == tree->root->child[L]);
        CHECK(Test::findNode(tree, 30) == tree->root->child[R]);

        // Check balance factors
        CHECK(tree->root->balanceFactor() == 0);
        CHECK(tree->root->height == 2);
    }

    SUBCASE("Right-Left rotation (RL)") {
        // Create a tree that needs RL rotation
        CHECK(tree->insert(10, "ten") == Status::SUCCESS);
        CHECK(tree->insert(30, "thirty") == Status::SUCCESS);
        CHECK(tree->insert(20, "twenty") == Status::SUCCESS);

        // Verify rotation occurred
        CHECK(Test::findNode(tree, 20) == tree->root);
        CHECK(Test::findNode(tree, 10) == tree->root->child[L]);
        CHECK(Test::findNode(tree, 30) == tree->root->child[R]);

        // Check balance factors
        CHECK(tree->root->balanceFactor() == 0);
        CHECK(tree->root->height == 2);
    }

    SUBCASE("Complex insertions and tree balance") {
        // Insert multiple values that trigger various rotations
        int n = 15;
        for (int i = 1; i <= n; i++) {
            CHECK(tree->insert(i, std::to_string(i)) == Status::SUCCESS);
            // debug(tree);
            CHECK(Test::traverse(tree->root, Test::CHECK_HEIGHT));
            CHECK(Test::traverse(tree->root, Test::CHECK_BALANCE));
            CHECK(tree->size() == i);
        }
    }

    SUBCASE("More complex insertions and tree balance") {
        tree->clear();
        // Insert multiple values that trigger various rotations
        int n = 2000;
        for (int i = 1; i <= n; i++) {
            tree->insert(i, std::to_string(i));
        }
        CHECK(tree->size() == n);
        CHECK(Test::traverse(tree->root, Test::CHECK_BALANCE));
        CHECK(Test::traverse(tree->root, Test::CHECK_HEIGHT));
        CHECK(tree->root->height <= std::ceil(std::sqrt(2) * std::log2(n)));
    }
}

TEST_CASE("`AVLTree` removal") {
    auto tree = std::make_unique<AVLTreeImpl<int, std::string>>();

    SUBCASE("Empty tree removal") {
        CHECK(tree->remove(10) == Status::FAILED);
        CHECK(tree->size() == 0);
    }

    SUBCASE("Leaf node removal") {
        // Setup a balanced tree
        tree->insert(50, "fifty");
        tree->insert(30, "thirty");
        tree->insert(70, "seventy");
        tree->insert(20, "twenty");
        tree->insert(40, "forty");
        tree->insert(60, "sixty");
        tree->insert(80, "eighty");

        // Remove a leaf node
        CHECK(tree->remove(20) == Status::SUCCESS);
        CHECK(tree->size() == 6);
        CHECK(tree->find(20) == nullptr);

        // Verify tree properties are maintained
        Test::checkAVL(tree);
    }

    SUBCASE("Single child removal") {
        // Setup tree with a node that has one child
        tree->insert(50, "fifty");
        tree->insert(30, "thirty");
        tree->insert(70, "seventy");
        tree->insert(20, "twenty");

        // Remove a node with one child
        CHECK(tree->remove(30) == Status::SUCCESS);
        CHECK(tree->size() == 3);
        CHECK(tree->find(30) == nullptr);
        CHECK(tree->find(20) != nullptr);

        // Verify tree properties
        Test::checkAVL(tree);
    }

    SUBCASE("Two children removal") {
        // Setup tree
        tree->insert(50, "fifty");
        tree->insert(30, "thirty");
        tree->insert(70, "seventy");
        tree->insert(20, "twenty");
        tree->insert(40, "forty");
        tree->insert(60, "sixty");
        tree->insert(80, "eighty");

        // Remove a node with two children
        CHECK(tree->remove(30) == Status::SUCCESS);
        CHECK(tree->size() == 6);
        CHECK(tree->find(30) == nullptr);
        CHECK(tree->find(20) != nullptr);
        CHECK(tree->find(40) != nullptr);

        // Verify tree properties
        Test::checkAVL(tree);
    }

    SUBCASE("Root removal") {
        // Setup a smaller tree
        tree->insert(50, "fifty");
        tree->insert(30, "thirty");
        tree->insert(70, "seventy");

        // Remove the root
        CHECK(tree->remove(50) == Status::SUCCESS);
        CHECK(tree->size() == 2);
        CHECK(tree->find(50) == nullptr);

        // Verify tree properties
        Test::checkAVL(tree);
    }

    SUBCASE("Removal requiring rebalancing") {
        // Create a tree that will require rebalancing after removal
        tree->insert(50, "fifty");
        tree->insert(30, "thirty");
        tree->insert(70, "seventy");
        tree->insert(20, "twenty");
        tree->insert(40, "forty");
        tree->insert(60, "sixty");
        tree->insert(80, "eighty");
        tree->insert(10, "ten");
        tree->insert(25, "twenty-five");

        // Remove nodes that will trigger rotations
        CHECK(tree->remove(70) == Status::SUCCESS);
        CHECK(tree->remove(80) == Status::SUCCESS);

        // Verify tree properties and balance
        Test::checkAVL(tree);
    }

    SUBCASE("Sequential removal") {
        // Create a larger tree
        for (int i = 1; i <= 15; i++) {
            tree->insert(i, std::to_string(i));
        }

        // Remove nodes in sequence
        for (int i = 1; i <= 7; i++) {
            CHECK(tree->remove(i) == Status::SUCCESS);
            // Verify tree properties after each removal
            Test::checkAVL(tree);
        }

        CHECK(tree->size() == 8);
    }
}

TEST_CASE("`AVLTree` join operation") {
    SUBCASE("Basic join with non-overlapping trees") {
        // Create two trees with non-overlapping keys
        auto tree1 = std::make_unique<AVLTreeImpl<int, std::string>>();
        auto tree2 = std::make_unique<AVLTreeImpl<int, std::string>>();

        // First tree with smaller keys
        tree1->insert(10, "ten");
        tree1->insert(5, "five");
        tree1->insert(15, "fifteen");

        // Second tree with larger keys
        tree2->insert(30, "thirty");
        tree2->insert(25, "twenty-five");
        tree2->insert(35, "thirty-five");

        // Verify trees before join
        CHECK(tree1->size() == 3);
        CHECK(tree2->size() == 3);
        Test::checkAVL(tree1);
        Test::checkAVL(tree2);

        // Perform join operation
        CHECK(tree1->merge(std::move(tree2)) == Status::SUCCESS);

        // Verify the joined tree
        CHECK(tree1->size() == 6);
        CHECK(tree1->find(5) != nullptr);
        CHECK(tree1->find(35) != nullptr);
        Test::checkAVL(tree1);
    }

    SUBCASE("Join with empty trees") {
        auto tree1 = std::make_unique<AVLTreeImpl<int, std::string>>();
        auto tree2 = std::make_unique<AVLTreeImpl<int, std::string>>();

        SUBCASE("First tree empty") {
            tree2->insert(30, "thirty");
            tree2->insert(20, "twenty");
            tree2->insert(40, "forty");

            CHECK(tree1->merge(std::move(tree2)) == Status::SUCCESS);
            CHECK(tree1->size() == 3);
            CHECK(tree1->find(30) != nullptr);
            Test::checkAVL(tree1);
        }

        SUBCASE("Second tree empty") {
            tree1->insert(10, "ten");
            tree1->insert(5, "five");
            tree1->insert(15, "fifteen");

            auto empty = std::make_unique<AVLTreeImpl<int, std::string>>();
            CHECK(tree1->merge(std::move(empty)) == Status::SUCCESS);
            CHECK(tree1->size() == 3);
            Test::checkAVL(tree1);
        }

        SUBCASE("Both trees empty") {
            CHECK(tree1->merge(std::move(tree2)) == Status::SUCCESS);
            CHECK(tree1->size() == 0);
        }
    }

    SUBCASE("Join with height differences") {
        auto tall = std::make_unique<AVLTreeImpl<int, std::string>>();
        auto short_tree = std::make_unique<AVLTreeImpl<int, std::string>>();

        // Create a taller tree
        for (int i = 1; i <= 7; i++) {
            tall->insert(i, std::to_string(i));
        }

        // Create a shorter tree with larger keys
        short_tree->insert(10, "ten");
        short_tree->insert(15, "fifteen");

        CHECK(tall->merge(std::move(short_tree)) == Status::SUCCESS);
        CHECK(tall->size() == 9);
        CHECK(tall->find(15) != nullptr);
        Test::checkAVL(tall);

        // Test the opposite case (short tree merging tall tree)
        auto tall2 = std::make_unique<AVLTreeImpl<int, std::string>>();
        auto short2 = std::make_unique<AVLTreeImpl<int, std::string>>();

        // Create a shorter tree with smaller keys
        short2->insert(1, "one");
        short2->insert(2, "two");

        // Create a taller tree with larger keys
        for (int i = 10; i <= 20; i++) {
            tall2->insert(i, std::to_string(i));
        }

        CHECK(short2->merge(std::move(tall2)) == Status::SUCCESS);
        CHECK(short2->size() == 13);
        CHECK(short2->find(1) != nullptr);
        CHECK(short2->find(20) != nullptr);
        Test::checkAVL(short2);
    }

    SUBCASE("Four joining scenarios based on tree size and key range") {
        // Test all four combinations of:
        // - This tree smaller/larger than other tree
        // - This tree's keys smaller/larger than other tree's keys

        SUBCASE("1. This tree smaller & keys smaller") {
            auto small_small = std::make_unique<AVLTreeImpl<int, std::string>>();
            auto large_large = std::make_unique<AVLTreeImpl<int, std::string>>();

            // Smaller tree with smaller keys (1-10)
            for (int i = 1; i <= 10; i++) {
                small_small->insert(i, std::to_string(i));
            }

            // Larger tree with larger keys (100-130)
            for (int i = 100; i <= 130; i++) {
                large_large->insert(i, std::to_string(i));
            }

            CHECK(small_small->merge(std::move(large_large)) == Status::SUCCESS);
            CHECK(small_small->size() == 41);
            CHECK(small_small->find(5) != nullptr);
            CHECK(small_small->find(120) != nullptr);
            Test::checkAVL(small_small);
        }

        SUBCASE("2. This tree smaller & keys larger") {
            auto small_large = std::make_unique<AVLTreeImpl<int, std::string>>();
            auto large_small = std::make_unique<AVLTreeImpl<int, std::string>>();

            // Smaller tree with larger keys (100-110)
            for (int i = 100; i <= 110; i++) {
                small_large->insert(i, std::to_string(i));
            }

            // Larger tree with smaller keys (1-30)
            for (int i = 1; i <= 30; i++) {
                large_small->insert(i, std::to_string(i));
            }

            CHECK(small_large->merge(std::move(large_small)) == Status::SUCCESS);
            CHECK(small_large->size() == 41);
            CHECK(small_large->find(15) != nullptr);
            CHECK(small_large->find(105) != nullptr);
            Test::checkAVL(small_large);
        }
    }
}

TEST_CASE("`AVLTree` split operation") {
    SUBCASE("Basic split functionality") {
        auto tree = std::make_unique<AVLTreeImpl<int, std::string>>();

        // Create a balanced tree
        tree->insert(50, "fifty");
        tree->insert(30, "thirty");
        tree->insert(70, "seventy");
        tree->insert(20, "twenty");
        tree->insert(40, "forty");
        tree->insert(60, "sixty");
        tree->insert(80, "eighty");

        // Split at key 50
        auto right_tree = tree->split(50);

        // Verify left tree (original)
        CHECK(tree->size() == 3);
        CHECK(tree->find(20) != nullptr);
        CHECK(tree->find(30) != nullptr);
        CHECK(tree->find(40) != nullptr);
        CHECK(tree->find(50) == nullptr);
        Test::checkAVL(tree);

        // Verify right tree
        CHECK(right_tree->size() == 4);
        CHECK(right_tree->find(50) != nullptr);
        CHECK(right_tree->find(60) != nullptr);
        CHECK(right_tree->find(70) != nullptr);
        CHECK(right_tree->find(80) != nullptr);
        Test::checkAVL(right_tree);
    }

    SUBCASE("Split with empty tree") {
        auto tree = std::make_unique<AVLTreeImpl<int, std::string>>();

        // Split empty tree
        auto right_tree = tree->split(50);

        // Both trees should be empty
        CHECK(tree->size() == 0);
        CHECK(right_tree->size() == 0);
    }

    SUBCASE("Split at minimum key") {
        auto tree = std::make_unique<AVLTreeImpl<int, std::string>>();

        // Insert keys 10-50
        for (int i = 10; i <= 50; i += 10) {
            tree->insert(i, std::to_string(i));
        }

        // Split at minimum key
        auto right_tree = tree->split(10);

        // Verify left tree is empty
        CHECK(tree->size() == 0);

        // Verify right tree has all elements
        CHECK(right_tree->size() == 5);
        CHECK(right_tree->find(10) != nullptr);
        CHECK(right_tree->find(50) != nullptr);
        Test::checkAVL(right_tree);
    }

    SUBCASE("Split at maximum key") {
        auto tree = std::make_unique<AVLTreeImpl<int, std::string>>();

        // Insert keys 10-50
        for (int i = 10; i <= 50; i += 10) {
            tree->insert(i, std::to_string(i));
        }

        // Split at maximum key
        auto right_tree = tree->split(50);

        // Verify left tree has all elements except maximum
        CHECK(tree->size() == 4);
        CHECK(tree->find(10) != nullptr);
        CHECK(tree->find(40) != nullptr);
        CHECK(tree->find(50) == nullptr);
        Test::checkAVL(tree);

        // Verify right tree has only maximum element
        CHECK(right_tree->size() == 1);
        CHECK(right_tree->find(50) != nullptr);
        Test::checkAVL(right_tree);
    }

    SUBCASE("Split larger tree") {
        auto tree = std::make_unique<AVLTreeImpl<int, std::string>>();

        // Insert 100 elements
        for (int i = 1; i <= 100; i++) {
            tree->insert(i, std::to_string(i));
        }

        // Split at middle
        auto right_tree = tree->split(50);

        // Verify left tree
        CHECK(tree->size() == 49);  // 1-49
        CHECK(tree->find(1) != nullptr);
        CHECK(tree->find(49) != nullptr);
        CHECK(tree->find(50) == nullptr);
        Test::checkAVL(tree);

        // Verify right tree
        CHECK(right_tree->size() == 51);  // 50-100
        CHECK(right_tree->find(50) != nullptr);
        CHECK(right_tree->find(100) != nullptr);
        Test::checkAVL(right_tree);
    }
}

TEST_CASE("`AVLTree` complex removal operations") {
    SUBCASE("Random deletion pattern maintaining balance") {
        auto tree = std::make_unique<AVLTreeImpl<int, std::string>>();

        // Create a larger balanced tree with 50 nodes
        for (int i = 1; i <= 50; i++) {
            tree->insert(i, std::to_string(i));
        }

        // Verify initial balance
        Test::checkAVL(tree);
        CHECK(tree->size() == 50);

        // Random deletion pattern - delete every third node
        for (int i = 3; i <= 50; i += 3) {
            CHECK(tree->remove(i) == Status::SUCCESS);
            Test::checkAVL(tree);
        }

        // Delete every fifth remaining node
        for (int i = 5; i <= 50; i += 5) {
            if (i % 3 != 0) {  // Skip already deleted nodes
                CHECK(tree->remove(i) == Status::SUCCESS);
                Test::checkAVL(tree);
            }
        }

        // Check final state
        CHECK(
            tree->size() ==
            50 - 16 - 7);  // 50 initial - 16 (every third) - 7 (every fifth remaining)
    }

    SUBCASE("Sequential deletion from both ends") {
        auto tree = std::make_unique<AVLTreeImpl<int, std::string>>();

        // Build a tree with 30 nodes
        for (int i = 1; i <= 30; i++) {
            tree->insert(i, std::to_string(i));
        }

        // Alternately delete from minimum and maximum
        for (int i = 0; i < 10; i++) {
            // Delete minimum
            int min_key = tree->min()->key;
            CHECK(tree->remove(min_key) == Status::SUCCESS);
            Test::checkAVL(tree);

            // Delete maximum
            int max_key = tree->max()->key;
            CHECK(tree->remove(max_key) == Status::SUCCESS);
            Test::checkAVL(tree);
        }

        // Should have 10 nodes left (30 - 20 deletions)
        CHECK(tree->size() == 10);
    }

    SUBCASE("Deletion causing cascading rotations") {
        auto tree = std::make_unique<AVLTreeImpl<int, std::string>>();

        // Create a specific tree structure that will require multiple rotations
        // after deletion
        for (int key = 1; key <= 31; key++) {
            tree->insert(key, std::to_string(key));
        }
        tree->insert(0, "0");

        // Verify initial balance
        Test::checkAVL(tree);

        // Delete nodes that trigger complex rebalancing
        std::vector<int> delete_sequence = {17, 19, 21, 23, 25, 27, 31, 18, 24};
        for (int key : delete_sequence) {
            CHECK(tree->remove(key) == Status::SUCCESS);
            Test::checkAVL(tree);
        }

        // Check final size
        CHECK(tree->size() == 32 - 9);  // 32 initial nodes - 9 deleted nodes
    }

    SUBCASE("Stress test with interleaved insertions and deletions") {
        auto tree = std::make_unique<AVLTreeImpl<int, std::string>>();

        // Phase 1: Insert 100 nodes
        for (int i = 1; i <= 100; i++) {
            tree->insert(i, std::to_string(i));
        }

        // Phase 2: Delete and insert in interleaved pattern
        for (int i = 1; i <= 50; i++) {
            // Delete a node
            CHECK(tree->remove(i) == Status::SUCCESS);
            Test::checkAVL(tree);

            // Insert a new node
            CHECK(tree->insert(i + 100, std::to_string(i + 100)) == Status::SUCCESS);
            Test::checkAVL(tree);
        }

        // Final check
        CHECK(tree->size() == 100);  // 100 initial, -50 deleted, +50 inserted
        CHECK(tree->find(1) == nullptr);
        CHECK(tree->find(51) != nullptr);
        CHECK(tree->find(150) != nullptr);
    }
}

TEST_CASE("`Treap` basic operations") {
    auto tree = std::make_unique<Treap<int, std::string>>();

    SUBCASE("Insert and Find") {
        CHECK(tree->insert(10, "ten") == Status::SUCCESS);
        CHECK(tree->insert(20, "twenty") == Status::SUCCESS);
        CHECK(tree->insert(15, "fifteen") == Status::SUCCESS);

        auto node = tree->find(10);
        CHECK(node != nullptr);
        CHECK(node->key == 10);
        CHECK(node->value == "ten");

        node = tree->find(20);
        CHECK(node != nullptr);
        CHECK(node->key == 20);

        node = tree->find(15);
        CHECK(node != nullptr);
        CHECK(node->key == 15);

        CHECK(tree->find(99) == nullptr);
    }

    SUBCASE("Remove") {
        tree->insert(10, "ten");
        tree->insert(20, "twenty");
        tree->insert(15, "fifteen");

        CHECK(tree->remove(15) == Status::SUCCESS);
        CHECK(tree->find(15) == nullptr);
        CHECK(tree->remove(10) == Status::SUCCESS);
        CHECK(tree->find(10) == nullptr);
        CHECK(tree->remove(20) == Status::SUCCESS);
        CHECK(tree->find(20) == nullptr);
        CHECK(tree->size() == 0);
    }

    SUBCASE("Split and Join") {
        for (int i = 1; i <= 10; ++i) {
            tree->insert(i, std::to_string(i));
        }
        auto right_tree = tree->split(5);
        CHECK(tree->size() == 4);        // 1-4
        CHECK(right_tree->size() == 6);  // 5-10

        // Check split correctness
        for (int i = 1; i <= 4; ++i) {
            CHECK(tree->find(i) != nullptr);
        }
        for (int i = 5; i <= 10; ++i) {
            CHECK(right_tree->find(i) != nullptr);
        }

        // Join back
        CHECK(tree->merge(std::move(right_tree)) == Status::SUCCESS);
        CHECK(tree->size() == 10);
        for (int i = 1; i <= 10; ++i) {
            CHECK(tree->find(i) != nullptr);
        }
    }
}

TEST_CASE("`Treap` complex operations") {
    auto tree = std::make_unique<TreapImpl<int, std::string>>();

    SUBCASE("Sequential insertions") {
        for (int i = 1; i <= 100; ++i) {
            CHECK(tree->insert(i, std::to_string(i)) == Status::SUCCESS);
            Test::checkTreap(tree);
        }
        CHECK(tree->size() == 100);
    }

    SUBCASE("Random insertions") {
        std::vector<int> keys(100);
        std::iota(keys.begin(), keys.end(), 1);
        std::shuffle(keys.begin(), keys.end(), std::mt19937{std::random_device{}()});
        for (int k : keys) {
            CHECK(tree->insert(k, std::to_string(k)) == Status::SUCCESS);
            Test::checkTreap(tree);
        }
        CHECK(tree->size() == 100);
    }

    SUBCASE("Random removals") {
        for (int i = 1; i <= 100; ++i) {
            tree->insert(i, std::to_string(i));
        }
        std::vector<int> keys(100);
        std::iota(keys.begin(), keys.end(), 1);
        std::shuffle(keys.begin(), keys.end(), std::mt19937{std::random_device{}()});
        for (int k : keys) {
            CHECK(tree->remove(k) == Status::SUCCESS);
            Test::checkTreap(tree);
        }
        CHECK(tree->size() == 0);
    }

    SUBCASE("Split and merge with structure check") {
        for (int i = 1; i <= 50; ++i) {
            tree->insert(i, std::to_string(i));
        }
        auto right_tree = tree->split(25);
        CHECK(tree->size() == 24);
        CHECK(right_tree->size() == 26);
        Test::checkTreap(tree);
        Test::checkTreap(right_tree);

        // Merge back
        CHECK(tree->merge(std::move(right_tree)) == Status::SUCCESS);
        CHECK(tree->size() == 50);
        Test::checkTreap(tree);
    }
}

TEST_CASE("`SplayTree` basic operations") {
    auto tree = std::make_unique<SplayTree<int, std::string>>();

    SUBCASE("Insert and Find") {
        CHECK(tree->insert(10, "ten") == Status::SUCCESS);
        CHECK(tree->insert(20, "twenty") == Status::SUCCESS);
        CHECK(tree->insert(15, "fifteen") == Status::SUCCESS);

        auto node = tree->find(10);
        CHECK(node != nullptr);
        CHECK(node->key == 10);
        CHECK(node->value == "ten");

        node = tree->find(20);
        CHECK(node != nullptr);
        CHECK(node->key == 20);

        node = tree->find(15);
        CHECK(node != nullptr);
        CHECK(node->key == 15);

        CHECK(tree->find(99) == nullptr);
    }

    SUBCASE("Remove") {
        tree->insert(10, "ten");
        tree->insert(20, "twenty");
        tree->insert(15, "fifteen");

        CHECK(tree->remove(15) == Status::SUCCESS);
        CHECK(tree->find(15) == nullptr);
        CHECK(tree->remove(10) == Status::SUCCESS);
        CHECK(tree->find(10) == nullptr);
        CHECK(tree->remove(20) == Status::SUCCESS);
        CHECK(tree->find(20) == nullptr);
        CHECK(tree->size() == 0);
    }

    SUBCASE("Split and Join") {
        for (int i = 1; i <= 10; ++i) {
            tree->insert(i, std::to_string(i));
        }
        auto right_tree = tree->split(5);
        // debug(right_tree);
        // right_tree->printCLI();
        CHECK(tree->size() == 4);        // 1-4
        CHECK(right_tree->size() == 6);  // 5-10

        // Check split correctness
        for (int i = 1; i <= 4; ++i) {
            CHECK(tree->find(i) != nullptr);
        }
        for (int i = 5; i <= 10; ++i) {
            CHECK(right_tree->find(i) != nullptr);
        }

        // Join back
        CHECK(tree->merge(std::move(right_tree)) == Status::SUCCESS);
        CHECK(tree->size() == 10);
        for (int i = 1; i <= 10; ++i) {
            CHECK(tree->find(i) != nullptr);
        }
    }
}
