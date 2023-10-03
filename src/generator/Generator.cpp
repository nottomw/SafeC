#include "Generator.hpp"

#include "logger/Logger.hpp"
#include "parser/Parser.hpp"

namespace safec
{

void Generator::generate( //
    std::shared_ptr<SemNodeTranslationUnit> ast,
    const bfs::path &outputFile)
{
    log("generating...");
}

} // namespace safec
