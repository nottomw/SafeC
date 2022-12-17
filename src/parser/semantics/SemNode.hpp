#pragma once

#include "SemNodeEnumeration.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace safec
{

class SemNodeWalker;

// clang-format off
#define SEMNODE_TYPE_SELECTOR_VALUE(x) x,
#define SEMNODE_TYPE_SELECTOR_VALUE_TO_STR(x)                                                                          \
    case Type::x: {                                                                                                    \
        return #x;                                                                                                     \
    }                                                                                                                  \
    break;
// clang-format on

class SemNode
{
public:
    enum class Type : uint32_t
    {
        SEMNODE_TYPE_ENUMERATE(SEMNODE_TYPE_SELECTOR_VALUE)
    };

    struct TypeInfo
    {
        static constexpr std::string_view toStr(const Type type)
        {
            switch (type)
            {
                SEMNODE_TYPE_ENUMERATE(SEMNODE_TYPE_SELECTOR_VALUE_TO_STR)
            }

            return "undefined";
        }
    };

    SemNode();
    SemNode(const Type type);
    virtual ~SemNode() = default;
    void reset();
    Type getType() const;
    void attach(std::shared_ptr<SemNode> node);

protected:
    Type mType;
    std::vector<std::shared_ptr<SemNode>> mRelatedNodes;

    friend class SemNodeWalker;
};

class SemNodeTranslationUnit : public SemNode
{
public:
    SemNodeTranslationUnit();
};

// Semantic node with dual position info (start & end).
class SemNodeScope : public SemNode
{
public:
    SemNodeScope(const uint32_t start);
    void setEnd(const uint32_t end);
    uint32_t getStart() const;
    uint32_t getEnd() const;

protected:
    uint32_t mStartIndex;
    uint32_t mEndIndex;
};

class SemNodeFunction final : public SemNodeScope
{
public:
    SemNodeFunction(const uint32_t start);
};

// Semantic node with a single position info.
class SemNodePositional : public SemNode
{
public:
    SemNodePositional(const uint32_t pos);

    uint32_t getPos() const;

private:
    uint32_t mPos;
};

class SemNodeDefer final : public SemNodePositional
{
public:
    SemNodeDefer(const uint32_t index);
};

class SemNodeReturn final : public SemNodePositional
{
public:
    SemNodeReturn(const uint32_t index);
};

class SemNodeLoop : public SemNodeScope
{
public:
    SemNodeLoop(const uint32_t start);
};

class SemNodeBreak : public SemNodePositional
{
public:
    SemNodeBreak(const uint32_t pos);
};

class SemNodeContinue : public SemNodePositional
{
public:
    SemNodeContinue(const uint32_t pos);
};

// clang-format off
class SemNodeUndefined : public SemNode{};
class SemNodeRawText : public SemNode{};
// clang-format on

} // namespace safec
