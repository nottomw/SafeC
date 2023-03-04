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

enum class SState
{
    Idle,
    WaitingForIdentifierReference
};

struct SemanticsState
{
    SemanticsState() //
        : mScopeStack{}
    {
    }

    using SemNodeScopePtr = std::weak_ptr<SemNodeScope>;
    std::vector<SemNodeScopePtr> mScopeStack;

    std::string mLastSeenIdentifier;
    uint32_t mLastSeenIdentifierPos;

    SState mState;
};

SemanticsState gSemState;

void syntaxReport(const uint32_t stringIndex, const std::string &name, const Color color = Color::LightGreen)
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
    constexpr uint32_t deferKeywordSize = sizeof("defer");

    uint32_t i = deferKeywordSize;
    for (/* nothing */; i < len; ++i)
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

// TODO: handle indentation
// make sure that for each break/continue/return/scope end the indentation (column) is also
// saved so then it can be used to generate C code (verify if tabs/spaces used)

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

void Semantics::handleIdentifier(const uint32_t stringIndex, const char *const identifierName)
{
    gSemState.mLastSeenIdentifier = std::string(identifierName);
    gSemState.mLastSeenIdentifierPos = stringIndex;

    if (gSemState.mState == SState::WaitingForIdentifierReference)
    {
        std::string syntaxReportStr = "reference: '";
        syntaxReportStr += gSemState.mLastSeenIdentifier;
        syntaxReportStr += "'";

        syntaxReport(stringIndex, syntaxReportStr, Color::LightBlue);

        gSemState.mState = SState::Idle;
    }
}

void Semantics::handleReference([[maybe_unused]] const uint32_t stringIndex)
{
    gSemState.mState = SState::WaitingForIdentifierReference;
}

void Semantics::handlePostfixExpression( //
    [[maybe_unused]] const uint32_t stringIndex,
    [[maybe_unused]] const bool containsArguments)
{
}

void Semantics::handleDeferCall( //
    const uint32_t tokenStartStringIndex,
    const uint32_t stringIndex)
{
    syntaxReport(stringIndex, "defer", Color::LightBlue);

    auto currentScope = gSemState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    auto deferNode = std::make_shared<SemNodeDefer>(tokenStartStringIndex);
    currentScopeSnap->attach(deferNode);

    deferNode->setDeferredStatementLen(stringIndex - tokenStartStringIndex);

    const uint32_t textStart = tokenStartStringIndex;
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
    const uint32_t stringIndex,
    const bool voidReturnType)
{
    if (voidReturnType == true)
    {
        syntaxReport(stringIndex, "function start (void ret)", Color::LightPurple);
    }
    else
    {
        syntaxReport(stringIndex, "function start", Color::LightPurple);
    }

    auto functionNode = std::make_shared<SemNodeFunction>(stringIndex);
    mTranslationUnit.attach(functionNode);

    functionNode->setIsVoidReturnType(voidReturnType);

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

    // TODO: handle break when not in loop (switch..case)

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
