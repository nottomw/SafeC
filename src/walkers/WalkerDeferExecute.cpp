#include "WalkerDeferExecute.hpp"

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"

using namespace safec;

WalkerDeferExecute::WalkerDeferExecute()
    : mAstLevelPrev{0}
{
    // TODO: handle case when defer is used in the last function with
    // no return in function scope - there are no more nodes that are
    // lower in ast level so the walker will not visit them and will
    // not have a chance to call scopeRemove()
}

void WalkerDeferExecute::peek(SemNode &node, const uint32_t astLevel)
{
    const AstLevelEvent ev = getAstLevelEvent(astLevel);
    if (ev == AstLevelEvent::Exit)
    {
        scopeRemove();
        mAstLevelPrev = astLevel;
    }
}

void WalkerDeferExecute::peek(SemNodeDefer &node, const uint32_t astLevel)
{
    (void)getAstLevelEvent(astLevel);
    mAstLevelPrev = astLevel;

    deferArm(node, astLevel);

    // fire the defer in current scope

    DeferApplyInfo deferApply;
    deferApply.mDeferNode = &node;
    deferApply.mDeferOwnerScope = scopeGetCurrent();
    deferApply.mScopeToAttachDefer = scopeGetCurrent();
    deferApply.mNodeToPrefixWithDefer = nullptr;
    mDeferApplyInfo.push_back(deferApply);
}

void WalkerDeferExecute::peek(SemNodeReturn &node, const uint32_t astLevel)
{
    (void)getAstLevelEvent(astLevel);
    mAstLevelPrev = astLevel;
}

void WalkerDeferExecute::peek(SemNodeJumpStatement &node, const uint32_t astLevel)
{
    (void)getAstLevelEvent(astLevel);
    mAstLevelPrev = astLevel;
}

void WalkerDeferExecute::peek(SemNodeScope &node, const uint32_t astLevel)
{
    (void)getAstLevelEvent(astLevel);
    mAstLevelPrev = astLevel;
    scopeAdd(node);
}

void WalkerDeferExecute::peek(SemNodeFunction &node, const uint32_t astLevel)
{
    //    mDefersArmed.clear(); // since this is a new function clear all previous defers
    scopeRemove(); // remove previous function scope

    (void)getAstLevelEvent(astLevel);
    mAstLevelPrev = astLevel;
    scopeAdd(node);
}

void WalkerDeferExecute::peek(SemNodeLoop &node, const uint32_t astLevel)
{
    (void)getAstLevelEvent(astLevel);
    mAstLevelPrev = astLevel;
    scopeAdd(node);
}

void WalkerDeferExecute::peek(SemNodeIf &node, const uint32_t astLevel)
{
    (void)getAstLevelEvent(astLevel);
    mAstLevelPrev = astLevel;
    scopeAdd(node);
}

void WalkerDeferExecute::peek(SemNodeSwitchCaseLabel &node, const uint32_t astLevel)
{
    (void)getAstLevelEvent(astLevel);
    mAstLevelPrev = astLevel;
    scopeAdd(node);
}

void WalkerDeferExecute::commit()
{
    // apply all deferred changes
    for (auto &it : mDeferApplyInfo)
    {
        auto &deferAttachedNodes = it.mDeferNode->getAttachedNodes();
        assert(deferAttachedNodes.size() > 0);

        auto deferredOperation = deferAttachedNodes[0];
        if (it.mScopeToAttachDefer != nullptr)
        {
            it.mScopeToAttachDefer->attach(deferredOperation);
        }
    }

    //        // remove the defer nodes from AST
    //    for (auto &it : mDeferApplyInfo)
    //    {
    //        auto &ownerScopeAttachedNodes = it.mDeferOwnerScope->getAttachedNodes();
    //        auto ownerScopeNodeIt = ownerScopeAttachedNodes.begin();
    //        while (ownerScopeNodeIt != ownerScopeAttachedNodes.end())
    //        {
    //            auto &nodeIt = *ownerScopeNodeIt;
    //            if (nodeIt->getId() == it.mDeferNode->getId())
    //            {
    //                ownerScopeNodeIt = ownerScopeAttachedNodes.erase(ownerScopeNodeIt);
    //                break;
    //            }
    //            else
    //            {
    //                ownerScopeNodeIt++;
    //            }
    //        }
    //    }
}

WalkerDeferExecute::AstLevelEvent WalkerDeferExecute::getAstLevelEvent(const uint32_t currentAstLevel)
{
    utils::DeferredCall guardOverwriteAstLevelPrev( //
        [self = this, &currentAstLevel]             //
        {                                           //
            self->mAstLevelPrev = currentAstLevel;
        });

    if (currentAstLevel == mAstLevelPrev)
    {
        return AstLevelEvent::Same;
    }
    else if (currentAstLevel > mAstLevelPrev)
    {
        return AstLevelEvent::Enter;
    }
    else
    {
        return AstLevelEvent::Exit;
    }
}

void WalkerDeferExecute::scopeAdd(SemNode &node)
{
    mScopes.push_back(&node);
}

SemNode *WalkerDeferExecute::scopeGetCurrent() const
{
    if (mScopes.size() == 0)
    {
        return nullptr;
    }

    return mScopes.back();
}

void WalkerDeferExecute::scopeRemove()
{
    if (mScopes.size() == 0)
    {
        return;
    }

    // erase all defers matching the previous & bigger ast levels
    auto it = mDefersArmed.begin();
    while (it != mDefersArmed.end())
    {
        if (it->mAstLevel >= mAstLevelPrev)
        {
            it = mDefersArmed.erase(it);
        }
        else
        {
            it++;
        }
    }

    mScopes.pop_back();
}

void WalkerDeferExecute::deferArm(SemNodeDefer &deferNode, const uint32_t astLevel)
{
    DeferInfo defer;
    defer.mAstLevel = astLevel;
    defer.mDeferNode = &deferNode;

    mDefersArmed.push_back(defer);
}
