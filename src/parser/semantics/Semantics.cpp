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

void Semantics::walk(SemNodeWalker &walker, WalkerStrategy &strategy)
{
    walker.walk(mTranslationUnit, strategy);
}

void Semantics::handlePostfixExpression( //
    [[maybe_unused]] const uint32_t stringIndex,
    [[maybe_unused]] const bool containsArguments)
{
}

void Semantics::handleDeferCall( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    log::syntaxReport(stringIndex, "defer", log::Color::LightBlue);

    auto currentScope = semState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    // TODO: add metadata to defer - whole text call (help from lexer?)

    currentScopeSnap->attach(std::make_shared<SemNodeDefer>(stringIndex));
}

void Semantics::handleReturn( //
    [[maybe_unused]] const uint32_t stringIndex,
    [[maybe_unused]] const bool returnValueAvailable)
{
    log::syntaxReport(stringIndex, "return", log::Color::LightRed);

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

    auto currentScope = semState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    currentScopeSnap->setEnd(stringIndex);

    semState.mScopeStack.pop_back();
}

void Semantics::handleFunctionStart( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    log::syntaxReport(stringIndex, "function start", log::Color::LightPurple);

    auto functionNode = std::make_shared<SemNodeFunction>(stringIndex);
    mTranslationUnit.attach(functionNode);

    // Hacky: function start is matched with compound statement end.

    semState.mScopeStack.push_back(functionNode);
}

void Semantics::handleFunctionEnd( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    log::syntaxReport(stringIndex, "function end", log::Color::LightPurple);
}

void Semantics::handleLoopStart( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    log::syntaxReport(stringIndex, "loop start", log::Color::LightYellow);

    auto currentScope = semState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    auto loop = std::make_shared<SemNodeLoop>(stringIndex);
    currentScopeSnap->attach(loop);

    semState.mScopeStack.push_back(loop);
}

void Semantics::handleLoopEnd( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    log::syntaxReport(stringIndex, "loop end", log::Color::LightYellow);

    auto currentScope = semState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    currentScopeSnap->setEnd(stringIndex);

    semState.mScopeStack.pop_back();
}

void Semantics::handleBreak(const uint32_t stringIndex)
{
    log::syntaxReport(stringIndex, "break", log::Color::LightYellow);

    auto currentScope = semState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    currentScopeSnap->attach(std::make_shared<SemNodeBreak>(stringIndex));
}

void Semantics::handleContinue(const uint32_t stringIndex)
{
    log::syntaxReport(stringIndex, "continue", log::Color::LightYellow);

    auto currentScope = semState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    currentScopeSnap->attach(std::make_shared<SemNodeContinue>(stringIndex));
}

} // namespace safec
