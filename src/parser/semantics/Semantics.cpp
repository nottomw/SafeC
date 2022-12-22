#include "Semantics.hpp"

#include "logger/Logger.hpp"
#include "walkers/SemNodeWalker.hpp"
#include "walkers/WalkerPrint.hpp"

#include <boost/filesystem/path.hpp>
#include <cassert>
#include <iostream>

extern "C"
{
    extern int lex_current_char;
}

namespace safec
{

namespace bio = ::boost::iostreams;
namespace bfs = ::boost::filesystem;

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

    std::weak_ptr<SemNodeDefer> mDeferCallStart;
};

SemanticsState gSemState;

void syntaxReport(const uint32_t stringIndex, const char *const name, const Color color = Color::LightGreen)
{
    log("@ % at % @", {color, logger::NewLine::No}).arg(name).arg(stringIndex);
}

template <typename TUnderlyingSemNode>
std::shared_ptr<TUnderlyingSemNode> semNodeConvert(std::weak_ptr<SemNode> &w)
{
    auto snap = w.lock();
    return static_cast<std::shared_ptr<TUnderlyingSemNode>>(snap);
}

std::string deferGetText(const char *const str, const uint32_t len)
{
    const char *p = str;
    uint32_t i = 0U;
    for (i = 0U; i < len; ++i)
    {
        if (isspace(p[i]))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return std::string{&str[i], (len - i)};
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

void Semantics::newTranslationUnit(const bfs::path &path)
{
    mTranslationUnit.reset();
    mSemanticsSourceFile = bio::mapped_file_source{path};
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

// TODO: could be reworked to use "token start" from lexer
void Semantics::handleDeferCallStart( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "defer start", Color::LightBlue);

    auto currentScope = gSemState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    auto deferNode = std::make_shared<SemNodeDefer>(stringIndex);
    currentScopeSnap->attach(deferNode);
    gSemState.mDeferCallStart = deferNode;
}

void Semantics::handleDeferCall( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "defer end", Color::LightBlue);

    auto deferNode = gSemState.mDeferCallStart.lock();
    assert(deferNode);

    const uint32_t textStart = deferNode->getPos();
    const uint32_t textEnd = stringIndex;
    const uint32_t textLen = textEnd - textStart;
    auto text = deferGetText(&mSemanticsSourceFile.data()[textStart], textLen);

    deferNode->setDeferredText(std::move(text));
}

void Semantics::handleReturn( //
    [[maybe_unused]] const uint32_t tokenStartStringIndex,
    [[maybe_unused]] const uint32_t stringIndex,
    [[maybe_unused]] const bool returnValueAvailable)
{
    syntaxReport(stringIndex, "return", Color::LightRed);

    auto currentScope = gSemState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    currentScopeSnap->attach(std::make_shared<SemNodeReturn>(tokenStartStringIndex));
}

void Semantics::handleCompoundStatementStart( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "scope start");

    auto currentScope = gSemState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    auto scopeNode = std::make_shared<SemNodeScope>(stringIndex);
    currentScopeSnap->attach(scopeNode);

    gSemState.mScopeStack.push_back(scopeNode);
}

void Semantics::handleCompoundStatementEnd( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "scope end");

    auto currentScope = gSemState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    currentScopeSnap->setEnd(stringIndex);

    gSemState.mScopeStack.pop_back();
}

void Semantics::handleFunctionStart( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "function start", Color::LightPurple);

    auto functionNode = std::make_shared<SemNodeFunction>(stringIndex);
    mTranslationUnit.attach(functionNode);

    // Hacky: function start is matched with compound statement end.

    gSemState.mScopeStack.push_back(functionNode);
}

void Semantics::handleFunctionEnd( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "function end", Color::LightPurple);
}

void Semantics::handleLoopStart( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "loop start", Color::LightYellow);

    auto currentScope = gSemState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    auto loop = std::make_shared<SemNodeLoop>(stringIndex);
    currentScopeSnap->attach(loop);

    gSemState.mScopeStack.push_back(loop);
}

void Semantics::handleLoopEnd( //
    [[maybe_unused]] const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "loop end", Color::LightYellow);

    auto currentScope = gSemState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    currentScopeSnap->setEnd(stringIndex);

    gSemState.mScopeStack.pop_back();
}

void Semantics::handleBreak( //
    const uint32_t tokenStartStringIndex,
    const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "break", Color::LightYellow);

    auto currentScope = gSemState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    currentScopeSnap->attach(std::make_shared<SemNodeBreak>(tokenStartStringIndex));
}

void Semantics::handleContinue( //
    const uint32_t tokenStartStringIndex,
    const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "continue", Color::LightYellow);

    auto currentScope = gSemState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    currentScopeSnap->attach(std::make_shared<SemNodeContinue>(tokenStartStringIndex));
}

} // namespace safec
