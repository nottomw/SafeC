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
    log("defer pos: (% -- %), ast: %, in scope: %",
        node.getSemStart(),
        node.getSemEnd(),
        astLevel,
        currentScope->getTypeStr());
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
    log(" --------- COMMIT", Color::Cyan);
    scopeRemoveIfLeftScope(0);

    // TODO: the scoping does not work as expected, because of the
    // way different nodes are incoming and marked as scope/no scope,
    // especially when leaving some scope there is no event at all,
    // and the next seen node will reset the scope which probably
    // results in strange behavior.

    // TODO: handle case when defer is used in the last function with
    // no return in function scope - there are no more nodes that are
    // lower in ast level so the walker will not visit them and will
    // not have a chance to call scopeRemove()

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

        log("DEFER COMMIT: % (% -- %) into %",
            Color::Blue,
            Color::BgWhite,
            deferredOperation->getTypeStr(),
            deferNode.getSemStart(),
            deferNode.getSemEnd(),
            scopeToAttachDefer.getTypeStr());
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
    for (uint32_t i = 0; i < mScopes.size(); i++)
        log("\t", NewLine::No);
    log("(count: %) ADDING SCOPE (%) pos: (% -- %)", //
        Color::Green,
        mScopes.size(),
        node.getTypeStr(),
        node.getSemStart(),
        node.getSemEnd());
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

    for (uint32_t i = 0; i < mScopes.size(); i++)
        log("\t", NewLine::No);

    log("(count: %) REMOVING SCOPE (%) pos: (% -- %)", //
        Color::Green,
        mScopes.size(),
        mScopes.back()->getTypeStr(),
        mScopes.back()->getSemStart(),
        mScopes.back()->getSemEnd());

    // remove all defers from the scope that we're leaving
    for (auto it = mDefersArmed.begin(); it != mDefersArmed.end(); /*empty*/)
    {
        if (it->mAstLevel >= mAstLevelOfScopePrev)
        {
            log("Removing defer: % (% -- %)",
                Color::Red,
                it->mDeferNode->getTypeStr(),
                it->mDeferNode->getSemStart(),
                it->mDeferNode->getSemEnd());
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
            log("SCOPE REMOVE: exit");
            scopeRemove();
            removed = true;
        }
        else if (ev == AstLevelEvent::Same)
        {
            log("SCOPE REMOVE: same");
            scopeRemove();
            break;
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
