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
            handleDirectDecl(stringIndex, additional);
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
                auto currentScope = mState.getCurrentScope();
                auto scopeNode = semNodeConvert<SemNodeScope>(currentScope);
                scopeNode->setEnd(stringIndex);
                mState.removeScope();
            }
            break;

        case SyntaxChunkType::kForLoopHeader:
            {
                mState.mState = SState::InForLoopContext;
                auto node = std::make_shared<SemNodeLoop>(stringIndex, "for");
                addNodeToAst(node);
                mState.addScope(node);
            }
            break;

        case SyntaxChunkType::kForLoopConditions:
            {
                mState.mState = SState::Idle;
                handleForLoopConditions();
            }
            break;

        case SyntaxChunkType::kForLoop:
            {
                auto currentScope = mState.getCurrentScope();
                auto scopeNode = semNodeConvert<SemNodeScope>(currentScope);
                scopeNode->setEnd(stringIndex);

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
            handleReturn(stringIndex, additional);
            break;

        case SyntaxChunkType::kUnaryOp:
            {
                auto node = std::make_shared<SemNodeUnaryOp>(stringIndex, additional);

                if (additional == "++")
                {
                    // ++prefix;
                    auto &stagedNodes = mState.getStagedNodes();
                    assert(stagedNodes.size() >= 1);
                    auto rhs = stagedNodes.back();
                    stagedNodes.pop_back();
                    node->setRhs(rhs);
                    //                    addNodeToAst(node);
                    mState.stageNode(node);
                }
                else
                {
                    mState.stageNode(node);
                }
            }
            break;

        case SyntaxChunkType::kSimpleExpr:
            {
                if (mState.mState != SState::InForLoopContext)
                {
                    // flush staged nodes
                    auto &stagedNodes = mState.getStagedNodes();
                    for (auto &it : stagedNodes)
                    {
                        addNodeToAst(it);
                    }

                    stagedNodes.clear();
                }
            }
            break;

        case SyntaxChunkType::kBinaryOp:
            {
                handleBinaryOp(stringIndex, additional);
            }
            break;

        case SyntaxChunkType::kJumpStatement:
            handleJumpStatement(stringIndex, additional);
            break;

        case SyntaxChunkType::kWhileLoopHeader:
            {
                auto node = std::make_shared<SemNodeLoop>(stringIndex, "while");
                addNodeToAst(node);
                mState.addScope(node);
            }
            break;

        case SyntaxChunkType::kWhileLoopConditions:
            handleWhileLoopConditions();
            break;

        case SyntaxChunkType::kWhileLoop:
            {
                auto currentScope = mState.getCurrentScope();
                auto scopeNode = semNodeConvert<SemNodeScope>(currentScope);
                scopeNode->setEnd(stringIndex);

                mState.removeScope();
            }
            break;

        case SyntaxChunkType::kArrayDecl:
            {
                auto &stagedNodes = mState.getStagedNodes();
                assert(stagedNodes.size() >= 1);

                auto &node = stagedNodes.back();
                assert(node->getType() == SemNode::Type::Declaration);
                auto nodeDecl = semNodeConvert<SemNodeDeclaration>(node);
                nodeDecl->appendToType("[]");
            }
            break;

        case SyntaxChunkType::kInitializerList:
            handleInitializerList(stringIndex);
            break;

        case SyntaxChunkType::kDefer:
            handleDefer(stringIndex);
            break;

        default:
            log("type not handled: %", Color::Red, static_cast<uint32_t>(type));
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

    // clear all chunks when using ...function(void) the "void" is added
    // as chunk type "kType"
    auto &chunks = mState.getChunks();
    chunks.clear();

    addNodeToAst(node);
    mState.addScope(node);
}

void Semantics::handleFunctionEnd(const uint32_t stringIndex)
{
    auto currentScope = mState.getCurrentScope();
    auto scopeNode = semNodeConvert<SemNodeScope>(currentScope);
    scopeNode->setEnd(stringIndex);

    mState.removeScope();
}

void Semantics::handleInitDeclaration( //
    const uint32_t stringIndex,
    const bool withAssignment)
{
    auto &stagedNodes = mState.getStagedNodes();

    if (withAssignment)
    {
        if (stagedNodes.size() > 2)
        {
            foldUnaryOps(stagedNodes.size() - 1);
        }

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

    // If we have more that 2 staged nodes, probably
    // unary operator was used...
    if (stagedNodes.size() > 2)
    {
        foldUnaryOps(stagedNodes.size() - 1);
    }

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
    else if (op == "[]")
    {
        // should have index & array at least
        assert(stagedNodes.size() >= 2);

        auto indexExprNode = stagedNodes.back();
        stagedNodes.pop_back();

        auto lhs = stagedNodes.back();
        stagedNodes.pop_back();

        auto node = std::make_shared<SemNodePostfixExpression>(stringIndex, op, lhs);
        node->addArg(indexExprNode);
        mState.stageNode(node);
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

    // all staged nodes should be now placed into AST, since
    // we now encountered condition expression the previously
    // staged nodes will not be used for anything but should be
    // placed in "upper" scope
    for (auto &it : stagedNodes)
    {
        addNodeToAst(it);
    }
    stagedNodes.clear();

    addNodeToAst(nodeIf);
    mState.addScope(nodeIf);
}

void Semantics::handleWhileLoopConditions()
{
    auto &stagedNodes = mState.getStagedNodes();

    assert(stagedNodes.size() >= 1);

    auto itCond = stagedNodes.back();
    stagedNodes.pop_back();

    auto currentScope = mState.getCurrentScope(); // the loop
    auto loopNode = semNodeConvert<SemNodeLoop>(currentScope);
    loopNode->setIteratorCondition(itCond);
}

void Semantics::handleDirectDecl(const uint32_t stringIndex, const std::string &additional)
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
        assert((chunks[0].mType == SyntaxChunkType::kType) || //
               (chunks[0].mType == SyntaxChunkType::kPointer));

        uint32_t pointerCountStartIndex = 1;
        std::string typeStr = chunks[0].mAdditional;
        if (chunks[0].mType == SyntaxChunkType::kPointer)
        {
            pointerCountStartIndex = 0;
            typeStr = "void";
        }

        const uint32_t ptrCount = countPointersInChunks(pointerCountStartIndex);
        for (uint32_t i = 0; i < ptrCount; ++i)
        {
            typeStr += "*";
        }

        auto node = std::make_shared<SemNodeDeclaration>(stringIndex, typeStr, additional);
        mState.stageNode(node);
    }

    chunks.clear();
}

void Semantics::handleInitializerList(const uint32_t stringIndex)
{
    // braced init list
    auto nodeInitList = std::make_shared<SemNodeInitializerList>(stringIndex);

    auto &stagedNodes = mState.getStagedNodes();
    assert(stagedNodes.size() > 1);

    assert(stagedNodes[0]->getType() == SemNode::Type::BinaryOp);
    auto binOpNode = semNodeConvert<SemNodeBinaryOp>(stagedNodes[0]);

    // first staged node should be the declaration
    // all following nodes should be identifiers - contents
    // of the initializer list

    // fold any unary operators in init list
    const size_t stagedNodeLastIdx = stagedNodes.size() - 1;
    for (size_t i = stagedNodeLastIdx; i > 0 /* skip first */; i--)
    {
        auto node = stagedNodes[i];
        if (node->getType() == SemNode::Type::UnaryOp)
        {
            auto nodeUnaryOp = semNodeConvert<SemNodeUnaryOp>(node);

            const bool nextNodeInRange = ((i + 1) < stagedNodeLastIdx);
            if (nextNodeInRange)
            {
                auto nodeFollowing = stagedNodes[i + 1];
                nodeUnaryOp->setRhs(nodeFollowing);
                stagedNodes.erase(stagedNodes.begin() + i + 1);
            }
        }
    }

    for (size_t i = 1 /* skip first */; i < stagedNodes.size(); i++)
    {
        auto node = stagedNodes[i];
        nodeInitList->addEntry(node);
    }
    stagedNodes.erase(stagedNodes.begin() + 1, stagedNodes.end());

    mState.stageNode(nodeInitList);
}

void Semantics::handleDefer(const uint32_t stringIndex)
{
    auto &stagedNodes = mState.getStagedNodes();
    assert(stagedNodes.size() > 1);
}

void Semantics::handleBinaryOp(const uint32_t stringIndex, const std::string &op)
{
    auto &stagedNodes = mState.getStagedNodes();

    // since this is a binary op we should have
    // at least two staged nodes already
    assert(stagedNodes.size() >= 2);

    auto rhs = stagedNodes.back();
    stagedNodes.pop_back();
    auto lhs = stagedNodes.back();
    stagedNodes.pop_back();

    auto node = std::make_shared<SemNodeBinaryOp>(stringIndex, op, lhs);
    node->setRhs(rhs);

    mState.stageNode(node);
}

void Semantics::handleReturn(const uint32_t stringIndex, const std::string &additional)
{
    if (additional == "no-value")
    {
        auto node = std::make_shared<SemNodeReturn>(stringIndex);
        addNodeToAst(node);
    }
    else
    {
        auto &stagedNodes = mState.getStagedNodes();
        assert(stagedNodes.empty() == false);

        auto rhs = stagedNodes.back();
        stagedNodes.pop_back();

        auto finalRhs = rhs;

        if (stagedNodes.size() > 0)
        {
            auto possibleUnaryOp = stagedNodes.back();
            while (possibleUnaryOp && possibleUnaryOp->getType() == SemNode::Type::UnaryOp)
            {
                // TODO: how far back should this while() reach?
                semNodeConvert<SemNodeUnaryOp>(possibleUnaryOp)->setRhs(rhs);
                finalRhs = possibleUnaryOp;
                stagedNodes.pop_back();
                rhs = finalRhs;

                if (stagedNodes.empty() == true)
                {
                    break;
                }

                possibleUnaryOp = stagedNodes.back();
            }
        }

        auto node = std::make_shared<SemNodeReturn>(stringIndex, finalRhs);
        addNodeToAst(node);
    }
}

void Semantics::handleJumpStatement( //
    const uint32_t stringIndex,
    const std::string &stmtName)
{
    auto node = std::make_shared<SemNodeJumpStatement>(stringIndex, stmtName);
    addNodeToAst(node);
}

void Semantics::addNodeToAst(std::shared_ptr<SemNode> node)
{
    mState.getCurrentScope()->attach(node);
}

void Semantics::foldUnaryOps(const size_t start)
{
    auto &stagedNodes = mState.getStagedNodes();

    for (size_t i = start; i > 0; i--)
    {
        auto node = stagedNodes[i];
        if (node->getType() == SemNode::Type::UnaryOp)
        {
            const bool nextNodeInRange = ((i + 1) < stagedNodes.size());
            assert(nextNodeInRange);

            auto unaryOpNode = semNodeConvert<SemNodeUnaryOp>(node);
            unaryOpNode->setRhs(stagedNodes[i + 1]);
            stagedNodes.erase(stagedNodes.begin() + i + 1);
        }
    }
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
    log("\nSTAGED NODES [ % ], staged nodes:", Color::Green, str);
    auto &stagedNodes = mState.getStagedNodes();
    for (auto &it : stagedNodes)
    {
        log("\tstaged node: [ % ] %", Color::Green, SemNode::TypeInfo::toStr(it->getType()), it->toStr());
    }
}

} // namespace safec
