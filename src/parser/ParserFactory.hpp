#pragma once

namespace safec
{

class Parser;

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
