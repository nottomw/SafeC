#pragma once

#include <any>
#include <cstdint>
#include <string>
#include <vector>

namespace safec
{

class ModPoint;

using ModPointsVector = std::vector<ModPoint>;

class ModPoint
{
public:
    enum class ModType
    {
        TextInsert,
        TextRemove,
        TextReplace
    };

    ModPoint( //
        const ModType mod,
        const uint32_t rangeStart,
        const uint32_t rangeEnd,
        std::string optionalTextToInsert)
        : mMod{mod}
        , mRangeStart{rangeStart}
        , mRangeEnd{rangeEnd}
        , mOptionalTextToInsert{optionalTextToInsert}
    {
        if ((mMod == ModType::TextInsert) || //
            (mMod == ModType::TextReplace))
        {
            assert(mOptionalTextToInsert.empty() == false);
        }
    }

    ModType getModType() const
    {
        return mMod;
    }

    uint32_t getRangeStart() const
    {
        return mRangeStart;
    }

    uint32_t getRangeEnd() const
    {
        return mRangeEnd;
    }

    std::string getOptionalTextToInsert() const
    {
        return mOptionalTextToInsert;
    }

private:
    const ModType mMod;
    const uint32_t mRangeStart;
    const uint32_t mRangeEnd;
    const std::string mOptionalTextToInsert;
};

} // namespace safec
