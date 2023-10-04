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
    : mAstLevelOfScopePrev{0}
{
}

void WalkerDeferExecute::peek(SemNode &node, const uint32_t astLevel)
{
    scopeRemoveIfLeftScope(astLevel);
}

void WalkerDeferExecute::peek(SemNodeDefer &node, const uint32_t astLevel)
{
    scopeRemoveIfLeftScope(astLevel);

    deferArm(node, astLevel);

    // fire the defer in current scope

    auto currentScope = scopeGetCurrent();
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
    scopeRemoveIfLeftScope(astLevel);
}

void WalkerDeferExecute::peek(SemNodeJumpStatement &node, const uint32_t astLevel)
{
    scopeRemoveIfLeftScope(astLevel);
}

void WalkerDeferExecute::peek(SemNodeScope &node, const uint32_t astLevel)
{
    scopeRemoveIfLeftScope(astLevel);
    mAstLevelOfScopePrev = astLevel;

    scopeAdd(node);
}

void WalkerDeferExecute::peek(SemNodeFunction &node, const uint32_t astLevel)
{
    mDefersArmed.clear(); // since this is a new function clear all previous defers
    mScopes.clear();      // remove all scopes

    scopeRemoveIfLeftScope(astLevel);
    mAstLevelOfScopePrev = astLevel;

    scopeAdd(node);
}

void WalkerDeferExecute::peek(SemNodeLoop &node, const uint32_t astLevel)
{
    scopeRemoveIfLeftScope(astLevel);
    mAstLevelOfScopePrev = astLevel;

    scopeAdd(node);
}

void WalkerDeferExecute::peek(SemNodeIf &node, const uint32_t astLevel)
{
    scopeRemoveIfLeftScope(astLevel);
    mAstLevelOfScopePrev = astLevel;

    scopeAdd(node);
}

void WalkerDeferExecute::peek(SemNodeSwitchCaseLabel &node, const uint32_t astLevel)
{
    scopeRemoveIfLeftScope(astLevel);
    mAstLevelOfScopePrev = astLevel;

    scopeAdd(node);
}

void WalkerDeferExecute::peek(SemNodeTranslationUnit &node, const uint32_t astLevel)
{
    mTranslationUnit = &node;
}

void WalkerDeferExecute::commit()
{
    scopeRemoveIfLeftScope(0);

    // apply all deferred changes
    for (auto &it : mDeferApplyInfo)
    {
        auto &deferNode = findNodeById(*mTranslationUnit, it.mDeferNodeId);

        auto &deferAttachedNodes = deferNode.getAttachedNodes();
        assert(deferAttachedNodes.size() > 0);

        auto deferredOperation = deferAttachedNodes[0];

        auto &scopeToAttachDefer = findNodeById(*mTranslationUnit, it.mScopeToAttachDeferId);

        // TODO: need to recalculate positions after this node to properly
        // generate the output source file. For now just zero out the
        // positions and mark the node as dirty.
        deferredOperation->setSemStart(0);
        deferredOperation->setSemEnd(0);
        deferredOperation->setDirty(true);

        scopeToAttachDefer.attach(deferredOperation);
    }

    // remove the defer nodes from AST
    for (auto &it : mDeferApplyInfo)
    {
        auto &deferOwnerScope = findNodeById(*mTranslationUnit, it.mDeferOwnerScopeId);

        auto &ownerScopeAttachedNodes = deferOwnerScope.getAttachedNodes();
        auto ownerScopeNodeIt = ownerScopeAttachedNodes.begin();
        while (ownerScopeNodeIt != ownerScopeAttachedNodes.end())
        {
            auto &nodeIt = *ownerScopeNodeIt;
            auto &deferredNode = findNodeById(*mTranslationUnit, it.mDeferNodeId);
            if (nodeIt->getId() == deferredNode.getId())
            {
                // set as dirty since node is removed from this scope
                deferOwnerScope.setDirty(true);
                ownerScopeNodeIt = ownerScopeAttachedNodes.erase(ownerScopeNodeIt);
                break;
            }
            else
            {
                ownerScopeNodeIt++;
            }
        }
    }
}

WalkerDeferExecute::AstLevelEvent WalkerDeferExecute::getAstLevelEvent(const uint32_t currentAstLevel)
{
    if (currentAstLevel == mAstLevelOfScopePrev)
    {
        return AstLevelEvent::Same;
    }
    else if (currentAstLevel > mAstLevelOfScopePrev)
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
        log("ERROR: scopeGetCurrent() called, but zero scopes", Color::Red);
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

    // remove all defers from the scope that we're leaving
    for (auto it = mDefersArmed.begin(); it != mDefersArmed.end(); /*empty*/)
    {
        if (it->mAstLevel >= mAstLevelOfScopePrev)
        {
            it = mDefersArmed.erase(it);
        }
        else
        {
            it++;
        }
    }

    mScopes.pop_back();
    mAstLevelOfScopePrev--;
}

void WalkerDeferExecute::scopeRemoveIfLeftScope(const uint32_t astLevel)
{
    bool removed = false;
    do
    {
        const AstLevelEvent ev = getAstLevelEvent(astLevel);
        if (ev == AstLevelEvent::Exit)
        {
            scopeRemove();
            removed = true;
        }
        else if (ev == AstLevelEvent::Same)
        {
            scopeRemove();
            break; // no further scopes to remove
        }
        else
        {
            removed = false;
        }
    } while (removed == true && mScopes.size() != 0);
}

void WalkerDeferExecute::deferArm(SemNodeDefer &deferNode, const uint32_t astLevel)
{
    DeferInfo defer;
    defer.mAstLevel = astLevel;
    defer.mDeferNode = &deferNode;

    mDefersArmed.push_back(defer);
}
