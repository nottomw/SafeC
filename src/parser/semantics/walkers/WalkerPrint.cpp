#include "WalkerPrint.hpp"

#include "semantics/SemNode.hpp"

#include <iostream>

namespace safec
{

void WalkerPrint::peek(SemNode &node, const uint32_t astLevel)
{
    std::cout << getPrefix(node, astLevel) << std::endl;
}

void WalkerPrint::peek(SemNodeTranslationUnit &node, const uint32_t astLevel)
{
    peekDefault(node, astLevel);
}

void WalkerPrint::peek(SemNodeScope &node, const uint32_t astLevel)
{
    std::cout << getPrefix(node, astLevel) << " { " << node.getStart() << ", " << node.getEnd() << " } " << std::endl;
}

void WalkerPrint::peek(SemNodeFunction &node, const uint32_t astLevel)
{
    std::cout << getPrefix(node, astLevel) << " { " << node.getStart() << ", " << node.getEnd() << " } " << std::endl;
}

void WalkerPrint::peek(SemNodeDefer &node, const uint32_t astLevel)
{
    std::cout << getPrefix(node, astLevel) << " { " << node.getPos() << " }" << std::endl;
}

void WalkerPrint::peek(SemNodeReturn &node, const uint32_t astLevel)
{
    peekDefault(node, astLevel);
}

void WalkerPrint::peek(SemNodeLoop &node, const uint32_t astLevel)
{
    peekDefault<SemNodeScope>(node, astLevel);
}

void WalkerPrint::peek(SemNodeBreak &node, const uint32_t astLevel)
{
    peekDefault(node, astLevel);
}

void WalkerPrint::peek(SemNodeContinue &node, const uint32_t astLevel)
{
    peekDefault(node, astLevel);
}

std::string WalkerPrint::getPrefix(SemNode &node, const uint32_t astLevel)
{
    std::string prefix{};

    for (uint32_t i = 0U; i < astLevel; ++i)
    {
        prefix.append("\t");
    }

    prefix.append("[");
    prefix.append(std::to_string(astLevel));
    prefix.append("] ");
    prefix.append(SemNode::TypeInfo::toStr(node.getType()));

    return prefix;
}

} // namespace safec
