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

    std::vector<std::shared_ptr<SemNode>> &getAttachedNodes()
    {
        return mRelatedNodes; // TODO: hack for now...
    }

    virtual std::string toStr() const
    {
        return "";
    }

    // TODO: toUnderlyingType() could be useful - convert
    // SemNode into the actual type determined by mType

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

// Semantic node with a single position info.
class SemNodePositional : public SemNode
{
public:
    SemNodePositional(const uint32_t pos);

    uint32_t getPos() const;

private:
    uint32_t mPos;
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

    std::string toStr() const override
    {
        std::string str = mReturnType + " " + mName + " (";
        for (auto &it : mParams)
        {
            str += it.mType;
            str += " ";
            str += it.mName;
            str += ", ";
        }
        str += " )";
        return str;
    }

private:
    std::string mReturnType;
    std::vector<Param> mParams;
    std::string mName;
};

class SemNodeReturn final : public SemNodePositional
{
public:
    SemNodeReturn(const uint32_t index, std::shared_ptr<SemNode> rhs);

    std::string toStr() const override
    {
        std::string rhs;
        if (mRhs)
            rhs = mRhs->toStr();

        return rhs;
    }

private:
    std::shared_ptr<SemNode> mRhs;
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

    std::string toStr() const override
    {
        return mName;
    }

private:
    std::string mName;
};

class SemNodeConstant : public SemNodePositional
{
public:
    SemNodeConstant(const uint32_t pos, const std::string &name);

    std::string getName() const;

    std::string toStr() const override
    {
        return mName;
    }

private:
    std::string mName;
};

class SemNodeDeclaration : public SemNodePositional
{
public:
    SemNodeDeclaration( //
        const uint32_t pos,
        const std::string &lhsType,
        const std::string &lhsIdentifier);

    std::string getLhsType() const;
    std::string getLhsIdentifier() const;

    std::string toStr() const override
    {
        return mLhsType + " " + mLhsIdentifier;
    }

private:
    std::string mLhsType;
    std::string mLhsIdentifier;
};

class SemNodePostfixExpression : public SemNodePositional
{
public:
    SemNodePostfixExpression( //
        const uint32_t pos,
        const std::string &op,
        std::shared_ptr<SemNode> lhs);

    std::string getOperator() const;
    std::shared_ptr<SemNode> getLhs() const;

    void addArg(std::shared_ptr<SemNode> arg);

    std::string toStr() const override
    {
        std::string lhs = "empty";
        if (mLhs)
            lhs = mLhs->toStr();

        lhs += " (";

        if (mOperator == "(...)")
        {
            for (auto &args : mArgs)
            {
                lhs += args->toStr();
                lhs += ", ";
            }
        }

        lhs += ")";

        return lhs;
    }

private:
    std::string mOperator;
    std::shared_ptr<SemNode> mLhs;

    std::vector<std::shared_ptr<SemNode>> mArgs;
};

class SemNodeLoop : public SemNodePositional
{
public:
    SemNodeLoop(const uint32_t pos);

    void setIteratorInit(std::shared_ptr<SemNode> node);
    void setIteratorCondition(std::shared_ptr<SemNode> node);
    void setIteratorChange(std::shared_ptr<SemNode> node);

    std::shared_ptr<SemNode> getIteratorInit() const;
    std::shared_ptr<SemNode> getIteratorCondition() const;
    std::shared_ptr<SemNode> getIteratorChange() const;

    std::string toStr() const override;

private:
    std::shared_ptr<SemNode> mIteratorInit;
    std::shared_ptr<SemNode> mIteratorCondition;
    std::shared_ptr<SemNode> mIteratorChange;
};

class SemNodeEmptyStatement : public SemNodePositional
{
public:
    SemNodeEmptyStatement(const uint32_t pos);

    std::string toStr() const override
    {
        return "empty stmt";
    }
};

class SemNodeBinaryOp : public SemNodePositional
{
public:
    SemNodeBinaryOp(const uint32_t pos, const std::string &op, std::shared_ptr<SemNode> lhs);

    void setRhs(std::shared_ptr<SemNode> rhs);

    std::string getOp() const
    {
        return mOp;
    }

    std::shared_ptr<SemNode> getLhs() const;
    std::shared_ptr<SemNode> getRhs() const;

    std::string toStr() const override
    {
        std::string lhs = "empty";
        std::string rhs = "empty";
        if (mLhs)
            lhs = mLhs->toStr();
        if (mRhs)
            rhs = mRhs->toStr();
        return lhs + " " + mOp + " " + rhs;
    }

private:
    std::string mOp;

    std::shared_ptr<SemNode> mLhs;
    std::shared_ptr<SemNode> mRhs;
};

class SemNodeIf : public SemNodeScope
{
public:
    SemNodeIf(const uint32_t pos, std::shared_ptr<SemNode> cond);

    std::string toStr() const override
    {
        std::string cond = "empty";
        if (mCond)
            cond = mCond->toStr();
        return cond;
    }

private:
    std::shared_ptr<SemNode> mCond;
};

} // namespace safec
