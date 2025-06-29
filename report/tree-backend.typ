#import "meta.typ": *
== 算法

语言：C++

使用 template 实现对于 `Key, Value` 的泛型支持

使用 CRTP & Mixin 实现代码复用

最大程度上复用代码的同时实现了多种平衡算法：`BasicTree`, `AVLTree`, `Treap`, `SplayTree`

三层类结构，逐级擦除类型信息

`TreeAdapter<K, V, Impl> : Tree<K, V> : TreeBase`

使用 `Tree<K, V>` 时不需要关心内部用的什么算法实现平衡树

使用 `TreeBase` 时不需要关心内部的 `key, value` 是什么类型

---

#figure(image("assets/tree-hierarchy.png"), caption: [Tree 类图，实线继承，虚线组合])

---

=== 基础操作实现

例：实现 `find` (`trait::node::Search, trait::Search`)

```cpp
template <typename Node> struct Search {
    auto find(auto&& key) {
        // ...
    }
};
template <typename Tree> struct Search {
    auto find(auto&& key) {
        auto&& root = static_cast<const Tree*>(this)->root;
        return root ? root->find(key) : nullptr;
    }
};
```

Define once, use everywhere.

```cpp
/// @pseudocode
struct BasicTreeImpl : Search<BasicTreeImpl>;
struct AVLTreeImpl : Search<AVLTreeImpl>;
struct TreapImpl : Search<TreapImpl>;
```

---

例：实现可提供不同功能的 `maintain()` (`trait::node::Maintain`)

```cpp
template <typename Node> struct Height {
    int height{1};
    void maintain() {
        auto& self = *(static_cast<Node*>(this));
        auto l = self.child[L] ? self.child[L]->height : 0;
        auto r = self.child[R] ? self.child[R]->height : 0;
        self.height = 1 + std::max(l, r);
    }
};
template <typename Node> struct Size {
    size_t size{1};
    void maintain() {
        auto& self = *(static_cast<Node*>(this));
        auto l = self.child[L] ? self.child[L]->size : 0;
        auto r = self.child[R] ? self.child[R]->size : 0;
        self.size = 1 + l + r;
    }
};
```

---

合并不同的属性，创建一个自动维护所有属性的 `maintain()` 方法

```cpp
// helper trait to maintain multiple properties
template <typename... Ts> struct Maintain : Ts... {
    void maintain() { (Ts::maintain(), ...); }
};
```

根据需要维护的属性，继承不同的 `Maintain` 特性

```cpp
/// @pseudocode

// imports a maintain() that maintains size
struct BasicNode : Maintain<Size<BasicNode>>;
// imports a maintain() that maintains both size and height
struct AVLNode : Maintain<Size<AVLNode>, Height<AVLNode>>;
```

---

=== 旋转实现 (`trait::Rotate`)

`bind, unbind` 控制子树的挂载和拆卸，`moveNode` 移动节点

这两个辅助函数都会记录下当前森林的状态到 `trace` 中

具体旋转的方法不再赘述

```cpp
template <typename Tree> struct Rotate {
    void rotate(int dir, auto& root) {
        auto& self = *static_cast<Tree*>(this);
        auto new_root = self.unbind(root, dir ^ 1);
        if (new_root->child[dir]) {
            self.bind(root, dir ^ 1, self.unbind(new_root, dir));
        }
        auto parent = root->parent;
        self.bind(new_root, dir, std::move(root));
        self.moveNode(root, std::move(new_root), parent);
        root->child[dir]->maintain();
        root->maintain();
    }
    void rotateL(auto& root) { return rotate(L, root); }
    void rotateR(auto& root) { return rotate(R, root); }
    void rotateLR(auto& root) {
        rotateL(root->child[L]), rotateR(root);
    }
    void rotateRL(auto& root) {
        rotateR(root->child[R]), rotateL(root);
    }
};
```

---

=== AVL树的 `split` 和` join` (`AVLTree::{join, split}`)

- 先实现 `join(left_tree, sperator_node, right_tree)`： 给定 key 值不交的两棵 AVL树和一个key值在两树之间的分界点节点，合并成一棵树

  考虑 $"height"_"left">="height"_"right"$ 的情况，反之对称

  在左树中找到高度为 $h_"right"$ 或 $h_"right"+1$ 的点 `cut_tree`，由于左树是AVL树，一定能找到

  将`cut_tree`和 `right_tree` 挂到`seperator_node`上，然后放回原先的位置

  高度最多改变 $1$，从`cut_tree`位置向上维护平衡即可。

  时间复杂度：$O(abs(h_"left" - h_"right"))$

#pause

- `join(left_tree, right_tree)`：

  删除 `left_tree.max()` 或 `right_tree.min()`，转换为带 seperator 的 `join`

---

#grid(
  columns: (1fr, 0.5fr),
  [
    - `split(tree, key)`

      如右图所示，在 `find(key)` 的路径上的位置将节点和它的左右子树分开

      然后自底向上合并，每一次合并用路径中的点（图中的$P_i$）作为 seperator 合并两子树

      $
        alpha P & <-- "join"(alpha, text(fill: #red, P)) \
        beta P_8 beta_8 & <-- "join"(beta, text(fill: #red, P_8), beta_8) \
        alpha_7 P_7 alpha P & <-- "join"(alpha_7, text(fill: #red, P_7), alpha P) \
        ... & <-- ...
      $

      每一次合并的复杂度是高度差，高度差之和不超过总高度，所以复杂度为 $O(log n)$
  ],
  [
    #figure(pad(left: 1em, image("assets/split.jpeg")), caption: [split演示. ref.TAOCP])
  ],
)

== 实现细节

- 基类接口，提供类型无关的方法：

```cpp
struct TreeBase {
    virtual ~TreeBase() = default;
    virtual auto size() const -> size_t = 0;
    virtual void clear() = 0;
    virtual auto view() const -> ForestView = 0;
    virtual auto trace() -> std::vector<ForestView> = 0;
    virtual auto trace(const std::function<void()>& func) -> std::vector<ForestView> = 0;
    virtual void traceStart() = 0;
    virtual void traceStop() = 0;
    virtual void printCLI() const = 0;
    virtual auto stringify() const -> std::string = 0;
    virtual auto name() const -> std::string = 0;
};
```
---

- 在 `TreeBase` 的基础上定义 `Tree` 接口，提供需要 `key,value` 类型信息的方法

```cpp
template <typename K, typename V> struct Tree : TreeBase {
    virtual auto find(const K& key) -> Pair<const K, V>* = 0;
    virtual auto findKth(size_t rank) -> Pair<const K, V>* = 0;
    virtual auto min() -> Pair<const K, V>* = 0;
    virtual auto max() -> Pair<const K, V>* = 0;
    virtual auto insert(const K& key, const V& value) -> Status = 0;
    virtual auto remove(const K& key) -> Status = 0;
    virtual void traverse(const std::function<void(const K&, V&)>& func) = 0;
    virtual auto operator[](const K& key) -> V& = 0;
    virtual auto operator[](const K& key) const -> const V& = 0;
};
```

---

- 通过 `TreeAdapter` 绑定具体实现到 `Tree` 接口上

`TreeAdapter` 拥有一个 `Impl` 对象，通过转发 `Tree` 接口的方法到 Impl 对象上来实现具体的树算法

- 对于 Algorithm-dependent 的方法，`TreeAdapter` 只需要转发调用到 `impl` 上，不需要重载

```cpp
template <typename K, typename V, template <typename, typename> typename Impl>
struct TreeAdapter : Tree<K, V> {
    // @override
    auto size() const -> size_t override { return impl->size(); }
    auto view() const -> ForestView override { return impl->view(); }
    ...

    // @no-override
    auto split(const K& k) -> std::unique_ptr<TreeAdapter> {
        return std::make_unique<TreeAdapter>(impl->split(k));
    }
    auto join(std::unique_ptr<TreeAdapter> other) -> Status {
        return impl->join(std::move(other->impl));
    }
    auto merge(std::unique_ptr<TreeAdapter> other) -> Status {
        return impl->merge(std::move(other->impl));
    }
    ...

    std::unique_ptr<Impl<K, V>> impl;
};
```

将算法通过模版参数 `Impl` 传入 `TreeAdapter`，进而能统一使用 `Tree` 接口控制不同类型的树

```cpp
template <typename K, typename V>
using BasicTree = TreeAdapter<K, V, BasicTreeImpl>;
template <typename K, typename V>
using AVLTree = TreeAdapter<K, V, AVLTreeImpl>;
template <typename K, typename V>
using SplayTree = TreeAdapter<K, V, SplayTreeImpl>;
template <typename K, typename V>
using Treap = TreeAdapter<K, V, TreapImpl>;
```

使用举例：

```cpp
std::vector<std::unique_ptr<Tree<int, int>>> trees;
trees.push_back(std::make_unique<BasicTree<int, int>>());
trees.push_back(std::make_unique<AVLTree<int, int>>());
trees.push_back(std::make_unique<SplayTree<int, int>>());
trees.push_back(std::make_unique<Treap<int, int>>());
// operations...
for (auto& tree : trees) {
    auto trace = tree->trace(); // 输出每一步后的状态
    printTrace(trace);
}
```

---

- Impl 的实现：每一种 `Tree` 都从 `trait::` 中选取使用的特性继承，比如：AVL/Splay使用旋转特性

AVLNode 需要维护 Height, 而普通 Node 不需要，\
所以分别继承 `Maintain<Size<AVLNode>, Height<AVLNode>>` 和 `Maintain<Size<Node>>`

```cpp
template <typename K, typename V>
struct BasicNode : Pair<const K, V>,
                   trait::node::TypeTraits<K, V>,
                   trait::node::Link<BasicNode<K, V>>,
                   trait::node::View<BasicNode<K, V>>,
                   trait::node::Maintain<trait::node::Size<BasicNode<K, V>>>,
                   trait::node::Search<BasicNode<K, V>> {

    BasicNode(const K& k, const V& v, BasicNode* parent = nullptr)
        : Pair<const K, V>(k, v), trait::node::Link<BasicNode<K, V>>(parent) {
        this->maintain();
    }
};
```

---

```cpp
template <typename K, typename V>
struct AVLNode
    : Pair<const K, V>,
      trait::node::TypeTraits<K, V>,
      trait::node::Link<AVLNode<K, V>>,
      trait::node::View<AVLNode<K, V>>,
      trait::node::Maintain<trait::node::Size<AVLNode<K, V>>, trait::node::Height<AVLNode<K, V>>>,
      trait::node::Search<AVLNode<K, V>> {

    AVLNode(const K& k, const V& v, AVLNode* parent = nullptr)
        : Pair<const K, V>(k, v), trait::node::Link<AVLNode<K, V>>(parent) {
        this->maintain();
    }
};
```

几乎不用写任何附加代码就完成了两种节点的创建

---

- 可以看到每一个基类模版的参数中都要写派生类比较麻烦，构造树时由于 `trait` 比较多，这个问题更加明显，写一个 `Mixin` 辅助模版类减少CRTP重复的派生类声明

```cpp
/// @struct Mixin
/// @brief simply mixin multiple traits into a single type, Mixin<T, A, B> serves as A<T>, B<T>
template <typename Type, template <typename> class... Traits> struct Mixin : Traits<Type>... {};
```

```cpp

template <typename K, typename V>
struct AVLTreeImpl
    : trait::Mixin<AVLNode<K, V>, trait::TypeTraits, trait::Maintain>,
      trait::Mixin<
          AVLTreeImpl<K, V>, trait::InsertRemove, trait::Search, trait::Clear,
          trait::Size, trait::Height, trait::Print, trait::Traverse, trait::Merge,
          trait::Subscript, trait::Conflict, trait::Box, trait::Detach, trait::View,
          trait::Trace, trait::TracedBind, trait::TracedConstruct, trait::Rotate,
          trait::Iterate> {
    // ...
};
```

---

=== 内存管理

使用 `std::unique_ptr` 管理节点所有权，防止内存泄露或者 `double free` 问题

```cpp
template <typename Node> struct Link {
    Node* parent{nullptr};
    std::unique_ptr<Node> child[2]{nullptr, nullptr};
    Link(Node* parent = nullptr) : parent(parent) {}
};
```

移交所有权时使用 `std::move()`，可读性与安全性都很好

#pause

=== Trace 记录 (`trait::Trace`)

- 在结构体中放一个 `std::vector<ForestView> record;` 记录每一步操作之后的状态

  所有对树结构的操作都通过调用 `bind(), unbind()` 方法，内部自动维护以及记录trace

#pause

- 记录 trace 的方法：

  维护当前森林的根节点列表 `std::set<Node*> entries;`

  每作一次记录 `snapshot()` 就复制出 `entries` 对应每一颗树中的信息，保存至 `record`


---

#grid(
  columns: (0.4fr, 0.6fr),
  [
    === 性能优化

    最开始的结构：

    - 非常容易想到
    #figure(image("assets/traditional-node-hierarchy.png"))
  ],
  [
    #figure(image("assets/traditional-tree-hierarchy.png", width: 80%))
  ],
)

---

但
- 以`maintain()`为例，每一次自底向上维护信息时，都需要调用 `node::maintain()`，然而这是一个虚函数，没法内联，每一次调用都有额外开销

#pause

- Tree中只会存一个基类的 `Node` 指针，每一次使用子类 `Node` 特有信息时都需要 `static_cast` 或者 `dynamic_cast`

#pause

- 拓展功能比较麻烦
  - `Splay` 和 `AVL` 都可以旋转，要么实现两遍，要么创建一个 `RotatableTree`，增加继承层级
  - 更好的想法应该是把 `Rotate` 抽出来作为一个只提供旋转功能的 trait
  - #emph[既然如此，为什么不把所有的功能都抽出来？]

---

*重构!*

- 所有树平级，\
  复用的功能只由 trait 提供

- 由于不同的树之间没有子类型关系，需要一个 `TreeAdapter` 来绑定到相同的接口上

#figure(image("assets/tree-hierarchy.png"))

---

查看是否内联：

- 重构前

  ```cpp
  $ nm | c++filt
  000000010001e7c8 legacy::AVLTree<int, int>::AVLNode::maintain()
  000000010001c4ec legacy::Tree<int, int>::Node::maintain()
  ```

  只有 `Tree::maintain()` 被内联了，Node 本身的 `maintain()` 没有被内联

- 重构后：

  ```cpp
  $ nm | c++filt
  0000000100020578 AVLNode<int, int>::stringify() const
  000000010001fe4c AVLNode<int, int>::~AVLNode()
  ```

```
Tree                       Insert         Find       Remove
legacy::AVLTree(ms)         48.40        11.65        49.86
AVLTree(ms)                 32.35        10.21        41.70
std::map(ms)                25.09        11.58        30.74

CRTP Improvement(%)         33.15        12.32        16.35
```

#figure(image("assets/benchmark-insert.png", width: 60%), caption: [benchmark: insert])

可以看到重构后提升还是很显著的

完整代码：#meta.repo

== 单元测试

整个项目十分复杂，因此为每个功能写了测试

使用 `doctest` 库进行单元测试

```bash
$ build/balanced_tree test
[doctest] doctest version is "2.4.12"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:   26 |   26 passed | 0 failed | 0 skipped
[doctest] assertions: 6854 | 6854 passed | 0 failed |
[doctest] Status: SUCCESS!
```

测试举例

```cpp
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
```

