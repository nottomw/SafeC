#include "WalkerDefer.hpp"

#include "logger/Logger.hpp"
#include "semantics/SemNode.hpp"

#include <algorithm>

namespace safec
{

void WalkerDefer::peek(SemNode &, const uint32_t)
{
    // nothing to do
}

void WalkerDefer::peek(SemNodeScope &node, const uint32_t astLevel)
{
    mProgramStructure.emplace(node.getStart(), ProgramElem{ElemType::ScopeStart, astLevel});
    mProgramStructure.emplace(node.getEnd(), ProgramElem{ElemType::ScopeEnd, astLevel});
}

void WalkerDefer::peek(SemNodeFunction &node, const uint32_t astLevel)
{
    mProgramStructure.emplace(node.getStart(), ProgramElem{ElemType::ScopeStart, astLevel});
    mProgramStructure.emplace(node.getEnd(), ProgramElem{ElemType::ScopeEnd, astLevel});
}

void WalkerDefer::peek(SemNodeLoop &node, const uint32_t astLevel)
{
    mProgramStructure.emplace(node.getStart(), ProgramElem{ElemType::LoopStart, astLevel});
    mProgramStructure.emplace(node.getEnd(), ProgramElem{ElemType::LoopEnd, astLevel});
}

void WalkerDefer::peek(SemNodeDefer &node, const uint32_t astLevel)
{
    mProgramStructure.emplace(node.getPos(), ProgramElem{ElemType::Defer, astLevel, &node});
}

void WalkerDefer::peek(SemNodeReturn &node, const uint32_t astLevel)
{
    mProgramStructure.emplace(node.getPos(), ProgramElem{ElemType::Return, astLevel});
}

void WalkerDefer::peek(SemNodeBreak &node, const uint32_t astLevel)
{
    mProgramStructure.emplace(node.getPos(), ProgramElem{ElemType::Break, astLevel});
}

void WalkerDefer::peek(SemNodeContinue &node, const uint32_t astLevel)
{
    mProgramStructure.emplace(node.getPos(), ProgramElem{ElemType::Continue, astLevel});
}

WalkerDefer::DeferFiresVector WalkerDefer::getDeferFires()
{
    for (const auto &it : mProgramStructure)
    {
        const uint32_t elemCharacterPos = it.first;
        const ProgramElem &elem = it.second;

        if (elem.type == ElemType::Defer)
        {
            const AstLevel deferActiveAtLevel = elem.astLevel - 1;
            mActiveDefers.emplace_back(std::make_pair(deferActiveAtLevel, elem.defer));
            continue;
        }

        if (elem.type == ElemType::ScopeEnd)
        {
            checkScopeEndDefers(elem, elemCharacterPos);
        }
        else if (elem.type == ElemType::Return)
        {
            checkReturnDefers(elem, elemCharacterPos);
        }
        else if ((elem.type == ElemType::Break) || (elem.type == ElemType::Continue))
        {
            checkBreakContinueDefers(elem, elemCharacterPos);
        }
        else if (elem.type == ElemType::LoopStart)
        {
            mLoopStack.push_back(elem.astLevel);
        }
        else if (elem.type == ElemType::LoopEnd)
        {
            mActiveDefers.erase(
                std::remove_if(mActiveDefers.begin(),
                               mActiveDefers.end(),
                               [&e = elem.astLevel](const AstDeferNodePair &p) -> bool { return (e == p.first); }),
                mActiveDefers.end());

            mLoopStack.erase(std::remove(mLoopStack.begin(), mLoopStack.end(), elem.astLevel), mLoopStack.end());
        }
    }

    return mDeferFires;
}

void WalkerDefer::checkScopeEndDefers(const ProgramElem &elem, const uint32_t elemCharacterPos)
{
    auto it = mActiveDefers.begin();
    while (it != mActiveDefers.end())
    {
        if (it->first == elem.astLevel)
        {
            safec::log::log("SCOPE END: defer fire - astLevel: %, pos: %, text: '%'")
                .arg(elem.astLevel)
                .arg(elemCharacterPos)
                .arg(it->second->getDeferredText());

            it = mActiveDefers.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void WalkerDefer::checkReturnDefers(const ProgramElem &elem, const uint32_t elemCharacterPos)
{
    auto it = mActiveDefers.begin();
    while (it != mActiveDefers.end())
    {
        if (it->first < elem.astLevel)
        {
            safec::log::log("RETURN: defer fire - astLevel: %, pos: %, text: '%'")
                .arg(elem.astLevel)
                .arg(elemCharacterPos)
                .arg(it->second->getDeferredText());
        }

        it++;
    }
}

void WalkerDefer::checkBreakContinueDefers(const ProgramElem &elem, const uint32_t elemCharacterPos)
{
    const uint32_t currentLoopAstLevel = mLoopStack.back();

    auto it = mActiveDefers.begin();
    while (it != mActiveDefers.end())
    {
        if (it->first > currentLoopAstLevel)
        {
            safec::log::log("BREAK/CONTINUE: defer fire - astLevel: %, pos: %, text: '%'")
                .arg(elem.astLevel)
                .arg(elemCharacterPos)
                .arg(it->second->getDeferredText());
        }

        it++;
    }
}

} // namespace safec
