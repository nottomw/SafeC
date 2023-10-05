#include "Semantics.hpp"

#include "config/Config.hpp"
#include "logger/Logger.hpp"
#include "walkers/SemNodeWalker.hpp"
#include "walkers/WalkerPrint.hpp"
#include "walkers/WalkerSourceCoverage.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>

// TODO: typedefs not supported at all, current parser does not recognize the type

namespace safec
{

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
    , mPrevReducePos{0}
{
    mState.addScope(mTranslationUnit);
}

void Semantics::newTranslationUnit(const fs::path &path)
{
    mTranslationUnit->reset();
    mTranslationUnit->setSourcePath(path);
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
    // huge switch..case, but leaving it here as-is for now to keep it simple
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
            {
                auto nodeIf = std::make_shared<SemNodeIf>(stringIndex);
                nodeIf->setSemStart(mPrevReducePos);
                mPrevReducePos = stringIndex;
                mState.stageNode(nodeIf);
            }
            break;

        case SyntaxChunkType::kConditionExpression:
            handleConditionExpression(stringIndex);
            break;

        case SyntaxChunkType::kCondition:
            {
                if (additional != "else")
                {
                    removeRedundantScopeFromCurrentScope();
                }

                auto currentScope = mState.getCurrentScope();
                auto scopeNode = semNodeConvert<SemNodeScope>(currentScope);
                scopeNode->setEnd(stringIndex);

                scopeNode->setSemEnd(stringIndex);
                mPrevReducePos = stringIndex;

                mState.removeScope();
            }
            break;

        case SyntaxChunkType::kForLoopHeader:
            {
                mState.mState = SState::InForLoopContext;
                auto node = std::make_shared<SemNodeLoop>(stringIndex, "for");

                node->setSemStart(mPrevReducePos);
                mPrevReducePos = stringIndex;

                addNodeToAst(node);
                mState.addScope(node);
            }
            break;

        case SyntaxChunkType::kForLoopConditions:
            {
                mState.mState = SState::Idle;
                handleForLoopConditions(stringIndex);
            }
            break;

        case SyntaxChunkType::kForLoop:
            {
                removeRedundantScopeFromCurrentScope();

                auto currentScope = mState.getCurrentScope();
                auto scopeNode = semNodeConvert<SemNodeScope>(currentScope);
                scopeNode->setEnd(stringIndex);

                scopeNode->setSemEnd(stringIndex);
                mPrevReducePos = stringIndex;

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
            handleUnaryOp(stringIndex, additional);
            break;

        case SyntaxChunkType::kSimpleExpr:
            handleSimpleExpr(stringIndex);
            break;

        case SyntaxChunkType::kBinaryOp:
            handleBinaryOp(stringIndex, additional);
            break;

        case SyntaxChunkType::kJumpStatement:
            handleJumpStatement(stringIndex, additional);
            break;

        case SyntaxChunkType::kWhileLoopHeader:
            {
                auto node = std::make_shared<SemNodeLoop>(stringIndex, "while");

                node->setSemStart(mPrevReducePos);
                mPrevReducePos = stringIndex;

                addNodeToAst(node);
                mState.addScope(node);
            }
            break;

        case SyntaxChunkType::kWhileLoopConditions:
            handleWhileLoopConditions(stringIndex);
            break;

        case SyntaxChunkType::kWhileLoop:
            {
                removeRedundantScopeFromCurrentScope();

                auto currentScope = mState.getCurrentScope();
                auto scopeNode = semNodeConvert<SemNodeScope>(currentScope);
                scopeNode->setEnd(stringIndex);

                scopeNode->setSemEnd(stringIndex);
                mPrevReducePos = stringIndex;

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

        case SyntaxChunkType::kDeferHeader:
            {
                auto deferNode = std::make_shared<SemNodeDefer>(stringIndex);
                deferNode->setSemStart(mPrevReducePos);
                mPrevReducePos = stringIndex;

                mState.mDeferNodeWaitingForDeferredOp = deferNode;
                mState.mState = SState::WaitingForDeferredOp;
            }
            break;

        case SyntaxChunkType::kDefer:
            handleDefer(stringIndex);
            break;

        case SyntaxChunkType::kSimpleScopeStart:
            {
                auto scopeNode = std::make_shared<SemNodeScope>(stringIndex);

                scopeNode->setSemStart(mPrevReducePos);
                mPrevReducePos = stringIndex;

                addNodeToAst(scopeNode);
                mState.addScope(scopeNode);
            }
            break;

        case SyntaxChunkType::kSimpleScopeEnd:
            {
                auto currentScope = mState.getCurrentScope();
                auto currentScopeTyped = semNodeConvert<SemNodeScope>(currentScope);
                currentScopeTyped->setEnd(stringIndex);

                currentScopeTyped->setSemEnd(stringIndex);
                mPrevReducePos = stringIndex;

                mState.removeScope();
            }
            break;

        case SyntaxChunkType::kSwitchHeader:
            {
                auto switchNode = std::make_shared<SemNodeSwitchCase>(stringIndex);

                switchNode->setSemStart(mPrevReducePos);
                mPrevReducePos = stringIndex;

                addNodeToAst(switchNode);
                mState.addScope(switchNode);
            }
            break;

        case SyntaxChunkType::kSwitchStatement:
            handleSwitchStatement(stringIndex);
            break;

        case SyntaxChunkType::kSwitchEnd:
            handleSwitchEnd(stringIndex);
            break;

        case SyntaxChunkType::kSwitchCaseHeader:
            handleSwitchCaseHeader(stringIndex, additional);
            break;

        case SyntaxChunkType::kSwitchCaseEnd:
            handleSwitchCaseEnd(stringIndex);
            break;

        default:
            log("type not handled: %", Color::Red, static_cast<uint32_t>(type));
            break;
    }
}

std::shared_ptr<SemNodeTranslationUnit> Semantics::getAst() const
{
    return mTranslationUnit;
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

    node->setSemStart(mPrevReducePos);
    mPrevReducePos = stringIndex;

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
    removeRedundantScopeFromCurrentScope();

    auto currentScopeFun = mState.getCurrentScope();
    assert(currentScopeFun->getType() == SemNode::Type::Function);
    auto funScopeNode = semNodeConvert<SemNodeFunction>(currentScopeFun);
    funScopeNode->setEnd(stringIndex);

    funScopeNode->setSemEnd(stringIndex);
    mPrevReducePos = stringIndex;

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

        // rhs might use a unary operator (&, *) -- this breaks
        // position setting for init decl, fix it here
        uint32_t fixedStartPos = rhs->getSemStart();
        if (fixedStartPos == 0)
        {
            fixedStartPos = mPrevReducePos;

            // check if initializer list with unary operator...
            if (rhs->getType() == SemNode::Type::InitializerList)
            {
                auto &initListEntries = semNodeConvert<SemNodeInitializerList>(rhs)->getEntries();
                if (initListEntries.size() > 0)
                {
                    const uint32_t initListWithUnaryOpSemStart = initListEntries[0]->getSemStart();
                    if (initListWithUnaryOpSemStart != 0)
                    {
                        fixedStartPos = initListWithUnaryOpSemStart;
                    }
                }
            }
        }

        binaryOp->setSemStart(fixedStartPos);
        binaryOp->setSemEnd(stringIndex);

        addNodeToAst(binaryOp);
    }
    else
    {
        auto decl = stagedNodes.back();
        stagedNodes.pop_back();

        decl->setSemStart(mPrevReducePos);
        decl->setSemEnd(stringIndex);

        addNodeToAst(decl);
    }

    mPrevReducePos = stringIndex;
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

    // if rhs contains an unary op, need to fix the position...
    uint32_t fixedStartPos = rhs->getSemStart();
    if (fixedStartPos == 0)
    {
        fixedStartPos = mPrevReducePos;
    }

    binaryOp->setSemStart(fixedStartPos);
    binaryOp->setSemEnd(stringIndex);
    mPrevReducePos = stringIndex;

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

    node->setSemStart(mPrevReducePos);
    node->setSemEnd(stringIndex);
    mPrevReducePos = stringIndex;

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

            node->setSemStart(mPrevReducePos);
            node->setSemEnd(stringIndex);
            mPrevReducePos = stringIndex;

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

            node->setSemStart(mPrevReducePos);
            node->setSemEnd(stringIndex);
            mPrevReducePos = stringIndex;

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

        node->setSemStart(mPrevReducePos);
        node->setSemEnd(stringIndex);
        mPrevReducePos = stringIndex;

        node->addArg(indexExprNode);
        mState.stageNode(node);
    }
    else
    {
        auto lhs = stagedNodes.back();
        stagedNodes.pop_back();

        auto node = std::make_shared<SemNodePostfixExpression>(stringIndex, op, lhs);

        node->setSemStart(mPrevReducePos);
        node->setSemEnd(stringIndex);
        mPrevReducePos = stringIndex;

        mState.stageNode(node);
    }
}

void Semantics::handleForLoopConditions(const uint32_t pos)
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

    // TODO: set pos to the group
    mPrevReducePos = pos;
}

void Semantics::handleConditionExpression(const uint32_t stringIndex)
{
    auto &stagedNodes = mState.getStagedNodes();

    auto condNode = stagedNodes.back();
    stagedNodes.pop_back();

    auto nodeIf = stagedNodes.back();
    stagedNodes.pop_back();

    assert(nodeIf->getType() == SemNode::Type::If);
    semNodeConvert<SemNodeIf>(nodeIf)->setCond(condNode);

    mPrevReducePos = stringIndex;

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

void Semantics::handleWhileLoopConditions(const uint32_t stringIndex)
{
    auto &stagedNodes = mState.getStagedNodes();

    assert(stagedNodes.size() >= 1);

    auto itCond = stagedNodes.back();
    stagedNodes.pop_back();

    auto currentScope = mState.getCurrentScope(); // the loop
    auto loopNode = semNodeConvert<SemNodeLoop>(currentScope);
    loopNode->setIteratorCondition(itCond);

    mPrevReducePos = stringIndex;
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
    assert(mState.mState == SState::WaitingForDeferredOp);
    mState.mState = SState::Idle;

    auto deferNode = mState.mDeferNodeWaitingForDeferredOp;
    mState.mDeferNodeWaitingForDeferredOp.reset();

    auto &stagedNodes = mState.getStagedNodes();
    assert(stagedNodes.size() >= 1);

    auto lastNode = stagedNodes.back();
    stagedNodes.pop_back();

    assert(deferNode->getType() == SemNode::Type::Defer);
    auto deferNodeTyped = semNodeConvert<SemNodeDefer>(deferNode);
    deferNodeTyped->setDeferredNode(lastNode);

    deferNode->setSemEnd(stringIndex);
    mPrevReducePos = stringIndex;

    mState.stageNode(deferNode);
}

void Semantics::handleSwitchStatement(const uint32_t stringIndex)
{
    auto &stagedNodes = mState.getStagedNodes();
    assert(stagedNodes.size() >= 1);

    auto lastNode = stagedNodes.back();
    stagedNodes.pop_back();

    // current scope must be a switch..case
    auto currentScope = mState.getCurrentScope();
    assert(currentScope->getType() == SemNode::Type::SwitchCase);

    auto switchCaseNode = semNodeConvert<SemNodeSwitchCase>(currentScope);
    switchCaseNode->setSwitchExpr(lastNode);

    mPrevReducePos = stringIndex;
}

void Semantics::handleSwitchEnd(const uint32_t stringIndex)
{
    removeRedundantScopeFromCurrentScope();

    auto currentScope = mState.getCurrentScope();

    // current scope must be a switch..case
    assert(currentScope->getType() == SemNode::Type::SwitchCase);

    auto switchCaseNode = semNodeConvert<SemNodeSwitchCase>(currentScope);
    switchCaseNode->setEnd(stringIndex);

    switchCaseNode->setSemEnd(stringIndex);
    mPrevReducePos = stringIndex;

    mState.removeScope();
}

void Semantics::handleSwitchCaseHeader(const uint32_t stringIndex, const std::string &additional)
{
    assert((additional == "case") || (additional == "default"));

    auto caseLabelNode = std::make_shared<SemNodeSwitchCaseLabel>(stringIndex);

    auto &stagedNodes = mState.getStagedNodes();

    // grab the case statement (grab X in "case X: ...")
    std::shared_ptr<SemNode> nodeToBeSetAsCaseLabel = //
        std::make_shared<SemNodeEmptyStatement>(stringIndex);
    if (additional == "case")
    {
        assert(stagedNodes.size() >= 1);
        nodeToBeSetAsCaseLabel = stagedNodes.back();
        stagedNodes.pop_back();
    }

    auto currentScope = mState.getCurrentScope();
    if (currentScope->getType() == SemNode::Type::SwitchCaseLabel)
    {
        // if the current scope is SwitchCaseLabel and we just saw a new case:, this
        // means the current scope is a fall-through case statement
        auto currentScopeLabel = semNodeConvert<SemNodeSwitchCaseLabel>(currentScope);
        currentScopeLabel->setIsFallthrough(true);

        // close current scope without waiting for parser, since we
        // already know the previous case statement is done
        // set stringIndex as previous reduce position, since we're
        // closing a previous parser match
        handleSwitchCaseEnd(mPrevReducePos);
    }
    else
    {
        // more complex case: the current scope is not a switch..case label,
        // but we still need to check for a fall-through case label that
        // contains some statements:
        // case 1: // fall-through
        //      someStatement;
        // case 2:

        // Currently this is handled in a bit sketchy way, inspecting
        // if there was a previous case label and the label contained "break".

        auto getLastAttachedNodeIfTypeMatches = //
            [&](const std::shared_ptr<SemNode> node,
                const SemNode::Type type) -> std::shared_ptr<SemNode> //
        {
            if (!node)
                return node;

            auto &nodesInScope = node->getAttachedNodes();
            if (nodesInScope.size() > 0)
            {
                auto lastNode = nodesInScope.back();
                if (lastNode->getType() == type)
                {
                    return lastNode;
                }
            }

            return std::shared_ptr<SemNode>{};
        };

        // if the last node in the current scope is a switch..case label (instead of break)
        // chances are that this is a fallthrough
        auto maybeSwitchCaseLabel = getLastAttachedNodeIfTypeMatches(currentScope, SemNode::Type::SwitchCaseLabel);
        if (maybeSwitchCaseLabel)
        {
            auto nd = semNodeConvert<SemNodeSwitchCaseLabel>(maybeSwitchCaseLabel);
            nd->setIsFallthrough(true);
        }

        // unfortunately it is possible to write
        // case X: { ...; break; }
        // instead of:
        // case X: { ...; } break;
        // this requires further inspection down the AST, since the AST
        // will now contain break inside of a scope inside of a case label node...

        auto maybeScope = //
            getLastAttachedNodeIfTypeMatches(maybeSwitchCaseLabel, SemNode::Type::Scope);
        auto maybeJumpStatement = //
            getLastAttachedNodeIfTypeMatches(maybeScope, SemNode::Type::JumpStatement);
        if (maybeJumpStatement)
        {
            auto jumpStatementTyped = semNodeConvert<SemNodeJumpStatement>(maybeJumpStatement);
            if (jumpStatementTyped->getName() == "break")
            {
                // turns out there was a break there...
                auto nd = semNodeConvert<SemNodeSwitchCaseLabel>(maybeSwitchCaseLabel);
                nd->setIsFallthrough(false);
            }
        }
    }

    caseLabelNode->setSemStart(mPrevReducePos);
    mPrevReducePos = stringIndex;

    caseLabelNode->setCaseLabel(nodeToBeSetAsCaseLabel);

    addNodeToAst(caseLabelNode);
    mState.addScope(caseLabelNode);
}

void Semantics::handleSwitchCaseEnd(const uint32_t stringIndex)
{
    auto currentScope = mState.getCurrentScope();
    if (currentScope->getType() == SemNode::Type::Scope)
    {
        // we are currently in the scope under SemNodeSwitch, this
        // means we had a fallthrough statements that were already closed,
        // ignore this chunk
        return;
    }

    // current scope must be a switch..case label
    assert(currentScope->getType() == SemNode::Type::SwitchCaseLabel);

    auto switchCaseLabelNode = semNodeConvert<SemNodeSwitchCaseLabel>(currentScope);
    switchCaseLabelNode->setEnd(stringIndex);

    switchCaseLabelNode->setSemEnd(stringIndex);
    mPrevReducePos = stringIndex;

    mState.removeScope();
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

        node->setSemStart(mPrevReducePos);
        node->setSemEnd(stringIndex);
        mPrevReducePos = stringIndex;

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

        // if returning unary op need to fix the sem position
        uint32_t fixedStartPos = rhs->getSemStart();
        if (fixedStartPos == 0)
        {
            fixedStartPos = mPrevReducePos;
        }

        node->setSemStart(fixedStartPos);
        node->setSemEnd(stringIndex);
        mPrevReducePos = stringIndex;

        addNodeToAst(node);
    }
}

void Semantics::handleUnaryOp(const uint32_t stringIndex, const std::string &additional)
{
    auto node = std::make_shared<SemNodeUnaryOp>(stringIndex, additional);

    node->setSemStart(mPrevReducePos);
    node->setSemEnd(stringIndex);
    mPrevReducePos = stringIndex;

    if (additional == "++")
    {
        // ++prefix;
        auto &stagedNodes = mState.getStagedNodes();
        assert(stagedNodes.size() >= 1);
        auto rhs = stagedNodes.back();
        stagedNodes.pop_back();
        node->setRhs(rhs);
        mState.stageNode(node);
    }
    else
    {
        mState.stageNode(node);
    }
}

void Semantics::handleSimpleExpr(const uint32_t stringIndex)
{
    auto &stagedNodes = mState.getStagedNodes();
    if (mState.mState != SState::InForLoopContext)
    {
        // flush staged nodes
        for (auto &it : stagedNodes)
        {
            addNodeToAst(it);
        }

        if (stagedNodes.size() > 0)
        {
            stagedNodes.back()->setSemEnd(stringIndex);
            mPrevReducePos = stringIndex;
        }

        stagedNodes.clear();
    }
}

void Semantics::handleJumpStatement( //
    const uint32_t stringIndex,
    const std::string &stmtName)
{
    auto node = std::make_shared<SemNodeJumpStatement>(stringIndex, stmtName);

    node->setSemStart(mPrevReducePos);
    node->setSemEnd(stringIndex);
    mPrevReducePos = stringIndex;

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

void Semantics::removeRedundantScopeFromCurrentScope()
{
    auto currentScope = mState.getCurrentScope();

    if (currentScope->getType() == SemNode::Type::Scope)
    {
        log("WARNING: removing redudant scope not in special scope - ignoring, type: %", //
            Color::Red,
            currentScope->getTypeStr());
        return;
    }

    const bool isSpecialScope =                                 //
        (currentScope->getType() == SemNode::Type::Function) || //
        (currentScope->getType() == SemNode::Type::Loop) ||     //
        (currentScope->getType() == SemNode::Type::If) ||       //
        (currentScope->getType() == SemNode::Type::SwitchCase);

    if (!isSpecialScope)
    {
        log("ERROR: removing redudant scope not in special scope - ignoring, type: %", //
            Color::Red,
            currentScope->getTypeStr());
        assert(isSpecialScope);
    }

    const bool isSingleNodeScope = //
        (currentScope->getType() == SemNode::Type::Function);

    auto specialCurrentScopeNode = semNodeConvert<SemNodeScope>(currentScope);
    auto &attachedNodes = specialCurrentScopeNode->getAttachedNodes();

    // special cases:
    // - empty function - no additional scope - do nothing
    // - if without scope (if(...) ...) - no additional scope - do nothing
    // - loop without scope - no additional scope - do nothing

    // loop & condition will have a single node or two nodes - one for
    // group - cond/loop statements (always) and one for scope (sometimes)
    const bool singleScopeAttached =                           //
        (!isSingleNodeScope && (attachedNodes.size() <= 2)) || //
        (isSingleNodeScope && (attachedNodes.size() <= 1));
    assert(singleScopeAttached);

    const bool functionWithRedundantScope = //
        isSingleNodeScope && (attachedNodes.size() == 1);
    const bool specialWithRedundantScope = //
        !isSingleNodeScope && (attachedNodes.size() == 2);

    if (functionWithRedundantScope || specialWithRedundantScope)
    {
        auto lastAttachedNode = attachedNodes.back();
        if (lastAttachedNode->getType() == SemNode::Type::Scope)
        {
            attachedNodes.pop_back(); // remove the redundant scope

            specialCurrentScopeNode        //
                ->devourAttachedNodesFrom( //
                    semNodeConvert<SemNodeScope>(lastAttachedNode));
        }
    }
}

void Semantics::printStagedNodes(const std::string &str)
{
    log("\nSTAGED NODES [ % ], staged nodes:", Color::Green, str);
    auto &stagedNodes = mState.getStagedNodes();
    for (auto &it : stagedNodes)
    {
        log("\tstaged node: [ % ] %", Color::Green, it->getTypeStr(), it->toStr());
    }
}

} // namespace safec
