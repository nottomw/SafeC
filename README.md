## Safe C - safe(r) version of C language.

Safer version of C89 language - transpiles to C89.

### About

#### Main ideas
1. Code as similar to C89 as possible, so there is no need to learn too much of new syntax.
2. Code transpiled from SafeC to C89, so it can be handled by older compilers (like the ones used in embedded development).
3. Transpiled output code should be readable, so it doesn't get in the way when debugging.

### Main features
1. `defer`
2. References
3. Pointer ownership indication (and enforcement)
4. NULL - optional automatic checks before dereference whenever possible
5. Ranged arrays - automatic checks
6. Integer overflow checks
7. Type-safe defines
8. Fixed width types (uint32_t, int32_t, ...) and booleans
9. static_assert
10. Templates/generics - type safe defines (?)
11. pragma once
12. Changes for confusing syntax: `volatile`, `static`
...TBC

### Build
Requires: conan, cmake, ninja, clang++.

```
SafeC $ mkdir build
SafeC $ cd build
SafeC/build $ cmake -G Ninja -S ../src
SafeC/build $ ninja
```
