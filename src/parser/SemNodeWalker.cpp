#include "SemNodeWalker.hpp"

#include "SemNode.hpp"

namespace safec
{

SemNodePrinter::SemNodePrinter() : mScopeLevel{0}
{
}

void SemNodePrinter::walk(SemNode &node)
{
    std::string prefix;
    for (uint32_t i = 0U; i < mScopeLevel; ++i)
    {
        prefix.append("\t");
    }

    // TODO: figure out something smarter
    switch (node.getType())
    {
    case SemNode::Type::UNDEFINED:
        std::cout << prefix << "UNDEFINED" << std::endl;
        break;
    case SemNode::Type::TRANSLATION_UNIT:
        std::cout << prefix << "TRANSLATION_UNIT" << std::endl;
        break;
    case SemNode::Type::LOOP:
        std::cout << prefix << "LOOP" << std::endl;
        break;
    case SemNode::Type::SCOPE: {
        SemNodeScope &scope = static_cast<SemNodeScope &>(node);
        std::cout << prefix << "SCOPE { " << scope.getStart() << ", " << scope.getEnd() << " } " << std::endl;
    }
    break;
    case SemNode::Type::FUNCTION: {
        SemNodeFunction &fun = static_cast<SemNodeFunction &>(node);
        std::cout << prefix << "FUNCTION { " << fun.getStart() << ", " << fun.getEnd() << " } " << std::endl;
    }
    break;
    case SemNode::Type::RAW_TEXT:
        std::cout << prefix << "RAW_TEXT" << std::endl;
        break;
    case SemNode::Type::DEFER_CALL: {
        SemNodeDefer &def = static_cast<SemNodeDefer &>(node);
        std::cout << prefix << "DEFER_CALL { " << def.getPos() << " }" << std::endl;
    }
    break;
    default:
        throw std::runtime_error("invalid SemNode type");
        break;
    }

    for (const auto &it : node.mRelatedNodes)
    {
        mScopeLevel += 1U;
        walk(*it);
        mScopeLevel -= 1U;
    }
}

} // namespace safec
