#pragma once

#include "SemNode.hpp"

#include <cstdint>
#include <string>

namespace safec
{

class Semantics
{
public:
    void display() const;
    void newTranslationUnit();

    void print(const uint32_t stringIndex, const std::string &str);

    void handlePostfixExpression(const uint32_t stringIndex, const bool containsArguments);
    void handleDeferCall(const uint32_t stringIndex);
    void handleReturn(const uint32_t stringIndex, const bool returnValueAvailable);
    void handleCompoundStatementStart(const uint32_t stringIndex);
    void handleCompoundStatementEnd(const uint32_t stringIndex);
    void handleFunctionStart(const uint32_t stringIndex);
    void handleFunctionEnd(const uint32_t stringIndex);

private:
    SemNode mTranslationUnit;
};

} // namespace safec
