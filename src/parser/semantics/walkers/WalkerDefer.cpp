#include "WalkerDefer.hpp"

#include "ModPoint.hpp"
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
    mProgramStructure.emplace(node.getEnd(), ProgramElem{ElemType::ScopeEnd, astLevel, &node});
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

ModPointsVector WalkerDefer::getModPoints()
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
                    mActiveDefers.emplace_back(std::make_pair(deferActiveAtLevel, elem.mDeferNode));
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

    reverseOrderOfSamePosDeferFire();

    // add removes of fired defers
    for (const auto &it : mProgramStructure)
    {
        if (it.second.mType == ElemType::Defer)
        {
            const SemNodeDefer *const node = it.second.mDeferNode;
            constexpr uint32_t deferOffset = 1U;
            const uint32_t newPos = node->getPos() - deferOffset;
            mModPoints.emplace_back(ModType::TextRemove, newPos, node->getDeferredStatementLen());
        }
    }

    return mModPoints;
}

void WalkerDefer::checkScopeEndDefers(const ProgramElem &elem, const uint32_t elemCharacterPos)
{
    auto it = mActiveDefers.begin();
    while (it != mActiveDefers.end())
    {
        if (it->first == elem.mAstLevel)
        {
            // scope end - if this is actually function end, and the function is
            // non-void (returns some value), then we do not fire the defer call
            // but still erase the defer - the last "defer" call should happen on the last
            // return.

            // TODO: for now commented out, changes related to AST
            //            if ((elem.mFunctionNode == nullptr) || //
            //                (elem.mFunctionNode->getIsVoidReturnType() == true))
            //            {
            //                constexpr uint32_t scopeEndOffset = 2U;
            //                const uint32_t newPos = elemCharacterPos - scopeEndOffset;
            //                mModPoints.emplace_back(ModType::TextInsert, newPos, it->second->getDeferredText());
            //            }

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
            mModPoints.emplace_back(ModType::TextInsert, elemCharacterPosWithOffset, it->second->getDeferredText());
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
            mModPoints.emplace_back(ModType::TextInsert, elemCharacterPosWithOffset, it->second->getDeferredText());
        }

        it++;
    }
}

void WalkerDefer::reverseOrderOfSamePosDeferFire()
{
    // reverse defers fire order on the same index/file position, so the deferred calls behave consistently
    // e.g.:
    //      defer call1(); defer call2(); defer call3();
    // will result in:
    //      call3(); call2(); call1();

    // this probably can be done in a nicer way, but going with this for now...

    std::vector<std::vector<uint32_t>> sameLineDefers;

    for (uint32_t i = 1U; i < mModPoints.size(); ++i)
    {
        // assuming all text inserts done before text removes
        assert((mModPoints[i].getModType() == ModType::TextInsert) &&
               (mModPoints[i - 1].getModType() == ModType::TextInsert));

        if (mModPoints[i - 1].getStart() == mModPoints[i].getStart())
        {
            std::vector<uint32_t> defersAtThisFilePosition;
            defersAtThisFilePosition.push_back(i - 1);
            defersAtThisFilePosition.push_back(i);
            i++;

            while ((i < mModPoints.size()) && //
                   (mModPoints[i - 1].getStart() == mModPoints[i].getStart()))
            {
                assert((mModPoints[i].getModType() == ModType::TextInsert) &&
                       (mModPoints[i - 1].getModType() == ModType::TextInsert));

                defersAtThisFilePosition.push_back(i);
                i++;
            }

            sameLineDefers.push_back(std::move(defersAtThisFilePosition));
        }
    }

    for (auto &it : sameLineDefers)
    {
        uint32_t forwardIndex = 0U;
        uint32_t backwardIndex = it.size() - 1U;
        while (forwardIndex < backwardIndex)
        {
            std::swap(mModPoints[it[forwardIndex]], mModPoints[it[backwardIndex]]);

            forwardIndex++;
            backwardIndex--;
        }
    }
}

} // namespace safec
