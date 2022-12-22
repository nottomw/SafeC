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
    mProgramStructure.emplace(node.getEnd(), ProgramElem{ElemType::ScopeEnd, astLevel});
}

void WalkerDefer::peek(SemNodeFunction &node, const uint32_t astLevel)
{
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

        switch (elem.mType)
        {
            case ElemType::Defer:
                {
                    const AstLevel deferActiveAtLevel = elem.mAstLevel - 1;
                    mActiveDefers.emplace_back(std::make_pair(deferActiveAtLevel, elem.mDefer));
                    continue;
                }
                break;

            case ElemType::ScopeEnd:
                checkScopeEndDefers(elem, elemCharacterPos);
                break;

            case ElemType::Return:
                checkReturnDefers(elem, elemCharacterPos);
                break;

            case ElemType::Break:
                [[fallthrough]];
            case ElemType::Continue:
                checkBreakContinueDefers(elemCharacterPos);
                break;

            case ElemType::LoopStart:
                mLoopStack.push_back(elem.mAstLevel);
                break;

            case ElemType::LoopEnd:
                {
                    mActiveDefers.erase(std::remove_if(mActiveDefers.begin(),
                                                       mActiveDefers.end(),
                                                       [&e = elem.mAstLevel](const AstDeferNodePair &p) -> bool {
                                                           return (e == p.first);
                                                       }),
                                        mActiveDefers.end());

                    mLoopStack.erase(std::remove(mLoopStack.begin(), mLoopStack.end(), elem.mAstLevel),
                                     mLoopStack.end());
                }
                break;

            default:
                assert(nullptr == "invalid elem.type");
                break;
        }
    }

    // TODO: defer on the same char should be in reverse order
    // TODO: precise defer positions - before closing bracket, before return, etc.
    // TODO: need to provide "removes" too: remove the "defer abc" call

    return mDeferFires;
}

WalkerDefer::DeferRemovesVector WalkerDefer::getDeferRemoves()
{
    DeferRemovesVector removes{};

    for (const auto &it : mProgramStructure)
    {
        if (it.second.mType == ElemType::Defer)
        {
            const SemNodeDefer *const node = it.second.mDefer;
            removes.emplace_back(node->getPos());
        }
    }

    return removes;
}

void WalkerDefer::checkScopeEndDefers(const ProgramElem &elem, const uint32_t elemCharacterPos)
{
    auto it = mActiveDefers.begin();
    while (it != mActiveDefers.end())
    {
        if (it->first == elem.mAstLevel)
        {
            constexpr uint32_t scopeEndOffset = 2U;
            const uint32_t newPos = elemCharacterPos - scopeEndOffset;
            mDeferFires.emplace_back(std::make_pair(newPos, it->second->getDeferredText()));
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
        if (it->first < elem.mAstLevel)
        {
            constexpr uint32_t returnOffset = 1U;
            const uint32_t elemCharacterPosWithOffset = elemCharacterPos - returnOffset;
            mDeferFires.emplace_back(std::make_pair(elemCharacterPosWithOffset, it->second->getDeferredText()));
        }

        it++;
    }
}

void WalkerDefer::checkBreakContinueDefers(const uint32_t elemCharacterPos)
{
    const uint32_t currentLoopAstLevel = mLoopStack.back();

    auto it = mActiveDefers.begin();
    while (it != mActiveDefers.end())
    {
        if (it->first > currentLoopAstLevel)
        {
            constexpr uint32_t breakContinueOffset = 1U;
            const uint32_t elemCharacterPosWithOffset = elemCharacterPos - breakContinueOffset;
            mDeferFires.emplace_back(std::make_pair(elemCharacterPosWithOffset, it->second->getDeferredText()));
        }

        it++;
    }
}

} // namespace safec
