#pragma once

#include "parser/semantics/SemNode.hpp"

#include <boost/filesystem.hpp>
#include <memory>

namespace bfs = boost::filesystem;

namespace safec
{

class Parser;

class Generator
{
public:
    void generate( //
        std::shared_ptr<SemNodeTranslationUnit> ast,
        const bfs::path &outputFile);

private:
};

} // namespace safec
