#include "WalkerPrint.hpp"

#include "logger/Logger.hpp"
#include "semantics/SemNode.hpp"

#include <iostream>

namespace safec
{

void WalkerPrint::peek(SemNode &node, const uint32_t astLevel)
{
    log("%", //
        Color::Magenta,
        getPrefix(node, astLevel));
}

void WalkerPrint::peek(SemNodePositional &node, const uint32_t astLevel)
{
    log("% { % }",
        Color::Magenta, //
        getPrefix(node, astLevel),
        getPos(node));
}

void WalkerPrint::peek(SemNodeScope &node, const uint32_t astLevel)
{
    log("% { % }",
        Color::Yellow, //
        getPrefix(node, astLevel),
        getPos(node));
}

void WalkerPrint::peek(SemNodeFunction &node, const uint32_t astLevel)
{
    log("% (%) { % }",
        Color::Black,
        Color::BgCyan, //
        getPrefix(node, astLevel),
        node.toStr(),
        getPos(node));
}

void WalkerPrint::peek(SemNodeLoop &node, const uint32_t astLevel)
{
    log("% % { % }",
        Color::Black,
        Color::BgGreen, //
        getPrefix(node, astLevel),
        node.getName(),
        getPos(node));
}

void WalkerPrint::peek(SemNodeReturn &node, const uint32_t astLevel)
{
    std::string returnedNodeType = "empty";

    auto returnedNode = node.getReturnedNode();
    if (returnedNode)
    {
        returnedNodeType = SemNode::TypeInfo::toStr(returnedNode->getType());
    }

    log("% '%' { % }", //
        Color::White,
        Color::BgBlack,
        getPrefix(node, astLevel),
        returnedNodeType,
        getPos(node));
}

void WalkerPrint::peek(SemNodeUnaryOp &node, const uint32_t astLevel)
{
    log("% '%' { % }",
        Color::Red, //
        getPrefix(node, astLevel),
        node.toStr(),
        getPos(node));
}

void WalkerPrint::peek(SemNodeJumpStatement &node, const uint32_t astLevel)
{
    log("% '%' { % }", //
        Color::White,
        Color::BgBlack,
        getPrefix(node, astLevel),
        node.toStr(),
        getPos(node));
}

void WalkerPrint::peek(SemNodeConstant &node, const uint32_t astLevel)
{
    log("% '%' { % }",
        Color::Magenta, //
        getPrefix(node, astLevel),
        node.getName(),
        getPos(node));
}

void WalkerPrint::peek(SemNodeDefer &node, const uint32_t astLevel)
{
    log("% { % }",
        Color::White, //
        Color::BgBlue,
        getPrefix(node, astLevel),
        getPos(node));
}

void WalkerPrint::peek(SemNodeSwitchCase &node, const uint32_t astLevel)
{
    log("% on '%' { % }",
        Color::Black,
        Color::BgYellow, //
        getPrefix(node, astLevel),
        SemNode::TypeInfo::toStr(node.getSwitchExpr()->getType()),
        getPos(node));
}

void WalkerPrint::peek(SemNodeSwitchCaseLabel &node, const uint32_t astLevel)
{
    const std::string caseLabelOn =                                       //
        (node.getCaseLabel()->getType() == SemNode::Type::EmptyStatement) //
            ? ("default")
            : std::string(SemNode::TypeInfo::toStr(node.getCaseLabel()->getType()));

    log("% on '%' %{ % }",
        Color::Black,
        Color::BgYellow, //
        getPrefix(node, astLevel),
        caseLabelOn,
        node.getIsFallthrough() ? "(fallthrough) " : "",
        getPos(node));
}

void WalkerPrint::peek(SemNodeDeclaration &node, const uint32_t astLevel)
{
    log("% '%' { % }",
        Color::Green, //
        getPrefix(node, astLevel),
        node.toStr(),
        getPos(node));
}

void WalkerPrint::peek(SemNodePostfixExpression &node, const uint32_t astLevel)
{
    log("% '%' { % }",
        Color::Yellow, //
        getPrefix(node, astLevel),
        node.toStr(),
        getPos(node));
}

void WalkerPrint::peek(SemNodeBinaryOp &node, const uint32_t astLevel)
{
    log("% '%' { % }",
        Color::Cyan, //
        getPrefix(node, astLevel),
        node.toStr(),
        getPos(node));
}

void WalkerPrint::peek(SemNodeIdentifier &node, const uint32_t astLevel)
{
    log("% '%' { % }",
        Color::Green, //
        getPrefix(node, astLevel),
        node.toStr(),
        getPos(node));
}

void WalkerPrint::peek(SemNodeIf &node, const uint32_t astLevel)
{
    log("% { % }", //
        Color::Black,
        Color::BgMagenta,
        getPrefix(node, astLevel),
        getPos(node));
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

std::string WalkerPrint::getPos(SemNode &node)
{
    std::string str;
    str += std::to_string(node.getSemStart());
    str += " -- ";
    str += std::to_string(node.getSemEnd());

    return str;
}

} // namespace safec
