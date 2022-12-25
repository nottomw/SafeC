#pragma once

#include <any>
#include <cstdint>
#include <string>
#include <vector>

namespace safec
{

class ModPoint;

using ModPointsVector = std::vector<ModPoint>;

enum class ModType
{
    TextInsert, // start + text
    TextRemove, // start + size
    TextReplace // start + text
};

class ModPoint
{
public:
    // TODO: add proper constructors for ModType

    ModPoint(const ModType mod, const uint32_t rangeStart, std::string textToInsert)
        : mMod{mod}
        , mStart{rangeStart}
        , mSize{0U}
        , mTextToInsert{textToInsert}
    {
        if ((mMod == ModType::TextInsert) || //
            (mMod == ModType::TextReplace))
        {
            assert(mTextToInsert.empty() == false);
        }
    }

    ModPoint(const ModType mod, const uint32_t rangeStart, const uint32_t size)
        : mMod{mod}
        , mStart{rangeStart}
        , mSize{size}
        , mTextToInsert{}
    {
        assert(mMod == ModType::TextRemove);
    }

    ModPoint(const ModPoint &other) = default;
    ModPoint(ModPoint &&other) = default;

    ModPoint &operator=(const ModPoint &other) = default;
    ModPoint &operator=(ModPoint &&other) = default;

    ModType getModType() const
    {
        return mMod;
    }

    uint32_t getStart() const
    {
        return mStart;
    }

    uint32_t getSize() const
    {
        return mSize;
    }

    std::string getText() const
    {
        return mTextToInsert;
    }

private:
    ModType mMod;
    uint32_t mStart;
    uint32_t mSize;
    std::string mTextToInsert;
};

} // namespace safec
