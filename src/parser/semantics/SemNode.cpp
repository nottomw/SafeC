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

SemNodePositional::SemNodePositional(const uint32_t pos) //
    : mPos{pos}
{
    mType = Type::Undefined;
}

uint32_t SemNodePositional::getPos() const
{
    return mPos;
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

SemNodeReturn::SemNodeReturn(const uint32_t index, std::shared_ptr<SemNode> rhs) //
    : SemNodePositional{index}
    , mRhs{rhs}
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
    mType = Type::Identifier;
}

std::string SemNodeIdentifier::getName() const
{
    return mName;
}

SemNodeConstant::SemNodeConstant(const uint32_t pos, const std::string &name)
    : SemNodePositional{pos}
    , mName{name}
{
    mType = Type::Constant;
}

std::string SemNodeConstant::getName() const
{
    return mName;
}

SemNodeDeclaration::SemNodeDeclaration( //
    const uint32_t pos,
    const std::string &lhsType,
    const std::string &lhsIdentifier)
    : SemNodePositional{pos}
    , mLhsType{lhsType}
    , mLhsIdentifier{lhsIdentifier}
{
    mType = Type::Declaration;
}

std::string SemNodeDeclaration::getLhsType() const
{
    return mLhsType;
}

std::string SemNodeDeclaration::getLhsIdentifier() const
{
    return mLhsIdentifier;
}

SemNodePostfixExpression::SemNodePostfixExpression( //
    const uint32_t pos,
    const std::string &op,
    std::shared_ptr<SemNode> lhs)
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

std::shared_ptr<SemNode> SemNodePostfixExpression::getLhs() const
{
    return mLhs;
}

void SemNodePostfixExpression::addArg(std::shared_ptr<SemNode> arg)
{
    mArgs.push_back(arg);
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
    std::string itInit = "empty";
    std::string itCond = "empty";
    std::string itChange = "empty";

    if (mIteratorInit)
        itInit = mIteratorInit->toStr();
    if (mIteratorCondition)
        itCond = mIteratorCondition->toStr();
    if (mIteratorChange)
        itChange = mIteratorChange->toStr();

    return "(" + itInit + //
           ", " +         //
           itCond +       //
           ", "           //
           + itChange + ")";
}

SemNodeEmptyStatement::SemNodeEmptyStatement(const uint32_t pos)
    : SemNodePositional{pos}
{
    mType = Type::EmptyStatement;
}

SemNodeBinaryOp::SemNodeBinaryOp( //
    const uint32_t pos,
    const std::string &op,
    std::shared_ptr<SemNode> lhs)
    : SemNodePositional{pos}
    , mOp{op}
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

SemNodeIf::SemNodeIf(const uint32_t pos, std::shared_ptr<SemNode> cond)
    : SemNodeScope{pos}
    , mCond{cond}
{
    mType = Type::If;
}

SemNodeUnaryOp::SemNodeUnaryOp(const uint32_t pos, const std::string &op)
    : SemNodePositional{pos}
    , mOp{op}
    , mRhs{}
{
    mType = Type::UnaryOp;
}

void SemNodeUnaryOp::setRhs(std::shared_ptr<SemNode> rhs)
{
    mRhs = rhs;
}

} // namespace safec
