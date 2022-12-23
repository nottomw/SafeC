#include "Generator.hpp"

#include "parser/Parser.hpp"

namespace safec
{

Generator::Generator(safec::Parser &parser)
    : mParser{parser}
{
}

void Generator::generate(const bfs::path &outputFile)
{
    // For now just forward the request to parser.
    mParser.dumpFileWithModifications(outputFile);
}

} // namespace safec
