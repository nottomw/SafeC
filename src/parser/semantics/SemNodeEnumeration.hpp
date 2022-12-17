#pragma once

// X-Macro used to generate a lot of internal code.
// WARNING: these names have to be strictly consistent with class
// names e.g. SemNode{EnumValueName}.
// clang-format off
#define SEMNODE_TYPE_ENUMERATE(selector) \
        selector(Undefined) \
        selector(TranslationUnit) \
        selector(Loop) \
        selector(Scope) \
        selector(Function) \
        selector(RawText) \
        selector(Defer) \
        selector(Return) \
        selector(Break) \
        selector(Continue)
// clang-format on
