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

    void printChunks() const
    {
        log("\n--- SYNTAX CHUNKS (chunks count: %):", {Color::Green}) //
            .arg(mSyntaxChunks.size());
        for (const auto &it : mSyntaxChunks)
        {
            log("---\t-> chunk type: %, pos: %, additional: '%'", {Color::Green}) //
                .arg(syntaxChunkTypeToStr(it.mType))
                .arg(it.mPos)
                .arg(it.mAdditional);
        }
    }

    std::shared_ptr<SemNode> getCurrentScope()
    {
        assert(mScope.size() > 0);
        return mScope.back();
    }

    void addStagedNodesToCurrentScope()
    {
        auto currentScope = getCurrentScope();

        for (auto &it : mStagedNodes)
        {
            currentScope->attach(it);
        }

        mStagedNodes.clear();
    }

    void addScope(std::shared_ptr<SemNode> node)
    {
        mScope.push_back(node);
    }

    void removeScope()
    {
        addStagedNodesToCurrentScope();
        mScope.pop_back();
    }

    void stageNode(std::shared_ptr<SemNode> node)
    {
        mStagedNodes.push_back(node);
    }

    std::vector<std::shared_ptr<SemNode>> &getStagedNodes()
    {
        return mStagedNodes;
    }

private:
    std::vector<SyntaxChunkInfo> mSyntaxChunks;
    std::vector<std::shared_ptr<SemNode>> mScope;

    std::vector<std::shared_ptr<SemNode>> mStagedNodes;
};

} // namespace safec
