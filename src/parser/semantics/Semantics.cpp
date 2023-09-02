#include "Semantics.hpp"

#include "logger/Logger.hpp"
#include "walkers/SemNodeWalker.hpp"
#include "walkers/WalkerPrint.hpp"

#include <boost/filesystem/path.hpp>
#include <cassert>
#include <iostream>

// TODO: typedefs not supported at all, parser does not recognize

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
        , mLastSeenIdentifier{}
        , mLastSeenIdentifierPos{0}
        , mState{SState::Idle}
    {
    }

    using SemNodeScopePtr = std::weak_ptr<SemNodeScope>;
    std::vector<SemNodeScopePtr> mScopeStack;

    std::string mLastSeenIdentifier;
    uint32_t mLastSeenIdentifierPos;

    SState mState;

    std::vector<std::shared_ptr<SemNode>> mNodesToBeAddedToNextSeenScope;
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

void Semantics::handle( //
    const SyntaxChunkType type,
    const uint32_t stringIndex)
{
    static uint32_t lastStringIndex = 0;

    const auto typeStr = syntaxChunkTypeToStr(type);

    log("[ % at % -- % ]", {Color::LightCyan, logger::NewLine::No}) //
        .arg(typeStr)
        .arg(lastStringIndex)
        .arg(stringIndex);

    lastStringIndex = stringIndex;

    // TODO: actually do something with syntax chunk - create node etc
}

void Semantics::handleIdentifier(const uint32_t stringIndex, const char *const identifierName)
{
    gSemState.mLastSeenIdentifier = std::string(identifierName);
    gSemState.mLastSeenIdentifierPos = stringIndex;

    if (gSemState.mState == SState::WaitingForIdentifierReference)
    {
        gSemState.mState = SState::Idle;

        // TODO: reset the state in any other case where this is not
        // actually a reference.

        std::string syntaxReportStr = "reference: '";
        syntaxReportStr += gSemState.mLastSeenIdentifier;
        syntaxReportStr += "'";

        syntaxReport(stringIndex, syntaxReportStr, Color::LightBlue);

        // TODO: is there a possibility that the scope stack is not
        // empty and the reference should not be added to a newly created
        // scope?

        auto nodeRef = std::make_shared<SemNodeReference>(stringIndex, gSemState.mLastSeenIdentifier);

        if (gSemState.mScopeStack.empty() == true)
        {
            // There is no scope created yet - probably parser is dealing with
            // a newly created function - the reference should be added to next
            // seen scope.

            gSemState.mNodesToBeAddedToNextSeenScope.push_back(nodeRef);
        }
        else
        {
            auto currentScope = gSemState.mScopeStack.back();
            auto currentScopeSnap = currentScope.lock();
            assert(currentScopeSnap);

            currentScopeSnap->attach(nodeRef);
        }
    }
}

void Semantics::handleReference([[maybe_unused]] const uint32_t stringIndex)
{
    // Parser encountered what might be a reference - switch the internal
    // state so when parser moves on and finds an identifier, a SemNodeReference
    // is created.

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

    // Add any outstanding nodes waiting for their scope.
    auto currentScope = gSemState.mScopeStack.back();
    auto currentScopeSnap = currentScope.lock();
    assert(currentScopeSnap);

    for (auto &it : gSemState.mNodesToBeAddedToNextSeenScope)
    {
        currentScopeSnap->attach(it);
    }

    gSemState.mNodesToBeAddedToNextSeenScope.clear();
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
