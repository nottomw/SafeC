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
    [[maybe_unused]] static uint32_t lastShiftedIndex = stringIndex;

    //    log("\n");

    //    // Uncomment to print all incoming chunks...
    //    const auto typeStr = syntaxChunkTypeToStr(type);
    //    log("[ % at % -- % ]", {Color::LightCyan, logger::NewLine::No}) //
    //        .arg(typeStr)
    //        .arg(lastShiftedIndex)
    //        .arg(stringIndex);

    //    mState.printChunks();

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
                addNodeIntoCurrentScope(node);
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
            reduced();
            handleRelationalExpression(stringIndex, additional);
            break;

        case SyntaxChunkType::kPostfixExpression:
            reduced();
            handlePostfixExpression(stringIndex, additional);
            break;

        case SyntaxChunkType::kEmptyStatement:
            {
                auto node = std::make_shared<SemNodeEmptyStatement>(stringIndex);
                addNodeIntoCurrentScope(node);
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

    addNodeIntoCurrentScope(funNode);
    mState.addScope(funNode);
}

void Semantics::handleFunctionEnd(const uint32_t stringIndex)
{
    // TODO: for now clearing all chunks found in function body
    mState.getChunks().clear();

    auto functionNode = mState.getCurrentScope();

    const auto nodeType = functionNode->getType();
    if (nodeType != SemNode::Type::Function)
    {
        log("EXPECTED FUNCTION, GOT TYPE: %") //
            .arg(SemNode::TypeInfo::toStr(nodeType));
        return;
    }

    semNodeConvert<SemNodeFunction>(functionNode)->setEnd(stringIndex);

    mState.removeScope();
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

        addNodeIntoCurrentScope(declNode);

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

        addNodeIntoCurrentScope(nodeAsign);
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

        addNodeIntoCurrentScope(node);
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
    // TODO: ignore for now function calls
    if (op == "()" || op == "(...)")
    {
        mState.getChunks().clear();
        return;
    }

    auto &chunks = mState.getChunks();

    // at least LHS should be available
    assert(chunks.size() >= 1);

    if (chunks.size() == 1)
    {
        auto &chunkLhs = chunks[0];
        auto node = std::make_shared<SemNodePostfixExpression>(stringIndex, op, chunkLhs.mAdditional);
        addNodeIntoCurrentScope(node);
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
