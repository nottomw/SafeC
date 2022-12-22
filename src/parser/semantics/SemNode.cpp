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
{
    mType = Type::Function;
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

SemNodeLoop::SemNodeLoop(const uint32_t start) //
    : SemNodeScope{start}
{
    mType = Type::Loop;
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

} // namespace safec
