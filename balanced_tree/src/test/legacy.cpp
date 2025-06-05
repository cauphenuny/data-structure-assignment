#include "_legacy/avl.hpp"
#include "_legacy/treap.hpp"
#include "_legacy/tree.hpp"
#include "doctest/doctest.h"

#include <chrono>
#include <iomanip>
#include <map>
#include <random>

namespace legacy {

struct Test {  // extract private/protected members from class
    static bool traverse(auto& node, auto func) {
        if (!node) return true;
        bool result = func(node);
        if (!result) {
            debug(node);
        }
        result = result && traverse(node->lchild, func);
        result = result && traverse(node->rchild, func);
        return result;
    }

    static bool sorted(auto& node) {
        std::vector<decltype(node->key)> ret;
        auto dfs = [&ret](auto self, auto& node) {
            if (!node) return;
            self(self, node->lchild);
            ret.push_back(node->key);
            self(self, node->rchild);
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

    constexpr static auto check_size = [](auto& node) {
        return node->size == 1 + (node->lchild ? node->lchild->size : 0) +
                                 (node->rchild ? node->rchild->size : 0);
    };

    constexpr static auto check_parent = [](auto& node) {
        if (node->lchild) {
            if (node->lchild->parent != node.get()) return false;
        }
        if (node->rchild) {
            if (node->rchild->parent != node.get()) return false;
        }
        return true;
    };

    constexpr static auto check_height = [](auto& node) {
        using Type = std::decay_t<decltype(*node)>;
        using Key = typename Type::Key;
        using Value = typename Type::Value;
        auto avl_node = dynamic_cast<const AVLTree<Key, Value>::AVLNode*>(node.get());
        int left_height = avl_node->avlLeft() ? avl_node->avlLeft()->height : 0;
        int right_height = avl_node->avlRight() ? avl_node->avlRight()->height : 0;
        return avl_node->height == std::max(left_height, right_height) + 1;
    };

    constexpr static auto check_balance = [](auto& node) {
        using Type = std::decay_t<decltype(*node)>;
        using Key = typename Type::Key;
        using Value = typename Type::Value;
        auto avl_node = dynamic_cast<const AVLTree<Key, Value>::AVLNode*>(node.get());
        int factor = avl_node->factor();
        return factor >= -1 && factor <= 1;
    };

    static void check(auto& tree) {
        CHECK(sorted(tree->root));
        CHECK(traverse(tree->root, check_parent));
        CHECK(traverse(tree->root, check_size));
    }

    static void checkAVL(auto& avl_tree) {
        CHECK(sorted(avl_tree->root));
        CHECK(traverse(avl_tree->root, check_parent));
        CHECK(traverse(avl_tree->root, check_size));
        CHECK(traverse(avl_tree->root, check_height));
        CHECK(traverse(avl_tree->root, check_balance));
    }

    template <typename K, typename V> static auto toAVL(std::unique_ptr<Tree<K, V>> tree) {
        CHECK(dynamic_cast<AVLTree<K, V>*>(tree.get()) != nullptr);
        return std::make_unique<AVLTree<K, V>>(std::move(tree->root));
    }

    constexpr static auto join = [](auto& tree1, auto tree2) {
        return tree1->join(std::move(tree2));
    };
    constexpr static auto mixin = [](auto& tree1, auto tree2) {
        return tree1->mixin(std::move(tree2));
    };
};

TEST_CASE("(legacy) `Tree` insertion, find") {
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

TEST_CASE("(legacy) `Tree` removal, split, merge") {
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
        CHECK(Test::traverse(tree->root, Test::check_size));
        CHECK(Test::traverse(tree->root, Test::check_parent));
        CHECK(Test::sorted(other->root));
        CHECK(Test::traverse(other->root, Test::check_size));
        CHECK(Test::traverse(other->root, Test::check_parent));

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

TEST_CASE("(legacy) `Tree` conflict, join, mix, merge") {
    SUBCASE("Basic non-overlapping merge (join)") {
        auto tree1 = std::make_unique<Tree<int, std::string>>();
        auto tree2 = std::make_unique<Tree<int, std::string>>();

        // First tree with smaller keys
        tree1->insert(10, "ten");
        tree1->insert(5, "five");
        tree1->insert(15, "fifteen");

        // Second tree with larger keys
        tree2->insert(30, "thirty");
        tree2->insert(25, "twenty-five");
        tree2->insert(35, "thirty-five");

        // Perform join operation
        auto result = Test::join(tree1, std::move(tree2));

        CHECK(result == Status::SUCCESS);
        CHECK(tree1->size() == 6);
        CHECK(tree1->find(5) != nullptr);
        CHECK(tree1->find(35) != nullptr);
        CHECK(tree1->find(25)->value == "twenty-five");
        Test::check(tree1);
    }

    SUBCASE("Overlapping merge (mix)") {
        auto tree1 = std::make_unique<Tree<int, std::string>>();
        auto tree2 = std::make_unique<Tree<int, std::string>>();

        // First tree
        tree1->insert(10, "ten");
        tree1->insert(5, "five");
        tree1->insert(15, "fifteen");

        // Second tree with overlapping keys
        tree2->insert(12, "twelve");
        tree2->insert(8, "eight");
        tree2->insert(20, "twenty");

        // Perform mix operation
        auto result = Test::mixin(tree1, std::move(tree2));

        CHECK(result == Status::SUCCESS);
        CHECK(tree1->size() == 6);
        CHECK(tree1->find(5) != nullptr);
        CHECK(tree1->find(20) != nullptr);
        CHECK(tree1->find(8)->value == "eight");
    }

    SUBCASE("Automatic merge selection") {
        auto tree1 = std::make_unique<Tree<int, std::string>>();
        auto tree2 = std::make_unique<Tree<int, std::string>>();

        // First tree
        tree1->insert(10, "ten");
        tree1->insert(5, "five");
        tree1->insert(15, "fifteen");

        // Second tree with non-overlapping keys
        tree2->insert(30, "thirty");
        tree2->insert(25, "twenty-five");
        tree2->insert(35, "thirty-five");

        // Perform automatic merge
        auto result = tree1->merge(std::move(tree2));

        CHECK(result == Status::SUCCESS);
        CHECK(tree1->size() == 6);
        CHECK(tree1->find(35) != nullptr);
    }

    SUBCASE("Edge cases") {
        // Empty trees
        auto empty1 = std::make_unique<Tree<int, std::string>>();
        auto empty2 = std::make_unique<Tree<int, std::string>>();

        // Merge empty trees
        CHECK(empty1->merge(std::move(empty2)) == Status::SUCCESS);
        CHECK(empty1->size() == 0);

        // Single node trees
        auto single1 = std::make_unique<Tree<int, std::string>>();
        auto single2 = std::make_unique<Tree<int, std::string>>();

        single1->insert(1, "one");
        single2->insert(2, "two");

        CHECK(single1->merge(std::move(single2)) == Status::SUCCESS);
        CHECK(single1->size() == 2);
        CHECK(single1->find(1) != nullptr);
        CHECK(single1->find(2) != nullptr);
    }
}

TEST_CASE("(legacy) `Tree` operator[]") {
    auto tree = std::make_unique<Tree<int, std::string>>();

    // Setup tree with some initial values
    tree->insert(10, "ten");
    tree->insert(20, "twenty");
    tree->insert(30, "thirty");

    SUBCASE("Non-const operator[] - accessing existing keys") {
        // Access and modify existing key
        CHECK(tree->operator[](10) == "ten");
        tree->operator[](10) = "TEN";
        CHECK(tree->find(10)->value == "TEN");

        // Shorthand syntax
        (*tree)[20] = "TWENTY";
        CHECK(tree->find(20)->value == "TWENTY");
    }

    SUBCASE("Non-const operator[] - inserting new keys") {
        // Access non-existing key should insert default value
        CHECK(tree->find(40) == nullptr);
        CHECK(tree->operator[](40) == "");  // Default-constructed string is empty
        CHECK(tree->find(40) != nullptr);
        CHECK(tree->size() == 4);

        // Modify the newly inserted value
        tree->operator[](40) = "forty";
        CHECK(tree->find(40)->value == "forty");
    }

    SUBCASE("Const operator[] - accessing existing keys") {
        const auto& const_tree = *tree;
        CHECK(const_tree[10] == "ten");
        CHECK(const_tree[20] == "twenty");
        CHECK(const_tree[30] == "thirty");
    }

    SUBCASE("Const operator[] - throws on missing keys") {
        const auto& const_tree = *tree;
        CHECK_THROWS_AS(const_tree[40], std::out_of_range);
        // Size remains unchanged
        CHECK(tree->size() == 3);
    }
}

TEST_CASE("(legacy) `AVLTree` insertion") {
    auto tree = std::make_unique<AVLTree<int, std::string>>();

    SUBCASE("Empty tree operations") {
        CHECK(tree->size() == 0);
        CHECK(tree->find(10) == nullptr);
    }

    using AVLNodeType = AVLTree<int, std::string>::AVLNode;

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
        auto avl_node = static_cast<const AVLNodeType*>(node);
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
        CHECK(tree->find(10) == tree->root->left());
        CHECK(tree->find(30) == tree->root->right());

        // Check balance factors
        auto root = static_cast<const AVLNodeType*>(tree->root.get());
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
        CHECK(tree->find(10) == tree->root->left());
        CHECK(tree->find(30) == tree->root->right());

        // Check balance factors
        auto root = static_cast<AVLNodeType*>(tree->root.get());
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
        CHECK(tree->find(10) == tree->root->left());
        CHECK(tree->find(30) == tree->root->right());

        // Check balance factors
        auto root = static_cast<AVLNodeType*>(tree->root.get());
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
        CHECK(tree->find(10) == tree->root->left());
        CHECK(tree->find(30) == tree->root->right());

        // Check balance factors
        auto root = static_cast<AVLNodeType*>(tree->root.get());
        CHECK(root->factor() == 0);
        CHECK(root->height == 2);
    }

    SUBCASE("Complex insertions and tree balance") {
        // Insert multiple values that trigger various rotations
        int N = 15;
        for (int i = 1; i <= N; i++) {
            CHECK(tree->insert(i, std::to_string(i)) == Status::SUCCESS);
            // debug(tree);
            CHECK(Test::traverse(tree->root, Test::check_height));
            CHECK(Test::traverse(tree->root, Test::check_balance));
            CHECK(tree->size() == i);
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
        CHECK(Test::traverse(tree->root, Test::check_balance));
        CHECK(Test::traverse(tree->root, Test::check_height));
        auto root = static_cast<AVLNodeType*>(tree->root.get());
        CHECK(root->height <= std::ceil(std::sqrt(2) * std::log2(N)));
    }
}

TEST_CASE("(legacy) `AVLTree` removal") {
    auto tree = std::make_unique<AVLTree<int, std::string>>();

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

TEST_CASE("(legacy) `AVLTree` join operation") {
    SUBCASE("Basic join with non-overlapping trees") {
        // Create two trees with non-overlapping keys
        auto tree1 = std::make_unique<AVLTree<int, std::string>>();
        auto tree2 = std::make_unique<AVLTree<int, std::string>>();

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
        auto tree1 = std::make_unique<AVLTree<int, std::string>>();
        auto tree2 = std::make_unique<AVLTree<int, std::string>>();

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

            auto empty = std::make_unique<AVLTree<int, std::string>>();
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
        auto tall = std::make_unique<AVLTree<int, std::string>>();
        auto short_tree = std::make_unique<AVLTree<int, std::string>>();

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
        auto tall2 = std::make_unique<AVLTree<int, std::string>>();
        auto short2 = std::make_unique<AVLTree<int, std::string>>();

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

    SUBCASE("Join larger trees") {
        auto tree1 = std::make_unique<AVLTree<int, std::string>>();
        auto tree2 = std::make_unique<AVLTree<int, std::string>>();

        // First tree with keys 1-50
        for (int i = 1; i <= 50; i++) {
            tree1->insert(i, std::to_string(i));
        }

        // Second tree with keys 100-150
        for (int i = 100; i <= 150; i++) {
            tree2->insert(i, std::to_string(i));
        }

        CHECK(tree1->merge(std::move(tree2)) == Status::SUCCESS);
        CHECK(tree1->size() == 101);
        CHECK(tree1->find(1) != nullptr);
        CHECK(tree1->find(50) != nullptr);
        CHECK(tree1->find(100) != nullptr);
        CHECK(tree1->find(150) != nullptr);
        Test::checkAVL(tree1);
    }

    SUBCASE("Join smaller tree into larger tree") {
        auto tree1 = std::make_unique<AVLTree<int, std::string>>();
        auto tree2 = std::make_unique<AVLTree<int, std::string>>();

        // First tree with fewer keys (1-25)
        for (int i = 1; i <= 25; i++) {
            tree1->insert(i, std::to_string(i));
        }

        // Second tree with more keys (100-150)
        for (int i = 100; i <= 150; i++) {
            tree2->insert(i, std::to_string(i));
        }

        // Check initial sizes
        CHECK(tree1->size() == 25);
        CHECK(tree2->size() == 51);

        // Join smaller tree into larger tree
        CHECK(tree2->merge(std::move(tree1)) == Status::SUCCESS);
        CHECK(tree2->size() == 76);
        CHECK(tree2->find(1) != nullptr);
        CHECK(tree2->find(25) != nullptr);
        CHECK(tree2->find(100) != nullptr);
        CHECK(tree2->find(150) != nullptr);
        Test::checkAVL(tree2);
    }
    SUBCASE("Four joining scenarios based on tree size and key range") {
        // Test all four combinations of:
        // - This tree smaller/larger than other tree
        // - This tree's keys smaller/larger than other tree's keys

        SUBCASE("1. This tree smaller & keys smaller") {
            auto small_small = std::make_unique<AVLTree<int, std::string>>();
            auto large_large = std::make_unique<AVLTree<int, std::string>>();

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
            auto small_large = std::make_unique<AVLTree<int, std::string>>();
            auto large_small = std::make_unique<AVLTree<int, std::string>>();

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

        SUBCASE("3. This tree larger & keys smaller") {
            auto large_small = std::make_unique<AVLTree<int, std::string>>();
            auto small_large = std::make_unique<AVLTree<int, std::string>>();

            // Larger tree with smaller keys (1-30)
            for (int i = 1; i <= 30; i++) {
                large_small->insert(i, std::to_string(i));
            }

            // Smaller tree with larger keys (100-110)
            for (int i = 100; i <= 110; i++) {
                small_large->insert(i, std::to_string(i));
            }

            CHECK(large_small->merge(std::move(small_large)) == Status::SUCCESS);
            CHECK(large_small->size() == 41);
            CHECK(large_small->find(15) != nullptr);
            CHECK(large_small->find(105) != nullptr);
            Test::checkAVL(large_small);
        }

        SUBCASE("4. This tree larger & keys larger") {
            auto large_large = std::make_unique<AVLTree<int, std::string>>();
            auto small_small = std::make_unique<AVLTree<int, std::string>>();

            // Larger tree with larger keys (100-130)
            for (int i = 100; i <= 130; i++) {
                large_large->insert(i, std::to_string(i));
            }

            // Smaller tree with smaller keys (1-10)
            for (int i = 1; i <= 10; i++) {
                small_small->insert(i, std::to_string(i));
            }

            CHECK(large_large->merge(std::move(small_small)) == Status::SUCCESS);
            CHECK(large_large->size() == 41);
            CHECK(large_large->find(5) != nullptr);
            CHECK(large_large->find(120) != nullptr);
            Test::checkAVL(large_large);
        }
    }
}

TEST_CASE("(legacy) `AVLTree` split operation") {
    SUBCASE("Basic split functionality") {
        auto tree = std::make_unique<AVLTree<int, std::string>>();

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
        auto avl_right_tree = Test::toAVL(std::move(right_tree));
        Test::checkAVL(avl_right_tree);
    }

    SUBCASE("Split with empty tree") {
        auto tree = std::make_unique<AVLTree<int, std::string>>();

        // Split empty tree
        auto right_tree = tree->split(50);

        // Both trees should be empty
        CHECK(tree->size() == 0);
        CHECK(right_tree->size() == 0);
    }

    SUBCASE("Split at minimum key") {
        auto tree = std::make_unique<AVLTree<int, std::string>>();

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
        auto avl_right_tree = Test::toAVL(std::move(right_tree));
        Test::checkAVL(avl_right_tree);
    }

    SUBCASE("Split at maximum key") {
        auto tree = std::make_unique<AVLTree<int, std::string>>();

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
        auto avl_right_tree = Test::toAVL(std::move(right_tree));
        Test::checkAVL(avl_right_tree);
    }

    SUBCASE("Split larger tree") {
        auto tree = std::make_unique<AVLTree<int, std::string>>();

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
        auto avl_right_tree = Test::toAVL(std::move(right_tree));
        Test::checkAVL(avl_right_tree);
    }
}

TEST_CASE("(legacy) `AVLTree` complex removal operations") {
    SUBCASE("Random deletion pattern maintaining balance") {
        auto tree = std::make_unique<AVLTree<int, std::string>>();

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
        auto tree = std::make_unique<AVLTree<int, std::string>>();

        // Build a tree with 30 nodes
        for (int i = 1; i <= 30; i++) {
            tree->insert(i, std::to_string(i));
        }

        // Alternately delete from minimum and maximum
        for (int i = 0; i < 10; i++) {
            // Delete minimum
            int min_key = tree->minimum()->key;
            CHECK(tree->remove(min_key) == Status::SUCCESS);
            Test::checkAVL(tree);

            // Delete maximum
            int max_key = tree->maximum()->key;
            CHECK(tree->remove(max_key) == Status::SUCCESS);
            Test::checkAVL(tree);
        }

        // Should have 10 nodes left (30 - 20 deletions)
        CHECK(tree->size() == 10);
    }

    SUBCASE("Deletion causing cascading rotations") {
        auto tree = std::make_unique<AVLTree<int, std::string>>();

        // Create a specific tree structure that will require multiple rotations
        // after deletion
        for (int key = 1; key <= 31; key++) {
            tree->insert(key, std::to_string(key));
        }
        tree->insert(0, "0");
        // Verify initial balance
        Test::checkAVL(tree);
        // tree->printCLI();

        // Delete nodes that trigger complex rebalancing
        std::vector<int> delete_sequence = {17, 19, 21, 23, 25, 27, 31, 18, 24};
        for (int key : delete_sequence) {
            CHECK(tree->remove(key) == Status::SUCCESS);
            // debug(key);
            // tree->printCLI();
            Test::checkAVL(tree);
        }

        // Verify height is still optimal after these deletions
        // auto avl_node = static_cast<AVLTree<int, std::string>::AVLNode*>(tree->root.get());
        // CHECK(avl_node->height <= 4);  // With 9 remaining nodes, height should be at most 4
    }

    SUBCASE("Stress test with interleaved insertions and deletions") {
        auto tree = std::make_unique<AVLTree<int, std::string>>();

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

TEST_CASE("(legacy) Performance Comparison") {
    constexpr int NUM_OPERATIONS = 100000;
    constexpr int NUM_OPERATIONS_SEQUENTIAL = 2000;  // Smaller size for sequential test
    constexpr int NUM_LOOKUPS = 10000;
    constexpr int NUM_RUNS = 3;

    std::cout << "\n===== PERFORMANCE BENCHMARK =====\n";
    std::cout << "Operations: " << NUM_OPERATIONS << ", Lookups: " << NUM_LOOKUPS
              << ", Runs: " << NUM_RUNS << "\n\n";

    auto random_generator = std::mt19937(std::random_device{}());

    // Create vectors for keys
    std::vector<int> sequential_keys(NUM_OPERATIONS_SEQUENTIAL);
    std::vector<int> random_keys(NUM_OPERATIONS);
    std::vector<int> lookup_keys(NUM_LOOKUPS);
    std::vector<int> lookup_keys_sequential(NUM_LOOKUPS);

    // Initialize keys
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        if (i < NUM_OPERATIONS_SEQUENTIAL) sequential_keys[i] = i;
        random_keys[i] = i;
    }

    // Shuffle random keys
    std::shuffle(random_keys.begin(), random_keys.end(), random_generator);

    // Select random keys for lookup
    std::uniform_int_distribution<int> dist(0, NUM_OPERATIONS - 1);
    std::uniform_int_distribution<int> dist_seq(0, NUM_OPERATIONS_SEQUENTIAL - 1);
    for (int i = 0; i < NUM_LOOKUPS; i++) {
        lookup_keys[i] = random_keys[dist(random_generator)];
        lookup_keys_sequential[i] = sequential_keys[dist_seq(random_generator)];
    }

    // Function to format time
    auto format_time = [](double ms) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << ms << " ms";
        return ss.str();
    };

    // Function to run timed tests
    auto run_timed_test = [&](const std::string& title, const std::vector<int>& keys,
                              const std::vector<int>& lookups) {
        std::cout << "--- " << title << " size: " << keys.size() << " ---\n";

        double std_map_insert = 0, tree_insert = 0, avl_insert = 0, treap_insert = 0;
        double std_map_lookup = 0, tree_lookup = 0, avl_lookup = 0, treap_lookup = 0;
        double std_map_delete = 0, tree_delete = 0, avl_delete = 0, treap_delete = 0;

        for (int run = 0; run < NUM_RUNS; run++) {
            // std::map test
            {
                std::map<int, std::string> m;

                auto start = std::chrono::high_resolution_clock::now();
                for (int key : keys) {
                    m[key] = std::to_string(key);
                }
                auto end = std::chrono::high_resolution_clock::now();
                std_map_insert += std::chrono::duration<double, std::milli>(end - start).count();

                start = std::chrono::high_resolution_clock::now();
                for (int key : lookups) {
                    auto it = m.find(key);
                    if (it != m.end()) {
                        // Use the value to prevent optimization
                        volatile auto _ = it->second.size();
                    }
                }
                end = std::chrono::high_resolution_clock::now();
                std_map_lookup += std::chrono::duration<double, std::milli>(end - start).count();

                start = std::chrono::high_resolution_clock::now();
                for (int key : lookups) {
                    m.erase(key);
                }
                end = std::chrono::high_resolution_clock::now();
                std_map_delete += std::chrono::duration<double, std::milli>(end - start).count();
            }

            // Tree test
            {
                auto tree = std::make_unique<Tree<int, std::string>>();

                auto start = std::chrono::high_resolution_clock::now();
                for (int key : keys) {
                    tree->insert(key, std::to_string(key));
                }
                auto end = std::chrono::high_resolution_clock::now();
                tree_insert += std::chrono::duration<double, std::milli>(end - start).count();

                start = std::chrono::high_resolution_clock::now();
                for (int key : lookups) {
                    auto node = tree->find(key);
                    if (node) {
                        // Use the value to prevent optimization
                        volatile auto _ = node->value.size();
                    }
                }
                end = std::chrono::high_resolution_clock::now();
                tree_lookup += std::chrono::duration<double, std::milli>(end - start).count();

                start = std::chrono::high_resolution_clock::now();
                for (int key : lookups) {
                    tree->remove(key);
                }
                end = std::chrono::high_resolution_clock::now();
                tree_delete += std::chrono::duration<double, std::milli>(end - start).count();
            }

            // AVLTree test
            {
                auto avl_tree = std::make_unique<AVLTree<int, std::string>>();

                auto start = std::chrono::high_resolution_clock::now();
                for (int key : keys) {
                    avl_tree->insert(key, std::to_string(key));
                }
                auto end = std::chrono::high_resolution_clock::now();
                avl_insert += std::chrono::duration<double, std::milli>(end - start).count();

                start = std::chrono::high_resolution_clock::now();
                for (int key : lookups) {
                    auto node = avl_tree->find(key);
                    if (node) {
                        // Use the value to prevent optimization
                        volatile auto _ = node->value.size();
                    }
                }
                end = std::chrono::high_resolution_clock::now();
                avl_lookup += std::chrono::duration<double, std::milli>(end - start).count();

                start = std::chrono::high_resolution_clock::now();
                for (int key : lookups) {
                    avl_tree->remove(key);
                }
                end = std::chrono::high_resolution_clock::now();
                avl_delete += std::chrono::duration<double, std::milli>(end - start).count();
            }

            // Treap test
            {
                auto treap = std::make_unique<Treap<int, std::string>>();

                auto start = std::chrono::high_resolution_clock::now();
                for (int key : keys) {
                    treap->insert(key, std::to_string(key));
                }
                auto end = std::chrono::high_resolution_clock::now();
                treap_insert += std::chrono::duration<double, std::milli>(end - start).count();

                start = std::chrono::high_resolution_clock::now();
                for (int key : lookups) {
                    auto node = treap->find(key);
                    if (node) {
                        volatile auto _ = node->value.size();
                    }
                }
                end = std::chrono::high_resolution_clock::now();
                treap_lookup += std::chrono::duration<double, std::milli>(end - start).count();

                start = std::chrono::high_resolution_clock::now();
                for (int key : lookups) {
                    treap->remove(key);
                }
                end = std::chrono::high_resolution_clock::now();
                treap_delete += std::chrono::duration<double, std::milli>(end - start).count();
            }
        }

        // Calculate averages
        std_map_insert /= NUM_RUNS;
        std_map_lookup /= NUM_RUNS;
        std_map_delete /= NUM_RUNS;
        tree_insert /= NUM_RUNS;
        tree_lookup /= NUM_RUNS;
        tree_delete /= NUM_RUNS;
        avl_insert /= NUM_RUNS;
        avl_lookup /= NUM_RUNS;
        avl_delete /= NUM_RUNS;
        treap_insert /= NUM_RUNS;
        treap_lookup /= NUM_RUNS;
        treap_delete /= NUM_RUNS;

        // Print results
        std::cout << std::left << std::setw(12) << "Operation" << std::setw(15) << "std::map"
                  << std::setw(15) << "Tree" << std::setw(15) << "AVLTree" << std::setw(15)
                  << "Treap"
                  << "\n";

        std::cout << std::setw(12) << "Insert" << std::setw(15) << format_time(std_map_insert)
                  << std::setw(15) << format_time(tree_insert) << std::setw(15)
                  << format_time(avl_insert) << std::setw(15) << format_time(treap_insert) << "\n";

        std::cout << std::setw(12) << "Lookup" << std::setw(15) << format_time(std_map_lookup)
                  << std::setw(15) << format_time(tree_lookup) << std::setw(15)
                  << format_time(avl_lookup) << std::setw(15) << format_time(treap_lookup) << "\n";

        std::cout << std::setw(12) << "Delete" << std::setw(15) << format_time(std_map_delete)
                  << std::setw(15) << format_time(tree_delete) << std::setw(15)
                  << format_time(avl_delete) << std::setw(15) << format_time(treap_delete)
                  << "\n\n";
    };

    // Run sequential and random tests
    run_timed_test("SEQUENTIAL ACCESS", sequential_keys, lookup_keys_sequential);
    run_timed_test("RANDOM ACCESS", random_keys, lookup_keys);
}

TEST_CASE("(legacy) Split/Merge Performance") {
    constexpr int SMALL_SIZE = 1000;
    constexpr int MEDIUM_SIZE = 10000;
    constexpr int LARGE_SIZE = 100000;
    constexpr int NUM_RUNS = 3;

    std::cout << "\n===== SPLIT/MERGE PERFORMANCE BENCHMARK =====\n";

    auto random_generator = std::mt19937(std::random_device{}());

    // Function to format time
    auto format_time = [](double ms) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << ms << " ms";
        return ss.str();
    };

    // Function to create a tree with random keys, type erased to unique_ptr<Tree<int, std::string>>
    auto insert_keys = [&random_generator](auto& tree, int size) {
        std::vector<int> keys(size);
        for (int i = 0; i < size; i++) {
            keys[i] = i;
        }
        tree->clear();
        std::shuffle(keys.begin(), keys.end(), random_generator);

        for (int key : keys) {
            tree->insert(key, std::to_string(key));
        }
    };

    // Test split operation
    auto test_split = [&](const std::string& title, int size) {
        std::cout << "--- " << title << " Split (" << size << " nodes) ---\n";

        double tree_begin = 0, tree_middle = 0, tree_end = 0;
        double avl_begin = 0, avl_middle = 0, avl_end = 0;
        double treap_begin = 0, treap_middle = 0, treap_end = 0;

        for (int run = 0; run < NUM_RUNS; run++) {
            // Test regular tree split
            {
                auto tree = std::make_unique<Tree<int, std::string>>();
                insert_keys(tree, size);

                // Split at beginning (25%)
                auto begin_key = size / 4;
                auto start = std::chrono::high_resolution_clock::now();
                auto right_tree = tree->split(begin_key);
                auto end = std::chrono::high_resolution_clock::now();
                tree_begin += std::chrono::duration<double, std::milli>(end - start).count();

                // Recreate tree for next test
                tree->clear();
                insert_keys(tree, size);

                // Split at middle (50%)
                auto middle_key = size / 2;
                start = std::chrono::high_resolution_clock::now();
                right_tree = tree->split(middle_key);
                end = std::chrono::high_resolution_clock::now();
                tree_middle += std::chrono::duration<double, std::milli>(end - start).count();

                // Recreate tree for next test
                tree->clear();
                insert_keys(tree, size);

                // Split at end (75%)
                auto end_key = size * 3 / 4;
                start = std::chrono::high_resolution_clock::now();
                right_tree = tree->split(end_key);
                end = std::chrono::high_resolution_clock::now();
                tree_end += std::chrono::duration<double, std::milli>(end - start).count();
            }

            // Test AVL tree split
            {
                auto avl_tree = std::make_unique<AVLTree<int, std::string>>();
                insert_keys(avl_tree, size);

                // Split at beginning (25%)
                auto begin_key = size / 4;
                auto start = std::chrono::high_resolution_clock::now();
                auto right_tree = avl_tree->split(begin_key);
                auto end = std::chrono::high_resolution_clock::now();
                avl_begin += std::chrono::duration<double, std::milli>(end - start).count();

                // Recreate tree for next test
                insert_keys(avl_tree, size);

                // Split at middle (50%)
                auto middle_key = size / 2;
                start = std::chrono::high_resolution_clock::now();
                right_tree = avl_tree->split(middle_key);
                end = std::chrono::high_resolution_clock::now();
                avl_middle += std::chrono::duration<double, std::milli>(end - start).count();

                // Recreate tree for next test
                insert_keys(avl_tree, size);

                // Split at end (75%)
                auto end_key = size * 3 / 4;
                start = std::chrono::high_resolution_clock::now();
                right_tree = avl_tree->split(end_key);
                end = std::chrono::high_resolution_clock::now();
                avl_end += std::chrono::duration<double, std::milli>(end - start).count();
            }

            // Test Treap Tree split
            {
                auto avl_tree = std::make_unique<Treap<int, std::string>>();
                insert_keys(avl_tree, size);

                // Split at beginning (25%)
                auto begin_key = size / 4;
                auto start = std::chrono::high_resolution_clock::now();
                auto right_tree = avl_tree->split(begin_key);
                auto end = std::chrono::high_resolution_clock::now();
                treap_begin += std::chrono::duration<double, std::milli>(end - start).count();

                // Recreate tree for next test
                insert_keys(avl_tree, size);

                // Split at middle (50%)
                auto middle_key = size / 2;
                start = std::chrono::high_resolution_clock::now();
                right_tree = avl_tree->split(middle_key);
                end = std::chrono::high_resolution_clock::now();
                treap_middle += std::chrono::duration<double, std::milli>(end - start).count();

                // Recreate tree for next test
                insert_keys(avl_tree, size);

                // Split at end (75%)
                auto end_key = size * 3 / 4;
                start = std::chrono::high_resolution_clock::now();
                right_tree = avl_tree->split(end_key);
                end = std::chrono::high_resolution_clock::now();
                treap_end += std::chrono::duration<double, std::milli>(end - start).count();
            }
        }

        // Calculate averages
        tree_begin /= NUM_RUNS;
        tree_middle /= NUM_RUNS;
        tree_end /= NUM_RUNS;
        avl_begin /= NUM_RUNS;
        avl_middle /= NUM_RUNS;
        avl_end /= NUM_RUNS;
        treap_begin /= NUM_RUNS;
        treap_middle /= NUM_RUNS;
        treap_end /= NUM_RUNS;

        // Print results
        std::cout << std::left << std::setw(12) << "Split Point" << std::setw(15) << "Tree"
                  << std::setw(15) << "AVLTree" << std::setw(15) << "Treap"
                  << "\n";

        std::cout << std::setw(12) << "Beginning" << std::setw(15) << format_time(tree_begin)
                  << std::setw(15) << format_time(avl_begin) << std::setw(15)
                  << format_time(treap_begin) << "\n";

        std::cout << std::setw(12) << "Middle" << std::setw(15) << format_time(tree_middle)
                  << std::setw(15) << format_time(avl_middle) << std::setw(15)
                  << format_time(treap_middle) << "\n";

        std::cout << std::setw(12) << "End" << std::setw(15) << format_time(tree_end)
                  << std::setw(15) << format_time(avl_end) << std::setw(15)
                  << format_time(treap_end) << "\n\n";
    };

    // Run split tests for different sizes
    test_split("Small", SMALL_SIZE);
    test_split("Medium", MEDIUM_SIZE);

    // Only run large test for AVL tree and Treap to avoid excessive runtime
    std::cout << "--- Large Split (" << LARGE_SIZE << " nodes) ---\n";
    double tree_begin = 0, tree_middle = 0, tree_end = 0;
    double avl_begin = 0, avl_middle = 0, avl_end = 0;
    double treap_begin = 0, treap_middle = 0, treap_end = 0;

    for (int run = 0; run < NUM_RUNS; run++) {
        // Tree large split
        auto tree = std::make_unique<Tree<int, std::string>>();
        insert_keys(tree, LARGE_SIZE);
        auto start = std::chrono::high_resolution_clock::now();
        auto right_tree = tree->split(LARGE_SIZE / 4);
        auto end = std::chrono::high_resolution_clock::now();
        tree_begin += std::chrono::duration<double, std::milli>(end - start).count();

        insert_keys(tree, LARGE_SIZE);
        start = std::chrono::high_resolution_clock::now();
        right_tree = tree->split(LARGE_SIZE / 2);
        end = std::chrono::high_resolution_clock::now();
        tree_middle += std::chrono::duration<double, std::milli>(end - start).count();

        insert_keys(tree, LARGE_SIZE);
        start = std::chrono::high_resolution_clock::now();
        right_tree = tree->split(LARGE_SIZE * 3 / 4);
        end = std::chrono::high_resolution_clock::now();
        tree_end += std::chrono::duration<double, std::milli>(end - start).count();

        // AVLTree large split
        auto avl_tree = std::make_unique<AVLTree<int, std::string>>();
        insert_keys(avl_tree, LARGE_SIZE);
        start = std::chrono::high_resolution_clock::now();
        right_tree = avl_tree->split(LARGE_SIZE / 4);
        end = std::chrono::high_resolution_clock::now();
        avl_begin += std::chrono::duration<double, std::milli>(end - start).count();

        insert_keys(avl_tree, LARGE_SIZE);
        start = std::chrono::high_resolution_clock::now();
        right_tree = avl_tree->split(LARGE_SIZE / 2);
        end = std::chrono::high_resolution_clock::now();
        avl_middle += std::chrono::duration<double, std::milli>(end - start).count();

        insert_keys(avl_tree, LARGE_SIZE);
        start = std::chrono::high_resolution_clock::now();
        right_tree = avl_tree->split(LARGE_SIZE * 3 / 4);
        end = std::chrono::high_resolution_clock::now();
        avl_end += std::chrono::duration<double, std::milli>(end - start).count();

        // Treap large split
        auto treap = std::make_unique<Treap<int, std::string>>();
        insert_keys(treap, LARGE_SIZE);
        start = std::chrono::high_resolution_clock::now();
        auto crtp_right_tree = treap->split(LARGE_SIZE / 4);
        end = std::chrono::high_resolution_clock::now();
        treap_begin += std::chrono::duration<double, std::milli>(end - start).count();

        insert_keys(treap, LARGE_SIZE);
        start = std::chrono::high_resolution_clock::now();
        crtp_right_tree = treap->split(LARGE_SIZE / 2);
        end = std::chrono::high_resolution_clock::now();
        treap_middle += std::chrono::duration<double, std::milli>(end - start).count();

        insert_keys(treap, LARGE_SIZE);
        start = std::chrono::high_resolution_clock::now();
        crtp_right_tree = treap->split(LARGE_SIZE * 3 / 4);
        end = std::chrono::high_resolution_clock::now();
        treap_end += std::chrono::duration<double, std::milli>(end - start).count();
    }

    // Calculate averages and print results
    tree_begin /= NUM_RUNS;
    tree_middle /= NUM_RUNS;
    tree_end /= NUM_RUNS;
    avl_begin /= NUM_RUNS;
    avl_middle /= NUM_RUNS;
    avl_end /= NUM_RUNS;
    treap_begin /= NUM_RUNS;
    treap_middle /= NUM_RUNS;
    treap_end /= NUM_RUNS;

    std::cout << std::left << std::setw(12) << "Split Point" << std::setw(15) << "Tree"
              << std::setw(15) << "AVLTree" << std::setw(15) << "Treap"
              << "\n";
    std::cout << std::setw(12) << "Beginning" << std::setw(15) << format_time(tree_begin)
              << std::setw(15) << format_time(avl_begin) << std::setw(15)
              << format_time(treap_begin) << "\n";
    std::cout << std::setw(12) << "Middle" << std::setw(15) << format_time(tree_middle)
              << std::setw(15) << format_time(avl_middle) << std::setw(15)
              << format_time(treap_middle) << "\n";
    std::cout << std::setw(12) << "End" << std::setw(15) << format_time(tree_end) << std::setw(15)
              << format_time(avl_end) << std::setw(15) << format_time(treap_end) << "\n\n";
}

TEST_CASE("(legacy) `Treap` basic operations") {
    auto treap = std::make_unique<Treap<int, std::string>>();

    SUBCASE("Empty treap operations") {
        CHECK(treap->size() == 0);
        CHECK(treap->find(10) == nullptr);
        CHECK(treap->remove(10) == Status::FAILED);
    }

    SUBCASE("Insertion and find") {
        CHECK(treap->insert(10, "ten") == Status::SUCCESS);
        CHECK(treap->insert(20, "twenty") == Status::SUCCESS);
        CHECK(treap->insert(15, "fifteen") == Status::SUCCESS);
        CHECK(treap->insert(5, "five") == Status::SUCCESS);
        CHECK(treap->insert(30, "thirty") == Status::SUCCESS);

        // debug(treap);
        CHECK(treap->size() == 5);
        CHECK(treap->find(10) != nullptr);
        CHECK(treap->find(30)->value == "thirty");
        CHECK(treap->find(100) == nullptr);

        // Duplicate insertion
        CHECK(treap->insert(10, "TEN") == Status::FAILED);
    }

    SUBCASE("Removal") {
        treap->insert(10, "ten");
        treap->insert(20, "twenty");
        treap->insert(15, "fifteen");
        treap->insert(5, "five");
        treap->insert(30, "thirty");

        CHECK(treap->remove(15) == Status::SUCCESS);
        CHECK(treap->find(15) == nullptr);
        CHECK(treap->size() == 4);

        CHECK(treap->remove(5) == Status::SUCCESS);
        CHECK(treap->find(5) == nullptr);
        CHECK(treap->size() == 3);

        CHECK(treap->remove(100) == Status::FAILED);
    }

    SUBCASE("Split and merge") {
        for (int i = 1; i <= 10; ++i) {
            treap->insert(i, std::to_string(i));
        }
        auto right = treap->split(6);
        CHECK(treap->size() == 5);
        CHECK(right->size() == 5);
        for (int i = 1; i <= 5; ++i) CHECK(treap->find(i) != nullptr);
        for (int i = 6; i <= 10; ++i) CHECK(right->find(i) != nullptr);

        // Merge back
        CHECK(treap->merge(std::move(right)) == Status::SUCCESS);
        CHECK(treap->size() == 10);
        for (int i = 1; i <= 10; ++i) CHECK(treap->find(i) != nullptr);
    }
}

TEST_CASE("(legacy) `Treap` randomized stress test") {
    auto treap = std::make_unique<Treap<int, int>>();
    constexpr int N = 500;
    std::vector<int> keys(N);
    for (int i = 0; i < N; ++i) keys[i] = i;
    std::shuffle(keys.begin(), keys.end(), std::mt19937{std::random_device{}()});

    // Insert all
    for (int i = 0; i < N; ++i) {
        CHECK(treap->insert(keys[i], keys[i]) == Status::SUCCESS);
    }
    CHECK(treap->size() == N);

    // Remove half
    for (int i = 0; i < N; i += 2) {
        CHECK(treap->remove(keys[i]) == Status::SUCCESS);
    }
    CHECK(treap->size() == N / 2);

    // Check remaining
    /*for (int i = 0; i < N; ++i) {
        if (i % 2 == 0)
            CHECK(treap->find(keys[i]) == nullptr);
        else
            CHECK(treap->find(keys[i]) != nullptr);
    }
            */
}
}  // namespace legacy
