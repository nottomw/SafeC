#pragma once

#include "SemNodeWalker.hpp"
#include "WalkerStrategy.hpp"

namespace safec
{

// Print the whole ast.
class WalkerPrint : public WalkerStrategy
{
public:
    void peek(SemNode &node, const uint32_t astLevel) override;
    void peek(SemNodeTranslationUnit &node, const uint32_t astLevel) override;
    void peek(SemNodeScope &node, const uint32_t astLevel) override;
    void peek(SemNodeFunction &node, const uint32_t astLevel) override;
    void peek(SemNodeDefer &node, const uint32_t astLevel) override;
    void peek(SemNodeReturn &node, const uint32_t astLevel) override;

private:
    std::string getPrefix(SemNode &node, const uint32_t astLevel);
};

} // namespace safec