#pragma once

#include "WalkerStrategy.hpp"

#include <cstdio>

namespace fs = std::filesystem;

namespace safec
{

enum class SpecialAction : uint32_t
{
    Nothing = 0,

    PrependNewline = (1 << 0),
    AppendSemicolon = (1 << 1),
};

void requestAction(SpecialAction &lhs, const SpecialAction rhs);
bool isActionRequested(const SpecialAction lhs, const SpecialAction rhs);

// Walker that generates the source code gathered from an AST back
// into a specified source file.
class WalkerSourceGen final : public WalkerStrategy
{
public:
    WalkerSourceGen(const fs::path &outputFile);
    ~WalkerSourceGen();

    void peek(SemNode &node, const uint32_t astLevel) override;
    void peek(SemNodeTranslationUnit &node, const uint32_t astLevel) override;

    void generate();

private:
    struct SourceRange
    {
        SourceRange()
            : mSpecialAction{SpecialAction::Nothing}
            , mAdded{false}
        {
        }

        uint32_t mStartPos;
        uint32_t mEndPos;
        std::string mNodeType;
        SpecialAction mSpecialAction;
        bool mAdded;
    };

    [[maybe_unused]] const fs::path &mOutputFile;
    FILE *mOutputFileFp;

    fs::path mSourceFile;
    FILE *mSourceFileFp;

    std::vector<SourceRange> mSourceRanges;
    std::vector<SourceRange> mRemovedRanges;

    std::string getStrFromSource( //
        const uint32_t startPos,
        const uint32_t endPos);

    void writeChunkToOutputFile( //
        const std::string &sourceChunk,
        const uint32_t sourceChunkSize);

    void squashRanges();
    void applyNodeRemoves();
};

} // namespace safec
