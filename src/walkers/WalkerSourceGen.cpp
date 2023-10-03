#include "WalkerSourceGen.hpp"

#include "logger/Logger.hpp"

#include <cerrno>

using namespace safec;

WalkerSourceGen::WalkerSourceGen( //
    const std::filesystem::path &outputFile)
    : mOutputFile{outputFile}
    , mOutputFileFp{nullptr}
    , mSourceFileFp{nullptr}
{
    mOutputFileFp = fopen(outputFile.c_str(), "w");
    if (mOutputFileFp == nullptr)
    {
        log("failed to open file %, error: %", //
            Color::Red,
            outputFile.c_str(),
            strerror(errno));
    }
}

WalkerSourceGen::~WalkerSourceGen()
{
    fclose(mOutputFileFp);
    fclose(mSourceFileFp);
}

void WalkerSourceGen::peek(SemNode &node, const uint32_t)
{
    assert(mOutputFileFp != nullptr);
    assert(mSourceFileFp != nullptr);

    // Extremely dumb walker for now that just overwrites the
    // source code with incoming nodes underlying source start/end
    // position. Has to be changed if any modifications to AST made...

    const int32_t startPos = node.getSemStart();
    const int32_t endPos = node.getSemEnd();
    const int32_t nodeStringLen = endPos - startPos;

    if ((startPos == 0) && (endPos == 0))
    {
        // skip node with no sem pos info
        return;
    }

    const int fseekRes = fseek(mOutputFileFp, startPos, 0);
    if (fseekRes != 0)
    {
        log("failed to seek output file to %, error: %", //
            Color::Red,
            startPos,
            strerror(errno));
        return;
    }

    const int fseekResSource = fseek(mSourceFileFp, startPos, 0);
    if (fseekResSource != 0)
    {
        log("failed to seek source file to %, error: %", //
            Color::Red,
            startPos,
            strerror(errno));
        return;
    }

    auto buffer = std::make_unique<uint8_t[]>(nodeStringLen + 1);

    const int freadRes = fread(buffer.get(), 1, nodeStringLen, mSourceFileFp);
    if (freadRes != nodeStringLen)
    {
        log("failed to read % bytes (read: %), error: %", //
            Color::Red,
            nodeStringLen,
            freadRes,
            strerror(errno));
        return;
    }

    const int fwriteRes = fwrite(buffer.get(), 1, nodeStringLen, mOutputFileFp);
    if (fwriteRes != nodeStringLen)
    {
        log("failed to write % bytes (wrote: %), error: %", //
            Color::Red,
            nodeStringLen,
            fwriteRes,
            strerror(errno));
        return;
    }
}

void WalkerSourceGen::peek(SemNodeTranslationUnit &node, const uint32_t)
{
    mSourceFile = node.getSourcePath();
    mSourceFileFp = fopen(mSourceFile.c_str(), "r");
    assert(mSourceFileFp != nullptr);
}
