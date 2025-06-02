#include "avl.hpp"
#include "tree.hpp"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

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

    constexpr static auto join = [](auto& tree1, auto tree2) {
        return tree1->join(std::move(tree2));
    };
    constexpr static auto mixin = [](auto& tree1, auto tree2) {
        return tree1->mixin(std::move(tree2));
    };
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

TEST_CASE("`Tree` conflict, join, mix, merge") {
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

TEST_CASE("`Tree` operator[]") {
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

TEST_CASE("`AVLTree` insertion") {
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

TEST_CASE("`AVLTree` removal") {
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

TEST_CASE("`AVLTree` join operation") {
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
}
