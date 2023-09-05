#pragma once

#include "SemNodeEnumeration.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace safec
{

class SemNodeWalker;

// clang-format off
#define SEMNODE_TYPE_SELECTOR_VALUE(x) x,
#define SEMNODE_TYPE_SELECTOR_VALUE_TO_STR(x)   \
    case Type::x: {                             \
        return #x;                              \
    }                                           \
    break;
// clang-format on

class SemNode
{
public:
    enum class Type : uint32_t
    {
        SEMNODE_TYPE_ENUMERATE(SEMNODE_TYPE_SELECTOR_VALUE)
    };

    struct TypeInfo
    {
        static constexpr std::string_view toStr(const Type type)
        {
            switch (type)
            {
                SEMNODE_TYPE_ENUMERATE(SEMNODE_TYPE_SELECTOR_VALUE_TO_STR)
            }

            return "undefined";
        }
    };

    SemNode();
    SemNode(const Type type);
    virtual ~SemNode() = default;
    void reset();
    Type getType() const;
    void attach(std::shared_ptr<SemNode> node);

protected:
    Type mType;
    std::vector<std::shared_ptr<SemNode>> mRelatedNodes;

    friend class SemNodeWalker;
};

// clang-format off
class SemNodeUndefined : public SemNode{};
// clang-format on

class SemNodeTranslationUnit : public SemNode
{
public:
    SemNodeTranslationUnit();
};

// Semantic node with dual position info (start & end).
class SemNodeScope : public SemNode
{
public:
    SemNodeScope(const uint32_t start);
    void setEnd(const uint32_t end);
    uint32_t getStart() const;
    uint32_t getEnd() const;

protected:
    uint32_t mStartIndex;
    uint32_t mEndIndex;
};

class SemNodeFunction final : public SemNodeScope
{
public:
    struct Param
    {
        Param() = default;

        Param(const std::string &type, const std::string &name)
            : mType{type}
            , mName{name}
        {
        }

        std::string mType;
        std::string mName;
    };

    SemNodeFunction(const uint32_t start);

    void setName(const std::string &name);
    void setReturn(const std::string &type);
    void addParam(const std::string &type, const std::string &name);

    std::string getName() const;
    std::string getReturn() const;
    std::vector<Param> getParams() const;

private:
    std::string mReturnType;
    std::vector<Param> mParams;
    std::string mName;
};

// Semantic node with a single position info.
class SemNodePositional : public SemNode
{
public:
    SemNodePositional(const uint32_t pos);

    uint32_t getPos() const;

private:
    uint32_t mPos;
};

class SemNodeDefer final : public SemNodePositional
{
public:
    SemNodeDefer(const uint32_t index);

    void setDeferredText(std::string &&deferredText);
    std::string getDeferredText() const;

    void setDeferredStatementLen(const uint32_t len);
    uint32_t getDeferredStatementLen() const;

private:
    std::string mDeferredText;
    uint32_t mDeferredStatementLen;
};

class SemNodeReturn final : public SemNodePositional
{
public:
    SemNodeReturn(const uint32_t index);
};

class SemNodeLoop : public SemNodeScope
{
public:
    SemNodeLoop(const uint32_t start);
};

class SemNodeBreak : public SemNodePositional
{
public:
    SemNodeBreak(const uint32_t pos);
};

class SemNodeContinue : public SemNodePositional
{
public:
    SemNodeContinue(const uint32_t pos);
};

class SemNodeIdentifier : public SemNodePositional
{
public:
    SemNodeIdentifier(const uint32_t pos, const std::string &name);

    std::string getName() const;

private:
    std::string mName;
};

class SemNodeReference : public SemNodeIdentifier
{
public:
    SemNodeReference(const uint32_t pos, const std::string &name);
};

class SemNodeDeclaration : public SemNodePositional
{
public:
    SemNodeDeclaration( //
        const uint32_t pos,
        const std::string &lhsType,
        const std::string &lhsIdentifier);

    // TODO: right-hand side might be an (unary)expression (= (other_var + 1234))
    // TODO: rhs might be a literal
    void setRhs(const std::string &rhsIdentifier);

    std::string getLhsType() const;
    std::string getLhsIdentifier() const;
    std::string getRhsIdentifier() const;

private:
    std::string mLhsType;
    std::string mLhsIdentifier;

    std::string mRhsIdentifier;
};

class SemNodeAssignment : public SemNodePositional
{
public:
    // TODO: rhs can be an expression
    SemNodeAssignment( //
        const uint32_t pos,
        const std::string &op,
        const std::string &lhs,
        const std::string &rhs);

    std::string getOperator() const;
    std::string getLhs() const;
    std::string getRhs() const;

private:
    std::string mOperator;
    std::string mLhs;
    std::string mRhs;
};

} // namespace safec
