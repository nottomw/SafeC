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

    log("error: integrity check node is not positional and not scope, type: %", //
        Color::Yellow,
        SemNode::TypeInfo::toStr(node.getType()));
    assert(nullptr == "node is not positional and not scope");
}

void WalkerSourceCoverage::peek( //
    SemNodePositional &node,
    const uint32_t astLevel)
{
    if (node.getSemStart() == 0 && node.getSemEnd() == 0)
    {
        // ignore if no semantic pos info
        return;
    }

    updateMinMax(node.getSemStart());
    updateMinMax(node.getSemEnd());

    PosInfo posInfo;
    posInfo.mStart = node.getSemStart();
    posInfo.mEnd = node.getSemEnd();
    posInfo.mAstLevel = astLevel;
    posInfo.mNode = &node;

    mScopesInfo.push_back(posInfo);
}

void WalkerSourceCoverage::peek( //
    SemNodeScope &node,
    const uint32_t astLevel)
{
    if (node.getSemStart() == 0 && node.getSemEnd() == 0)
    {
        // ignore if no semantic pos info
        return;
    }

    updateMinMax(node.getSemStart());
    updateMinMax(node.getSemEnd());

    PosInfo scopeInfo;
    scopeInfo.mStart = node.getSemStart();
    scopeInfo.mEnd = node.getSemEnd();
    scopeInfo.mAstLevel = astLevel;
    scopeInfo.mNode = &node;

    mScopesInfo.push_back(scopeInfo);
}

void WalkerSourceCoverage::peek(SemNodeGroup &node, const uint32_t astLevel)
{
    // ignore
}

void WalkerSourceCoverage::printReport()
{
    log("Coverage info:");
    checkScopes();

    log("\n\tcovered min/max range: % -- %", mMinIndex, mMaxIndex);
}

void WalkerSourceCoverage::checkScopes()
{
    assert(mScopesInfo.size() > 1);

    auto scopeInfoCopy = mScopesInfo;

    std::vector<std::pair<PosInfo, PosInfo>> covGapsInfo;

    for (size_t i = 1; i < scopeInfoCopy.size(); i++)
    {
        auto &scopePrev = scopeInfoCopy[i - 1];
        auto &scopeCur = scopeInfoCopy[i];

        if (scopePrev.mAstLevel == scopeCur.mAstLevel)
        {
            if ((scopePrev.mEnd != scopeCur.mStart) && //
                (scopePrev.mEnd != (scopeCur.mStart + 1)))
            {
                covGapsInfo.push_back(std::make_pair(scopePrev, scopeCur));
            }

            // scope checked
            scopeInfoCopy.erase(scopeInfoCopy.begin() + i - 1);
            i = 0; // reset loop
        }
        else if (scopePrev.mAstLevel > scopeCur.mAstLevel)
        {
            // scope left
            scopeInfoCopy.erase(scopeInfoCopy.begin() + i - 1);
            i = 0; // reset loop
        }
        else // prevSeenScope.mAstLevel < scopeInfo.mAstLevel
        {
            // nothing
        }
    }

    for (auto &it : covGapsInfo)
    {
        auto &scopePrev = it.first;
        auto &scopeCur = it.second;
        log("\tscope gap (% -- %) between nodes % (% - %) and % (% - %)", //
            Color::Red,
            scopePrev.mEnd,
            scopeCur.mStart,
            scopePrev.mNode->getTypeStr(),
            scopePrev.mStart,
            scopePrev.mEnd,
            scopeCur.mNode->getTypeStr(),
            scopeCur.mStart,
            scopeCur.mEnd);
    }

    if (covGapsInfo.size() == 0)
    {
        log("\twhole source covered", Color::Green);
    }
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
