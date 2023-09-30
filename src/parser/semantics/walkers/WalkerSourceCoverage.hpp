#pragma once

#include "SemNodeWalker.hpp"
#include "WalkerStrategy.hpp"

#include <memory>
#include <vector>

namespace safec
{

// verify the AST corresponds directly to the source file
class WalkerSourceCoverage final : public WalkerStrategy
{
public:
    WalkerSourceCoverage();

    void peek(SemNode &node, const uint32_t astLevel) override;
    void peek(SemNodePositional &node, const uint32_t astLevel) override;
    void peek(SemNodeScope &node, const uint32_t astLevel) override;

    void printReport();

private:
    struct ScopeInfo
    {
        uint32_t mStart;
        uint32_t mEnd;
        SemNode *mNode; // TODO: add NonOwningPtr<> / NonNullPtr<>
    };

    std::vector<ScopeInfo> mScopesInfo;
    uint32_t mMinIndex;
    uint32_t mMaxIndex;

    void updateMinMax(const uint32_t pos);
    void prepareScopesInfo();
};

} // namespace safec
