#include "Parser.hpp"

#include "logger/Logger.hpp"
#include "semantics/Semantics.hpp"
#include "semantics/walkers/SemNodeWalker.hpp"
#include "semantics/walkers/WalkerDefer.hpp"
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

void Parser::parse(const std::string &path)
{
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

    mCurrentlyParsedFile = path;

    // TODO: handle preprocessor stuff
    // currently cannot use e.g.:
    //      #define TEST "asdf"
    //      printf(TEST);
    // not detected by grammar as a valid case

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

    log("\nParsing done, characters in file %: %\n") //
        .arg(path.c_str())
        .arg(lex_current_char);

    log("Current AST:");
    mSemantics.display();
    log("\n\n", {logger::NewLine::No});
}

void Parser::addModPoint(ModPoint &&modPoint)
{
    log("Adding mod point: ", {logger::NewLine::No});

    if (modPoint.getModType() == ModPoint::ModType::CallDeferred)
    {
        const auto functionName = std::any_cast<ModPoint::DataCallDeferred>(modPoint.getData()).functionName;
        log("\tCALL_DEFERRED\n"    //
            "\t ~ file index: %\n" //
            "\t ~ mod type: %\n"   //
            "\t ~ function name: %")
            .arg(modPoint.getIndex())
            .arg(static_cast<uint32_t>(modPoint.getModType()))
            .arg(functionName);
    }

    mModPoints.emplace_back(std::move(modPoint));
}

void Parser::dumpFileWithModifications(const boost::filesystem::path &path)
{
    assert(path.has_filename() == true);
    assert(path.is_absolute() == true);

    // TODO: fun: multiple walkers can run in parallel

    // Try to identify defer call points.
    WalkerDefer walkerDefer;
    SemNodeWalker walker;
    mSemantics.walk(walker, walkerDefer);

    const auto deferFires = walkerDefer.getDeferFires();
    const auto deferRemoves = walkerDefer.getDeferRemoves();
    assert(deferFires.size() >= deferRemoves.size());

    boost::iostreams::mapped_file_source mappedFile{mCurrentlyParsedFile};
    const char *const fileSource = mappedFile.data();

    bfs::ofstream fileOutputStream{path, std::ios_base::trunc};

    for (uint32_t i = 0U; i < mappedFile.size(); ++i)
    {
        fileOutputStream << fileSource[i];

        for (const auto &it : deferFires)
        {
            if (it.first == i)
            {
                // adding newline after defer fire
                fileOutputStream << it.second << '\n';
            }
        }

        for (const auto &it : deferRemoves)
        {
            if (it.first == i)
            {
                const uint32_t deferStatementLen = it.second + 1U;
                const std::string_view removedStr(&fileSource[i], deferStatementLen);
                fileOutputStream << "/* defer removed: " << deferStatementLen << " chars, '" << removedStr << "' */";

                // move past the to-be deleted defer
                i += it.second;
            }
        }
    }
}

} // namespace safec
