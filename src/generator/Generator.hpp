#pragma once

#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace safec {

class Parser;

class Generator
{
public:
    Generator(Parser &parser);

    void generate(const bfs::path &outputFile);

private:
    Parser &mParser;
};

}
