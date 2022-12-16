#pragma once

#include <any>
#include <cstdint>
#include <string>

namespace safec
{

class Parser;

class ModPoint
{
public:
    enum class ModType
    {
        CallDeferred
    };

    ModPoint( //
        const uint32_t idx,
        const ModType mod,
        std::any &&data)
        : mIndex(idx) //
        , mMod(mod)
        , mData(std::move(data))
    {
    }

    struct DataCallDeferred
    {
        const std::string &functionName;
    };

    uint32_t getIndex() const
    {
        return mIndex;
    }

    ModType getModType() const
    {
        return mMod;
    }

    std::any getData() const
    {
        return mData;
    }

private:
    const uint32_t mIndex;
    const ModType mMod;
    const std::any mData;
};

} // namespace safec