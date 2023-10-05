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
    // position.

    // TODO: Has to be changed if any modifications to AST made...
    // TODO: proper output file generation:
    //  - how to handle removed nodes (maybe just mark node as
    //    removed/ignored during generation instead of removing from AST?)
    //  - how to handle added nodes - all following nodes must
    //    be offset by the strlen of the added node
    //  - maybe could:
    //      1) copy the original source code
    //      2) if node was removed, remove part of code
    //         (offset all following nodes by - strlen(removedNode))
    //      3) if node was added, insert code just before following node
    //         (offset all following nodes by + strlen(addedNode))

    // 1) walk through all nodes, get all available ranges
    //      - if node deleted - ignore the range
    // 2) during the walk split ranges into smaller chunks, eg:
    //      function { 1 -- 100 }
    //          if { 2 - 50 }
    //          loop { 51 - 99 }
    //    should be split into ranges:
    //      {1  - 2}   - function start
    //      {2  - 50}  - if
    //      {51 - 99}  - loop
    //      {99 - 100} - function end
    //

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
