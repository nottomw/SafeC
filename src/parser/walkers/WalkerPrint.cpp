#include "WalkerPrint.hpp"

#include "SemNode.hpp"

#include <iostream>

namespace safec
{

void WalkerPrint::peek(SemNode &node, const uint32_t astLevel)
{
    std::cout << getPrefix(astLevel) << "NODE type: " << static_cast<uint32_t>(node.getType()) << std::endl;
}

void WalkerPrint::peek(SemNodeFunction &node, const uint32_t astLevel)
{
    std::cout << getPrefix(astLevel) << "FUNCTION { " << node.getStart() << ", " << node.getEnd() << " } " << std::endl;
}

void WalkerPrint::peek(SemNodeScope &node, const uint32_t astLevel)
{
    std::cout << getPrefix(astLevel) << "SCOPE { " << node.getStart() << ", " << node.getEnd() << " } " << std::endl;
}

void WalkerPrint::peek(SemNodeDefer &node, const uint32_t astLevel)
{
    std::cout << getPrefix(astLevel) << "DEFER_CALL { " << node.getPos() << " }" << std::endl;
}

std::string WalkerPrint::getPrefix(const uint32_t astLevel)
{
    std::string prefix;
    for (uint32_t i = 0U; i < astLevel; ++i)
    {
        prefix.append("\t");
    }

    return prefix;
}

} // namespace safec
