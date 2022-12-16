#include "Parser.hpp"

#include "Semantics.hpp"

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

namespace safec
{

namespace bfs = ::boost::filesystem;
namespace bio = ::boost::iostreams;

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

    mSemantics.newTranslationUnit();

    const int32_t parseRes = yyparse();
    assert(parseRes == 0);

    std::cout << "Parsing done, characters in file " << path << ": " << characters << std::endl;

    // After the parsing is done we should know the exact layout of intersting code blocks
    // in the provided source file.

    mSemantics.display();
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

} // namespace safec
