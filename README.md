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
- [ ] begin block
- [x] function
  - [x] handle nodes as body
- [x] macro
  - [x] handle nodes as body
- [x] atom
  - [x] number
  - [x] string
  - [x] boolean
  - [x] nil
  - [x] symbol
- [x] comment
- [x] function calls
- [ ] identifiers
  - [ ] symbol
  - [ ] capture
  - [ ] dot notation

Error context generation:
```
ERROR
Missing closing paren after node
At l @ 0:2
    1 | (let aaaaaaa 1)
      |   ^
    2 | 
    3 | # test comment
```
