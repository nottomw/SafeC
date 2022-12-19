#include "WalkerDefer.hpp"

#include "semantics/SemNode.hpp"

#include <algorithm>

namespace safec
{

void WalkerDefer::peek(SemNode &node, const uint32_t)
{
    std::cout << "not interested: " << SemNode::TypeInfo::toStr(node.getType()) << std::endl;
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
    mProgramStructure.emplace(node.getPos(), ProgramElem{ElemType::Defer, astLevel});
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
    // Vector of astLevels where defer was seen
    std::vector<uint32_t> activeDefers;

    // TODO: cleanup defer fire analysis

    auto removeFromDefers = [&activeDefers](
                                const uint32_t astLevel,
                                std::function<bool(const uint32_t, const uint32_t)> cmp =
                                    [](const uint32_t a, const uint32_t b) { return a == b; }) {
        auto it = activeDefers.begin();
        while (it != activeDefers.end())
        {
            auto itAstLevel = *it;
            if (cmp(itAstLevel, astLevel))
            {
                it = activeDefers.erase(it);
            }
            else
            {
                it++;
            }
        }
    };

    // TODO: grab the defer function call

    for (const auto &it : mProgramStructure)
    {
        for (uint32_t i = 0; i < it.second.astLevel; ++i)
        {
            std::cout << "\t";
        }

        std::cout << it.first << " PROGRAM: " << static_cast<uint32_t>(it.second.type) << std::endl;
    }

    for (const auto &it : mProgramStructure)
    {
        const uint32_t pos = it.first;
        const ProgramElem &elem = it.second;

        std::cout << "------------------------------------------- DEFER COUNT: " << activeDefers.size() << std::endl;

        if (elem.type == ElemType::Defer)
        {
            std::cout << elem.astLevel << " ~~ defer at " << pos << " active in astLevel: " << (elem.astLevel - 1)
                      << std::endl;

            activeDefers.push_back(elem.astLevel - 1);
            continue;
        }

        if (elem.type == ElemType::ScopeEnd)
        {
            std::cout << elem.astLevel << " ~~ scope end at " << pos << std::endl;

            const uint32_t defersInThisScope = std::count(activeDefers.begin(), activeDefers.end(), elem.astLevel);
            if (defersInThisScope > 0)
            {
                std::cout << "scope end, firing " << defersInThisScope << " defers\n";
                removeFromDefers(elem.astLevel);
            }
        }
        else if (elem.type == ElemType::Return)
        {
            std::cout << elem.astLevel << " ~~ return at " << pos << std::endl;

            auto cond = [&astLevel = elem.astLevel](const uint32_t astLevelDefer) {
                return (astLevel > astLevelDefer);
            };

            const uint32_t defersInThisScope = std::count_if(activeDefers.begin(), activeDefers.end(), cond);
            if (defersInThisScope > 0)
            {
                std::cout << "return, firing " << defersInThisScope << " defers\n";
            }
        }
        else if ((elem.type == ElemType::Break) || (elem.type == ElemType::Continue))
        {
            std::cout << elem.astLevel << " ~~ break/continue at " << pos << std::endl;

            const uint32_t currentLoopAstLevel = mLoopStack.back();
            auto cond = [&currentLoopAstLevel](const uint32_t astLevelDefer) {
                return (currentLoopAstLevel < astLevelDefer);
            };

            const uint32_t defersInThisScope = std::count_if(activeDefers.begin(), activeDefers.end(), cond);
            if (defersInThisScope > 0)
            {
                std::cout << "break/continue, firing " << defersInThisScope << " defers\n";
            }
        }
        else if (elem.type == ElemType::LoopStart)
        {
            std::cout << elem.astLevel << " ~~ loop start at " << pos << std::endl;

            mLoopStack.push_back(elem.astLevel);
        }
        else if (elem.type == ElemType::LoopEnd)
        {
            std::cout << elem.astLevel << " ~~ loop end at " << pos << std::endl;

            removeFromDefers(elem.astLevel);

            mLoopStack.erase(std::remove(mLoopStack.begin(), mLoopStack.end(), elem.astLevel), mLoopStack.end());
        }
    }

    return mDeferFires;
}

} // namespace safec
