#pragma once

#include "Parser.hpp"

namespace safec
{

class ParserFactory final
{
public:
    static Parser &getParser()
    {
        static Parser parser;
        return parser;
    }
};

} // namespace safec
