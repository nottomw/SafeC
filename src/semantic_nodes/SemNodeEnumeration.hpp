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
        selector(Return) \
        selector(JumpStatement) \
        selector(Declaration) \
        selector(PostfixExpression) \
        selector(EmptyStatement) \
        selector(BinaryOp) \
        selector(Identifier) \
        selector(Constant) \
        selector(If) \
        selector(UnaryOp) \
        selector(InitializerList) \
        selector(Group) \
        selector(Defer) \
        selector(SwitchCase) \
        selector(SwitchCaseLabel)
// clang-format on
