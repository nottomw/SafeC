#pragma once

#include "SemNodeWalker.hpp"
#include "WalkerStrategy.hpp"

#include <map>
#include <vector>

namespace safec
{

class WalkerDefer final : public WalkerStrategy
{
public:
    void peek(SemNode &node, const uint32_t astLevel) override;
    void peek(SemNodeScope &node, const uint32_t astLevel) override;
    void peek(SemNodeFunction &node, const uint32_t astLevel) override;
    void peek(SemNodeLoop &node, const uint32_t astLevel) override;
    void peek(SemNodeDefer &node, const uint32_t astLevel) override;
    void peek(SemNodeReturn &node, const uint32_t astLevel) override;
    void peek(SemNodeBreak &node, const uint32_t astLevel) override;
    void peek(SemNodeContinue &node, const uint32_t astLevel) override;

    // TODO: return modpoints
    using DeferFirePair = std::pair<uint32_t, std::string>;
    using DeferFiresVector = std::vector<DeferFirePair>;
    DeferFiresVector getDeferFires();

    // pair: defer index, defer statement length
    using DeferRemovesVector = std::vector<std::pair<uint32_t, uint32_t>>;
    DeferRemovesVector getDeferRemoves();

private:
    enum class ElemType
    {
        ScopeEnd,
        LoopStart,
        LoopEnd,
        Return,
        Break,
        Continue,
        Defer
    };

    using AstLevel = uint32_t;
    using AstDeferNodePair = std::pair<AstLevel, const SemNodeDefer *>;

    struct ProgramElem
    {
        ProgramElem(const ElemType type, const AstLevel ast)
            : mType{type}
            , mAstLevel{ast}
        {
        }

        ProgramElem(const ElemType type, const AstLevel ast, const SemNodeDefer *const deferNode)
            : mType{type}
            , mAstLevel{ast}
            , mDefer{deferNode}
        {
        }

        ElemType mType;
        AstLevel mAstLevel;
        const SemNodeDefer *mDefer;
    };

    void checkScopeEndDefers(const ProgramElem &elem, const uint32_t elemCharacterPos);
    void checkReturnDefers(const ProgramElem &elem, const uint32_t elemCharacterPos);
    void checkBreakContinueDefers(const uint32_t elemCharacterPos);

    void reverseOrderOfSamePosDeferFire();

    std::multimap<AstLevel, ProgramElem> mProgramStructure;
    std::vector<AstLevel> mLoopStack;

    std::vector<AstDeferNodePair> mActiveDefers;

    DeferFiresVector mDeferFires;
};

} // namespace safec
