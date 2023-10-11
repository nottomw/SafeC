![safec](https://github.com/nottomw/safec/actions/workflows/safec.yml/badge.svg)
![safec-tests](https://github.com/nottomw/safec/actions/workflows/safec-tests.yml/badge.svg)

## Safe C - safe(r) version of C language.

Safer version of C89 language - transpiles to C89.

### About

#### Main ideas
1. Code as similar to C89 as possible, so there is no need to learn too much of new syntax.
2. Code transpiled from SafeC to C89, so it can be handled by older compilers (like the ones used in embedded development).
3. Transpiled output code should be readable, so it doesn't get in the way when debugging.

### Main features
1. `defer`
    - partially done, see:
        [input file](https://github.com/nottomw/SafeC/blob/main/src/safec_testfiles/AST_defer.sc)
        [output file](https://github.com/nottomw/SafeC/blob/main/src/safec_testfiles/testfiles_generated_defer_ast/AST_defer.c)
2. References
3. NULL - optional automatic checks before dereference whenever possible
4. Ranged arrays - automatic checks
5. Type-safe defines / templates / generics 
6. Fixed width types (`uint32_t`, `int32_t`, ...) and `bool`
7. `static_assert`
9. `pragma once`
9. Changes for confusing syntax: `volatile`, `static`
...TBC

### Build
Requires: conan, cmake, ninja, clang++, flex, bison.

```
SafeC $ mkdir build
SafeC $ cd build
SafeC/build $ cmake -G Ninja -S ../src
SafeC/build $ ninja
```
