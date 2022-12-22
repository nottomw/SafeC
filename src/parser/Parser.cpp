#include "Parser.hpp"

#include "logger/Logger.hpp"
#include "semantics/Semantics.hpp"
#include "semantics/walkers/SemNodeWalker.hpp"
#include "semantics/walkers/WalkerDefer.hpp"

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

namespace safec
{

namespace bfs = ::boost::filesystem;

Parser::Parser(Semantics &sem) //
    : mSemantics{sem}
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
        log("parser: got directory");
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

    mSemantics.newTranslationUnit(path);

    const int32_t parseRes = yyparse();
    assert(parseRes == 0);

    fclose(yyin);

    log("\nParsing done, characters in file %: %\n") //
        .arg(path.c_str())
        .arg(lex_current_char);

    log("Current AST:");
    mSemantics.display();
    log("\n\n", {logger::NewLine::No});

    // TODO: fun: multiple walkers can run in parallel

    // Try to identify defer call points.
    WalkerDefer walkerDefer;
    SemNodeWalker walker;
    mSemantics.walk(walker, walkerDefer);

    log("Defer fire analysis: ", Color::Yellow);
    const auto deferFires = walkerDefer.getDeferFires();
    for (const auto &it : deferFires)
    {
        log("\t'defer' fire at: %, \ttext: '%'") //
            .arg(it.first)
            .arg(it.second);
    }

    const auto deferRemoves = walkerDefer.getDeferRemoves();
    for (const auto &it : deferRemoves)
    {
        log("\t'defer' remove at: %") //
            .arg(it);
    }

    log("\n\n--- file dump ---");

    boost::iostreams::mapped_file_source mappedFile{path};
    const char *const fileSource = mappedFile.data();

    for (uint32_t i = 0U; i < mappedFile.size(); ++i)
    {
        log("%", {logger::NewLine::No}) //
            .arg(fileSource[i]);

        for (const auto &it : deferFires)
        {
            if (it.first == i)
            {
                log("%").arg(it.second);
            }
        }

        for (const auto &it : deferRemoves)
        {
            if (it == i)
            {
                // searching for end of "defer ...;" statement
                while (fileSource[i] != ';')
                {
                    i++;
                }
            }
        }
    }
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

} // namespace safec
