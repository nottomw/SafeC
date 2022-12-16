#pragma once

#include "semantics/SemNodeEnumeration.hpp"

#include <cassert>
#include <cstdint>

namespace safec
{

// clang-format off
#define SEMNODE_TYPE_SELECTOR_FORWARD_DECLARE(x) \
    class SemNode##x;
// clang-format on

// clang-format off
#define SEMNODE_TYPE_SELECTOR_WALKER_PEEKERS_INTERFACE_CREATE(x) \
    virtual void peek([[maybe_unused]] SemNode##x &node, [[maybe_unused]] const uint32_t astLevel) { assert(nullptr == "peek method not implemented" ); }
// clang-format on

SEMNODE_TYPE_ENUMERATE(SEMNODE_TYPE_SELECTOR_FORWARD_DECLARE)

class WalkerStrategy
{
public:
    virtual void peek( //
        [[maybe_unused]] SemNode &node,
        [[maybe_unused]] const uint32_t astLevel)
    {
        assert(nullptr == "peek method not implemented");
    }

    SEMNODE_TYPE_ENUMERATE(SEMNODE_TYPE_SELECTOR_WALKER_PEEKERS_INTERFACE_CREATE)

protected:
    // Helper template to forward the typed call to common base call.
    template <typename TNode>
    void peekDefault(TNode &node, const uint32_t astLevel)
    {
        peek(static_cast<SemNode &>(node), astLevel);
    }
};

} // namespace safec
