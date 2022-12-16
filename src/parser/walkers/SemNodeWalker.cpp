#include "SemNodeWalker.hpp"

#include "SemNode.hpp"
#include "WalkerStrategy.hpp"

namespace safec
{

void SemNodeWalker::walk(SemNode &node, WalkerStrategy &strategy)
{
    walkLeveled(node, strategy, 0);
}

void SemNodeWalker::walkLeveled(SemNode &node, WalkerStrategy &strategy, const uint32_t level)
{
    // Could be reworked to visitor but all nodes would have to have accept
    // implementation with ast level management... maybe later

    switch (node.getType())
    {
    case SemNode::Type::UNDEFINED:
        strategy.peek(node, level);
        break;
    case SemNode::Type::TRANSLATION_UNIT:
        strategy.peek(node, level);
        break;
    case SemNode::Type::LOOP:
        strategy.peek(node, level);
        break;
    case SemNode::Type::SCOPE: {
        SemNodeScope &scope = static_cast<SemNodeScope &>(node);
        strategy.peek(scope, level);
    }
    break;
    case SemNode::Type::FUNCTION: {
        SemNodeFunction &fun = static_cast<SemNodeFunction &>(node);
        strategy.peek(fun, level);
    }
    break;
    case SemNode::Type::RAW_TEXT:
        strategy.peek(node, level);
        break;
    case SemNode::Type::DEFER_CALL: {
        SemNodeDefer &def = static_cast<SemNodeDefer &>(node);
        strategy.peek(def, level);
    }
    break;
    default:
        throw std::runtime_error("invalid SemNode type");
        break;
    }

    for (const auto &it : node.mRelatedNodes)
    {
        const uint32_t newLevel = (level + 1U);
        walkLeveled(*it, strategy, newLevel);
    }
}

} // namespace safec
