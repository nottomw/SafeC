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
    const uint32_t stringIndex,
    const std::string &additional)
{
    log("\n");

    // Uncomment to print all incoming chunks...
    static uint32_t lastShiftedIndex = stringIndex;
    const auto typeStr = syntaxChunkTypeToStr(type);
    log("[ % at % -- % ]", {Color::LightCyan, logger::NewLine::No}) //
        .arg(typeStr)
        .arg(lastShiftedIndex)
        .arg(stringIndex);

    mState.printChunks();

    auto reduced = [&] { lastShiftedIndex = stringIndex; };

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
            mState.addChunk({type, stringIndex, additional});
            break;

        case SyntaxChunkType::kFunctionHeader:
            {
                reduced();
                const bool isVoidRetType = (additional == "void");
                handleFunctionHeader(stringIndex, isVoidRetType);
            }
            break;

        case SyntaxChunkType::kFunction:
            reduced();
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
            reduced();
            handleInitDeclaration(stringIndex);
            break;

        case SyntaxChunkType::kAssignment:
            reduced();
            handleAssignment(stringIndex);
            break;

        case SyntaxChunkType::kAssignmentOperator:
            mState.addChunk({type, stringIndex, additional});
            break;

        case SyntaxChunkType::kIdentifier:
            mState.addChunk({type, stringIndex, additional});
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
    auto funNode = std::make_shared<SemNodeFunction>(stringIndex);

    // assumptions:
    //  - no other chunks stashed apart from ret type, params and pointers
    //  - chunks for function params are in order: type, name, type, name, ... (except when pointers used)

    // TODO: need to fetch struct name function returns struct or has a struct param

    auto &chunks = mState.getChunks();

    auto chunksIt = chunks.begin();

    // Maybe could be handled nicer?
    uint32_t retTypePointerCount = 0U;
    while (chunksIt->mType == SyntaxChunkType::kPointer)
    {
        retTypePointerCount += 1;
        chunksIt++;
    }

    std::string retTypeStr;
    if (isVoidOrVoidPtrRetType)
    {
        retTypeStr = "void";
    }
    else
    {
        retTypeStr = chunksIt->mAdditional;
        // Move function return type out of the way
        chunksIt++;
    }

    // handle pointers
    for (uint32_t i = 0; i < retTypePointerCount; i++)
    {
        retTypeStr += "*";
    }

    funNode->setReturn(retTypeStr);

    // now chunksIt should point to function name
    assert(chunksIt->mType == SyntaxChunkType::kDirectDecl);
    funNode->setName(chunksIt->mAdditional);

    // move function name out of the way
    chunksIt++;

    while (chunksIt != chunks.end())
    {
        // +1 should be param name if type is not void
        auto chunksItParamName = chunksIt + 1;

        // If there is no following chunk we're sure this is not a pointer
        const bool notAVoidPointer = (chunksItParamName == chunks.end());

        // count pointers next to parsed argument
        uint32_t paramPointerCount = 0U;
        if (notAVoidPointer == false)
        {
            while (chunksItParamName->mType == SyntaxChunkType::kPointer)
            {
                paramPointerCount++;
                chunksItParamName++;
            }
        }

        const bool paramIsVoid = (chunksIt->mAdditional == "void") && (paramPointerCount == 0U);

        if (paramIsVoid == false)
        {
            // normal situation where we have a proper type (not void nor void pointer)

            std::string paramType = chunksIt->mAdditional;
            const std::string paramName = chunksItParamName->mAdditional;

            for (uint32_t i = 0; i < paramPointerCount; i++)
            {
                paramType += "*";
            }

            funNode->addParam(paramType, paramName);
            chunksItParamName++; // param name consumed
        }

        chunksIt = chunksItParamName;
    }

    // chunks consumed
    chunks.clear();

    mState.stageNode(funNode);
}

void Semantics::handleFunctionEnd(const uint32_t stringIndex)
{
    // TODO: for now clearing all chunks found in function body
    mState.getChunks().clear();

    auto &stagedNodes = mState.getStagedNodes();
    auto lastNode = stagedNodes.back();
    stagedNodes.pop_back();

    const auto nodeType = lastNode->getType();
    if (nodeType != SemNode::Type::Function)
    {
        log("EXPECTED FUNCTION, GOT TYPE: %") //
            .arg(SemNode::TypeInfo::toStr(nodeType));
        return;
    }

    semNodeConvert<SemNodeFunction>(lastNode)->setEnd(stringIndex);

    for (auto &stagedNode : stagedNodes)
    {
        lastNode->attach(stagedNode);
    }

    stagedNodes.clear();

    mTranslationUnit.attach(lastNode);
}

void Semantics::handleInitDeclaration(const uint32_t stringIndex)
{
    auto &chunks = mState.getChunks();

    const auto chunksCount = chunks.size();

    // should have at least two chunks now - type and name
    if (chunksCount >= 2)
    {
        // int var;
        // int var = otherVar;
        // int *var;
        // int *var = &otherVar;

        auto &chunkType = chunks[0];
        assert(chunkType.mType == SyntaxChunkType::kType);
        std::string chunkTypeStr = chunkType.mAdditional;

        uint32_t chunkIndex = 1;
        if (chunksCount > 2)
        {
            // at this point we might receive a pointer
            while (chunks[chunkIndex].mType == SyntaxChunkType::kPointer)
            {
                chunkTypeStr += "*";
                chunkIndex++;
            }
        }

        auto &chunkIdentifier = chunks[chunkIndex];
        assert(chunkIdentifier.mType == SyntaxChunkType::kDirectDecl);

        chunkIndex++; // move direct decl out of the way

        auto declNode = std::make_shared<SemNodeDeclaration>( //
            stringIndex,
            chunkTypeStr,
            chunkIdentifier.mAdditional);

        // TODO: kConstant should become a SemNodeConstant in expression
        // Is there a right-hand side?
        if (chunksCount > chunkIndex)
        {
            if (chunks[chunkIndex].mType == SyntaxChunkType::kConstant)
            {
                declNode->setRhs(chunks[chunkIndex].mAdditional);
            }
        }

        stageNode(declNode);

        chunks.clear(); // remove all processed chunks
    }
}

void Semantics::handleAssignment(const uint32_t stringIndex)
{
    auto &chunks = mState.getChunks();

    // At least three chunks should be available - lhs, operator, rhs
    assert(chunks.size() >= 3);

    // easiest case for now...
    if (chunks.size() == 3)
    {
        auto &chunkLhs = chunks[0];
        auto &chunkOperator = chunks[1];
        auto &chunkRhs = chunks[2];

        assert(chunkLhs.mType = SyntaxChunkType::kIdentifier);
        assert(chunkOperator.mType = SyntaxChunkType::kIdentifier);
        assert((chunkRhs.mType == SyntaxChunkType::kIdentifier) || //
               (chunkRhs.mType == SyntaxChunkType::kConstant));

        auto nodeAsign = std::make_shared<SemNodeAssignment>(
            stringIndex, chunkOperator.mAdditional, chunkLhs.mAdditional, chunkRhs.mAdditional);

        stageNode(nodeAsign);
    }

    mState.getChunks().clear(); // all chunks consumed
}

void Semantics::stageNode(std::shared_ptr<SemNode> node)
{
    // TODO: extremely simple staging for now, requires handling of other
    // scopes, like plain {}, loops, conditions, ...

    auto &stagedNodes = mState.getStagedNodes();

    if (stagedNodes.size() == 0)
    {
        // If there are no staged nodes, attach the node to translation unit
        mTranslationUnit.attach(node);
    }
    else
    {
        // If there are staged nodes, attach the node to the first staged node
        auto &firstStagedNode = stagedNodes[0];
        firstStagedNode->attach(node);
    }
}

} // namespace safec
