#include "Generator.hpp"

#include "logger/Logger.hpp"
#include "parser/Parser.hpp"
#include "walkers/SemNodeWalker.hpp"
#include "walkers/WalkerSourceGen.hpp"

namespace safec
{

void Generator::generate( //
    std::shared_ptr<SemNodeTranslationUnit> ast,
    const fs::path &outputFile)
{
    // run modifiying walkers here...

    generateFinalSource(ast, outputFile);
}

void Generator::generateFinalSource( //
    std::shared_ptr<SemNodeTranslationUnit> ast,
    const std::filesystem::path &outputFile)
{
    SemNodeWalker walker;
    WalkerSourceGen sourceGen{outputFile};

    walker.walk(*ast, sourceGen);
}

} // namespace safec
