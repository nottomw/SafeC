#include "WalkerPrint.hpp"

#include "logger/Logger.hpp"
#include "semantics/SemNode.hpp"

#include <iostream>

namespace safec
{

// TODO: add a printer with graphical tree display? image generation?

void WalkerPrint::peek(SemNode &node, const uint32_t astLevel)
{
    log("%", Color::Magenta, getPrefix(node, astLevel));
}

void WalkerPrint::peek(SemNodePositional &node, const uint32_t astLevel)
{
    log("% { % }",
        Color::Magenta, //
        getPrefix(node, astLevel),
        node.getPos());
}

void WalkerPrint::peek(SemNodeScope &node, const uint32_t astLevel)
{
    log("% { %, % }",
        Color::Yellow, //
        getPrefix(node, astLevel),
        node.getStart(),
        node.getEnd());
}

void WalkerPrint::peek(SemNodeFunction &node, const uint32_t astLevel)
{
    log("% (%) { % -- % }",
        Color::Black,
        Color::BgYellow, //
        getPrefix(node, astLevel),
        node.toStr(),
        node.getStart(),
        node.getEnd());
}

void WalkerPrint::peek(SemNodeLoop &node, const uint32_t astLevel)
{
    log("% { % -- % }",
        Color::Black,
        Color::BgGreen, //
        getPrefix(node, astLevel),
        node.getStart(),
        node.getEnd());
}

void WalkerPrint::peek(SemNodeReturn &node, const uint32_t astLevel)
{
    log("% '%' { % }", //
        Color::White,
        Color::BgBlack,
        getPrefix(node, astLevel),
        node.toStr(),
        node.getPos());
}

void WalkerPrint::peek(SemNodeUnaryOp &node, const uint32_t astLevel)
{
    log("% '%' { % }",
        Color::Red, //
        getPrefix(node, astLevel),
        node.toStr(),
        node.getPos());
}

void WalkerPrint::peek(SemNodeJumpStatement &node, const uint32_t astLevel)
{
    log("% '%' { % }", //
        Color::White,
        Color::BgBlack,
        getPrefix(node, astLevel),
        node.toStr(),
        node.getPos());
}

void WalkerPrint::peek(SemNodeConstant &node, const uint32_t astLevel)
{
    log("% '%' { % }",
        Color::Magenta, //
        getPrefix(node, astLevel),
        node.getName(),
        node.getPos());
}

void WalkerPrint::peek(SemNodeDeclaration &node, const uint32_t astLevel)
{
    log("% '%' { % }",
        Color::Green, //
        getPrefix(node, astLevel),
        node.toStr(),
        node.getPos());
}

void WalkerPrint::peek(SemNodePostfixExpression &node, const uint32_t astLevel)
{
    log("% '%' { % }",
        Color::Yellow, //
        getPrefix(node, astLevel),
        node.toStr(),
        node.getPos());
}

void WalkerPrint::peek(SemNodeBinaryOp &node, const uint32_t astLevel)
{
    log("% '%' { % }",
        Color::Cyan, //
        getPrefix(node, astLevel),
        node.toStr(),
        node.getPos());
}

void WalkerPrint::peek(SemNodeIdentifier &node, const uint32_t astLevel)
{
    log("% '%' { % }",
        Color::Green, //
        getPrefix(node, astLevel),
        node.toStr(),
        node.getPos());
}

void WalkerPrint::peek(SemNodeIf &node, const uint32_t astLevel)
{
    log("% '%' { % -- % }",
        Color::Black,
        Color::BgMagenta,
        getPrefix(node, astLevel),
        node.toStr(),
        node.getStart(),
        node.getEnd());
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
