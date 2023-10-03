#pragma once

#include "SemNodeEnumeration.hpp"

#include <cassert>
#include <filesystem>
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
// clang-format on

class SemNode
{
public:
    enum class Type : uint32_t
    {
        SEMNODE_TYPE_ENUMERATE(SEMNODE_TYPE_SELECTOR_VALUE)
    };

    SemNode();
    SemNode(const Type type);
    virtual ~SemNode() = default;

    void reset();

    Type getType() const;
    std::string_view getTypeStr() const;

    void attach(std::shared_ptr<SemNode> node);

    std::vector<std::shared_ptr<SemNode>> &getAttachedNodes()
    {
        return mRelatedNodes;
    }

    virtual std::string toStr() const
    {
        return "";
    }

    void setSemStart(const uint32_t i);
    void setSemEnd(const uint32_t i);

    uint32_t getSemStart() const;
    uint32_t getSemEnd() const;

protected:
    Type mType;
    std::vector<std::shared_ptr<SemNode>> mRelatedNodes;

    uint32_t mSemStart;
    uint32_t mSemEnd;

    friend class SemNodeWalker;
};

// clang-format off
class SemNodeUndefined : public SemNode{};
// clang-format on

class SemNodeTranslationUnit : public SemNode
{
public:
    SemNodeTranslationUnit();

    void setSourcePath(const std::filesystem::path &path);
    std::filesystem::path getSourcePath() const;

private:
    std::filesystem::path mSourcePath;
};

// Semantic node with dual position info (start & end).
class SemNodeScope : public SemNode
{
public:
    SemNodeScope(const uint32_t start);
    void setEnd(const uint32_t end);
    uint32_t getStart() const;
    uint32_t getEnd() const;

    void devourAttachedNodesFrom(std::shared_ptr<SemNodeScope> node);

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

class SemNodeGroup : public SemNodePositional
{
public:
    SemNodeGroup(const uint32_t pos);

private:
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

    std::string toStr() const override;

private:
    std::string mReturnType;
    std::vector<Param> mParams;
    std::string mName;
};

class SemNodeReturn final : public SemNodePositional
{
public:
    SemNodeReturn(const uint32_t index);
    SemNodeReturn(const uint32_t index, std::shared_ptr<SemNode> rhs);

    std::shared_ptr<SemNode> getReturnedNode() const;

    std::string toStr() const override;

private:
    std::shared_ptr<SemNode> mRhs;
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

    void appendToType(const std::string &str);

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

    std::string toStr() const override;

private:
    std::string mOperator;
    std::shared_ptr<SemNode> mLhs;

    std::vector<std::shared_ptr<SemNode>> mArgs;
};

class SemNodeLoop : public SemNodeScope
{
public:
    SemNodeLoop(const uint32_t pos, const std::string &loopName);

    void setIteratorInit(std::shared_ptr<SemNode> node);
    void setIteratorCondition(std::shared_ptr<SemNode> node);
    void setIteratorChange(std::shared_ptr<SemNode> node);

    std::shared_ptr<SemNode> getIteratorInit() const;
    std::shared_ptr<SemNode> getIteratorCondition() const;
    std::shared_ptr<SemNode> getIteratorChange() const;

    std::string getName() const
    {
        return mLoopName;
    }

    std::string toStr() const override;

private:
    std::string mLoopName;
    std::shared_ptr<SemNodeGroup> mLoopStatementsGroup;
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

    std::string toStr() const override;

private:
    std::string mOp;

    std::shared_ptr<SemNode> mLhs;
    std::shared_ptr<SemNode> mRhs;
};

class SemNodeIf : public SemNodeScope
{
public:
    SemNodeIf(const uint32_t pos);

    void setCond(std::shared_ptr<SemNode> cond);

    std::string toStr() const override;

private:
    std::shared_ptr<SemNode> mCond;
    std::shared_ptr<SemNodeGroup> mGroup;
};

class SemNodeUnaryOp : public SemNodePositional
{
public:
    SemNodeUnaryOp(const uint32_t pos, const std::string &op);

    void setRhs(std::shared_ptr<SemNode> rhs);

    std::string toStr() const override;

private:
    std::string mOp;
    std::shared_ptr<SemNode> mRhs;
};

class SemNodeJumpStatement : public SemNodePositional
{
public:
    SemNodeJumpStatement(const uint32_t pos, const std::string &name);

    std::string getName() const;

    std::string toStr() const override
    {
        return mName;
    }

private:
    std::string mName;
};

class SemNodeInitializerList : public SemNodePositional
{
public:
    SemNodeInitializerList(const uint32_t pos);

    void addEntry(std::shared_ptr<SemNode> node);

    std::vector<std::shared_ptr<SemNode>> &getEntries();

    std::string toStr() const override;

private:
    std::vector<std::shared_ptr<SemNode>> mEntries;
};

class SemNodeDefer : public SemNodePositional
{
public:
    SemNodeDefer(const uint32_t pos, std::shared_ptr<SemNode> deferred);

    std::string toStr() const override
    {
        return "defer";
    }

private:
    std::shared_ptr<SemNode> mDeferredNode;
};

// strange name, contains all necessary nodes for
// case X: { ... } break;
class SemNodeSwitchCaseLabel : public SemNodeScope
{
public:
    SemNodeSwitchCaseLabel(const uint32_t pos);

    void setCaseLabel(std::shared_ptr<SemNode> label);
    void setIsFallthrough(const bool isFallthrough);

    std::shared_ptr<SemNode> getCaseLabel() const;
    bool getIsFallthrough() const;

    // code under label just attached in AST

    std::string toStr() const override
    {
        return "case label";
    }

private:
    std::shared_ptr<SemNode> mCaseLabel;
    bool mIsFallthrough;
};

class SemNodeSwitchCase : public SemNodeScope
{
public:
    SemNodeSwitchCase(const uint32_t pos);

    void setSwitchExpr(std::shared_ptr<SemNode> expr);
    void setDefaultStatement(std::shared_ptr<SemNode> stmt);

    std::shared_ptr<SemNode> getSwitchExpr() const;

    // case statements attached as AST nodes

    std::string toStr() const override
    {
        return "switch..case";
    }

private:
    std::shared_ptr<SemNode> mSwitchExpr;
    std::shared_ptr<SemNode> mDefaultStatement;
};

} // namespace safec