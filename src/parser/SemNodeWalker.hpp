#pragma once

#include <cstdint>

namespace safec
{

class SemNode;

class SemNodePrinter
{
public:
    SemNodePrinter();

    void walk(SemNode &node);

private:
    uint32_t mScopeLevel;
};

} // namespace safec
