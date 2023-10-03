#pragma once

#include "semantic_nodes/SemNode.hpp"

#include <cassert>
#include <cstdint>

namespace safec
{

// clang-format off
#define SEMNODE_TYPE_SELECTOR_WALKER_PEEKERS_INTERFACE_CREATE(x) \
    virtual void peek(SemNode##x &node, const uint32_t astLevel) { peekDefault(node, astLevel); }
// clang-format on

// TODO: split walkers into readers and writers, readers
// can be run in parallel or in a single pass together

class WalkerStrategy
{
public:
    virtual ~WalkerStrategy() = default;

    virtual void peek( //
        [[maybe_unused]] SemNode &node,
        [[maybe_unused]] const uint32_t astLevel)
    {
        assert(nullptr == "default peeker not implemented");
    }

    // peek for internal "positional" type
    virtual void peek(SemNodePositional &node, const uint32_t astLevel)
    {
        peekDefault(node, astLevel);
    }

    SEMNODE_TYPE_ENUMERATE(SEMNODE_TYPE_SELECTOR_WALKER_PEEKERS_INTERFACE_CREATE)

protected:
    // Helper template to forward the typed call to common base call.
    template <typename TNodeTarget = SemNode>
    void peekDefault(SemNode &node, const uint32_t astLevel)
    {
        peek(static_cast<TNodeTarget &>(node), astLevel);
    }
};

} // namespace safec
