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

std::string SemNodeFunction::toStr() const
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

SemNodeReturn::SemNodeReturn(const uint32_t index)
    : SemNodeReturn{index, {}}
{
}

SemNodeReturn::SemNodeReturn(const uint32_t index, std::shared_ptr<SemNode> rhs) //
    : SemNodePositional{index}
    , mRhs{rhs}
{
    mType = Type::Return;
}

std::string SemNodeReturn::toStr() const
{
    std::string rhs;
    if (mRhs)
        rhs = mRhs->toStr();

    return rhs;
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

void SemNodeDeclaration::appendToType(const std::string &str)
{
    mLhsType += str;
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

std::string SemNodePostfixExpression::toStr() const
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
    else if (mOperator == "[]")
    {
        lhs += "[";

        // can this be even multiple args?
        for (auto &args : mArgs)
        {
            lhs += args->toStr();
            lhs += ", ";
        }

        lhs += "]";
    }
    else
    {
        lhs += mOperator;
    }

    lhs += ")";

    return lhs;
}

SemNodeLoop::SemNodeLoop( //
    const uint32_t pos,
    const std::string &loopName)
    : SemNodeScope{pos}
    , mLoopName{loopName}
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

    return mLoopName + "(" + itInit + //
           ", " +                     //
           itCond +                   //
           ", "                       //
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

    attach(lhs);
}

void SemNodeBinaryOp::setRhs(std::shared_ptr<SemNode> rhs)
{
    mRhs = rhs;
    attach(rhs);
}

std::shared_ptr<SemNode> SemNodeBinaryOp::getLhs() const
{
    return mLhs;
}

std::shared_ptr<SemNode> SemNodeBinaryOp::getRhs() const
{
    return mRhs;
}

std::string SemNodeBinaryOp::toStr() const
{
    return mOp;
}

SemNodeIf::SemNodeIf(const uint32_t pos, std::shared_ptr<SemNode> cond)
    : SemNodeScope{pos}
    , mCond{cond}
{
    mType = Type::If;
}

std::string SemNodeIf::toStr() const
{
    std::string cond = "empty";
    if (mCond)
        cond = mCond->toStr();
    return cond;
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

std::string SemNodeUnaryOp::toStr() const
{
    std::string str = mOp + " ";
    if (mRhs)
        str += mRhs->toStr();
    return str;
}

SemNodeJumpStatement::SemNodeJumpStatement( //
    const uint32_t pos,
    const std::string &name)
    : SemNodePositional{pos}
    , mName{name}
{
    mType = Type::JumpStatement;
}

SemNodeInitializerList::SemNodeInitializerList(const uint32_t pos)
    : SemNodePositional{pos}
{
    mType = Type::InitializerList;
}

void SemNodeInitializerList::addEntry(std::shared_ptr<SemNode> node)
{
    mEntries.push_back(node);
}

std::string SemNodeInitializerList::toStr() const
{
    std::string str;
    str += "{ ";

    for (auto &it : mEntries)
    {
        str += it->toStr();
        str += ", ";
    }

    str += " }";
    return str;
}

} // namespace safec
