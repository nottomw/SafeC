#include "Generator.hpp"

#include "logger/Logger.hpp"
#include "parser/Parser.hpp"
#include "walkers/SemNodeWalker.hpp"
#include "walkers/WalkerDeferExecute.hpp"
#include "walkers/WalkerPrint.hpp"
#include "walkers/WalkerSourceGen.hpp"

namespace safec
{

void Generator::generate( //
    std::shared_ptr<SemNodeTranslationUnit> ast,
    const fs::path &outputFile)
{
    // run modifiying walkers here...
    WalkerDeferExecute deferExec;
    mWalker.walk(*ast, deferExec);

    // DBG: print modified AST
    WalkerPrint printer;
    mWalker.walk(*ast, printer);

    generateFinalSource(ast, outputFile);
}

void Generator::generateFinalSource( //
    std::shared_ptr<SemNodeTranslationUnit> ast,
    const std::filesystem::path &outputFile)
{
    WalkerSourceGen sourceGen{outputFile};
    mWalker.walk(*ast, sourceGen);
}

} // namespace safec
