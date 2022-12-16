#pragma once

#include "Semantics.hpp"

namespace safec
{

class SemanticsFactory final
{
public:
    static Semantics &get()
    {
        static Semantics sem;
        return sem;
    }
};

} // namespace safec
