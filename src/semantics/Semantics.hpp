#pragma once

#include "SemanticsState.hpp"
#include "SyntaxChunkTypes.hpp"
#include "semantic_nodes/SemNode.hpp"

#include <cstdint>
#include <filesystem>
#include <string>

namespace safec
{

namespace fs = ::std::filesystem;

class SemNodeWalker;
class WalkerStrategy;

class Semantics
{
public:
    Semantics();

    Semantics(const Semantics &) = delete;
    Semantics(Semantics &&) = delete;
    Semantics &operator=(const Semantics &) = delete;
    Semantics &operator=(Semantics &&) = delete;

    void newTranslationUnit(const fs::path &path);
    void walk(SemNodeWalker &walker, WalkerStrategy &strategy);

    void handle( //
        const SyntaxChunkType type,
        const uint32_t stringIndex,
        const std::string &additional = "");

    std::shared_ptr<SemNodeTranslationUnit> getAst() const;

private:
    std::shared_ptr<SemNodeTranslationUnit> mTranslationUnit;

    SemanticsState mState;

    void handleFunctionHeader(const uint32_t stringIndex, const bool isVoidRetType);
    void handleFunctionEnd(const uint32_t stringIndex);
    void handleInitDeclaration(const uint32_t stringIndex, const bool withAssignment);
    void handleAssignment(const uint32_t stringIndex);
    void handleRelationalExpression(const uint32_t stringIndex, const std::string &op);
    void handlePostfixExpression(const uint32_t stringIndex, const std::string &op);
    void handleForLoopConditions(const uint32_t pos);
    void handleConditionExpression(const uint32_t stringIndex);
    void handleBinaryOp(const uint32_t stringIndex, const std::string &op);
    void handleReturn(const uint32_t stringIndex, const std::string &additional);
    void handleUnaryOp(const uint32_t stringIndex, const std::string &additional);
    void handleSimpleExpr(const uint32_t stringIndex);
    void handleJumpStatement(const uint32_t stringIndex, const std::string &stmtName);
    void handleWhileLoopConditions(const uint32_t stringIndex);
    void handleDirectDecl(const uint32_t stringIndex, const std::string &additional);
    void handleInitializerList(const uint32_t stringIndex);
    void handleDefer(const uint32_t stringIndex);
    void handleSwitchStatement(const uint32_t stringIndex);
    void handleSwitchEnd(const uint32_t stringIndex);
    void handleSwitchCaseHeader(const uint32_t stringIndex, const std::string &additional);
    void handleSwitchCaseEnd(const uint32_t stringIndex);

    void addNodeToAst(std::shared_ptr<SemNode> node);

    void foldUnaryOps(const size_t start);

    uint32_t countPointersInChunks(const uint32_t index);

    void printStagedNodes(const std::string &str = "");

    void setPrevReducePos(const uint32_t pos);

    uint32_t mPrevReducePos;
};

} // namespace safec
