#include "WalkerFindById.hpp"

using namespace safec;

WalkerFindById::WalkerFindById(const uint32_t id)
    : mIdToBeFound{id}
    , mNodeFound{nullptr}
{
}

void WalkerFindById::peek(SemNode &node, const uint32_t astLevel)
{
    if (mNodeFound != nullptr)
    {
        return;
    }

    if (node.getId() == mIdToBeFound)
    {
        mNodeFound = &node;
    }
}

SemNode *WalkerFindById::getResult() const
{
    return mNodeFound;
}
