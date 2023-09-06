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
    auto funNode = std::make_shared<SemNodeFunction>(stringIndex);

    // assumptions:
    //  - no other chunks stashed apart from ret type, params and pointers
    //  - chunks for function params are in order: type, name, type, name, ... (except when pointers used)

    // TODO: need to fetch struct name function returns struct or has a struct param

    assert(mState.getChunks().size() == 0);

    // at this point the first direct decl in staged nodes should be the function name,
    // and the following direct decls should be the parameters
    auto &stagedNodes = mState.getStagedNodes();
    assert(stagedNodes.size() >= 1);

    log("\nSTAGED NODES: (%)\n", Color::Red).arg(stagedNodes.size());
    for (auto &it : stagedNodes)
    {
        log("staged node: %").arg(it->toStr());
    }

    //    funNode->setReturn(retTypeStr);

    addNodeIntoCurrentScope(funNode);
    mState.addScope(funNode);
}

void Semantics::handleFunctionEnd(const uint32_t stringIndex)
{
    // TODO: for now clearing all chunks found in function body
    mState.getChunks().clear();

    auto functionNode = mState.getCurrentScope();

    const auto nodeType = functionNode->getType();
    assert(nodeType == SemNode::Type::Function);

    semNodeConvert<SemNodeFunction>(functionNode)->setEnd(stringIndex);

    mState.removeScope();
}

void Semantics::handleInitDeclaration(const uint32_t stringIndex)
{
    auto &chunks = mState.getChunks();

    const auto chunksCount = chunks.size();

    // RHS could be also already packed into a node

    if (chunksCount == 0)
    {
        // No rhs
        return;
    }
    else
    {
        assert(chunks[0].mType == SyntaxChunkType::kAssignmentOperator);
    }

    chunks.clear(); // remove all processed chunks
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

        mState.stageNode(nodeAsign);
    }

    mState.getChunks().clear(); // all chunks consumed
}

void Semantics::handleRelationalExpression( //
    const uint32_t stringIndex,
    const std::string &op)
{
    auto &chunks = mState.getChunks();

    // At least two chunks should be available - lhs, rhs
    assert(chunks.size() >= 2);

    // easiest case for now...
    if (chunks.size() == 2)
    {
        auto &chunkLhs = chunks[0];
        auto &chunkRhs = chunks[1];

        auto node = std::make_shared<SemNodeRelationalExpression>( //
            stringIndex,
            op,
            chunkLhs.mAdditional,
            chunkRhs.mAdditional);

        mState.stageNode(node);
    }
    else
    {
        assert(false);
    }

    mState.getChunks().clear();
}

void Semantics::handlePostfixExpression( //
    const uint32_t stringIndex,
    const std::string &op)
{
    auto &chunks = mState.getChunks();

    // handle function call
    if (op == "()" || op == "(...)")
    {
        log("FUNCTION CALL CHUNKS:", Color::Red);
        mState.printChunks();

        if (op == "()")
        {
            // function call with no args
            // should just get an identifier (or some expression, TODO)
            auto &functionName = chunks.back();
            auto node = std::make_shared<SemNodePostfixExpression>(stringIndex, op, functionName.mAdditional);
            mState.stageNode(node);
        }
        else
        {
            // function call with args
        }

        mState.getChunks().clear();
        return;
    }

    // Handle ++ etc
    // at least LHS should be available
    assert(chunks.size() >= 1);

    if (chunks.size() == 1)
    {
        auto &chunkLhs = chunks[0];
        auto node = std::make_shared<SemNodePostfixExpression>(stringIndex, op, chunkLhs.mAdditional);
        mState.stageNode(node);
    }

    mState.getChunks().clear();
}

void Semantics::handleForLoopConditions()
{
    auto currentScope = mState.getCurrentScope();
    auto &currentScopeNodes = currentScope->getAttachedNodes();

    // this is hackish - the for(1; 2; 3) 1,2 and 3 statements are
    // now attached as nodes to the loop node, now they will be taken
    // out of the attached loop nodes and put into the loop itself

    // should have three nodes already attached: for(1; 2; 3)
    assert(currentScopeNodes.size() == 3);
    assert(currentScope->getType() == SemNode::Type::Loop);

    auto nodeInit = currentScopeNodes[0];
    auto nodeCond = currentScopeNodes[1];
    auto nodeChange = currentScopeNodes[2];

    // Current scope must be a loop here
    semNodeConvert<SemNodeLoop>(currentScope)->setIteratorInit(nodeInit);
    semNodeConvert<SemNodeLoop>(currentScope)->setIteratorCondition(nodeCond);
    semNodeConvert<SemNodeLoop>(currentScope)->setIteratorChange(nodeChange);

    currentScopeNodes.clear();
}

void Semantics::addNodeIntoCurrentScope(std::shared_ptr<SemNode> node)
{
    // TODO: extremely simple staging for now, requires handling of other
    // scopes, like plain {}, loops, conditions, ...

    auto currentScopeNode = mState.getCurrentScope();
    currentScopeNode->attach(node);
}

} // namespace safec
