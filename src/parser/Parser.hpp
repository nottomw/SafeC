#pragma once

#include "ModPoint.hpp"

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

    void parse(const std::string &path);

private:
    void parseFile(const boost::filesystem::path &path);
    void addModPoint(ModPoint &&modPoint);

    void dumpFileWithModifications(const boost::filesystem::path &path);

    std::vector<ModPoint> mModPoints;
    Semantics &mSemantics;

    boost::filesystem::path mCurrentlyParsedFile;

    friend class Generator;
};

} // namespace safec
