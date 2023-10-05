#include "WalkerSourceGen.hpp"

#include "logger/Logger.hpp"

#include <cerrno>

using namespace safec;

void safec::requestAction(SpecialAction &lhs, const SpecialAction rhs)
{
    uint32_t lhsu32 = static_cast<uint32_t>(lhs);
    lhsu32 |= static_cast<uint32_t>(rhs);

    lhs = static_cast<SpecialAction>(lhsu32);
}

bool safec::isActionRequested(const SpecialAction lhs, const SpecialAction rhs)
{
    return ((static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)) != 0);
}

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

    // walk through all nodes, get all available ranges
    //  - if node deleted - ignore the range
    //  - if node added - check start/end pos, if 0 - toStr(), else just add

    const int32_t startPos = node.getSemStart();
    const int32_t endPos = node.getSemEnd();

    if ((startPos == 0) && (endPos == 0))
    {
        // skip node with no sem pos info
        return;
    }

    if (node.getDirty() == SemNode::DirtyType::Removed)
    {
        // skip removed nodes
        return;
    }

    SourceRange sourceRange;
    sourceRange.mStartPos = startPos;
    sourceRange.mEndPos = endPos;
    sourceRange.mNodeType = node.getTypeStr();

    if (node.getDirty() == SemNode::DirtyType::Added)
    {
        // prepend newline before defer since otherwise this
        // will be in the exact same line as previous node
        requestAction(sourceRange.mSpecialAction, SpecialAction::PrependNewline);

        // defer will never contain ';' since the deferred op is never terminated
        // (the incoming ';' is always for the "defer" keyword)
        requestAction(sourceRange.mSpecialAction, SpecialAction::AppendSemicolon);
    }

    mSourceRanges.push_back(sourceRange);
}

void WalkerSourceGen::peek(SemNodeTranslationUnit &node, const uint32_t)
{
    mSourceFile = node.getSourcePath();
    mSourceFileFp = fopen(mSourceFile.c_str(), "r");
    assert(mSourceFileFp != nullptr);
}

void WalkerSourceGen::generate()
{
    squashRanges();

    {
        // write the first range, since iteration starts from "1"
        uint32_t chunkStartPos = mSourceRanges[0].mStartPos;
        uint32_t chunkEndPos = mSourceRanges[0].mEndPos;

        auto sourceChunk = getStrFromSource(chunkStartPos, chunkEndPos);
        const uint32_t sourceChunkLen = chunkEndPos - chunkStartPos;

        writeChunkToOutputFile(sourceChunk, sourceChunkLen);
    }

    uint32_t prevEnd = mSourceRanges[0].mEndPos;
    for (uint32_t i = 1; i < mSourceRanges.size(); i++)
    {
        auto &it = mSourceRanges[i];

        uint32_t chunkStartPos = it.mStartPos;
        uint32_t chunkEndPos = it.mEndPos;

        if (chunkStartPos == prevEnd)
        {
            chunkStartPos += 1;
        }

        const auto sourceChunk = getStrFromSource(chunkStartPos, chunkEndPos);
        const uint32_t sourceChunkLen = chunkEndPos - chunkStartPos;

        if (isActionRequested(it.mSpecialAction, SpecialAction::PrependNewline))
        {
            std::string s = "\r\n";
            writeChunkToOutputFile(s, s.size());
        }

        writeChunkToOutputFile(sourceChunk, sourceChunkLen);

        if (isActionRequested(it.mSpecialAction, SpecialAction::AppendSemicolon))
        {
            std::string s = ";";
            writeChunkToOutputFile(s, s.size());
        }

        prevEnd = it.mEndPos;
    }
}

std::string WalkerSourceGen::getStrFromSource( //
    const uint32_t startPos,
    const uint32_t endPos)
{

    if (startPos == endPos)
    {
        return std::string{};
    }

    assert(startPos < endPos);

    const int fseekResSource = fseek(mSourceFileFp, startPos, 0);
    if (fseekResSource != 0)
    {
        log("failed to seek source file to %, error: %", //
            Color::Red,
            startPos,
            strerror(errno));
        return std::string{};
    }

    const int32_t nodeStringLen = endPos - startPos;

    std::string buffer;
    buffer.reserve(nodeStringLen + 1);

    const int freadRes = fread(buffer.data(), 1, nodeStringLen, mSourceFileFp);
    if (freadRes != nodeStringLen)
    {
        log("failed to read % bytes (read: %), error: %", //
            Color::Red,
            nodeStringLen,
            freadRes,
            strerror(errno));
        return std::string{};
    }

    return buffer;
}

void WalkerSourceGen::writeChunkToOutputFile( //
    const std::string &sourceChunk,
    const uint32_t sourceChunkSize)
{
    const uint32_t fwriteRes = //
        fwrite(sourceChunk.c_str(), 1, sourceChunkSize, mOutputFileFp);
    if (fwriteRes != sourceChunkSize)
    {
        log("failed to write % bytes (wrote: %), error: %", //
            Color::Red,
            sourceChunkSize,
            fwriteRes,
            strerror(errno));
        return;
    }
}

void WalkerSourceGen::squashRanges()
{
    // split ranges into smaller chunks, eg:
    //  [0] function { 1 -- 100 }
    //  [1] if { 2 - 50 }
    //  [2] loop { 51 - 99 }
    // should be split into:
    //  [0] {1  - 2}   - function start
    //  [1] {2  - 50}  - if
    //  [2] {51 - 99}  - loop
    //  [3] {99 - 100} - function end

    // probably sub-optimal algo...

    for (uint32_t i = 0; i < mSourceRanges.size(); i++)
    {
        auto &currentRange = mSourceRanges[i];

        const uint32_t currentRangeFirstIdx = i + 1;
        uint32_t currentRangeLastIdx = i + 1;
        uint32_t maxEndPosInCurrentRange = 0;
        for (uint32_t j = i + 1; j < mSourceRanges.size(); j++)
        {
            auto &followingRange = mSourceRanges[j];
            if ((followingRange.mStartPos >= currentRange.mStartPos) && //
                (followingRange.mEndPos <= currentRange.mEndPos))
            {
                currentRangeLastIdx = j;
                if (maxEndPosInCurrentRange < followingRange.mEndPos)
                {
                    maxEndPosInCurrentRange = followingRange.mEndPos;
                }
            }
            else
            {
                break;
            }
        }

        if (currentRangeFirstIdx == currentRangeLastIdx)
        {
            continue;
        }

        // first idx is the index of first node that is included in currentRange
        // last idx is the index of last node that is include in current range

        auto &currentRangeFirst = mSourceRanges[currentRangeFirstIdx];
        // auto &currentRangeLast = mSourceRanges[currentRangeLastIdx];

        SourceRange endingRange;
        endingRange.mStartPos = maxEndPosInCurrentRange; // currentRangeLast.mEndPos;
        endingRange.mEndPos = currentRange.mEndPos;
        endingRange.mNodeType = currentRange.mNodeType;

        // modify the current range to end at the start of first nested node
        currentRange.mEndPos = currentRangeFirst.mStartPos;

        // insert the ending range just after the last node
        mSourceRanges.insert( //
            mSourceRanges.begin() + currentRangeLastIdx + 1,
            endingRange);

        i = 0; // reset loop to handle nested ranges
    }

    for (auto &it : mSourceRanges)
    {
        log("squashed range: (% -- %) %", it.mStartPos, it.mEndPos, it.mNodeType);
    }
}
