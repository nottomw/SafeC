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

SemNodeScope::SemNodeScope(const uint32_t start) //
    : mStartIndex{start}, mEndIndex{0}
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
{
    mType = Type::Function;
}

SemNodeDefer::SemNodeDefer(const uint32_t index) : mDeferEndIndex{index}
{
    mType = Type::Defer;
}

uint32_t SemNodeDefer::getPos() const
{
    return mDeferEndIndex;
}

} // namespace safec
