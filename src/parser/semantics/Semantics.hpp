#pragma once

#include "SemNode.hpp"
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

    void handle(const SyntaxChunkType type, const uint32_t stringIndex, const bool shouldCount = true);

    void handlePostfixExpression(const uint32_t stringIndex, const bool containsArguments);
    void handleDeferCall(const uint32_t tokenStartStringIndex, const uint32_t stringIndex);
    void handleReturn(const uint32_t tokenStartStringIndex,
                      const uint32_t stringIndex,
                      const bool returnValueAvailable);

    void handleCompoundStatementStart(const uint32_t stringIndex);
    void handleCompoundStatementEnd(const uint32_t stringIndex);
    void handleFunctionStart(const uint32_t stringIndex, const bool voidReturnType);
    void handleFunctionEnd(const uint32_t stringIndex);

    void handleLoopStart(const uint32_t stringIndex);
    void handleLoopEnd(const uint32_t stringIndex);
    void handleBreak(const uint32_t tokenStartStringIndex, const uint32_t stringIndex);
    void handleContinue(const uint32_t tokenStartStringIndex, const uint32_t stringIndex);

    void handleIdentifier(const uint32_t stringIndex, const char *const identifierName);
    void handleReference(const uint32_t stringIndex);

private:
    SemNodeTranslationUnit mTranslationUnit;
    boost::iostreams::mapped_file_source mSemanticsSourceFile;
};

} // namespace safec
