#include "Parser.hpp"

#include "logger/Logger.hpp"
#include "semantics/Semantics.hpp"
#include "semantics/walkers/SemNodeWalker.hpp"
#include "utils/Utils.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
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
    extern int lex_current_char;
}

namespace bfs = boost::filesystem;

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

    const bfs::path filePathBoost{path};
    if (bfs::exists(filePathBoost) == false)
    {
        throw std::runtime_error("file not found");
    }

    if (bfs::is_directory(filePathBoost) == true)
    {
        throw std::runtime_error("got directory to transpile - file required");
    }
    else if (bfs::is_regular_file(filePathBoost) == true)
    {
        charCount = parseFile(filePathBoost);
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
    mSemantics.display();
    log("\n\n", NewLine::No);
}

void Parser::displayCoverage() const
{
    mSemantics.displayCoverage();
}

size_t Parser::parseFile(const bfs::path &path)
{
    assert(bfs::is_regular_file(path) == true);

    mCurrentlyParsedFile = path;

    // TODO: handle preprocessor stuff
    // currently cannot use e.g.:
    //      #define TEST "asdf"
    //      printf(TEST);
    // not detected by grammar as a valid case

    // TODO: handle string literal concat
    // printf("one" "two"); will raise an error in current grammar

    // TODO: handle defer use in preprocessor
    // should be able to do things like:
    //      #define LOCK_GUARD(lock) acquire(lock); defer release(lock);

    {
        yyin = fopen(path.c_str(), "r");
        assert(yyin != nullptr);

        utils::DeferredCall defer{[] { fclose(yyin); }};

        // TODO: add class TranslationUnitFile, will be helpful when implementing preprocessor support
        mSemantics.newTranslationUnit(path);

        const int32_t parseRes = yyparse();
        assert(parseRes == 0);
    }

    const size_t parsedCharsCount = lex_current_char;

    return parsedCharsCount;
}

void Parser::dumpFileWithModifications(const boost::filesystem::path &path)
{
    (void)path;
    //    assert(path.has_filename() == true);
    //    assert(path.is_absolute() == true);

    //    // TODO: fun: multiple walkers can run in parallel

    //    // Try to identify defer call points.
    //    WalkerDefer walkerDefer;
    //    SemNodeWalker walker;
    //    mSemantics.walk(walker, walkerDefer);

    //    const auto modPoints = walkerDefer.getModPoints();

    //    boost::iostreams::mapped_file_source mappedFile{mCurrentlyParsedFile};
    //    const char *const fileSource = mappedFile.data();
    //    bfs::ofstream fileOutputStream{path, std::ios_base::trunc};

    //    for (uint32_t i = 0U; i < mappedFile.size(); ++i)
    //    {
    //        fileOutputStream << fileSource[i];

    //        for (const auto &it : modPoints)
    //        {
    //            if (it.getStart() == i)
    //            {
    //                if (it.getModType() == ModType::TextInsert)
    //                {
    //                    // adding newline after defer fire
    //                    fileOutputStream << it.getText() << '\n';
    //                }
    //                else if (it.getModType() == ModType::TextRemove)
    //                {
    //                    const uint32_t deferStatementLen = it.getSize() + 1U;
    //                    const std::string_view removedStr(&fileSource[i], deferStatementLen);
    //                    fileOutputStream << "/* defer removed: " //
    //                                     << deferStatementLen    //
    //                                     << " chars, '"          //
    //                                     << removedStr           //
    //                                     << "' */";

    //                    // move past the to-be deleted defer
    //                    i += it.getSize();
    //                }
    //                else
    //                {
    //                    TODO();
    //                }
    //            }
    //        }
    //    }
}

} // namespace safec
