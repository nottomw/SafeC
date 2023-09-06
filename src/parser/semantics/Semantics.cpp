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
    // this is actually another shift-reduce step, where shift chunks are
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
                const auto chunksSize = chunks.size();

                if (chunksSize == 0)
                {
                    std::string typeStr = "void";

                    const uint32_t ptrCount = countPointersInChunks(0);
                    for (uint32_t i = 0; i < ptrCount; ++i)
                    {
                        typeStr += "*";
                    }

                    // void decl - probably a function
                    auto node = std::make_shared<SemNodeDeclaration>(stringIndex, "void", additional);
                    mState.stageNode(node);
                }
                else
                {
                    // should have only type on stack, maybe with pointers
                    assert(chunksSize >= 1);
                    assert(chunks[0].mType == SyntaxChunkType::kType);

                    std::string typeStr = chunks[0].mAdditional;

                    const uint32_t ptrCount = countPointersInChunks(1);
                    for (uint32_t i = 0; i < ptrCount; ++i)
                    {
                        typeStr += "*";
                    }

                    auto node = std::make_shared<SemNodeDeclaration>(stringIndex, typeStr, additional);
                    mState.stageNode(node);
                }

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
            {
                auto node = std::make_shared<SemNodeConstant>(stringIndex, additional);
                mState.stageNode(node);
            }
            break;

        case SyntaxChunkType::kInitDeclaration:
            {
                const bool withAssignment = (additional == "asgn");
                handleInitDeclaration(stringIndex, withAssignment);
            }
            break;

        case SyntaxChunkType::kAssignment:
            handleAssignment(stringIndex);
            break;

        case SyntaxChunkType::kAssignmentOperator:
            {
                auto &stagedNodes = mState.getStagedNodes();
                auto lhs = stagedNodes.back();
                stagedNodes.pop_back();

                auto node = std::make_shared<SemNodeBinaryOp>(stringIndex, additional, lhs);

                mState.stageNode(node);
            }
            break;

        case SyntaxChunkType::kIdentifier:
            {
                auto node = std::make_shared<SemNodeIdentifier>(stringIndex, additional);
                mState.stageNode(node);
            }
            break;

        case SyntaxChunkType::kConditionHeader:
            break;

        case SyntaxChunkType::kConditionExpression:
            handleConditionExpression(stringIndex);
            break;

        case SyntaxChunkType::kCondition:
            {
                mState.removeScope();
            }
            break;

        case SyntaxChunkType::kForLoopHeader:
            {
                auto node = std::make_shared<SemNodeLoop>(stringIndex);
                addNodeToAst(node);
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

        case SyntaxChunkType::kReturn:
            {
                auto &stagedNodes = mState.getStagedNodes();
                auto rhs = stagedNodes.back();
                stagedNodes.pop_back();

                auto node = std::make_shared<SemNodeReturn>(stringIndex, rhs);
                addNodeToAst(node);
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
    auto &stagedNodes = mState.getStagedNodes();

    // first node is the function name & return type
    auto declNode = semNodeConvert<SemNodeDeclaration>(stagedNodes[0]);

    auto node = std::make_shared<SemNodeFunction>(stringIndex);
    node->setReturn(declNode->getLhsType());
    node->setName(declNode->getLhsIdentifier());

    // add function params
    for (uint32_t i = 1; i < stagedNodes.size(); i++)
    {
        auto it = stagedNodes[i];

        // all nodes should be direct decls now
        assert(it->getType() == SemNode::Type::Declaration);
        auto nodeDecl = semNodeConvert<SemNodeDeclaration>(it);
        node->addParam(nodeDecl->getLhsType(), nodeDecl->getLhsIdentifier());
    }

    stagedNodes.clear(); // all staged nodes should be consumed now

    addNodeToAst(node);
    mState.addScope(node);
}

void Semantics::handleFunctionEnd(const uint32_t stringIndex)
{
    mState.removeScope();
}

void Semantics::handleInitDeclaration( //
    const uint32_t stringIndex,
    const bool withAssignment)
{
    auto &stagedNodes = mState.getStagedNodes();

    if (withAssignment)
    {
        auto rhs = stagedNodes.back();
        stagedNodes.pop_back();
        auto binaryOp = semNodeConvert<SemNodeBinaryOp>(stagedNodes.back());
        stagedNodes.pop_back();
        binaryOp->setRhs(rhs);

        addNodeToAst(binaryOp);
    }
    else
    {
        auto decl = stagedNodes.back();
        stagedNodes.pop_back();

        addNodeToAst(decl);
    }
}

void Semantics::handleAssignment(const uint32_t stringIndex)
{
    auto &stagedNodes = mState.getStagedNodes();

    auto rhs = stagedNodes.back();
    stagedNodes.pop_back();
    auto binaryOp = semNodeConvert<SemNodeBinaryOp>(stagedNodes.back());
    stagedNodes.pop_back();
    binaryOp->setRhs(rhs);

    mState.stageNode(binaryOp);
}

void Semantics::handleRelationalExpression( //
    const uint32_t stringIndex,
    const std::string &op)
{
    auto &stagedNodes = mState.getStagedNodes();

    auto rhs = stagedNodes.back();
    stagedNodes.pop_back();

    auto lhs = stagedNodes.back();
    stagedNodes.pop_back();

    auto node = std::make_shared<SemNodeBinaryOp>(stringIndex, op, lhs);
    node->setRhs(rhs);

    mState.stageNode(node);
}

void Semantics::handlePostfixExpression( //
    const uint32_t stringIndex,
    const std::string &op)
{
    auto &stagedNodes = mState.getStagedNodes();

    if (op == "(...)")
    {
        auto &stagedNodes = mState.getStagedNodes();
        assert(stagedNodes.size() > 0);

        if (stagedNodes[0]->getType() == SemNode::Type::BinaryOp)
        {
            // this is postfix with binary op (i.e. int res = fun(a, b, c);)
            // should have staged binary op & function name at least
            assert(stagedNodes.size() > 2);

            auto functionNameNode = stagedNodes[1];
            auto node = std::make_shared<SemNodePostfixExpression>(stringIndex, op, functionNameNode);

            for (uint32_t i = 2; i < stagedNodes.size(); i++)
            {
                node->addArg(stagedNodes[i]);
            }

            stagedNodes.erase(stagedNodes.begin() + 1, stagedNodes.end());
            mState.stageNode(node);
        }
        else
        {
            auto functionNameNode = stagedNodes[0];
            auto node = std::make_shared<SemNodePostfixExpression>(stringIndex, op, functionNameNode);

            for (uint32_t i = 1; i < stagedNodes.size(); i++)
            {
                node->addArg(stagedNodes[i]);
            }

            stagedNodes.clear();
            mState.stageNode(node);
        }
    }
    else
    {
        auto lhs = stagedNodes.back();
        stagedNodes.pop_back();

        auto node = std::make_shared<SemNodePostfixExpression>(stringIndex, op, lhs);
        mState.stageNode(node);
    }
}

void Semantics::handleForLoopConditions()
{
    auto &stagedNodes = mState.getStagedNodes();

    auto itChange = stagedNodes.back();
    stagedNodes.pop_back();
    auto itCond = stagedNodes.back();
    stagedNodes.pop_back();
    auto itInit = stagedNodes.back();
    stagedNodes.pop_back();

    auto currentScope = mState.getCurrentScope(); // the loop
    auto loopNode = semNodeConvert<SemNodeLoop>(currentScope);
    loopNode->setIteratorInit(itInit);
    loopNode->setIteratorCondition(itCond);
    loopNode->setIteratorChange(itChange);
}

void Semantics::handleConditionExpression(const uint32_t stringIndex)
{
    auto &stagedNodes = mState.getStagedNodes();

    auto condNode = stagedNodes.back();
    stagedNodes.pop_back();

    auto nodeIf = std::make_shared<SemNodeIf>(stringIndex, condNode);
    addNodeToAst(nodeIf);
    mState.addScope(nodeIf);
}

void Semantics::addNodeToAst(std::shared_ptr<SemNode> node)
{
    mState.getCurrentScope()->attach(node);
}

uint32_t Semantics::countPointersInChunks(const uint32_t index)
{
    auto &chunks = mState.getChunks();
    const auto chunksSize = chunks.size();
    uint32_t ptrCount = 0;
    for (uint32_t i = index; i < chunksSize; ++i)
    {
        if (chunks[index].mType == SyntaxChunkType::kPointer)
        {
            ptrCount++;
        }
        else
        {
            break;
        }
    }

    chunks.erase(chunks.begin() + index, chunks.begin() + index + ptrCount);

    return ptrCount;
}

void Semantics::stagedNodesPrint(const std::string &str)
{
    log("\nSTAGED NODES [ % ], staged nodes:", Color::Red).arg(str);
    auto &stagedNodes = mState.getStagedNodes();
    for (auto &it : stagedNodes)
    {
        log("staged node: %", Color::Red).arg(it->toStr());
    }
}

} // namespace safec
