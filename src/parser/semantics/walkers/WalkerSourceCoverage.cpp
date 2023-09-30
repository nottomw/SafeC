#include "WalkerSourceCoverage.hpp"

#include "logger/Logger.hpp"
#include "semantics/SemNode.hpp"

#include <algorithm>

using namespace safec;

WalkerSourceCoverage::WalkerSourceCoverage()
    : WalkerStrategy{}
    , mMinIndex{UINT32_MAX}
    , mMaxIndex{0}
{
}

void WalkerSourceCoverage::peek( //
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

void WalkerSourceCoverage::peek( //
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

void WalkerSourceCoverage::peek( //
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

void WalkerSourceCoverage::printReport()
{
    if (mScopesInfo.size() == 0)
    {
        log("no scope info", Color::Red);
        return;
    }

    prepareScopesInfo();

    // TODO: parser - add info on start/end when doing reduce
    // TODO: divide into ScopedInfo and PositionalInfo
    // TODO: verify scopes are continuous
    // TODO: verify positionals in scope covers the whole scope

    std::vector<ScopeInfo *> scopeStack;

    for (auto &it : mScopesInfo)
    {
        assert(it.mNode != nullptr);

        log(" - scope info: start: %, end: %, node type: %",
            it.mStart,
            it.mEnd,
            SemNode::TypeInfo::toStr(it.mNode->getType()));

        // enter each scope and check if the scope is covered
        (void)scopeStack;
    }

    log("covered indexes range: % -- %", mMinIndex, mMaxIndex);
}

void WalkerSourceCoverage::updateMinMax(const uint32_t pos)
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

void WalkerSourceCoverage::prepareScopesInfo()
{
    auto cmpLambda =                                   //
        [](const ScopeInfo &lhs, const ScopeInfo &rhs) //
    {
        // true if lhs > rhs; here: reverse sort -> ascending order
        return (lhs.mStart < rhs.mStart);
    };

    std::sort(               //
        mScopesInfo.begin(), //
        mScopesInfo.end(),   //
        cmpLambda);
}
