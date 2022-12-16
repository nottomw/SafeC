#include "Semantics.hpp"

#include "logger/Logger.hpp"
#include "walkers/SemNodeWalker.hpp"
#include "walkers/WalkerPrint.hpp"

#include <cassert>
#include <iostream>

extern "C"
{
    extern int lex_current_char;
}

namespace safec
{

namespace
{

struct SemanticsState
{
    SemanticsState() //
        : mScopeStack{}
    {
    }

    using SemNodeScopePtr = std::weak_ptr<SemNodeScope>;
    std::vector<SemNodeScopePtr> mScopeStack;
};

SemanticsState semState;

template <typename TUnderlyingSemNode>
std::shared_ptr<TUnderlyingSemNode> semNodeConvert(std::weak_ptr<SemNode> &w)
{
    auto snap = w.lock();
    return static_cast<std::shared_ptr<TUnderlyingSemNode>>(snap);
}

} // namespace

Semantics::Semantics() //
    : mTranslationUnit{}
{
}

void Semantics::display()
{
    WalkerPrint printer;
    SemNodeWalker walker;
    walker.walk(mTranslationUnit, printer);
}

void Semantics::newTranslationUnit()
{
    mTranslationUnit.reset();
}

void Semantics::print(const uint32_t stringIndex, const std::string &str)
{
    log::syntaxReport(stringIndex, str, LOGGER_TERM_COLOR_LGREEN);
}

void Semantics::handlePostfixExpression( //
    [[maybe_unused]] const uint32_t stringIndex,
    [[maybe_unused]] const bool containsArguments)
{
    // if (containsArguments == true)
    // {
    //     log::syntaxReport(stringIndex, "call with args");
    // }
    // else
    // {
    //     log::syntaxReport(stringIndex, "call no args");
    // }
}

void Semantics::handleDeferCall( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    log::syntaxReport(stringIndex, "defer", LOGGER_TERM_COLOR_LBLUE);

    // Add deferred call to current scope.

    auto currentScope = semState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    currentScopeSnap->attach(std::make_shared<SemNodeDefer>(stringIndex));
}

void Semantics::handleReturn( //
    [[maybe_unused]] const uint32_t stringIndex,
    [[maybe_unused]] const bool returnValueAvailable)
{
    log::syntaxReport(stringIndex, "return", LOGGER_TERM_COLOR_LRED);

    auto currentScope = semState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    currentScopeSnap->attach(std::make_shared<SemNodeReturn>(stringIndex));
}

void Semantics::handleCompoundStatementStart( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    log::syntaxReport(stringIndex, "scope start");

    auto currentScope = semState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    auto scopeNode = std::make_shared<SemNodeScope>(stringIndex);
    currentScopeSnap->attach(scopeNode);

    semState.mScopeStack.push_back(scopeNode);
}

void Semantics::handleCompoundStatementEnd( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    log::syntaxReport(stringIndex, "scope end");

    {
        auto currentScope = semState.mScopeStack.back();
        auto currentScopeSnap = currentScope.lock();
        assert(currentScopeSnap);

        currentScopeSnap->setEnd(stringIndex);
    }

    semState.mScopeStack.pop_back();
}

void Semantics::handleFunctionStart( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    log::syntaxReport(stringIndex, "function start", LOGGER_TERM_COLOR_LPURPLE);

    auto functionNode = std::make_shared<SemNodeFunction>(stringIndex);
    mTranslationUnit.attach(functionNode);

    semState.mScopeStack.push_back(functionNode);
}

void Semantics::handleFunctionEnd( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    log::syntaxReport(stringIndex, "function end", LOGGER_TERM_COLOR_LPURPLE);
}

} // namespace safec
