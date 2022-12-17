#include "WalkerDefer.hpp"

#include "semantics/SemNode.hpp"

#include <vector>

namespace safec
{

void WalkerDefer::peek(SemNode &, const uint32_t)
{
    // not interested
}

void WalkerDefer::peek(SemNodeScope &, const uint32_t)
{
    // potentially fire deferred call
}

void WalkerDefer::peek(SemNodeFunction &, const uint32_t)
{
    // potentially fire deferred call
}

void WalkerDefer::peek(SemNodeDefer &, const uint32_t astLevel)
{
    const uint32_t deferScopeLevel = (astLevel - 1U);
    std::cout << "defer at astLevel: " << deferScopeLevel << std::endl;
}

void WalkerDefer::peek(SemNodeReturn &, const uint32_t)
{
    // potentially fire deferred call
}

void WalkerDefer::peek(SemNodeBreak &, const uint32_t)
{
    // potentially fire deferred call
}

void WalkerDefer::peek(SemNodeContinue &, const uint32_t)
{
    // potentially fire deferred call
}

} // namespace safec
