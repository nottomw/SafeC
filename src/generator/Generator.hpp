#pragma once

#include "parser/semantics/SemNode.hpp"

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
};

} // namespace safec
