#include "WalkerDefer.hpp"

#include "logger/Logger.hpp"
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
    // Vector of astLevels where defer was seen
    using AstDeferNodePair = std::pair<uint32_t, const SemNodeDefer *>;
    std::vector<AstDeferNodePair> activeDefers;

    // TODO: cleanup defer fire analysis

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

            activeDefers.emplace_back(std::make_pair(elem.astLevel - 1, elem.defer));
            continue;
        }

        if (elem.type == ElemType::ScopeEnd)
        {
            std::cout << elem.astLevel << " ~~ scope end at " << pos << std::endl;

            auto it = activeDefers.begin();
            while (it != activeDefers.end())
            {
                if (it->first == elem.astLevel)
                {
                    safec::log::log("SCOPE END: found matching defer on astLevel: %, pos: %, text: '%'")
                        .arg(elem.astLevel)
                        .arg(pos)
                        .arg(it->second->getDeferredText());

                    it = activeDefers.erase(it);
                }
                else
                {
                    it++;
                }
            }
        }
        else if (elem.type == ElemType::Return)
        {
            std::cout << elem.astLevel << " ~~ return at " << pos << std::endl;

            auto it = activeDefers.begin();
            while (it != activeDefers.end())
            {
                if (it->first < elem.astLevel)
                {
                    safec::log::log("RETURN: found matching defer on astLevel: %, pos: %, text: '%'")
                        .arg(elem.astLevel)
                        .arg(pos)
                        .arg(it->second->getDeferredText());
                }

                it++;
            }
        }
        else if ((elem.type == ElemType::Break) || (elem.type == ElemType::Continue))
        {
            std::cout << elem.astLevel << " ~~ break/continue at " << pos << std::endl;

            const uint32_t currentLoopAstLevel = mLoopStack.back();

            auto it = activeDefers.begin();
            while (it != activeDefers.end())
            {
                if (it->first > currentLoopAstLevel)
                {
                    safec::log::log("BREAK/CONTINUE: found matching defer on astLevel: %, pos: %, text: '%'")
                        .arg(elem.astLevel)
                        .arg(pos)
                        .arg(it->second->getDeferredText());
                }

                it++;
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

            activeDefers.erase(
                std::remove_if(activeDefers.begin(),
                               activeDefers.end(),
                               [&e = elem.astLevel](const AstDeferNodePair &p) -> bool { return (e == p.first); }),
                activeDefers.end());

            mLoopStack.erase(std::remove(mLoopStack.begin(), mLoopStack.end(), elem.astLevel), mLoopStack.end());
        }
    }

    return mDeferFires;
}

} // namespace safec
