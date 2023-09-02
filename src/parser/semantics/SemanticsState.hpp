#pragma once

#include "SyntaxChunkTypes.hpp"
#include "logger/Logger.hpp"

#include <memory>
#include <string>
#include <vector>

namespace safec
{

class SemNode;

enum class SState
{
    Idle,
    WaitingForStructType
};

struct SyntaxChunkInfo
{
    SyntaxChunkInfo()
        : mType{SyntaxChunkType::kUnknown}
        , mPos{0}
        , mAdditional{}
    {
    }

    SyntaxChunkInfo(const SyntaxChunkType type, const uint32_t pos)
        : mType{type}
        , mPos{pos}
        , mAdditional{}
    {
    }

    SyntaxChunkInfo(const SyntaxChunkType type, const uint32_t pos, const std::string &additional)
        : mType{type}
        , mPos{pos}
        , mAdditional{additional}
    {
    }

    SyntaxChunkType mType;
    uint32_t mPos;
    std::string mAdditional;
};

class SemanticsState
{
public:
    SState mState;
    using StagedNodesType = std::vector<std::shared_ptr<SemNode>>;

    SemanticsState() //
        : mState{SState::Idle}
        , mSyntaxChunks{}
    {
    }

    void addChunk(SyntaxChunkInfo &&chunk)
    {
        mSyntaxChunks.emplace_back(chunk);
    }

    std::vector<SyntaxChunkInfo> &getChunks()
    {
        return mSyntaxChunks;
    }

    StagedNodesType &getStagedNodes()
    {
        return mStagedNodes;
    }

    void stageNode(std::shared_ptr<SemNode> node)
    {
        mStagedNodes.push_back(node);
    }

    void printChunks() const
    {
        log("--- SYNTAX CHUNKS (chunks count: %, staged nodes count: %):", {Color::Green}) //
            .arg(mSyntaxChunks.size())
            .arg(mStagedNodes.size());
        for (const auto &it : mSyntaxChunks)
        {
            log("\tchunk type: %, pos: %, additional: '%'") //
                .arg(syntaxChunkTypeToStr(it.mType))
                .arg(it.mPos)
                .arg(it.mAdditional);
        }
        log("------", {Color::Green});
    }

private:
    std::vector<SyntaxChunkInfo> mSyntaxChunks;
    StagedNodesType mStagedNodes;
};

} // namespace safec
