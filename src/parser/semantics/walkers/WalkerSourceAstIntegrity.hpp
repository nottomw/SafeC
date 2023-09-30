#pragma once

#include "SemNodeWalker.hpp"
#include "WalkerStrategy.hpp"

#include <memory>
#include <vector>

namespace safec
{

// verify the AST corresponds directly to the source file
class WalkerSourceAstIntegrity final : public WalkerStrategy
{
public:
    WalkerSourceAstIntegrity();

    void peek(SemNodePositional &node, const uint32_t astLevel) override;
    void peek(SemNodeScope &node, const uint32_t astLevel) override;

    bool getIntegrityOk() const;

private:
    struct ScopeInfo
    {
        uint32_t mStart;
        uint32_t mEnd;
        std::shared_ptr<SemNode> mNode;
    };

    bool mIntegrityOk;
    std::vector<ScopeInfo> mScopesInfo;

    void insertSorted(ScopeInfo &info);
};

} // namespace safec
