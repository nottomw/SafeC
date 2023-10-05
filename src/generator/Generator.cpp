#include "Generator.hpp"

#include "config/Config.hpp"
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
    deferExec.commit();

    if (Config::getInstance().getDisplayAstMod())
    {
        log("\nModified AST:\n");
        WalkerPrint printer;
        mWalker.walk(*ast, printer);
    }

    generateFinalSource(ast, outputFile);

    log("Generated file %", outputFile.c_str());
}

void Generator::generateFinalSource( //
    std::shared_ptr<SemNodeTranslationUnit> ast,
    const std::filesystem::path &outputFile)
{
    WalkerSourceGen sourceGen{outputFile};
    mWalker.walk(*ast, sourceGen);

    sourceGen.generate();
}

} // namespace safec
