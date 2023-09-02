#pragma once

#include <vector>
#include <string>
#include <memory>

#include "SyntaxChunkTypes.hpp"

namespace safec {

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

    StagedNodesType getStagedNodes() const
    {
        return mStagedNodes;
    }

    void stageNode(std::shared_ptr<SemNode> node)
    {
        mStagedNodes.push_back(node);
    }

private:
    std::vector<SyntaxChunkInfo> mSyntaxChunks;
    StagedNodesType mStagedNodes;
};

}
