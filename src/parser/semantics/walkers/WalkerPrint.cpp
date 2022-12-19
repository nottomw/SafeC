#include "WalkerPrint.hpp"

#include "logger/Logger.hpp"
#include "semantics/SemNode.hpp"

#include <iostream>

namespace safec
{

void WalkerPrint::peek(SemNode &node, const uint32_t astLevel)
{
    log("%", Color::Magenta).arg(getPrefix(node, astLevel));
}

void WalkerPrint::peek(SemNodeScope &node, const uint32_t astLevel)
{
    log("% { %, % }", Color::LightYellow) //
        .arg(getPrefix(node, astLevel))
        .arg(node.getStart())
        .arg(node.getEnd());
}

void WalkerPrint::peek(SemNodeFunction &node, const uint32_t astLevel)
{
    peekDefault<SemNodeScope>(node, astLevel);
}

void WalkerPrint::peek(SemNodeDefer &node, const uint32_t astLevel)
{
    log("% { % }", Color::LightGreen) //
        .arg(getPrefix(node, astLevel))
        .arg(node.getPos());
}

void WalkerPrint::peek(SemNodeLoop &node, const uint32_t astLevel)
{
    peekDefault<SemNodeScope>(node, astLevel);
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
