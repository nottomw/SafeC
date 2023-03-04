#include "WalkerReference.hpp"

#include "logger/Logger.hpp"
#include "semantics/SemNode.hpp"

#include <iostream>

namespace safec
{

// TODO: reference walker implement
//      1) program structure - need to scope references
//          - reference valid only in scope
//          - same as in defer - probably need to extract "program structure"
//            code to a separate module - TODO: refactor ProgramElem & program structure
//      2) replace reference declarations:
//          - int &ref          -> int *const ref
//          - int &ref = *ptr   -> int *const ref = ptr
//      3) track references in scope, if reference used - replace operators
//          - &ref              -> ref
//          - ref = something   -> *ref = something
//          - lvalue = ref      -> lvalue = *ref
//          - ref1 = ref2       -> error (reassignment)
//          - ref1 = *ptr       -> error (reassignment)
//          - ref = 5           -> error
//          - *ref              -> error
//      4) track function calls with reference parameters
//         e.g.: fun(int a, int &ref, const int &constRef))
//          - fun(5, someVar, someVar2)     -> fun(5, &someVar, &someVar2)
//          - fun(5, someVar, 7)            -> temporary for '7'
//          - fun(5, 6, 7)                  -> error (ref non-const)

void WalkerReference::peek(SemNode &node, const uint32_t astLevel)
{
}

void WalkerReference::peek(SemNodePositional &node, const uint32_t astLevel)
{
}

void WalkerReference::peek(SemNodeScope &node, const uint32_t astLevel)
{
}

void WalkerReference::peek(SemNodeFunction &node, const uint32_t astLevel)
{
}

void WalkerReference::peek(SemNodeDefer &node, const uint32_t astLevel)
{
}

void WalkerReference::peek(SemNodeLoop &node, const uint32_t astLevel)
{
}

void WalkerReference::peek(SemNodeReturn &node, const uint32_t astLevel)
{
}

void WalkerReference::peek(SemNodeBreak &node, const uint32_t astLevel)
{
}

void WalkerReference::peek(SemNodeContinue &node, const uint32_t astLevel)
{
}

void WalkerReference::peek(SemNodeReference &node, const uint32_t astLevel)
{
}

} // namespace safec
