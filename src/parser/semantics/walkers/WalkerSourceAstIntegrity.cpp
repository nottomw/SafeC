#include "WalkerSourceAstIntegrity.hpp"

#include "logger/Logger.hpp"
#include "semantics/SemNode.hpp"

#include <algorithm>

using namespace safec;

WalkerSourceAstIntegrity::WalkerSourceAstIntegrity()
    : WalkerStrategy{}
    , mIntegrityOk{false}
    , mMinIndex{UINT32_MAX}
    , mMaxIndex{0}
{
}

void WalkerSourceAstIntegrity::peek( //
    SemNode &node,
    const uint32_t astLevel)
{
    auto posNode = dynamic_cast<SemNodePositional *>(&node);
    if (posNode != nullptr)
    {
        peek(*posNode, astLevel);
        return;
    }

    auto scopeNode = dynamic_cast<SemNodeScope *>(&node);
    if (scopeNode != nullptr)
    {
        peek(*scopeNode, astLevel);
        return;
    }

    if (node.getType() == SemNode::Type::TranslationUnit)
    {
        return;
    }

    log("warn: integrity check node is not positional and not scope, type: %", //
        Color::Yellow,
        SemNode::TypeInfo::toStr(node.getType()));
    assert(nullptr == "node is not positional and not scope");
}

void WalkerSourceAstIntegrity::peek( //
    SemNodePositional &node,
    const uint32_t astLevel)
{
    ScopeInfo info{};
    info.mStart = node.getPos();
    info.mEnd = node.getPos();
    info.mNode = &node;

    updateMinMax(info.mStart);

    mScopesInfo.push_back(info);
}

void WalkerSourceAstIntegrity::peek( //
    SemNodeScope &node,
    const uint32_t astLevel)
{
    ScopeInfo info{};
    info.mStart = node.getStart();
    info.mEnd = node.getEnd();
    info.mNode = &node;

    updateMinMax(info.mStart);
    updateMinMax(info.mEnd);

    mScopesInfo.push_back(info);
}

bool WalkerSourceAstIntegrity::getIntegrityOk() const
{
    return mIntegrityOk;
}

void WalkerSourceAstIntegrity::printReport()
{
    uint32_t counter = 0;

    auto cmpLambda =                                   //
        [](const ScopeInfo &lhs, const ScopeInfo &rhs) //
    {
        // true if lhs > rhs
        // reverse sort
        return (lhs.mStart < rhs.mStart);
    };

    std::sort(               //
        mScopesInfo.begin(), //
        mScopesInfo.end(),   //
        cmpLambda);

    for (auto &it : mScopesInfo)
    {
        assert(it.mNode != nullptr);

        log("[%] scope info: start: %, end: %, node type: %",
            counter,
            it.mStart,
            it.mEnd,
            SemNode::TypeInfo::toStr(it.mNode->getType()));
        counter++;
    }

    log("covered indexes range: % -- %", mMinIndex, mMaxIndex);
}

void WalkerSourceAstIntegrity::updateMinMax(const uint32_t pos)
{
    if (pos > mMaxIndex)
    {
        mMaxIndex = pos;
    }

    if (pos < mMinIndex)
    {
        mMinIndex = pos;
    }
}
