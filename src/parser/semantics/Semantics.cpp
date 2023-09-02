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

[[maybe_unused]] void syntaxReport(const uint32_t stringIndex,
                                   const std::string &name,
                                   const Color color = Color::LightGreen)
{
    log("@ % at % @", {color, logger::NewLine::No}).arg(name).arg(stringIndex);
}

template <typename TUnderlyingSemNode>
std::shared_ptr<TUnderlyingSemNode> semNodeConvert(std::weak_ptr<SemNode> &w)
{
    auto snap = w.lock();
    return static_cast<std::shared_ptr<TUnderlyingSemNode>>(snap);
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

} // namespace safec
