#include "Parser.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <cassert>
#include <iostream>

namespace safec
{

namespace bfs = ::boost::filesystem;
namespace bio = ::boost::iostreams;
namespace lex = ::boost::spirit::lex;

enum Tokens : uint32_t
{
    ID_BRACKET_OPEN = 1, // has to start at one
    ID_BRACKET_CLOSE,
    ID_RETURN,
    ID_DEFER_CALL,
    ID_CHAR
};

template <typename TLexer>
struct TokenDefinitions : lex::lexer<TLexer>
{
    TokenDefinitions()
    {
        this->self.add                                        //
            ("\\{", Tokens::ID_BRACKET_OPEN)                  //
            ("\\}", Tokens::ID_BRACKET_CLOSE)                 //
            ("return", Tokens::ID_RETURN)                     //
            ("defer [a-zA-Z0-9_]+\\(", Tokens::ID_DEFER_CALL) //
            (".", Tokens::ID_CHAR)                            //
            ;
    }
};

struct TokenHandler
{
    template <typename TToken>
    bool operator()(const TToken &token, uint32_t &stringIndex)
    {
        switch (token.id())
        {
        case Tokens::ID_BRACKET_OPEN:
            std::cout << " ~ bracket open at #" << stringIndex << std::endl;
            stringIndex += 1U;
            break;
        case Tokens::ID_BRACKET_CLOSE:
            std::cout << " ~ bracket close at #" << stringIndex << std::endl;
            stringIndex += 1U;
            break;
        case Tokens::ID_RETURN:
            std::cout << " ~ return at #" << stringIndex << std::endl;
            stringIndex += token.value().size();
            break;
        case Tokens::ID_DEFER_CALL:
            std::cout << " ~ defer call at #" << stringIndex << std::endl;
            stringIndex += token.value().size();
            break;
        case Tokens::ID_CHAR:
            stringIndex += 1U;
            break;
        default:
            throw std::runtime_error{"unknown token id - lexer error"};
            break;
        }

        return true;
    }
};

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
        // TODO: for each .sc file in directory...
    }

    if (bfs::is_regular_file(filePathBoost) == true)
    {
        bio::mapped_file_source source{filePathBoost};
        const std::string_view sourceView{source.data(), source.size()};

        parseString(sourceView);
    }
}

void Parser::parseString(const std::string_view &source)
{
    TokenDefinitions<lex::lexertl::lexer<>> findTokens;

    uint32_t stringIndex = 0U;

    auto tokenHandler = std::bind(TokenHandler(), std::placeholders::_1, std::ref(stringIndex));

    auto first = source.begin();
    auto last = source.end();

    const bool lexSuccess = lex::tokenize( //
        first, last, findTokens, tokenHandler);
    assert(lexSuccess == true);
}

} // namespace safec
