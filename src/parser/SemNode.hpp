#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace safec
{

class SemNodeWalker;

class SemNode
{
public:
    // TODO: x-macro to have stringified versions
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
