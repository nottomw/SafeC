#include "SemNodeWalker.hpp"

#include "WalkerStrategy.hpp"
#include "semantics/SemNode.hpp"
#include "semantics/SemNodeEnumeration.hpp"

namespace safec
{

void SemNodeWalker::walk(SemNode &node, WalkerStrategy &strategy)
{
    walkLeveled(node, strategy, 0);
}

void SemNodeWalker::walkLeveled(SemNode &node, WalkerStrategy &strategy, const uint32_t level)
{
    // Create switch..case for each SemNode type.
    // clang-format off
    #define SEMNODE_TYPE_SELECTOR_TYPED_PEEK_CALL(x) \
        case SemNode::Type::x: \
        { \
            using NodeType = SemNode##x; \
            NodeType &nodeTyped = static_cast<NodeType &>(node); \
            strategy.peek(nodeTyped, level); \
        } \
        break;
    // clang-format on

    switch (node.getType())
    {
        SEMNODE_TYPE_ENUMERATE(SEMNODE_TYPE_SELECTOR_TYPED_PEEK_CALL)

        default:
            throw std::runtime_error("invalid SemNode type");
            break;
    }

    // TODO: walker need to be able to handle the new AST,
    // this means i.e. in BinaryOp have to traverse into rhs and lhs

    for (const auto &it : node.mRelatedNodes)
    {
        const uint32_t newLevel = (level + 1U);
        walkLeveled(*it, strategy, newLevel);
    }
}

} // namespace safec
