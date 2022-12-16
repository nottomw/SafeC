#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace safec
{

// TODO: add SemNodeVisitor

class SemNode
{
public:
    enum class Type : uint32_t
    {
        UNDEFINED = 1,
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

    virtual void display() const
    {
        std::cout << "SemNode type: " << static_cast<uint32_t>(mType) << std::endl;
        displayAttached();
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
    void displayAttached() const
    {
        if (mRelatedNodes.empty() == false)
        {
            for (const auto &it : mRelatedNodes)
            {
                it->display();
            }
        }
    }

    Type mType;
    std::vector<std::shared_ptr<SemNode>> mRelatedNodes;
};

class SemNodeScope : public SemNode
{
public:
    SemNodeScope(const uint32_t start) //
        : SemNode{Type::FUNCTION}      //
        , mStartIndex{start}
        , mEndIndex{0}
    {
    }

    void setEnd(const uint32_t end)
    {
        mEndIndex = end;
    }

    void display() const override
    {
        std::cout << "START SemNodeScope { start: " << mStartIndex << ", end: " << mEndIndex << " }" << std::endl;
        displayAttached();
        std::cout << "END SemNodeScope" << std::endl;
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

    void display() const override
    {
        std::cout << "START SemNodeFunction { start: " << mStartIndex << ", end: " << mEndIndex << " }" << std::endl;
        displayAttached();
        std::cout << "END SemNodeFunction" << std::endl;
    }
};

class SemNodeDefer final : public SemNode
{
public:
    SemNodeDefer(const uint32_t index) : mDeferEndIndex{index}
    {
    }

    void display() const override
    {
        std::cout << "SemNodeDefer { end: " << mDeferEndIndex << " }" << std::endl;
        displayAttached();
    }

private:
    uint32_t mDeferEndIndex;
};

} // namespace safec
