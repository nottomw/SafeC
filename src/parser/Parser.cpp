#include "Parser.hpp"

#include "logger/Logger.hpp"
#include "semantics/Semantics.hpp"
#include "utils/Utils.hpp"
#include "walkers/SemNodeWalker.hpp"
#include "walkers/WalkerPrint.hpp"
#include "walkers/WalkerSourceCoverage.hpp"

#include <cassert>
#include <iostream>
#include <string_view>

extern "C"
{
#include "SafecParser.yacc.hpp"
}

extern "C"
{
    extern FILE *yyin;
    extern int lex_current_char;
}

namespace fs = std::filesystem;

namespace safec
{

Parser::Parser(Semantics &sem) //
    : mSemantics{sem}
    , mCurrentlyParsedFile{}
{
}

size_t Parser::parse(const std::string &path)
{
    size_t charCount = 0;

    const fs::path filePath{path};
    if (fs::exists(filePath) == false)
    {
        throw std::runtime_error("file not found");
    }

    if (fs::is_directory(filePath) == true)
    {
        throw std::runtime_error("got directory to transpile - file required");
    }
    else if (fs::is_regular_file(filePath) == true)
    {
        charCount = parseFile(filePath);
    }
    else
    {
        throw std::runtime_error{"invalid path?"};
    }

    return charCount;
}

void Parser::displayAst() const
{
    log("AST:");

    SemNodeWalker walker;
    WalkerPrint printer;

    mSemantics.walk(walker, printer);

    log("\n\n", NewLine::No);
}

void Parser::displayCoverage() const
{
    SemNodeWalker walker;
    WalkerSourceCoverage covChecker;

    mSemantics.walk(walker, covChecker);

    covChecker.printReport();
}

std::shared_ptr<SemNodeTranslationUnit> Parser::getAst() const
{
    return mSemantics.getAst();
}

size_t Parser::parseFile(const fs::path &path)
{
    assert(fs::is_regular_file(path) == true);

    mCurrentlyParsedFile = path;

    // TODO: handle preprocessor stuff
    //      currently cannot use e.g.:
    //          #define TEST "asdf"
    //          printf(TEST);
    //      not detected by grammar as a valid case
    // TODO: handle defer use in preprocessor
    //      should be able to do things like:
    //          #define LOCK_GUARD(lock) acquire(lock); defer release(lock);

    // TODO: handle string literal concat
    // printf("one" "two"); will raise an error in current grammar

    {
        yyin = fopen(path.c_str(), "r");
        assert(yyin != nullptr);

        utils::DeferredCall defer{[] { fclose(yyin); }};

        mSemantics.newTranslationUnit(path);

        const int32_t parseRes = yyparse();
        assert(parseRes == 0);
    }

    const size_t parsedCharsCount = lex_current_char;

    return parsedCharsCount;
}

} // namespace safec
