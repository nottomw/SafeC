#pragma once

#include "semantic_nodes/SemNode.hpp"

#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

namespace safec
{

class Parser;

class Generator
{
public:
    void generate( //
        std::shared_ptr<SemNodeTranslationUnit> ast,
        const fs::path &outputFile);

private:
    void generateFinalSource( //
        std::shared_ptr<SemNodeTranslationUnit> ast,
        const fs::path &outputFile);
};

} // namespace safec
