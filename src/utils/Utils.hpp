#pragma once

#include <cassert>
#include <functional>

namespace safec
{

namespace utils
{

class DeferredCall
{
public:
    DeferredCall(std::function<void(void)> &&f)
        : mDeferredFunction{std::move(f)}
    {
    }

    ~DeferredCall()
    {
        assert(mDeferredFunction);
        mDeferredFunction();
    }

private:
    std::function<void(void)> mDeferredFunction;
};

} // namespace utils

} // namespace safec
