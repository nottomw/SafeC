#pragma once

#include "SemNodeWalker.hpp"
#include "WalkerStrategy.hpp"

namespace safec
{

class WalkerPrint : public WalkerStrategy
{
public:
    void peek(SemNode &node, const uint32_t astLevel) override;
    void peek(SemNodeFunction &node, const uint32_t astLevel) override;
    void peek(SemNodeScope &node, const uint32_t astLevel) override;
    void peek(SemNodeDefer &node, const uint32_t astLevel) override;

private:
    std::string getPrefix(const uint32_t astLevel);
};

} // namespace safec
