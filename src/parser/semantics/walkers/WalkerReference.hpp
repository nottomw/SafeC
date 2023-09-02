#pragma once

#include "SemNodeWalker.hpp"
#include "WalkerStrategy.hpp"

namespace safec
{

class WalkerReference final : public WalkerStrategy
{
public:
    void peek(SemNode &node, const uint32_t astLevel) override;
    void peek(SemNodePositional &node, const uint32_t astLevel) override;
    void peek(SemNodeScope &node, const uint32_t astLevel) override;
    void peek(SemNodeFunction &node, const uint32_t astLevel) override;
    void peek(SemNodeDefer &node, const uint32_t astLevel) override;
    void peek(SemNodeLoop &node, const uint32_t astLevel) override;

    void peek(SemNodeReturn &node, const uint32_t astLevel) override;
    void peek(SemNodeBreak &node, const uint32_t astLevel) override;
    void peek(SemNodeContinue &node, const uint32_t astLevel) override;

    void peek(SemNodeReference &node, const uint32_t astLevel) override;
};

} // namespace safec