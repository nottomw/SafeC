#pragma once

#include "SemNode.hpp"

#include <cstdint>
#include <string>

namespace safec
{

class Semantics
{
public:
    Semantics();

    Semantics(const Semantics &) = delete;
    Semantics(Semantics &&) = delete;
    Semantics &operator=(const Semantics &) = delete;
    Semantics &operator=(Semantics &&) = delete;

    void display();
    void newTranslationUnit();

    void handlePostfixExpression(const uint32_t stringIndex, const bool containsArguments);
    void handleDeferCall(const uint32_t stringIndex);
    void handleReturn(const uint32_t stringIndex, const bool returnValueAvailable);

    void handleCompoundStatementStart(const uint32_t stringIndex);
    void handleCompoundStatementEnd(const uint32_t stringIndex);
    void handleFunctionStart(const uint32_t stringIndex);
    void handleFunctionEnd(const uint32_t stringIndex);

    void handleLoopStart(const uint32_t stringIndex);
    void handleLoopEnd(const uint32_t stringIndex);
    void handleBreak(const uint32_t stringIndex);
    void handleContinue(const uint32_t stringIndex);

private:
    SemNodeTranslationUnit mTranslationUnit;
};

} // namespace safec
