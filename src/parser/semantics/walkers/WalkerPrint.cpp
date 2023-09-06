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

void WalkerPrint::peek(SemNodePositional &node, const uint32_t astLevel)
{
    log("% { % }", Color::Magenta) //
        .arg(getPrefix(node, astLevel))
        .arg(node.getPos());
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
    std::string paramsListStr;
    const auto params = node.getParams();

    if (params.size() == 0)
    {
        paramsListStr = "no args";
    }

    for (auto &it : params)
    {
        paramsListStr += "[";
        paramsListStr += it.mType;
        paramsListStr += " ";
        paramsListStr += it.mName;
        paramsListStr += "]";
    }

    log("% (ret: [%] name: [%] args: [%]) { %, % }", Color::LightCyan) //
        .arg(getPrefix(node, astLevel))
        .arg(node.getReturn())
        .arg(node.getName())
        .arg(paramsListStr)
        .arg(node.getStart())
        .arg(node.getEnd());
}

void WalkerPrint::peek(SemNodeLoop &node, const uint32_t astLevel)
{
    log("% % { % }", Color::LightCyan) //
        .arg(getPrefix(node, astLevel))
        .arg(node.toStr())
        .arg(node.getPos());
}

void WalkerPrint::peek(SemNodeReturn &node, const uint32_t astLevel)
{
    peekDefault<SemNodePositional>(node, astLevel);
}

void WalkerPrint::peek(SemNodeBreak &node, const uint32_t astLevel)
{
    peekDefault<SemNodePositional>(node, astLevel);
}

void WalkerPrint::peek(SemNodeContinue &node, const uint32_t astLevel)
{
    peekDefault<SemNodePositional>(node, astLevel);
}

void WalkerPrint::peek(SemNodeDeclaration &node, const uint32_t astLevel)
{
    log("% '%' { % }", Color::Green) //
        .arg(getPrefix(node, astLevel))
        .arg(node.toStr())
        .arg(node.getPos());
}

void WalkerPrint::peek(SemNodePostfixExpression &node, const uint32_t astLevel)
{
    log("% '% %' { % }", Color::LightYellow) //
        .arg(getPrefix(node, astLevel))
        .arg(node.getLhs())
        .arg(node.getOperator())
        .arg(node.getPos());
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
