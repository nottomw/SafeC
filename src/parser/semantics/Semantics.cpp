#include "Semantics.hpp"

#include "logger/Logger.hpp"
#include "walkers/SemNodeWalker.hpp"
#include "walkers/WalkerPrint.hpp"

#include <boost/filesystem/path.hpp>
#include <cassert>
#include <iostream>

// TODO: typedefs not supported at all, current parser does not recognize the type

// TODO: handle indentation
// make sure that for each break/continue/return/scope end the indentation (column) is also
// saved so then it can be used to generate C code (verify if tabs/spaces used)

namespace safec
{

namespace bio = ::boost::iostreams;
namespace bfs = ::boost::filesystem;

namespace
{

[[maybe_unused]] void syntaxReport(const uint32_t stringIndex,
                                   const std::string &name,
                                   const Color color = Color::LightGreen)
{
    log("@ % at % @", {color, logger::NewLine::No}).arg(name).arg(stringIndex);
}

template <typename TUnderlyingSemNode>
std::shared_ptr<TUnderlyingSemNode> semNodeConvert(std::shared_ptr<SemNode> &w)
{
    return std::static_pointer_cast<TUnderlyingSemNode>(w);
}

} // namespace

Semantics::Semantics() //
    : mTranslationUnit{std::make_shared<SemNodeTranslationUnit>()}
{
    mState.addScope(mTranslationUnit);
}

void Semantics::display()
{
    WalkerPrint printer;
    SemNodeWalker walker;
    walker.walk(*mTranslationUnit, printer);
}

void Semantics::newTranslationUnit(const bfs::path &path)
{
    mTranslationUnit->reset();
    mSemanticsSourceFile = bio::mapped_file_source{path};
}

void Semantics::walk(SemNodeWalker &walker, WalkerStrategy &strategy)
{
    walker.walk(*mTranslationUnit, strategy);
}

void Semantics::handle( //
    const SyntaxChunkType type,
    const uint32_t stringIndex,
    const std::string &additional)
{
    // TODO: this is actually another shift-reduce step, where shift chunks are
    // stashed and reduce should consume all shifted chunks. Could be used when
    // creating nodes to mark start-stop positions precisely.

    switch (type)
    {
        case SyntaxChunkType::kType:
            {
                if (mState.mState == SState::WaitingForStructType)
                {
                    // struct consumed - reset state
                    mState.mState = SState::Idle;
                }
                else
                {
                    mState.addChunk({type, stringIndex, additional});
                }
            }
            break;

        case SyntaxChunkType::kDirectDecl:
            {
                auto &chunks = mState.getChunks();

                // should have only type on stack
                assert(chunks.size() == 1);
                assert(chunks[0].mType == SyntaxChunkType::kType);

                auto node = std::make_shared<SemNodeDeclaration>(stringIndex, chunks[0].mAdditional, additional);
                mState.stageNode(node);

                chunks.clear();
            }
            break;

        case SyntaxChunkType::kFunctionHeader:
            {
                const bool isVoidRetType = (additional == "void");
                handleFunctionHeader(stringIndex, isVoidRetType);
            }
            break;

        case SyntaxChunkType::kFunction:
            handleFunctionEnd(stringIndex);
            break;

        case SyntaxChunkType::kStructOrUnionDecl:
            mState.getChunks().clear(); // TODO: handle struct declaration
            mState.mState = SState::WaitingForStructType;
            break;

        case SyntaxChunkType::kPointer:
            mState.addChunk({type, stringIndex});
            break;

        case SyntaxChunkType::kConstant:
            mState.addChunk({type, stringIndex, additional});
            break;

        case SyntaxChunkType::kInitDeclaration:
            handleInitDeclaration(stringIndex);
            break;

        case SyntaxChunkType::kAssignment:
            handleAssignment(stringIndex);
            break;

        case SyntaxChunkType::kAssignmentOperator:
            mState.addChunk({type, stringIndex, additional});
            break;

        case SyntaxChunkType::kIdentifier:
            mState.addChunk({type, stringIndex, additional});
            break;

        case SyntaxChunkType::kConditionHeader:
            break;

        case SyntaxChunkType::kConditionExpression:
            mState.getChunks().clear(); // TODO: condition now removed
            break;

        case SyntaxChunkType::kCondition:
            break;

        case SyntaxChunkType::kForLoopHeader:
            {
                auto node = std::make_shared<SemNodeLoop>(stringIndex);
                mState.stageNode(node);
                mState.addScope(node);
            }
            break;

        case SyntaxChunkType::kForLoopConditions:
            handleForLoopConditions();
            break;

        case SyntaxChunkType::kForLoop:
            {
                mState.removeScope();
            }
            break;

        case SyntaxChunkType::kRelationalExpression:
            handleRelationalExpression(stringIndex, additional);
            break;

        case SyntaxChunkType::kPostfixExpression:
            handlePostfixExpression(stringIndex, additional);
            break;

        case SyntaxChunkType::kEmptyStatement:
            {
                auto node = std::make_shared<SemNodeEmptyStatement>(stringIndex);
                mState.stageNode(node);
            }
            break;

        default:
            log("type not handled: %", {Color::Red}).arg(static_cast<uint32_t>(type));
            break;
    }
}

void Semantics::handleFunctionHeader( //
    const uint32_t stringIndex,
    const bool isVoidOrVoidPtrRetType)
{
}

void Semantics::handleFunctionEnd(const uint32_t stringIndex)
{
}

void Semantics::handleInitDeclaration(const uint32_t stringIndex)
{
}

void Semantics::handleAssignment(const uint32_t stringIndex)
{
}

void Semantics::handleRelationalExpression( //
    const uint32_t stringIndex,
    const std::string &op)
{
}

void Semantics::handlePostfixExpression( //
    const uint32_t stringIndex,
    const std::string &op)
{
}

void Semantics::handleForLoopConditions()
{
}

} // namespace safec
