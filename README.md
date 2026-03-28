# libmgcregex
library for pattern matching with extended POSIX regular expressions.

Utilizes Thompsons Construction to implement a Linked Digraph representation
of a non-deterministic finite automaton described by the supplied regular expression.

Linear-time non-backtracking pattern matching using Power Set construction algorithm.

RE -> AST -> NFA

## build
```
      make
      sudo make install
      sudo ldconfig
      make clean
```

## use

```
    gcc prog.c -o prog -lmgcregex
```