#include "Parser.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/range/iterator_range.hpp>
#include <cassert>
#include <iostream>

extern "C"
{
#include "SafecParser.yacc.h"
}

extern "C"
{
    extern FILE *yyin;
    extern int characters;
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

    std::cout << "yyparse() result: " << parseRes << std::endl;
    assert(parseRes == 0);

    std::cout << "Characters in file " << path << ":" << characters << std::endl;

    // TODO: parser action -- defer
    // TODO: lex/parse: move extern functions to separate file
    // TODO: lex/parse: add exact position in file
    // TODO: lex/parse: add boolean and sintX_t
    // TODO: lex/parse: allow int i in for loop
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

    mModPoints.emplace_back(modPoint);
}

void Parser::handleBraceOpen(const uint32_t)
{
    mState.mCurrentBraceLevel += 1U;
}

void Parser::handleBraceClose(const uint32_t stringIndex)
{
    for (auto it = mState.mDeferAtBraceLevel.begin(); //
         it != mState.mDeferAtBraceLevel.end();
         ++it)
    {
        auto &deferAtBrace = it->first;
        if (deferAtBrace == mState.mCurrentBraceLevel)
        {
            auto &functionName = it->second;
            std::cout << " <-- firing defer at end of brace level " << mState.mCurrentBraceLevel
                      << ", call: " << functionName << std::endl;
            addModPoint(
                ModPoint{stringIndex, ModPoint::ModType::CALL_DEFERRED, ModPoint::DataCallDeferred{functionName}});
            mState.mDeferAtBraceLevel.erase(it);
            break;
        }
    }

    mState.mCurrentBraceLevel -= 1U;
}

void Parser::handleReturn(const uint32_t stringIndex)
{
    for (auto it = mState.mDeferAtBraceLevel.begin(); //
         it != mState.mDeferAtBraceLevel.end();
         ++it)
    {
        auto &deferAtBrace = it->first;
        if (deferAtBrace <= mState.mCurrentBraceLevel)
        {
            auto &functionName = it->second;
            std::cout << " <-- firing defer at return, brace level " << mState.mCurrentBraceLevel
                      << ", call: " << functionName << std::endl;
            addModPoint(
                ModPoint{stringIndex, ModPoint::ModType::CALL_DEFERRED, ModPoint::DataCallDeferred{functionName}});

            // TODO: detect if this is last return in function - if yes, erase
            // mState.mDeferAtBraceLevel.erase(it);
            break;
        }
    }
}

void Parser::handleDeferCall(const std::string &token, const uint32_t)
{
    constexpr size_t deferSize = sizeof("defer ") - 1;
    const size_t tokenSize = token.length();

    const std::string deferredFunctionName = token.substr(deferSize, (tokenSize - deferSize));

    std::cout << " --> defer detected, brace level: " //
              << mState.mCurrentBraceLevel            //
              << ", token: "                          //
              << deferredFunctionName                 //
              << std::endl;

    mState.mDeferAtBraceLevel.emplace_back(std::make_pair(mState.mCurrentBraceLevel, deferredFunctionName));
}

} // namespace safec
