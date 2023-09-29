#pragma once

#include "SemNodeWalker.hpp"
#include "WalkerStrategy.hpp"

namespace safec
{

// Print the whole ast.
class WalkerPrint final : public WalkerStrategy
{
public:
    void peek(SemNode &node, const uint32_t astLevel) override;
    void peek(SemNodePositional &node, const uint32_t astLevel) override;
    void peek(SemNodeScope &node, const uint32_t astLevel) override;
    void peek(SemNodeFunction &node, const uint32_t astLevel) override;
    void peek(SemNodeLoop &node, const uint32_t astLevel) override;

    void peek(SemNodeDeclaration &node, const uint32_t astLevel) override;

    void peek(SemNodePostfixExpression &node, const uint32_t astLevel) override;
    void peek(SemNodeBinaryOp &node, const uint32_t astLevel) override;
    void peek(SemNodeIdentifier &node, const uint32_t astLevel) override;
    void peek(SemNodeIf &node, const uint32_t astLevel) override;
    void peek(SemNodeReturn &node, const uint32_t astLevel) override;
    void peek(SemNodeUnaryOp &node, const uint32_t astLevel) override;
    void peek(SemNodeJumpStatement &node, const uint32_t astLevel) override;
    void peek(SemNodeConstant &node, const uint32_t astLevel) override;
    void peek(SemNodeDefer &node, const uint32_t astLevel) override;

private:
    std::string getPrefix(SemNode &node, const uint32_t astLevel);
};

} // namespace safec
