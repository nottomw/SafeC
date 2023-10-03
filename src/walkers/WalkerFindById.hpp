#pragma once

#include "WalkerStrategy.hpp"
namespace safec
{

class WalkerFindById final : public WalkerStrategy
{
public:
    WalkerFindById(const uint32_t id);

    void peek(SemNode &node, const uint32_t astLevel) override;

    SemNode *getResult() const;

private:
    uint32_t mIdToBeFound;
    SemNode *mNodeFound;
};

} // namespace safec
