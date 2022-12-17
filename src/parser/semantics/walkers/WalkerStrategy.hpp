#pragma once

#include "semantics/SemNode.hpp"
#include "semantics/SemNodeEnumeration.hpp"

#include <cassert>
#include <cstdint>

namespace safec
{

// clang-format off
#define SEMNODE_TYPE_SELECTOR_WALKER_PEEKERS_INTERFACE_CREATE(x) \
    virtual void peek(SemNode##x &node, const uint32_t astLevel) { peekDefault(node, astLevel); }
// clang-format on

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
