#pragma once

#include <cstdint>
#include <string>

namespace safec
{

class WalkerStrategy;
class SemNode;

class SemNodeWalker
{
public:
    void walk(SemNode &node, WalkerStrategy &strategy);

private:
    void walkLeveled(SemNode &node, WalkerStrategy &strategy, const uint32_t level);
};

} // namespace safec
