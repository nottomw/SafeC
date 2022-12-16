#include "Semantics.hpp"

#include "walkers/SemNodeWalker.hpp"
#include "walkers/WalkerPrint.hpp"

#include <cassert>
#include <iostream>

extern "C"
{
    extern int characters;
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

// TODO: fix & move to logger when available
#define TERM_COLOR_NC "\033[0m"
#define TERM_COLOR_YELLOW "\033[00;33m"
#define TERM_COLOR_RED "\033[00;31m"
#define TERM_COLOR_GREEN "\033[00;32m"
#define TERM_COLOR_LRED "\033[01;31m"
#define TERM_COLOR_LGREEN "\033[01;32m"
#define TERM_COLOR_LYELLOW "\033[01;33m"
#define TERM_COLOR_LBLUE "\033[01;34m"
#define TERM_COLOR_LPURPLE "\033[01;35m"
#define TERM_COLOR_LCYAN "\033[01;36m"

static void syntaxReport(const uint32_t stringIndex,
                         const std::string &name,
                         const std::string color = TERM_COLOR_LCYAN)
{
    std::cout << color << "@ " << name << " at: " << stringIndex << " @" << TERM_COLOR_NC;
}

Semantics::Semantics() //
    : mTranslationUnit{SemNode::Type::TranslationUnit}
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
    syntaxReport(stringIndex, str, TERM_COLOR_LGREEN);
}

void Semantics::handlePostfixExpression( //
    [[maybe_unused]] const uint32_t stringIndex,
    [[maybe_unused]] const bool containsArguments)
{
    // if (containsArguments == true)
    // {
    //     syntaxReport(stringIndex, "call with args");
    // }
    // else
    // {
    //     syntaxReport(stringIndex, "call no args");
    // }
}

void Semantics::handleDeferCall(const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "defer", TERM_COLOR_LBLUE);

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
    // if (returnValueAvailable == true)
    // {
    //     syntaxReport(stringIndex, "return with value", TERM_COLOR_LRED);
    // }
    // else
    // {
    //     syntaxReport(stringIndex, "return", TERM_COLOR_LRED);
    // }
}

void Semantics::handleCompoundStatementStart(const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "scope start");

    auto currentScope = semState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    auto scopeNode = std::make_shared<SemNodeScope>(stringIndex);
    currentScopeSnap->attach(scopeNode);

    semState.mScopeStack.push_back(scopeNode);
}

void Semantics::handleCompoundStatementEnd(const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "scope end");

    {
        auto currentScope = semState.mScopeStack.back();
        auto currentScopeSnap = currentScope.lock();
        assert(currentScopeSnap);

        currentScopeSnap->setEnd(stringIndex);
    }

    semState.mScopeStack.pop_back();
}

void Semantics::handleFunctionStart(const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "function start", TERM_COLOR_LPURPLE);

    auto functionNode = std::make_shared<SemNodeFunction>(stringIndex);
    mTranslationUnit.attach(functionNode);

    semState.mScopeStack.push_back(functionNode);
}

void Semantics::handleFunctionEnd(const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "function end", TERM_COLOR_LPURPLE);

    // {
    //     auto currentFunction = semState.mScopeStack.back();
    //     auto lastFunctionNode = semNodeConvert<SemNodeFunction>(currentFunction);
    //     assert(lastFunctionNode);

    //     lastFunctionNode->setEnd(stringIndex);
    // }

    // semState.mScopeStack.pop_back();
}

} // namespace safec
