#pragma once

#include "WalkerStrategy.hpp"

#include <vector>

namespace safec
{

// Print the whole ast.
class WalkerDeferExecute final : public WalkerStrategy
{
public:
    WalkerDeferExecute();

    void peek(SemNode &node, const uint32_t astLevel) override;

    // the defer keyword
    void peek(SemNodeDefer &node, const uint32_t astLevel) override;

    // all nodes that have impact on defer keyword
    void peek(SemNodeReturn &node, const uint32_t astLevel) override;
    void peek(SemNodeJumpStatement &node, const uint32_t astLevel) override;

    // all scopes
    void peek(SemNodeScope &node, const uint32_t astLevel) override;
    void peek(SemNodeFunction &node, const uint32_t astLevel) override;
    void peek(SemNodeLoop &node, const uint32_t astLevel) override;
    void peek(SemNodeIf &node, const uint32_t astLevel) override;
    void peek(SemNodeSwitchCaseLabel &node, const uint32_t astLevel) override;

    void peek(SemNodeTranslationUnit &node, const uint32_t astLevel) override;

    void commit();

private:
    struct DeferInfo
    {
        uint32_t mAstLevel;
        SemNodeDefer *mDeferNode;
    };

    struct DeferApplyInfo
    {
        int32_t mDeferNodeId;
        int32_t mDeferOwnerScopeId;
        int32_t mScopeToAttachDeferId;
        int32_t mNodeToPrefixWithDeferId;
    };

    uint32_t mAstLevelOfScopePrev;
    std::vector<SemNode *> mScopes;
    std::vector<DeferInfo> mDefersArmed;
    std::vector<DeferApplyInfo> mDeferApplyInfo;
    SemNodeTranslationUnit *mTranslationUnit;

    enum class AstLevelEvent
    {
        Same,
        Enter,
        Exit
    };

    AstLevelEvent getAstLevelEvent(const uint32_t currentAstLevel);

    void scopeAdd(SemNode &node);
    SemNode *scopeGetCurrent() const;
    void scopeRemove();
    void scopeRemoveIfLeftScope(const uint32_t astLevel);

    void deferArm(SemNodeDefer &deferNode, const uint32_t astLevel);
};

} // namespace safec
