# inline-optimize

`nm | c++filt`:

```cpp
000000010001e7c8 legacy::AVLTree<int, int>::AVLNode::maintain()
000000010001e448 legacy::AVLTree<int, int>::AVLNode::stringify() const
000000010001e384 legacy::AVLTree<int, int>::AVLNode::~AVLNode()
000000010001e3e4 legacy::AVLTree<int, int>::AVLNode::~AVLNode()
000000010001c4ec legacy::Tree<int, int>::Node::maintain()
000000010001c2c0 legacy::Tree<int, int>::Node::stringify() const
000000010001c1fc legacy::Tree<int, int>::Node::~Node()
000000010001c25c legacy::Tree<int, int>::Node::~Node()
000000010001a5bc legacy::Tree<int, int>::clear()
000000010001b204 legacy::Tree<int, int>::insert(int const&, int const&)
000000010001b630 legacy::Tree<int, int>::merge(std::__1::unique_ptr<legacy::Tree<int, int>, std::__1::default_delete<legacy::Tree<int, int>>>)
000000010001b844 legacy::Tree<int, int>::name() const
000000010001a5dc legacy::Tree<int, int>::print() const
000000010001a5e0 legacy::Tree<int, int>::printCLI() const
000000010001b2c8 legacy::Tree<int, int>::remove(int const&)
000000010001a5a4 legacy::Tree<int, int>::size() const
000000010001b528 legacy::Tree<int, int>::split(int const&)
000000010001aff4 legacy::Tree<int, int>::stringify() const
000000010001af60 legacy::Tree<int, int>::~Tree()
000000010001afa8 legacy::Tree<int, int>::~Tree()
```

Only `Tree::maintain()` is optimized.

Calling `start->maintain();` in `Tree::maintain()` requires a virtual call every time.

```cpp
0000000100020578 AVLNode<int, int>::stringify() const
000000010001fe4c AVLNode<int, int>::~AVLNode()
0000000100022658 AVLTreeImpl<int, int>::balance(std::__1::unique_ptr<AVLNode<int, int>, std::__1::default_delete<AVLNode<int, int>>>&)
0000000100022458 AVLTreeImpl<int, int>::checkBalance(AVLNode<int, int>*)
0000000100028858 AVLTreeImpl<int, int>::join(std::__1::unique_ptr<AVLNode<int, int>, std::__1::default_delete<AVLNode<int, int>>>, std::__1::unique_ptr<AVLTreeImpl<int, int>, std::__1::default_delete<AVLTreeImpl<int, int>>>)
0000000100028cc8 AVLTreeImpl<int, int>::join(std::__1::unique_ptr<AVLTreeImpl<int, int>, std::__1::default_delete<AVLTreeImpl<int, int>>>)
0000000100022930 AVLTreeImpl<int, int>::remove(int const&)
00000001000282f8 AVLTreeImpl<int, int>::split(int const&)
000000010001ffec AVLTreeImpl<int, int>::stringify() const
```

Both `AVLNode::maintain()` and `AVLTreeImpl::maintain()` is inline optimized.

Benchmark:

```
Tree                       Insert         Find       Remove
legacy::AVLTree(ms)         47.67        11.98        46.97
AVLTree(ms)                 29.78        10.78        41.85
std::map(ms)                22.46        11.74        36.41

CRTP Improvement(%)         37.52         9.96        10.89
```
