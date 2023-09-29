#pragma once

#include "SemNode.hpp"
#include "SemanticsState.hpp"
#include "SyntaxChunkTypes.hpp"

#include <boost/iostreams/device/mapped_file.hpp>
#include <cstdint>
#include <string>

namespace boost::filesystem
{
class path;
} // namespace boost::filesystem

namespace safec
{

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

    void display();
    void newTranslationUnit(const boost::filesystem::path &path);
    void walk(SemNodeWalker &walker, WalkerStrategy &strategy);

    void handle( //
        const SyntaxChunkType type,
        const uint32_t stringIndex,
        const std::string &additional = "");

private:
    std::shared_ptr<SemNodeTranslationUnit> mTranslationUnit;
    boost::iostreams::mapped_file_source mSemanticsSourceFile;

    SemanticsState mState;

    void handleFunctionHeader(const uint32_t stringIndex, const bool isVoidRetType);
    void handleFunctionEnd(const uint32_t stringIndex);
    void handleInitDeclaration(const uint32_t stringIndex, const bool withAssignment);
    void handleAssignment(const uint32_t stringIndex);
    void handleRelationalExpression(const uint32_t stringIndex, const std::string &op);
    void handlePostfixExpression(const uint32_t stringIndex, const std::string &op);
    void handleForLoopConditions();
    void handleConditionExpression(const uint32_t stringIndex);
    void handleBinaryOp(const uint32_t stringIndex, const std::string &op);
    void handleReturn(const uint32_t stringIndex, const std::string &additional);
    void handleJumpStatement(const uint32_t stringIndex, const std::string &stmtName);
    void handleWhileLoopConditions();
    void handleDirectDecl(const uint32_t stringIndex, const std::string &additional);
    void handleInitializerList(const uint32_t stringIndex);
    void handleDefer(const uint32_t stringIndex);

    void addNodeToAst(std::shared_ptr<SemNode> node);

    void foldUnaryOps(const size_t start);

    uint32_t countPointersInChunks(const uint32_t index);
    void stagedNodesPrint(const std::string &str = "");
};

} // namespace safec
