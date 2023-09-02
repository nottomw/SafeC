#include "Semantics.hpp"

#include "logger/Logger.hpp"
#include "walkers/SemNodeWalker.hpp"
#include "walkers/WalkerPrint.hpp"

#include <boost/filesystem/path.hpp>
#include <cassert>
#include <iostream>

// TODO: typedefs not supported at all, current parser does not recognize

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
    static uint32_t lastStringIndex = 0;

    const auto typeStr = syntaxChunkTypeToStr(type);

    log("[ % at % -- % ]", {Color::LightCyan, logger::NewLine::No}) //
        .arg(typeStr)
        .arg(lastStringIndex)
        .arg(stringIndex);

    lastStringIndex = stringIndex;

    switch (type)
    {
        case SyntaxChunkType::kType:
            {
                if (mState.mState == SState::WaitingForStructType)
                {
                    // struct consumed - reset state
                    mState.mState = SState::Idle;
                    log("###NOT adding type - struct decl", Color::Red);
                }
                else
                {
                    log("###adding type", Color::Red);
                    mState.addChunk({type, stringIndex, additional});
                }
            }
            break;

        case SyntaxChunkType::kDirectDecl:
            log("###adding decl", Color::Red);
            mState.addChunk({type, stringIndex, additional});
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
            mState.getChunks().clear(); // TODO: handle struct decl
            mState.mState = SState::WaitingForStructType;
            break;

        case SyntaxChunkType::kPointerDecl:
            // TODO: handle pointer return from function
            //            log("###adding pointer", Color::Red);
            //            mState.addChunk({type, stringIndex});
            break;

        default:
            log("type not handled: %").arg(static_cast<uint32_t>(type));
            break;
    }
}

void Semantics::handleFunctionHeader( //
    const uint32_t stringIndex,
    const bool isVoidRetType)
{
    auto funNode = std::make_shared<SemNodeFunction>(stringIndex);

    // assumptions:
    //  - no other chunks stashed apart from ret type and params
    //  - chunks for function params are in order: type, name, type, name, ...

    auto &chunks = mState.getChunks();

    log("\nALL CHUNKS IN handleFunctionHeader:");
    for (auto &it : chunks)
    {
        log("CHUNK: additional: %, type: %") //
            .arg(it.mAdditional)
            .arg(syntaxChunkTypeToStr(it.mType));
    }

    auto chunksIt = chunks.begin();

    if (isVoidRetType)
    {
        funNode->setReturn("void");
    }
    else
    {
        funNode->setReturn(chunksIt->mAdditional);
    }

    // Move function return type out of the way
    chunksIt++;

    // now chunksIt should point to function name
    assert(chunksIt->mType == SyntaxChunkType::kDirectDecl);
    funNode->setName(chunksIt->mAdditional);

    // move function name out of the way
    chunksIt++;

    log("\n");

    uint32_t chunksCounter = 1U;
    bool alreadyConsumed = false;
    for (/* nothing */; chunksIt != chunks.end(); chunksIt++)
    {
        if (alreadyConsumed)
        {
            alreadyConsumed = false;
            continue;
        }

        const bool shouldBeArgName = ((chunksCounter % 2) == 0);

        if (shouldBeArgName)
        {
            assert(chunksIt->mType == SyntaxChunkType::kDirectDecl);
        }
        else
        {
            assert(chunksIt->mType == SyntaxChunkType::kType);
        }

        // +1 should be param name if type is not void
        if (chunksIt->mAdditional != "void")
        {
            auto chunksItParamName = chunksIt + 1;
            alreadyConsumed = true; // move param name out of the way

            log("\nadding function param: % %") //
                .arg(chunksIt->mAdditional)
                .arg(chunksItParamName->mAdditional);

            funNode->addParam(chunksIt->mAdditional, chunksItParamName->mAdditional);
        }
        else
        {
            log("\nno params");
        }
    }

    // chunks consumed
    chunks.clear();

    mState.stageNode(funNode);
}

void Semantics::handleFunctionEnd(const uint32_t stringIndex)
{
    // TODO: for now clearing all chunks found in function body
    mState.getChunks().clear();

    auto stagedNodes = mState.getStagedNodes();
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

    mTranslationUnit.attach(lastNode);
}

} // namespace safec
