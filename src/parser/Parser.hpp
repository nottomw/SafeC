#pragma once

#include <any>
#include <boost/filesystem.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace safec
{

class Semantics;

class Parser final
{
public:
    Parser(Semantics &sem);
    ~Parser() = default;

    Parser(const Parser &) = delete;
    Parser(Parser &&) = delete;
    Parser &operator=(const Parser &) = delete;
    Parser &operator=(Parser &&) = delete;

    size_t parse(const std::string &path);
    void displayAst() const;
    void displayCoverage() const;

private:
    size_t parseFile(const boost::filesystem::path &path);
    void dumpFileWithModifications(const boost::filesystem::path &path);

    Semantics &mSemantics;
    boost::filesystem::path mCurrentlyParsedFile;

    friend class Generator;
};

} // namespace safec
