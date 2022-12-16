#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace safec
{

class SemNodeWalker;

// Magic X-Macro to generate enum & enum to string converter.
// clang-format off
#define SEMNODE_TYPE_ENUMERATE(selector) \
        selector(Undefined) \
        selector(TranslationUnit) \
        selector(Loop) \
        selector(Scope) \
        selector(Function) \
        selector(RawText) \
        selector(Defer)

#define SEMNODE_TYPE_SELECTOR_VALUE(x) x,
#define SEMNODE_TYPE_SELECTOR_VALUE_TO_STR(x) \
    case Type::x: { return #x; } break;
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

class SemNodeDefer final : public SemNode
{
public:
    SemNodeDefer(const uint32_t index);
    uint32_t getPos() const;

private:
    uint32_t mDeferEndIndex;
};

} // namespace safec
