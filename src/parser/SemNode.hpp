#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace safec
{

class SemNodePrinter;

class SemNode
{
public:
    enum class Type : uint32_t
    {
        UNDEFINED = 1,
        TRANSLATION_UNIT,
        LOOP,
        SCOPE,
        FUNCTION,
        RAW_TEXT,
        DEFER_CALL
    };

    SemNode() //
        : mType{Type::UNDEFINED}
    {
    }

    SemNode(const Type type) //
        : mType{type}
    {
    }

    virtual ~SemNode() = default;

    void reset()
    {
        mRelatedNodes.clear();
    }

    void setType(const Type type)
    {
        // TODO: allow only some changes (like scope --> function)
        mType = type;
    }

    Type getType() const
    {
        return mType;
    }

    void attach(std::shared_ptr<SemNode> node)
    {
        mRelatedNodes.push_back(node);
    }

protected:
    Type mType;
    std::vector<std::shared_ptr<SemNode>> mRelatedNodes;

    friend class SemNodePrinter;
};

class SemNodeScope : public SemNode
{
public:
    SemNodeScope(const uint32_t start) //
        : mStartIndex{start}, mEndIndex{0}
    {
        mType = Type::SCOPE;
    }

    void setEnd(const uint32_t end)
    {
        mEndIndex = end;
    }

    uint32_t getStart() const
    {
        return mStartIndex;
    }

    uint32_t getEnd() const
    {
        return mEndIndex;
    }

protected:
    uint32_t mStartIndex;
    uint32_t mEndIndex;
};

class SemNodeFunction final : public SemNodeScope
{
public:
    SemNodeFunction(const uint32_t start) //
        : SemNodeScope{start}             //
    {
        mType = Type::FUNCTION;
    }
};

class SemNodeDefer final : public SemNode
{
public:
    SemNodeDefer(const uint32_t index) : mDeferEndIndex{index}
    {
        mType = Type::DEFER_CALL;
    }

    uint32_t getPos() const
    {
        return mDeferEndIndex;
    }

private:
    uint32_t mDeferEndIndex;
};

} // namespace safec
