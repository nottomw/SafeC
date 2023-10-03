#pragma once

#include "WalkerStrategy.hpp"
#include <cstdio>

namespace fs = std::filesystem;

namespace safec
{

// Walker that generates the source code gathered from an AST back
// into a specified source file.
class WalkerSourceGen final : public WalkerStrategy
{
public:
    WalkerSourceGen(const fs::path &outputFile);
    ~WalkerSourceGen();

    void peek(SemNode &node, const uint32_t astLevel) override;
    void peek(SemNodeTranslationUnit &node, const uint32_t astLevel) override;

private:
    [[maybe_unused]] const fs::path &mOutputFile;
    FILE* mOutputFileFp;

    fs::path mSourceFile;
    FILE *mSourceFileFp;
};

} // namespace safec
