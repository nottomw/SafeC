#include "WalkerSourceAstIntegrity.hpp"

safec::WalkerSourceAstIntegrity::WalkerSourceAstIntegrity()
    : WalkerStrategy{}
    , mIntegrityOk{false}
{
}

void safec::WalkerSourceAstIntegrity::peek( //
    safec::SemNodePositional &node,
    const uint32_t astLevel)
{
}

void safec::WalkerSourceAstIntegrity::peek( //
    safec::SemNodeScope &node,
    const uint32_t astLevel)
{
}

bool safec::WalkerSourceAstIntegrity::getIntegrityOk() const
{
    return mIntegrityOk;
}

void safec::WalkerSourceAstIntegrity::insertSorted( //
    safec::WalkerSourceAstIntegrity::ScopeInfo &info)
{
    if (mScopesInfo.size() == 0)
    {
        mScopesInfo.emplace_back(info);
        return;
    }

    for (uint32_t i = 0; i < mScopesInfo.size(); i++)
    {
        auto &it = mScopesInfo[i];
        if (it.mEnd <= info.mStart)
        {
            mScopesInfo.insert(mScopesInfo.begin() + i + 1, info);
            break;
        }
    }
}
