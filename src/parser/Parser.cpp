#include "Parser.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/range/iterator_range.hpp>
#include <cassert>
#include <iostream>

extern "C"
{
#include "SafecParser.yacc.hpp"
}

extern "C"
{
    extern FILE *yyin;
    extern int characters;
}

// TODO: fix & move to logger when available
#define TERM_COLOR_NC "\033[0m"
#define TERM_COLOR_YELLOW "\033[00;33m"
#define TERM_COLOR_RED "\033[00;31m"
#define TERM_COLOR_GREEN "\033[00;32m"
#define TERM_COLOR_LRED "\033[01;31m"
#define TERM_COLOR_LGREEN "\033[01;32m"
#define TERM_COLOR_LYELLOW "\033[01;33m"
#define TERM_COLOR_LBLUE "\033[01;34m"
#define TERM_COLOR_LPURPLE "\033[01;35m"
#define TERM_COLOR_LCYAN "\033[01;36m"

static void syntaxReport(const uint32_t stringIndex,
                         const std::string &name,
                         const std::string color = TERM_COLOR_LCYAN)
{
    std::cout << color << "@ " << name << " at: " << stringIndex << " @" << TERM_COLOR_NC;
}

namespace safec
{

namespace bfs = ::boost::filesystem;
namespace bio = ::boost::iostreams;

Parser::Parser() //
    : mState{}
{
}

void Parser::parse(const std::string &path)
{
    const bfs::path filePathBoost{path};
    if (bfs::exists(filePathBoost) == false)
    {
        throw std::runtime_error("file not found");
    }

    if (bfs::is_directory(filePathBoost) == true)
    {
        // TODO: fun: multiple processes for multiple files (?)
        std::cout << "parser: got directory\n";
        for (auto &entry : boost::make_iterator_range(bfs::directory_iterator(filePathBoost), {}))
        {
            parseFile(entry);
        }
    }
    else if (bfs::is_regular_file(filePathBoost) == true)
    {
        parseFile(filePathBoost);
    }
    else
    {
        throw std::runtime_error{"invalid path?"};
    }
}

void Parser::parseFile(const bfs::path &path)
{
    assert(bfs::is_regular_file(path) == true);

    yyin = fopen(path.c_str(), "r");
    assert(yyin != nullptr);

    const int32_t parseRes = yyparse();
    assert(parseRes == 0);

    std::cout << "Parsing done, characters in file " << path << ": " << characters << std::endl;

    // After the parsing is done we should know the exact layout of intersting code blocks
    // in the provided source file.
    // TODO: use boost graph?
}

void Parser::addModPoint(ModPoint &&modPoint)
{
    std::cout << "Adding mod point:\n";
    if (modPoint.getModType() == ModPoint::ModType::CALL_DEFERRED)
    {
        const auto functionName = std::any_cast<ModPoint::DataCallDeferred>(modPoint.getData()).functionName;
        std::cout << "\tCALL_DEFERRED\n";
        std::cout << "\t ~ file index: " << modPoint.getIndex() << "\n";
        std::cout << "\t ~ mod type: " << static_cast<uint32_t>(modPoint.getModType()) << "\n";
        std::cout << "\t ~ function name: " << functionName << "\n";
    }

    mModPoints.emplace_back(std::move(modPoint));
}

void Parser::handleIdentifier(const uint32_t stringIndex, std::string &&name)
{
    name.insert(0, "identifier: ");
    syntaxReport(stringIndex, name);
}

void Parser::handlePostfixExpression(const uint32_t stringIndex, const bool containsArguments)
{
    if (containsArguments == true)
    {
        syntaxReport(stringIndex, "call with args");
    }
    else
    {
        syntaxReport(stringIndex, "call no args");
    }
}

void Parser::handleDeferCall(const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "defer", TERM_COLOR_LBLUE);

    // At this point the handler knows the exact location of deferred call end,
    // so to simplify it should just backtrace to the "defer" token and copy the whole
    // call into mod point.

    mState.mDeferAtBraceLevel.emplace_back(mState.mCurrentBraceLevel, "to_be_implemented()");
}

void Parser::handleReturn(const uint32_t stringIndex, const bool returnValueAvailable)
{
    if (returnValueAvailable == true)
    {
        syntaxReport(stringIndex, "return with value", TERM_COLOR_LRED);
    }
    else
    {
        syntaxReport(stringIndex, "return", TERM_COLOR_LRED);
    }
}

void Parser::handleCompoundStatementStart(const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "compound stmt start");

    mState.mCurrentBraceLevel += 1U;
}

void Parser::handleCompoundStatementEnd(const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "compound stmt end");

    // handle defers in current brace level
    auto it = mState.mDeferAtBraceLevel.begin();
    while (it != mState.mDeferAtBraceLevel.end())
    {
        // TODO: handle return/continue/break/loops
        //  - return: always fire before return
        //  - break: fire only if the defer was called in current loop
        //  - continue: same as break
        //  - scope end: fire only if the defer was at current scope

        if (it->first == mState.mCurrentBraceLevel)
        {
            std::cout << TERM_COLOR_LYELLOW << "@@@ firing a defer call: '" << it->second << "' @@@" << TERM_COLOR_NC
                      << std::endl;
        }

        if (it->first == mState.mCurrentBraceLevel)
        {
            it = mState.mDeferAtBraceLevel.erase(it);
        }
        else
        {
            ++it;
        }
    }

    mState.mCurrentBraceLevel -= 1U;
}

} // namespace safec
