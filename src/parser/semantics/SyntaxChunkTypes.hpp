#pragma once

// clang-format off

// TODO: duplicate with SEMNODE_TYPE_SELECTOR_VALUE, move to common header
#define SYNTAXCHUNKTYPE_LIST(x) x,
#define SYNTAXCHUNKTYPE_TOSTR(x)   \
    case SyntaxChunkType::x: {                             \
        return #x;                              \
    }                                           \
    break;

#define SYNTAXCHUNKTYPE_ENUM(entry) \
    entry(kUnknown) \
    entry(kFunctionHeader) \
    entry(kFunction) \
    entry(kStructOrUnionDecl) \
    entry(kEnumDecl) \
    entry(kType) \
    entry(kDirectDecl) \
    entry(kPointer) \
    entry(kConstant) \
    entry(kIdentifier) \
    entry(kInitDeclaration) \
    entry(kAssignment) \
    entry(kAssignmentOperator) \
    entry(kConditionHeader) \
    entry(kConditionExpression) \
    entry(kCondition) \
    entry(kForLoopHeader) \
    entry(kForLoopConditions) \
    entry(kForLoop) \
    entry(kRelationalExpression) \
    entry(kPostfixExpression)

// clang-format on

enum class SyntaxChunkType : uint32_t
{
    SYNTAXCHUNKTYPE_ENUM(SYNTAXCHUNKTYPE_LIST)
};

static constexpr std::string_view syntaxChunkTypeToStr(const SyntaxChunkType type)
{
    switch (type)
    {
        SYNTAXCHUNKTYPE_ENUM(SYNTAXCHUNKTYPE_TOSTR)
    }

    return "undefined chunk type";
}
