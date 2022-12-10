#pragma once

#include "ModPoint.hpp"
#include <any>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace safec
{

struct TokenHandler;

using LexerToken = boost::spirit::lex::lexertl::token<>;

class Parser
{
public:
    Parser();
    ~Parser() = default;

    Parser(const Parser &) = delete;
    Parser(Parser &&) = delete;
    Parser &operator=(const Parser &) = delete;
    Parser &operator=(Parser &&) = delete;

    void parse(const std::string &path);

private:
    void parseString(const std::string_view &source);

    void addModPoint(ModPoint &&modPoint);

    void handleBraceOpen(const uint32_t stringIndex);
    void handleBraceClose(const uint32_t stringIndex);
    void handleReturn(const uint32_t stringIndex);
    void handleDeferCall(const LexerToken &token, const uint32_t stringIndex);

    std::vector<ModPoint> mModPoints;

    struct ParserState
    {
        ParserState() : mCurrentBraceLevel{0}, mDeferAtBraceLevel{}
        {
        }

        uint32_t mCurrentBraceLevel;

        // TODO: choose better data structure
        using DeferBraceIndexFunctionNamePair = std::pair<uint32_t, std::string>;
        std::vector<DeferBraceIndexFunctionNamePair> mDeferAtBraceLevel;
    };

    ParserState mState;

    friend TokenHandler;
};

} // namespace safec
