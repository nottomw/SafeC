#include "WalkerDeferExecute.hpp"

#include "SemNodeWalker.hpp"
#include "WalkerFindById.hpp"
#include "WalkerStrategy.hpp"
#include "logger/Logger.hpp"
#include "utils/Utils.hpp"

using namespace safec;

namespace
{

[[maybe_unused]] SemNode &findNodeById(SemNode &haystack, const uint32_t id)
{
    WalkerFindById finder{id};
    SemNodeWalker walker;

    walker.walk(haystack, finder);

    assert(finder.getResult() != nullptr);
    return *finder.getResult();
}

} // namespace

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

    auto currentScope = scopeGetCurrent();
    log("defer pos: (% -- %), ast: %", node.getSemStart(), node.getSemEnd(), astLevel);
    assert(currentScope != nullptr);

    DeferApplyInfo deferApply;
    deferApply.mDeferNodeId = node.getId();
    deferApply.mDeferOwnerScopeId = scopeGetCurrent()->getId();
    deferApply.mScopeToAttachDeferId = scopeGetCurrent()->getId();
    deferApply.mNodeToPrefixWithDeferId = -1;
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
    //    scopeRemove(); // remove previous function scope

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

void WalkerDeferExecute::peek(SemNodeTranslationUnit &node, const uint32_t)
{
    mTranslationUnit = &node;
}

void WalkerDeferExecute::commit()
{
    // TODO: the scoping does not work as expected, because of the
    // way different nodes are incoming and marked as scope/no scope,
    // especially when leaving some scope there is no event at all,
    // and the next seen node will reset the scope which probably
    // results in strange behavior.

    // apply all deferred changes
    //    for (auto &it : mDeferApplyInfo)
    //    {
    //        auto &deferNode = findNodeById(*mTranslationUnit, it.mDeferNodeId);

    //        auto &deferAttachedNodes = deferNode.getAttachedNodes();
    //        assert(deferAttachedNodes.size() > 0);

    //        auto deferredOperation = deferAttachedNodes[0];

    //        auto &scopeToAttachDefer = findNodeById(*mTranslationUnit, it.mScopeToAttachDeferId);
    //        scopeToAttachDefer.attach(deferredOperation);
    //    }

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
    log("(count: %) ADDING SCOPE", Color::Green, mScopes.size());
    mScopes.push_back(&node);
}

SemNode *WalkerDeferExecute::scopeGetCurrent() const
{
    if (mScopes.size() == 0)
    {
        log("ERROR: ZERO SCOPES", Color::Red);
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

    log("(count: %) REMOVING SCOPE pos: %", Color::Green, mScopes.size(), mScopes.back()->getSemEnd());

    // erase all defers matching the previous & bigger ast levels
    //    auto it = mDefersArmed.begin();
    //    while (it != mDefersArmed.end())
    //    {
    //        if (it->mAstLevel >= mAstLevelPrev)
    //        {
    //            it = mDefersArmed.erase(it);
    //        }
    //        else
    //        {
    //            it++;
    //        }
    //    }

    mScopes.pop_back();
}

void WalkerDeferExecute::deferArm(SemNodeDefer &deferNode, const uint32_t astLevel)
{
    DeferInfo defer;
    defer.mAstLevel = astLevel;
    defer.mDeferNode = &deferNode;

    mDefersArmed.push_back(defer);
}
