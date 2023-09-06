#include "SemNode.hpp"

namespace safec
{

SemNode::SemNode() //
    : mType{Type::Undefined}
{
}

SemNode::SemNode(const SemNode::Type type) //
    : mType{type}
{
}

void SemNode::reset()
{
    mRelatedNodes.clear();
}

SemNode::Type SemNode::getType() const
{
    return mType;
}

void SemNode::attach(std::shared_ptr<SemNode> node)
{
    mRelatedNodes.push_back(node);
}

SemNodeTranslationUnit::SemNodeTranslationUnit()
{
    mType = Type::TranslationUnit;
}

SemNodeScope::SemNodeScope(const uint32_t start) //
    : mStartIndex{start}
    , mEndIndex{0}
{
    mType = Type::Scope;
}

void SemNodeScope::setEnd(const uint32_t end)
{
    mEndIndex = end;
}

uint32_t SemNodeScope::getStart() const
{
    return mStartIndex;
}

uint32_t SemNodeScope::getEnd() const
{
    return mEndIndex;
}

SemNodeFunction::SemNodeFunction(const uint32_t start) //
    : SemNodeScope{start}                              //
    , mReturnType{}
    , mParams{}
{
    mType = Type::Function;
}

std::string SemNodeFunction::getName() const
{
    return mName;
}

std::string SemNodeFunction::getReturn() const
{
    return mReturnType;
}

std::vector<SemNodeFunction::Param> SemNodeFunction::getParams() const
{
    return mParams;
}

void SemNodeFunction::setName(const std::string &name)
{
    mName = name;
}

void SemNodeFunction::setReturn(const std::string &type)
{
    mReturnType = type;
}

void SemNodeFunction::addParam(const std::string &type, const std::string &name)
{
    mParams.emplace_back(Param{type, name});
}

SemNodePositional::SemNodePositional(const uint32_t pos) //
    : mPos{pos}
{
    mType = Type::Undefined;
}

uint32_t SemNodePositional::getPos() const
{
    return mPos;
}

SemNodeDefer::SemNodeDefer(const uint32_t index) //
    : SemNodePositional{index}
{
    mType = Type::Defer;
}

void SemNodeDefer::setDeferredText(std::string &&deferredText)
{
    mDeferredText = std::move(deferredText);
}

std::string SemNodeDefer::getDeferredText() const
{
    return mDeferredText;
}

void SemNodeDefer::setDeferredStatementLen(const uint32_t len)
{
    mDeferredStatementLen = len;
}

uint32_t SemNodeDefer::getDeferredStatementLen() const
{
    return mDeferredStatementLen;
}

SemNodeReturn::SemNodeReturn(const uint32_t index) //
    : SemNodePositional{index}
{
    mType = Type::Return;
}

SemNodeBreak::SemNodeBreak(const uint32_t pos) //
    : SemNodePositional{pos}
{
    mType = Type::Break;
}

SemNodeContinue::SemNodeContinue(const uint32_t pos) //
    : SemNodePositional{pos}
{
    mType = Type::Continue;
}

SemNodeIdentifier::SemNodeIdentifier(const uint32_t pos, const std::string &name)
    : SemNodePositional{pos}
    , mName{name}
{
    mType = Type::Undefined;
}

std::string SemNodeIdentifier::getName() const
{
    return mName;
}

SemNodeReference::SemNodeReference(const uint32_t pos, const std::string &name)
    : SemNodeIdentifier(pos, name)
{
    mType = Type::Reference;
}

SemNodeDeclaration::SemNodeDeclaration( //
    const uint32_t pos,
    const std::string &lhsType,
    const std::string &lhsIdentifier)
    : SemNodePositional{pos}
    , mLhsType{lhsType}
    , mLhsIdentifier{lhsIdentifier}
    , mRhs{}
{
    mType = Type::Declaration;
}

void SemNodeDeclaration::setRhs(std::shared_ptr<SemNode> rhsIdentifier)
{
    mRhs = rhsIdentifier;
}

std::string SemNodeDeclaration::getLhsType() const
{
    return mLhsType;
}

std::string SemNodeDeclaration::getLhsIdentifier() const
{
    return mLhsIdentifier;
}

std::shared_ptr<SemNode> SemNodeDeclaration::getRhs() const
{
    return mRhs;
}

SemNodeAssignment::SemNodeAssignment( //
    const uint32_t pos,
    const std::string &op,
    const std::string &lhs,
    const std::string &rhs)
    : SemNodePositional{pos}
    , mOperator{op}
    , mLhs{lhs}
    , mRhs{rhs}
{
    mType = Type::Assignment;
}

std::string SemNodeAssignment::getOperator() const
{
    return mOperator;
}

std::string SemNodeAssignment::getLhs() const
{
    return mLhs;
}

std::string SemNodeAssignment::getRhs() const
{
    return mRhs;
}

SemNodeRelationalExpression::SemNodeRelationalExpression( //
    const uint32_t pos,
    const std::string &op,
    const std::string &lhs,
    const std::string &rhs)
    : SemNodePositional{pos}
    , mOperator{op}
    , mLhs{lhs}
    , mRhs{rhs}
{
    mType = Type::RelationalExpression;
}

std::string SemNodeRelationalExpression::getOperator() const
{
    return mOperator;
}

std::string SemNodeRelationalExpression::getLhs() const
{
    return mLhs;
}

std::string SemNodeRelationalExpression::getRhs() const
{
    return mRhs;
}

SemNodePostfixExpression::SemNodePostfixExpression( //
    const uint32_t pos,
    const std::string &op,
    const std::string &lhs)
    : SemNodePositional{pos}
    , mOperator{op}
    , mLhs{lhs}
{
    mType = Type::PostfixExpression;
}

std::string SemNodePostfixExpression::getOperator() const
{
    return mOperator;
}

std::string SemNodePostfixExpression::getLhs() const
{
    return mLhs;
}

SemNodeLoop::SemNodeLoop(const uint32_t pos)
    : SemNodePositional{pos}
{
    mType = Type::Loop;
}

void SemNodeLoop::setIteratorInit(std::shared_ptr<SemNode> node)
{
    mIteratorInit = node;
}

void SemNodeLoop::setIteratorCondition(std::shared_ptr<SemNode> node)
{
    mIteratorCondition = node;
}

void SemNodeLoop::setIteratorChange(std::shared_ptr<SemNode> node)
{
    mIteratorChange = node;
}

std::shared_ptr<SemNode> SemNodeLoop::getIteratorInit() const
{
    return mIteratorInit;
}

std::shared_ptr<SemNode> SemNodeLoop::getIteratorCondition() const
{
    return mIteratorCondition;
}

std::shared_ptr<SemNode> SemNodeLoop::getIteratorChange() const
{
    return mIteratorChange;
}

std::string SemNodeLoop::toStr() const
{
    return "(" + mIteratorInit->toStr() + //
           ", " +                         //
           mIteratorCondition->toStr() +  //
           ", "                           //
           + mIteratorChange->toStr() + ")";
}

SemNodeEmptyStatement::SemNodeEmptyStatement(const uint32_t pos)
    : SemNodePositional{pos}
{
    mType = Type::EmptyStatement;
}

SemNodeBinaryOp::SemNodeBinaryOp(const uint32_t pos, std::shared_ptr<SemNode> lhs)
    : SemNodePositional{pos}
    , mLhs{lhs}
{
    mType = Type::BinaryOp;
}

void SemNodeBinaryOp::setRhs(std::shared_ptr<SemNode> rhs)
{
    mRhs = rhs;
}

std::shared_ptr<SemNode> SemNodeBinaryOp::getLhs() const
{
    return mLhs;
}

std::shared_ptr<SemNode> SemNodeBinaryOp::getRhs() const
{
    return mRhs;
}

} // namespace safec
