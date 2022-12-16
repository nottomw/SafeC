#pragma once

#include <cstdint>

namespace safec
{

class SemNode;
class SemNodeFunction;
class SemNodeScope;
class SemNodeDefer;

class WalkerStrategy
{
public:
    virtual void peek(SemNode &node, const uint32_t astLevel) = 0;
    virtual void peek(SemNodeFunction &node, const uint32_t astLevel) = 0;
    virtual void peek(SemNodeScope &node, const uint32_t astLevel) = 0;
    virtual void peek(SemNodeDefer &node, const uint32_t astLevel) = 0;
};

} // namespace safec
