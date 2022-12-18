# Parser combinators experiment

An experiment with parser combinators, to replace the current lexer/parser of [ArkScript](https://github.com/ArkScript-lang/Ark).

This project has multiple goals:
- making a more extensible parser than the current one Ark has;
- removing weird edge cases the current parser has ;
- reducing the number of bugs the parser has ;
- easier generation of error contexts

## Building

You need CMake >= 3.24 and a C++17 capable compiler (eg Clang 14).

```shell
cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug
cmake --build build

build/parser <filename>
```

## Current state

Subparsers:
- [x] let, mut, set
  - [x] handle nodes as values
- [x] del
- [x] condition
  - [x] handle nodes as condition
  - [x] handle nodes as values
- [x] loop
  - [x] handle nodes as conditions
  - [x] handle nodes as body
- [x] import
- [x] begin block
- [x] function
  - [x] handle nodes as body
- [x] macro
  - [x] handle nodes as body
- [x] atom
  - [x] number
  - [x] string
    - [ ] handle `\uxxxxx`, `\Uxxxxx`, `\xabc` in strings
    - [x] handle other espace sequences: n, r, t, a, b, f, 0, \, "
  - [x] boolean
  - [x] nil
  - [x] symbol
- [x] comment
  - [x] comments in blocks and not only top level ones
- [x] function calls
  - [x] anonymous calls: ((fun () (print 1)))
- [x] identifiers
  - [x] symbol
  - [x] capture
  - [x] dot notation
    - [ ] dot notation after call: (@ list 14).field
  - [x] non alnum identifiers (`+`, `!=`, `>=`...)
- [x] special syntax for (list ...): [...]

Error context generation:
```
ERROR
Package name expected after '.'
At ' ' @ 1:12
    1 | (import a. )
      |           ^
```

## Breaking changes

This is for ArkScript, but some things had to change for the next version of the language, implemented by this parser.

- quote is no longer supported, use functions with no arguments instead
- import do not work the same way as before: `(import "path.ark")` won't work, we are using a package like syntax now:
```lisp
(import a)
(import a.b)  # everything is prefixed by b
(import foo.bar.egg)
(import foo:*)  # everything is imported in the current scope
(import foo.bar :a :b)  # we import only a and b from foo.bar, in the current scope
```
- fields aren't chained in the AST: `(Symbol:a GetField:b GetField:c)` was the old way of having a `a.b.c` in the AST, now we have `Field(Symbol:a Symbol:b Symbol:c)`, the node holding the field being a list of symbols
