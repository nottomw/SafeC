#pragma once

#include "ModPoint.hpp"
#include <any>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace boost
{
namespace filesystem
{
class path;
}
} // namespace boost

namespace safec
{

class Parser final
{
public:
    Parser();
    ~Parser() = default;
    Parser(const Parser &) = delete;
    Parser(Parser &&) = delete;
    Parser &operator=(const Parser &) = delete;
    Parser &operator=(Parser &&) = delete;

    void parse(const std::string &path);

    // TOOD: move to separate Semantics class
    void handleIdentifier(const uint32_t stringIndex, const std::string &&name);
    void handlePostfixExpression(const uint32_t stringIndex, const bool containsArguments);
    void handleDeferCall(const uint32_t stringIndex);
    void handleReturn(const uint32_t stringIndex, const bool returnValueAvailable);
    void handleCompoundStatementStart(const uint32_t stringIndex);
    void handleCompoundStatementEnd(const uint32_t stringIndex);

private:
    void parseFile(const boost::filesystem::path &path);

    void addModPoint(ModPoint &&modPoint);

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
};

} // namespace safec
