#pragma once

#include "ModPoint.hpp"

#include <any>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace boost::filesystem
{
class path;
} // namespace boost::filesystem

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

    std::vector<ModPoint> mModPoints;
    Semantics &mSemantics;
};

} // namespace safec
