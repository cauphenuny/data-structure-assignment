# Balanced Tree Visualizer

```bash
commands:
    [q]uit
    [h]elp

    [c]reate <tree-id: a-z|A-Z> <algo: basic|avl|treap|splay>
    [d]elete <tree-id>
    [p]rint <tree-id>*
    [l]ist

    [i]nsert <tree-id> <key: int> <value: int>
    [r]emove <tree-id> <key: int>
    [f]ind <tree-id> <key: int>

    [s]plit <dest-id> <src-id> <key: int>
    [m]erge <dest-id> <src-id>

    [R]andom_insert <tree-id> <count: int>
    [S]equential_insert <tree-id> <start: int> <end: int>

trace mode:
    [n]: next
    [c]: auto continue

examples:
>>> c T treap
>>> i T 1 1
>>> i T 2 2
>>> S T 1 10
>>> p

>>> A = avl.create()
>>> A.SequentialInsert(1, 10)
>>> B = A.split(5)
>>> B.print
>>> print
>>> B.merge(A)
>>> print
>>> delete(B)
```

---

Contributors:

- Backend: @[Cauphenuny](https://github.com/cauphenuny)

- CLI: @[Cauphenuny](https://github.com/cauphenuny)

- GUI: @[Naiq](https://github.com/Naiq-zyx)
